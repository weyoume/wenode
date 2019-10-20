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


void database::process_asset_staking()
{
   const auto& widx = get_index< account_balance_index >().indices().get< by_next_unstake_time >();
   const auto& didx = get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
   
   auto current = widx.begin();
   const auto& cprops = get_dynamic_global_properties();

   while( current != widx.end() && current->next_unstake_time <= head_block_time() )
   {
      const auto& from_account_balance = *current; ++current;
      const auto& from_account = get_account( from_account_balance.owner );
      const auto& symbol = from_account_balance.symbol;

      share_type to_unstake;

      if ( from_account_balance.to_unstake - from_account_balance.total_unstaked < from_account_balance.unstake_rate )
         to_unstake = std::min( from_account_balance.staked_balance, from_account_balance.to_unstake % from_account_balance.unstake_rate).value;
      else
         to_unstake = std::min( from_account_balance.staked_balance, from_account_balance.unstake_rate ).value;

      share_type total_restake = 0;
      share_type total_withdrawn = 0;

      asset unstake_asset = asset(to_unstake, symbol);

      adjust_staked_balance(from_account, asset(unstake_asset, symbol));

      for( auto itr = didx.upper_bound( boost::make_tuple( from_account_balance.owner, account_id_type() ) );
           itr != didx.end() && itr->from_account == from_account_balance.owner; ++itr )
      {
         if( itr->auto_stake )
         {
            share_type to_restake = (( to_unstake * itr->percent ) / PERCENT_100 ).to_uint64();
            total_restake += to_restake;

            if( to_restake > 0 )
            {
               const auto& to_account = get_account(itr->to_account);
               adjust_staked_balance(to_account, to_restake);
            }
         }
      }

      for( auto itr = didx.upper_bound( boost::make_tuple( from_account_balance.owner, account_id_type() ) );
           itr != didx.end() && itr->from_account == from_account_balance.owner;
           ++itr )
      {
         if( !itr->auto_stake )
         {
            const auto& to_account = get_account(itr->to_account);

            share_type to_withdraw = (( to_unstake * itr->percent ) / PERCENT_100 ).to_uint64();
            total_withdrawn += to_withdraw;
            asset withdraw_asset = asset(to_withdraw, symbol);

            if( to_withdraw > 0 )
            {
               adjust_liquid_balance(to_account, withdraw_asset);
            }
         }
      }

      modify( from_account_balance, [&]( account_balance_object& abo )
      {
         abo.total_unstaked += to_unstake;

         if( abo.total_unstaked >= abo.to_unstake || abo.staked_balance == 0 )
         {
            abo.unstake_rate = 0;
            abo.next_unstake_time = fc::time_point::maximum();
         }
         else
         {
            abo.next_unstake_time += fc::seconds( STAKE_WITHDRAW_INTERVAL_SECONDS );
         }
      });
   }
}

void database::process_savings_withdraws()
{
  const auto& idx = get_index< savings_withdraw_index >().indices().get< by_complete_from_rid >();
  auto itr = idx.begin();
  while( itr != idx.end() ) {
     if( itr->complete > head_block_time() )
        break;
     adjust_liquid_balance( get_account( itr->to ), itr->amount );

     modify( get_account( itr->from ), [&]( account_object& a )
     {
        a.savings_withdraw_requests--;
     });

     push_virtual_operation( fill_transfer_from_savings_operation( itr->from, itr->to, itr->amount, itr->request_id, to_string( itr->memo) ) );

     remove( *itr );
     itr = idx.begin();
  }
}

/**
 * Calaulates the relative share of equity reward dividend distribution that an account should recieve
 * based on its network contribution information.
 */
share_type get_equity_shares( const account_balance_object& balance)
{
   FC_ASSERT( balance.symbol == SYMBOL_EQUITY, "Equity rewards requires equity asset balance object." );
   FC_ASSERT( balance.staked_balance >= BLOCKCHAIN_PRECISION, "Equity rewards requires minimum balance of 1 Equity asset." );
   const account_object& account = get_account(balance.owner);
   time_point now = head_block_time();
   if( (account.witnesses_voted_for < MIN_EQUITY_WITNESSES) || 
      (now > (account.last_activity_reward + fc::days(30)) )
   {
      return 0;  // Account does not recieve equity reward when witness votes are insufficient, or no activity in last 30 days.
   }

   share_type equity_shares = balance.staked_balance;  // Start with staked balance.

   if( (balance.staked_balance >= 10 * BLOCKCHAIN_PRECISION) &&
      (account.witnesses_voted_for >= EQUITY_BOOST_WITNESSES) &&
      (account.recent_activity_claims >= EQUITY_BOOST_ACTVITY) ) // TODO: additional conditions for doubling equity reward 
   {
      equity_shares *= 2;    // Doubles equity reward when 10+ WYM balance, 50+ witness votes, and 15+ Activity rewards in last 30 days
   }

   if( account.membership == TOP) 
   {
      equity_shares = (equity_shares * EQUITY_BOOST_TOP_PERCENT) / PERCENT_100;
   }
}

/**
 * Allocates the equity dividend rewards to account holders with a minimum balance
 * and recent actvity
 */
void database::process_equity_rewards()
{ try {
   if( (head_block_num() % EQUITY_INTERVAL_BLOCKS) != 0 )    // Runs once per week
      return;

   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol_stake >();
   auto itr = balance_idx.lower_bound( SYMBOL_EQUITY );
   const reward_fund_object& reward_fund = get_reward_fund();
   const asset_dynamic_data_object& core_asset_dynamic_data = get_core_dynamic_data();

   vector< pair < account_name_type, share_type > > balances;
   share_type total_equity_shares = 0;

   while( itr != balance_idx.end() && itr->staked_balance >= BLOCKCHAIN_PRECISION ) 
   {
      share_type equity_shares = get_equity_shares( *itr );

      if(equity_shares > 0 )
      {
         total_equity_shares += equity_shares;
         balances.push_back( std::make_pair( &(*itr), equity_shares ) ); // Add balance pointer and equity shares to vector
      }
      ++itr;
   }

   asset equity_reward_balance = reward_fund.equity_reward_balance;  // record the opening balance of the equity reward fund

   FC_ASSERT( equity_reward_balance > 0, "Critical Error: Negative or zero equity reward fund balance.");

   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.adjust_equity_reward_balance( -equity_reward_balance );  // Empty the equity reward fund balance.
   });

   modify( core_asset_dyn_data, [&](asset_dynamic_data_object& addo) 
   {
      addo.adjust_pending_supply( -equity_reward_balance );   // Deduct equity reward fund balance from pending supply.
   });

   for( pair < account_name_type, share_type > b : balances )
   {
      asset equity_reward = (equity_reward_balance * b.second) / total_equity_shares; 
      adjust_reward_balance(b.first, equity_reward);       // Pay equity reward to each account 
   }
} FC_CAPTURE_AND_RETHROW() }


void database::expire_escrow_ratification()
{
   const auto& escrow_idx = get_index< escrow_index >().indices().get< by_ratification_deadline >();
   auto escrow_itr = escrow_idx.lower_bound( false );

   while( escrow_itr != escrow_idx.end() && !escrow_itr->is_approved() && escrow_itr->ratification_deadline <= head_block_time() )
   {
      const auto& old_escrow = *escrow_itr;
      ++escrow_itr;

      const auto& from_account = get_account( old_escrow.from );
      adjust_liquid_balance( from_account, old_escrow.balance);
      adjust_liquid_balance( from_account, old_escrow.pending_fee );

      remove( old_escrow );
   }
}

/**
void database::update_median_feed() 
{ try {
   if( (head_block_num() % FEED_INTERVAL_BLOCKS) != 0 )
      return;

   auto now = head_block_time();
   const witness_schedule_object& wso = get_witness_schedule_object();
   vector<price> feeds; feeds.reserve( wso.num_scheduled_witnesses );
   for( int i = 0; i < wso.num_scheduled_witnesses; i++ )
   {
      const auto& wit = get_witness( wso.current_shuffled_producers[i] );
      if( now < wit.last_USD_exchange_update + MAX_FEED_AGE
         && !wit.USD_exchange_rate.is_null() )
      {
         feeds.push_back( wit.USD_exchange_rate );
      }
   }

   if( feeds.size() >= MIN_FEEDS )
   {
      std::sort( feeds.begin(), feeds.end() );
      auto median_feed = feeds[feeds.size()/2];

      modify( get_feed_history(), [&]( feed_history_object& fho )
      {
         fho.price_history.push_back( median_feed );

         size_t feed_history_window = FEED_HISTORY_WINDOW;

         if( fho.price_history.size() > feed_history_window )
            fho.price_history.pop_front();

         if( fho.price_history.size() )
         {
            std::deque< price > copy;
            for( auto i : fho.price_history )
            {
               copy.push_back( i );
            }

            std::sort( copy.begin(), copy.end() );               /// TODO: use nth_item
            fho.current_median_history = copy[copy.size()/2];
         }
      });
   }
} FC_CAPTURE_AND_RETHROW() }
*/


