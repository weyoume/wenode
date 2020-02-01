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
#include <node/producer/producer_objects.hpp>

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
   const auto& unstake_idx = get_index< account_balance_index >().indices().get< by_next_unstake_time >();
   const auto& stake_idx = get_index< account_balance_index >().indices().get< by_next_stake_time >();
   const auto& didx = get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   
   auto unstake_itr = unstake_idx.begin();

   while( unstake_itr != unstake_idx.end() && unstake_itr->next_unstake_time <= now )
   {
      const account_balance_object& from_account_balance = *unstake_itr; 
      ++unstake_itr;

      share_type to_unstake;

      if ( from_account_balance.to_unstake - from_account_balance.total_unstaked < from_account_balance.unstake_rate )
      {
         to_unstake = std::min( from_account_balance.staked_balance, from_account_balance.to_unstake % from_account_balance.unstake_rate);
      }
      else
      {
         to_unstake = std::min( from_account_balance.staked_balance, from_account_balance.unstake_rate );
      }
         
      share_type total_restake = 0;
      share_type total_withdrawn = 0;

      asset unstake_asset = asset( to_unstake, from_account_balance.symbol );

      adjust_staked_balance( from_account_balance.owner, -unstake_asset );
      
      for( auto itr = didx.lower_bound( from_account_balance.owner ); itr != didx.end() && itr->from == from_account_balance.owner; ++itr )
      {
         if( itr->auto_stake )
         {
            share_type to_restake = (( to_unstake * itr->percent ) / PERCENT_100 );
            total_restake += to_restake;

            if( to_restake > 0 )
            {
               adjust_staked_balance( itr->to, to_restake );
            }
         }
         else
         {
            share_type to_withdraw = (( to_unstake * itr->percent ) / PERCENT_100 );
            total_withdrawn += to_withdraw;
            asset withdraw_asset = asset( to_withdraw, from_account_balance.symbol );

            if( to_withdraw > 0 )
            {
               adjust_liquid_balance( itr->to, withdraw_asset );
            }
         }
      }

      asset remaining_unstake = asset( to_unstake - total_restake - total_withdrawn, from_account_balance.symbol);
      adjust_liquid_balance( from_account_balance.owner, remaining_unstake );
         
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
            abo.next_unstake_time += STAKE_WITHDRAW_INTERVAL;
         }
      });
   }

   auto stake_itr = stake_idx.begin();

   while( stake_itr != stake_idx.end() && stake_itr->next_stake_time <= now )
   {
      const account_balance_object& from_account_balance = *stake_itr; 
      ++stake_itr;

      share_type to_stake;

      if ( from_account_balance.to_stake - from_account_balance.total_staked < from_account_balance.stake_rate )
      {
         to_stake = std::min( from_account_balance.staked_balance, from_account_balance.to_stake % from_account_balance.stake_rate);
      }
      else
      {
         to_stake = std::min( from_account_balance.staked_balance, from_account_balance.stake_rate );
      }
         
      share_type total_vested = 0;

      asset stake_asset = asset( to_stake, from_account_balance.symbol );

      adjust_staked_balance( from_account_balance.owner, -stake_asset );

      for( auto itr = didx.lower_bound( from_account_balance.owner ); itr != didx.end() && itr->from == from_account_balance.owner; ++itr )
      {
         share_type to_vest = (( to_stake * itr->percent ) / PERCENT_100 );
         total_vested += to_vest;
         asset vest_asset = asset( to_vest, from_account_balance.symbol );

         if( to_vest > 0 )
         {
            adjust_staked_balance( itr->to, vest_asset );
         }
      }

      asset remaining_stake = asset( to_stake - total_vested, from_account_balance.symbol);
      adjust_staked_balance( from_account_balance.owner, remaining_stake );
         
      modify( from_account_balance, [&]( account_balance_object& abo )
      {
         abo.total_staked += to_stake;

         if( abo.total_staked >= abo.to_stake || abo.liquid_balance == 0 )
         {
            abo.stake_rate = 0;
            abo.next_stake_time = fc::time_point::maximum();
         }
         else
         {
            abo.next_stake_time += STAKE_WITHDRAW_INTERVAL;
         }
      });
   }
}

