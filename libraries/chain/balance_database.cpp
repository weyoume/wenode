#include <node/protocol/types.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/transaction.hpp>
#include <node/chain/database.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>

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
#include <fc/io/json.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>

namespace node { namespace chain {


const account_balance_object& database::get_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const
{ try {
	return get< account_balance_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
} FC_CAPTURE_AND_RETHROW( (owner) (symbol) ) }

const account_balance_object* database::find_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const
{
   return find< account_balance_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
}

const confidential_balance_object& database::get_confidential_balance( const digest_type& hash )const
{ try {
	return get< confidential_balance_object, by_hash >( hash );
} FC_CAPTURE_AND_RETHROW( (hash) ) }

const confidential_balance_object* database::find_confidential_balance( const digest_type& hash )const
{
   return find< confidential_balance_object, by_hash >( hash );
}

const asset_delegation_object& database::get_asset_delegation( const account_name_type& delegator, const account_name_type& delegatee, const asset_symbol_type& symbol )const
{ try {
	return get< asset_delegation_object, by_delegator >( boost::make_tuple( delegator, delegatee, symbol ) );
} FC_CAPTURE_AND_RETHROW( (delegator)(delegatee)(symbol) ) }

const asset_delegation_object* database::find_asset_delegation( const account_name_type& delegator, const account_name_type& delegatee, const asset_symbol_type& symbol )const
{
   return find< asset_delegation_object, by_delegator >( boost::make_tuple( delegator, delegatee, symbol ) );
}

const savings_withdraw_object& database::get_savings_withdraw( const account_name_type& owner, const shared_string& request_id )const
{ try {
   return get< savings_withdraw_object, by_request_id >( boost::make_tuple( owner, request_id ) );
} FC_CAPTURE_AND_RETHROW( (owner)(request_id) ) }

const savings_withdraw_object* database::find_savings_withdraw( const account_name_type& owner, const shared_string& request_id )const
{
   return find< savings_withdraw_object, by_request_id >( boost::make_tuple( owner, request_id ) );
}

const savings_withdraw_object& database::get_savings_withdraw( const account_name_type& owner, const string& request_id )const
{ try {
   return get< savings_withdraw_object, by_request_id >( boost::make_tuple( owner, request_id ) );
} FC_CAPTURE_AND_RETHROW( (owner)(request_id) ) }

const savings_withdraw_object* database::find_savings_withdraw( const account_name_type& owner, const string& request_id )const
{
   return find< savings_withdraw_object, by_request_id >( boost::make_tuple( owner, request_id ) );
}


/**
 * Adjusts an account's liquid balance of a specified asset.
 */
void database::adjust_liquid_balance( const account_object& a, const asset& delta )
{
   adjust_liquid_balance( a.name, delta );
}

/**
 * Adjusts an account's liquid balance of a specified asset.
 */
void database::adjust_liquid_balance( const account_name_type& a, const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }
   else if( a == NULL_ACCOUNT )
   {
      FC_ASSERT( delta.amount > 0,
         "Cannot reduce the balance of the Null Account. It has nothing." );
      if( delta.symbol == SYMBOL_COIN )
      {
         const dynamic_global_property_object& props = get_dynamic_global_properties();
         modify( props, [&]( dynamic_global_property_object& dgpo )
         {
            dgpo.accumulated_network_revenue += delta;
         });
      }
      return;
   }

   const account_balance_object* account_balance_ptr = find_account_balance( a, delta.symbol );
   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   
   if( account_balance_ptr == nullptr )      // New balance object
   {
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}", 
         ("a", a )
         ("b", to_pretty_string( asset( 0, delta.symbol )))
         ("r", to_pretty_string( -delta )));

      time_point now = head_block_time();

      create< account_balance_object >( [&]( account_balance_object& abo ) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.liquid_balance = delta.amount;
         abo.last_interest_time = now;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_liquid_supply( delta );
      });
   } 
   else 
   {
      if( delta.amount < 0 ) 
      {
         FC_ASSERT( account_balance_ptr->get_liquid_balance() >= -delta, 
            "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                  ("a", a)
                  ("b", to_pretty_string( account_balance_ptr->get_liquid_balance() ))
                  ("r", to_pretty_string( -delta )));
      }
      modify( *account_balance_ptr, [&]( account_balance_object& abo ) 
      {
         abo.adjust_liquid_balance( delta );
      });

      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_liquid_supply( delta );
      });
   }
   // ilog( "Account: ${a} adjust liquid balance: ${d}", ("d", delta )("a", a) );
} FC_CAPTURE_AND_RETHROW( (a)( delta ) ) }