bool database::apply_order( const limit_order_object& new_order_object )
{
   auto order_id = new_order_object.id;

   const auto& limit_price_idx = get_index<limit_order_index>().indices().get<by_price>();

   auto max_price = ~new_order_object.sell_price;
   auto limit_itr = limit_price_idx.lower_bound(max_price.max());
   auto limit_end = limit_price_idx.upper_bound(max_price);

   bool finished = false;
   while( !finished && limit_itr != limit_end )
   {
      auto old_limit_itr = limit_itr;
      ++limit_itr;
      // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
      finished = ( match(new_order_object, *old_limit_itr, old_limit_itr->sell_price) & 0x1 );
   }

   return find< limit_order_object >( order_id ) == nullptr;
}

int database::match( const limit_order_object& new_order, const limit_order_object& old_order, const price& match_price )
{
   assert( new_order.sell_price.quote.symbol == old_order.sell_price.base.symbol );
   assert( new_order.sell_price.base.symbol  == old_order.sell_price.quote.symbol );
   assert( new_order.for_sale > 0 && old_order.for_sale > 0 );
   assert( match_price.quote.symbol == new_order.sell_price.base.symbol );
   assert( match_price.base.symbol == old_order.sell_price.base.symbol );

   auto new_order_for_sale = new_order.amount_for_sale();
   auto old_order_for_sale = old_order.amount_for_sale();

   asset new_order_pays, new_order_receives, old_order_pays, old_order_receives;

   if( new_order_for_sale <= old_order_for_sale * match_price )
   {
      old_order_receives = new_order_for_sale;
      new_order_receives  = new_order_for_sale * match_price;
   }
   else
   {
      //This line once read: assert( old_order_for_sale < new_order_for_sale * match_price );
      //This assert is not always true -- see trade_amount_equals_zero in operation_tests.cpp
      //Although new_order_for_sale is greater than old_order_for_sale * match_price, old_order_for_sale == new_order_for_sale * match_price
      //Removing the assert seems to be safe -- apparently no asset is created or destroyed.
      new_order_receives = old_order_for_sale;
      old_order_receives = old_order_for_sale * match_price;
   }

   old_order_pays = new_order_receives;
   new_order_pays = old_order_receives;

   assert( new_order_pays == new_order.amount_for_sale() ||
           old_order_pays == old_order.amount_for_sale() );

   auto age = head_block_time() - old_order.created;

   push_virtual_operation( fill_order_operation( new_order.seller, new_order.orderid, new_order_pays, old_order.seller, old_order.orderid, old_order_pays ) );

   int result = 0;
   result |= fill_order( new_order, new_order_pays, new_order_receives );
   result |= fill_order( old_order, old_order_pays, old_order_receives ) << 1;
   assert( result != 0 );
   return result;
}


bool database::fill_order( const limit_order_object& order, const asset& pays, const asset& receives )
{
   try
   {
      FC_ASSERT( order.amount_for_sale().symbol == pays.symbol );
      FC_ASSERT( pays.symbol != receives.symbol );

      const account_object& seller = get_account( order.seller );

      adjust_liquid_balance( seller, receives );

      if( pays == order.amount_for_sale() )
      {
         remove( order );
         return true;
      }
      else
      {
         modify( order, [&]( limit_order_object& b )
         {
            b.for_sale -= pays.amount;
         } );
         /**
          *  There are times when the AMOUNT_FOR_SALE * SALE_PRICE == 0 which means that we
          *  have hit the limit where the seller is asking for nothing in return.  When this
          *  happens we must refund any balance back to the seller, it is too small to be
          *  sold at the sale price.
          */
         if( order.amount_to_receive().amount == 0 )
         {
            cancel_limit_order(order);
            return true;
         }
         return false;
      }
   }
   FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) )
}

void database::cancel_limit_order( const limit_order_object& order )
{
   adjust_liquid_balance( get_account(order.seller), order.amount_for_sale() );
   remove(order);
}

void database::clear_expired_transactions()
{
   //Look for expired transactions in the deduplication list, and remove them.
   //Transactions must have expired by at least two forking windows in order to be removed.
   auto& transaction_idx = get_index< transaction_index >();
   const auto& dedupe_index = transaction_idx.indices().get< by_expiration >();
   while( ( !dedupe_index.empty() ) && ( head_block_time() > dedupe_index.begin()->expiration ) )
      remove( *dedupe_index.begin() );
}

void database::clear_expired_orders()
{
   auto now = head_block_time();
   const auto& orders_by_exp = get_index<limit_order_index>().indices().get<by_expiration>();
   auto itr = orders_by_exp.begin();
   while( itr != orders_by_exp.end() && itr->expiration < now )
   {
      cancel_order( *itr );
      itr = orders_by_exp.begin();
   }
}

void database::clear_expired_delegations()
{
   auto now = head_block_time();
   const auto& delegations_by_exp = get_index< asset_delegation_expiration_index, by_expiration >();
   auto itr = delegations_by_exp.begin();
   while( itr != delegations_by_exp.end() && itr->expiration < now )
   {
      modify( get_account( itr->delegator ), [&]( account_object& a )
      {
         adjust_delegated_balance(a, itr->amount);
      });

      push_virtual_operation( return_asset_delegation_operation( itr->delegator, itr->amount ) );

      remove( *itr );
      itr = delegations_by_exp.begin();
   }
}

void database::adjust_liquid_balance( const account_object& a, const asset& delta )
{
   adjust_liquid_balance(a.name, delta);
}

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
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      create<account_balance_object>( [&]( account_balance_object& abo) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.liquid_balance = delta.amount;
         if( delta.symbol == SYMBOL_COIN )
         {
            abo.maintenance_flag = true;
         }
      });
      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_liquid_supply(delta);
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
      modify( *account_balance_ptr, [&](account_balance_object& abo) 
      {
         abo.adjust_liquid_balance(delta);
      });

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_liquid_supply(delta);
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)(delta) ) }

void database::adjust_staked_balance( const account_object& a, const asset& delta )
{
   adjust_staked_balance(a.name, delta);
}

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
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      create<account_balance_object>( [&]( account_balance_object& abo) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.staked_balance = delta.amount;
         if( delta.symbol == SYMBOL_COIN )
         {
            abo.maintenance_flag = true;
         }
      });
      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_staked_supply(delta);
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
      modify( *account_balance_ptr, [&](account_balance_object& abo) 
      {
         abo.adjust_staked_balance(delta);
      });

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_staked_supply(delta);
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)(delta) ) }

void database::adjust_savings_balance( const account_object& a, const asset& delta )
{
   adjust_savings_balance(a.name, delta);
}

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
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      create<account_balance_object>( [&]( account_balance_object& abo) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.savings_balance = delta.amount;
         if( delta.symbol == SYMBOL_COIN )
         {
            abo.maintenance_flag = true;
         }
      });
      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_savings_supply(delta);
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
      modify( *account_balance_ptr, [&](account_balance_object& abo) 
      {
         abo.adjust_savings_balance(delta);
      });

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_savings_supply(delta);
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)(delta) ) }

void database::adjust_reward_balance( const account_object& a, const asset& delta )
{
   adjust_reward_balance(a.name, delta);
}

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
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      create<account_balance_object>( [&]( account_balance_object& abo) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.reward_balance = delta.amount;
         if( delta.symbol == SYMBOL_COIN )
         {
            abo.maintenance_flag = true;
         }
      });
      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_reward_supply(delta);
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
      modify( *account_balance_ptr, [&](account_balance_object& abo) 
      {
         abo.adjust_reward_balance(delta);
      });

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_reward_supply(delta);
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)(delta) ) }

void database::adjust_delegated_balance( const account_object& a, const asset& delta )
{
   adjust_delegated_balance(a.name, delta);
}

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
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      create<account_balance_object>( [&]( account_balance_object& abo) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.delegated_balance = delta.amount;
         if( delta.symbol == SYMBOL_COIN )
         {
            abo.maintenance_flag = true;
         }
      });
      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_delegated_supply(delta);
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
      modify( *account_balance_ptr, [&](account_balance_object& abo) 
      {
         abo.adjust_delegated_balance(delta);
      });

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_delegated_supply(delta);
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)(delta) ) }

void database::adjust_receiving_balance( const account_object& a, const asset& delta )
{
   adjust_receiving_balance(a.name, delta);
}

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
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      create<account_balance_object>( [&]( account_balance_object& abo) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.receiving_balance = delta.amount;
         if( delta.symbol == SYMBOL_COIN )
         {
            abo.maintenance_flag = true;
         }
      });
      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_receiving_supply(delta);
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
      modify( *account_balance_ptr, [&](account_balance_object& abo) 
      {
         abo.adjust_receiving_balance(delta);
      });

      modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
      {
         addo.adjust_receiving_supply(delta);
      });
   }
} FC_CAPTURE_AND_RETHROW( (a)(delta) ) }


