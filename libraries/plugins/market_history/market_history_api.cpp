#include <node/market_history/market_history_api.hpp>

#include <node/chain/node_objects.hpp>

namespace node { namespace market_history {

namespace detail
{

class market_history_api_impl
{
   public:
      market_history_api_impl( node::app::application& _app )
         :app( _app ) {}

      market_ticker get_ticker( string buy_symbol, string sell_symbol ) const;
      market_volume get_volume( string buy_symbol, string sell_symbol ) const;
      order_book get_order_book( string buy_symbol, string sell_symbol, uint32_t limit ) const;
      vector< market_trade > get_trade_history( string buy_symbol, string sell_symbol, time_point start, time_point end, uint32_t limit ) const;
      vector< market_trade > get_recent_trades( string buy_symbol, string sell_symbol, uint32_t limit ) const;
      vector< market_duration_object > get_market_history( string buy_symbol, string sell_symbol, uint32_t seconds, time_point start, time_point end ) const;
      flat_set< uint32_t > get_market_history_durations() const;

      node::app::application& app;
};

market_ticker market_history_api_impl::get_ticker( string buy_symbol, string sell_symbol ) const
{
   market_ticker result;

   auto db = app.chain_database();
   const asset_object& buy_asset = db.get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = db.get_asset( asset_symbol_type( sell_symbol ) );
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

   const auto& duration_idx = db->get_index< market_duration_index >().indices().get< by_asset_pair >();
   auto current_itr = duration_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, 60, db->head_block_time() ) );

   auto hour_itr = duration_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, 60, ( db->head_block_time() - fc::hours(1) ) ) );
   auto day_itr = duration_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, 3600, ( db->head_block_time() - fc::days(1) ) ) );
   auto week_itr = duration_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, 3600, ( db->head_block_time() - fc::days(7) ) ) );
   auto month_itr = duration_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, 3600, ( db->head_block_time() - fc::days(30) ) ) );

   if( current_itr != duration_idx.end() )
   {
      price current_price = current_itr->close_price_real( asset_symbol_type( sell_symbol ) );
      result.last_price = current_price;
   }
   else
   {
      result.last_price = 0;
   }

   if( hour_itr != duration_idx.end() )
   {
      double hour_price = hour_itr->close_price_real( asset_symbol_type( sell_symbol ) );
      result.hour_percent_change = ( ( result.last_price - hour_price ) / hour_price ) * 100;
   }
   else
   {
      result.hour_percent_change = 0;
   }

   if( day_itr != duration_idx.end() )
   {
      double day_price = day_itr->last_price.to_real();
      result.day_percent_change = ( ( result.last_price - day_price ) / day_price ) * 100;
   }
   else
   {
      result.day_percent_change = 0;
   }

   if( week_itr != duration_idx.end() )
   {
      double week_price = week_itr->close_price_real( asset_symbol_type( sell_symbol ) );
      result.week_percent_change = ( ( result.last_price - week_price ) / week_price ) * 100;
   }
   else
   {
      result.week_percent_change = 0;
   }

   if( month_itr != duration_idx.end() )
   {
      double month_price = month_itr->close_price_real( asset_symbol_type( sell_symbol ) );
      result.month_percent_change = ( ( result.last_price - month_price ) / month_price ) * 100;
   }
   else
   {
      result.month_percent_change = 0;
   }
   
   order_book orders = get_order_book( buy_symbol, sell_symbol, 1 );    // Get top orders for bid and ask
   if( orders.bids.size() )
   {
      result.highest_bid = orders.bids[0].price;
   }
      
   if( orders.asks.size() )
   {
      result.lowest_ask = orders.asks[0].price;
   }
      
   auto volume = get_volume( buy_symbol, sell_symbol );
   result.buy_volume = volume.buy_volume;
   result.sell_volume = volume.sell_volume;

   return result;
}

market_volume market_history_api_impl::get_volume( string buy_symbol, string sell_symbol ) const
{
   auto db = app.chain_database();
   const asset_object& buy_asset = db->get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = db->get_asset( asset_symbol_type( sell_symbol ) );
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
   const auto& duration_idx = db->get_index< market_duration_index >().indices().get< by_asset_pair >();
   auto itr = duration_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, 60, ( db->head_block_time() - fc::days(1) ) ) );
   market_volume result;

   if( itr == duration_idx.end() )
   {
      return result;
   }
      
   uint32_t duration_secs = itr->seconds;
   do
   {
      if( symbol_a == asset_symbol_type(buy_symbol) )
      {
         result.buy_volume += itr->volume_a;
         result.sell_volume += itr->volume_b;
      }
      else
      {
         result.buy_volume += itr->volume_b;
         result.sell_volume += itr->volume_a;
      }

      ++itr;
   } while( itr != duration_idx.end() && itr->seconds == duration_secs );

   return result;
}

