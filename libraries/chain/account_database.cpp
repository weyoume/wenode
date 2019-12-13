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
#include <node/chain/witness_schedule.hpp>
#include <node/witness/witness_objects.hpp>

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
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& account_idx = get_index< account_index >().indices().get< by_membership_expiration >();
   auto account_itr = account_idx.begin();
   
   while( account_itr != account_idx.end() && account_itr->membership_expiration >= now )
   {
      const account_object& member = *account_itr;

      if( member.recurring_membership > 0 )
      {
         const account_object& int_account = get_account( member.membership_interface );
         const interface_object& interface = get_interface( member.membership_interface );

         asset liquid = get_liquid_balance( member.name, SYMBOL_COIN );
         asset monthly_fee = asset(0, SYMBOL_USD );

         switch( member.membership )
         {
            case NONE:
            {
               break; 
            }
            case STANDARD_MEMBERSHIP:
            {
               monthly_fee = props.median_props.membership_base_price;
               break; 
            }
            case MID_MEMBERSHIP:
            {
               monthly_fee = props.median_props.membership_mid_price;
               break; 
            }
            case TOP_MEMBERSHIP:
            {
               monthly_fee = props.median_props.membership_top_price;
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
asset database::pay_membership_fees( const account_object& member, const asset& payment, const account_object& interface )
{ try {
   FC_ASSERT( payment.symbol == SYMBOL_USD, 
      "Payment asset must be denominated in USD asset.");

   const reward_fund_object& reward_fund = get_reward_fund();
   asset membership_fee = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).hour_median_price * payment;

   asset network_fees = ( membership_fee * NETWORK_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( membership_fee * INTERFACE_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset partners_fees = ( membership_fee * PARTNERS_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;

   asset network_paid = pay_network_fees( member, network_fees );
   asset interface_paid = pay_fee_share( interface, network_fees );

   asset total_fees = network_paid + interface_paid + partners_fees;
   adjust_liquid_balance( member.name, -total_fees );
   
   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_premium_partners_fund_balance( partners_fees );    // Adds funds to premium partners fund for distribution to premium creators
   });

} FC_CAPTURE_AND_RETHROW() }



/**
 * Updates Account reputation values depending on the account's total rewards,
 * and the total rewards of its followers, divided by the number of accounts each account follows. 
 */
void database::update_account_reputations()
{ try {
   if( (head_block_num() % REP_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;
   
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;

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

   std::sort( reward_vector.begin(), reward_vector.end(), [&](share_type a, share_type b)
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
asset database::claim_activity_reward( const account_object& account, const witness_object& witness)
{ try {
   const account_balance_object& abo = get_account_balance(account.name, SYMBOL_EQUITY);
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const witness_schedule_object& wso = get_witness_schedule();
   time_point now = props.time;
   auto decay_rate = RECENT_REWARD_DECAY_RATE;
   const reward_fund_object& reward_fund = get_reward_fund();
   price equity_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_EQUITY ).hour_median_price;
   share_type activity_shares = BLOCKCHAIN_PRECISION;
   if( abo.staked_balance >= 10 * BLOCKCHAIN_PRECISION ) 
   {
      activity_shares *= 2;
   }

   if( account.membership == STANDARD_MEMBERSHIP )
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_STANDARD_PERCENT) / PERCENT_100;
   }
   else if( account.membership == MID_MEMBERSHIP )
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_MID_PERCENT) / PERCENT_100;
   }
   else if( account.membership == TOP_MEMBERSHIP )
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_TOP_PERCENT) / PERCENT_100;
   }

   // Decay recent claims of activity reward fund and add new shares of this claim.
   modify( reward_fund, [&]( reward_fund_object& rfo )   
   {
      rfo.recent_activity_claims -= ( rfo.recent_activity_claims * ( now - rfo.last_update ).to_seconds() ) / decay_rate.to_seconds();
      rfo.last_update = now;
      rfo.recent_activity_claims += activity_shares.value;
   }); 

   asset activity_reward = ( reward_fund.activity_reward_balance * activity_shares ) / reward_fund.recent_activity_claims;

   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_activity_reward_balance( -activity_reward );    // Deduct activity reward from reward fund.
   });

   adjust_pending_supply( -activity_reward );                    // Deduct activity reward from pending supply.
   
   // Update recent activity claims on account
   modify( account, [&]( account_object& a ) 
   {
      a.recent_activity_claims -= ( a.recent_activity_claims * ( now - a.last_activity_reward ).to_seconds() ) / decay_rate.to_seconds();
      a.last_activity_reward = now;                // Update activity reward time.
      a.recent_activity_claims += BLOCKCHAIN_PRECISION;          // Increments rolling activity average by one claim.
   });

   adjust_reward_balance( account, activity_reward );            // Add activity reward to reward balance of claiming account. 

   uint128_t voting_power = get_voting_power( account.name, equity_price ).value + get_proxied_voting_power( account.name, equity_price ).value;

   modify( witness, [&]( witness_object& w ) 
   {
      w.accumulated_activity_stake += voting_power;
      w.decay_weights( now, wso );
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
         hist.previous_owner_authority = get< account_authority_object, by_account >( account.name ).owner;
         hist.last_valid_time = head_block_time();
      });
   }

   modify( get< account_authority_object, by_account >( account.name ), [&]( account_authority_object& auth )
   {
      auth.owner = owner_authority;
      auth.last_owner_update = head_block_time();
   });
}