void database::process_recurring_transfers()
{
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& transfer_idx = get_index< transfer_recurring_index >().indices().get< by_next_transfer >();
   auto transfer_itr = transfer_idx.begin();

   while( transfer_itr != transfer_idx.end() && transfer_itr->next_transfer <= now )
   {
      const transfer_recurring_object& transfer = *transfer_itr;
      asset liquid = get_liquid_balance( transfer.from, transfer.amount.symbol );

      if( liquid >= transfer.amount )    // Account has sufficient funds to pay
      {
         adjust_liquid_balance( transfer.from, -transfer.amount );
         adjust_liquid_balance( transfer.to, transfer.amount );

         modify( transfer, [&]( transfer_recurring_object& tro )
         {
            tro.next_transfer += tro.interval;
            tro.payments_remaining--;
         });

         if( transfer.payments_remaining == 0 )
         {
            remove( transfer );
         }
      }
      else     // Account cannot make the payment
      {
         if( transfer.fill_or_kill )     // Fill or kill causes transfer to be cancelled if payment cannot be made.
         {
            remove( transfer );
         }
         else if( transfer.extensible )   // Extensible recurring transfer is extended if a payment is missed.
         {
            modify( transfer, [&]( transfer_recurring_object& tro )
            {
               tro.next_transfer += tro.interval;
               tro.end += tro.interval;
            });
         }
         else if( transfer.payments_remaining > 1 )    // Payments are remaining, not extensible, so payment is not extended
         {
            modify( transfer, [&]( transfer_recurring_object& tro )
            {
               tro.next_transfer += tro.interval;
               tro.payments_remaining--;
            });
         }
         else     // No payments remaining
         {
            remove( transfer );
         }
      }
   }
}

void database::process_savings_withdraws()
{
   const auto& idx = get_index< savings_withdraw_index >().indices().get< by_complete_from_request_id >();
   auto itr = idx.begin();
   time_point now = head_block_time();

   while( itr != idx.end() )
   {
      if( itr->complete > now )
      {
         break;
      }
      
      adjust_liquid_balance( itr->to , itr->amount );

      modify( get_account( itr->from ), [&]( account_object& a )
      {
         a.savings_withdraw_requests--;
      });

      push_virtual_operation( 
         fill_transfer_from_savings_operation( 
         itr->from, 
         itr->to, 
         itr->amount, 
         to_string( itr->request_id ), 
         to_string( itr->memo )
         ) 
      );

      remove( *itr );
      itr = idx.begin();
   }
}


void database::process_escrow_transfers()
{
   const auto& escrow_acc_idx = get_index< escrow_index >().indices().get< by_acceptance_time >();
   auto escrow_acc_itr = escrow_acc_idx.lower_bound( false );
   time_point now = head_block_time();

   while( escrow_acc_itr != escrow_acc_idx.end() && 
      !escrow_acc_itr->is_approved() && 
      escrow_acc_itr->acceptance_time <= now )
   {
      const escrow_object& old_escrow = *escrow_acc_itr;
      ++escrow_acc_itr;

      release_escrow( old_escrow );
   }

   const auto& escrow_dis_idx = get_index< escrow_index >().indices().get< by_dispute_release_time >();
   auto escrow_dis_itr = escrow_dis_idx.lower_bound( true );

   while( escrow_dis_itr != escrow_dis_idx.end() && 
      escrow_dis_itr->disputed && 
      escrow_dis_itr->dispute_release_time <= now )
   {
      const escrow_object& old_escrow = *escrow_acc_itr;
      ++escrow_acc_itr;

      release_escrow( old_escrow );
   }
}

