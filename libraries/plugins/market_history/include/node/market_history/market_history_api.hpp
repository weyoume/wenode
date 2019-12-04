#pragma once

#include <node/market_history/market_history_plugin.hpp>
#include <node/app/database_api.hpp>

#include <node/protocol/types.hpp>

#include <fc/api.hpp>

namespace node { namespace app {
   struct api_context;
} }

namespace node{ namespace market_history {

using chain::share_type;
using fc::time_point;

namespace detail
{
   class market_history_api_impl;
}

struct market_ticker
{
   double      last_price = 0;
   double      highest_bid = 0;
   double      lowest_ask = 0;
   double      hour_percent_change = 0;
   double      day_percent_change = 0;
   double      week_percent_change = 0;
   double      month_percent_change = 0;
   asset       buy_volume;
   asset       sell_volume;
};

struct market_volume
{
   asset       buy_volume;
   asset       sell_volume;
};

struct order
{
   price                order_price;
   double               real_price;
   asset                buy_asset;
   asset                sell_asset;
   fc::time_point       created;
};

struct order_book
{
   vector< order > bids;
   vector< order > asks;
};

struct market_trade
{
   time_point     date;
   asset          current_pays;
   asset          open_pays;
};

struct candle_stick 
{
   time_point      open_time;
   uint32_t        period;
   double          high;
   double          low;
   double          open;
   double          close;
   asset           buy_volume;
   asset           sell_volume;
};



class market_history_api
{
   public:
      market_history_api( const node::app::api_context& ctx );

      void on_api_startup();

      /**
       * @brief Returns the market ticker for the specified market, plus the 24h volume and 24h change
       * @param buy_symbol The asset symbol for the buy side of the desired market. Quote of the price.
       * @param sell_symbol The asset symbol for the sell side of the desired market. Base of the price.
       */
      market_ticker get_ticker( string buy_symbol, string sell_symbol ) const;

      /**
       * @brief Returns the market volume for the past 24 hours
       * @param buy_symbol The asset symbol for the buy side of the desired market. Quote of the price.
       * @param sell_symbol The asset symbol for the sell side of the desired market. Base of the price.
       */
      market_volume get_volume( string buy_symbol, string sell_symbol ) const;

      /**
       * @brief Returns the current order book for the specified market.
       * @param buy_symbol The asset symbol for the buy side of the desired market. Quote of the price.
       * @param sell_symbol The asset symbol for the sell side of the desired market. Base of the price.
       * @param limit The number of orders to have on each side of the order book. Maximum is 500
       */
      order_book get_order_book( string buy_symbol, string sell_symbol, uint32_t limit = 500 ) const;

      /**
       * @brief Returns the trade history for the specified market.
       * @param buy_symbol The asset symbol for the buy side of the desired market. Quote of the price.
       * @param sell_symbol The asset symbol for the sell side of the desired market. Base of the price.
       * @param start The start time of the trade history.
       * @param end The end time of the trade history.
       * @param limit The number of trades to return. Maximum is 1000.
       * @return A list of completed trades.
       */
      std::vector< market_trade > get_trade_history( string buy_symbol, string sell_symbol, time_point start, time_point end, uint32_t limit = 1000 ) const;

      /**
       * @brief Returns the N most recent trades for the specified market.
       * @param buy_symbol The asset symbol for the buy side of the desired market. Quote of the price.
       * @param sell_symbol The asset symbol for the sell side of the desired market. Base of the price.
       * @param limit The number of recent trades to return. Maximum is 1000.
       * @returns A list of completed trades.
       */
       std::vector< market_trade > get_recent_trades( string buy_symbol, string sell_symbol, uint32_t limit = 1000 ) const;

      /**
       * @brief Returns the market history for the specified market.
       * @param buy_symbol The asset symbol for the buy side of the desired market. Quote of the price.
       * @param sell_symbol The asset symbol for the sell side of the desired market. Base of the price.
       * @param seconds The size of durations the history is broken into. The duration size must be configured in the plugin options.
       * @param start The start time to get market history.
       * @param end The end time to get market history
       * @return A list of market history durations.
       */
      std::vector< candle_stick > get_market_history( string buy_symbol, string sell_symbol, uint32_t seconds,
         time_point start, time_point end ) const;

      /**
       * @brief Returns the duration seconds being tracked by the plugin.
       */
      flat_set< uint32_t > get_market_history_durations() const;

   private:
      std::shared_ptr< detail::market_history_api_impl > my;
};

} } // node::market_history

FC_REFLECT( node::market_history::market_ticker,
         (latest)
         (lowest_ask)
         (highest_bid)
         (hour_percent_change)
         (day_percent_change)
         (week_percent_change)
         (month_percent_change)
         (buy_volume)
         (sell_volume)
         );

FC_REFLECT( node::market_history::market_volume,
         (buy_volume)
         (sell_volume)
         );

FC_REFLECT( node::market_history::order,
         (order_price)
         (real_price)
         (buy_asset)
         (sell_asset)
         (created)
         );

FC_REFLECT( node::market_history::order_book,
         (bids)
         (asks)
         );

FC_REFLECT( node::market_history::market_trade,
         (date)
         (current_pays)
         (open_pays)
         );

FC_API( node::market_history::market_history_api,
         (get_ticker)
         (get_volume)
         (get_order_book)
         (get_trade_history)
         (get_recent_trades)
         (get_market_history)
         (get_market_history_durations)
         );