asset database::get_liquid_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_liquid_balance(a.name, symbol);
}

asset database::get_liquid_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_liquid_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

asset database::get_staked_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_staked_balance(a.name, symbol);
}

asset database::get_staked_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_staked_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

asset database::get_reward_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_reward_balance(a.name, symbol);
}

asset database::get_reward_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_reward_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

asset database::get_savings_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_savings_balance(a.name, symbol);
}

asset database::get_savings_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_savings_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

asset database::get_delegated_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_delegated_balance(a.name, symbol);
}

asset database::get_delegated_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_delegated_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

asset database::get_receiving_balance( const account_object& a, const asset_symbol_type& symbol )const
{
   return get_receiving_balance(a.name, symbol);
}

asset database::get_receiving_balance( const account_name_type& a, const asset_symbol_type& symbol)const
{ try {
   const account_balance_object* abo_ptr = find_account_balance(a, symbol);
   if( abo_ptr == nullptr )
   {
      return asset(0, symbol);
   }
   else
   {
      return abo_ptr->get_receiving_balance();
   }
} FC_CAPTURE_AND_RETHROW( (a)(symbol) ) }

share_type database::get_voting_power( const account_object& a )const
{
   return get_voting_power(a.name);
}

share_type database::get_voting_power( const account_name_type& a )const
{
   const account_balance_object* coin_ptr = find_account_balance(a, SYMBOL_COIN);
   const account_balance_object* equity_ptr = find_account_balance(a, SYMBOL_EQUITY);
   price equity_coin_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_EQUITY).hour_median_price;
   share_type voting_power = 0;
   if( coin_ptr != nullptr )
   {
      voting_power += coin_ptr->get_voting_power().amount;
   }
   if( equity_ptr != nullptr )
   {
      voting_power += (equity_ptr->get_voting_power()*equity_coin_price).amount;
   }
   return voting_power;
}

share_type database::get_voting_power( const account_object& a, const price& equity_coin_price )const
{
   return get_voting_power( a.name, equity_coin_price );
}

share_type database::get_voting_power( const account_name_type& a, const price& equity_coin_price )const
{
   const account_balance_object* coin_ptr = find_account_balance(a, SYMBOL_COIN);
   const account_balance_object* equity_ptr = find_account_balance(a, SYMBOL_EQUITY);
   share_type voting_power = 0;
   if( coin_ptr != nullptr )
   {
      voting_power += coin_ptr->get_voting_power().amount;
   }
   if( equity_ptr != nullptr )
   {
      voting_power += (equity_ptr->get_voting_power()*equity_coin_price).amount;
   }
   return voting_power;
}

string database::to_pretty_string( const asset& a )const
{
   return a.amount_to_pretty_string(a);
}

/**
 * All margin positions are force closed at the swan price
 * Collateral received goes into a force-settlement fund
 * No new margin positions can be created for this asset
 * Force settlement happens without delay at the swan price, deducting from force-settlement fund
 * No more asset updates may be issued.
*/
void database::globally_settle_asset( const asset_object& mia, const price& settlement_price )
{ try {
   const asset_bitasset_data_object& bitasset = get_bitasset_data(mia.symbol);

   FC_ASSERT( !bitasset.has_settlement(), "black swan already occurred, it should not happen again" );

   const asset_symbol_type& backing_asset = bitasset.backing_asset;

   const asset_object& backing_asset_object = get_asset(backing_asset);

   asset collateral_gathered = asset(0, backing_asset);

   const asset_dynamic_data_object& mia_dyn = get_dynamic_data(mia.symbol);

   auto original_mia_supply = mia_dyn.total_supply;

   const auto& call_price_index = get_index< call_order_index >().indices().get<by_price>();

   // cancel all call orders and accumulate it into collateral_gathered
   auto call_itr = call_price_index.lower_bound( price::min( bitasset.backing_asset, mia.symbol ) );
   auto call_end = call_price_index.upper_bound( price::max( bitasset.backing_asset, mia.symbol ) );
   asset pays;
   while( call_itr != call_end )
   {
      pays = call_itr->get_debt().multiply_and_round_up( settlement_price ); // round up, in favor of global settlement fund

      if( pays > call_itr->get_collateral() )
         pays = call_itr->get_collateral();

      collateral_gathered += pays;
      const auto&  order = *call_itr;
      ++call_itr;
      FC_ASSERT( fill_call_order( order, pays, order.get_debt(), settlement_price, true ) ); // call order is maker
   }

   modify( bitasset, [&original_mia_supply, &collateral_gathered, &mia]( asset_bitasset_data_object& obj ){
           obj.settlement_price = asset(original_mia_supply, mia.symbol) / collateral_gathered;
           obj.settlement_fund  = collateral_gathered.amount;
           });

   /// After all margin positions are closed, the current supply will be reported as 0, but
   /// that is a lie, the supply didn't change.   We need to capture the current supply before
   /// filling all call orders and then restore it afterward. Then in the force settlement
   /// evaluator reduce the supply
   modify( mia_dyn, [original_mia_supply]( asset_dynamic_data_object& obj ){
           obj.total_supply = original_mia_supply;
         });

} FC_CAPTURE_AND_RETHROW( (mia)(settlement_price) ) }

void database::revive_bitasset( const asset_object& bitasset ) 
{ try {
   FC_ASSERT( bitasset.is_market_issued() );
   const asset_bitasset_data_object& bad = get_bitasset_data(bitasset.symbol);
   FC_ASSERT( bad.has_settlement() );
   const asset_dynamic_data_object& bdd = get_dynamic_data(bitasset.symbol);
   FC_ASSERT( !bad.is_prediction_market );
   FC_ASSERT( !bad.current_feed.settlement_price.is_null() );

   if( bdd.total_supply > 0 )
   {
      // Create + execute a "bid" with 0 additional collateral
      const collateral_bid_object& pseudo_bid = create<collateral_bid_object>([&](collateral_bid_object& bid) {
         bid.bidder = bitasset.issuer;
         bid.inv_swan_price = asset(0, bad.backing_asset)
                              / asset(bdd.total_supply, bitasset.symbol);
      });
      execute_bid( pseudo_bid, bdd.total_supply, bad.settlement_fund, bad.current_feed );
   } else
      FC_ASSERT( bad.settlement_fund == 0 );

   cancel_bids_and_revive_mpa( bitasset, bad );
} FC_CAPTURE_AND_RETHROW( (bitasset) ) }

void database::cancel_bids_and_revive_mpa( const asset_object& bitasset, const asset_bitasset_data_object& bad )
{ try {
   FC_ASSERT( bitasset.is_market_issued() );
   FC_ASSERT( bad.has_settlement() );
   FC_ASSERT( !bad.is_prediction_market );

   // cancel remaining bids
   const auto& bid_idx = get_index< collateral_bid_index >().indices().get<by_price>();
   auto itr = bid_idx.lower_bound( boost::make_tuple( bitasset.symbol, price::max( bad.backing_asset, bitasset.symbol )));
   while( itr != bid_idx.end() && itr->inv_swan_price.quote.symbol == bitasset.symbol )
   {
      const collateral_bid_object& bid = *itr;
      ++itr;
      cancel_bid( bid , true );
   }

   modify( bad, [&]( asset_bitasset_data_object& obj ){
              obj.settlement_price = price();
              obj.settlement_fund = 0;
           });
} FC_CAPTURE_AND_RETHROW( (bitasset) ) }

void database::cancel_bid(const collateral_bid_object& bid, bool create_virtual_op)
{
   const account_object& bidder_account = get_account(bid.bidder);
   adjust_liquid_balance(bidder_account, bid.inv_swan_price.base);

   if( create_virtual_op )
   {
      bid_collateral_operation vop;
      vop.bidder = bid.bidder;
      vop.additional_collateral = bid.inv_swan_price.base;
      vop.debt_covered = asset( 0, bid.inv_swan_price.quote.symbol );
      push_virtual_operation( vop );
   }
   remove(bid);
}

void database::execute_bid( const collateral_bid_object& bid, share_type debt_covered, share_type collateral_from_fund, const price_feed& current_feed )
{
   create<call_order_object>( [&](call_order_object& call ){
         call.borrower = bid.bidder;
         call.collateral = bid.inv_swan_price.base.amount + collateral_from_fund;
         call.debt = debt_covered;
         // bid.inv_swan_price is in collateral / debt
         call.call_price = price( asset( 1, bid.inv_swan_price.base.symbol ), asset( 1, bid.inv_swan_price.quote.symbol ) );
      });

   push_virtual_operation( execute_bid_operation( bid.bidder, asset( bid.inv_swan_price.base.amount + collateral_from_fund, bid.inv_swan_price.base.symbol ),
                           asset( debt_covered, bid.inv_swan_price.quote.symbol ) ) );
   remove(bid);
}

