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


void database::process_asset_staking()
{
   const auto& unstake_idx = get_index< account_balance_index >().indices().get< by_next_unstake_time >();
   const auto& stake_idx = get_index< account_balance_index >().indices().get< by_next_stake_time >();
   const auto& vesting_idx = get_index< account_vesting_balance_index >().indices().get< by_vesting_time >();
   const auto& didx = get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   
   auto unstake_itr = unstake_idx.begin();

   while( unstake_itr != unstake_idx.end() && 
      unstake_itr->next_unstake_time <= now )
   {
      const account_balance_object& from_account_balance = *unstake_itr;
      
      ++unstake_itr;

      share_type to_unstake = 0;

      if( from_account_balance.to_unstake - from_account_balance.total_unstaked < from_account_balance.unstake_rate )
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

      asset remaining_unstake = asset( to_unstake - total_restake - total_withdrawn, from_account_balance.symbol );
      
      adjust_liquid_balance( from_account_balance.owner, remaining_unstake );
      
      modify( from_account_balance, [&]( account_balance_object& abo )
      {
         abo.total_unstaked += to_unstake;
         if( abo.total_unstaked >= abo.to_unstake || abo.staked_balance == 0 )
         {
            abo.unstake_rate = 0;
            abo.to_unstake = 0;
            abo.total_unstaked = 0;
            abo.next_unstake_time = fc::time_point::maximum();
         }
         else
         {
            abo.next_unstake_time += STAKE_WITHDRAW_INTERVAL;
         }
      });

      ilog( "Processed Asset Unstaking: ${s} - ${f}",
         ("s",unstake_asset.to_string())("f",from_account_balance.owner));
   }

   auto stake_itr = stake_idx.begin();

   while( stake_itr != stake_idx.end() && 
      stake_itr->next_stake_time <= now )
   {
      const account_balance_object& from_account_balance = *stake_itr;
      ++stake_itr;

      share_type to_stake = 0;

      if( from_account_balance.to_stake - from_account_balance.total_staked < from_account_balance.stake_rate )
      {
         to_stake = std::min( from_account_balance.liquid_balance, from_account_balance.to_stake % from_account_balance.stake_rate );
      }
      else
      {
         to_stake = std::min( from_account_balance.liquid_balance, from_account_balance.stake_rate );
      }

      asset stake_asset = asset( to_stake, from_account_balance.symbol );

      adjust_liquid_balance( from_account_balance.owner, -stake_asset );
      adjust_staked_balance( from_account_balance.owner, stake_asset );
 
      modify( from_account_balance, [&]( account_balance_object& abo )
      {
         abo.total_staked += to_stake;
         if( abo.total_staked >= abo.to_stake || abo.liquid_balance == 0 )
         {
            abo.stake_rate = 0;
            abo.to_stake = 0;
            abo.total_staked = 0;
            abo.next_stake_time = fc::time_point::maximum();
         }
         else
         {
            abo.next_stake_time += STAKE_WITHDRAW_INTERVAL;
         }
      });

      ilog( "Processed Asset Staking: ${s} : ${f}",
         ("s",stake_asset.to_string())("f",from_account_balance.owner));
   }

   auto vesting_itr = vesting_idx.begin();

   while( vesting_itr != vesting_idx.end() && 
      vesting_itr->vesting_time <= now )
   {
      const account_vesting_balance_object& vesting_balance = *vesting_itr;
      ++vesting_itr;

      adjust_liquid_balance( vesting_balance.owner, vesting_balance.get_vesting_balance() );
      adjust_pending_supply( -vesting_balance.get_vesting_balance() );
      ilog( "Removed: ${v}",("v",vesting_balance));
      remove( vesting_balance );
   }
}

void database::process_recurring_transfers()
{
   // ilog( "Process Recurring Transfers" );

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& transfer_idx = get_index< transfer_recurring_index >().indices().get< by_next_transfer >();
   auto transfer_itr = transfer_idx.begin();

   while( transfer_itr != transfer_idx.end() &&
      transfer_itr->next_transfer <= now )
   {
      const transfer_recurring_object& transfer = *transfer_itr;
      ++transfer_itr;
      asset liquid = get_liquid_balance( transfer.from, transfer.amount.symbol );

      if( liquid >= transfer.amount )    // Account has sufficient funds to pay
      {
         adjust_liquid_balance( transfer.from, -transfer.amount );
         adjust_liquid_balance( transfer.to, transfer.amount );

         modify( transfer, [&]( transfer_recurring_object& tro )
         {
            tro.next_transfer += tro.interval;
            tro.payments_remaining -= 1;
         });

         ilog( "Processed Recurring Transfer: \n ${t} \n",
            ("t",transfer));

         if( transfer.payments_remaining == 0 )
         {
            ilog( "Removed: ${v}",("v",transfer));
            remove( transfer );
         }
      }
      else     // Account cannot make the payment
      {
         if( transfer.fill_or_kill )     // Fill or kill causes transfer to be cancelled if payment cannot be made.
         {
            ilog( "Removed: ${v}",("v",transfer));
            remove( transfer );
         }
         else if( transfer.extensible )   // Extensible recurring transfer is extended if a payment is missed.
         {
            modify( transfer, [&]( transfer_recurring_object& tro )
            {
               tro.next_transfer += tro.interval;
               tro.end += tro.interval;
            });
            
            ilog( "Processed Recurring Transfer: \n ${t} \n",
               ("t",transfer));
         }
         else if( transfer.payments_remaining > 1 )    // Payments are remaining, not extensible, so payment is not extended
         {
            modify( transfer, [&]( transfer_recurring_object& tro )
            {
               tro.next_transfer += tro.interval;
               tro.payments_remaining -= 1;
            });

            ilog( "Processed Recurring Transfer: \n ${t} \n",
               ("t",transfer));
         }
         else     // No payments remaining
         {
            ilog( "Removed: ${v}",("v",transfer));
            remove( transfer );
         }
      }
   }
}

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


