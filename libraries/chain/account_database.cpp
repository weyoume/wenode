#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/db_with.hpp>
#include <node/chain/evaluator_registry.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/index.hpp>
#include <node/chain/node_evaluator.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/chain/shared_db_merkle.hpp>
#include <node/chain/operation_notification.hpp>
#include <node/chain/producer_schedule.hpp>


#include <node/chain/util/asset.hpp>
#include <node/chain/util/reward.hpp>
#include <node/chain/util/uint256.hpp>
#include <node/chain/util/reward.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/uint128.hpp>

#include <fc/container/deque.hpp>

#include <fc/io/fstream.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>


namespace node { namespace chain {

using boost::container::flat_set;

/**
 * Expires memberships, and renews recurring memberships when the 
 * membership expiration time is reached.
 */
void database::process_membership_updates()
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = head_block_time();
   const auto& account_idx = get_index< account_index >().indices().get< by_membership_expiration >();
   auto account_itr = account_idx.begin();
   
   while( account_itr != account_idx.end() && account_itr->membership_expiration >= now )
   {
      const account_object& member = *account_itr;

      if( member.recurring_membership > 0 )
      {
         const account_object& int_account = get_account( member.membership_interface );

         asset liquid = get_liquid_balance( member.name, SYMBOL_COIN );
         asset monthly_fee = asset( 0, SYMBOL_USD );

         switch( member.membership )
         {
            case membership_tier_type::NONE:
            {
               break; 
            }
            case membership_tier_type::STANDARD_MEMBERSHIP:
            {
               monthly_fee = median_props.membership_base_price;
               break; 
            }
            case membership_tier_type::MID_MEMBERSHIP:
            {
               monthly_fee = median_props.membership_mid_price;
               break; 
            }
            case membership_tier_type::TOP_MEMBERSHIP:
            {
               monthly_fee = median_props.membership_top_price;
               break; 
            }
            default:
            {
               FC_ASSERT( false, "Membership type Invalid: ${m}.", ("m", member.membership ) ); 
               break;
            }
         }

         asset total_fees = monthly_fee * member.recurring_membership; 

         if( liquid >= total_fees && total_fees.amount > 0)
         {
            pay_membership_fees( member, -total_fees, int_account );      // Pays splits to interface, premium partners, and network

            modify( member, [&]( account_object& a )
            {
               a.membership_expiration = now + fc::days( 30 * a.recurring_membership );
            });
         }
         else
         {
            modify( member, [&]( account_object& a )
            {
               a.membership = NONE;
               a.membership_expiration = fc::time_point::maximum();
               a.recurring_membership = 0;
               a.membership_interface = NULL_ACCOUNT;
            });
         } 
      }
      else
      {
         modify( member, [&]( account_object& a )
         {
            a.membership = NONE;
            a.membership_expiration = fc::time_point::maximum();
            a.recurring_membership = 0;
            a.membership_interface = NULL_ACCOUNT;
         });
      }
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays protocol membership fees, and splits to network contributors
 * member: The account that is paying to upgrade to a membership level
 * payment: The asset being received as payment
 * interface: The owner account of the interface that sold the membership
 */
asset database::pay_membership_fees( const account_object& member, const asset& mem_payment, const account_object& interface )
{ try {
   FC_ASSERT( mem_payment.symbol == SYMBOL_USD, 
      "Payment asset must be denominated in USD asset.");

   const reward_fund_object& reward_fund = get_reward_fund( SYMBOL_COIN );
   price usd_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).hour_median_price;
   asset membership_fee = mem_payment * usd_price;       // Membership fee denominated in Core asset

   asset network_fees = ( membership_fee * NETWORK_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( membership_fee * INTERFACE_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset partners_fees = ( membership_fee * PARTNERS_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;

   asset network_paid = pay_network_fees( member, network_fees );
   asset interface_paid = pay_fee_share( interface, interface_fees );

   asset total_fees = network_paid + interface_paid + partners_fees;
   adjust_liquid_balance( member.name, -total_fees );
   
   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_premium_partners_fund_balance( partners_fees );    // Adds funds to premium partners fund for distribution to premium creators
   });

   return total_fees;

} FC_CAPTURE_AND_RETHROW() }

/**
 * Pays protocol membership fees, and splits to network contributors
 * member: The account that is paying to upgrade to a membership level
 * payment: The asset being received as payment
 */
asset database::pay_membership_fees( const account_object& member, const asset& mem_payment )
{ try {
   FC_ASSERT( mem_payment.symbol == SYMBOL_USD, 
      "Payment asset must be denominated in USD asset.");

   const reward_fund_object& reward_fund = get_reward_fund( SYMBOL_COIN );
   price usd_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).hour_median_price;
   asset membership_fee = mem_payment * usd_price;       // Membership fee denominated in Core asset

   asset network_fees = ( membership_fee * NETWORK_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( membership_fee * INTERFACE_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset partners_fees = ( membership_fee * PARTNERS_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;

   asset network_paid = pay_network_fees( member, network_fees + interface_fees );
   
   asset total_fees = network_paid + partners_fees;
   adjust_liquid_balance( member.name, -total_fees );
   
   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_premium_partners_fund_balance( partners_fees );    // Adds funds to premium partners fund for distribution to premium creators
   });

   return total_fees;

} FC_CAPTURE_AND_RETHROW() }



/**
 * Updates Account reputation values depending on the account's total rewards,
 * and the total rewards of its followers, divided by the number of accounts each account follows. 
 */
void database::update_account_reputations()
{ try {
   if( (head_block_num() % REP_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;
   
   const auto& account_idx = get_index< account_index >().indices().get< by_total_rewards >();
   auto account_itr = account_idx.begin();
   flat_map< account_name_type, pair< share_type, uint32_t > > reward_map;
   vector< pair< account_name_type, share_type > > reward_vector; 
   reward_vector.reserve( account_idx.size() );

   while( account_itr != account_idx.end() && account_itr->total_rewards >= 0 )
   {
      reward_map[ account_itr->name ] = std::make_pair( account_itr->total_rewards, account_itr->following_count);
      ++account_itr;
   }

   for( auto account : reward_map )
   {
      const account_following_object& account_following = get_account_following( account.first );
      for( auto follower : account_following.followers )
      {
         reward_map[ account.first ].first += ( reward_map[ follower ].first / reward_map[ follower ].second );
      }
   }

   for( auto account : reward_map )
   {
      reward_vector.push_back( std::make_pair( account.first, account.second.first ));
   }

   std::sort( reward_vector.begin(), reward_vector.end(), [&]( pair< account_name_type, share_type > a, pair< account_name_type, share_type > b )
   {
      return a < b;
   });

   share_type user_count = reward_vector.size();

   for( auto i = 0; i < user_count; i++ )
   {
      const account_object& user = get_account( reward_vector[ i ].first );
      modify( user, [&]( account_object& a )
      {
         a.author_reputation = ( BLOCKCHAIN_PRECISION * ( user_count - i ) ) / user_count;
      });
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays an account its activity reward shares from the reward pool.
 */
asset database::claim_activity_reward( const account_object& account, 
   const producer_object& producer, asset_symbol_type currency_symbol )
{ try {
   const asset_currency_data_object& currency = get_currency_data( currency_symbol );
   const reward_fund_object& reward_fund = get_reward_fund( currency_symbol );
   const account_balance_object& abo = get_account_balance( account.name, currency.equity_asset );
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = props.time;
   auto decay_rate = RECENT_REWARD_DECAY_RATE;
   price equity_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_EQUITY ).hour_median_price;
   share_type activity_shares = BLOCKCHAIN_PRECISION;

   if( abo.staked_balance >= 10 * BLOCKCHAIN_PRECISION )
   {
      activity_shares *= 2;
   }
   if( account.membership == membership_tier_type::STANDARD_MEMBERSHIP )
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_STANDARD_PERCENT) / PERCENT_100;
   }
   else if( account.membership == membership_tier_type::MID_MEMBERSHIP )
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_MID_PERCENT) / PERCENT_100;
   }
   else if( account.membership == membership_tier_type::TOP_MEMBERSHIP )
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_TOP_PERCENT) / PERCENT_100;
   }

   // Decay recent claims of activity reward fund and add new shares of this claim.
   modify( reward_fund, [&]( reward_fund_object& rfo )   
   {
      rfo.recent_activity_claims -= ( rfo.recent_activity_claims * ( now - rfo.last_updated ).to_seconds() ) / decay_rate.to_seconds();
      rfo.last_updated = now;
      rfo.recent_activity_claims += activity_shares.value;
   }); 

   asset activity_reward = asset( ( reward_fund.activity_reward_balance.amount * activity_shares ) / int64_t( reward_fund.recent_activity_claims.to_uint64() ), currency_symbol );

   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_activity_reward_balance( -activity_reward );    // Deduct activity reward from reward fund.
   });