/**
 * Adjusts an account's staked balance of a specified asset.
 */
void database::adjust_staked_balance( const account_object& a, const asset& delta )
{
   adjust_staked_balance(a.name, delta);
}

/**
 * Adjusts an account's staked balance of a specified asset.
 */
void database::adjust_staked_balance( const account_name_type& a, const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }
   else if( a == NULL_ACCOUNT )
   {
      FC_ASSERT( delta.amount > 0,
         "Cannot reduce the balance of the Null Account. It has nothing." );
      if( delta.symbol == SYMBOL_COIN )
      {
         const dynamic_global_property_object& props = get_dynamic_global_properties();
         modify( props, [&]( dynamic_global_property_object& dgpo) 
         {
            dgpo.accumulated_network_revenue += delta;
         });
      }
      return;
   }

   const account_balance_object* account_balance_ptr = find_account_balance( a, delta.symbol );
   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   
   if( account_balance_ptr == nullptr )      // New balance object
   {
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}", 
         ("a", a )
         ("b", to_pretty_string( asset( 0, delta.symbol )))
         ("r", to_pretty_string( -delta )));

      time_point now = head_block_time();

      create< account_balance_object >( [&]( account_balance_object& abo ) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.staked_balance = delta.amount;
         abo.last_interest_time = now;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo )
      {
         addo.adjust_staked_supply( delta );
      });
   } 
   else 
   {
      if( delta.amount < 0 ) 
      {
         FC_ASSERT( account_balance_ptr->get_staked_balance() >= -delta, 
            "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                  ("a", a)
                  ("b", to_pretty_string( account_balance_ptr->get_staked_balance() ))
                  ("r", to_pretty_string( -delta )));
      }
      modify( *account_balance_ptr, [&]( account_balance_object& abo ) 
      {
         abo.adjust_staked_balance( delta );
      });

      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_staked_supply( delta );
      });
   }
   // ilog( "Account: ${a} adjust staked balance: ${d}", ("d", delta )("a", a) );
} FC_CAPTURE_AND_RETHROW( (a)( delta ) ) }

/**
 * Adjusts an account's savings balance of a specified asset.
 */
void database::adjust_savings_balance( const account_object& a, const asset& delta )
{
   adjust_savings_balance(a.name, delta);
}

/**
 * Adjusts an account's savings balance of a specified asset.
 */
void database::adjust_savings_balance( const account_name_type& a, const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }
   else if( a == NULL_ACCOUNT )
   {
      FC_ASSERT( delta.amount > 0,
         "Cannot reduce the balance of the Null Account. It has nothing." );
      if( delta.symbol == SYMBOL_COIN )
      {
         const dynamic_global_property_object& props = get_dynamic_global_properties();
         modify( props, [&]( dynamic_global_property_object& dgpo ) 
         {
            dgpo.accumulated_network_revenue += delta;
         });
      }
      return;
   }

   const account_balance_object* account_balance_ptr = find_account_balance( a, delta.symbol );
   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   
   if( account_balance_ptr == nullptr )      // New balance object
   {
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}", 
         ("a", a )
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      time_point now = head_block_time();

      create< account_balance_object >( [&]( account_balance_object& abo )
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.savings_balance = delta.amount;
         abo.last_interest_time = now;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo )
      {
         addo.adjust_savings_supply( delta );
      });
   }
   else
   {
      if( delta.amount < 0 ) 
      {
         FC_ASSERT( account_balance_ptr->get_savings_balance() >= -delta, 
            "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                  ("a", a)
                  ("b", to_pretty_string( account_balance_ptr->get_savings_balance() ))
                  ("r", to_pretty_string( -delta )));
      }
      modify( *account_balance_ptr, [&]( account_balance_object& abo ) 
      {
         abo.adjust_savings_balance( delta );
      });

      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_savings_supply( delta );
      });
   }
   // ilog( "Account: ${a} adjust savings balance: ${d}", ("d", delta )("a", a) );
} FC_CAPTURE_AND_RETHROW( (a)( delta ) ) }

/**
 * Adjusts an account's reward balance of a specified asset.
 */
void database::adjust_reward_balance( const account_object& a, const asset& delta )
{
   adjust_reward_balance(a.name, delta);
}

/**
 * Adjusts an account's reward balance of a specified asset.
 */
