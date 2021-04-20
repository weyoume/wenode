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

const asset_object& database::get_asset( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_object, by_symbol >( symbol );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_object* database::find_asset( const asset_symbol_type& symbol ) const
{
   return find< asset_object, by_symbol >( symbol );
}

const asset_dynamic_data_object& database::get_dynamic_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_dynamic_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_dynamic_data_object* database::find_dynamic_data( const asset_symbol_type& symbol ) const
{
   return find< asset_dynamic_data_object, by_symbol >( (symbol) );
}

const asset_currency_data_object& database::get_currency_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_currency_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_currency_data_object* database::find_currency_data( const asset_symbol_type& symbol ) const
{
   return find< asset_currency_data_object, by_symbol >( (symbol) );
}

const asset_reward_fund_object& database::get_reward_fund( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_reward_fund_object, by_symbol >( symbol );
} FC_CAPTURE_AND_RETHROW( (symbol)) }

const asset_reward_fund_object* database::find_reward_fund( const asset_symbol_type& symbol ) const
{
   return find< asset_reward_fund_object, by_symbol >( symbol );
}

const asset_stablecoin_data_object& database::get_stablecoin_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_stablecoin_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_stablecoin_data_object* database::find_stablecoin_data( const asset_symbol_type& symbol ) const
{
   return find< asset_stablecoin_data_object, by_symbol >( (symbol) );
}

const asset_equity_data_object& database::get_equity_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_equity_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_equity_data_object* database::find_equity_data( const asset_symbol_type& symbol ) const
{
   return find< asset_equity_data_object, by_symbol >( (symbol) );
}

const asset_bond_data_object& database::get_bond_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_bond_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_bond_data_object* database::find_bond_data( const asset_symbol_type& symbol ) const
{
   return find< asset_bond_data_object, by_symbol >( (symbol) );
}

const asset_credit_data_object& database::get_credit_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_credit_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_credit_data_object* database::find_credit_data( const asset_symbol_type& symbol ) const
{
   return find< asset_credit_data_object, by_symbol >( (symbol) );
}

const asset_stimulus_data_object& database::get_stimulus_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_stimulus_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_stimulus_data_object* database::find_stimulus_data( const asset_symbol_type& symbol ) const
{
   return find< asset_stimulus_data_object, by_symbol >( (symbol) );
}