void database::cancel_settle_order(const force_settlement_object& order, bool create_virtual_op)
{
   const account_object& account_object = get_account(order.owner);
   adjust_liquid_balance(account_object, order.balance);

   if( create_virtual_op )
   {
      asset_settle_cancel_operation vop;
      vop.settlement = order.id;
      vop.account = order.owner;
      vop.amount = order.balance;
      push_virtual_operation( vop );
   }
   remove(order);
}

void database::cancel_limit_order( const limit_order_object& order )
{
   asset refunded = order.amount_for_sale();
   
   adjust_liquid_balance(order.seller, refunded); // refund funds in order

   remove(order);
}

/* Cancels limit orders with 0 assets remaining for the recipient, 
   Returns true if the order is cancelled.*/
bool database::maybe_cull_small_order( const limit_order_object& order )
{
   /**
    *  There are times when the AMOUNT_FOR_SALE * SALE_PRICE == 0 which means that we
    *  have hit the limit where the seller is asking for nothing in return.  When this
    *  happens we must refund any balance back to the seller, it is too small to be
    *  sold at the sale price.
    *
    *  If the order is a taker order (as opposed to a maker order), so the price is
    *  set by the counterparty, this check is deferred until the order becomes unmatched
    *  (see #555) -- however, detecting this condition is the responsibility of the caller.
    */
   if( order.amount_to_receive().amount == 0 )
   {
      cancel_limit_order( order );
      return true;
   }
   return false;
}

bool database::apply_order(const limit_order_object& new_order_object, bool allow_black_swan)
{
   auto order_id = new_order_object.id;
   asset_symbol_type sell_asset_symbol = new_order_object.sell_asset();
   asset_symbol_type recv_asset_symbol = new_order_object.receive_asset();

   // We only need to check if the new order will match with others if it is at the front of the book
   const auto& limit_price_idx = get_index<limit_order_index>().indices().get<by_price>();
   auto limit_itr = limit_price_idx.lower_bound( boost::make_tuple( new_order_object.sell_price, order_id ) );
   if( limit_itr != limit_price_idx.begin() )
   {
      --limit_itr;
      if( limit_itr->sell_asset() == sell_asset_symbol && limit_itr->receive_asset() == recv_asset_symbol )
         return false;
   }

   // this is the opposite side (on the book)
   auto max_price = ~new_order_object.sell_price;
   limit_itr = limit_price_idx.lower_bound( max_price.max() );
   auto limit_end = limit_price_idx.upper_bound( max_price );

   // Order matching should be in favor of the taker.
   // When a new limit order is created, e.g. an ask, need to check if it will match the highest bid.
   // We were checking call orders first. However, due to MSSR (maximum_short_squeeze_ratio),
   // effective price of call orders may be worse than limit orders, so we should also check limit orders here.

   // Question: will a new limit order trigger a black swan event?
   //
   // 1. as of writing, it's possible due to the call-order-and-limit-order overlapping issue:
   //       https://github.com/bitshares/bitshares-core/issues/606 .
   //    when it happens, a call order can be very big but don't match with the opposite,
   //    even when price feed is too far away, further than swan price,
   //    if the new limit order is in the same direction with the call orders, it can eat up all the opposite,
   //    then the call order will lose support and trigger a black swan event.
   // 2. after issue 606 is fixed, there will be no limit order on the opposite side "supporting" the call order,
   //    so a new order in the same direction with the call order won't trigger a black swan event.
   // 3. calling is one direction. if the new limit order is on the opposite direction,
   //    no matter if matches with the call, it won't trigger a black swan event.
   //    (if a match at MSSP caused a black swan event, it means the call order is already undercollateralized,
   //      which should trigger a black swan event earlier.)
   //
   // Since it won't trigger a black swan, no need to check here.

   // currently we don't do cross-market (triangle) matching.
   // the limit order will only match with a call order if meet all of these:
   // 1. it's buying collateral, which means sell_asset is the MIA, receive_asset is the backing asset.
   // 2. sell_asset is not a prediction market
   // 3. sell_asset is not globally settled
   // 4. sell_asset has a valid price feed
   // 5. the call order's collateral ratio is below or equals to MCR
   // 6. the limit order provided a good price

   bool to_check_call_orders = false;
   const asset_object& sell_asset = get_asset( sell_asset_symbol );
   const asset_bitasset_data_object& sell_abd = get_bitasset_data( sell_asset_symbol );
   price call_match_price;
   if( sell_asset.is_market_issued() )
   {
      if( sell_abd.backing_asset == recv_asset_symbol
          && !sell_abd.is_prediction_market
          && !sell_abd.has_settlement()
          && !sell_abd.current_feed.settlement_price.is_null() )
      {
         call_match_price = ~sell_abd.current_feed.max_short_squeeze_price();
         if( ~new_order_object.sell_price <= call_match_price ) // new limit order price is good enough to match a call
            to_check_call_orders = true;
      }
   }

   bool finished = false; // whether the new order is gone
   if( to_check_call_orders )
   {
      // check limit orders first, match the ones with better price in comparison to call orders
      while( !finished && limit_itr != limit_end && limit_itr->sell_price > call_match_price )
      {
         auto old_limit_itr = limit_itr;
         ++limit_itr;
         // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         finished = ( match( new_order_object, *old_limit_itr, old_limit_itr->sell_price ) != 2 );
      }

      if( !finished ) // check if there are margin calls
      {
         const auto& call_collateral_idx = get_index<call_order_index>().indices().get<by_collateral>();
         auto call_min = price::min( recv_asset_symbol, sell_asset_symbol );
         while( !finished )
         {
            // check call order with least collateral ratio
            auto call_itr = call_collateral_idx.lower_bound( call_min );
            if( call_itr == call_collateral_idx.end() || call_itr->debt_type() != sell_asset_symbol
                  // feed protected https://github.com/cryptonomex/graphene/issues/436
                  || call_itr->collateralization() > sell_abd.current_maintenance_collateralization )
               break;
            int match_result = match( new_order_object, *call_itr, call_match_price,
                                      sell_abd.current_feed.settlement_price,
                                      sell_abd.current_feed.maintenance_collateral_ratio,
                                      sell_abd.current_maintenance_collateralization );
            // match returns 1 or 3 when the new order was fully filled. In this case, we stop matching; otherwise keep matching.
            if( match_result == 1 || match_result == 3 )
               finished = true;
         }
      }
   }
   while( !finished && limit_itr != limit_end ) //check limit orders
   {
      auto old_limit_itr = limit_itr;
      ++limit_itr;
      // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
      finished = ( match( new_order_object, *old_limit_itr, old_limit_itr->sell_price ) != 2 );
   }

   const limit_order_object* updated_order_object = find< limit_order_object >( order_id );
   if( updated_order_object == nullptr )
      return true;

   // before #555 we would have done maybe_cull_small_order() logic as a result of fill_order() being called by match() above
   // however after #555 we need to get rid of small orders -- #555 hardfork defers logic that was done too eagerly before, and
   // this is the point it's deferred to.
   return maybe_cull_small_order( *updated_order_object );
}

/**
 *  Matches the two orders, the first parameter is taker, the second is maker.
 *  @return a bit field indicating which orders were filled (and thus removed)
 *  0 - no orders were matched
 *  1 - taker was filled
 *  2 - maker was filled
 *  3 - both were filled
 */
int database::match( const limit_order_object& taker, const limit_order_object& maker, const price& match_price )
{
   FC_ASSERT( taker.sell_price.quote.symbol == maker.sell_price.base.symbol );
   FC_ASSERT( taker.sell_price.base.symbol  == maker.sell_price.quote.symbol );
   FC_ASSERT( taker.for_sale > 0 && maker.for_sale > 0 );

   auto taker_for_sale = taker.amount_for_sale();
   auto maker_for_sale = maker.amount_for_sale();

   asset taker_pays, taker_receives, maker_pays, maker_receives;

   bool cull_taker = false;
   if( taker_for_sale <= maker_for_sale * match_price ) // rounding down here should be fine
   {
      taker_receives  = taker_for_sale * match_price; // round down, in favor of bigger order

      // Be here, it's possible that taker is paying something for nothing due to partially filled in last loop.
      // In this case, we see it as filled and cancel it later
      if( taker_receives.amount == 0 )
         return 1;

      // The remaining amount in order `taker` would be too small,
      //   so we should cull the order in fill_limit_order() below.
      // The order would receive 0 even at `match_price`, so it would receive 0 at its own price,
      //   so calling maybe_cull_small() will always cull it.
      maker_receives = taker_receives.multiply_and_round_up( match_price );
      cull_taker = true;
   }
   else
   {
      // The maker won't be paying something for nothing, since if it would, it would have been cancelled already.
      maker_receives = maker_for_sale * match_price; // round down, in favor of bigger order
      
      // The remaining amount in order `maker` would be too small,
      //   so the order will be culled in fill_limit_order() below
      taker_receives = maker_receives.multiply_and_round_up( match_price );
   }

   maker_pays = taker_receives;
   taker_pays = maker_receives;

   int result = 0;
   result |= fill_limit_order( taker, taker_pays, taker_receives, cull_taker, match_price, false ); // the first param is taker
   result |= fill_limit_order( maker, maker_pays, maker_receives, true, match_price, true ) << 1; // the second param is maker
   FC_ASSERT( result != 0 );
   return result;
}