void database::process_escrow_transfers()
{
   // ilog( "Process Escrow Transfers" );

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
      const escrow_object& old_escrow = *escrow_dis_itr;
      ++escrow_dis_itr;

      release_escrow( old_escrow );
   }
}

void database::update_median_liquidity()
{ try {
   if( (head_block_num() % MEDIAN_LIQUIDITY_INTERVAL_BLOCKS) != 0 )
      return;

   // ilog( "Update Median Liquidity" );

   const auto& liq_idx = get_index< asset_liquidity_pool_index >().indices().get< by_asset_pair>();
   auto liq_itr = liq_idx.begin();

   size_t day_history_window = 1 + fc::days(1).to_seconds() / MEDIAN_LIQUIDITY_INTERVAL.to_seconds();
   size_t hour_history_window = 1 + fc::hours(1).to_seconds() / MEDIAN_LIQUIDITY_INTERVAL.to_seconds();

   while( liq_itr != liq_idx.end() )
   {
      const asset_liquidity_pool_object& pool = *liq_itr;
      ++liq_itr;
      vector< price > day; 
      vector< price > hour;
      day.reserve( day_history_window );
      hour.reserve( hour_history_window );

      modify( pool, [&]( asset_liquidity_pool_object& p )
      {
         p.price_history.push_back( pool.current_price() );
         
         while( p.price_history.size() > day_history_window )
         {
            p.price_history.pop_front();       // Maintain one day worth of price history
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

         offset = hour.size()/2;

         std::nth_element( hour.begin(), hour.begin()+offset, hour.end(),
         []( price a, price b )
         {
            return a < b;
         });

         p.hour_median_price = hour[ offset ];   // Set hour median price to the median of all prices in the last hour, at 10 min intervals

      });
   } 
} FC_CAPTURE_AND_RETHROW() }


/**
 * Executes Bond coupon interest payments.
 * 
 * All bond assets pay a coupon interest rate once 
 * per week from the issuing business account to all bondholders.
 */         
void database::process_bond_interest()
{ try {
   if( (head_block_num() % BOND_COUPON_INTERVAL_BLOCKS) != 0 )
      return;

   ilog( "Update Bond Interest" );

   const auto& bond_idx = get_index< asset_bond_data_index >().indices().get< by_symbol >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   auto bond_itr = bond_idx.begin();
   time_point now = head_block_time();

   while( bond_itr != bond_idx.end() )
   {
      const asset_bond_data_object& bond = *bond_itr;
      ++bond_itr;

      asset total_interest_fees = asset( 0, bond.value.symbol );
      asset total_interest = asset( 0, bond.value.symbol );
      auto balance_itr = balance_idx.lower_bound( bond.symbol );

      while( balance_itr != balance_idx.end() && 
         balance_itr->symbol == bond.symbol )
      {
         const account_balance_object& balance = *balance_itr;
         ++balance_itr;

         uint128_t interest_seconds = ( now - balance.last_interest_time ).to_seconds();
         uint128_t interest_amount = uint128_t( bond.value.amount.value ) * uint128_t( balance.get_total_balance().amount.value ) * uint128_t( bond.coupon_rate_percent ) * interest_seconds;
         interest_amount /= uint128_t( fc::days(365).to_seconds() * PERCENT_100 );
         asset interest = asset( share_type( interest_amount.to_uint64() ), bond.value.symbol );
         total_interest += interest;

         asset interest_fees = ( interest * INTEREST_FEE_PERCENT ) / PERCENT_100;
         total_interest_fees += interest_fees;
         
         adjust_liquid_balance( balance.owner, ( interest - interest_fees ) );

         modify( balance, [&]( account_balance_object& b )
         {
            b.last_interest_time = now;
         });
      }

      adjust_liquid_balance( bond.business_account, -total_interest );
      pay_network_fees( total_interest_fees );
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Check for and close all bonds which have reached their maturity date.
 */
void database::process_bond_assets()
{ try {
   // ilog( "Process Bond Assets" );

   time_point now = head_block_time();
   const auto& bond_idx = get_index< asset_bond_data_index >().indices().get< by_maturity >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   auto bond_itr = bond_idx.begin();
   
   while( bond_itr != bond_idx.end() &&
      now >= bond_itr->maturity() )
   {
      const asset_bond_data_object& bond = *bond_itr;
      auto balance_itr = balance_idx.lower_bound( bond.symbol );
      ++bond_itr;

      const asset_dynamic_data_object& bond_dyn_data = get_dynamic_data( bond.symbol );
      asset total_principle = ( bond.value * bond_dyn_data.get_total_supply().amount ) / BLOCKCHAIN_PRECISION;
      asset issuer_liquid = get_liquid_balance( bond.business_account, bond.value.symbol );
      asset principle_remaining = total_principle;

      if( issuer_liquid >= total_principle )
      {
         while( balance_itr != balance_idx.end() && 
            balance_itr->symbol == bond.symbol &&
            principle_remaining.amount > 0 )
         {
            const account_balance_object& balance = *balance_itr;
            asset principle = asset( balance.liquid_balance * bond.value.amount, bond.value.symbol );
            adjust_liquid_balance( balance.owner, principle );
            principle_remaining -= principle;
            ++balance_itr;
         }

         asset paid = total_principle - principle_remaining - bond.collateral_pool;
         adjust_liquid_balance( bond.business_account, -paid );
      }
      else
      {
         principle_remaining = issuer_liquid + bond.collateral_pool;
         asset partial_value = principle_remaining / bond_dyn_data.get_total_supply().amount;

         while( balance_itr != balance_idx.end() && 
            balance_itr->symbol == bond.symbol &&
            principle_remaining.amount > 0 )
         {
            const account_balance_object& balance = *balance_itr;
            asset principle = asset( balance.liquid_balance * partial_value.amount, bond.value.symbol );

            if( principle > principle_remaining )
            {
               principle = principle_remaining;
            }

            adjust_liquid_balance( balance.owner, principle );
            principle_remaining -= principle;
            ++balance_itr;
         }

         asset paid = issuer_liquid - principle_remaining;
         adjust_liquid_balance( bond.business_account, -paid );
      }

      clear_asset_balances( bond.symbol );      // Clear all balances and order positions of the bond.

      ilog( "Removed: ${v}",("v",bond));
      remove( bond );
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

   // ilog( "Process Credit Buybacks" );

   const auto& credit_idx = get_index< asset_credit_data_index >().indices().get< by_symbol >();
   auto credit_itr = credit_idx.begin();

   while( credit_itr != credit_idx.end() )
   {
      const asset_credit_data_object& credit = *credit_itr;
      ++credit_itr;

      if( credit.buyback_pool.amount > 0 )
      {
         const asset_liquidity_pool_object& pool = get_liquidity_pool( credit.buyback_asset, credit.symbol );
         price buyback_price = credit.buyback_price;
         price market_price = pool.base_hour_median_price( buyback_price.base.symbol );
         if( market_price > buyback_price )
         {
            pair< asset, asset > buyback = liquid_limit_exchange( credit.buyback_pool, buyback_price, pool, true, true );
            modify( credit, [&]( asset_credit_data_object& c )
            {
               c.adjust_pool( -buyback.first );    // buyback the credit asset from its liquidity pool, up to the buyback price, and deduct from the pool.
            });
            adjust_pending_supply( buyback.second );
         }
      }
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

   // ilog( "Process Credit Interest" );

   time_point now = head_block_time();
   const auto& credit_idx = get_index< asset_credit_data_index >().indices().get< by_symbol >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   auto credit_itr = credit_idx.begin();

   while( credit_itr != credit_idx.end() )
   {
      const asset_credit_data_object& credit = *credit_itr;
      ++credit_itr;

      asset_symbol_type cs = credit.symbol;
      const asset_dynamic_data_object& dyn_data = get_dynamic_data( cs );
      price buyback = credit.buyback_price;      // Base is buyback asset, Quote is Credit asset.
      price market = get_liquidity_pool( credit.buyback_asset, credit.symbol ).base_hour_median_price( credit.buyback_asset );
      
      asset unit = asset( BLOCKCHAIN_PRECISION, credit.buyback_asset );    // One unit of the Credit asset.
      share_type range = credit.var_interest_range;                        // Percentage range that caps the price divergence between market and buyback.
      share_type pr = PERCENT_100;
      share_type hpr = PERCENT_100 / 2;

      share_type buy = ( unit * buyback ).amount;       // Buyback price of the credit asset.
      share_type mar = ( unit * market ).amount;        // Market price of the credit asset.

      share_type liqf = credit.liquid_fixed_interest_rate;
      share_type staf = credit.staked_fixed_interest_rate;
      share_type savf = credit.savings_fixed_interest_rate;
      share_type liqv = credit.liquid_variable_interest_rate;
      share_type stav = credit.staked_variable_interest_rate;
      share_type savv = credit.savings_variable_interest_rate;

      share_type var_factor = ( ( -hpr * std::min( pr, std::max( -pr, pr * ( mar-buy ) / ( ( ( buy * range ) / pr ) ) ) ) ) / pr ) + hpr;

      uint128_t liq_ir = ( ( liqv * var_factor + liqf * pr ) / pr ).value;
      uint128_t sta_ir = ( ( stav * var_factor + staf * pr ) / pr ).value;
      uint128_t sav_ir = ( ( savv * var_factor + savf * pr ) / pr ).value;

      // Applies interest rates that scale with the current market price / buyback price ratio, within a specified boundary range.

      asset total_liquid_interest = asset( 0, cs );
      asset total_staked_interest = asset( 0, cs );
      asset total_savings_interest = asset( 0, cs );

      auto balance_itr = balance_idx.lower_bound( cs );

      while( balance_itr != balance_idx.end() && 
         balance_itr->symbol == cs )
      {
         const account_balance_object& balance = *balance_itr;
         int64_t elapsed_sec = ( now - balance.last_interest_time ).to_seconds();
         int64_t year_sec = fc::days(365).to_seconds();

         uint128_t liq_b = balance.liquid_balance.value;
         uint128_t sta_b = balance.staked_balance.value;
         uint128_t sav_b = balance.savings_balance.value;

         uint128_t liq_i = ( liq_b * liq_ir ) / uint128_t( PERCENT_100 );
         uint128_t sta_i = ( sta_b * sta_ir ) / uint128_t( PERCENT_100 );
         uint128_t sav_i = ( sav_b * sav_ir ) / uint128_t( PERCENT_100 );

         uint128_t liq_acc = ( liq_i * elapsed_sec ) / year_sec;
         uint128_t sta_acc = ( sta_i * elapsed_sec ) / year_sec;
         uint128_t sav_acc = ( sav_i * elapsed_sec ) / year_sec;

         asset liquid_interest = asset( int64_t( liq_acc.to_uint64() ), cs );
         asset staked_interest = asset( int64_t( sta_acc.to_uint64() ), cs );
         asset savings_interest = asset( int64_t( sav_acc.to_uint64() ), cs );

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

         ilog( "Account: ${a} Earned Credit interest: Seconds: ${sec} - Liquid: ${l} - Staked: ${staked} - Savings: ${s}",
            ("a",balance.owner)("sec",elapsed_sec)("l",liquid_interest.to_string())("staked",staked_interest.to_string())("s",savings_interest.to_string()));
      }

      modify( dyn_data, [&]( asset_dynamic_data_object& d )
      {
         d.adjust_liquid_supply( total_liquid_interest ); 
         d.adjust_staked_supply( total_staked_interest ); 
         d.adjust_savings_supply( total_savings_interest ); 
      });
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Expires all stimulus balances, and redistributes new balances
 * to the accounts within the distribution list.
 * Redeems stimulus assets for the equivalent amount of
 * redemption assets.
 */
void database::process_stimulus_assets()
{ try {
   if( (head_block_num() % STIMULUS_INTERVAL_BLOCKS ) != 0 )
   { 
      return; 
   }

   // ilog( "Process Stimulus Assets" );

   time_point now = head_block_time();
   const auto& stimulus_idx = get_index< asset_stimulus_data_index >().indices().get< by_expiration >();
   auto stimulus_itr = stimulus_idx.begin();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_owner_symbol >();
   auto balance_itr = balance_idx.begin();

   asset_symbol_type stimulus_symbol;

   while( stimulus_itr != stimulus_idx.end() &&
      now >= stimulus_itr->expiration() )
   {
      const asset_stimulus_data_object& stimulus = *stimulus_itr;
      ++stimulus_itr;

      asset total_redemption = asset( 0, stimulus.redemption_asset );
      asset balance_to_redeem = asset( 0, stimulus.symbol );
      price redemption_price = stimulus.redemption_price;
      date_type new_date;
      
      if( stimulus.next_distribution_date.month > 11 )
      {
         new_date = date_type( 1, 1, stimulus.next_distribution_date.year + 1 );
      }
      else
      {
         new_date = date_type( 1, stimulus.next_distribution_date.month + 1, stimulus.next_distribution_date.year );
      }

      modify( stimulus, [&]( asset_stimulus_data_object& asdo )
      {
         asdo.next_distribution_date = new_date;
      });

      for( account_name_type s : stimulus.redemption_list )     // Exchange all balances in redemption list for redemption pool assets
      {
         balance_itr = balance_idx.find( boost::make_tuple( s, stimulus.symbol ) );
         if( balance_itr != balance_idx.end() )
         {
            asset balance = balance_itr->get_total_balance();
            balance_to_redeem += balance;
         }
      }

      if( stimulus.redemption_pool < balance_to_redeem * stimulus.redemption_price )
      {
         redemption_price = price( stimulus.redemption_pool, balance_to_redeem );
      }

      for( account_name_type s : stimulus.redemption_list )     // Exchange all balances in redemption list for redemption pool assets
      {
         balance_itr = balance_idx.find( boost::make_tuple( s, stimulus.symbol ) );
         if( balance_itr != balance_idx.end() )
         {
            asset balance = balance_itr->get_total_balance();
            asset redemption = balance * stimulus.redemption_price;
            total_redemption += redemption;
            adjust_liquid_balance( s, redemption );
         }
      }

      modify( stimulus, [&]( asset_stimulus_data_object& asdo )
      {
         asdo.adjust_pool( -total_redemption );
      });

      adjust_pending_supply( -total_redemption );

      clear_asset_balances( stimulus.symbol );      // Clear all balances and order positions of the stimulus asset.

      if( stimulus.redemption_pool >= stimulus.amount_to_distribute() * stimulus.redemption_price )
      {
         for( account_name_type s : stimulus.distribution_list )
         {
            adjust_liquid_balance( s, stimulus.distribution_amount );
         }
      }
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
 * Expires all outstanding option orders, balances 
 * and resets option pool strike prices.
 * Adds in a new month of option assets by shifting 
 * the option strike forward by one year.
 */
void database::process_option_assets()
{ try {
   if( (head_block_num() % OPTION_INTERVAL_BLOCKS ) != 0 )
   { 
      return; 
   }

   // ilog( "Process Option Assets" );

   time_point now = head_block_time();
   const auto& option_order_idx = get_index< option_order_index >().indices().get< by_expiration >();
   auto option_order_itr = option_order_idx.begin();

   asset_symbol_type option_symbol;
   option_strike strike;
   price current_price;

   while( option_order_itr != option_order_idx.end() &&
      now >= option_order_itr->expiration() )
   {
      const option_order_object& order = *option_order_itr;
      ++option_order_itr;

      option_symbol = order.debt_type();
      strike = option_strike::from_string( option_symbol );
      const asset_option_pool_object& option_pool = get_option_pool( strike.strike_price.base.symbol, strike.strike_price.quote.symbol );
      const asset_liquidity_pool_object& liquidity_pool = get_liquidity_pool( strike.strike_price.base.symbol, strike.strike_price.quote.symbol );
      current_price = liquidity_pool.base_day_median_price( strike.strike_price.base.symbol );
      flat_set< asset_symbol_type > new_strikes;
      const asset_object& base_asset = get_asset( strike.strike_price.base.symbol );
      const asset_object& quote_asset = get_asset( strike.strike_price.quote.symbol );

      modify( option_pool, [&]( asset_option_pool_object& aopo )
      {
         aopo.expire_strike_prices( strike.expiration_date );
         date_type new_date = date_type( 1, strike.expiration_date.month, strike.expiration_date.year + 1 );
         new_strikes = aopo.add_strike_prices( current_price, new_date );
      });

      for( asset_symbol_type s : new_strikes )     // Create the new asset objects for the option.
      {
         create< asset_object >( [&]( asset_object& a )
         {
            option_strike strike = option_strike::from_string(s);

            a.symbol = s;
            a.asset_type = asset_property_type::OPTION_ASSET;
            a.issuer = NULL_ACCOUNT;
            from_string( a.display_symbol, strike.display_symbol() );

            from_string( 
               a.details, 
               strike.details( 
                  to_string( quote_asset.display_symbol ), 
                  to_string( quote_asset.details ), 
                  to_string( base_asset.display_symbol ), 
                  to_string( base_asset.details ) ) );
            
            from_string( a.json, "" );
            from_string( a.url, "" );
            a.max_supply = MAX_ASSET_SUPPLY;
            a.stake_intervals = 0;
            a.unstake_intervals = 0;
            a.market_fee_percent = 0;
            a.market_fee_share_percent = 0;
            a.issuer_permissions = 0;
            a.flags = 0;
            a.created = now;
            a.last_updated = now;
         });

         create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
         {
            a.symbol = s;
         });
      }

      clear_asset_balances( option_symbol );      // Clear all balances and order positions of the option.
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Check for and close all prediction pools which have reached their resolution time.
 */
void database::process_prediction_assets()
{ try {

   // ilog( "Process Prediction Assets" );

   time_point now = head_block_time();
   const auto& prediction_pool_idx = get_index< asset_prediction_pool_index >().indices().get< by_resolution_time >();
   auto prediction_pool_itr = prediction_pool_idx.begin();
   
   while( prediction_pool_itr != prediction_pool_idx.end() &&
      now >= prediction_pool_itr->resolution_time )
   {
      const asset_prediction_pool_object& prediction_pool = *prediction_pool_itr;
      ++prediction_pool_itr;

      close_prediction_pool( prediction_pool );
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Resolves a prediction pool.
 * 
 * - Finds the winning outcome according ot quadratic voting for resolutions.
 * - Pays the holders of the successful outcome asset.
 * - Pays the voters of the successful outcome.
 * - Wipes the blances of all holders of all outcome assets and prediciton assets.
 */
void database::close_prediction_pool( const asset_prediction_pool_object& pool )
{ try {
   ilog( "Closing Prediction Pool: \n ${p} \n",
      ("p",pool));

   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   const auto& resolution_idx = get_index< asset_prediction_pool_resolution_index >().indices().get< by_outcome_symbol >();
   auto balance_itr = balance_idx.begin();
   auto resolution_itr = resolution_idx.lower_bound( pool.prediction_symbol );
   asset_symbol_type top_outcome;
   uint128_t top_outcome_votes = 0;
   uint128_t top_outcome_shares = 0;
   flat_map< asset_symbol_type, uint128_t > resolution_votes;
   flat_map< asset_symbol_type, uint128_t > resolution_shares;

   for( asset_symbol_type s : pool.outcome_assets )
   {
      resolution_votes[ s ] = 0;
      resolution_shares[ s ] = 0;
   }

   // Determine the top outcome by accumulating resolution votes.

   while( resolution_itr != resolution_idx.end() &&
      resolution_itr->prediction_symbol == pool.prediction_symbol )
   {
      const asset_prediction_pool_resolution_object& resolution = *resolution_itr;
      ++resolution_itr;

      resolution_votes[ resolution.resolution_outcome ] += uint128_t( resolution.resolution_votes().value );
      resolution_shares[ resolution.resolution_outcome ] += uint128_t( resolution.amount.amount.value );

      if( resolution_votes[ resolution.resolution_outcome ] > top_outcome_votes )
      {
         top_outcome_votes = resolution_votes[ resolution.resolution_outcome ];
         top_outcome_shares = resolution_shares[ resolution.resolution_outcome ];
         top_outcome = resolution.resolution_outcome;
      }
   }

   asset prediction_bond_remaining = pool.prediction_bond_pool;
   asset collateral_remaining = pool.collateral_pool;
   resolution_itr = resolution_idx.lower_bound( boost::make_tuple( pool.prediction_symbol, top_outcome ) );
   asset pool_split = asset( 0, pool.collateral_symbol );

   // Distribute funds from the prediction bond pool to the resolvers of the top outcome.

   while( resolution_itr != resolution_idx.end() &&
      resolution_itr->prediction_symbol == pool.prediction_symbol &&
      resolution_itr->resolution_outcome == top_outcome &&
      prediction_bond_remaining.amount > 0 )
   {
      const asset_prediction_pool_resolution_object& resolution = *resolution_itr;
      ++resolution_itr;
      uint128_t split_amount = ( uint128_t( resolution.amount.amount.value ) * uint128_t( pool.prediction_bond_pool.amount.value ) ) / top_outcome_shares;
      pool_split = asset( share_type( split_amount.to_uint64() ), pool.collateral_symbol );
      
      if( pool_split > prediction_bond_remaining )
      {
         pool_split = prediction_bond_remaining;
      }

      ilog( "Account: ${a} received resolving amount: ${am}",
         ("a",resolution.account)("am",pool_split.to_string()));

      adjust_liquid_balance( resolution.account, pool_split );
      prediction_bond_remaining -= pool_split;
   }

   balance_itr = balance_idx.lower_bound( top_outcome );

   // Distribute funds from the collateral pool to the holders of the succesful outcome asset.

   while( balance_itr != balance_idx.end() &&
      balance_itr->symbol == top_outcome &&
      collateral_remaining.amount > 0 )
   {
      const account_balance_object& balance = *balance_itr;
      ++balance_itr;

      pool_split = asset( balance.get_total_balance().amount, pool.collateral_symbol );
      
      if( pool_split > collateral_remaining )
      {
         pool_split = collateral_remaining;
      }

      ilog( "Account: ${a} received prediction outcome amount: ${am}",
         ("a",balance.owner)("am",pool_split.to_string()));

      adjust_liquid_balance( balance.owner, pool_split );
      collateral_remaining -= pool_split;
   }

   // Clear all ballances and orders in the prediction pool base asset and outcome assets.

   clear_asset_balances( pool.prediction_symbol );    

   for( asset_symbol_type a : pool.outcome_assets )
   {
      clear_asset_balances( a );
   }

   ilog( "Removed: ${v}",("v",pool.prediction_symbol));
   remove( pool );
   
} FC_CAPTURE_AND_RETHROW() }


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


/**
 * Distributes funds from access list to ownership asset holders.
 */
void database::process_unique_assets()
{ try {
   if( (head_block_num() % UNIQUE_INTERVAL_BLOCKS ) != 0 )    // Runs once per day
      return;

   // ilog( "Process Unique Assets" );

   const auto& unique_idx = get_index< asset_unique_data_index >().indices().get< by_access_price >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol_stake >();
   auto unique_itr = unique_idx.begin();

   while( unique_itr != unique_idx.end() && 
      unique_itr->access_price_amount() > 0 )
   {
      const asset_unique_data_object& unique = *unique_itr;
      ++unique_itr;

      asset revenue_pool = asset( 0, unique.access_price.symbol );
      flat_set< account_name_type > access_list = unique.access_list;

      for( account_name_type a : access_list )     // Charge Daily access price.
      {
         asset liquid = get_liquid_balance( a, unique.access_price.symbol );

         if( liquid >= unique.access_price )
         {
            adjust_liquid_balance( a, -unique.access_price );
            revenue_pool += unique.access_price;
         }
         else
         {
            modify( unique, [&]( asset_unique_data_object& e )
            {
               e.access_list.erase( a );
            });
         }
      }

      auto balance_itr = balance_idx.lower_bound( unique.ownership_asset );
      flat_map< account_name_type, uint128_t > unique_map;
      uint128_t total_unique_shares = 0;
      
      asset remaining = revenue_pool;

      while( balance_itr != balance_idx.end() &&
         balance_itr->symbol == unique.ownership_asset &&
         balance_itr->staked_balance.value > 0 )
      {
         const account_balance_object& balance = *balance_itr;
         ++balance_itr;
         total_unique_shares += balance.staked_balance.value;
         unique_map[ balance_itr->owner ] = balance.staked_balance.value;
      }
      
      // Pay access fees to each ownership asset stakeholder proportionally.

      for( auto b : unique_map )
      {
         uint128_t reward_amount = ( revenue_pool.amount.value * b.second ) / total_unique_shares;
         asset unique_reward = asset( reward_amount.to_uint64(), unique.access_price.symbol );
         if( unique_reward > remaining )
         {
            unique_reward = remaining;
         }
         adjust_reward_balance( b.first, unique_reward );
         remaining -= unique_reward;
      }

      if( remaining.amount.value > 0 )
      {
         adjust_reward_balance( unique.controlling_owner, remaining );
      }
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Processes asset distribution rounds.
 * 
 * 1 - Checks for asset distributions that have reached their next round time.
 * 2 - Checks if they have reached their soft cap input amount.
 * 3 - Splits all incoming distribution balances to the component accounts of the input asset unit
 * 4 - Issues new assets to the component accounts of the output distribution asset unit.
 * 5 - Increments the counter of the intervals paid or missed.
 * 6 - Closes distributions that have been fully completed.
 */
void database::process_asset_distribution()
{ try {

   time_point now = head_block_time();
   const auto& distribution_idx = get_index< asset_distribution_index >().indices().get< by_next_round_time >();
   const auto& balance_idx = get_index< asset_distribution_balance_index >().indices().get< by_distribution_account >();
   auto distribution_itr = distribution_idx.begin();
   auto balance_itr = balance_idx.begin();

   while( distribution_itr != distribution_idx.end() &&
      now >= distribution_itr->next_round_time )
   {
      const asset_distribution_object& distribution = *distribution_itr;

      ilog( "Begin Processing Asset Distribution: ${d}",
         ("d",distribution.distribution_asset));
      
      asset total_input = asset( 0, distribution.fund_asset );
      share_type total_input_units = 0;
      share_type max_output_units = distribution.max_output_distribution_units();
      share_type input_unit_amount = distribution.input_unit_amount();
      asset total_distributed = asset( 0, distribution.distribution_asset );
      asset total_funded = asset( 0, distribution.fund_asset );
      
      balance_itr = balance_idx.lower_bound( distribution.distribution_asset );

      while( balance_itr != balance_idx.end() &&
         distribution.distribution_asset == balance_itr->distribution_asset )
      {
         const asset_distribution_balance_object& balance = *balance_itr;
         ++balance_itr;
         total_input_units += ( balance.amount.amount / input_unit_amount );
         total_input += balance.amount;
      }

      bool distributed = false;

      // Passed the minimum to distribute

      if( total_input_units >= distribution.min_input_fund_units )
      {
         flat_set< asset_unit > input_fund_unit = distribution.input_fund_unit;
         flat_set< asset_unit > output_distribution_unit = distribution.output_distribution_unit;
         share_type unit_ratio = max_output_units / total_input_units;

         if( unit_ratio > distribution.max_unit_ratio )
         {
            unit_ratio = distribution.max_unit_ratio;
         }
         else if( unit_ratio < distribution.min_unit_ratio )
         {
            unit_ratio = distribution.min_unit_ratio;
         }

         share_type input_units = 0;
         share_type output_units = 0;

         balance_itr = balance_idx.lower_bound( distribution.distribution_asset );

         while( balance_itr != balance_idx.end() &&
            distribution.distribution_asset == balance_itr->distribution_asset )
         {
            const asset_distribution_balance_object& balance = *balance_itr;
            ++balance_itr;

            input_units = balance.amount.amount / input_unit_amount;
            output_units = input_units * unit_ratio;
            
            account_name_type input_account;
            account_name_type output_account;
            account_balance_type balance_type = account_balance_type::LIQUID_BALANCE;

            for( asset_unit u : input_fund_unit )   // Pay incoming assets to the input asset unit accounts
            {
               input_account = u.name;
               asset input_asset = asset( input_units * u.units, distribution.fund_asset );

               balance_type = account_balance_type::LIQUID_BALANCE;

               for( size_t i = 0; i < account_balance_values.size(); i++ )
               {
                  if( u.balance_type == account_balance_values[ i ] )
                  {
                     balance_type = account_balance_type( i );
                     break;
                  }
               }

               if( u.name == ASSET_UNIT_SENDER )
               {
                  input_account = balance.sender;
               }

               switch( balance_type )
               {
                  case account_balance_type::LIQUID_BALANCE:
                  {
                     adjust_liquid_balance( input_account, input_asset );
                  }
                  break;
                  case account_balance_type::STAKED_BALANCE:
                  {
                     adjust_staked_balance( input_account, input_asset );
                  }
                  break;
                  case account_balance_type::REWARD_BALANCE:
                  {
                     adjust_reward_balance( input_account, input_asset );
                  }
                  break;
                  case account_balance_type::SAVINGS_BALANCE:
                  {
                     adjust_savings_balance( input_account, input_asset );
                  }
                  break;
                  case account_balance_type::VESTING_BALANCE:
                  {
                     create< account_vesting_balance_object >( [&]( account_vesting_balance_object& avbo )
                     {
                        avbo.owner = input_account;
                        avbo.symbol = distribution.fund_asset;
                        avbo.vesting_balance = input_units * u.units;
                        avbo.vesting_time = u.vesting_time;
                     });
                  }
                  break;
                  default:
                  {
                     break;
                  }
               }

               ilog( "Account: ${a} Received Distribution Input: ${i}",
                  ("a",input_account)("i",input_asset.to_string()) );

               total_funded += balance.amount;
            }

            for( asset_unit u : output_distribution_unit )     // Pay newly distributed assets to the asset output unit accounts
            {
               output_account = u.name;
               asset output_asset = asset( output_units * u.units, distribution.distribution_asset );

               if( u.name == ASSET_UNIT_SENDER )
               {
                  output_account = balance.sender;
               }

               balance_type = account_balance_type::LIQUID_BALANCE;

               for( size_t i = 0; i < account_balance_values.size(); i++ )
               {
                  if( u.balance_type == account_balance_values[ i ] )
                  {
                     balance_type = account_balance_type( i );
                     break;
                  }
               }

               switch( balance_type )
               {
                  case account_balance_type::LIQUID_BALANCE:
                  {
                     adjust_liquid_balance( output_account, output_asset );
                  }
                  break;
                  case account_balance_type::STAKED_BALANCE:
                  {
                     adjust_staked_balance( output_account, output_asset );
                  }
                  break;
                  case account_balance_type::REWARD_BALANCE:
                  {
                     adjust_reward_balance( output_account, output_asset );
                  }
                  break;
                  case account_balance_type::SAVINGS_BALANCE:
                  {
                     adjust_savings_balance( output_account, output_asset );
                  }
                  break;
                  case account_balance_type::VESTING_BALANCE:
                  {
                     create< account_vesting_balance_object >( [&]( account_vesting_balance_object& avbo )
                     {
                        avbo.owner = output_account;
                        avbo.symbol = distribution.distribution_asset;
                        avbo.vesting_balance = output_units * u.units;
                        avbo.vesting_time = u.vesting_time;
                     });
                     adjust_pending_supply( asset( input_units * u.units, distribution.fund_asset ) );
                  }
                  break;
                  default:
                  {
                     break;
                  }
               }

               ilog( "Account: ${a} Received Distribution Output: ${o}",
                  ("a",output_account)("o",output_asset.to_string()) );
               
               total_distributed += output_asset;
            }

            ilog( "Removed: ${v}",("v",balance));
            remove( balance );
         }

         distributed = true;
      }

      modify( distribution, [&]( asset_distribution_object& ado )
      {
         if( distributed )
         {
            ado.intervals_paid++;    // Increment days paid if the round was sucessfully distributed
         }
         else
         {
            ado.intervals_missed++;
         }
         
         ado.next_round_time += fc::days( ado.distribution_interval_days );
         ado.total_distributed += total_distributed;
         ado.total_funded += total_funded;
      });

      ++distribution_itr;

      ilog( "Processed Asset Distribution: \n ${d} \n",
         ("d",distribution));

      // If the distribution has reached its final round, or missed its final max missed amount, refund all outstanding balances.

      if( distribution.intervals_paid >= distribution.distribution_rounds &&
         distribution.intervals_missed >= distribution.max_intervals_missed )
      {
         balance_itr = balance_idx.lower_bound( distribution.distribution_asset );

         while( balance_itr != balance_idx.end() &&
            distribution.distribution_asset == balance_itr->distribution_asset )
         {
            const asset_distribution_balance_object& balance = *balance_itr;
            ++balance_itr;
            adjust_liquid_balance( balance.sender, balance.amount );
            adjust_pending_supply( -balance.amount );
            ilog( "Removed: ${v}",("v",balance));
            remove( balance );
         }
         ilog( "Removed: ${v}",("v",distribution));
         remove( distribution );
      }
   }
} FC_CAPTURE_AND_RETHROW() }


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
asset database::get_receiving_balance( const account_name_type& a, const asset_symbol_type& symbol)const
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
 * Determined by the staked and net delegated balance of SYMBOL_EQUITY and SYMBOL_COIN.
 */
share_type database::get_voting_power( const account_object& a )const
{
   return get_voting_power( a.name );
}

/**
 * Retrieves an account's voting power.
 * 
 * Determined by the staked and net delegated balance of SYMBOL_EQUITY and SYMBOL_COIN.
 */
share_type database::get_voting_power( const account_name_type& a )const
{
   price equity_coin_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_EQUITY ).hour_median_price;
   return get_voting_power( a, equity_coin_price );
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
   const account_balance_object* coin_ptr = find_account_balance( a, SYMBOL_COIN );
   const account_balance_object* equity_ptr = find_account_balance( a, SYMBOL_EQUITY );
   asset coin_vote = asset( 0, SYMBOL_COIN );
   asset equity_vote = asset( 0, SYMBOL_EQUITY );
   
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
      const asset_equity_data_object& equity = get_equity_data( symbol );
      const account_balance_object* abo_ptr = find_account_balance( a, symbol );

      if( abo_ptr != nullptr )
      {
         voting_power += abo_ptr->liquid_balance * share_type( equity.liquid_voting_rights );
         voting_power += abo_ptr->staked_balance * share_type( equity.staked_voting_rights );
         voting_power += abo_ptr->savings_balance * share_type( equity.savings_voting_rights );
      }
   }
   return voting_power;
} FC_CAPTURE_AND_RETHROW() }


string database::to_pretty_string( const asset& a )const
{
   return a.to_string();
}


void database::update_expired_feeds()
{ try {
   // ilog( "Update Expired Feeds" );

   const auto head_time = head_block_time();

   const auto& idx = get_index< asset_stablecoin_data_index >().indices().get< by_feed_expiration >();
   auto itr = idx.begin();
   while( itr != idx.end() && itr->feed_is_expired( head_time ) ) // update feeds, check margin calls for each asset whose feed is expired
   {
      const asset_stablecoin_data_object& stablecoin = *itr;
      ++itr; 
      const asset_object* asset_ptr = nullptr;
      price_feed old_median_feed = stablecoin.current_feed;

      modify( stablecoin, [&]( asset_stablecoin_data_object& abdo )
      {
         abdo.update_median_feeds( head_time );
      });

      if( !stablecoin.current_feed.settlement_price.is_null() && !( stablecoin.current_feed == old_median_feed ) ) // `==` check is safe here
      {
         asset_ptr = find_asset( stablecoin.symbol );
         check_call_orders( *asset_ptr, true, false );
      }
   } 
} FC_CAPTURE_AND_RETHROW() }


void database::process_bids( const asset_stablecoin_data_object& bad )
{ try {
   if( bad.current_feed.settlement_price.is_null() )       // Asset must have settlement price
   {
      return;
   }

   const asset_object& to_revive = get_asset( bad.symbol ); 
   const asset_dynamic_data_object& bdd = get_dynamic_data( bad.symbol );

   const auto& bid_idx = get_index< asset_collateral_bid_index >().indices().get< by_price >();
   const auto start = bid_idx.lower_bound( boost::make_tuple( bad.symbol, price::max( bad.backing_asset, bad.symbol ) ) );

   share_type covered = 0;

   auto bid_itr = start;
   // Count existing bids and determine if enough bids have been made to 
   // Revive asset, entire supply should be covered.

   while( covered < bdd.get_total_supply().amount && 
      bid_itr != bid_idx.end() && 
      bid_itr->debt.symbol == bad.symbol )
   {
      const asset_collateral_bid_object& bid = *bid_itr;

      asset debt_in_bid = bid.debt;

      if( debt_in_bid.amount > bdd.get_total_supply().amount )
      {
         debt_in_bid.amount = bdd.get_total_supply().amount;
      }
         
      asset total_collateral = debt_in_bid * bad.settlement_price;

      total_collateral += bid.collateral;

      price collateralization = price( debt_in_bid, total_collateral );

      if( ~collateralization >= bad.current_feed.settlement_price )
      {
         break;
      }

      covered += debt_in_bid.amount;
      ++bid_itr;
   }

   if( covered < bdd.get_total_supply().amount ) 
   {
      return;   // Supply not yet covered
   }
   else
   {
      const auto end = bid_itr;
      share_type to_cover = bdd.get_total_supply().amount;
      asset remaining_fund = bad.settlement_fund;
      for( bid_itr = start; bid_itr != end; )
      {
         const asset_collateral_bid_object& bid = *bid_itr;
         ++bid_itr;
         asset debt_in_bid = bid.debt;
         if( debt_in_bid.amount > bdd.get_total_supply().amount )
         {
            debt_in_bid.amount = bdd.get_total_supply().amount;
         }

         share_type debt = debt_in_bid.amount;
         share_type collateral = ( debt_in_bid * bad.settlement_price ).amount;

         if( debt >= to_cover )
         {
            debt = to_cover;
            collateral = remaining_fund.amount;
         }

         to_cover -= debt;
         remaining_fund.amount -= collateral;
         execute_bid( bid, debt, collateral, bad.current_feed );
      }

      FC_ASSERT( remaining_fund.amount == 0, 
         "Settlement fund not completely allocated by bids." );
      FC_ASSERT( to_cover == 0,
         "Asset debt not completely covered by bids." );

      cancel_bids_and_revive_mpa( to_revive, bad );
   }

   ilog( "Processed Stablecoin Collateral bids: ${s}",
      ("s",bad.symbol));
      
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

   mediator_itr = mediator_idx.begin();

   // Select top position mediators
   while( mediator_itr != mediator_idx.end() && 
      top_mediators.size() < ( 10 * ESCROW_DISPUTE_MEDIATOR_AMOUNT ) )
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
      });
   }

   modify( escrow, [&]( escrow_object& esc )
   {
      esc.disputed = true;
      esc.mediators = allocated_mediators;
      esc.last_updated = now;
      esc.dispute_release_time = now + ESCROW_DISPUTE_DURATION;
   });

   ilog( "Disputing Escrow: \n ${e} \n", ("e",escrow) );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Selects the median release percentage in the escrow and divides the payment between the TO and FROM accounts.
 * 
 * Fofeits security bonds based on the difference between median
 * and individual votes to create an incentive to reach 
 * cooperative consensus between the mediators about the
 * escrow details.
 * 
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

      ilog( "Released Approved Escrow: ${e}", ("e",escrow) );
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
      ilog( "Cancelled Unapproved Escrow: ${e}", ("e",escrow) );
   }

   adjust_pending_supply( -escrow.balance );
   remove( escrow );

} FC_CAPTURE_AND_RETHROW() }

} } //node::chain