void database::adjust_reward_balance( const account_name_type& a, const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }
   else if( a == NULL_ACCOUNT )
   {
      FC_ASSERT( delta.amount > 0,
         "Cannot reduce the balance of the Null Account. It has nothing." );
      if( delta.symbol == SYMBOL_COIN )
      {
         const dynamic_global_property_object& props = get_dynamic_global_properties();
         modify( props, [&]( dynamic_global_property_object& dgpo ) 
         {
            dgpo.accumulated_network_revenue += delta;
         });
      }
      return;
   }

   const account_balance_object* account_balance_ptr = find_account_balance( a, delta.symbol );
   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   
   if( account_balance_ptr == nullptr )      // New balance object
   {
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}", 
         ("a", a )
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      time_point now = head_block_time();

      create< account_balance_object >( [&]( account_balance_object& abo )
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.reward_balance = delta.amount;
         abo.last_interest_time = now;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo )
      {
         addo.adjust_reward_supply( delta );
      });
   }
   else
   {
      if( delta.amount < 0 )
      {
         FC_ASSERT( account_balance_ptr->get_reward_balance() >= -delta, 
            "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                  ("a", a)
                  ("b", to_pretty_string( account_balance_ptr->get_reward_balance() ))
                  ("r", to_pretty_string( -delta )));
      }
      modify( *account_balance_ptr, [&]( account_balance_object& abo ) 
      {
         abo.adjust_reward_balance( delta );
      });

      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_reward_supply( delta );
      });
   }
   // ilog( "Account: ${a} adjust reward balance: ${d}", ("d", delta )("a", a) );
} FC_CAPTURE_AND_RETHROW( (a)( delta ) ) }

/**
 * Adjusts an account's delegated balance of a specified asset.
 */
void database::adjust_delegated_balance( const account_object& a, const asset& delta )
{
   adjust_delegated_balance(a.name, delta);
}

/**
 * Adjusts an account's delegated balance of a specified asset.
 */
void database::adjust_delegated_balance( const account_name_type& a, const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }
   else if( a == NULL_ACCOUNT )
   {
      FC_ASSERT( delta.amount > 0,
         "Cannot reduce the balance of the Null Account. It has nothing." );
      if( delta.symbol == SYMBOL_COIN )
      {
         const dynamic_global_property_object& props = get_dynamic_global_properties();
         modify( props, [&]( dynamic_global_property_object& dgpo )
         {
            dgpo.accumulated_network_revenue += delta;
         });
      }
      return;
   }

   const account_balance_object* account_balance_ptr = find_account_balance( a, delta.symbol );
   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   
   if( account_balance_ptr == nullptr )      // New balance object
   {
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}", 
         ("a", a )
         ("b", to_pretty_string( asset( 0, delta.symbol )))
         ("r", to_pretty_string( -delta )));

      time_point now = head_block_time();

      create< account_balance_object >( [&]( account_balance_object& abo )
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.delegated_balance = delta.amount;
         abo.last_interest_time = now;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_delegated_supply( delta );
      });
   } 
   else 
   {
      if( delta.amount < 0 ) 
      {
         FC_ASSERT( account_balance_ptr->get_delegated_balance() >= -delta, 
            "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                  ("a", a)
                  ("b", to_pretty_string( account_balance_ptr->get_delegated_balance() ))
                  ("r", to_pretty_string( -delta )));
      }
      modify( *account_balance_ptr, [&]( account_balance_object& abo ) 
      {
         abo.adjust_delegated_balance( delta );
      });

      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_delegated_supply( delta );
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)( delta ) ) }

/**
 * Adjusts an account's recieving balance of a specified asset.
 */
void database::adjust_receiving_balance( const account_object& a, const asset& delta )
{
   adjust_receiving_balance(a.name, delta);
}

/**
 * Adjusts an account's recieving balance of a specified asset.
 */
