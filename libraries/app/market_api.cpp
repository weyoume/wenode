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


   //====================//
   // === Market API === //
   //====================//
   

vector< order_state > database_api::get_open_orders( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_open_orders( names );
   });
}

vector< order_state > database_api_impl::get_open_orders( vector< string > names )const
{
   vector< order_state > results;
   const auto& limit_idx = _db.get_index< limit_order_index >().indices().get< by_account >();
   const auto& margin_idx = _db.get_index< margin_order_index >().indices().get< by_account >();
   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_account >();
   const auto& loan_idx = _db.get_index< credit_loan_index >().indices().get< by_owner >();
   const auto& collateral_idx = _db.get_index< credit_collateral_index >().indices().get< by_owner >();

   for( auto name : names )
   {
      order_state ostate;
      auto limit_itr = limit_idx.lower_bound( name );
      while( limit_itr != limit_idx.end() && limit_itr->seller == name ) 
      {
         ostate.limit_orders.push_back( limit_order_api_obj( *limit_itr ) );
         ++limit_itr;
      }

      auto margin_itr = margin_idx.lower_bound( name );
      while( margin_itr != margin_idx.end() && margin_itr->owner == name ) 
      {
         ostate.margin_orders.push_back( margin_order_api_obj( *margin_itr ) );
         ++margin_itr;
      }

      auto call_itr = call_idx.lower_bound( name );
      while( call_itr != call_idx.end() && call_itr->borrower == name ) 
      {
         ostate.call_orders.push_back( call_order_api_obj( *call_itr ) );
         ++call_itr;
      }

      auto loan_itr = loan_idx.lower_bound( name );
      while( loan_itr != loan_idx.end() && loan_itr->owner == name ) 
      {
         ostate.loan_orders.push_back( credit_loan_api_obj( *loan_itr ) );
         ++loan_itr;
      }

      auto collateral_itr = collateral_idx.lower_bound( name );
      while( collateral_itr != collateral_idx.end() && collateral_itr->owner == name ) 
      {
         ostate.collateral.push_back( credit_collateral_api_obj( *collateral_itr ) );
         ++collateral_itr;
      }
   }
   return results;
}

market_limit_orders database_api::get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_limit_orders( buy_symbol, sell_symbol, limit );
   });
}

market_limit_orders database_api_impl::get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_limit_orders results;

   const auto& limit_price_idx = _db.get_index< limit_order_index >().indices().get< by_high_price >();

   auto max_sell = price::max( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) );
   auto max_buy = price::max( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) );
   
   auto limit_sell_itr = limit_price_idx.lower_bound( max_sell );
   auto limit_buy_itr = limit_price_idx.lower_bound( max_buy );
   auto limit_end = limit_price_idx.end();

   while( limit_sell_itr != limit_end &&
      limit_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) && 
      results.limit_bids.size() < limit )
   {
      results.limit_bids.push_back( limit_order_api_obj( *limit_sell_itr ) );
      ++limit_sell_itr;
   }
   while( limit_buy_itr != limit_end && 
      limit_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol ) && 
      results.limit_asks.size() < limit )
   {
      results.limit_asks.push_back( limit_order_api_obj( *limit_buy_itr ) );
      ++limit_buy_itr;  
   }
   return results;
}

market_margin_orders database_api::get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_margin_orders( buy_symbol, sell_symbol, limit );
   });
}

market_margin_orders database_api_impl::get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_margin_orders results;

   const auto& margin_price_idx = _db.get_index< margin_order_index >().indices().get< by_high_price >();

   auto max_sell = price::max( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) );
   auto max_buy = price::max( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) );
   
   auto margin_sell_itr = margin_price_idx.lower_bound( boost::make_tuple( false, max_sell ) );
   auto margin_buy_itr = margin_price_idx.lower_bound( boost::make_tuple( false, max_buy ) );
   auto margin_end = margin_price_idx.end();

   while( margin_sell_itr != margin_end &&
      margin_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) &&
      results.margin_bids.size() < limit )
   {
      results.margin_bids.push_back( margin_order_api_obj( *margin_sell_itr ) );
      ++margin_sell_itr;
   }
   while( margin_buy_itr != margin_end && 
      margin_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol ) &&
      results.margin_asks.size() < limit )
   {
      results.margin_asks.push_back( margin_order_api_obj( *margin_buy_itr ) );
      ++margin_buy_itr;  
   }
   return results;
}