int database::match( const limit_order_object& bid, const call_order_object& ask, const price& match_price,
                     const price& feed_price, const uint16_t maintenance_collateral_ratio,
                     const optional<price>& maintenance_collateralization )
{
   FC_ASSERT( bid.sell_asset() == ask.debt_type() );
   FC_ASSERT( bid.receive_asset() == ask.collateral_type() );
   FC_ASSERT( bid.for_sale > 0 && ask.debt.amount > 0 && ask.collateral.amount > 0 );

   bool cull_taker = false;

   asset taker_for_sale = bid.amount_for_sale();
   asset taker_to_buy = asset( ask.get_max_debt_to_cover( match_price, feed_price, maintenance_collateral_ratio, maintenance_collateralization ), ask.debt_type() );
   asset call_pays, call_receives, order_pays, order_receives;

   if( taker_to_buy > taker_for_sale ) // fill limit order
   {  
      order_receives  = taker_for_sale * match_price; // round down here, in favor of call order

      if( order_receives.amount == 0  )
         return 1;

      call_receives = order_receives.multiply_and_round_up( match_price );
      cull_taker = true;
   }
   else // fill call order
   {  
      call_receives = taker_to_buy;
      order_receives = taker_to_buy.multiply_and_round_up( match_price ); // round up here, in favor of limit order
   }

   call_pays = order_receives;
   order_pays = call_receives;

   int result = 0;
   result |= fill_limit_order( bid, order_pays, order_receives, cull_taker, match_price, false ); // the limit order is taker
   result |= fill_call_order( ask, call_pays, call_receives, match_price, true ) << 1;      // the call order is maker
   // result can be 0 when call order has target_collateral_ratio option set.

   return result;
}

asset database::match( const call_order_object& call, const force_settlement_object& settle, const price& match_price, asset max_settlement, const price& fill_price )
{ try {
   FC_ASSERT(call.get_debt().symbol == settle.balance.symbol );
   FC_ASSERT(call.debt.amount > 0 && call.collateral.amount > 0 && settle.balance.amount > 0);

   auto settle_for_sale = std::min(settle.balance, max_settlement);
   auto call_debt = call.get_debt();

   asset call_receives  = std::min(settle_for_sale, call_debt);
   asset call_pays = call_receives * match_price; // round down here, in favor of call order, for first check

   // Be here, the call order may be paying nothing.
   bool cull_settle_order = false; // whether need to cancel dust settle order
   if( call_pays.amount == 0 )
   {
      if( call_receives == call_debt ) // the call order is smaller than or equal to the settle order
      {
         wlog( "Something for nothing issue (#184, variant C-1) handled at block #${block}", ("block",head_block_num()) );
         call_pays.amount = 1;
      }
      else
      {
         if( call_receives == settle.balance ) // the settle order is smaller
         {
            wlog( "Something for nothing issue (#184, variant C-2) handled at block #${block}", ("block",head_block_num()) );
            cancel_settle_order( settle, true );
         }
         // else do nothing: neither order will be completely filled, perhaps due to max_settlement too small

         return asset( 0, settle.balance.symbol );
      }
   }
   else // the call order is not paying nothing, but still possible it's paying more than minimum required due to rounding
   {
      if( call_receives == call_debt ) // the call order is smaller than or equal to the settle order
      {
         call_pays = call_receives.multiply_and_round_up( match_price ); // round up here, in favor of settle order
         // be here, we should have: call_pays <= call_collateral
      }
      else
      {
         // be here, call_pays has been rounded down
         // be here, we should have: call_pays <= call_collateral

         if( call_receives == settle.balance ) // the settle order will be completely filled, assuming we need to cull it
            cull_settle_order = true;
         // else do nothing, since we can't cull the settle order

         call_receives = call_pays.multiply_and_round_up( match_price ); 

         if( call_receives == settle.balance ) // the settle order will be completely filled, no need to cull
            cull_settle_order = false;
         // else do nothing, since we still need to cull the settle order or still can't cull the settle order
      }
   }

   asset settle_pays = call_receives;
   asset settle_receives = call_pays;
   /**
    *  If the least collateralized call position lacks sufficient
    *  collateral to cover at the match price then this indicates a black 
    *  swan event according to the price feed, but only the market 
    *  can trigger a black swan. So now we must cancel the forced settlement
    *  object.
    */

   fill_call_order( call, call_pays, call_receives, fill_price, true ); // call order is maker
   fill_settle_order( settle, settle_pays, settle_receives, fill_price, false ); // force settlement order is taker

   if( cull_settle_order )
      cancel_settle_order( settle, true );

   return call_receives;
} FC_CAPTURE_AND_RETHROW( (call)(settle)(match_price)(max_settlement) ) }

bool database::fill_limit_order( const limit_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
                           const price& fill_price, const bool is_maker )
{ try {
   FC_ASSERT( order.amount_for_sale().symbol == pays.symbol );
   FC_ASSERT( pays.symbol != receives.symbol );

   const account_object& seller = get_account(order.seller);
   const asset_object& recv_asset = get_asset(receives.symbol);
   const auto& fee_asset_dyn_data = get_dynamic_data(receives.symbol);

   auto issuer_fees = pay_issuer_fees(seller, recv_asset, receives); // fees paid to issuer of the asset

   auto trading_fees = pay_trading_fees(seller, recv_asset, receives); // fees paid to the protocol, and interfaces

   auto fees_paid = issuer_fees + trading_fees;

   asset delta = receives - fees_paid;

   modify( fee_asset_dyn_data, [&](asset_dynamic_data_object& addo) {
      addo.adjust_pending_supply( -delta );
   });

   adjust_liquid_balance( seller, delta );

   push_virtual_operation( fill_order_operation( order.id, order.seller, pays, receives, issuer_fees, fill_price, is_maker ) );

   if( pays == order.amount_for_sale() )
   {
      remove( order );
      return true;
   }
   else
   {
      modify( order, [&]( limit_order_object& b ) {
         b.for_sale -= pays.amount; 
      });

      if( cull_if_small )
         return maybe_cull_small_order( order );
      return false;
   }
} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) ) }


bool database::fill_call_order( const call_order_object& order, const asset& pays, const asset& receives, const price& fill_price, const bool is_maker )
{ try {
   FC_ASSERT( order.debt_type() == receives.symbol );
   FC_ASSERT( order.collateral_type() == pays.symbol );
   FC_ASSERT( order.collateral.amount >= pays.amount );

   const asset_object& mia = get_asset(receives.symbol);
   FC_ASSERT( mia.is_market_issued() );

   optional<asset> collateral_freed;

   modify( order, [&]( call_order_object& o )
         {
            o.debt -= receives.amount;
            o.collateral -= pays.amount;
            if( o.debt.amount == 0 )
            {
              collateral_freed = o.get_collateral();
              o.collateral.amount = 0;
            }
      });

   const asset_dynamic_data_object& mia_ddo = get_dynamic_data(receives.symbol);

   modify( mia_ddo, [&]( asset_dynamic_data_object& ao )
      { // update current supply
         ao.adjust_pending_supply(-receives.amount);
      });

   if( collateral_freed.valid() )
      adjust_liquid_balance( order.borrower, *collateral_freed ); // Adjust balance

   push_virtual_operation( fill_order_operation( order.id, order.borrower, pays, receives, asset(0, pays.symbol), fill_price, is_maker ) );

   if( collateral_freed.valid() )
      remove( order );

   return collateral_freed.valid();
} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) ) }

bool database::fill_settle_order( const force_settlement_object& settle, const asset& pays, const asset& receives, const price& fill_price, const bool is_maker )
{ try {
   const asset_object& rec_asset = get_asset(receives.symbol);
   const asset_dynamic_data_object& rec_ddo = get_dynamic_data(receives.symbol);
   bool filled = false;

   auto issuer_fees = pay_issuer_fees(rec_asset, receives);
   auto trading_fees = pay_trading_fees(rec_asset, receives);
   auto fees_paid = issuer_fees + trading_fees;

   if( pays < settle.balance )
   {
      modify(settle, [&](force_settlement_object& s) {
         s.balance -= pays;
      });
      filled = false;
   } else {
      filled = true;
   }

   asset delta = receives - fees_paid;

   adjust_liquid_balance(settle.owner, delta);

   modify( rec_ddo, [&]( asset_dynamic_data_object& ao )
      { // update current supply
         ao.adjust_pending_supply(-delta);
      });

   FC_ASSERT( pays.symbol != receives.symbol );
   push_virtual_operation( fill_order_operation( settle.id, settle.owner, pays, receives, fees_paid, fill_price, is_maker ) );

   if (filled)
      remove(settle);

   return filled;
} FC_CAPTURE_AND_RETHROW( (settle)(pays)(receives) ) }