/**
 * Aligns witness votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_witness_votes( const account_object& account )
{
   const auto& vote_idx = get_index< witness_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const witness_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( witness_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }

   modify( account, [&]( account_object& a )
   {
      a.witness_vote_count = ( new_vote_rank - 1 );
   });
}

/**
 * Aligns witness votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_witness_votes( const account_object& account, const account_name_type& witness, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< witness_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const witness_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( witness_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< witness_vote_object >([&]( witness_vote_object& v )
   {
      v.account = account.name;
      v.witness = witness;
      v.vote_rank = input_vote_rank;
   });

   modify( account, [&]( account_object& a )
   {
      a.witness_vote_count = ( new_vote_rank - 1 );
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

   flat_map< network_officer_types, uint16_t > vote_rank;
   vote_rank[ DEVELOPMENT ] = 1;
   vote_rank[ MARKETING ] = 1;
   vote_rank[ ADVOCACY ] = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const network_officer_vote_object& vote = *vote_itr;
      if( vote.vote_rank != vote_rank[ vote.officer_type ] )
      {
         modify( vote, [&]( witness_vote_object& v )
         {
            v.vote_rank = vote_rank[ vote.officer_type ];   // Updates vote rank to linear order of index retrieval.
         });
      }
      vote_rank[ vote.officer_type ]++;
      ++vote_itr;
   }

   modify( account, [&]( account_object& a )
   {
      a.officer_vote_count = ( vote_rank[ DEVELOPMENT ] + vote_rank[ MARKETING ] + vote_rank[ ADVOCACY ] - 3 );
   });
}

/**
 * Aligns network_officer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_network_officer_votes( const account_object& account, const account_name_type& network_officer, 
   network_officer_types officer_type, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< network_officer_vote_index >().indices().get< by_account_type_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   flat_map< network_officer_types, uint16_t > vote_rank;
   vote_rank[ DEVELOPMENT ] = 1;
   vote_rank[ MARKETING ] = 1;
   vote_rank[ ADVOCACY ] = 1;

   while( vote_itr != vote_idx.end() && vote_itr->account == account.name )
   {
      const network_officer_vote_object& vote = *vote_itr;

      if( vote.vote_rank == input_vote_rank && vote.officer_type == officer_type )
      {
         vote_rank[ vote.officer_type ]++;
      }
      if( vote.vote_rank != vote_rank[ vote.officer_type ] )
      {
         modify( vote, [&]( witness_vote_object& v )
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
      a.officer_vote_count = ( vote_rank[ DEVELOPMENT ] + vote_rank[ MARKETING ] + vote_rank[ ADVOCACY ] - 3 );
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
      a.executive_board_votes = ( new_vote_rank - 1 );
   });
}


/**
 * Aligns board moderator votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_board_moderator_votes( const account_object& account, const board_name_type& board )
{
   const auto& vote_idx = get_index< board_moderator_vote_index >().indices().get< by_account_board_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, board ) );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->board == board )
   {
      const board_moderator_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( board_moderator_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }
}

/**
 * Aligns board moderator votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_board_moderator_votes( const account_object& account, const board_name_type& board, 
   const account_name_type& moderator, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< board_moderator_vote_index >().indices().get< by_account_board_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, board ) );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->board == board )
   {
      const board_moderator_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( board_moderator_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< board_moderator_vote_object >([&]( board_moderator_vote_object& v )
   {
      v.account = account.name;
      v.moderator = moderator;
      v.board = board;
      v.vote_rank = input_vote_rank;
   });
}


/**
 * Aligns enterprise approval votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_enterprise_votes( const account_object& account )
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
void database::update_enterprise_votes( const account_object& account, const account_name_type& creator, 
const shared_string& enterprise_id, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< enterprise_approval_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

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
      v.enterprise_id = enterprise_id;
      v.creator = creator;
      v.vote_rank = input_vote_rank;
   });
}


/**
 * Aligns business account executive votes in a continuous order.
 */
