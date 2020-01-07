#pragma once
#include <node/app/plugin.hpp>

#include <node/chain/node_object_types.hpp>

#include <boost/multi_index/composite_key.hpp>

//
// Plugins should #define their SPACE_ID's so plugins with
// conflicting SPACE_ID assignments can be compiled into the
// same binary (by simply re-assigning some of the conflicting #defined
// SPACE_ID's in a build script).
//
// Assignment of SPACE_ID's cannot be done at run-time because
// various template automagic depends on them being known at compile
// time.
//
#ifndef MARKET_HISTORY_SPACE_ID
#define MARKET_HISTORY_SPACE_ID 7
#endif

#ifndef MARKET_HISTORY_PLUGIN_NAME
#define MARKET_HISTORY_PLUGIN_NAME "market_history"
#endif


namespace node { namespace market_history {

using namespace chain;
using node::app::application;

enum market_history_object_types
{
   market_duration_object_type        = ( MARKET_HISTORY_SPACE_ID << 8 ),
   order_history_object_type = ( MARKET_HISTORY_SPACE_ID << 8 ) + 1
};

namespace detail
{
   class market_history_plugin_impl;
}

class market_history_plugin : public node::app::plugin
{
   public:
      market_history_plugin( application* app );
      virtual ~market_history_plugin();

      virtual std::string plugin_name()const override { return MARKET_HISTORY_PLUGIN_NAME; }
      virtual void plugin_set_program_options(
         boost::program_options::options_description& cli,
         boost::program_options::options_description& cfg ) override;
      virtual void plugin_initialize( const boost::program_options::variables_map& options ) override;
      virtual void plugin_startup() override;

      flat_set< uint32_t > get_tracked_durations() const;
      uint32_t get_max_history_per_duration() const;

   private:
      friend class detail::market_history_plugin_impl;
      std::unique_ptr< detail::market_history_plugin_impl > _my;
};


/**
 * Operates to create a candlestick of the trading price history of an asset pair. 
 * Variable duration of candlestick.
 */
struct market_duration_object : public object< market_duration_object_type, market_duration_object >
{
   template< typename Constructor, typename Allocator >
   market_duration_object( Constructor&& c, allocator< Allocator > a )
   {
      c( *this );
   }

   id_type              id;

   fc::time_point       open_time;

   uint32_t             seconds = 0;

   asset_symbol_type    symbol_a;          // Asset with the lower ID. Base of prices.

   asset_symbol_type    symbol_b;          // Asset with the greater ID. Quote of prices.

   price                open_price;        // The price of the first closed order in the time duration.

   price                high_price;        // The price of the highest closed order in the time duration.

   price                low_price;         // The price of the lowest closed order in the time duration.

   price                close_price;       // The price of the last closed order in the time duration.
   
   asset                volume_a;          // The exchanged amount of asset A.

   asset                volume_b;          // The exchanged amount of asset B.

   double open_price_real( asset_symbol_type base )const 
   {
      if( base == symbol_a )
      {
         return open_price.to_real(); 
      }
      else
      {
         return (~open_price).to_real();
      }
   }
   double high_price_real( asset_symbol_type base )const 
   {
      if( base == symbol_a )
      {
         return high_price.to_real(); 
      }
      else
      {
         return (~high_price).to_real();
      }
   }
   double low_price_real( asset_symbol_type base )const 
   {
      if( base == symbol_a )
      {
         return low_price.to_real(); 
      }
      else
      {
         return (~low_price).to_real();
      }
   }
   double close_price_real( asset_symbol_type base )const 
   {
      if( base == symbol_a )
      {
         return close_price.to_real(); 
      }
      else
      {
         return (~close_price).to_real();
      }
   }
};

typedef oid< market_duration_object > market_duration_id_type;

struct order_history_object : public object< order_history_object_type, order_history_object >
{
   template< typename Constructor, typename Allocator >
   order_history_object( Constructor&& c, allocator< Allocator > a )
   {
      c( *this );
   }

   id_type                              id;

   fc::time_point                       time;

   protocol::fill_order_operation       op;

   asset_symbol_type                    symbol_a() const 
   {
      return op.symbol_a;
   }