void database::update_median_liquidity()
{ try {
   if( (head_block_num() % MEDIAN_LIQUIDITY_INTERVAL_BLOCKS) != 0 )
      return;

   const auto& liq_idx = get_index< asset_liquidity_pool_index >().indices().get< by_asset_pair>();
   auto liq_itr = liq_idx.begin();

   size_t day_history_window = 1 + fc::days(1).to_seconds() / MEDIAN_LIQUIDITY_INTERVAL.to_seconds();
   size_t hour_history_window = 1 + fc::hours(1).to_seconds() / MEDIAN_LIQUIDITY_INTERVAL.to_seconds();

   while( liq_itr != liq_idx.end() )
   {
      const asset_liquidity_pool_object& pool = *liq_itr;
      vector<price> day; 
      vector<price> hour;
      day.reserve( day_history_window );
      hour.reserve( hour_history_window );

      bip::deque< price, allocator< price > > price_history = pool.price_history;

      modify( pool, [&]( asset_liquidity_pool_object& p )
      {
         p.price_history.push_back( pool.current_price() );
         if( p.price_history.size() > day_history_window )
         {
            price_history.pop_front();  // Maintain one day worth of price history
         }

         for( auto i : p.price_history )
         {
            day.push_back( i );
         }

         size_t offset = day.size()/2;

         std::nth_element( day.begin(), day.begin()+offset, day.end(),
         []( price a, price b )
         {
            return a < b;
         });

         p.day_median_price = day[ offset ];    // Set day median price to the median of all prices in the last day, at 10 min intervals

         auto hour_itr = p.price_history.rbegin();

         while( hour_itr != p.price_history.rend() && hour.size() < hour_history_window )
         {
            hour.push_back( *hour_itr );
            ++hour_itr;
         }

         size_t offset = hour.size()/2;

         std::nth_element( hour.begin(), hour.begin()+offset, hour.end(),
         []( price a, price b )
         {
            return a < b;
         });

         p.hour_median_price = hour[ offset ];   // Set hour median price to the median of all prices in the last hour, at 10 min intervals

      });

      ++liq_itr;
   } 
} FC_CAPTURE_AND_RETHROW() }


/**
 * Executes buyback orders to repurchase credit assets using
 * an asset's buyback pool of funds
 * up to the asset's buyback price, or face value.
 */         
void database::process_credit_buybacks()
{ try {
   if( (head_block_num() % CREDIT_BUYBACK_INTERVAL_BLOCKS) != 0 )
      return;

   const auto& credit_idx = get_index< asset_credit_data_index >().indices().get< by_symbol >();
   auto credit_itr = credit_idx.begin();

   while( credit_itr != credit_idx.end() )
   {
      const asset_credit_data_object& credit = *credit_itr;
      if( credit.buyback_pool.amount > 0 )
      {
         const asset_liquidity_pool_object& pool = get_liquidity_pool( credit.buyback_asset, credit.symbol );
         price buyback_price = credit.buyback_price;
         price market_price = pool.base_hour_median_price( buyback_price.base.symbol );
         if( market_price > buyback_price )
         {
            pair<asset, asset> buyback = liquid_limit_exchange( credit.buyback_pool, buyback_price, pool, true, true );
            modify( credit, [&]( asset_credit_data_object& c )
            {
               c.adjust_pool( -buyback.first );    // buyback the credit asset from its liquidity pool, up to the buyback price, and deduct from the pool.
            });
            adjust_pending_supply( buyback.second );
         }
      }
      ++credit_itr;
   }
} FC_CAPTURE_AND_RETHROW() }

/**
 * Pays accrued interest to all balance holders of credit assets,
 * according to the fixed and variable components of the asset's
 * interest options, and the current market price of the asset, 
 * relative to its target buyback face value price.
 * 
 * The interest rate increases when the the price of the credit 
 * asset falls, and decreases, when it is above buyback price.
 * 
 * When the price of the asset falls below the specified range
 * the variable interest rate goes to maximum value, and if it increases
 * above the specified range the variable interest rate falls to 0.
 */