   adjust_pending_supply( -activity_reward );                    // Deduct activity reward from pending supply.
   
   // Update recent activity claims on account
   modify( account, [&]( account_object& a )
   {
      a.recent_activity_claims -= ( a.recent_activity_claims * ( now - a.last_activity_reward ).to_seconds() ) / decay_rate.to_seconds();
      a.last_activity_reward = now;                              // Update activity reward time.
      a.recent_activity_claims += BLOCKCHAIN_PRECISION;          // Increments rolling activity average by one claim.
   });

   adjust_reward_balance( account.name, activity_reward );            // Add activity reward to reward balance of claiming account. 

   uint128_t voting_power = get_voting_power( account.name, equity_price ).value + get_proxied_voting_power( account.name, equity_price ).value;

   modify( producer, [&]( producer_object& p )
   {
      p.accumulated_activity_stake += voting_power;
      p.decay_weights( now, median_props );
   });

   return activity_reward;

} FC_CAPTURE_AND_RETHROW( (account.name) ) }


void database::update_owner_authority( const account_object& account, const authority& owner_authority )
{
   if( head_block_num() >= OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM )
   {
      create< owner_authority_history_object >( [&]( owner_authority_history_object& hist )
      {
         hist.account = account.name;
         hist.previous_owner_authority = get< account_authority_object, by_account >( account.name ).owner_auth;
         hist.last_valid_time = head_block_time();
      });
   }

   modify( get< account_authority_object, by_account >( account.name ), [&]( account_authority_object& auth )
   {
      auth.owner_auth = owner_authority;
      auth.last_owner_update = head_block_time();
   });
}