void database::adjust_receiving_balance( const account_name_type& a, const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }
   else if( a == NULL_ACCOUNT )
   {
      FC_ASSERT( delta.amount > 0,
         "Cannot reduce the balance of the Null Account. It has nothing." );
      if( delta.symbol == SYMBOL_COIN )
      {
         const dynamic_global_property_object& props = get_dynamic_global_properties();
         modify( props, [&]( dynamic_global_property_object& dgpo ) 
         {
            dgpo.accumulated_network_revenue += delta;
         });
      }
      return;
   }

   const account_balance_object* account_balance_ptr = find_account_balance( a, delta.symbol );
   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   
   if( account_balance_ptr == nullptr )      // New balance object
   {
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}", 
         ("a", a )
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta )));

      time_point now = head_block_time();

      create< account_balance_object >( [&]( account_balance_object& abo ) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.receiving_balance = delta.amount;
         abo.last_interest_time = now;
         abo.stake_rate = 0;
         abo.next_stake_time = time_point::maximum();
         abo.to_stake = 0;
         abo.total_staked = 0;
         abo.unstake_rate = 0;
         abo.next_unstake_time = time_point::maximum();
         abo.to_unstake = 0;
         abo.total_unstaked = 0;
      });
      modify( asset_dyn_data, [&] ( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_receiving_supply( delta );
      });
   } 
   else 
   {
      if( delta.amount < 0 ) 
      {
         FC_ASSERT( account_balance_ptr->get_receiving_balance() >= -delta, 
            "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                  ("a", a)
                  ("b", to_pretty_string( account_balance_ptr->get_receiving_balance() ))
                  ("r", to_pretty_string( -delta )));
      }
      modify( *account_balance_ptr, [&]( account_balance_object& abo ) 
      {
         abo.adjust_receiving_balance( delta );
      });

      modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
      {
         addo.adjust_receiving_supply( delta );
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)( delta ) ) }

/**
 * Adjusts the network's pending supply of a specified asset.
 */
void database::adjust_pending_supply( const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }

   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   if( delta.amount < 0 ) 
   {
      FC_ASSERT( asset_dyn_data.get_pending_supply() >= -delta, 
         "Insufficient Pending supply: ${a}'s balance of ${b} is less than required ${r}",
               ("a", delta.symbol)
               ("b", to_pretty_string( asset_dyn_data.get_pending_supply() ))
               ("r", to_pretty_string( -delta )));
   }
   
   modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
   {
      addo.adjust_pending_supply( delta );
   });
   
} FC_CAPTURE_AND_RETHROW( ( delta ) ) }


/**
 * Adjusts the network's confidential supply of a specified asset.
 */
void database::adjust_confidential_supply( const asset& delta )
{ try {
   if( delta.amount == 0 )
   {
      return;
   }

   const asset_dynamic_data_object& asset_dyn_data = get_dynamic_data( delta.symbol );
   if( delta.amount < 0 ) 
   {
      FC_ASSERT( asset_dyn_data.get_confidential_supply() >= -delta, 
         "Insufficient Pending supply: ${a}'s balance of ${b} is less than required ${r}",
               ("a", delta.symbol)
               ("b", to_pretty_string( asset_dyn_data.get_confidential_supply() ))
               ("r", to_pretty_string( -delta )));
   }
   
   modify( asset_dyn_data, [&]( asset_dynamic_data_object& addo ) 
   {
      addo.adjust_confidential_supply( delta );
   });
   
} FC_CAPTURE_AND_RETHROW( ( delta ) ) }

/**
 * Retrieves an account's liquid balance of a specified asset.
 */
asset database::get_liquid_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_liquid_balance(a.name, symbol);
}

/**
 * Retrieves an account's liquid balance of a specified asset.
 */
asset database::get_liquid_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      ilog( "Liquid ${s} balance of account: ${acc} is not found",
      ("acc",a)("s",symbol));
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_liquid_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

/**
 * Retrieves an account's staked balance of a specified asset.
 */
asset database::get_staked_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_staked_balance(a.name, symbol);
}

/**
 * Retrieves an account's staked balance of a specified asset.
 */
asset database::get_staked_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      ilog( "Staked ${s} balance of account: ${acc} is not found",
      ("acc",a)("s",symbol));
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_staked_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

/**
 * Retrieves an account's reward balance of a specified asset.
 */
asset database::get_reward_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_reward_balance(a.name, symbol);
}

/**
 * Retrieves an account's reward balance of a specified asset.
 */
asset database::get_reward_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      ilog( "Reward ${s} balance of account: ${acc} is not found",
      ("acc",a)("s",symbol));
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_reward_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

/**
 * Retrieves an account's savings balance of a specified asset.
 */
asset database::get_savings_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_savings_balance(a.name, symbol);
}

/**
 * Retrieves an account's savings balance of a specified asset.
 */
asset database::get_savings_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      ilog( "Savings ${s} balance of account: ${acc} is not found",
      ("acc",a)("s",symbol));
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_savings_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