void database::process_credit_interest()
{ try {
   if( (head_block_num() % CREDIT_INTERVAL_BLOCKS) != 0 )
   { 
      return; 
   }

   time_point now = head_block_time();
   const auto& credit_idx = get_index< asset_credit_data_index >().indices().get< by_symbol >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   auto credit_itr = credit_idx.begin();

   while( credit_itr != credit_idx.end() )
   {
      const asset_credit_data_object& credit = *credit_itr;
      asset_symbol_type cs = credit.symbol;
      const asset_dynamic_data_object& dyn_data = get_dynamic_data( cs );
      price buyback = credit.buyback_price;      // Base is buyback asset, Quote is Credit asset.
      price market = get_liquidity_pool( credit.buyback_asset, credit.symbol ).base_hour_median_price( credit.buyback_asset );
      
      asset unit = asset( BLOCKCHAIN_PRECISION, credit.buyback_asset );    // One unit of the Credit asset.
      share_type range = credit.var_interest_range;                        // Percentage range that caps the price divergence between market and buyback.
      share_type pr = PERCENT_100;
      share_type hpr = PERCENT_100 / 2;

      share_type buy = ( buyback * unit ).amount;       // Buyback price of the credit asset.
      share_type mar = ( market * unit ).amount;        // Market price of the credit asset.

      share_type liqf = credit.liquid_fixed_interest_rate;
      share_type staf = credit.staked_fixed_interest_rate;
      share_type savf = credit.savings_fixed_interest_rate;
      share_type liqv = credit.liquid_variable_interest_rate;
      share_type stav = credit.staked_variable_interest_rate;
      share_type savv = credit.savings_variable_interest_rate;

      share_type var_factor = ( ( -hpr * std::min( pr, std::max( -pr, pr * ( mar-buy ) / ( ( ( buy * range ) / pr ) ) ) ) ) / pr ) + hpr;

      share_type liq_ir = liqv * var_factor + liqf;
      share_type sta_ir = stav * var_factor + staf;
      share_type sav_ir = savv * var_factor + savf;

      // Applies interest rates that scale with the current market price / buyback price ratio, within a specified boundary range.

      asset total_liquid_interest = asset( 0, cs );
      asset total_staked_interest = asset( 0, cs );
      asset total_savings_interest = asset( 0, cs );

      auto balance_itr = balance_idx.lower_bound( cs );
      while( balance_itr != balance_idx.end() && balance_itr->symbol == cs )
      {
         const account_balance_object& balance = *balance_itr;

         asset liquid_interest = asset( ( ( ( balance.liquid_balance * liq_ir * ( now - balance.last_interest_time ).to_seconds() ) / fc::days(365).to_seconds() ) ) / pr , cs );
         asset staked_interest = asset( ( ( ( balance.staked_balance * sta_ir * ( now - balance.last_interest_time ).to_seconds() ) / fc::days(365).to_seconds() ) ) / pr , cs);
         asset savings_interest = asset( ( ( ( balance.savings_balance * sav_ir * ( now - balance.last_interest_time ).to_seconds() ) / fc::days(365).to_seconds() ) ) / pr , cs);

         total_liquid_interest += liquid_interest;
         total_staked_interest += staked_interest;
         total_savings_interest += savings_interest;

         modify( balance, [&]( account_balance_object& b )
         {
            b.adjust_liquid_balance( liquid_interest ); 
            b.adjust_staked_balance( staked_interest ); 
            b.adjust_savings_balance( savings_interest ); 
            b.last_interest_time = now;
         });
         
         ++balance_itr;
      }

      modify( dyn_data, [&]( asset_dynamic_data_object& d )
      {
         d.adjust_liquid_supply( total_liquid_interest ); 
         d.adjust_staked_supply( total_staked_interest ); 
         d.adjust_savings_supply( total_savings_interest ); 
      });

      ++credit_itr;
   }
} FC_CAPTURE_AND_RETHROW() }

/**
void database::update_median_feed() 
{ try {
   if( (head_block_num() % FEED_INTERVAL_BLOCKS) != 0 )
      return;

   auto now = head_block_time();
   const producer_schedule_object& pso = get_producer_schedule_object();
   vector<price> feeds; feeds.reserve( pso.num_scheduled_producers );
   for( int i = 0; i < pso.num_scheduled_producers; i++ )
   {
      const auto& wit = get_producer( pso.current_shuffled_producers[i] );
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

            std::sort( copy.begin(), copy.end() );
            fho.current_median_history = copy[copy.size()/2];
         }
      });
   }
} FC_CAPTURE_AND_RETHROW() }
*/

/**
 * Decrement an active asset delegation upon the expiration of
 * a delegation expiration object.
 * 
 * Withdrawing a delegation has a 24 hour time delay to ensure stake is not 
 * used to vote multiple times in rapid succession.
 */