   asset_symbol_type                    symbol_b() const 
   {
      return op.symbol_b;
   }
};

typedef oid< order_history_object > order_history_id_type;


struct by_duration;
struct by_old_asset_pair;
struct by_new_asset_pair;

typedef multi_index_container<
   market_duration_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< market_duration_object, market_duration_id_type, &market_duration_object::id > >,
      ordered_unique< tag< by_old_asset_pair >,
         composite_key< market_duration_object,
            member< market_duration_object, asset_symbol_type, &market_duration_object::symbol_a >,
            member< market_duration_object, asset_symbol_type, &market_duration_object::symbol_b >,
            member< market_duration_object, uint32_t, &market_duration_object::seconds >,
            member< market_duration_object, time_point, &market_duration_object::open_time >
         >,
         composite_key_compare< 
            std::less< asset_symbol_type >, 
            std::less< asset_symbol_type >, 
            std::less< uint32_t >, 
            std::less< fc::time_point > 
         >
      >,
      ordered_unique< tag< by_new_asset_pair >,
         composite_key< market_duration_object,
            member< market_duration_object, asset_symbol_type, &market_duration_object::symbol_a >,
            member< market_duration_object, asset_symbol_type, &market_duration_object::symbol_b >,
            member< market_duration_object, uint32_t, &market_duration_object::seconds >,
            member< market_duration_object, time_point, &market_duration_object::open_time >
         >,
         composite_key_compare< 
            std::less< asset_symbol_type >, 
            std::less< asset_symbol_type >, 
            std::less< uint32_t >, 
            std::greater< fc::time_point > 
         >
      >,
      ordered_unique< tag< by_duration >,
         composite_key< market_duration_object,
            member< market_duration_object, uint32_t, &market_duration_object::seconds >,
            member< market_duration_object, time_point, &market_duration_object::open_time >
         >,
         composite_key_compare< std::less< uint32_t >, std::less< fc::time_point > >
      >
   >,
   allocator< market_duration_object >
> market_duration_index;

struct by_time;

typedef multi_index_container<
   order_history_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< order_history_object, order_history_id_type, &order_history_object::id > >,
      ordered_non_unique< tag< by_time >, member< order_history_object, time_point, &order_history_object::time > >,
      ordered_unique< tag< by_old_asset_pair >,
         composite_key< order_history_object,
            const_mem_fun< order_history_object, asset_symbol_type, &order_history_object::symbol_a >,
            const_mem_fun< order_history_object, asset_symbol_type, &order_history_object::symbol_b >,
            member< order_history_object, time_point, &order_history_object::time >,
            member< order_history_object, order_history_id_type, &order_history_object::id >
         >,
         composite_key_compare< 
            std::less< asset_symbol_type >,
            std::less< asset_symbol_type >,
            std::less< uint32_t >,
            std::less< fc::time_point >
         >
      >,
      ordered_unique< tag< by_new_asset_pair >,
         composite_key< order_history_object,
            const_mem_fun< order_history_object, asset_symbol_type, &order_history_object::symbol_a >,
            const_mem_fun< order_history_object, asset_symbol_type, &order_history_object::symbol_b >,
            member< order_history_object, time_point, &order_history_object::time >,
            member< order_history_object, order_history_id_type, &order_history_object::id >
         >,
         composite_key_compare< 
            std::less< asset_symbol_type >, 
            std::less< asset_symbol_type >, 
            std::less< uint32_t >, 
            std::greater< fc::time_point > 
         >
      >
   >,
   allocator< order_history_object >
> order_history_index;

} } // node::market_history

FC_REFLECT( node::market_history::market_duration_object,
         (id)
         (open_time)
         (symbol_a)
         (symbol_b)
         (seconds)
         (open_price)
         (high_price)
         (low_price)
         (close_price)
         (volume_a)
         (volume_b) 
         );
                     
CHAINBASE_SET_INDEX_TYPE( node::market_history::market_duration_object, node::market_history::market_duration_index );

FC_REFLECT( node::market_history::order_history_object,
         (id)
         (time)
         (op)
         );
         
CHAINBASE_SET_INDEX_TYPE( node::market_history::order_history_object, node::market_history::order_history_index );