market_option_orders database_api::get_option_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_option_orders( buy_symbol, sell_symbol, limit );
   });
}

market_option_orders database_api_impl::get_option_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_option_orders results;

   const asset_object& buy_asset = _db.get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = _db.get_asset( asset_symbol_type( sell_symbol ) );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( buy_asset.id < sell_asset.id )
   {
      symbol_a = buy_asset.symbol;
      symbol_b = sell_asset.symbol;
   }
   else
   {
      symbol_b = buy_asset.symbol;
      symbol_a = sell_asset.symbol;
   }

   const auto& option_idx = _db.get_index< option_order_index >().indices().get< by_high_price >();

   auto max_price = price::max( asset_symbol_type( symbol_a ), asset_symbol_type( symbol_b ) );
   auto option_itr = option_idx.lower_bound( max_price );
   auto option_end = option_idx.end();

   while( option_itr != option_end &&
      option_itr->option_price().base.symbol == asset_symbol_type( symbol_a ) &&
      option_itr->option_price().quote.symbol == asset_symbol_type( symbol_b ) &&
      results.option_calls.size() < limit &&
      results.option_puts.size() < limit )
   {
      if( option_itr->call() )
      {
         results.option_calls.push_back( option_order_api_obj( *option_itr ) );
      }
      else
      {
         results.option_puts.push_back( option_order_api_obj( *option_itr ) );
      }
      
      ++option_itr;
   }
   
   return results;
}

market_call_orders database_api::get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_call_orders( buy_symbol, sell_symbol, limit );
   });
}

market_call_orders database_api_impl::get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_call_orders results;

   const asset_object& buy_asset = _db.get_asset( buy_symbol );
   const asset_object& sell_asset = _db.get_asset( sell_symbol );

   asset_symbol_type stablecoin_symbol;

   if( buy_asset.asset_type == asset_property_type::STABLECOIN_ASSET )
   {
      stablecoin_symbol = buy_asset.symbol;
   }
   else if( sell_asset.asset_type == asset_property_type::STABLECOIN_ASSET )
   {
      stablecoin_symbol = sell_asset.symbol;
   }
   else
   {
      return results;
   }

   const asset_stablecoin_data_object& bit_obj = _db.get_stablecoin_data( stablecoin_symbol );
   results.settlement_price = bit_obj.current_feed.settlement_price;

   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_debt >();
   
   auto call_itr = call_idx.lower_bound( stablecoin_symbol );
   auto call_end = call_idx.end();

   while( call_itr != call_end &&
      call_itr->debt_type() == stablecoin_symbol && 
      results.calls.size() < limit )
   {
      results.calls.push_back( call_order_api_obj( *call_itr ) );
      ++call_itr;
   }
   
   return results;
}

market_auction_orders database_api::get_auction_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_auction_orders( buy_symbol, sell_symbol, limit );
   });
}

market_auction_orders database_api_impl::get_auction_orders( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );

   market_auction_orders results;

   const auto& auction_price_idx = _db.get_index< auction_order_index >().indices().get< by_high_price >();

   auto max_sell = price::max( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) );
   auto max_buy = price::max( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) );
   
   auto auction_sell_itr = auction_price_idx.lower_bound( max_sell );
   auto auction_buy_itr = auction_price_idx.lower_bound( max_buy );
   auto auction_end = auction_price_idx.end();

   while( auction_sell_itr != auction_end &&
      auction_sell_itr->sell_asset() == asset_symbol_type( sell_symbol ) &&
      results.product_auction_bids.size() < limit )
   {
      results.product_auction_bids.push_back( auction_order_api_obj( *auction_sell_itr ) );
      ++auction_sell_itr;
   }
   while( auction_buy_itr != auction_end && 
      auction_buy_itr->sell_asset() == asset_symbol_type( buy_symbol ) &&
      results.auction_asks.size() < limit )
   {
      results.auction_asks.push_back( auction_order_api_obj( *auction_buy_itr ) );
      ++auction_buy_itr;  
   }
   return results;
}

market_credit_loans database_api::get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_credit_loans( buy_symbol, sell_symbol, limit );
   });
}