void database::clear_expired_delegations()
{
   auto now = head_block_time();
   const auto& exp_idx = get_index< asset_delegation_expiration_index, by_expiration >();
   
   auto exp_itr = exp_idx.begin();

   while( exp_itr != exp_idx.end() && exp_itr->expiration < now )
   {
      const asset_delegation_expiration_object& exp = *exp_itr;
      ++exp_itr;
      const asset_delegation_object& delegation = get_asset_delegation( exp.delegator, exp.delegatee, exp.amount.symbol );

      adjust_delegated_balance( exp.delegator, -exp.amount );     // decrease delegated balance of delegator account.
      adjust_receiving_balance( exp.delegatee, -exp.amount );     // decrease receiving balance of delegatee account.
         
      push_virtual_operation( return_asset_delegation_operation( exp_itr->delegator, exp_itr->amount ) );

      remove( exp );
   }
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
         ("b", to_pretty_string( asset(0, delta.symbol)))
         ("r", to_pretty_string( -delta)));

      create<account_balance_object>( [&]( account_balance_object& abo) 
      {
         abo.owner = a;
         abo.symbol = delta.symbol;
         abo.staked_balance = delta.amount;
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
   
   modify( asset_dyn_data, [&](asset_dynamic_data_object& addo) 
   {
      addo.adjust_pending_supply( delta );
   });
   
} FC_CAPTURE_AND_RETHROW( (delta) ) }

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

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of SYMBOL_EQUITY and SYMBOL_COIN.
 */
share_type database::get_voting_power( const account_object& a )const
{
   return get_voting_power(a.name);
}

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of SYMBOL_EQUITY and SYMBOL_COIN.
 */
share_type database::get_voting_power( const account_name_type& a )const
{
   const account_balance_object* coin_ptr = find_account_balance( a, SYMBOL_COIN );
   const account_balance_object* equity_ptr = find_account_balance( a, SYMBOL_EQUITY );
   price equity_coin_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_EQUITY ).hour_median_price;
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

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of SYMBOL_EQUITY and SYMBOL_COIN.
 */
share_type database::get_voting_power( const account_object& a, const price& equity_coin_price )const
{
   return get_voting_power( a.name, equity_coin_price );
}

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of SYMBOL_EQUITY and SYMBOL_COIN.
 */
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
      voting_power += get_proxied_voting_power( name, equity_price );    // Recursively finds voting power of proxies
   }
   return voting_power;

} FC_CAPTURE_AND_RETHROW() }


share_type database::get_proxied_voting_power( const account_name_type& a, const price& equity_price )const
{ try {
   return get_proxied_voting_power( get_account(a), equity_price);
} FC_CAPTURE_AND_RETHROW() }


share_type database::get_equity_voting_power( const account_object& a, const account_business_object& b )const
{ try {
   return get_equity_voting_power( a.name, b);
} FC_CAPTURE_AND_RETHROW() }


share_type database::get_equity_voting_power( const account_name_type& a, const account_business_object& b )const
{ try {
   share_type voting_power = 0;
   for( auto symbol : b.equity_assets )
   {
      const asset_equity_data_object& equity = get_equity_data( symbol );
      const account_balance_object* abo_ptr = find_account_balance( a, symbol );
      if( abo_ptr != nullptr )
      {
         voting_power += abo_ptr->get_liquid_balance() * equity.liquid_voting_rights;
         voting_power += abo_ptr->get_voting_power() * equity.staked_voting_rights;
         voting_power += abo_ptr->get_savings_balance() * equity.savings_voting_rights;
      }
   }
   return voting_power;
} FC_CAPTURE_AND_RETHROW() }


string database::to_pretty_string( const asset& a )const
{
   return a.amount_to_pretty_string(a);
}