/**
 * Aligns producer votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_producer_votes( const account_object& account )
{
   const auto& vote_idx = get_index< producer_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const producer_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( producer_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }

   modify( account, [&]( account_object& a )
   {
      a.producer_vote_count = ( new_vote_rank - 1 );
   });
}

/**
 * Aligns producer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_producer_votes( const account_object& account, const account_name_type& producer, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< producer_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const producer_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( producer_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< producer_vote_object >([&]( producer_vote_object& v )
   {
      v.account = account.name;
      v.producer = producer;
      v.vote_rank = input_vote_rank;
   });

   modify( account, [&]( account_object& a )
   {
      a.producer_vote_count = ( new_vote_rank - 1 );
   });
}

/**
 * Aligns network_officer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_network_officer_votes( const account_object& account )
{
   const auto& vote_idx = get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   flat_map< network_officer_role_type, uint16_t > vote_rank;
   vote_rank[ network_officer_role_type::DEVELOPMENT ] = 1;
   vote_rank[ network_officer_role_type::MARKETING ] = 1;
   vote_rank[ network_officer_role_type::ADVOCACY ] = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const network_officer_vote_object& vote = *vote_itr;
      if( vote.vote_rank != vote_rank[ vote.officer_type ] )
      {
         modify( vote, [&]( network_officer_vote_object& v )
         {
            v.vote_rank = vote_rank[ vote.officer_type ];   // Updates vote rank to linear order of index retrieval.
         });
      }
      vote_rank[ vote.officer_type ]++;
      ++vote_itr;
   }

   modify( account, [&]( account_object& a )
   {
      a.officer_vote_count = ( vote_rank[ network_officer_role_type::DEVELOPMENT ] + vote_rank[ network_officer_role_type::MARKETING ] + vote_rank[ network_officer_role_type::ADVOCACY ] - 3 );
   });
}

/**
 * Aligns network_officer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_network_officer_votes( const account_object& account, const account_name_type& network_officer, 
   network_officer_role_type officer_type, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   flat_map< network_officer_role_type, uint16_t > vote_rank;
   vote_rank[ network_officer_role_type::DEVELOPMENT ] = 1;
   vote_rank[ network_officer_role_type::MARKETING ] = 1;
   vote_rank[ network_officer_role_type::ADVOCACY ] = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const network_officer_vote_object& vote = *vote_itr;

      if( vote.vote_rank == input_vote_rank && vote.officer_type == officer_type )
      {
         vote_rank[ vote.officer_type ]++;
      }
      if( vote.vote_rank != vote_rank[ vote.officer_type ] )
      {
         modify( vote, [&]( network_officer_vote_object& v )
         {
            v.vote_rank = vote_rank[ vote.officer_type ];   // Updates vote rank to linear order of index retrieval.
         });
      }
      vote_rank[ vote.officer_type ]++;
      ++vote_itr;
   }

   create< network_officer_vote_object >([&]( network_officer_vote_object& v )
   {
      v.account = account.name;
      v.network_officer = network_officer;
      v.officer_type = officer_type;
      v.vote_rank = input_vote_rank;
   });

   modify( account, [&]( account_object& a )
   {
      a.officer_vote_count = ( vote_rank[ network_officer_role_type::DEVELOPMENT ] + vote_rank[ network_officer_role_type::MARKETING ] + vote_rank[ network_officer_role_type::ADVOCACY ] - 3 );
   });
}


/**
 * Aligns executive board votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_executive_board_votes( const account_object& account )
{
   const auto& vote_idx = get_index< executive_board_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const executive_board_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( executive_board_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }

   modify( account, [&]( account_object& a )
   {
      a.executive_board_vote_count = ( new_vote_rank - 1 );
   });
}

/**
 * Aligns executive board votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_executive_board_votes( const account_object& account, const account_name_type& executive, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< executive_board_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const executive_board_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( executive_board_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< executive_board_vote_object >([&]( executive_board_vote_object& v )
   {
      v.account = account.name;
      v.executive_board = executive;
      v.vote_rank = input_vote_rank;
   });

   modify( account, [&]( account_object& a )
   {
      a.executive_board_vote_count = ( new_vote_rank - 1 );
   });
}


/**
 * Aligns community moderator votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_community_moderator_votes( const account_object& account, const community_name_type& community )
{
   const auto& vote_idx = get_index< community_moderator_vote_index >().indices().get< by_account_community_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, community ) );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->community == community )
   {
      const community_moderator_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( community_moderator_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }
}

/**
 * Aligns community moderator votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_community_moderator_votes( const account_object& account, const community_name_type& community, 
   const account_name_type& moderator, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< community_moderator_vote_index >().indices().get< by_account_community_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, community ) );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->community == community )
   {
      const community_moderator_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( community_moderator_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< community_moderator_vote_object >([&]( community_moderator_vote_object& v )
   {
      v.account = account.name;
      v.moderator = moderator;
      v.community = community;
      v.vote_rank = input_vote_rank;
   });
}


/**
 * Aligns enterprise approval votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_enterprise_approvals( const account_object& account )
{
   const auto& vote_idx = get_index< enterprise_approval_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const enterprise_approval_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( enterprise_approval_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }
}

/**
 * Aligns enterprise approval votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_enterprise_approvals( const account_object& account, const account_name_type& creator,
   string enterprise_id, uint16_t input_vote_rank, int16_t input_milestone )
{
   const auto& vote_idx = get_index< enterprise_approval_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, 1 ) );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const enterprise_approval_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( enterprise_approval_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< enterprise_approval_object >([&]( enterprise_approval_object& v )
   {
      v.account = account.name;
      from_string( v.enterprise_id, enterprise_id );
      v.creator = creator;
      v.vote_rank = input_vote_rank;
      v.milestone = input_milestone;
   });
}


/**
 * Aligns business account executive votes in a continuous order.
 */