market_credit_loans database_api_impl::get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit = 1000 )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );
   limit = std::min( limit, uint32_t( 1000 ) );
   
   market_credit_loans results;

   const auto& loan_idx = _db.get_index<credit_loan_index>().indices().get< by_liquidation_spread >();
   
   auto loan_buy_itr = loan_idx.lower_bound( boost::make_tuple( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) ) );
   auto loan_sell_itr = loan_idx.lower_bound( boost::make_tuple( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) ) );
   auto loan_end = loan_idx.end();

   while( loan_sell_itr != loan_end &&
      loan_sell_itr->debt_asset() == asset_symbol_type( sell_symbol ) &&
      results.loan_bids.size() < limit )
   {
      results.loan_bids.push_back( credit_loan_api_obj( *loan_sell_itr ) );
      ++loan_sell_itr;
   }
   while( loan_buy_itr != loan_end &&
      loan_buy_itr->debt_asset() == asset_symbol_type( buy_symbol ) &&
      results.loan_asks.size() < limit )
   {
      results.loan_asks.push_back( credit_loan_api_obj( *loan_buy_itr ) );
      ++loan_buy_itr;
   }
   return results;
}

vector< credit_pool_api_obj> database_api::get_credit_pools( vector< string > assets )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_credit_pools( assets );
   });
}

vector< credit_pool_api_obj> database_api_impl::get_credit_pools( vector< string > assets )const
{
   vector< credit_pool_api_obj> results;

   const auto& pool_idx = _db.get_index< asset_credit_pool_index >().indices().get< by_base_symbol >();

   for( auto symbol : assets )
   {
      auto pool_itr = pool_idx.find( symbol );
      if( pool_itr != pool_idx.end() )
      {
         results.push_back( credit_pool_api_obj( *pool_itr ) );
      }
   }

   return results; 
}

vector< liquidity_pool_api_obj > database_api::get_liquidity_pools( string buy_symbol, string sell_symbol )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_liquidity_pools( buy_symbol, sell_symbol );
   });
}

vector< liquidity_pool_api_obj > database_api_impl::get_liquidity_pools( string buy_symbol, string sell_symbol )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );

   vector< liquidity_pool_api_obj > results;

   const asset_object& buy_asset = _db.get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = _db.get_asset( asset_symbol_type( sell_symbol ) );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( buy_asset.id < sell_asset.id )
   {
      symbol_a = buy_asset.symbol;
      symbol_b = sell_asset.symbol;
   }
   else
   {
      symbol_b = buy_asset.symbol;
      symbol_a = sell_asset.symbol;
   }

   const auto& pool_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_asset_pair >();

   liquidity_pool_api_obj buy_core_pool;
   liquidity_pool_api_obj sell_core_pool;
   liquidity_pool_api_obj buy_usd_pool;
   liquidity_pool_api_obj sell_usd_pool;
   liquidity_pool_api_obj direct_pool;

   auto pool_itr = pool_idx.find( boost::make_tuple( symbol_a, symbol_b ) );
   if( pool_itr != pool_idx.end() )
   {
      direct_pool = liquidity_pool_api_obj( *pool_itr );
      results.push_back( direct_pool );
   }

   if( asset_symbol_type( buy_symbol ) != SYMBOL_COIN )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_COIN, asset_symbol_type( buy_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         buy_core_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( buy_core_pool );
      }
   }

   if( asset_symbol_type( sell_symbol ) != SYMBOL_COIN )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_COIN, asset_symbol_type( sell_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         sell_core_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( sell_core_pool );
      }
   }

   if( asset_symbol_type( buy_symbol ) != SYMBOL_USD )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_USD, asset_symbol_type( buy_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         buy_usd_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( buy_usd_pool );
      }
   }

   if( asset_symbol_type( sell_symbol ) != SYMBOL_USD )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_USD, asset_symbol_type( sell_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         sell_usd_pool = liquidity_pool_api_obj( *pool_itr );
         results.push_back( sell_usd_pool );
      }
   }
   
   return results; 
}


vector< option_pool_api_obj > database_api::get_option_pools( string buy_symbol, string sell_symbol )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_option_pools( buy_symbol, sell_symbol );
   });
}