void database::update_expired_feeds()
{ try {
   const auto head_time = head_block_time();

   const auto& idx = get_index< asset_bitasset_data_index >().indices().get< by_feed_expiration >();
   auto itr = idx.begin();
   while( itr != idx.end() && itr->feed_is_expired( head_time ) ) // update feeds, check margin calls for each asset whose feed is expired
   {
      const asset_bitasset_data_object& bitasset = *itr;
      ++itr; 
      const asset_object* asset_ptr = nullptr;
      
      auto old_median_feed = bitasset.current_feed;
      modify( bitasset, [&]( asset_bitasset_data_object& abdo )
      {
         abdo.update_median_feeds( head_time );
      });

      if( !bitasset.current_feed.settlement_price.is_null() && !( bitasset.current_feed == old_median_feed ) ) // `==` check is safe here
      {
         asset_ptr = find_asset(bitasset.symbol);
         check_call_orders( *asset_ptr, true, false );
      }
   } 
} FC_CAPTURE_AND_RETHROW() }


void database::process_bids( const asset_bitasset_data_object& bad )
{ try {
   if( bad.current_feed.settlement_price.is_null() )       // Asset must have settlement price
   {
      return;
   } 

   const asset_object& to_revive = get_asset( bad.symbol ); 
   const asset_dynamic_data_object& bdd = get_dynamic_data( bad.symbol );

   const auto& bid_idx = get_index< collateral_bid_index >().indices().get< by_price >();
   const auto start = bid_idx.lower_bound( boost::make_tuple( bad.symbol, price::max( bad.backing_asset, bad.symbol ) ) );

   share_type covered = 0;

   auto bid_itr = start;
   // Count existing bids and determine if enough bids have been made to 
   // Revive asset, entire supply should be covered.

   while( covered < bdd.total_supply && 
      bid_itr != bid_idx.end() && 
      bid_itr->debt.symbol == bad.symbol )
   {
      const collateral_bid_object& bid = *bid_itr;

      asset debt_in_bid = bid.debt;

      if( debt_in_bid.amount > bdd.total_supply )
      {
         debt_in_bid.amount = bdd.total_supply;
      }
         
      asset total_collateral = debt_in_bid * bad.settlement_price;

      total_collateral += bid.collateral;

      price call_price = price( debt_in_bid, total_collateral );

      if( ~call_price >= bad.current_feed.settlement_price )
      {
         break;
      }

      covered += debt_in_bid.amount;
      ++bid_itr;
   }

   if( covered < bdd.total_supply ) 
   {
      return;   // Supply not yet covered
   }
   else
   {
      const auto end = bid_itr;
      share_type to_cover = bdd.total_supply;
      share_type remaining_fund = bad.settlement_fund;
      for( bid_itr = start; bid_itr != end; )
      {
         const collateral_bid_object& bid = *bid_itr;
         ++bid_itr;
         asset debt_in_bid = bid.debt;
         if( debt_in_bid.amount > bdd.total_supply )
         {
            debt_in_bid.amount = bdd.total_supply;
         }

         share_type debt = debt_in_bid.amount;
         share_type collateral = ( debt_in_bid * bad.settlement_price ).amount;

         if( debt >= to_cover )
         {
            debt = to_cover;
            collateral = remaining_fund;
         }
         to_cover -= debt;
         remaining_fund -= collateral;
         execute_bid( bid, debt, collateral, bad.current_feed );
      }

      FC_ASSERT( remaining_fund == 0, 
         "Settlement fund not completely allocated by bids." );
      FC_ASSERT( to_cover == 0,
         "Asset debt not completely covered by bids." );

      cancel_bids_and_revive_mpa( to_revive, bad );
   }
} FC_CAPTURE_AND_RETHROW() }