/**
 * Retrieves an account's delegated balance of a specified asset.
 */
asset database::get_delegated_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_delegated_balance(a.name, symbol);
}

/**
 * Retrieves an account's delegated balance of a specified asset.
 */
asset database::get_delegated_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      ilog( "Delegated ${s} balance of account: ${acc} is not found",
      ("acc",a)("s",symbol));
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_delegated_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

/**
 * Retrieves an account's recieving balance of a specified asset.
 */
asset database::get_receiving_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_receiving_balance(a.name, symbol);
}

/**
 * Retrieves an account's recieving balance of a specified asset.
 */
asset database::get_receiving_balance( const account_name_type& a, const asset_symbol_type& symbol )const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      ilog( "Receiving ${s} balance of account: ${acc} is not found",
      ("acc",a)("s",symbol));
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_receiving_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of the specified currency and its equity asset.
 */
share_type database::get_voting_power( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_voting_power( a.name, symbol );
}

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of the specified currency and its equity asset.
 */
share_type database::get_voting_power( const account_name_type& a, const asset_symbol_type& symbol )const
{
   const asset_currency_data_object& currency = get_currency_data( symbol );
   price equity_coin_price = get_liquidity_pool( symbol, currency.equity_asset ).hour_median_price;
   return get_voting_power( a, equity_coin_price );
}

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of the specified currency and its equity asset.
 * Currency is base, equity is quote.
 */
share_type database::get_voting_power( const account_object& a, const price& equity_coin_price )const
{
   return get_voting_power( a.name, equity_coin_price );
}

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of the specified currency and its equity asset.
 */
share_type database::get_voting_power( const account_name_type& a, const price& equity_coin_price )const
{
   const account_balance_object* coin_ptr = find_account_balance( a, equity_coin_price.base.symbol );
   const account_balance_object* equity_ptr = find_account_balance( a, equity_coin_price.quote.symbol );
   asset coin_vote = asset( 0, equity_coin_price.base.symbol );
   asset equity_vote = asset( 0, equity_coin_price.quote.symbol );
   
   if( coin_ptr != nullptr )
   {
      coin_vote = coin_ptr->get_voting_power();
   }
   if( equity_ptr != nullptr )
   {
      equity_vote = equity_ptr->get_voting_power();
   }

   share_type voting_power = coin_vote.amount + ( equity_vote * equity_coin_price ).amount;

   /**
   ilog( "Account: ${a} has Voting Power: ${p} [ Coin:${c} Equity:${q} at Equity Price: ${e} ]", 
   ("a", a)("p", voting_power )("e", equity_coin_price)("c", coin_vote)("q", equity_vote) );
   */

   return voting_power;
}

share_type database::get_proxied_voting_power( const account_object& a, const price& equity_price )const
{ try {
   if( a.proxied.size() == 0 )
   {
      return 0;
   }
   share_type voting_power = 0;
   for( auto name : a.proxied )
   {
      voting_power += get_voting_power( name, equity_price );
      voting_power += get_proxied_voting_power( name, equity_price );    // Recursively finds voting power of proxies.
   }
   return voting_power;

} FC_CAPTURE_AND_RETHROW() }


share_type database::get_proxied_voting_power( const account_name_type& a, const price& equity_price )const
{ try {
   return get_proxied_voting_power( get_account(a), equity_price );
} FC_CAPTURE_AND_RETHROW() }


share_type database::get_equity_voting_power( const account_object& a, const account_business_object& b )const
{ try {
   return get_equity_voting_power( a.name, b );
} FC_CAPTURE_AND_RETHROW() }


share_type database::get_equity_voting_power( const account_name_type& a, const account_business_object& b )const
{ try {
   share_type voting_power = 0;
   for( auto symbol : b.equity_assets )
   {
      const account_balance_object* abo_ptr = find_account_balance( a, symbol );
      if( abo_ptr != nullptr )
      {
         voting_power += abo_ptr->get_voting_power().amount;
      }
   }
   return voting_power;
} FC_CAPTURE_AND_RETHROW() }