vector< option_pool_api_obj > database_api_impl::get_option_pools( string buy_symbol, string sell_symbol )const
{
   FC_ASSERT( buy_symbol != sell_symbol, 
      "Buy Symbol cannot be equal to be Sell symbol." );

   vector< option_pool_api_obj > results;

   const asset_object& buy_asset = _db.get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = _db.get_asset( asset_symbol_type( sell_symbol ) );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( buy_asset.id < sell_asset.id )
   {
      symbol_a = buy_asset.symbol;
      symbol_b = sell_asset.symbol;
   }
   else
   {
      symbol_b = buy_asset.symbol;
      symbol_a = sell_asset.symbol;
   }

   const auto& pool_idx = _db.get_index< asset_option_pool_index >().indices().get< by_asset_pair >();

   option_pool_api_obj buy_core_pool;
   option_pool_api_obj sell_core_pool;
   option_pool_api_obj buy_usd_pool;
   option_pool_api_obj sell_usd_pool;
   option_pool_api_obj direct_pool;

   auto pool_itr = pool_idx.find( boost::make_tuple( symbol_a, symbol_b ) );
   if( pool_itr != pool_idx.end() )
   {
      direct_pool = option_pool_api_obj( *pool_itr );
      results.push_back( direct_pool );
   }

   if( asset_symbol_type( buy_symbol ) != SYMBOL_COIN )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_COIN, asset_symbol_type( buy_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         buy_core_pool = option_pool_api_obj( *pool_itr );
         results.push_back( buy_core_pool );
      }
   }

   if( asset_symbol_type( sell_symbol ) != SYMBOL_COIN )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_COIN, asset_symbol_type( sell_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         sell_core_pool = option_pool_api_obj( *pool_itr );
         results.push_back( sell_core_pool );
      }
   }

   if( asset_symbol_type( buy_symbol ) != SYMBOL_USD )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_USD, asset_symbol_type( buy_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         buy_usd_pool = option_pool_api_obj( *pool_itr );
         results.push_back( buy_usd_pool );
      }
   }

   if( asset_symbol_type( sell_symbol ) != SYMBOL_USD )
   {
      auto pool_itr = pool_idx.find( boost::make_tuple( SYMBOL_USD, asset_symbol_type( sell_symbol ) ) );
      if( pool_itr != pool_idx.end() )
      {
         sell_usd_pool = option_pool_api_obj( *pool_itr );
         results.push_back( sell_usd_pool );
      }
   }
   
   return results;
}

market_state database_api::get_market_state( string buy_symbol, string sell_symbol )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_market_state( buy_symbol, sell_symbol );
   });
}

market_state database_api_impl::get_market_state( string buy_symbol, string sell_symbol )const
{
   market_state results;

   results.limit_orders = get_limit_orders( buy_symbol, sell_symbol );
   results.margin_orders = get_margin_orders( buy_symbol, sell_symbol );
   results.option_orders = get_option_orders( buy_symbol, sell_symbol );

   const asset_object& buy_asset = _db.get_asset( buy_symbol );
   const asset_object& sell_asset = _db.get_asset( sell_symbol );

   if( buy_asset.is_market_issued() )
   {
      const asset_stablecoin_data_object& buy_stablecoin = _db.get_stablecoin_data( buy_symbol );
      if( buy_stablecoin.backing_asset == sell_symbol )
      {
         results.call_orders = get_call_orders( buy_symbol, sell_symbol );
      }
   }
   if( sell_asset.is_market_issued() )
   {
      const asset_stablecoin_data_object& sell_stablecoin = _db.get_stablecoin_data( sell_symbol );
      if( sell_stablecoin.backing_asset == buy_symbol )
      {
         results.call_orders = get_call_orders( buy_symbol, sell_symbol );
      }
   }

   results.auction_orders = get_auction_orders( buy_symbol, sell_symbol );
   results.liquidity_pools = get_liquidity_pools( buy_symbol, sell_symbol );
   results.option_pools = get_option_pools( buy_symbol, sell_symbol );

   vector< string > assets;

   assets.push_back( buy_symbol );
   assets.push_back( sell_symbol );
   results.credit_pools = get_credit_pools( assets );
   results.credit_loans = get_credit_loans( buy_symbol, sell_symbol );

   return results; 
}

} } // node::app