void database::dispute_escrow( const escrow_object& escrow )
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;

   const auto& mediator_idx = get_index< mediator_index >().indices().get< by_virtual_position >();
   auto mediator_itr = mediator_idx.begin();

   vector< account_name_type > top_mediators;
   vector< account_name_type > shuffled_mediators;
   flat_set< account_name_type > allocated_mediators;

   // Increment all mediators virtual position by their mediation stake balance.
   while( mediator_itr != mediator_idx.end() )
   {
      if( mediator_itr->active )
      {
         modify( *mediator_itr, [&]( mediator_object& m )
         {
            m.mediation_virtual_position += m.mediator_bond.amount.value;
         });
      }
      ++mediator_itr;
   }

   auto mediator_itr = mediator_idx.begin();

   // Select top position mediators
   while( mediator_itr != mediator_idx.end() && top_mediators.size() < ( 10 * ESCROW_DISPUTE_MEDIATOR_AMOUNT ) )
   {
      if( mediator_itr->active )
      {
         top_mediators.push_back( mediator_itr->account );
      }
      ++mediator_itr;
   }

   shuffled_mediators = shuffle_accounts( top_mediators );   // Get a random ordered vector of mediator names

   for( auto i = 0; i < ESCROW_DISPUTE_MEDIATOR_AMOUNT; i++ )
   {
      allocated_mediators.insert( shuffled_mediators[ i ] );

      const mediator_object& mediator = get_mediator( shuffled_mediators[ i ] );
      modify( mediator, [&]( mediator_object& m )
      {
         m.mediation_virtual_position = 0;
         m.last_escrow_id = escrow.escrow_id;
      });
   }

   modify( escrow, [&]( escrow_object& esc )
   {
      esc.disputed = true;
      esc.mediators = allocated_mediators;
      esc.last_updated = now;
      esc.dispute_release_time = now + ESCROW_DISPUTE_DURATION;
   });

} FC_CAPTURE_AND_RETHROW() }

/**
 * Selects the median release percentage in the escrow
 * and divides the payment between the TO and FROM accounts.
 * Fofeits security bonds based on the difference between median
 * and individual votes to create an incentive to reach 
 * cooperative consensus between the mediators about the
 * escrow details. 
 * All voters receive a split of all forfeited bonds, distributing funds
 * as a net profit to accounts the voted closest to the median.
 */
void database::release_escrow( const escrow_object& escrow )
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   asset escrow_bond = asset( ( escrow.payment.amount * median_props.escrow_bond_percent ) / PERCENT_100, escrow.payment.symbol );

   if( escrow.is_approved() )
   {
      vector< uint16_t > release_percentages;
      uint16_t median_release = PERCENT_100;

      for( auto p : escrow.release_percentages )
      {
         release_percentages.push_back( p.second );
      }

      size_t offset = release_percentages.size()/2;

      std::nth_element( release_percentages.begin(), release_percentages.begin()+offset, release_percentages.end(),
      []( uint16_t a, uint16_t b )
      {
         return a < b;
      });

      median_release = release_percentages[ offset ];

      asset to_share = ( escrow.payment * median_release ) / PERCENT_100;
      asset from_share = escrow.payment - to_share;
      asset balance = escrow.balance;

      adjust_liquid_balance( escrow.to, to_share );
      balance -= to_share;
      adjust_liquid_balance( escrow.from, from_share );
      balance -= from_share;

      if( escrow.disputed )
      {
         for( auto a : escrow.release_percentages )     // Refund escrow bonds, minus loss from vote differential
         {
            int16_t delta = abs( median_release - a.second );
            asset escrow_bond_return = asset( ( escrow_bond.amount * ( PERCENT_100 - delta ) ) / PERCENT_100, escrow_bond.symbol );
            balance -= escrow_bond_return;
            adjust_liquid_balance( a.first, escrow_bond_return );
         }
         asset escrow_split = asset( balance.amount / escrow.release_percentages.size(), balance.symbol );
         for( auto a : escrow.release_percentages )     // Distribute remaining balance evenly between voters
         {
            balance -= escrow_split;
            adjust_liquid_balance( a.first, escrow_split );
         }
      }
      else
      {
         for( auto a : escrow.approvals )     // Refund escrow bonds to all approving accounts
         {
            if( a.second == true )
            {
               balance -= escrow_bond;
               adjust_liquid_balance( a.first, escrow_bond );
            }
         }
      }

      adjust_liquid_balance( escrow.from, balance );   // Return remaining balance to FROM. 
   }
   else      // Escrow is being released before being approved, all accounts refunded
   {
      for( auto a : escrow.approvals )
      {
         if( a.second == true )
         {
            if( a.first == escrow.from )
            {
               adjust_liquid_balance( a.first, escrow.payment + escrow_bond );
            }
            else
            {
               adjust_liquid_balance( a.first, escrow_bond );
            }
         }
      }
   }

   adjust_pending_supply( -escrow.balance );
   remove( escrow );

} FC_CAPTURE_AND_RETHROW() }

} } //node::chain