const asset_unique_data_object& database::get_unique_data( const asset_symbol_type& symbol ) const
{ try {
   return get< asset_unique_data_object, by_symbol >( (symbol) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_unique_data_object* database::find_unique_data( const asset_symbol_type& symbol ) const
{
   return find< asset_unique_data_object, by_symbol >( (symbol) );
}

const asset_distribution_object& database::get_asset_distribution( const asset_symbol_type& symbol )const
{ try {
   return get< asset_distribution_object, by_symbol >( symbol );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_distribution_object* database::find_asset_distribution( const asset_symbol_type& symbol )const
{
   return find< asset_distribution_object, by_symbol >( symbol );
}

const asset_distribution_balance_object& database::get_asset_distribution_balance( const account_name_type& name, const asset_symbol_type& symbol )const
{ try {
   return get< asset_distribution_balance_object, by_account_distribution >( boost::make_tuple( name, symbol ) );
} FC_CAPTURE_AND_RETHROW( (name)(symbol) ) }

const asset_distribution_balance_object* database::find_asset_distribution_balance( const account_name_type& name, const asset_symbol_type& symbol )const
{
   return find< asset_distribution_balance_object, by_account_distribution >( boost::make_tuple( name, symbol ) );
}


/**
 * Distributes currency issuance of all currency assets.
 * Pays out Staked and liquid Currency assets, including MEC, 
 * every block to all network contributors.
 * 
 * For MeCoin, the issuance rate is one Billion per year.
 * 
 *  - 25% of issuance is directed to Content Creator rewards.
 *  - 20% of issuance is directed to Equity Holder rewards.
 *  - 20% of issuance is directed to Block producers.
 *  - 10% of issuance is directed to Supernode Operator rewards.
 *  - 10% of issuance is directed to Staked MeCoin Holder rewards.
 *  -  5% of issuance is directed to The Community Enterprise fund.
 *  - 2.5% of issuance is directed to The Development reward pool.
 *  - 2.5% of issuance is directed to The Marketing reward pool.
 *  - 2.5% of issuance is directed to The Advocacy reward pool.
 *  - 2.5% of issuance is directed to The Activity reward pool.
 */
void database::process_funds()
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const producer_object& current_producer = get_producer( props.current_producer );
   const account_object& producer_account = get_account( props.current_producer );

   FC_ASSERT( current_producer.active && 
      producer_account.active,
      "Block Producer cannot process funds while account or producer object is inactive." );

   if( props.head_block_number > 0 )      // First block uses init genesis for reward.
   {
      const auto& currency_idx = get_index< asset_currency_data_index >().indices().get< by_id >();
      auto currency_itr = currency_idx.begin();

      while( currency_itr != currency_idx.end() )
      {
         const asset_currency_data_object& currency = *currency_itr;

         asset block_reward = currency.block_reward;

         asset content_reward            = ( block_reward * currency.content_reward_percent               ) / PERCENT_100;
         asset equity_reward             = ( block_reward * currency.equity_reward_percent                ) / PERCENT_100;
         asset producer_reward           = ( block_reward * currency.producer_reward_percent              ) / PERCENT_100;
         asset supernode_reward          = ( block_reward * currency.supernode_reward_percent             ) / PERCENT_100;
         asset power_reward              = ( block_reward * currency.power_reward_percent                 ) / PERCENT_100;
         asset enterprise_fund_reward    = ( block_reward * currency.enterprise_fund_reward_percent       ) / PERCENT_100;
         asset development_reward        = ( block_reward * currency.development_reward_percent           ) / PERCENT_100;
         asset marketing_reward          = ( block_reward * currency.marketing_reward_percent             ) / PERCENT_100;
         asset advocacy_reward           = ( block_reward * currency.advocacy_reward_percent              ) / PERCENT_100;
         asset activity_reward           = ( block_reward * currency.activity_reward_percent              ) / PERCENT_100;

         asset producer_block_reward     = ( producer_reward * currency.producer_block_reward_percent     ) / PERCENT_100;
         asset validation_reward         = ( producer_reward * currency.validation_reward_percent         ) / PERCENT_100;
         asset txn_stake_reward          = ( producer_reward * currency.txn_stake_reward_percent          ) / PERCENT_100;
         asset work_reward               = ( producer_reward * currency.work_reward_percent               ) / PERCENT_100;
         asset producer_activity_reward  = ( producer_reward * currency.producer_activity_reward_percent  ) / PERCENT_100;

         asset reward_checksum = content_reward + equity_reward + producer_reward + supernode_reward + power_reward + enterprise_fund_reward + development_reward + marketing_reward + advocacy_reward + activity_reward;
         
         FC_ASSERT( reward_checksum == block_reward,
            "Block reward issuance checksum failed: ${r} != ${c}, for currency: ${c}",
            ("r", reward_checksum)("b", block_reward)("c", currency));

         asset producer_checksum = producer_block_reward + validation_reward + txn_stake_reward + work_reward + producer_activity_reward;
         
         FC_ASSERT( producer_checksum == producer_reward,
            "Producer reward issuance checksum failed: ${r} != ${c}, for currency: ${c}",
            ("r", producer_checksum)("b", producer_reward)("c", currency));
         
         const asset_reward_fund_object& reward_fund = get_reward_fund( currency.symbol );
         const asset_equity_data_object& equity = get_equity_data( currency.equity_asset );

         modify( reward_fund, [&]( asset_reward_fund_object& rfo )
         {
            rfo.adjust_content_reward_balance( content_reward );
            rfo.adjust_validation_reward_balance( validation_reward );
            rfo.adjust_txn_stake_reward_balance( txn_stake_reward );
            rfo.adjust_work_reward_balance( work_reward );
            rfo.adjust_producer_activity_reward_balance( producer_activity_reward );
            rfo.adjust_supernode_reward_balance( supernode_reward );
            rfo.adjust_power_reward_balance( power_reward );
            rfo.adjust_enterprise_fund_balance( enterprise_fund_reward );
            rfo.adjust_development_reward_balance( development_reward );
            rfo.adjust_marketing_reward_balance( marketing_reward );
            rfo.adjust_advocacy_reward_balance( advocacy_reward );
            rfo.adjust_activity_reward_balance( activity_reward );
         });

         modify( equity, [&]( asset_equity_data_object& aedo )
         {
            aedo.adjust_pool( equity_reward );
         });

         adjust_reward_balance( producer_account, producer_block_reward );

         asset producer_pending = validation_reward + txn_stake_reward + work_reward + producer_activity_reward;
         asset pending_issuance = content_reward + equity_reward + supernode_reward + power_reward + enterprise_fund_reward + development_reward + marketing_reward + advocacy_reward + activity_reward;

         adjust_pending_supply( pending_issuance + producer_pending );
         
         push_virtual_operation( producer_reward_operation( producer_account.name, producer_block_reward ) );

         if( currency.block_reward_reduction_days > 0 && currency.block_reward.amount.value > 0 )     // Reduce Currency Block reward if block interval reached
         {
            if( props.head_block_number % ( currency.block_reward_reduction_days * BLOCKS_PER_DAY ) == 0 )
            {
               modify( currency, [&]( asset_currency_data_object& acdo )
               {
                  acdo.block_reward.amount -= ( ( acdo.block_reward.amount * currency.block_reward_reduction_percent ) / PERCENT_100 );
               });
            }
         }

         ++currency_itr;
      }
   }
} FC_CAPTURE_AND_RETHROW() }


const price& database::get_usd_price() const
{
   return get_stablecoin_data( SYMBOL_USD ).current_feed.settlement_price;
}

asset database::asset_to_USD( const price& p, const asset& a ) const
{
   FC_ASSERT( a.symbol != SYMBOL_USD );
   asset_symbol_type quote_symbol = p.quote.symbol;
   asset_symbol_type base_symbol = p.base.symbol;
   FC_ASSERT( base_symbol == SYMBOL_USD || quote_symbol == SYMBOL_USD );
   asset value_usd = asset( 0, SYMBOL_USD);

   if( p.is_null() )
   {
      return value_usd;
   }
   else
   {
      value_usd = a * p;
      return value_usd;
   }
}


asset database::asset_to_USD( const asset& a ) const
{
   price usd_price = get_usd_price();
   asset coin_value = a;
   asset usd_value = asset( 0, SYMBOL_USD );
   
   if( a.symbol != SYMBOL_COIN )
   {
      price coin_price = get_liquidity_pool( SYMBOL_COIN, a.symbol ).current_price();
      usd_value = asset_to_USD( usd_price, coin_value * coin_price );
   }
   else
   {
      usd_value = asset_to_USD( usd_price, coin_value );
   }
   return usd_value;
}


asset database::USD_to_asset( const price& p, const asset& a ) const
{
   FC_ASSERT( a.symbol == SYMBOL_USD );
   asset_symbol_type quote_symbol = p.quote.symbol;
   asset_symbol_type base_symbol = p.base.symbol;
   FC_ASSERT( base_symbol == SYMBOL_USD || quote_symbol == SYMBOL_USD );

   asset usd_value = a;
   asset coin_value = asset( 0, SYMBOL_USD );
   price coin_price = p;

   if( p.is_null() ) 
   {
      if( base_symbol == SYMBOL_USD ) 
      {
         coin_value = asset( 0, quote_symbol );
      } 
      else if( quote_symbol == SYMBOL_USD ) 
      {
         coin_value = asset( 0, base_symbol );
      }
   }
   else
   {
      coin_value = usd_value * coin_price;
   }
   
   return coin_value;
}


asset database::USD_to_asset( const asset& a ) const
{
   price usd_price = get_usd_price();
   return USD_to_asset( usd_price, a );
}


/**
 * Process updates across all stablecoins, execute collateral bids
 * for settled stablecoins, and update price feeds and force settlement volumes
 */
void database::process_stablecoins()
{ try {
   if( (head_block_num() % STABLECOIN_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;

   // ilog( "Process Stablecoins" );

   time_point_sec now = head_block_time();
   uint64_t head_epoch_seconds = now.sec_since_epoch();

   const auto& stablecoin_idx = get_index< asset_stablecoin_data_index >().indices().get< by_symbol >();
   auto stablecoin_itr = stablecoin_idx.begin();

   while( stablecoin_itr != stablecoin_idx.end() )
   {
      const asset_stablecoin_data_object& stablecoin = *stablecoin_itr;
      ++stablecoin_itr;

      const asset_object& asset_obj = get_asset( stablecoin.symbol );
      uint32_t flags = asset_obj.flags;
      uint64_t feed_lifetime = stablecoin.feed_lifetime.to_seconds();

      if( stablecoin.has_settlement() )
      {
         process_bids( stablecoin );
      }

      modify( stablecoin, [&]( asset_stablecoin_data_object& abdo )
      {
         abdo.force_settled_volume = 0;        // Reset all BitAsset force settlement volumes to zero

         if ( ( flags & int( asset_issuer_permission_flags::producer_fed_asset ) ) &&
              feed_lifetime < head_epoch_seconds )            // if smartcoin && check overflow
         {
            fc::time_point calculated = now - feed_lifetime;

            for( auto comment_feed_itr = abdo.feeds.rbegin(); comment_feed_itr != abdo.feeds.rend(); )       // loop feeds
            {
               auto feed_time = comment_feed_itr->second.first;
               std::advance( comment_feed_itr, 1 );

               if( feed_time < calculated )
               {
                  abdo.feeds.erase( comment_feed_itr.base() ); // delete expired feed
               }
            }
         }
      });
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Allocates rewards to staked currency asset holders
 * according to proportional balances.
 */
void database::process_power_rewards()
{ try {
   if( (head_block_num() % EQUITY_INTERVAL_BLOCKS) != 0 )    // Runs once per week
      return;

   // ilog( "Process Power Rewards" );

   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol_stake >();
   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   while( fund_itr != fund_idx.end() )
   {
      const asset_reward_fund_object& reward_fund = *fund_itr;

      asset power_reward_balance = reward_fund.power_reward_balance;       // Record the opening balance of the power reward fund
      auto balance_itr = balance_idx.lower_bound( reward_fund.symbol );
      flat_map < account_name_type, uint128_t > power_map;
      uint128_t total_power_shares = 0;
      asset distributed = asset( 0, reward_fund.symbol );

      while( balance_itr != balance_idx.end() && 
         balance_itr->symbol == reward_fund.symbol && 
         balance_itr->staked_balance >= BLOCKCHAIN_PRECISION )
      {
         uint128_t power_shares = balance_itr->staked_balance.value;         // Get the staked balance for each stakeholder.

         if( power_shares > uint128_t( 0 ) )
         {
            total_power_shares += power_shares;
            power_map[ balance_itr->owner ] = power_shares;
         }
         ++balance_itr;
      }

      for( auto b : power_map )
      {
         uint128_t reward_amount = ( uint128_t( power_reward_balance.amount.value ) * b.second ) / total_power_shares;
         asset power_reward = asset( reward_amount.to_uint64(), reward_fund.symbol );
         adjust_staked_balance( b.first, power_reward );       // Pay power reward to each stakeholder account proportionally.
         distributed += power_reward;
      }

      modify( reward_fund, [&]( asset_reward_fund_object& r )
      {
         r.adjust_power_reward_balance( -distributed ); 
      });

      adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.

      ++fund_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Calculates the relative share of equity reward dividend distribution.
 * 
 * Each account should receive the equity reward based on its balances, and account activity.
 */
share_type database::get_equity_shares( const account_balance_object& balance, const asset_equity_data_object& equity )
{
   const account_object& account = get_account( balance.owner );
   time_point now = head_block_time();

   // Account does not receive equity reward when producer votes or last activity are insufficient.

   if( ( account.producer_vote_count < equity.min_producers ) || 
      ( now > ( account.last_activity_reward + equity.min_active_time ) ) )
   {
      return 0;
   }

   share_type equity_shares = 0;
   equity_shares += balance.staked_balance;

   // Doubles equity reward when above Boost balance, Boost producer votes, and Boost Activity rewards in last 30 days
   
   if( ( balance.staked_balance >= equity.boost_balance ) &&
      ( account.producer_vote_count >= equity.boost_producers ) &&
      ( account.recent_activity_claims >= equity.boost_activity ) )
   {
      equity_shares *= 2;
   }

   if( account.membership == membership_tier_type::TOP_MEMBERSHIP )
   {
      equity_shares += ( ( equity_shares * equity.boost_top ) / PERCENT_100 );
   }

   return equity_shares;
}


/**
 * Allocates equity asset dividends from each dividend reward pool,
 * according to proportional balances.
 */
void database::process_equity_rewards()
{ try {
   if( (head_block_num() % EQUITY_INTERVAL_BLOCKS) != 0 )    // Runs once per week
      return;

   time_point now = head_block_time();
   const auto& equity_idx = get_index< asset_equity_data_index >().indices().get< by_symbol >();
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol_stake >();
   auto equity_itr = equity_idx.begin();

   while( equity_itr != equity_idx.end() )
   {
      const asset_equity_data_object& equity = *equity_itr;

      for( auto a : equity.dividend_pool )     // Distribute every asset in the dividend pool
      {
         if( a.second.amount > 0 )
         {
            asset equity_reward_balance = a.second;  // Record the opening balance of the equity reward fund
            auto balance_itr = balance_idx.lower_bound( equity.symbol );
            flat_map < account_name_type, uint128_t > equity_map;
            uint128_t total_equity_shares = 0;
            asset distributed = asset( 0, a.first );

            while( balance_itr != balance_idx.end() &&
               balance_itr->symbol == equity.symbol ) 
            {
               share_type equity_shares = get_equity_shares( *balance_itr, equity );  // Get the equity shares for each stakeholder

               if( equity_shares > 0 )
               {
                  total_equity_shares += equity_shares.value;
                  equity_map[ balance_itr->owner ] = equity_shares.value;
               }
               ++balance_itr;
            }

            for( auto b : equity_map )
            {
               uint128_t reward_amount = ( uint128_t( equity_reward_balance.amount.value ) * b.second ) / total_equity_shares;
               asset equity_reward = asset( reward_amount.to_uint64(), equity_reward_balance.symbol ); 
               adjust_reward_balance( b.first, equity_reward );       // Pay equity dividend to each stakeholder account proportionally.
               distributed += equity_reward;
            }

            modify( equity, [&]( asset_equity_data_object& e )
            {
               e.adjust_pool( -distributed ); 
               e.last_dividend = now;        // Remove the distributed amount from the dividend pool.
            });

            adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.
         }
      }
      
      ++equity_itr;
   }
} FC_CAPTURE_AND_RETHROW() }



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

      adjust_liquid_balance( bond.issuer, -total_interest );
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
      asset issuer_liquid = get_liquid_balance( bond.issuer, bond.value.symbol );
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
         adjust_liquid_balance( bond.issuer, -paid );
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
         adjust_liquid_balance( bond.issuer, -paid );
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
   if( (head_block_num() % COMMENT_FEED_INTERVAL_BLOCKS) != 0 )
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

      ilog( "Begin Processing Asset Distribution: \n ${d} \n",
         ("d",distribution));
      
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

         ilog( "Input Balance: \n ${b} \n Total Input: ${t} Total Input Units: ${u}",
         ("t",total_input.to_string())("u",total_input_units));
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

               total_funded += balance.amount;

               ilog( "Account: ${a} Received Distribution Input: ${i} Total Funded: ${t}",
                  ("a",input_account)("i",input_asset.to_string())("t",total_funded.to_string()) );
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

               total_distributed += output_asset;

               ilog( "Account: ${a} Received Distribution Output: ${o} Total Distributed: ${d}",
                  ("a",output_account)("o",output_asset.to_string())("d",total_distributed.to_string()) );
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


void database::update_expired_feeds()
{ try {
   // ilog( "Update Expired Feeds" );

   const auto head_time = head_block_time();

   const auto& idx = get_index< asset_stablecoin_data_index >().indices().get< by_feed_expiration >();
   auto itr = idx.begin();
   while( itr != idx.end() && itr->comment_feed_is_expired( head_time ) ) // update feeds, check margin calls for each asset whose feed is expired
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


/**
 * All margin positions are force closed at the swan price.
 * 
 * Collateral received goes into a force-settlement fund
 * No new margin positions can be created for this asset
 * Force settlement happens without delay at the swan price, 
 * deducting from force-settlement fund.
 * No more asset updates may be issued.
*/
void database::globally_settle_asset( const asset_object& mia, const price& settlement_price )
{ try {
   const asset_stablecoin_data_object& stablecoin = get_stablecoin_data( mia.symbol );
   FC_ASSERT( !stablecoin.has_settlement(),
      "Black swan already occurred, it should not happen again" );

   asset_symbol_type backing_asset = stablecoin.backing_asset;
   asset collateral_gathered = asset( 0, backing_asset );
   const asset_dynamic_data_object& mia_dyn = get_dynamic_data( mia.symbol );
   auto original_mia_supply = mia_dyn.get_total_supply().amount;

   const auto& call_idx = get_index< call_order_index >().indices().get< by_high_price >();
   auto call_itr = call_idx.lower_bound( price::max( backing_asset, mia.symbol ) );
   auto call_end = call_idx.upper_bound( price::min( backing_asset, mia.symbol ) );
   asset pays = asset( 0, backing_asset );

   while( call_itr != call_end )
   {
      const call_order_object& order = *call_itr;
      ++call_itr;
      pays = order.debt.multiply_and_round_up( settlement_price );
      if( pays > order.collateral )
      {
         pays = order.collateral;
      }
      collateral_gathered += pays;
      // Fill Call orders without deducting pending supply of stablecoin.
      FC_ASSERT( fill_call_order( order, pays, order.debt, settlement_price, true, NULL_ACCOUNT, true ) );
   }

   price global_settlement_price = asset( original_mia_supply, mia.symbol ) / collateral_gathered;
   
   // Activate Global settlement price on asset

   modify( stablecoin, [&]( asset_stablecoin_data_object& a )
   {
      a.settlement_price = global_settlement_price;
      a.settlement_fund = collateral_gathered.amount;
   });

   ilog( "Globally Settled Asset: ${s} at Settlement Price: ${p} \n ${st} \n",
      ("s",mia.symbol)("p",global_settlement_price.to_string())("st",stablecoin));

} FC_CAPTURE_AND_RETHROW( (mia)(settlement_price) ) }


void database::revive_stablecoin( const asset_object& stablecoin )
{ try {
   FC_ASSERT( stablecoin.is_market_issued(),
      "Asset must be a market issued asset." );
   const asset_stablecoin_data_object& bad = get_stablecoin_data( stablecoin.symbol );

   FC_ASSERT( bad.has_settlement(),
      "Asset must have a settlement price before it can be revived.");
   const asset_dynamic_data_object& bdd = get_dynamic_data( stablecoin.symbol );

   FC_ASSERT( !bad.current_feed.settlement_price.is_null(),
      "Settlement price cannot be null to revive asset." );

   if( bdd.get_total_supply().amount > 0 )    // Create + execute a "bid" with 0 additional collateral
   {
      const asset_collateral_bid_object& pseudo_bid = create< asset_collateral_bid_object >([&]( asset_collateral_bid_object& bid )
      {
         bid.bidder = stablecoin.issuer;
         bid.collateral = asset( 0, bad.backing_asset );
         bid.debt = asset( bdd.get_total_supply().amount, stablecoin.symbol );
      });

      execute_bid( pseudo_bid, bdd.get_total_supply().amount, bad.settlement_fund, bad.current_feed );
   } 
   else
   {
      FC_ASSERT( bad.settlement_fund == 0,
         "Cannot have settlement fund with zero total asset supply." );
   }
      
   cancel_bids_and_revive_mpa( stablecoin, bad );
} FC_CAPTURE_AND_RETHROW( (stablecoin) ) }


void database::cancel_bids_and_revive_mpa( const asset_object& stablecoin, const asset_stablecoin_data_object& bad )
{ try {
   FC_ASSERT( stablecoin.is_market_issued(),
      "Asset must be a market issued asset." );
   FC_ASSERT( bad.has_settlement(),
      "Asset must have a settlement price before it can be revived." );
   
   const auto& bid_idx = get_index< asset_collateral_bid_index >().indices().get< by_price >();
   auto bid_itr = bid_idx.lower_bound( boost::make_tuple( stablecoin.symbol, price::max( bad.backing_asset, stablecoin.symbol ) ) );

   while( bid_itr != bid_idx.end() && 
      bid_itr->inv_swan_price().quote.symbol == stablecoin.symbol )
   {
      const asset_collateral_bid_object& bid = *bid_itr;
      ++bid_itr;
      cancel_bid( bid );    // cancel remaining bids
   }

   modify( bad, [&]( asset_stablecoin_data_object& obj )
   {
      obj.settlement_price = price();
      obj.settlement_fund = 0;
   });

   ilog( "Cancel Bids and Revive Stablecoin: \n ${s} \n",
      ("s",bad));

} FC_CAPTURE_AND_RETHROW( ( stablecoin ) ) }



asset database::calculate_issuer_fee( const asset_object& trade_asset, const asset& trade_amount )
{ try {
   FC_ASSERT( trade_asset.symbol == trade_amount.symbol,
   "Trade asset symbol must be equal to trade amount symbol." );
      
   if( trade_asset.market_fee_percent == 0 )
   {
      return asset( 0, trade_asset.symbol );
   }

   share_type value = (( trade_amount.amount * trade_asset.market_fee_percent ) / PERCENT_100  );
   asset percent_fee = asset( value, trade_asset.symbol );

   if( percent_fee.amount > trade_asset.max_market_fee )
   {
      percent_fee.amount = trade_asset.max_market_fee;
   }
      
   return percent_fee;
} FC_CAPTURE_AND_RETHROW() }

asset database::pay_issuer_fees( const asset_object& recv_asset, const asset& receives )
{ try {
   asset issuer_fees = calculate_issuer_fee( recv_asset, receives );

   FC_ASSERT( issuer_fees <= receives, 
      "Market fee shouldn't be greater than receives." );

   if( issuer_fees.amount > 0 )
   {
      adjust_reward_balance( recv_asset.issuer, issuer_fees );
   }

   return issuer_fees;
} FC_CAPTURE_AND_RETHROW() }


asset database::pay_issuer_fees( const account_object& seller, const asset_object& recv_asset, const asset& receives )
{ try {
   const asset& issuer_fees = calculate_issuer_fee( recv_asset, receives );
   FC_ASSERT( issuer_fees <= receives,
      "Market fee shouldn't be greater than receives." );
   
   if( issuer_fees.amount > 0 )
   {
      asset reward = asset( 0, recv_asset.symbol );
      asset reward_paid = asset( 0, recv_asset.symbol );

      uint16_t reward_percent = recv_asset.market_fee_share_percent;       // Percentage of market fees shared with registrars

      if( reward_percent > 0 )         // Calculate and pay market fee sharing rewards
      {
         const account_permission_object& issuer_permissions = get_account_permissions( seller.name );
         const account_permission_object& registrar_permissions = get_account_permissions( seller.registrar );
         const account_permission_object& referrer_permissions = get_account_permissions( seller.referrer );

         share_type reward_value = ( issuer_fees.amount * reward_percent ) / PERCENT_100;
         asset registrar_reward = asset( 0, recv_asset.symbol );
         asset referrer_reward = asset( 0, recv_asset.symbol );

         if( reward_value > 0 )
         {
            reward = asset( reward_value, recv_asset.symbol );

            FC_ASSERT( reward < issuer_fees,
               "Market reward should be less than issuer fees." );

            if( registrar_permissions.is_authorized_transfer( recv_asset.issuer, recv_asset ) &&
               issuer_permissions.is_authorized_transfer( seller.registrar, recv_asset ) )
            {
               registrar_reward = reward;          // Registrar begins with all reward
            }

            if( seller.referrer != seller.registrar )
            {
               share_type referrer_rewards_value;

               if( registrar_reward == reward )
               {
                  referrer_rewards_value = ( reward.amount * seller.referrer_rewards_percentage ) / PERCENT_100;
               }
               else
               {
                  referrer_rewards_value = reward.amount;         // Referrer gets all reward if registrar cannot receive.
               }
               
               FC_ASSERT ( referrer_rewards_value <= reward.amount.value,
                  "Referrer reward shouldn't be greater than total reward." );

               if( referrer_rewards_value > 0 )
               {
                  if( referrer_permissions.is_authorized_transfer( recv_asset.issuer, recv_asset ) &&
                     issuer_permissions.is_authorized_transfer( seller.referrer, recv_asset ) )
                  {
                     referrer_reward = asset( referrer_rewards_value, recv_asset.symbol );
                     registrar_reward -= referrer_reward;    // Referrer and registrar split reward;
                  }
               }  
            }

            if( registrar_reward.amount > 0 )
            {
               adjust_reward_balance( seller.registrar, registrar_reward );
               reward_paid += registrar_reward;
            }
            if( referrer_reward.amount > 0 )
            {
               adjust_reward_balance( seller.referrer, referrer_reward );
               reward_paid += referrer_reward;
            }
         }
      }

      adjust_reward_balance( recv_asset.issuer, issuer_fees - reward_paid );
   }

   return issuer_fees;
} FC_CAPTURE_AND_RETHROW() }

} } //node::chain