/**
 *  Starting with the least collateralized orders, fill them if their
 *  call price is above the max(lowest bid,call_limit).
 *
 *  This method will return true if it filled a short or limit.
 *
 *  @param mia - the market issued asset that should be called.
 *  @param enable_black_swan - when adjusting collateral, triggering a black swan is invalid and will throw if enable_black_swan is not set to true.
 *  @param for_new_limit_order - true if this function is called when matching call orders with a new limit order
 *  @param bitasset_ptr - an optional pointer to the bitasset_data object of the asset
 *
 *  @return true if a margin call was executed.
 */
bool database::check_call_orders( const asset_object& mia, bool enable_black_swan, bool for_new_limit_order, const asset_bitasset_data_object* bitasset_ptr )
{ try {
   if( !mia.is_market_issued() )
   {
      return false;
   }

   const asset_bitasset_data_object& bitasset = get_bitasset_data( mia.symbol );

   if( check_for_blackswan( mia, enable_black_swan, &bitasset ) )
   {
      return false;
   }
   if( bitasset.is_prediction_market ) 
   {
      return false;
   }
   if( bitasset.current_feed.settlement_price.is_null() ) 
   {
      return false;
   }

   auto limit_index = get_index < limit_order_index >();
   const auto& limit_price_index = limit_index.indices().get<by_price>();

   auto max_price = price::max( mia.symbol, bitasset.backing_asset ); // looking for limit orders selling the most USD for the least CORE

   auto min_price = bitasset.current_feed.max_short_squeeze_price(); // stop when limit orders are selling too little USD for too much CORE

   // NOTE limit_price_index is sorted from greatest to least
   auto limit_itr = limit_price_index.lower_bound( max_price );
   auto limit_end = limit_price_index.upper_bound( min_price );

   if( limit_itr == limit_end ) 
   {
     return false; 
   }

   auto call_index = get_index<call_order_index>();
   const auto& call_price_index = call_index.indices().get<by_price>();
   const auto& call_collateral_index = call_index.indices().get<by_collateral>();

   auto call_min = price::min( bitasset.backing_asset, mia.symbol );
   auto call_max = price::max( bitasset.backing_asset, mia.symbol );

   auto call_price_itr = call_price_index.begin();
   auto call_price_end = call_price_itr;
   auto call_collateral_itr = call_collateral_index.begin();
   auto call_collateral_end = call_collateral_itr;
   
   call_collateral_itr = call_collateral_index.lower_bound( call_min );
   call_collateral_end = call_collateral_index.upper_bound( call_max );
   
   bool filled_limit = false;
   bool margin_called = false;

   auto head_time = head_block_time();
   auto head_num = head_block_num();

   while( !check_for_blackswan( mia, enable_black_swan, &bitasset ) && limit_itr != limit_end && ( call_collateral_itr != call_collateral_end ) )
   {
      bool filled_call = false;

      const call_order_object& call_order = *call_collateral_itr;

      if( ( bitasset.current_maintenance_collateralization < call_order.collateralization() )) 
      {
         return margin_called;
      }
         
      const limit_order_object& limit_order = *limit_itr;
      price match_price  = limit_order.sell_price;
      
      margin_called = true;

      auto usd_to_buy = call_order.get_debt();
      if( usd_to_buy * match_price > call_order.get_collateral() )
      {
         elog( "black swan detected on asset ${symbol} (${id}) at block ${b}", ("id",mia.symbol)("symbol",mia.symbol)("b",head_num) );
         edump((enable_black_swan));
         FC_ASSERT( enable_black_swan );
         globally_settle_asset(mia, bitasset.current_feed.settlement_price );
         return true;
      }

      usd_to_buy.amount = call_order.get_max_debt_to_cover( match_price, bitasset.current_feed.settlement_price, bitasset.current_feed.maintenance_collateral_ratio, bitasset.current_maintenance_collateralization );
      
      asset usd_for_sale = limit_order.amount_for_sale();
      asset call_pays, call_receives, order_pays, order_receives;
      if( usd_to_buy > usd_for_sale ) // fill order
      {  
         order_receives = usd_for_sale * match_price; // round down, in favor of call order
         call_receives = order_receives.multiply_and_round_up( match_price );
         filled_limit = true;
      } 
      else // fill call
      { 
         call_receives  = usd_to_buy;
         order_receives = usd_to_buy.multiply_and_round_up( match_price ); // round up, in favor of limit order
         filled_call    = true; 

         if( usd_to_buy == usd_for_sale ) {
            filled_limit = true;
         }
      }

      call_pays  = order_receives;
      order_pays = call_receives;

      fill_call_order( call_order, call_pays, call_receives, match_price, for_new_limit_order );
      
      call_collateral_itr = call_collateral_index.lower_bound( call_min );

      auto next_limit_itr = std::next( limit_itr );
      // when for_new_limit_order is true, the limit order is taker, otherwise the limit order is maker
      bool really_filled = fill_limit_order( limit_order, order_pays, order_receives, true, match_price, !for_new_limit_order );
      if( really_filled ) 
      {
         limit_itr = next_limit_itr;
      }
   } // while call_itr != call_end

   return margin_called;
} FC_CAPTURE_AND_RETHROW() }


asset database::calculate_issuer_fee( const asset_object& trade_asset, const asset& trade_amount )
{
   FC_ASSERT( trade_asset.symbol == trade_amount.symbol );

   if( !trade_asset.charges_market_fees() )
      return asset(0, trade_asset.symbol);
   if( trade_asset.options.market_fee_percent == 0 )
      return asset(0, trade_asset.symbol);

   share_type value = (( trade_amount.amount * trade_asset.options.market_fee_percent ) / PERCENT_100  );
   asset percent_fee = asset( value, trade_asset.symbol );

   if( percent_fee.amount > trade_asset.options.max_market_fee )
      percent_fee.amount = trade_asset.options.max_market_fee;

   return percent_fee;
}

asset database::pay_issuer_fees( const asset_object& recv_asset, const asset& receives )
{
   asset issuer_fees = calculate_issuer_fee( recv_asset, receives );
   FC_ASSERT( issuer_fees <= receives, "Market fee shouldn't be greater than receives");

   //Don't dirty undo state if not actually collecting any fees
   if( issuer_fees.amount > 0 )
   {
      const asset_dynamic_data_object& recv_dyn_data = get_dynamic_data(recv_asset.symbol);
      modify( recv_dyn_data, [&]( asset_dynamic_data_object& addo )
      {
         addo.adjust_accumulated_fees( issuer_fees.amount );
      });
   }

   return issuer_fees;
}

asset database::pay_issuer_fees(const account_object& seller, const asset_object& recv_asset, const asset& receives )
{
   const auto issuer_fees = calculate_issuer_fee( recv_asset, receives );
   FC_ASSERT( issuer_fees <= receives, "Market fee shouldn't be greater than receives");
   //Don't dirty undo state if not actually collecting any fees
   if ( issuer_fees.amount > 0 ) 
   {
      asset reward = asset(0, recv_asset.symbol);

      const auto reward_percent = recv_asset.options.market_fee_share_percent; 
      if ( reward_percent > 0 ) // calculate and pay rewards
      {
         const account_object& registrar_account = get_account( seller.registrar );
         const account_object& referrer_account = get_account( seller.referrer );
         const account_permission_object& registrar_permissions = get_account_permissions(seller.registrar);
         const account_permission_object& referrer_permissions = get_account_permissions(seller.referrer);

         const auto reward_value = ( issuer_fees.amount * reward_percent) / PERCENT_100;
         if ( reward_value > 0 && registrar_permissions.is_authorized_asset( recv_asset) )
         {
            asset reward = asset( reward_value, recv_asset.symbol);
            FC_ASSERT( reward < issuer_fees, "Market reward should be less than issuer fees");
            // cut referrer percent from reward
            auto registrar_reward = reward;
            if( seller.referrer != seller.registrar )
            {
               const auto referrer_rewards_value = ( reward.amount * seller.referrer_rewards_percentage ) / PERCENT_100;

               if ( referrer_rewards_value > 0 && referrer_permissions.is_authorized_asset( recv_asset) )
               {
                  FC_ASSERT ( referrer_rewards_value <= reward.amount.value, "Referrer reward shouldn't be greater than total reward" );
                  const asset referrer_reward = asset( referrer_rewards_value, recv_asset.symbol);
                  registrar_reward -= referrer_reward;
                  adjust_reward_balance(seller.referrer, referrer_reward);
               }
            }
            adjust_reward_balance(seller.registrar, registrar_reward);
         }
      }

      const asset_dynamic_data_object& recv_dyn_data = get_dynamic_data(recv_asset.symbol);
      modify( recv_dyn_data, [&]( asset_dynamic_data_object& obj )
      {
         obj.adjust_accumulated_fees(issuer_fees.amount - reward.amount);
      });
   }

   return issuer_fees;
}