void database::update_account_executive_votes( const account_object& account, const account_name_type& business )
{
   const auto& vote_idx = get_index< account_executive_vote_index >().indices().get< by_account_business_role_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );

   flat_map< executive_types, uint16_t >vote_rank;
   vote_rank[ CHIEF_EXECUTIVE_OFFICER ] = 1;
   vote_rank[ CHIEF_OPERATING_OFFICER ] = 1;
   vote_rank[ CHIEF_FINANCIAL_OFFICER ] = 1;
   vote_rank[ CHIEF_TECHNOLOGY_OFFICER ] = 1;
   vote_rank[ CHIEF_DEVELOPMENT_OFFICER ] = 1;
   vote_rank[ CHIEF_MARKETING_OFFICER ] = 1;
   vote_rank[ CHIEF_ADVOCACY_OFFICER ] = 1;
   vote_rank[ CHIEF_GOVERNANCE_OFFICER ] = 1;
   vote_rank[ CHIEF_SECURITY_OFFICER ] = 1;
   vote_rank[ CHIEF_DESIGN_OFFICER ] = 1;

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
   executive_types role, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< account_executive_vote_index >().indices().get< by_account_business_role_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );

   flat_map< executive_types, uint16_t >vote_rank;
   vote_rank[ CHIEF_EXECUTIVE_OFFICER ] = 1;
   vote_rank[ CHIEF_OPERATING_OFFICER ] = 1;
   vote_rank[ CHIEF_FINANCIAL_OFFICER ] = 1;
   vote_rank[ CHIEF_TECHNOLOGY_OFFICER ] = 1;
   vote_rank[ CHIEF_DEVELOPMENT_OFFICER ] = 1;
   vote_rank[ CHIEF_MARKETING_OFFICER ] = 1;
   vote_rank[ CHIEF_ADVOCACY_OFFICER ] = 1;
   vote_rank[ CHIEF_GOVERNANCE_OFFICER ] = 1;
   vote_rank[ CHIEF_SECURITY_OFFICER ] = 1;
   vote_rank[ CHIEF_DESIGN_OFFICER ] = 1;

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

void database::clear_witness_votes( const account_object& a )
{
   const auto& vidx = get_index< witness_vote_index >().indices().get<by_account_witness>();
   auto itr = vidx.lower_bound( boost::make_tuple( a.id, witness_id_type() ) );
   while( itr != vidx.end() && itr->account == a.id )
   {
      const auto& current = *itr;
      ++itr;
      remove(current);
   }

   modify( a, [&](account_object& acc )
   {
      acc.witness_vote_count = 0;
   });
}

} } //node::chain