void database::update_account_executive_votes( const account_object& account, const account_name_type& business )
{
   const auto& vote_idx = get_index< account_executive_vote_index >().indices().get< by_account_business_role_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );

   flat_map< executive_role_type, uint16_t >vote_rank;
   vote_rank[ executive_role_type::CHIEF_EXECUTIVE_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_OPERATING_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_FINANCIAL_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_TECHNOLOGY_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_DEVELOPMENT_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_MARKETING_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_ADVOCACY_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_GOVERNANCE_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_SECURITY_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_DESIGN_OFFICER ] = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->business_account == business )
   {
      const account_executive_vote_object& vote = *vote_itr;

      if( vote.vote_rank != vote_rank[vote.role] )
      {
         modify( vote, [&]( account_executive_vote_object& v )
         {
            v.vote_rank = vote_rank[vote.role];
         });
      }
      vote_rank[vote.role]++;
      ++vote_itr;  
   }
}

/**
 * Aligns business account executive votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_account_executive_votes( const account_object& account, const account_name_type& business, const account_object& executive,
   executive_role_type role, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< account_executive_vote_index >().indices().get< by_account_business_role_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );

   flat_map< executive_role_type, uint16_t >vote_rank;
   vote_rank[ executive_role_type::CHIEF_EXECUTIVE_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_OPERATING_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_FINANCIAL_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_TECHNOLOGY_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_DEVELOPMENT_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_MARKETING_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_ADVOCACY_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_GOVERNANCE_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_SECURITY_OFFICER ] = 1;
   vote_rank[ executive_role_type::CHIEF_DESIGN_OFFICER ] = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->business_account == business )
   {
      const account_executive_vote_object& vote = *vote_itr;

      if( vote.vote_rank == input_vote_rank && vote.role == role )
      {
         vote_rank[vote.role]++;
      }
      if( vote.vote_rank != vote_rank[vote.role] )
      {
         modify( vote, [&]( account_executive_vote_object& v )
         {
            v.vote_rank = vote_rank[vote.role];
         });
      }
      vote_rank[vote.role]++;
      ++vote_itr;  
   }

   create< account_executive_vote_object >([&]( account_executive_vote_object& v )
   {
      v.account = account.name;
      v.business_account = business;
      v.executive_account = executive.name;
      v.role = role;
      v.vote_rank = input_vote_rank;
   });
}

/**
 * Aligns business account officer votes in a continuous order
 */