order_book market_history_api_impl::get_order_book( string buy_symbol, string sell_symbol,  uint32_t limit ) const
{
   FC_ASSERT( limit <= 1000, "Maximum orderbook API limit is 1000 orders per side." );
   auto db = app.chain_database();

   order_book result;

   auto max_sell = price::max( asset_symbol_type( sell_symbol ), asset_symbol_type( buy_symbol ) );
   auto max_buy = price::max( asset_symbol_type( buy_symbol ), asset_symbol_type( sell_symbol ) );

   const auto& limit_price_idx = db->get_index<limit_order_index>().indices().get<by_price>();
   const auto& margin_price_idx = db->get_index<margin_order_index>().indices().get<by_price>();

   auto limit_sell_itr = limit_price_idx.lower_bound( max_sell );
   auto limit_buy_itr = limit_price_idx.lower_bound( max_buy );
   auto limit_end = limit_price_idx.end();

   auto margin_sell_itr = margin_price_idx.lower_bound (max_sell );
   auto margin_buy_itr = margin_price_idx.lower_bound( max_buy );
   auto margin_end = margin_price_idx.end();

   while( ( ( limit_sell_itr != limit_end &&
      limit_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) ) ||
      ( margin_sell_itr != margin_end &&
      margin_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) ) ) && 
      result.bids.size() < limit )
   {
      if( limit_sell_itr->sell_price >= margin_sell_itr->sell_price && limit_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) )
      {
         order cur;
         auto itr = limit_sell_itr;
         cur.order_price = itr->sell_price;
         cur.real_price = cur.order_price.to_real();
         cur.sell_asset = itr->amount_for_sale();
         cur.buy_asset = itr->amount_to_receive();
         cur.created = itr->created;
         result.bids.push_back( cur );
         ++limit_sell_itr;
      }
      else if( margin_sell_itr->sell_price >= limit_sell_itr->sell_price && margin_sell_itr->sell_price.base.symbol == asset_symbol_type( sell_symbol ) )
      {
         order cur;
         auto itr = margin_sell_itr;
         cur.order_price = itr->sell_price;
         cur.real_price = cur.order_price.to_real();
         cur.sell_asset = itr->amount_for_sale();
         cur.buy_asset = itr->amount_to_receive();
         cur.created = itr->created;
         result.bids.push_back( cur );
         ++margin_sell_itr;
      }
      else
      {
         break;
      } 
   }
   while( ( ( limit_buy_itr != limit_end && 
      limit_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol ) ) || 
      ( margin_buy_itr != margin_end && 
      margin_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol ) ) ) && 
      result.asks.size() < limit )
   {
      if( limit_buy_itr->sell_price >= margin_buy_itr->sell_price && limit_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol )  )
      {
         order cur;
         auto itr = limit_buy_itr;
         cur.order_price = ~(itr->sell_price);
         cur.real_price = cur.order_price.to_real();
         cur.sell_asset = itr->amount_for_sale();
         cur.buy_asset = itr->amount_to_receive();
         cur.created = itr->created;
         result.asks.push_back( cur );
         ++limit_buy_itr;
      }
      else if( margin_buy_itr->sell_price >= limit_buy_itr->sell_price && margin_buy_itr->sell_price.base.symbol == asset_symbol_type( buy_symbol ) )
      {
         order cur;
         auto itr = margin_buy_itr;
         cur.order_price = ~(itr->sell_price);
         cur.real_price = cur.order_price.to_real();
         cur.sell_asset = itr->amount_for_sale();
         cur.buy_asset = itr->amount_to_receive();
         cur.created = itr->created;
         result.asks.push_back( cur );
         ++margin_buy_itr;
      }
      else
      {
         break;
      }  
   }
   return result;
}

std::vector< market_trade > market_history_api_impl::get_trade_history( string buy_symbol, string sell_symbol, time_point start, time_point end, uint32_t limit ) const
{
   FC_ASSERT( limit <= 1000 );
   auto db = app.chain_database();
   const asset_object& buy_asset = db->get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = db->get_asset( asset_symbol_type( sell_symbol ) );
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

   const auto& order_idx = db->get_index< order_history_index >().indices().get< by_old_asset_pair >();
   auto itr = order_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, start ) );

   std::vector< market_trade > result;

   while( itr != order_idx.end() && itr->time <= end && result.size() < limit )
   {
      market_trade trade;
      trade.date = itr->time;
      trade.current_pays = itr->op.current_pays;
      trade.open_pays = itr->op.open_pays;
      result.push_back( trade );
      ++itr;
   }

   return result;
}

