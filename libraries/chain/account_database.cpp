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
 * TODO:
 * Expires memberships, and renews recurring memberships when the 
 * membership expiration time is reached.
 */
void database::process_membership_updates()
{ try {


} FC_CAPTURE_AND_RETHROW() }


/**
 * Pays an account its activity reward shares from the reward pool.
 */
asset database::claim_activity_reward( const account_object& account, const witness_object& witness)
{ try {
   const account_balance_object& abo = get_account_balance(account.name, SYMBOL_EQUITY);
   auto decay_rate = RECENT_REWARD_DECAY_RATE;
   const reward_fund_object& reward_fund = get_reward_fund();
   const asset_dynamic_data_object& core_asset_dynamic_data = get_core_dynamic_data();
   share_type activity_shares = BLOCKCHAIN_PRECISION;
   if( abo.staked_balance >= 10 * BLOCKCHAIN_PRECISION) 
   {
      activity_shares *= 2;
   }

   if( account.membership == STANDARD_MEMBERSHIP) 
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_STANDARD_PERCENT) / PERCENT_100;
   }
   else if( account.membership == MID_MEMBERSHIP) 
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_MID_PERCENT) / PERCENT_100;
   }
   else if( account.membership == TOP_MEMBERSHIP) 
   {
      activity_shares = (activity_shares * ACTIVITY_BOOST_TOP_PERCENT) / PERCENT_100;
   }

   // Decay recent claims of activity reward fund and add new shares of this claim.
   modify( reward_fund, [&]( reward_fund_object& rfo )   
   {
      rfo.recent_activity_claims -= ( rfo.recent_activity_claims * ( head_block_time() - rfo.last_update ).to_seconds() ) / decay_rate.to_seconds();
      rfo.last_update = head_block_time();
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
      a.recent_activity_claims -= ( a.recent_activity_claims * ( head_block_time() - a.last_activity_reward ).to_seconds() ) / decay_rate.to_seconds();
      a.last_activity_reward = head_block_time();                // Update activity reward time.
      a.recent_activity_claims += BLOCKCHAIN_PRECISION;          // Increments rolling activity average by one claim.
   });

   adjust_reward_balance( account, activity_reward );            // Add activity reward to reward balance of claiming account. 

   uint128_t voting_power = get_voting_power( account.name ).value;

   modify( witness, [&]( witness_object& w ) 
   {
      w.accumulated_activity_stake += voting_power;
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

void database::account_recovery_processing()
{
   // Clear expired recovery requests
   const auto& rec_req_idx = get_index< account_recovery_request_index >().indices().get< by_expiration >();
   auto rec_req = rec_req_idx.begin();

   while( rec_req != rec_req_idx.end() && rec_req->expires <= head_block_time() )
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
   const auto& request_idx = get_index< decline_voting_rights_request_index >().indices().get< by_effective_date >();
   auto itr = request_idx.begin();

   while( itr != request_idx.end() && itr->effective_date <= head_block_time() )
   {
      const account_object& account = get_account(itr->account);
      share_type voting_power = get_voting_power(account);

      clear_witness_votes( account );

      modify( get_account(itr->account), [&]( account_object& a )
      {
         a.can_vote = false;
         a.proxy = PROXY_TO_SELF_ACCOUNT;
      });

      remove( *itr );
      itr = request_idx.begin();
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
      acc.witnesses_voted_for = 0;
   });
}

} } //node::chain