void database::update_account_officer_votes( const account_object& account, const account_name_type& business )
{
   const auto& vote_idx = get_index< account_officer_vote_index >().indices().get< by_account_business_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->business_account == business )
   {
      const account_officer_vote_object& vote = *vote_itr;

      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( account_officer_vote_object& v )
         {
            v.vote_rank = new_vote_rank;
         });
      }
      new_vote_rank++;
      ++vote_itr;  
   }
}

/**
 * Aligns business account officer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_account_officer_votes( const account_object& account, const account_name_type& business, const account_object& officer, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< account_officer_vote_index >().indices().get< by_account_business_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->business_account == business )
   {
      const account_officer_vote_object& vote = *vote_itr;

      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( account_officer_vote_object& v )
         {
            v.vote_rank = new_vote_rank;
         });
      }
      new_vote_rank++;
      ++vote_itr;  
   }

   create< account_officer_vote_object >([&]( account_officer_vote_object& v )
   {
      v.account = account.name;
      v.business_account = business;
      v.officer_account = officer.name;
      v.vote_rank = input_vote_rank;
   });
}


void database::account_recovery_processing()
{
   // Clear expired recovery requests
   const auto& rec_req_idx = get_index< account_recovery_request_index >().indices().get< by_expiration >();
   auto rec_req = rec_req_idx.begin();

   while( rec_req != rec_req_idx.end() && rec_req->expiration <= head_block_time() )
   {
      remove( *rec_req );
      rec_req = rec_req_idx.begin();
   }

   // Clear invalid historical authorities
   const auto& hist_idx = get_index< owner_authority_history_index >().indices(); //by id
   auto hist = hist_idx.begin();

   while( hist != hist_idx.end() && time_point( hist->last_valid_time + OWNER_AUTH_RECOVERY_PERIOD ) < head_block_time() )
   {
      remove( *hist );
      hist = hist_idx.begin();
   }

   // Apply effective recovery_account changes
   const auto& change_req_idx = get_index< change_recovery_account_request_index >().indices().get< by_effective_date >();
   auto change_req = change_req_idx.begin();

   while( change_req != change_req_idx.end() && change_req->effective_on <= head_block_time() )
   {
      modify( get_account( change_req->account_to_recover ), [&]( account_object& a )
      {
         a.recovery_account = change_req->recovery_account;
      });

      remove( *change_req );
      change_req = change_req_idx.begin();
   }
}

void database::clear_network_votes( const account_object& a )
{
   const auto& producer_vote_idx = get_index< producer_vote_index >().indices().get< by_account_producer >();
   auto producer_vote_itr = producer_vote_idx.lower_bound( a.name );
   while( producer_vote_itr != producer_vote_idx.end() && producer_vote_itr->account == a.name )
   {
      const producer_vote_object& vote = *producer_vote_itr;
      ++producer_vote_itr;
      remove(vote);
   }

   const auto& officer_vote_idx = get_index< network_officer_vote_index >().indices().get< by_account_officer >();
   auto officer_vote_itr = officer_vote_idx.lower_bound( a.name );
   while( officer_vote_itr != officer_vote_idx.end() && officer_vote_itr->account == a.name )
   {
      const network_officer_vote_object& vote = *officer_vote_itr;
      ++officer_vote_itr;
      remove(vote);
   }

   const auto& executive_vote_idx = get_index< executive_board_vote_index >().indices().get< by_account_executive >();
   auto executive_vote_itr = executive_vote_idx.lower_bound( a.name );
   while( executive_vote_itr != executive_vote_idx.end() && executive_vote_itr->account == a.name )
   {
      const executive_board_vote_object& vote = *executive_vote_itr;
      ++executive_vote_itr;
      remove(vote);
   }

   const auto& enterprise_approval_idx = get_index< enterprise_approval_index >().indices().get< by_account_enterprise >();
   auto enterprise_approval_itr = enterprise_approval_idx.lower_bound( a.name );
   while( enterprise_approval_itr != enterprise_approval_idx.end() && enterprise_approval_itr->account == a.name )
   {
      const enterprise_approval_object& vote = *enterprise_approval_itr;
      ++enterprise_approval_itr;
      remove(vote);
   }

   const auto& community_moderator_vote_idx = get_index< community_moderator_vote_index >().indices().get< by_account >();
   auto community_moderator_vote_itr = community_moderator_vote_idx.lower_bound( a.name );
   while( community_moderator_vote_itr != community_moderator_vote_idx.end() && community_moderator_vote_itr->account == a.name )
   {
      const community_moderator_vote_object& vote = *community_moderator_vote_itr;
      ++community_moderator_vote_itr;
      remove(vote);
   }

   const auto& account_officer_vote_idx = get_index< account_officer_vote_index >().indices().get< by_account_business_officer >();
   auto account_officer_vote_itr = account_officer_vote_idx.lower_bound( a.name );
   while( account_officer_vote_itr != account_officer_vote_idx.end() && account_officer_vote_itr->account == a.name )
   {
      const account_officer_vote_object& vote = *account_officer_vote_itr;
      ++account_officer_vote_itr;
      remove(vote);
   }

   const auto& account_executive_vote_idx = get_index< account_executive_vote_index >().indices().get< by_account_business_executive >();
   auto account_executive_vote_itr = account_executive_vote_idx.lower_bound( a.name );
   while( account_executive_vote_itr != account_executive_vote_idx.end() && account_executive_vote_itr->account == a.name )
   {
      const account_executive_vote_object& vote = *account_executive_vote_itr;
      ++account_executive_vote_itr;
      remove(vote);
   }
}

void database::process_decline_voting_rights()
{
   time_point now = head_block_time();
   const auto& req_idx = get_index< decline_voting_rights_request_index >().indices().get< by_effective_date >();
   auto req_itr = req_idx.begin();

   while( req_itr != req_idx.end() && req_itr->effective_date <= now )
   {
      const account_object& account = get_account( req_itr->account );

      clear_network_votes( account );

      for( auto proxy : account.proxied )   // Remove all accounts proxied to the acocunt.
      {
         modify( get_account( proxy ), [&]( account_object& a )
         {
            a.proxy = PROXY_TO_SELF_ACCOUNT;
         });
      }

      modify( get_account( req_itr->account ), [&]( account_object& a )
      {
         a.can_vote = false;
         a.proxy = PROXY_TO_SELF_ACCOUNT;
         a.proxied.clear();
      });

      remove( *req_itr );
   }
}

} } //node::chain