// Pays the network fee by burning the core asset into accumulated network revenue.
asset database::pay_network_fees( const asset& amount, const account_object& payer )
{
   FC_ASSERT(amount.symbol == SYMBOL_COIN);

   const asset_dynamic_data_object& core_dyn_data = get_dynamic_data( SYMBOL_COIN );

   modify( core_dyn_data, [&]( asset_dynamic_data_object& obj )
   {
      obj.burn_asset(-amount);
   });

   const dynamic_global_property_object& gprops = get_dynamic_global_properties();

   modify( gprops, [&](dynamic_global_property_object& gpo ) 
   {
      gpo.accumulated_network_revenue += amount;
   });
}

// Pays protocol membership fees .
// member: The account that is paying to upgrade to a membership level
// payment: The asset being received as payment
// duration: The number of months being paid for
// type: the level of membership purchased [standard,mid,top]
// seller_int: The owner account of the interface that sold the membership
asset database::pay_membership_fees( const account_object& member, const asset& payment, int16_t duration, const account_name_type& seller_int ) 
{
   FC_ASSERT( payment.symbol == SYMBOL_COIN, "Payment asset must be core asset");
   const asset_bitasset_data_object& USD_bitasset = get_bitasset_data( SYMBOL_USD );
   price USD_price = USD_bitasset.current_feed.settlement_price;



   asset payment_USD_value = payment * USD_price;


   asset total_fees = ( receives * TRADING_FEE_PERCENT ) / PERCENT_100;
   asset governance_account_share = ( total_fees * GOVERNANCE_SHARE_PERCENT ) / PERCENT_100;
   asset referral_share = ( total_fees * REFERRAL_SHARE_PERCENT ) / PERCENT_100;

   asset fee_share = total_fees - governance_account_share - referral_share; // subtracts referral and governance account fee shares before trading share percentages. 

   asset maker_interface_share = ( fee_share * MAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset taker_interface_share = ( fee_share * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset network_fee = ( fee_share * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;

   asset network_paid = pay_network_fees( network_fee, taker );
   asset gov_paid = pay_governance_reward( governance_account_share, taker );
   asset ref_paid = pay_referral_reward( referral_share, taker );
   asset maker_paid = pay_interface_reward( maker_int, maker_interface_share );
   asset taker_paid = pay_interface_reward( taker_int, taker_interface_share );

   FC_ASSERT( network_paid + gov_paid + ref_paid + maker_paid + taker_paid == total_fees, "Trading fee components are not equal to sum");
   return total_fees;
}

// Pays protocol trading fees on taker orders.
// taker: The account that is the taker on the trade
// recv_asset: The asset being received from the filling of the order
// receives: The asset object being received from the trade
// maker_int: The owner account of the interface of the maker of the trade
// taker_int: The owner account of the interface of the taker of the trade
asset database::pay_trading_fees( const account_object& taker, const asset_object& recv_asset, 
   const asset& receives, const account_name_type& maker_int, const account_name_type& taker_int ) 
{
   FC_ASSERT( receives.symbol == recv_asset.symbol, "Asset symbol for receiving asset and asset object are not equal");

   asset total_fees = ( receives * TRADING_FEE_PERCENT ) / PERCENT_100;
   asset governance_account_share = ( total_fees * GOVERNANCE_SHARE_PERCENT ) / PERCENT_100;
   asset referral_share = ( total_fees * REFERRAL_SHARE_PERCENT ) / PERCENT_100;

   asset fee_share = total_fees - governance_account_share - referral_share; // subtracts referral and governance account fee shares before trading share percentages. 

   asset maker_interface_share = ( fee_share * MAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset taker_interface_share = ( fee_share * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset network_fee = ( fee_share * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;

   asset network_paid = pay_network_fees( network_fee, taker );
   asset gov_paid = pay_governance_reward( governance_account_share, taker );
   asset ref_paid = pay_referral_reward( referral_share, taker );
   asset maker_paid = pay_interface_reward( maker_int, maker_interface_share );
   asset taker_paid = pay_interface_reward( taker_int, taker_interface_share );

   FC_ASSERT( network_paid + gov_paid + ref_paid + maker_paid + taker_paid == total_fees, "Trading fee components are not equal to sum");
   return total_fees;
}



/**
 *  let HB = the highest bid for the collateral (aka who will pay the most DEBT for the least collateral)
 *  let SP = current median feed's Settlement Price 
 *  let LC = the least collateralized call order's swan price (debt/collateral)
 *  If there is no valid price feed or no bids then there is no black swan.
 *  A black swan occurs if MAX(HB,SP) <= LC
 */
bool database::check_for_blackswan( const asset_object& mia, bool enable_black_swan, const asset_bitasset_data_object* bitasset_ptr )
{
   if( !mia.is_market_issued() ) return false;

   const asset_bitasset_data_object& bitasset = ( bitasset_ptr ? *bitasset_ptr : get_bitasset_data(mia.symbol));
   if( bitasset.has_settlement() ) return true; // already force settled
   auto settle_price = bitasset.current_feed.settlement_price;
   if( settle_price.is_null() ) return false; // no feed

   const call_order_object* call_ptr = nullptr; // place holder for the call order with least collateral ratio

   asset_symbol_type debt_asset_symbol = mia.symbol;
   auto call_min = price::min( bitasset.backing_asset, debt_asset_symbol );

   const auto& call_collateral_index = get_index<call_order_index>().indices().get<by_collateral>();
   auto call_itr = call_collateral_index.lower_bound( call_min );
   if( call_itr == call_collateral_index.end() ) // no call order
      return false;
   call_ptr = &(*call_itr);
    
    if( call_ptr->debt_type() != debt_asset_symbol ) // no call order
      return false;

   price highest = settle_price;
   
   highest = bitasset.current_feed.max_short_squeeze_price();

   const auto& limit_index = get_index<limit_order_index>();
   const auto& limit_price_index = limit_index.indices().get<by_price>();

   // looking for limit orders selling the most USD for the least CORE
   auto highest_possible_bid = price::max( mia.symbol, bitasset.backing_asset );
   // stop when limit orders are selling too little USD for too much CORE
   auto lowest_possible_bid  = price::min( mia.symbol, bitasset.backing_asset );

   FC_ASSERT( highest_possible_bid.base.symbol == lowest_possible_bid.base.symbol );
   // NOTE limit_price_index is sorted from greatest to least
   auto limit_itr = limit_price_index.lower_bound( highest_possible_bid );
   auto limit_end = limit_price_index.upper_bound( lowest_possible_bid );

   if( limit_itr != limit_end ) {
      FC_ASSERT( highest.base.symbol == limit_itr->sell_price.base.symbol );
      highest = std::max( limit_itr->sell_price, highest );
   }

   auto least_collateral = call_ptr->collateralization();
   if( ~least_collateral >= highest  ) 
   {
      wdump( (*call_ptr) );
      elog( "Black Swan detected on asset ${symbol} (${id}) at block ${b}: \n"
            "   Least collateralized call: ${lc}  ${~lc}\n"
         //  "   Highest Bid:               ${hb}  ${~hb}\n"
            "   Settle Price:              ${~sp}  ${sp}\n"
            "   Max:                       ${~h}  ${h}\n",
         ("id",mia.id)("symbol",mia.symbol)("b",head_block_num())
         ("lc",least_collateral.to_real())("~lc",(~least_collateral).to_real())
         //  ("hb",limit_itr->sell_price.to_real())("~hb",(~limit_itr->sell_price).to_real())
         ("sp",settle_price.to_real())("~sp",(~settle_price).to_real())
         ("h",highest.to_real())("~h",(~highest).to_real()) );
      edump((enable_black_swan));

      FC_ASSERT( enable_black_swan, "Black swan was detected during a margin update which is not allowed to trigger a blackswan" );
      if( ~least_collateral <= settle_price )
         // global settle at feed price if possible
         globally_settle_asset(mia, settle_price );
      else
         globally_settle_asset(mia, ~least_collateral );
      return true;
   } 
   return false;
}

void database::clear_expired_orders()
{ try {
         //Cancel expired limit orders
         auto head_time = head_block_time();
         auto maint_time = get_dynamic_global_properties().next_maintenance_time;

         auto& limit_index = get_index<limit_order_index>().indices().get<by_expiration>();
         while( !limit_index.empty() && limit_index.begin()->expiration <= head_time )
         {
            const limit_order_object& order = *limit_index.begin();
            auto base_asset = order.sell_price.base.symbol;
            auto quote_asset = order.sell_price.quote.symbol;
            cancel_limit_order( order );
         }

   //Process expired force settlement orders
   auto& settlement_index = get_index<force_settlement_index>().indices().get<by_expiration>();
   if( !settlement_index.empty() )
   {
      asset_symbol_type current_asset = settlement_index.begin()->settlement_asset_symbol();
      asset max_settlement_volume;
      price settlement_fill_price;
      price settlement_price;
      bool current_asset_finished = false;
      bool extra_dump = false;

      auto next_asset = [&current_asset, &current_asset_finished, &settlement_index, &extra_dump] {
         auto bound = settlement_index.upper_bound(current_asset);
         if( bound == settlement_index.end() )
         {
            if( extra_dump )
            {
               ilog( "next_asset() returning false" );
            }
            return false;
         }
         if( extra_dump )
         {
            ilog( "next_asset returning true, bound is ${b}", ("b", *bound) );
         }
         current_asset = bound->settlement_asset_symbol();
         current_asset_finished = false;
         return true;
      };

      uint32_t count = 0;

      // At each iteration, we either consume the current order and remove it, or we move to the next asset
      for( auto itr = settlement_index.lower_bound(current_asset);
           itr != settlement_index.end();
           itr = settlement_index.lower_bound(current_asset) )
      {
         ++count;
         const force_settlement_object& order = *itr;
         auto order_id = order.id;
         current_asset = order.settlement_asset_symbol();
         const asset_object& mia_object = get_asset(current_asset);
         const asset_bitasset_data_object& mia_bitasset = get_bitasset_data(mia_object.symbol);

         extra_dump = ((count >= 1000) && (count <= 1020));

         if( extra_dump )
         {
            wlog( "clear_expired_orders() dumping extra data for iteration ${c}", ("c", count) );
            ilog( "head_block_num is ${hb} current_asset is ${a}", ("hb", head_block_num())("a", current_asset) );
         }

         if( mia_bitasset.has_settlement() )
         {
            ilog( "Canceling a force settlement because of black swan" );
            cancel_settle_order( order, true );
            continue;
         }

         // Has this order not reached its settlement date?
         if( order.settlement_date > head_time )
         {
            if( next_asset() )
            {
               if( extra_dump )
               {
                  ilog( "next_asset() returned true when order.settlement_date > head_block_time()" );
               }
               continue;
            }
            break;
         }
         // Can we still settle in this asset?
         if( mia_bitasset.current_feed.settlement_price.is_null() )
         {
            ilog("Canceling a force settlement in ${asset} because settlement price is null",
                 ("asset", mia_object.symbol));
            cancel_settle_order(order, true);
            continue;
         }
         
         if( max_settlement_volume.symbol != current_asset ) {  // only calculate once per asset
            const asset_dynamic_data_object& dyn_data = get_dynamic_data( mia_object.symbol );
            max_settlement_volume = asset( mia_bitasset.max_force_settlement_volume(dyn_data.total_supply), mia_object.symbol );
         }

         if( mia_bitasset.force_settled_volume >= max_settlement_volume.amount || current_asset_finished )
         {
            if( next_asset() )
            {
               if( extra_dump )
               {
                  ilog( "next_asset() returned true when mia.force_settled_volume >= max_settlement_volume.amount" );
               }
               continue;
            }
            break;
         }

         if( settlement_fill_price.base.symbol != current_asset )  // only calculate once per asset
         {
            bitasset_options options = *mia_bitasset.options;
            uint16_t offset = options.force_settlement_offset_percent;
            settlement_fill_price = mia_bitasset.current_feed.settlement_price / ratio_type( PERCENT_100 - offset, PERCENT_100 );
         }
            
         if( settlement_price.base.symbol != current_asset )  // only calculate once per asset
         {
            settlement_price = settlement_fill_price;
         }

         auto& call_index = get_index<call_order_index>().indices().get<by_collateral>();
         asset settled = asset( mia_bitasset.force_settled_volume , mia_object.symbol);
         // Match against the least collateralized short until the settlement is finished or we reach max settlements
         while( settled < max_settlement_volume && find(order_id) )
         {
            auto itr = call_index.lower_bound(boost::make_tuple( price::min( mia_bitasset.backing_asset, mia_object.symbol )));
            // There should always be a call order, since asset exists!
            FC_ASSERT(itr != call_index.end() && itr->debt_type() == mia_object.symbol);
            asset max_settlement = max_settlement_volume - settled;

            if( order.balance.amount == 0 )
            {
               wlog( "0 settlement detected" );
               cancel_settle_order( order, true );
               break;
            }
            asset new_settled = match(*itr, order, settlement_price, max_settlement, settlement_fill_price);
            if( new_settled.amount == 0 ) // unable to fill this settle order
            {
               if( find( order_id ) ) // the settle order hasn't been cancelled
                  current_asset_finished = true;
               break;
            }
            settled += new_settled;
         }
         if( mia_bitasset.force_settled_volume != settled.amount )
         {
            modify(mia_bitasset, [settled](asset_bitasset_data_object& b) {
               b.force_settled_volume = settled.amount;
            });
         }
      }
   }
} FC_CAPTURE_AND_RETHROW() }

void database::update_expired_feeds()
{
   const auto head_time = head_block_time();
   const auto next_maint_time = get_dynamic_global_properties().next_maintenance_time;

   const auto& idx = get_index<asset_bitasset_data_index>().indices().get<by_feed_expiration>();
   auto itr = idx.begin();
   while( itr != idx.end() && itr->feed_is_expired( head_time ) ) // update feeds, check margin calls for each asset whose feed is expired
   {
      const asset_bitasset_data_object& bitasset = *itr;
      ++itr; 
      bool update_cer = false; 
      const asset_object* asset_ptr = nullptr;
      
      auto old_median_feed = bitasset.current_feed;
      modify( bitasset, [head_time,next_maint_time,&update_cer]( asset_bitasset_data_object& abdo )
      {
         abdo.update_median_feeds( head_time, next_maint_time );
         if( abdo.need_to_update_cer() )
         {
            update_cer = true;
            abdo.asset_cer_updated = false;
            abdo.feed_cer_updated = false;
         }
      });

      if( !bitasset.current_feed.settlement_price.is_null() && !( bitasset.current_feed == old_median_feed ) ) // `==` check is safe here
      {
         asset_ptr = find_asset(bitasset.symbol);
         check_call_orders( *asset_ptr, true, false, &bitasset );
      }
      
      if( update_cer ) // update CER
      {
         if( !asset_ptr )
            asset_ptr = find_asset(bitasset.symbol);
         if( asset_ptr->options.core_exchange_rate != bitasset.current_feed.core_exchange_rate )
         {
            modify( *asset_ptr, [&bitasset]( asset_object& ao )
            {
               ao.options.core_exchange_rate = bitasset.current_feed.core_exchange_rate;
            });
         }
      }
   } 
}

void database::update_core_exchange_rates()
{
   const auto& idx = get_index<asset_bitasset_data_index>().indices().get<by_cer_update>();
   if( idx.begin() != idx.end() )
   {
      for( auto itr = idx.rbegin(); itr->need_to_update_cer(); itr = idx.rbegin() )
      {
         const asset_bitasset_data_object& bitasset = *itr;
         const asset_object& asset = get_asset ( bitasset.symbol );
         if( asset.options.core_exchange_rate != bitasset.current_feed.core_exchange_rate )
         {
            modify( asset, [&bitasset]( asset_object& ao )
            {
               ao.options.core_exchange_rate = bitasset.current_feed.core_exchange_rate;
            });
         }
         modify( bitasset, []( asset_bitasset_data_object& abdo )
         {
            abdo.asset_cer_updated = false;
            abdo.feed_cer_updated = false;
         });
      }
   }
}

void database::update_maintenance_flag( bool new_maintenance_flag )
{
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   modify(props, [&]( dynamic_global_property_object& dpo )
   {
      auto maintenance_flag = dynamic_global_property_object::maintenance_flag;
      dpo.dynamic_flags =
           (dpo.dynamic_flags & ~maintenance_flag)
         | (new_maintenance_flag ? maintenance_flag : 0);
   } );
   return;
}

/*
void database::update_withdraw_permissions()
{
   auto& permit_index = get_index<withdraw_permission_index>().indices().get<by_expiration>();
   while( !permit_index.empty() && permit_index.begin()->expiration <= head_block_time() )
      remove(*permit_index.begin());
}

void database::clear_expired_htlcs()
{
   const auto& htlc_idx = get_index<htlc_index>().indices().get<by_expiration>();
   while ( htlc_idx.begin() != htlc_idx.end()
         && htlc_idx.begin()->conditions.time_lock.expiration <= head_block_time() )
   {
      const htlc_object& obj = *htlc_idx.begin();
      adjust_balance( obj.transfer.from, asset(obj.transfer.amount, obj.transfer.asset_id) );
      // virtual op
      htlc_refund_operation vop( obj.id, obj.transfer.from );
      vop.htlc_id = htlc_idx.begin()->id;
      push_applied_operation( vop );

      // remove the db object
      remove( *htlc_idx.begin() );
   }
}
*/



} } //node::chain