#include <node/app/api_context.hpp>
#include <node/app/application.hpp>
#include <node/app/database_api.hpp>

#include <node/protocol/get_config.hpp>

#include <node/chain/util/reward.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hex.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include <cctype>
#include <cfenv>
#include <iostream>

namespace node { namespace app {


   //===================//
   // === Asset API === //
   //===================//


vector< extended_asset > database_api::get_assets( vector< string > assets )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_assets( assets );
   });
}

vector< extended_asset > database_api_impl::get_assets( vector< string > assets )const
{ 
   vector< extended_asset > results;

   const auto& asset_idx = _db.get_index< asset_index >().indices().get< by_symbol >();
   const auto& asset_dyn_idx = _db.get_index< asset_dynamic_data_index >().indices().get< by_symbol >();

   for( auto asset : assets )
   {
      auto asset_itr = asset_idx.find( asset );
      if( asset_itr != asset_idx.end() )
      {
         results.push_back( extended_asset( *asset_itr ) );
      }
      
      auto asset_dyn_itr = asset_dyn_idx.find( asset );
      if( asset_dyn_itr != asset_dyn_idx.end() )
      {
         results.back().total_supply = asset_dyn_itr->get_total_supply().amount.value;
         results.back().liquid_supply = asset_dyn_itr->liquid_supply.value;
         results.back().reward_supply = asset_dyn_itr->reward_supply.value;
         results.back().savings_supply = asset_dyn_itr->savings_supply.value;
         results.back().delegated_supply = asset_dyn_itr->delegated_supply.value;
         results.back().receiving_supply = asset_dyn_itr->receiving_supply.value;
         results.back().pending_supply = asset_dyn_itr->pending_supply.value;
         results.back().confidential_supply = asset_dyn_itr->confidential_supply.value;
      }

      const auto& currency_idx = _db.get_index< asset_currency_data_index >().indices().get< by_symbol >();
      auto currency_itr = currency_idx.find( asset );
      if( currency_itr != currency_idx.end() )
      {
         results.back().currency = currency_data_api_obj( *currency_itr );
      }

      const auto& stablecoin_idx = _db.get_index< asset_stablecoin_data_index >().indices().get< by_symbol >();
      auto stablecoin_itr = stablecoin_idx.find( asset );
      if( stablecoin_itr != stablecoin_idx.end() )
      {
         results.back().stablecoin = stablecoin_data_api_obj( *stablecoin_itr );
      }

      const auto& equity_idx = _db.get_index< asset_equity_data_index >().indices().get< by_symbol >();
      auto equity_itr = equity_idx.find( asset );
      if( equity_itr != equity_idx.end() )
      {
         results.back().equity = equity_data_api_obj( *equity_itr );
      }

      const auto& bond_idx = _db.get_index< asset_bond_data_index >().indices().get< by_symbol >();
      auto bond_itr = bond_idx.find( asset );
      if( bond_itr != bond_idx.end() )
      {
         results.back().bond = bond_data_api_obj( *bond_itr );
      }

      const auto& credit_idx = _db.get_index< asset_credit_data_index >().indices().get< by_symbol >();
      auto credit_itr = credit_idx.find( asset );
      if( credit_itr != credit_idx.end() )
      {
         results.back().credit = credit_data_api_obj( *credit_itr );
      }

      const auto& stimulus_idx = _db.get_index< asset_stimulus_data_index >().indices().get< by_symbol >();
      auto stimulus_itr = stimulus_idx.find( asset );
      if( stimulus_itr != stimulus_idx.end() )
      {
         results.back().stimulus = stimulus_data_api_obj( *stimulus_itr );
      }

      const auto& unique_idx = _db.get_index< asset_unique_data_index >().indices().get< by_symbol >();
      auto unique_itr = unique_idx.find( asset );
      if( unique_itr != unique_idx.end() )
      {
         results.back().unique = unique_data_api_obj( *unique_itr );
      }

      const auto& credit_pool_idx = _db.get_index< asset_credit_pool_index >().indices().get< by_base_symbol >();
      auto credit_pool_itr = credit_pool_idx.find( asset );
      if( credit_pool_itr != credit_pool_idx.end() )
      {
         results.back().credit_pool = credit_pool_api_obj( *credit_pool_itr );
      }

      const auto& pool_a_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_symbol_a >();
      const auto& pool_b_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_symbol_b >();
      auto pool_a_itr = pool_a_idx.lower_bound( asset );
      auto pool_b_itr = pool_b_idx.lower_bound( asset );

      while( pool_a_itr != pool_a_idx.end() && pool_a_itr->symbol_a == asset )
      {
         results.back().liquidity_pools[ pool_a_itr->symbol_b ] = liquidity_pool_api_obj( *pool_a_itr );
      }

      while( pool_b_itr != pool_b_idx.end() && pool_b_itr->symbol_b == asset )
      {
         results.back().liquidity_pools[ pool_b_itr->symbol_a ] = liquidity_pool_api_obj( *pool_b_itr );
      }

      const auto& base_idx = _db.get_index< asset_option_pool_index >().indices().get< by_base_symbol >();
      const auto& quote_idx = _db.get_index< asset_option_pool_index >().indices().get< by_quote_symbol >();
      auto base_itr = base_idx.lower_bound( asset );
      auto quote_itr = quote_idx.lower_bound( asset );

      while( base_itr != base_idx.end() && base_itr->base_symbol == asset )
      {
         results.back().option_pools[ base_itr->quote_symbol ] = option_pool_api_obj( *base_itr );
      }

      while( quote_itr != quote_idx.end() && quote_itr->quote_symbol == asset )
      {
         results.back().option_pools[ quote_itr->base_symbol ] = option_pool_api_obj( *quote_itr );
      }

      const auto& prediction_idx = _db.get_index< asset_prediction_pool_index >().indices().get< by_prediction_symbol >();
      auto prediction_itr = prediction_idx.find( asset );
      if( prediction_itr != prediction_idx.end() )
      {
         results.back().prediction = prediction_pool_api_obj( *prediction_itr );
      }

      const auto& resolution_idx = _db.get_index< asset_prediction_pool_resolution_index >().indices().get< by_prediction_symbol >();
      auto resolution_itr = resolution_idx.lower_bound( asset );

      while( resolution_itr != resolution_idx.end() && resolution_itr->prediction_symbol == asset )
      {
         results.back().resolutions[ resolution_itr->resolution_outcome ] = prediction_pool_resolution_api_obj( *resolution_itr );
      }

      const auto& distribution_idx = _db.get_index< asset_distribution_index >().indices().get< by_symbol >();
      auto distribution_itr = distribution_idx.find( asset );
      if( distribution_itr != distribution_idx.end() )
      {
         results.back().distribution = distribution_api_obj( *distribution_itr );
      }

      const auto& balance_idx = _db.get_index< asset_distribution_balance_index >().indices().get< by_distribution_account >();
      auto balance_itr = balance_idx.find( asset );
      if( balance_itr != balance_idx.end() )
      {
         results.back().distribution_balances[ balance_itr->sender ] = distribution_balance_api_obj( *balance_itr );
      }

      const auto& fund_idx = _db.get_index< asset_reward_fund_index >().indices().get< by_symbol >();
      auto fund_itr = fund_idx.find( asset );
      if( fund_itr != fund_idx.end() )
      {
         results.back().reward_fund = reward_fund_api_obj( *fund_itr );
      }  
   }
   return results;
}


optional< escrow_api_obj > database_api::get_escrow( string from, string escrow_id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_escrow( from, escrow_id );
   });
}


optional< escrow_api_obj > database_api_impl::get_escrow( string from, string escrow_id )const
{
   optional< escrow_api_obj > results;

   try
   {
      results = _db.get_escrow( from, escrow_id );
   }
   catch ( ... ) {}

   return results;
}


} } // node::app