vector< market_trade > market_history_api_impl::get_recent_trades( string buy_symbol, string sell_symbol, uint32_t limit = 1000 ) const
{
   FC_ASSERT( limit <= 1000 );
   auto db = app.chain_database();
   const asset_object& buy_asset = db->get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = db->get_asset( asset_symbol_type( sell_symbol ) );
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

   const auto& order_idx = db->get_index< order_history_index >().indices().get< by_new_asset_pair >();
   auto itr = order_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b ) );

   vector< market_trade > result;

   while( itr != order_idx.rend() && result.size() < limit )
   {
      market_trade trade;
      trade.date = itr->time;
      trade.current_pays = itr->op.current_pays;
      trade.open_pays = itr->op.open_pays;
      result.push_back( trade );
      ++itr;
   }

   return result;
}

std::vector< candle_stick > market_history_api_impl::get_market_history( string buy_symbol, string sell_symbol, uint32_t seconds, time_point start, time_point end ) const
{
   auto db = app.chain_database();
   const asset_object& buy_asset = db->get_asset( asset_symbol_type( buy_symbol ) );
   const asset_object& sell_asset = db->get_asset( asset_symbol_type( sell_symbol ) );
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

   const auto& duration_idx = db->get_index< market_duration_index >().indices().get< by_old_asset_pair >();
   auto itr = duration_idx.lower_bound( boost::make_tuple( symbol_a, symbol_b, seconds, start ) );

   std::vector< candle_stick > result;

   while( itr != duration_idx.end() && itr->seconds == seconds && itr->open_time < end )
   {
      candle_stick candle;
      candle.open_time = itr->open_time;
      candle.period = itr->seconds;
      candle.open = itr->open_price_real( asset_symbol_type( sell_symbol ) );
      candle.high = itr->high_price_real( asset_symbol_type( sell_symbol ) );
      candle.low = itr->low_price_real( asset_symbol_type( sell_symbol ) );
      candle.close = itr->close_price_real( asset_symbol_type( sell_symbol ) );
      if( symbol_a == asset_symbol_type( buy_symbol ) )
      {
         candle.buy_volume = itr->volume_a;
         candle.sell_volume = itr->volume_b;
      }
      else
      {
         candle.buy_volume = itr->volume_b;
         candle.sell_volume = itr->volume_a;
      }

      result.push_back( candle );

      ++itr;
   }

   return result;
}

flat_set< uint32_t > market_history_api_impl::get_market_history_durations() const
{
   auto durations = app.get_plugin< market_history_plugin >( MARKET_HISTORY_PLUGIN_NAME )->get_tracked_durations();
   return durations;
}

} // detail

market_history_api::market_history_api( const node::app::api_context& ctx )
{
   my = std::make_shared< detail::market_history_api_impl >( ctx.app );
}

void market_history_api::on_api_startup() {}

market_ticker market_history_api::get_ticker() const
{
   return my->app.chain_database()->with_read_lock( [&]()
   {
      return my->get_ticker();
   });
}

market_volume market_history_api::get_volume( string buy_symbol, string sell_symbol ) const
{
   return my->app.chain_database()->with_read_lock( [&]()
   {
      return my->get_volume();
   });
}

order_book market_history_api::get_order_book( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->app.chain_database()->with_read_lock( [&]()
   {
      return my->get_order_book( limit );
   });
}

std::vector< market_trade > market_history_api::get_trade_history( string buy_symbol, string sell_symbol, time_point start, time_point end, uint32_t limit ) const
{
   return my->app.chain_database()->with_read_lock( [&]()
   {
      return my->get_trade_history( start, end, limit );
   });
}

std::vector< market_trade > market_history_api::get_recent_trades( string buy_symbol, string sell_symbol, uint32_t limit ) const
{
   return my->app.chain_database()->with_read_lock( [&]()
   {
      return my->get_recent_trades( limit );
   });
}

std::vector< candle_stick > market_history_api::get_market_history( string buy_symbol, string sell_symbol, uint32_t seconds, time_point start, time_point end ) const
{
   return my->app.chain_database()->with_read_lock( [&]()
   {
      return my->get_market_history( seconds, start, end );
   });
}

flat_set< uint32_t > market_history_api::get_market_history_durations() const
{
   return my->app.chain_database()->with_read_lock( [&]()
   {
      return my->get_market_history_durations();
   });
}

} } // node::market_history
