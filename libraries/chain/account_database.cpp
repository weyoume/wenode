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


const account_object& database::get_account( const account_name_type& name )const
{ try {
	return get< account_object, by_name >( name );
} FC_CAPTURE_AND_RETHROW( (name) ) }

const account_object* database::find_account( const account_name_type& name )const
{
   return find< account_object, by_name >( name );
}

const account_authority_object& database::get_account_authority( const account_name_type& account )const
{ try {
	return get< account_authority_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_authority_object* database::find_account_authority( const account_name_type& account )const
{
   return find< account_authority_object, by_account >( account );
}

const account_permission_object& database::get_account_permissions( const account_name_type& account )const
{ try {
	return get< account_permission_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_permission_object* database::find_account_permissions( const account_name_type& account )const
{
   return find< account_permission_object, by_account >( account );
}

const account_verification_object& database::get_account_verification( const account_name_type& verifier_account, const account_name_type& verified_account )const
{ try {
	return get< account_verification_object, by_verifier_verified >( boost::make_tuple( verifier_account, verified_account ) );
} FC_CAPTURE_AND_RETHROW( (verifier_account)(verified_account) ) }

const account_verification_object* database::find_account_verification( const account_name_type& verifier_account, const account_name_type& verified_account )const
{
   return find< account_verification_object, by_verifier_verified >( boost::make_tuple( verifier_account, verified_account ) );
}

const account_following_object& database::get_account_following( const account_name_type& account )const
{ try {
	return get< account_following_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const account_following_object* database::find_account_following( const account_name_type& account )const
{
   return find< account_following_object, by_account >( account );
}

const account_tag_following_object& database::get_account_tag_following( const tag_name_type& tag )const
{ try {
	return get< account_tag_following_object, by_tag >( tag );
} FC_CAPTURE_AND_RETHROW( (tag) ) }

const account_tag_following_object* database::find_account_tag_following( const tag_name_type& tag )const
{
   return find< account_tag_following_object, by_tag >( tag );
}


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
   
   while( account_itr != account_idx.end() && 
      account_itr->membership_expiration <= now )
   {
      const account_object& member = *account_itr;

      if( member.recurring_membership > 0 )
      {
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

         if( liquid >= total_fees && 
            total_fees.amount > 0 )
         {
            if( member.membership_interface != NULL_ACCOUNT )
            {
               const account_object& int_account = get_account( member.membership_interface );
               pay_membership_fees( member, total_fees, int_account );
            }
            else
            {
               pay_membership_fees( member, total_fees );
            }
            
            modify( member, [&]( account_object& a )
            {
               a.membership_expiration = ( now + fc::days( 30 * a.recurring_membership ) );
            });
         }
         else
         {
            modify( member, [&]( account_object& a )
            {
               a.membership = membership_tier_type::NONE;
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
            a.membership = membership_tier_type::NONE;
            a.membership_expiration = fc::time_point::maximum();
            a.recurring_membership = 0;
            a.membership_interface = NULL_ACCOUNT;
         });
      }

      ilog( "Updated Account Membership: ${m}",("m",member));

      ++account_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays protocol membership fees.
 * 
 * Splits to network contributors -
 * member: The account that is paying to upgrade to a membership level.
 * payment: The asset being received as payment.
 * interface: The owner account of the interface that sold the membership.
 */
asset database::pay_membership_fees( const account_object& member, const asset& mem_payment, const account_object& interface )
{ try {
   FC_ASSERT( mem_payment > asset( 0, SYMBOL_USD ), 
      "Payment must be denominated in USD asset and greater than 0." );

   price usd_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).hour_median_price;
   asset membership_fee = mem_payment * usd_price;       // Membership fee denominated in Core asset
   asset network_fees = ( membership_fee * NETWORK_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( membership_fee * INTERFACE_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   
   asset network_paid = pay_network_fees( member, network_fees );
   asset interface_paid = pay_fee_share( interface, interface_fees, true );
   asset total_fees = network_paid + interface_paid;
   adjust_liquid_balance( member.name, -total_fees );

   return total_fees;

} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays protocol membership fees, and splits to network contributors
 * member: The account that is paying to upgrade to a membership level
 * payment: The asset being received as payment
 */
asset database::pay_membership_fees( const account_object& member, const asset& mem_payment )
{ try {
   FC_ASSERT( mem_payment > asset( 0, SYMBOL_USD ), 
      "Payment must be denominated in USD asset and greater than 0." );

   price usd_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).hour_median_price;
   asset membership_fee = mem_payment * usd_price;       // Membership fee denominated in Core asset
   asset network_fees = ( membership_fee * NETWORK_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( membership_fee * INTERFACE_MEMBERSHIP_FEE_PERCENT ) / PERCENT_100;
   asset network_paid = pay_network_fees( member, network_fees + interface_fees );
   asset total_fees = network_paid;
   adjust_liquid_balance( member.name, -total_fees );

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

   ilog( "Update Account Reputations");
   
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
   const asset_reward_fund_object& reward_fund = get_reward_fund( currency_symbol );
   const account_balance_object& abo = get_account_balance( account.name, currency.equity_asset );
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = props.time;
   auto decay_rate = RECENT_REWARD_DECAY_RATE;
   price equity_price = get_liquidity_pool( currency_symbol, currency.equity_asset ).hour_median_price;
   uint128_t activity_shares = BLOCKCHAIN_PRECISION.value;

   FC_ASSERT( abo.staked_balance >= BLOCKCHAIN_PRECISION,
      "Account: ${a} must have at least one staked equity asset to claim activity reward. Stake: ${s}",
      ("a", account.name)("s",abo.staked_balance));

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

   modify( reward_fund, [&]( asset_reward_fund_object& rfo )   
   {
      rfo.recent_activity_claims -= ( rfo.recent_activity_claims * ( now - rfo.last_updated ).count() ) / decay_rate.count();
      rfo.last_updated = now;
      rfo.recent_activity_claims += activity_shares;
   });

   uint128_t reward_shares_amount = ( uint128_t( reward_fund.activity_reward_balance.amount.value ) * activity_shares ) / uint128_t( reward_fund.recent_activity_claims.to_uint64() );

   asset activity_reward = asset( share_type( reward_shares_amount.to_uint64() ), currency_symbol );

   modify( reward_fund, [&]( asset_reward_fund_object& rfo )
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
      p.decay_weights( now, median_props );
      p.accumulated_activity_stake += voting_power;
   });

   return activity_reward;

} FC_CAPTURE_AND_RETHROW( (account.name) ) }


void database::update_owner_authority( const account_object& account, const authority& owner_authority )
{
   time_point now = head_block_time();
   const account_authority_object& auth = get< account_authority_object, by_account >( account.name );

   create< account_authority_history_object >( [&]( account_authority_history_object& h )
   {
      h.account = account.name;
      h.previous_owner_authority = auth.owner_auth;
      h.last_valid_time = now;
   });
   
   modify( auth, [&]( account_authority_object& a )
   {
      a.owner_auth = owner_authority;
      a.last_owner_update = now;
   });

   ilog( "Updated Owner Authority Account: ${a} New Authority: \n ${o} \n",
      ("a", account.name)("o",auth));
}


void database::account_recovery_processing()
{
   time_point now = head_block_time();

   // Clear expired recovery requests
   const auto& rec_req_idx = get_index< account_recovery_request_index >().indices().get< by_expiration >();
   auto rec_req = rec_req_idx.begin();

   while( rec_req != rec_req_idx.end() && 
      rec_req->expiration <= now )
   {
      ilog( "Removed: ${v}",("v",*rec_req));
      remove( *rec_req );
      rec_req = rec_req_idx.begin();
   }

   // Clear invalid historical authorities
   const auto& hist_idx = get_index< account_authority_history_index >().indices();
   auto hist = hist_idx.begin();

   while( hist != hist_idx.end() && 
      time_point( hist->last_valid_time + OWNER_AUTH_RECOVERY_PERIOD ) <= now )
   {
      ilog( "Removed: ${v}",("v",*hist));
      remove( *hist );
      hist = hist_idx.begin();
   }

   // Apply effective recovery_account changes
   const auto& change_req_idx = get_index< account_recovery_update_request_index >().indices().get< by_effective_date >();
   auto change_req = change_req_idx.begin();

   while( change_req != change_req_idx.end() && 
      change_req->effective_on <= now )
   {
      const account_object& account = get_account( change_req->account_to_recover );

      modify( account, [&]( account_object& a )
      {
         a.recovery_account = change_req->recovery_account;
      });

      ilog( "Account: ${a} updated recovery account to: ${v}",
         ("a",account.name)("v",account.recovery_account));

      ilog( "Removed: ${v}",("v",*change_req));
      remove( *change_req );
      change_req = change_req_idx.begin();
   }
}

void database::process_account_decline_voting()
{
   time_point now = head_block_time();
   const auto& req_idx = get_index< account_decline_voting_request_index >().indices().get< by_effective_date >();
   auto req_itr = req_idx.begin();

   while( req_itr != req_idx.end() && 
      req_itr->effective_date <= now )
   {
      const account_decline_voting_request_object& request = *req_itr;
      const account_object& account = get_account( request.account );
      ++req_itr;
   
      clear_account_votes( account.name );

      for( auto proxy : account.proxied )   // Remove all accounts proxied to the account.
      {
         modify( get_account( proxy ), [&]( account_object& a )
         {
            a.proxy = PROXY_TO_SELF_ACCOUNT;
         });
      }

      modify( account, [&]( account_object& a )
      {
         a.can_vote = false;
         a.proxy = PROXY_TO_SELF_ACCOUNT;
         a.proxied.clear();
      });

      ilog( "Account: ${a} Processed Decline Voting Rights Request: \n ${r} \n",
         ("a",account.name)("r",request));

      ilog( "Removed: ${v}",("v",request));
      remove( request );
   }
}



void database::clear_account_votes( const account_name_type& a )
{
   const auto& producer_vote_idx = get_index< producer_vote_index >().indices().get< by_account_producer >();
   auto producer_vote_itr = producer_vote_idx.lower_bound( a );
   while( producer_vote_itr != producer_vote_idx.end() && 
      producer_vote_itr->account == a )
   {
      const producer_vote_object& vote = *producer_vote_itr;
      ++producer_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }

   const auto& officer_vote_idx = get_index< network_officer_vote_index >().indices().get< by_account_officer >();
   auto officer_vote_itr = officer_vote_idx.lower_bound( a );
   while( officer_vote_itr != officer_vote_idx.end() && 
      officer_vote_itr->account == a )
   {
      const network_officer_vote_object& vote = *officer_vote_itr;
      ++officer_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }

   const auto& enterprise_vote_idx = get_index< enterprise_vote_index >().indices().get< by_account_enterprise >();
   auto enterprise_vote_itr = enterprise_vote_idx.lower_bound( a );
   while( enterprise_vote_itr != enterprise_vote_idx.end() && 
      enterprise_vote_itr->account == a )
   {
      const enterprise_vote_object& vote = *enterprise_vote_itr;
      ++enterprise_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }

   const auto& community_member_vote_idx = get_index< community_member_vote_index >().indices().get< by_account_community_rank >();
   auto community_member_vote_itr = community_member_vote_idx.lower_bound( a );
   while( community_member_vote_itr != community_member_vote_idx.end() && 
      community_member_vote_itr->account == a )
   {
      const community_member_vote_object& vote = *community_member_vote_itr;
      ++community_member_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }

   const auto& business_director_vote_idx = get_index< business_director_vote_index >().indices().get< by_account_business_director >();
   auto business_director_vote_itr = business_director_vote_idx.lower_bound( a );
   while( business_director_vote_itr != business_director_vote_idx.end() && 
      business_director_vote_itr->account == a )
   {
      const business_director_vote_object& vote = *business_director_vote_itr;
      ++business_director_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }

   const auto& business_executive_vote_idx = get_index< business_executive_vote_index >().indices().get< by_director >();
   auto business_executive_vote_itr = business_executive_vote_idx.lower_bound( a );
   while( business_executive_vote_itr != business_executive_vote_idx.end() && 
      business_executive_vote_itr->director == a )
   {
      const business_executive_vote_object& vote = *business_executive_vote_itr;
      ++business_executive_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }

   const auto& governance_director_vote_idx = get_index< governance_director_vote_index >().indices().get< by_account_governance >();
   auto governance_director_vote_itr = governance_director_vote_idx.lower_bound( a );
   while( governance_director_vote_itr != governance_director_vote_idx.end() && 
      governance_director_vote_itr->account == a )
   {
      const governance_director_vote_object& vote = *governance_director_vote_itr;
      ++governance_director_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }

   const auto& governance_executive_vote_idx = get_index< governance_executive_vote_index >().indices().get< by_director >();
   auto governance_executive_vote_itr = governance_executive_vote_idx.lower_bound( a );
   while( governance_executive_vote_itr != governance_executive_vote_idx.end() && 
      governance_executive_vote_itr->director == a )
   {
      const governance_executive_vote_object& vote = *governance_executive_vote_itr;
      ++governance_executive_vote_itr;
      ilog( "Removed: ${v}",("v",vote));
      remove(vote);
   }
}

} } //node::chain