void database::process_savings_withdraws()
{
   // ilog( "Process Savings Withdraws" );

   const auto& savings_idx = get_index< savings_withdraw_index >().indices().get< by_complete_from_request_id >();
   auto savings_itr = savings_idx.begin();
   time_point now = head_block_time();

   while( savings_itr != savings_idx.end() && 
      savings_itr->complete <= now )
   {
      const savings_withdraw_object& withdraw = *savings_itr;
      ++savings_itr;

      adjust_liquid_balance( withdraw.to, withdraw.amount );
      const account_object& account = get_account( withdraw.from );

      modify( account, [&]( account_object& a )
      {
         a.savings_withdraw_requests--;
      });

      push_virtual_operation( 
         fill_transfer_from_savings_operation( 
         withdraw.from, 
         withdraw.to, 
         withdraw.amount, 
         to_string( withdraw.request_id ), 
         to_string( withdraw.memo ))
      );

      ilog( "Processed Savings Withdrawal from account: ${f} to recipient: ${t} of amount: ${a}",
         ("f",withdraw.from)("t",withdraw.to)("a",withdraw.amount.to_string()) );

      remove( withdraw );
   }
}


/**
 * Decrement an active asset delegation upon the expiration of
 * a delegation expiration object.
 * 
 * Withdrawing a delegation has a 24 hour time delay to ensure stake is not 
 * used to vote multiple times in rapid succession.
 */
void database::clear_expired_delegations()
{
   // ilog( "Clear Expired Delegations" );

   time_point now = head_block_time();
   const auto& exp_idx = get_index< asset_delegation_expiration_index, by_expiration >();
   
   auto exp_itr = exp_idx.begin();

   while( exp_itr != exp_idx.end() && 
      exp_itr->expiration <= now )
   {
      const asset_delegation_expiration_object& exp = *exp_itr;
      ++exp_itr;
      
      adjust_delegated_balance( exp.delegator, -exp.amount );
      adjust_receiving_balance( exp.delegatee, -exp.amount );
      push_virtual_operation( return_asset_delegation_operation( exp_itr->delegator, exp_itr->amount ) );
      ilog( "Removed: ${v}",("v",exp));
      remove( exp );
   }
}


/**
 * Clear all balance and supply values, and open orders for a temporary asset.
 */
void database::clear_asset_balances( const asset_symbol_type& symbol )
{ try {
   const asset_object& asset_obj = get_asset( symbol );

   FC_ASSERT( asset_obj.is_temp_asset(),
      "Cannot clear asset balances of a non-temporary Asset." );

   ilog( "Clear Asset Balances: ${s}", ("s", symbol ) );

   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   const auto& limit_idx = get_index< limit_order_index >().indices().get< by_symbol >();
   const auto& auction_idx = get_index< auction_order_index >().indices().get< by_symbol >();
   const auto& option_idx = get_index< option_order_index >().indices().get< by_symbol >();
   const auto& dyn_data_idx = get_index< asset_dynamic_data_index >().indices().get< by_symbol >();

   auto balance_itr = balance_idx.lower_bound( symbol );
   
   while( balance_itr != balance_idx.end() &&
   balance_itr->symbol == symbol )
   {
      const account_balance_object& balance = *balance_itr;
      ++balance_itr;
      ilog( "Removed: ${v}",("v",balance));
      remove( balance );
   }

   auto limit_itr = limit_idx.lower_bound( symbol );

   while( limit_itr != limit_idx.end() &&
   limit_itr->sell_asset() == symbol )
   {
      const limit_order_object& limit = *limit_itr;
      ++limit_itr;
      ilog( "Removed: ${v}",("v",limit));
      remove( limit );
   }

   auto auction_itr = auction_idx.lower_bound( symbol );

   while( auction_itr != auction_idx.end() &&
   auction_itr->sell_asset() == symbol )
   {
      const auction_order_object& auction = *auction_itr;
      ++auction_itr;
      ilog( "Removed: ${v}",("v",auction));
      remove( auction );
   }

   auto option_itr = option_idx.lower_bound( symbol );

   while( option_itr != option_idx.end() &&
      option_itr->debt_type() == symbol )
   {
      const option_order_object& order = *option_itr;
      ++option_itr;
      close_option_order( order );
   }

   auto dyn_data_itr = dyn_data_idx.lower_bound( symbol );

   while( dyn_data_itr != dyn_data_idx.end() &&
      dyn_data_itr->symbol == symbol )
   {
      const asset_dynamic_data_object& dyn_data = *dyn_data_itr;
      ++dyn_data_itr;

      modify( dyn_data, [&]( asset_dynamic_data_object& addo )
      {
         addo.clear_supply();
      });
   }
} FC_CAPTURE_AND_RETHROW() }


string database::to_pretty_string( const asset& a )const
{
   return a.to_string();
}


} } // node::chain