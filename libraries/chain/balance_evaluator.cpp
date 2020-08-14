
#include <node/chain/node_evaluator.hpp>
#include <node/chain/database.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <cmath>

#include <node/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
//#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace node { namespace chain {


//============================//
// === Balance Evaluators === //
//============================//


void claim_reward_balance_evaluator::do_apply( const claim_reward_balance_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_officer( o.signatory ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object& account = _db.get_account( o.account );
   asset reward = _db.get_reward_balance( o.account, o.reward.symbol );
   
   FC_ASSERT( o.reward.amount <= reward.amount,
      "Account: ${a} Cannot claim requested reward - Claimed: ${c} Available: ${r}",
      ("a", o.account)("c", o.reward)("r", reward) );

   asset shared_reward = asset( 0, o.reward.symbol );

   if( account.revenue_share )     // Distributes revenue from rewards to listed equity and credit assets for business accounts.
   {
      const account_business_object& bus_acc = _db.get_account_business( signed_for );

      for( auto eq : bus_acc.equity_revenue_shares )
      {
         const asset_equity_data_object& equity = _db.get_equity_data( eq.first );
         asset share = ( o.reward * eq.second ) / PERCENT_100;
         shared_reward += share;
         _db.modify( equity, [&]( asset_equity_data_object& aedo )
         {
            aedo.adjust_pool( share );
         }); 
      }
      for( auto cr : bus_acc.credit_revenue_shares )
      {
         const asset_credit_data_object& credit = _db.get_credit_data( cr.first );
         asset share = ( o.reward * cr.second ) / PERCENT_100;
         shared_reward += share;
         _db.modify( credit, [&]( asset_credit_data_object& acdo )
         {
            acdo.adjust_pool( share );
         }); 
      }
   }

   asset claimed_reward = o.reward - shared_reward;
   asset staked_reward = ( claimed_reward * REWARD_STAKED_PERCENT ) / PERCENT_100;
   asset liquid_reward = claimed_reward - staked_reward;

   _db.adjust_reward_balance( o.account, -o.reward );
   _db.adjust_liquid_balance( o.account, liquid_reward );
   _db.adjust_staked_balance( o.account, staked_reward );

   _db.modify( account, [&]( account_object& a )
   {
      a.total_rewards += claimed_reward.amount;
   }); 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void stake_asset_evaluator::do_apply( const stake_asset_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& from_account = _db.get_account( o.from );
   const account_object& to_account = o.to.size() ? _db.get_account( o.to ) : from_account;
   const asset_object& asset_object = _db.get_asset( o.amount.symbol );
   const account_balance_object* account_balance_ptr = _db.find_account_balance( to_account.name, o.amount.symbol );
   time_point now = _db.head_block_time();

   if( account_balance_ptr == nullptr )     // Make balance if none exists
   {
      _db.create< account_balance_object >( [&]( account_balance_object& abo )
      {
         abo.owner = to_account.name;
         abo.symbol = o.amount.symbol;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
         abo.last_interest_time = now;
      });
   }

   const account_balance_object& account_balance = _db.get_account_balance( to_account.name, o.amount.symbol );

   asset liquid = _db.get_liquid_balance( o.from, o.amount.symbol );

   FC_ASSERT( liquid >= o.amount,
      "Account: ${a} does not have sufficient liquid balance: ${l} for stake amount: ${s}.",
      ("a",o.from)("l",liquid.to_string())("s",o.amount.to_string()) );

   if( o.amount.amount == 0 )
   {
      FC_ASSERT( account_balance.stake_rate != 0,
         "This operation would not change the stake rate." );

      _db.modify( account_balance, [&]( account_balance_object& abo )
      {
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
   }
   else
   {
      if( asset_object.stake_intervals == 0 )
      {
         _db.adjust_liquid_balance( from_account, -o.amount );
         _db.adjust_staked_balance( to_account, o.amount );
      }
      else if( asset_object.stake_intervals >= 1 )
      {
         FC_ASSERT( to_account.name == from_account.name,
            "Cannot directly stake transfer an asset with a staking interval to another account. Please use a liquid transfer." );

         share_type new_stake_rate = ( o.amount.amount / asset_object.stake_intervals );

         FC_ASSERT( account_balance.stake_rate != new_stake_rate,
            "This operation would not change the stake rate." );

         _db.modify( account_balance, [&]( account_balance_object& abo )
         {
            abo.stake_rate = new_stake_rate;
            abo.next_stake_time = now + STAKE_WITHDRAW_INTERVAL;
            abo.to_stake = o.amount.amount;
            abo.total_staked = 0;
            abo.unstake_rate = 0;
            abo.next_unstake_time = time_point::maximum();
            abo.to_unstake = 0;
            abo.total_unstaked = 0;
         });

         ilog( "Stake Asset: ${b}",("b",account_balance));
      } 
   }
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void unstake_asset_evaluator::do_apply( const unstake_asset_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& from_account = _db.get_account( o.from );
   const account_object& to_account = o.to.size() ? _db.get_account( o.to ) : from_account;
   const asset_object& asset_object = _db.get_asset( o.amount.symbol );
   const account_balance_object* account_balance_ptr = _db.find_account_balance( to_account.name, o.amount.symbol );
   time_point now = _db.head_block_time();

   if( account_balance_ptr == nullptr )     // Make balance if none exists
   {
      _db.create< account_balance_object >( [&]( account_balance_object& abo )
      {
         abo.owner = to_account.name;
         abo.symbol = o.amount.symbol;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
         abo.last_interest_time = now;
      });
   }

   const account_balance_object& account_balance = _db.get_account_balance( to_account.name, o.amount.symbol );
   asset stake = _db.get_staked_balance( o.from, o.amount.symbol );
   const median_chain_property_object& median_props = _db.get_median_chain_properties();

   FC_ASSERT( stake >= o.amount, 
      "Account: ${a} has Insufficient Staked balance: ${s} for unstake of amount: ${w}.",
      ("a",o.from)("s",stake.to_string())("w",o.amount.to_string()) );

   if( !from_account.mined && 
      o.amount.symbol == SYMBOL_COIN )
   {
      asset min_stake = median_props.account_creation_fee * 10;
      FC_ASSERT( stake >= min_stake,
         "Account ${a} requires 10x account creation fee: ${m} before it can be unstaked. Staked balance: ${s} Unstake amount: ${w}.", 
         ("a",o.from)("s",stake.to_string())("w",o.amount.to_string())("m",min_stake.to_string()));
   }

   if( o.amount.amount == 0 )
   {
      FC_ASSERT( account_balance.unstake_rate != 0,
         "This operation would not change the unstake rate." );

      _db.modify( account_balance, [&]( account_balance_object& abo ) 
      {
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
   }
   else
   {
      if( asset_object.unstake_intervals == 0 )
      {
         _db.adjust_staked_balance( from_account, -o.amount );
         _db.adjust_liquid_balance( to_account, o.amount );
      }
      else if( asset_object.unstake_intervals >= 1 )
      {
         share_type new_unstake_rate = ( o.amount.amount / asset_object.unstake_intervals );
         FC_ASSERT( account_balance.unstake_rate != new_unstake_rate, 
            "This operation would not change the unstake rate." );

         _db.modify( account_balance, [&]( account_balance_object& abo )
         {
            abo.stake_rate = 0;
            abo.next_stake_time = time_point::maximum();
            abo.to_stake = 0;
            abo.total_staked = 0;
            abo.unstake_rate = new_unstake_rate;
            abo.next_unstake_time = now + STAKE_WITHDRAW_INTERVAL;
            abo.to_unstake = o.amount.amount;
            abo.total_unstaked = 0;
         });

         ilog( "Unstake Asset: ${b}",("b",account_balance));
      }
   }
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void unstake_asset_route_evaluator::do_apply( const unstake_asset_route_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& from = _db.get_account( o.from );
   const account_object& to = _db.get_account( o.to );
   const auto& wd_idx = _db.get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
   auto itr = wd_idx.find( boost::make_tuple( from.name, to.name ) );

   if( itr == wd_idx.end() )
   {
      FC_ASSERT( from.withdraw_routes < MAX_WITHDRAW_ROUTES,
         "Account already has the maximum number of routes." );

      _db.create< unstake_asset_route_object >( [&]( unstake_asset_route_object& wvdo )
      {
         wvdo.from = from.name;
         wvdo.to = to.name;
         wvdo.percent = o.percent;
         wvdo.auto_stake = o.auto_stake;
      });

      _db.modify( from, [&]( account_object& a )
      {
         a.withdraw_routes++;
      });
   }
   else if( o.percent == 0 )
   {
      ilog( "Removed: ${v}",("v",*itr));
      _db.remove( *itr );

      _db.modify( from, [&]( account_object& a )
      {
         a.withdraw_routes--;
      });
   }
   else
   {
      _db.modify( *itr, [&]( unstake_asset_route_object& wvdo )
      {
         wvdo.from = from.name;
         wvdo.to = to.name;
         wvdo.percent = o.percent;
         wvdo.auto_stake = o.auto_stake;
      });
   }

   itr = wd_idx.upper_bound( boost::make_tuple( from.name, account_name_type() ) );
   uint16_t total_percent = 0;

   while( itr->from == from.name && itr != wd_idx.end() )
   {
      total_percent += itr->percent;
      ++itr;
   }

   FC_ASSERT( total_percent <= PERCENT_100,
      "More than 100% of unstake operations allocated to destinations." );
} FC_CAPTURE_AND_RETHROW() }


void transfer_to_savings_evaluator::do_apply( const transfer_to_savings_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& to = _db.get_account( o.to );
   const account_object& from = _db.get_account( o.from );
   const asset& liquid = _db.get_liquid_balance( from, o.amount.symbol );

   FC_ASSERT( liquid >= o.amount,
      "Account: ${a} has Insufficient Liquid balance: ${s} for savings transfer of amount: ${w}.",
      ("a",o.from)("s",liquid.to_string())("w",o.amount.to_string()) );

   _db.adjust_liquid_balance( from, -o.amount );
   _db.adjust_savings_balance( to, o.amount );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void transfer_from_savings_evaluator::do_apply( const transfer_from_savings_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& to_account = _db.get_account( o.to );
   FC_ASSERT( to_account.active,
      "Account: ${s} must be active to withdraw savings balance to.",
      ("s", o.to) );
   
   const account_object& from_account = _db.get_account( o.from );
   time_point now = _db.head_block_time();

   FC_ASSERT( from_account.savings_withdraw_requests < SAVINGS_WITHDRAW_REQUEST_LIMIT,
      "Account has reached limit for pending withdraw requests." );

   asset savings = _db.get_savings_balance( o.from, o.amount.symbol );
   FC_ASSERT( savings.amount >= o.amount.amount,
      "Account: ${a} has Insufficient Savings balance: ${s} for withdrawal request of: ${w}.",
      ("a",o.from)("s",savings)("w",o.amount) );

   const auto& savings_idx = _db.get_index< savings_withdraw_index >().indices().get< by_request_id >();
   auto savings_itr = savings_idx.find( boost::make_tuple( from_account.name, o.request_id ) );

   if( savings_itr == savings_idx.end() )     // Savings request does not exist
   {
      FC_ASSERT( o.transferred, 
         "No Transfer request with the specified id exists to cancel." );

      _db.adjust_savings_balance( o.from, -o.amount );

      const savings_withdraw_object& withdraw = _db.create< savings_withdraw_object >( [&]( savings_withdraw_object& s )
      {
         s.from = o.from;
         s.to = o.to;
         s.amount = o.amount;
         from_string( s.memo, o.memo );
         from_string( s.request_id, o.request_id );
         s.complete = now + SAVINGS_WITHDRAW_TIME;
      });

      _db.modify( from_account, [&]( account_object& a )
      {
         a.savings_withdraw_requests++;
      });

      ilog( "New Savings Withdrawal: ${s}",
         ("s",withdraw));
   }
   else      // Savings request exists
   {
      const savings_withdraw_object& withdraw = *savings_itr;

      if( o.transferred )    // Editing transfer request
      {
         _db.modify( withdraw, [&]( savings_withdraw_object& s )
         {
            s.to = o.to;
            s.amount = o.amount;
            from_string( s.memo, o.memo );
         });
         ilog( "Edit Savings Withdrawal: ${s}",
            ("s",withdraw));
      }
      else     // Cancelling request
      {
         _db.adjust_savings_balance( withdraw.from, withdraw.amount );
         ilog( "Removed: ${v}",("v",withdraw));
         _db.modify( from_account, [&]( account_object& a )
         {
            a.savings_withdraw_requests--;
         });

         _db.remove( withdraw );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void delegate_asset_evaluator::do_apply( const delegate_asset_operation& o )
{ try {
   const account_name_type& signed_for = o.delegator;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& delegator_account = _db.get_account( o.delegator );
   const account_object& delegatee_account = _db.get_account( o.delegatee );
   FC_ASSERT( delegatee_account.active, 
      "Account: ${s} must be active to receive delegated balance.",("s", o.delegatee) );
   const account_balance_object& delegator_balance = _db.get_account_balance( o.delegator, o.amount.symbol );
   time_point now = _db.head_block_time();
   const asset_delegation_object* delegation_ptr = _db.find_asset_delegation( o.delegator, o.delegatee, o.amount.symbol );

   asset available_stake = delegator_balance.get_staked_balance() - delegator_balance.get_delegated_balance() - asset( delegator_balance.to_unstake - delegator_balance.total_unstaked, o.amount.symbol );

   if( delegation_ptr == nullptr )          // If delegation doesn't exist, create it.
   {
      FC_ASSERT( available_stake >= o.amount,
         "Account does not have enough stake to delegate." );

      _db.create< asset_delegation_object >( [&]( asset_delegation_object& ado ) 
      {
         ado.delegator = o.delegator;
         ado.delegatee = o.delegatee;
         ado.amount = o.amount;
      }); 

      _db.adjust_delegated_balance( delegator_account, o.amount );       // increase delegated balance of delegator account
      _db.adjust_receiving_balance( delegatee_account, o.amount );       // increase receiving balance of delegatee account
   }
   else if( o.amount >= delegation_ptr->amount )       // Else if the delegation is increasing
   {
      asset delta = o.amount - delegation_ptr->amount;

      FC_ASSERT( available_stake >= o.amount - delegation_ptr->amount, 
         "Account does not have enough stake to delegate." );

      _db.adjust_delegated_balance( delegator_account, delta );     // increase delegated balance of delegator account
      _db.adjust_receiving_balance( delegatee_account, delta );     // increase receiving balance of delegatee account

      _db.modify( *delegation_ptr, [&]( asset_delegation_object& ado )
      {
         ado.amount = o.amount;
      });
   }
   else        // Else the delegation is decreasing ( delegation->asset > o.amount )
   {
      const asset_delegation_object& delegation = *delegation_ptr;
      asset delta = delegation.amount - o.amount;

      FC_ASSERT( delta.amount != 0,
         "Delegation amount must change." );

      // Asset delegation expiration object causes delegated and recieving balance to decrement after a delay.

      _db.create< asset_delegation_expiration_object >( [&]( asset_delegation_expiration_object& obj )
      {
         obj.delegator = o.delegator;
         obj.delegatee = o.delegatee;
         obj.amount = delta;
         obj.expiration = now + fc::days(1);
      });

      _db.modify( delegation, [&]( asset_delegation_object& ado )
      {
         ado.amount = o.amount;
      });
      
      if( o.amount.amount == 0 )
      {
         ilog( "Removed: ${v}",("v",delegation));
         _db.remove( delegation );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain