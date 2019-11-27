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

bool database::apply_order( const limit_order_object& new_order_object )
{
   auto order_id = new_order_object.id;
   asset_symbol_type sell_asset_symbol = new_order_object.sell_asset();
   asset_symbol_type recv_asset_symbol = new_order_object.receive_asset();

   // We only need to check if the new order will match with others if it is at the front of the book
   const auto& limit_price_idx = get_index<limit_order_index>().indices().get<by_price>();
   const auto& margin_price_idx = get_index<limit_order_index>().indices().get<by_price>();

   bool match_limit = false;
   bool match_margin = false;
   bool check_pool = false;
   bool match_pool = false;

   auto limit_itr = limit_price_idx.lower_bound( boost::make_tuple( new_order_object.sell_price, order_id ) );
   if( limit_itr != limit_price_idx.begin() )
   {
      --limit_itr;
      if( limit_itr->sell_asset() != sell_asset_symbol || limit_itr->receive_asset() != recv_asset_symbol )
      {
         match_limit = true;
      }   
   }

   auto margin_itr = margin_price_idx.lower_bound( boost::make_tuple( false, new_order_object.sell_price, order_id ) );
   if( margin_itr != margin_price_idx.begin() )
   {
      --margin_itr;
      if( margin_itr->sell_asset() != sell_asset_symbol || margin_itr->receive_asset() != recv_asset_symbol )
      {
         match_margin = true;
      }   
   }

   const asset_object& sell_asset = get_asset( sell_asset_symbol );
   const asset_object& recv_asset = get_asset( recv_asset_symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   price pool_price;

   if( sell_asset.id < recv_asset.id )
   {
      symbol_a = sell_asset_symbol;
      symbol_b = recv_asset_symbol;
   }
   else
   {
      symbol_b = sell_asset_symbol;
      symbol_a = recv_asset_symbol;
   }

   const asset_liquidity_pool_object* liq_ptr = find_liquidity_pool( symbol_a, symbol_b );
   if( liq_ptr != nullptr )
   {
      check_pool = true;
      pool_price = liq_ptr->base_price( new_order_object.sell_price.base.symbol );
      match_pool = pool_price >= new_order_object.sell_price;
   }

   if( !match_limit && !match_margin && !match_pool )
   {
      return false;
   }

   auto max_price = ~new_order_object.sell_price;                  // this is the opposite side (on the book)
   limit_itr = limit_price_idx.lower_bound( max_price.max() );
   auto limit_end = limit_price_idx.upper_bound( max_price );
   margin_itr = margin_price_idx.lower_bound( max_price.max() );
   auto margin_end = margin_price_idx.upper_bound( boost::make_tuple( false, max_price ) );

   if( check_pool )
   {
      pool_price = liq_ptr->base_price( max_price.base.symbol );
   }

   // Order matching should be in favor of the taker.
   // the limit order will only match with a call order if meet all of these:
   // 1. it's buying collateral, which means sell_asset is the MIA, receive_asset is the backing asset.
   // 3. sell_asset is not globally settled
   // 4. sell_asset has a valid price feed
   // 5. the call order's collateral ratio is below or equals to MCR
   // 6. the limit order provided a good price

   bool to_check_call_orders = false;

   const asset_bitasset_data_object* sell_abd_ptr = find_bitasset_data( sell_asset_symbol );
   price call_match_price;

   if( sell_asset.is_market_issued() )
   {
      if( sell_abd_ptr->backing_asset == recv_asset_symbol
         && !sell_abd_ptr->has_settlement()
         && !sell_abd_ptr->current_feed.settlement_price.is_null() )
      {
         call_match_price = ~sell_abd_ptr->current_feed.max_short_squeeze_price();
         if( ~new_order_object.sell_price <= call_match_price ) 
         {
            to_check_call_orders = true;        // new limit order price is good enough to match a call
         }
      }
   }

   bool finished = false;      // whether the new order is gone

   if( to_check_call_orders )
   {
      while( !finished && 
         ( ( limit_itr != limit_end && limit_itr->sell_price > call_match_price ) ||
         ( margin_itr != margin_end && margin_itr->sell_price > call_match_price ) ||
         ( check_pool && pool_price > call_match_price ) ) )   // check limit/margin/pool orders first, match the ones with better price in comparison to call orders
      {
         auto old_limit_itr = limit_itr;
         auto old_margin_itr = margin_itr;
         
         if( check_pool )
         {
            price book_price = std::max( limit_itr->sell_price, margin_itr->sell_price );
            pool_price = liq_ptr->base_price( old_limit_itr->sell_price.base.symbol );
            if( pool_price > book_price )
            {
               finished = ( match( new_order_object, *liq_ptr, book_price ) != 2 );
               continue;   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            }
         }

         if( limit_itr->sell_price > margin_itr->sell_price )
         {
            ++limit_itr;
            finished = ( match( new_order_object, *old_limit_itr, old_limit_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
         else
         {
            ++margin_itr;
            finished = ( match( new_order_object, *old_margin_itr, old_margin_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
      }

      if( !finished ) // check if there are margin calls
      {
         const auto& call_collateral_idx = get_index<call_order_index>().indices().get<by_collateral>();
         auto call_min = price::min( recv_asset_symbol, sell_asset_symbol );
         while( !finished )
         {
            auto call_itr = call_collateral_idx.lower_bound( call_min );    // check call order with least collateral ratio
            if( call_itr == call_collateral_idx.end() || call_itr->debt_type() != sell_asset_symbol || 
               call_itr->collateralization() > sell_abd_ptr->current_maintenance_collateralization )
            {
               break;
            }
               
            int match_result = match( new_order_object, *call_itr, call_match_price,
                                      sell_abd_ptr->current_feed.settlement_price,
                                      sell_abd_ptr->current_feed.maintenance_collateral_ratio,
                                      sell_abd_ptr->current_maintenance_collateralization );
            
            if( match_result == 1 || match_result == 3 )   // match returns 1 or 3 when the new order was fully filled. In this case, we stop matching; otherwise keep matching.
            {
               finished = true;
            }
         }
      }
   }

   while( !finished && ( ( limit_itr != limit_end ) || ( margin_itr != margin_end ) || ( check_pool ) ) )
      {
         auto old_limit_itr = limit_itr;
         auto old_margin_itr = margin_itr;
         
         if( check_pool )  // Match with liquidity pool if present for this price pair
         {
            price book_price = std::max( limit_itr->sell_price, margin_itr->sell_price );
            pool_price = liq_ptr->base_price( old_limit_itr->sell_price.base.symbol );
            if( pool_price > book_price )
            {
               finished = ( match( new_order_object, *liq_ptr, book_price ) != 2 );
               continue;   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            }
         }

         if( limit_itr->sell_price > margin_itr->sell_price )  // Match with higher price of available margin and limit orders.
         {
            ++limit_itr;
            finished = ( match( new_order_object, *old_limit_itr, old_limit_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
         else
         {
            ++margin_itr;
            finished = ( match( new_order_object, *old_margin_itr, old_margin_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
      }

   const limit_order_object* updated_order_object = find< limit_order_object >( order_id );
   if( updated_order_object == nullptr )
   {
      return true;
   }
   else
   {
      return maybe_cull_small_order( *updated_order_object );
   } 
}


bool database::apply_order( const margin_order_object& new_order_object )
{
   auto order_id = new_order_object.id;
   asset_symbol_type sell_asset_symbol = new_order_object.sell_asset();
   asset_symbol_type recv_asset_symbol = new_order_object.receive_asset();

   // We only need to check if the new order will match with others if it is at the front of the book
   const auto& limit_price_idx = get_index<limit_order_index>().indices().get<by_price>();
   const auto& margin_price_idx = get_index<limit_order_index>().indices().get<by_price>();

   bool match_limit = false;
   bool match_margin = false;
   bool check_pool = false;
   bool match_pool = false;

   auto limit_itr = limit_price_idx.lower_bound( boost::make_tuple( new_order_object.sell_price, order_id ) );
   if( limit_itr != limit_price_idx.begin() )
   {
      --limit_itr;
      if( limit_itr->sell_asset() != sell_asset_symbol || limit_itr->receive_asset() != recv_asset_symbol )
      {
         match_limit = true;
      }   
   }

   auto margin_itr = margin_price_idx.lower_bound( boost::make_tuple( false, new_order_object.sell_price, order_id ) );
   if( margin_itr != margin_price_idx.begin() )
   {
      --margin_itr;
      if( margin_itr->sell_asset() != sell_asset_symbol || margin_itr->receive_asset() != recv_asset_symbol )
      {
         match_margin = true;
      }   
   }

   const asset_object& sell_asset = get_asset( sell_asset_symbol );
   const asset_object& recv_asset = get_asset( recv_asset_symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   price pool_price;

   if( sell_asset.id < recv_asset.id )
   {
      symbol_a = sell_asset_symbol;
      symbol_b = recv_asset_symbol;
   }
   else
   {
      symbol_b = sell_asset_symbol;
      symbol_a = recv_asset_symbol;
   }

   const asset_liquidity_pool_object* liq_ptr = find_liquidity_pool( symbol_a, symbol_b );
   if( liq_ptr != nullptr )
   {
      check_pool = true;
      pool_price = liq_ptr->base_price( new_order_object.sell_price.base.symbol );
      match_pool = pool_price >= new_order_object.sell_price;
   }

   if( !match_limit && !match_margin && !match_pool )
   {
      return false;
   }

   auto max_price = ~new_order_object.sell_price;                  // this is the opposite side (on the book)
   limit_itr = limit_price_idx.lower_bound( max_price.max() );
   auto limit_end = limit_price_idx.upper_bound( max_price );
   margin_itr = margin_price_idx.lower_bound( max_price.max() );
   auto margin_end = margin_price_idx.upper_bound( boost::make_tuple( false, max_price ) );

   if( check_pool )
   {
      pool_price = liq_ptr->base_price( max_price.base.symbol );
   }

   // Order matching should be in favor of the taker.
   // the limit order will only match with a call order if meet all of these:
   // 1. it's buying collateral, which means sell_asset is the MIA, receive_asset is the backing asset.
   // 3. sell_asset is not globally settled
   // 4. sell_asset has a valid price feed
   // 5. the call order's collateral ratio is below or equals to MCR
   // 6. the limit order provided a good price

   bool to_check_call_orders = false;

   const asset_bitasset_data_object* sell_abd_ptr = find_bitasset_data( sell_asset_symbol );
   price call_match_price;

   if( sell_asset.is_market_issued() )
   {
      if( sell_abd_ptr->backing_asset == recv_asset_symbol
         && !sell_abd_ptr->has_settlement()
         && !sell_abd_ptr->current_feed.settlement_price.is_null() )
      {
         call_match_price = ~sell_abd_ptr->current_feed.max_short_squeeze_price();
         if( ~new_order_object.sell_price <= call_match_price ) 
         {
            to_check_call_orders = true;        // new limit order price is good enough to match a call
         }
      }
   }

   bool finished = false;      // whether the new order is gone

   if( to_check_call_orders )
   {
      while( !finished && 
         ( ( limit_itr != limit_end && limit_itr->sell_price > call_match_price ) ||
         ( margin_itr != margin_end && margin_itr->sell_price > call_match_price ) ||
         ( check_pool && pool_price > call_match_price ) ) )   // check limit/margin/pool orders first, match the ones with better price in comparison to call orders
      {
         auto old_limit_itr = limit_itr;
         auto old_margin_itr = margin_itr;
         
         if( check_pool )
         {
            price book_price = std::max( limit_itr->sell_price, margin_itr->sell_price );
            pool_price = liq_ptr->base_price( old_limit_itr->sell_price.base.symbol );
            if( pool_price > book_price )
            {
               finished = ( match( new_order_object, *liq_ptr, book_price ) != 2 );
               continue;   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            }
         }

         if( limit_itr->sell_price > margin_itr->sell_price )
         {
            ++limit_itr;
            finished = ( match( new_order_object, *old_limit_itr, old_limit_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
         else
         {
            ++margin_itr;
            finished = ( match( new_order_object, *old_margin_itr, old_margin_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
      }

      if( !finished ) // check if there are margin calls
      {
         const auto& call_collateral_idx = get_index<call_order_index>().indices().get<by_collateral>();
         auto call_min = price::min( recv_asset_symbol, sell_asset_symbol );
         while( !finished )
         {
            auto call_itr = call_collateral_idx.lower_bound( call_min );    // check call order with least collateral ratio
            if( call_itr == call_collateral_idx.end() || call_itr->debt_type() != sell_asset_symbol || 
               call_itr->collateralization() > sell_abd_ptr->current_maintenance_collateralization )
            {
               break;
            }
               
            int match_result = match( new_order_object, *call_itr, call_match_price,
                                      sell_abd_ptr->current_feed.settlement_price,
                                      sell_abd_ptr->current_feed.maintenance_collateral_ratio,
                                      sell_abd_ptr->current_maintenance_collateralization );
            
            if( match_result == 1 || match_result == 3 )   // match returns 1 or 3 when the new order was fully filled. In this case, we stop matching; otherwise keep matching.
            {
               finished = true;
            }
         }
      }
   }

   while( !finished && ( ( limit_itr != limit_end ) || ( margin_itr != margin_end ) || ( check_pool ) ) )
      {
         auto old_limit_itr = limit_itr;
         auto old_margin_itr = margin_itr;
         
         if( check_pool )  // Match with liquidity pool if present for this price pair
         {
            price book_price = std::max( limit_itr->sell_price, margin_itr->sell_price );
            pool_price = liq_ptr->base_price( old_limit_itr->sell_price.base.symbol );
            if( pool_price > book_price )
            {
               finished = ( match( new_order_object, *liq_ptr, book_price ) != 2 );
               continue;   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            }
         }

         if( limit_itr->sell_price > margin_itr->sell_price )  // Match with higher price of available margin and limit orders.
         {
            ++limit_itr;
            finished = ( match( new_order_object, *old_limit_itr, old_limit_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
         else
         {
            ++margin_itr;
            finished = ( match( new_order_object, *old_margin_itr, old_margin_itr->sell_price ) != 2 );   // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
      }

   const margin_order_object& updated_order_object = get< margin_order_object >( order_id );
   if( updated_order_object.filled() )
   {
      return true;
   }
   else
   {
      return maybe_cull_small_order( updated_order_object );
   } 
}

/**
 *  Matches the two orders, the first parameter is taker, the second is maker.
 *  1 - taker was filled.
 *  2 - maker was filled.
 *  3 - both were filled.
 */
int database::match( const limit_order_object& taker, const limit_order_object& maker, const price& match_price )
{
   FC_ASSERT( taker.sell_price.quote.symbol == maker.sell_price.base.symbol );
   FC_ASSERT( taker.sell_price.base.symbol  == maker.sell_price.quote.symbol );
   FC_ASSERT( taker.amount_for_sale().amount > 0 && maker.amount_for_sale().amount > 0 );

   asset taker_for_sale = taker.amount_for_sale();
   asset maker_for_sale = maker.amount_for_sale();
   const asset_object& taker_asset = get_asset( taker_for_sale.symbol );
   const asset_object& maker_asset = get_asset( maker_for_sale.symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   if( taker_asset.id < maker_asset.id )
   {
      symbol_a = taker_asset.symbol;
      symbol_b = maker_asset.symbol;
   }
   else
   {
      symbol_a = maker_asset.symbol;
      symbol_b = taker_asset.symbol;
   }
   
   asset taker_pays, taker_receives, maker_pays, maker_receives;

   bool cull_taker = false;
   if( taker_for_sale <= maker_for_sale * match_price ) // rounding down here should be fine
   {
      taker_receives  = taker_for_sale * match_price; // round down, in favor of bigger order
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_receives.multiply_and_round_up( match_price );
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price; // round down, in favor of bigger order
      taker_receives = maker_receives.multiply_and_round_up( match_price );
   }

   maker_pays = taker_receives;
   taker_pays = maker_receives;

   int result = 0;

   result |= fill_limit_order(      
      taker,      // The first order matched is taker
      taker_pays, 
      taker_receives, 
      cull_taker, 
      match_price, 
      false, 
      maker.interface 
   );

   result |= fill_limit_order( 
      maker,       // The second order is maker
      maker_pays, 
      maker_receives, 
      true, 
      match_price, 
      true, 
      taker.interface 
   ) << 1;      
   FC_ASSERT( result != 0 );

   push_virtual_operation(    // Record matched trading details for market history API.
      fill_order_operation( 
         taker.seller, 
         to_string(taker.order_id), 
         taker_pays, 
         maker.seller, 
         to_string(maker.order_id), 
         maker_pays, 
         symbol_a, 
         symbol_b
      )
   );

   return result;
}

/**
 *  Matches the two orders, the first parameter is taker, the second is maker.
 *  1 - taker was filled.
 *  2 - maker was filled.
 *  3 - both were filled.
 */
int database::match( const margin_order_object& taker, const margin_order_object& maker, const price& match_price )
{
   FC_ASSERT( taker.sell_price.quote.symbol == maker.sell_price.base.symbol );
   FC_ASSERT( taker.sell_price.base.symbol  == maker.sell_price.quote.symbol );
   FC_ASSERT( taker.amount_for_sale().amount > 0 && maker.amount_for_sale().amount > 0 );

   asset taker_for_sale = taker.amount_for_sale();
   asset maker_for_sale = maker.amount_for_sale();
   const asset_object& taker_asset = get_asset( taker_for_sale.symbol );
   const asset_object& maker_asset = get_asset( maker_for_sale.symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   if( taker_asset.id < maker_asset.id )
   {
      symbol_a = taker_asset.symbol;
      symbol_b = maker_asset.symbol;
   }
   else
   {
      symbol_a = maker_asset.symbol;
      symbol_b = taker_asset.symbol;
   }

   asset taker_pays, taker_receives, maker_pays, maker_receives;

   bool cull_taker = false;
   if( taker_for_sale <= maker_for_sale * match_price ) // rounding down here should be fine
   {
      taker_receives  = taker_for_sale * match_price; // round down, in favor of bigger order
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_receives.multiply_and_round_up( match_price );
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price; // round down, in favor of bigger order
      taker_receives = maker_receives.multiply_and_round_up( match_price );
   }

   maker_pays = taker_receives;
   taker_pays = maker_receives;

   int result = 0;
   result |= fill_margin_order(      
      taker,      // The first order matched is taker
      taker_pays, 
      taker_receives, 
      cull_taker, 
      match_price, 
      false, 
      maker.interface 
   );

   result |= fill_margin_order( 
      maker,       // The second order is maker
      maker_pays, 
      maker_receives, 
      true, 
      match_price, 
      true, 
      taker.interface 
   ) << 1;      
   FC_ASSERT( result != 0 );

   push_virtual_operation(    // Record matched trading details for market history API.
      fill_order_operation( 
         taker.owner, 
         to_string(taker.order_id), 
         taker_pays, 
         maker.owner, 
         to_string(maker.order_id), 
         maker_pays, 
         symbol_a, 
         symbol_b
      )
   );
   return result;
}

/**
 *  Matches the two orders, the first parameter is taker, the second is maker.
 *  1 - taker was filled.
 *  2 - maker was filled.
 *  3 - both were filled.
 */
int database::match( const limit_order_object& taker, const margin_order_object& maker, const price& match_price )
{
   FC_ASSERT( taker.sell_price.quote.symbol == maker.sell_price.base.symbol );
   FC_ASSERT( taker.sell_price.base.symbol  == maker.sell_price.quote.symbol );
   FC_ASSERT( taker.amount_for_sale().amount > 0 && maker.amount_for_sale().amount > 0 );

   asset taker_for_sale = taker.amount_for_sale();
   asset maker_for_sale = maker.amount_for_sale();
   const asset_object& taker_asset = get_asset( taker_for_sale.symbol );
   const asset_object& maker_asset = get_asset( maker_for_sale.symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   if( taker_asset.id < maker_asset.id )
   {
      symbol_a = taker_asset.symbol;
      symbol_b = maker_asset.symbol;
   }
   else
   {
      symbol_a = maker_asset.symbol;
      symbol_b = taker_asset.symbol;
   }

   asset taker_pays, taker_receives, maker_pays, maker_receives;

   bool cull_taker = false;
   if( taker_for_sale <= maker_for_sale * match_price ) // rounding down here should be fine
   {
      taker_receives  = taker_for_sale * match_price; // round down, in favor of bigger order
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_receives.multiply_and_round_up( match_price );
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price; // round down, in favor of bigger order
      taker_receives = maker_receives.multiply_and_round_up( match_price );
   }

   maker_pays = taker_receives;
   taker_pays = maker_receives;

   int result = 0;
   result |= fill_limit_order(      
      taker,      // The first order matched is taker
      taker_pays, 
      taker_receives, 
      cull_taker, 
      match_price, 
      false, 
      maker.interface 
   );

   result |= fill_margin_order( 
      maker,       // The second order is maker
      maker_pays, 
      maker_receives, 
      true, 
      match_price, 
      true, 
      taker.interface 
   ) << 1;      
   FC_ASSERT( result != 0 );

   push_virtual_operation(    // Record matched trading details for market history API.
      fill_order_operation( 
         taker.seller, 
         to_string(taker.order_id), 
         taker_pays, 
         maker.owner, 
         to_string(maker.order_id), 
         maker_pays, 
         symbol_a, 
         symbol_b
      )
   );
   return result;
}

/**
 *  Matches the two orders, the first parameter is taker, the second is maker.
 *  1 - taker was filled.
 *  2 - maker was filled.
 *  3 - both were filled.
 */
int database::match( const margin_order_object& taker, const limit_order_object& maker, const price& match_price )
{
   FC_ASSERT( taker.sell_price.quote.symbol == maker.sell_price.base.symbol );
   FC_ASSERT( taker.sell_price.base.symbol  == maker.sell_price.quote.symbol );
   FC_ASSERT( taker.amount_for_sale().amount > 0 && maker.amount_for_sale().amount > 0 );

   asset taker_for_sale = taker.amount_for_sale();
   asset maker_for_sale = maker.amount_for_sale();
   const asset_object& taker_asset = get_asset( taker_for_sale.symbol );
   const asset_object& maker_asset = get_asset( maker_for_sale.symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   if( taker_asset.id < maker_asset.id )
   {
      symbol_a = taker_asset.symbol;
      symbol_b = maker_asset.symbol;
   }
   else
   {
      symbol_a = maker_asset.symbol;
      symbol_b = taker_asset.symbol;
   }

   asset taker_pays, taker_receives, maker_pays, maker_receives;

   bool cull_taker = false;
   if( taker_for_sale <= maker_for_sale * match_price )      // rounding down here should be fine
   {
      taker_receives  = taker_for_sale * match_price;        // round down, in favor of bigger order
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_receives.multiply_and_round_up( match_price );
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price;        // round down, in favor of bigger order
      taker_receives = maker_receives.multiply_and_round_up( match_price );
   }

   maker_pays = taker_receives;
   taker_pays = maker_receives;

   int result = 0;
   result |= fill_margin_order(      
      taker,      // The first order matched is taker
      taker_pays, 
      taker_receives, 
      cull_taker, 
      match_price, 
      false, 
      maker.interface 
   );

   result |= fill_limit_order( 
      maker,       // The second order is maker
      maker_pays, 
      maker_receives, 
      true, 
      match_price, 
      true, 
      taker.interface 
   ) << 1;      
   FC_ASSERT( result != 0 );

   push_virtual_operation(    // Record matched trading details for market history API.
      fill_order_operation( 
         taker.owner, 
         to_string(taker.order_id), 
         taker_pays, 
         maker.seller, 
         to_string(maker.order_id), 
         maker_pays, 
         symbol_a, 
         symbol_b
      )
   );
   return result;
}

/**
 * Matches a limit order against an asset liquidity pool
 * by liquid limit exchanging the asset up to the match price.
 *  1 - taker was filled.
 *  2 - taker was not filled.
 */
int database::match( const limit_order_object& taker, const asset_liquidity_pool_object& pool, const price& match_price )
{
   FC_ASSERT( taker.amount_for_sale().amount > 0 );
   auto taker_for_sale = taker.amount_for_sale();
   asset taker_pays, taker_receives;

   pair<asset, asset> exchange = liquid_limit_exchange( taker_for_sale, match_price, pool, true, true );
   taker_pays = exchange.first;
   taker_receives = exchange.second;

   bool result = fill_limit_order( taker, taker_pays, taker_receives, true, match_price, false, taker.interface );     // the first param is taker

   if( result )
   {
      return 1;
   }
   else
   {
      return 2;
   }
}

/**
 * Matches a margin order against an asset liquidity pool
 * by liquid limit exchanging the asset up to the match price.
 * 1 - taker was filled.
 * 2 - taker was not filled.
 */
int database::match( const margin_order_object& taker, const asset_liquidity_pool_object& pool, const price& match_price )
{
   FC_ASSERT( taker.amount_for_sale().amount > 0 );
   auto taker_for_sale = taker.amount_for_sale();
   asset taker_pays, taker_receives;

   pair<asset, asset> exchange = liquid_limit_exchange( taker_for_sale, match_price, pool, true, true );
   taker_pays = exchange.first;
   taker_receives = exchange.second;

   bool result = fill_margin_order( taker, taker_pays, taker_receives, true, match_price, false, taker.interface );     // the first param is taker

   if( result )
   {
      return 1;
   }
   else
   {
      return 2;
   }
}

int database::match( const limit_order_object& bid, const call_order_object& ask, const price& match_price, 
   const price& feed_price, const uint16_t maintenance_collateral_ratio, const optional<price>& maintenance_collateralization )
{
   FC_ASSERT( bid.sell_asset() == ask.debt_type() );
   FC_ASSERT( bid.receive_asset() == ask.collateral_type() );
   FC_ASSERT( bid.amount_for_sale().amount > 0 && ask.debt.amount > 0 && ask.collateral.amount > 0 );

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
   result |= fill_limit_order( bid, order_pays, order_receives, cull_taker, match_price, false, ask.interface );     // the limit order is taker
   result |= fill_call_order( ask, call_pays, call_receives, match_price, true, bid.interface, false ) << 1;                // the call order is maker
   // result can be 0 when call order has target_collateral_ratio option set.

   return result;
}

int database::match( const margin_order_object& bid, const call_order_object& ask, const price& match_price,
   const price& feed_price, const uint16_t maintenance_collateral_ratio, const optional<price>& maintenance_collateralization )
{
   FC_ASSERT( bid.sell_asset() == ask.debt_type() );
   FC_ASSERT( bid.receive_asset() == ask.collateral_type() );
   FC_ASSERT( bid.amount_for_sale().amount > 0 && ask.debt.amount > 0 && ask.collateral.amount > 0 );

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
   result |= fill_margin_order( bid, order_pays, order_receives, cull_taker, match_price, false, ask.interface );     // the limit order is taker
   result |= fill_call_order( ask, call_pays, call_receives, match_price, true, bid.interface, false ) << 1;                 // the call order is maker
   // result can be 0 when call order has target_collateral_ratio option set.

   return result;
}

asset database::match( const call_order_object& call, const force_settlement_object& settle, const price& match_price, asset max_settlement, const price& fill_price )
{ try {
   FC_ASSERT(call.debt_type() == settle.balance.symbol );
   FC_ASSERT(call.debt.amount > 0 && call.collateral.amount > 0 && settle.balance.amount > 0);

   auto settle_for_sale = std::min(settle.balance, max_settlement);
   auto call_debt = call.amount_to_receive();

   asset call_receives  = std::min(settle_for_sale, call_debt);
   asset call_pays = call_receives * match_price;       // round down here, in favor of call order, for first check

   bool cull_settle_order = false;       // whether need to cancel dust settle order
   if( call_pays.amount == 0 )
   {
      if( call_receives == call_debt ) // the call order is smaller than or equal to the settle order
      {
         wlog( "Something for nothing issue (#184, variant C-1) handled at block #${block}", ("block", head_block_num()) );
         call_pays.amount = 1;
      }
      else
      {
         if( call_receives == settle.balance ) // the settle order is smaller
         {
            wlog( "Something for nothing issue (#184, variant C-2) handled at block #${block}", ("block",head_block_num()) );
            cancel_settle_order( settle, true );
         }

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
         if( call_receives == settle.balance ) // the settle order will be completely filled, assuming we need to cull it
         {
            cull_settle_order = true;
         }
         call_receives = call_pays.multiply_and_round_up( match_price ); 

         if( call_receives == settle.balance ) // the settle order will be completely filled, no need to cull
         {
            cull_settle_order = false;
         }
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

   fill_call_order( call, call_pays, call_receives, fill_price, true, settle.interface, false );                  // call order is maker
   fill_settle_order( settle, settle_pays, settle_receives, fill_price, false, call.interface );           // force settlement order is taker

   if( cull_settle_order )
   {
      cancel_settle_order( settle, true );
   }
   
   return call_receives;
} FC_CAPTURE_AND_RETHROW( (call)(settle)(match_price)(max_settlement) ) }

/**
 * Fills a limit order against another order, until the asset remaining to sell
 * is all sold, or the order is cancelled.
 */
bool database::fill_limit_order( const limit_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
   const price& fill_price, const bool is_maker, const account_name_type& match_interface )
{ try {
   FC_ASSERT( order.amount_for_sale().symbol == pays.symbol );
   FC_ASSERT( pays.symbol != receives.symbol );

   const account_object& seller = get_account(order.seller);
   const asset_object& recv_asset = get_asset(receives.symbol);
   const auto& fee_asset_dyn_data = get_dynamic_data(receives.symbol);
   asset issuer_fees = asset( 0, receives.symbol );
   asset trading_fees = asset( 0, receives.symbol );
   asset fees_paid = asset( 0, receives.symbol );

   if( !is_maker )  // Pay fees if we are taker order
   {
      issuer_fees = pay_issuer_fees(seller, recv_asset, receives);      // fees paid to issuer of the asset
      trading_fees = pay_trading_fees(seller, receives, match_interface, order.interface );      // fees paid to the protocol, and interfaces
      fees_paid = issuer_fees + trading_fees;
   }

   asset delta = receives - fees_paid;

   adjust_pending_supply( -delta );
   adjust_liquid_balance( seller, delta );

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
      });

      if( cull_if_small )
      {
         return maybe_cull_small_order( order );
      }
      return false;
   }
} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) ) }


/**
 * Fills a margin order against another order, accumulating the position asset into
 * the order, until the order is liquidated or filled. 
 * Upon liquidation, the order executes in reverse, selling the position to repurchase the 
 * debt asset, and become closed out.
 */
bool database::fill_margin_order( const margin_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
   const price& fill_price, const bool is_maker, const account_name_type& match_interface )
{ try {
   FC_ASSERT( order.amount_for_sale().symbol == pays.symbol );
   FC_ASSERT( pays.symbol != receives.symbol );

   const account_object& seller = get_account(order.owner);
   const asset_object& recv_asset = get_asset(receives.symbol);
   asset issuer_fees = asset( 0, receives.symbol );
   asset trading_fees = asset( 0, receives.symbol );
   asset fees_paid = asset( 0, receives.symbol );

   if( !is_maker )  // Pay fees if we are taker order
   {
      issuer_fees = pay_issuer_fees(seller, recv_asset, receives);      // fees paid to issuer of the asset
      trading_fees = pay_trading_fees(seller, receives, match_interface, order.interface );      // fees paid to the protocol, and interfaces
      fees_paid = issuer_fees + trading_fees;
   }

   asset delta = receives - fees_paid;

   modify( order, [&]( margin_order_object& m ) 
   {
      if( m.liquidating )   // If liquidating, we are paying position asset to repurchase debt.
      {
         m.debt_balance += delta;
         m.position_balance -= pays;
      }
      else   // If not liquidating, we are paying debt to purchase position asset.
      {
         m.debt_balance -= pays; 
         m.position_balance += delta;
      }
   });

   if( cull_if_small )
   {
      return maybe_cull_small_order( order );
   }
   else
   {
      return order.filled();
   }
} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) ) }

/**
 * Fills a call order that is below maintenance collateral ratio
 * against another order.
 */
bool database::fill_call_order( const call_order_object& order, const asset& pays, const asset& receives, const price& fill_price, 
   const bool is_maker, const account_name_type& match_interface, bool global_settle = false )
{ try {
   FC_ASSERT( order.debt_type() == receives.symbol );
   FC_ASSERT( order.collateral_type() == pays.symbol );
   FC_ASSERT( order.collateral.amount >= pays.amount );
   const account_object& seller = get_account( order.borrower );
   const asset_object& recv_asset = get_asset( receives.symbol );
   asset issuer_fees;
   asset trading_fees;
   asset fees_paid = asset( 0, pays.symbol );
   FC_ASSERT( recv_asset.is_market_issued() );

   optional<asset> collateral_freed;

   if( !is_maker )  // Pay fees if we are taker order
   {
      issuer_fees = pay_issuer_fees( seller, recv_asset, pays );      // fees paid to issuer of the asset
      trading_fees = pay_trading_fees( seller, pays, match_interface, order.interface );      // fees paid to the protocol, and interfaces
      fees_paid = issuer_fees + trading_fees;
   }

   asset total_paid = pays + fees_paid;

   modify( order, [&]( call_order_object& o )
   {
      o.debt -= receives.amount;
      o.collateral -= total_paid.amount;
      if( o.debt.amount == 0 )
      {
         collateral_freed = o.amount_for_sale();
         o.collateral.amount = 0;
      }
   });

   if( !global_settle )
   {
      adjust_pending_supply( -receives );    // reduce the pending supply of the bitasset, as it has been repaid, unless globally settling
   }

   if( collateral_freed.valid() )
   {
      adjust_pending_supply( -*collateral_freed );
      adjust_liquid_balance( order.borrower, *collateral_freed );     // Return collateral when freed.
   }

   if( collateral_freed.valid() )
   {
      remove( order );
   }
   
   return collateral_freed.valid();
} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) ) }

/**
 * Executes a force settlement order filling from it being matched against a call order.
 */
bool database::fill_settle_order( const force_settlement_object& settle, const asset& pays, const asset& receives, 
   const price& fill_price, const bool is_maker, const account_name_type& match_interface )
{ try {
   FC_ASSERT( pays.symbol != receives.symbol );
   const asset_object& rec_asset = get_asset(receives.symbol);
   const account_object& owner = get_account( settle.owner );
   bool filled = false;

   auto issuer_fees = pay_issuer_fees(rec_asset, receives);
   auto trading_fees = pay_trading_fees( owner, receives, match_interface, settle.interface );  // Settlement order is always taker. 
   auto fees_paid = issuer_fees + trading_fees;

   if( pays < settle.balance )
   {
      modify(settle, [&]( force_settlement_object& s )
      {
         s.balance -= pays;
      });
      filled = false;
   } 
   else 
   {
      filled = true;
   }

   asset delta = receives - fees_paid;
   adjust_liquid_balance( settle.owner, delta);
   adjust_pending_supply( -delta);

   if(filled)
   {
      remove(settle);
   }

   return filled;

} FC_CAPTURE_AND_RETHROW( (settle)(pays)(receives)(fill_price)(is_maker) ) }

/**
 * Adds an asset into a liquidity pool
 * and receives the pool's liquidity pool asset,
 * which earns a portion of fees from trading through the pool.
 */
void database::liquid_fund( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool)
{ try {
   asset liquid = get_liquid_balance( account.name, input.symbol );
   FC_ASSERT( liquid >= input, 
      "Account: ${a} does not have enough liquid balance to fund requested amount: ${i}.",("a", account.name)("i", input) );
   FC_ASSERT( liquid.symbol == pool.symbol_a || liquid.symbol == pool.symbol_b, 
      "Invalid symbol input to liquidity pool: ${s}.",("s", input.symbol) );

   uint128_t pr = BLOCKCHAIN_PRECISION;
   uint128_t pr_sq = BLOCKCHAIN_PRECISION * BLOCKCHAIN_PRECISION;
   uint128_t sup = pool.balance_liquid.amount.value;
   uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
   uint128_t in = input.amount.value;

   uint128_t return_amount = ( sup * ( approx_sqrt( pr_sq + ( ( pr_sq * in ) / ib ) ) - pr  ) ) / pr;
   asset return_asset = asset( return_amount, pool.symbol_liquid );
   
   adjust_liquid_balance( account.name, -input );

   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( input.symbol == p.symbol_a )
      {
         p.balance_a += input;
      }
      else if( input.symbol == p.symbol_b )
      {
         p.balance_b += input;
      }
      p.balance_liquid += return_asset;
   });

   adjust_liquid_balance( account.name, return_asset );

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }


/**
 * Withdraws a pool's liquidity asset for some of its underlying assets,
 * lowering the total supply of the pool's liquidity asset
 */
void database::liquid_withdraw( const asset& input, const asset_symbol_type& receive, const account_object& account, const asset_liquidity_pool_object& pool)
{ try {
   asset liquid = get_liquid_balance( account.name, input.symbol );
   FC_ASSERT( liquid >= input, 
      "Account: ${a} does not have enough liquid balance to withdraw requested amount: ${i}.",("a", account.name)("i", input) );

   uint128_t pr = BLOCKCHAIN_PRECISION;
   uint128_t pr_sq = BLOCKCHAIN_PRECISION * BLOCKCHAIN_PRECISION;
   uint128_t sup = pool.balance_liquid.amount.value;
   uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
   uint128_t rb = pool.asset_balance( receive ).amount.value;
   uint128_t in = input.amount.value;

   uint128_t var = pr - ( ( in * pr ) / sup );
   uint128_t return_amount = ( rb * ( pr_sq - ( var * var ) ) ) / pr_sq;
   asset return_asset = asset( return_amount, receive );
   
   adjust_liquid_balance( account.name, -input );

   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      p.balance_liquid -= input;
      if( receive == p.symbol_a )
      {
         p.balance_a -= return_asset;
      }
      else if( input.symbol == p.symbol_b )
      {
         p.balance_b -= return_asset ;
      }
   });

   adjust_liquid_balance( account.name, return_asset );

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }

/**
 * Exchanges an asset for any other asset in the network
 * by using the core asset as a liquidity pathway.
 */
asset database::liquid_exchange( const asset& input, const asset_symbol_type& receive, bool execute = true, bool apply_fees = true )
{ try {
   FC_ASSERT( input.symbol != receive,
      "Assets must have different symbols to exchange.");

   asset coin_input;

   if( input.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& input_pool = get_liquidity_pool( SYMBOL_COIN, input.symbol );

      uint128_t pr = BLOCKCHAIN_PRECISION;
      uint128_t ib = input_pool.asset_balance( input.symbol ).amount.value;
      uint128_t rb = input_pool.asset_balance( SYMBOL_COIN ).amount.value;
      uint128_t in = input.amount.value;

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;

      asset total_fees = asset( ( return_amount * TRADING_FEE_PERCENT ) / PERCENT_100,  SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;
   
      asset return_asset = asset( return_amount, SYMBOL_COIN );

      if( apply_fees )
      {
         return_asset -= total_fees;
      }
      
      if( execute )
      {
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
         modify( input_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a -= return_asset;
            p.balance_b += input ; 
         });
      }

      coin_input = return_asset;
   }
   else
   {
      coin_input = input;
   }

   if( receive != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& receive_pool = get_liquidity_pool( SYMBOL_COIN, receive );

      asset total_fees = asset( ( coin_input.amount.value * TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;

      if( apply_fees )
      {
         coin_input -= total_fees;
      }

      uint128_t in = coin_input.amount.value;
      uint128_t pr = BLOCKCHAIN_PRECISION;
      uint128_t ib = receive_pool.asset_balance( SYMBOL_COIN ).amount.value;
      uint128_t rb = receive_pool.asset_balance( receive ).amount.value;

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;
      asset return_asset = asset( return_amount, receive );
      
      if( execute )
      {
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
         modify( receive_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a += coin_input;
            p.balance_b -= return_asset ; 
         });
      }

      return return_asset;
   }
   else
   {
      return coin_input;
   }
} FC_CAPTURE_AND_RETHROW( (input)(receive) ) }


void database::liquid_exchange( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool, const account_object& int_account)
{ try {
   asset total_fees;
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   uint128_t pr = BLOCKCHAIN_PRECISION;
   uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
   uint128_t rb = pool.asset_balance( rec ).amount.value;
   uint128_t in = input.amount.value;

   if( input.symbol == SYMBOL_COIN )
   {
      total_fees = asset( ( input.amount.value * TRADING_FEE_PERCENT ) / PERCENT_100,  SYMBOL_COIN );
      in -= total_fees.amount.value;
   }

   uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;

   if( input.symbol != SYMBOL_COIN )
   {
      total_fees = asset( ( return_amount * TRADING_FEE_PERCENT ) / PERCENT_100,  rec );
      return_amount -= total_fees.amount.value;
   }
   
   asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset pool_fees = total_fees - network_fees - interface_fees;
   
   asset return_asset = asset( return_amount, SYMBOL_COIN );
   
   adjust_liquid_balance( account.name, -input );
   pay_network_fees( account, network_fees );
   pay_fee_share( int_account, interface_fees );

   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( input.symbol == p.symbol_a )
      {
         p.balance_a += input; 
         p.balance_b -= return_asset;
      }
      else if( input.symbol == p.symbol_b )
      {
         p.balance_b += input; 
         p.balance_a -= return_asset;
      }
      if( pool_fees.symbol == p.symbol_a )
      {
         p.balance_a += pool_fees;
      }
      else if( pool_fees.symbol == p.symbol_b )
      {
         p.balance_b += pool_fees;
      }
   });

   adjust_liquid_balance( account.name, return_asset );

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool)(int_account) ) }


asset database::liquid_acquire( const asset& receive, const asset_symbol_type& input, bool execute = true, bool apply_fees = true )
{ try {
   FC_ASSERT( receive.symbol != input,
      "Assets must have different symbols to acquire.");

   asset coin_asset;

   if( receive.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& acquire_pool = get_liquidity_pool( SYMBOL_COIN, receive.symbol );

      uint128_t pr = BLOCKCHAIN_PRECISION;
      uint128_t pr_sq = BLOCKCHAIN_PRECISION * BLOCKCHAIN_PRECISION;
      uint128_t ib = acquire_pool.asset_balance( SYMBOL_COIN ).amount.value;
      uint128_t rb = acquire_pool.asset_balance( receive.symbol ).amount.value;
      uint128_t re = receive.amount.value;

      uint128_t input_coin = ( ( ( pr_sq * ib ) / ( pr - ( ( pr * re ) / rb ) ) ) - ( pr * ib ) ) / pr;

      asset total_fees = asset( ( input_coin * TRADING_FEE_PERCENT ) / PERCENT_100,  SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;
      
      asset coin_asset = asset( input_coin, SYMBOL_COIN );
      
      if( execute )
      {
         modify( acquire_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a += coin_asset;
            p.balance_b -= receive; 
         });
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
      }

      if( apply_fees )
      {
         coin_asset += total_fees;
      }
   }
   else
   {
      coin_asset = receive;
   }

   if( input != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& receive_pool = get_liquidity_pool( SYMBOL_COIN, input );

      asset total_fees = asset( ( coin_asset.amount.value * TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;

      if( apply_fees )
      {
         coin_asset += total_fees;
      }

      uint128_t in = coin_asset.amount.value;
      uint128_t pr = BLOCKCHAIN_PRECISION;
      uint128_t ib = receive_pool.asset_balance( input ).amount.value;
      uint128_t rb = receive_pool.asset_balance( SYMBOL_COIN ).amount.value;

      uint128_t input_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;
      asset input_asset = asset( input_amount, input );
      
      if( execute )
      {
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
         modify( receive_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a -= coin_asset;
            p.balance_b += input_asset; 
         });
      }

      return input_asset;
   }
   else
   {
      return coin_asset;
   }

} FC_CAPTURE_AND_RETHROW( (receive)(input) ) }


void database::liquid_acquire( const asset& receive, const account_object& account, const asset_liquidity_pool_object& pool, const account_object& int_account)
{ try {
   FC_ASSERT( receive.symbol == pool.symbol_a || receive.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset total_fees;
   asset_symbol_type in = pool.base_price( receive.symbol ).quote.symbol;
   uint128_t pr = BLOCKCHAIN_PRECISION;
   uint128_t pr_sq = BLOCKCHAIN_PRECISION * BLOCKCHAIN_PRECISION;
   uint128_t ib = pool.asset_balance( in ).amount.value;
   uint128_t rb = pool.asset_balance( receive.symbol ).amount.value;
   uint128_t re = receive.amount.value;

   if( receive.symbol == SYMBOL_COIN )
   {
      total_fees = ( receive * TRADING_FEE_PERCENT ) / PERCENT_100;
      re += total_fees.amount.value;
   }

   uint128_t input_amount = ( ( ( pr_sq * ib ) / ( pr - ( ( pr * re ) / rb ) ) ) - ( pr * ib ) ) / pr;
   asset input_asset = asset( input_amount, in );
   
   if( receive.symbol != SYMBOL_COIN )
   {
      total_fees = ( input_asset * TRADING_FEE_PERCENT ) / PERCENT_100;
      input_asset += total_fees;
   }

   asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset pool_fees = total_fees - network_fees - interface_fees;

   asset liquid = get_liquid_balance( account.name, input_asset.symbol );

   FC_ASSERT( liquid >= input_asset, 
      "Insufficient Balance to acquire requested amount: ${a}.", ("a", receive) );

   adjust_liquid_balance( account.name , -input_asset );

   pay_network_fees( account, network_fees );
   pay_fee_share( int_account, interface_fees );
   
   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( receive.symbol == p.symbol_a )
      {
         p.balance_a -= receive;
         p.balance_b += input_asset;
      }
      else if( receive.symbol == p.symbol_a )
      {
         p.balance_b -= receive;
         p.balance_a += input_asset;
      }
      if( pool_fees.symbol == p.symbol_a )
      {
         p.balance_a += pool_fees;
      }
      else if( pool_fees.symbol == p.symbol_b )
      {
         p.balance_b += pool_fees;
      }
   });

   adjust_liquid_balance( account.name, receive );

} FC_CAPTURE_AND_RETHROW( (receive)(account)(pool) ) }


/**
 * Sells an input asset into an asset liquidity pool, up to the lower of a specified amount, or
 * an amount that would cause the sale price to fall below a specified limit price.
 */
pair< asset, asset > database::liquid_limit_exchange( const asset& input, const price& limit_price, 
   const asset_liquidity_pool_object& pool, bool execute = true, bool apply_fees = true )
{ try {
   FC_ASSERT( input.symbol == pool.symbol_a || input.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset total_fees;
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   price current = pool.base_price( limit_price.base.symbol );
   price lim;
   if( limit_price.base.symbol == input.symbol )
   {
      lim = limit_price;
   }
   else if( limit_price.quote.symbol == input.symbol)
   {
      lim = ~limit_price;
   }

   if( current > limit_price )
   {
      uint128_t pr = BLOCKCHAIN_PRECISION;
      uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
      uint128_t rb = pool.asset_balance( rec ).amount.value;
      uint128_t in = input.amount.value;

      uint128_t product = ( ib * rb * lim.base.amount.value ) / lim.quote.amount.value;
      uint128_t limit_amount = approx_sqrt( product ) - in;

      FC_ASSERT( limit_amount >= 0, 
         "Negative limit amount, limit price above current price.");
      
      uint128_t lim_in = std::min( in, limit_amount );
      asset input_asset = asset( lim_in, input.symbol );

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( lim_in + ib ) ) ) ) / pr;
      
      if( apply_fees )
      {
         total_fees = asset( ( return_amount * TRADING_FEE_PERCENT ) / PERCENT_100,  rec );
         return_amount -= total_fees.amount.value;
      }

      asset return_asset = asset( return_amount, rec );
      asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset pool_fees = total_fees - network_fees;
      
      if( execute )
      {
         modify( pool, [&]( asset_liquidity_pool_object& p )
         {
            if( input.symbol == p.symbol_a )
            {
               p.balance_a += input_asset; 
               p.balance_b -= return_asset;
            }
            else if( input.symbol == p.symbol_b )
            {
               p.balance_b += input_asset; 
               p.balance_a -= return_asset;
            }
            if( apply_fees )
            {
               if( pool_fees.symbol == p.symbol_a )
               {
                  p.balance_a += pool_fees;
               }
               else if( pool_fees.symbol == p.symbol_b )
               {
                  p.balance_b += pool_fees;
               }
            }
         });

         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
      }

      return std::make_pair( input_asset, return_asset );
   }
   else
   {
     return std::make_pair( asset(0, input.symbol), asset(0, rec) );
   }
} FC_CAPTURE_AND_RETHROW( (input)(limit_price)(pool) ) }

/**
 * Sells an input asset into an asset liquidity pool, up to the lower of a specified amount, or
 * an amount that would cause the sale price to fall below a specified limit price.
 */
void database::liquid_limit_exchange( const asset& input, const price& limit_price, const account_object& account, 
   const asset_liquidity_pool_object& pool, const account_object& int_account )
{ try {
   FC_ASSERT( input.symbol == pool.symbol_a || input.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   price current = pool.base_price( limit_price.base.symbol );
   price lim;
   if( limit_price.base.symbol == input.symbol )
   {
      lim = limit_price;
   }
   else if( limit_price.quote.symbol == input.symbol )
   {
      lim = ~limit_price;
   }

   if( current > limit_price )
   {
      uint128_t pr = BLOCKCHAIN_PRECISION;
      uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
      uint128_t rb = pool.asset_balance( rec ).amount.value;
      uint128_t in = input.amount.value;

      uint128_t product = ( ib * rb * lim.base.amount.value ) / lim.quote.amount.value;
      uint128_t limit_amount = approx_sqrt( product ) - in;

      FC_ASSERT( limit_amount >= 0, 
         "Negative limit amount, limit price above current price.");
      
      uint128_t lim_in = std::min( in, limit_amount );
      asset input_asset = asset( lim_in, input.symbol );

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( lim_in + ib ) ) ) ) / pr;
      
      asset total_fees = asset( ( return_amount * TRADING_FEE_PERCENT ) / PERCENT_100,  rec );
      return_amount -= total_fees.amount.value;
      
      asset return_asset = asset( return_amount, rec );
      asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset pool_fees = total_fees - network_fees - interface_fees;

      asset liquid = get_liquid_balance( account.name, input_asset.symbol );

      FC_ASSERT( liquid >= input_asset, 
         "Insufficient Balance to acquire requested amount: ${a}.", ("a", return_asset) );

      adjust_liquid_balance( account.name , -input_asset );
      
      modify( pool, [&]( asset_liquidity_pool_object& p )
      {
         if( input.symbol == p.symbol_a )
         {
            p.balance_a += input_asset; 
            p.balance_b -= return_asset;
         }
         else if( input.symbol == p.symbol_b )
         {
            p.balance_b += input_asset; 
            p.balance_a -= return_asset;
         }
         
         if( pool_fees.symbol == p.symbol_a )
         {
            p.balance_a += pool_fees;
         }
         else if( pool_fees.symbol == p.symbol_b )
         {
            p.balance_b += pool_fees;
         }
      });

      pay_network_fees( account, network_fees );
      pay_fee_share( int_account, interface_fees );

      adjust_liquid_balance( account.name , return_asset );    
   }
} FC_CAPTURE_AND_RETHROW( (input)(limit_price)(account)(pool) ) }


void database::credit_lend( const asset& input, const account_object& account, const asset_credit_pool_object& pool)
{ try {
   FC_ASSERT( input.symbol == pool.base_symbol );
   price credit_price = pool.current_price();
   asset borrowed = input * credit_price;
   adjust_liquid_balance( account.name, -input );

   modify( pool, [&]( asset_credit_pool_object& acpo )
   {
      acpo.base_balance += input;
      acpo.credit_balance += borrowed;
      acpo.last_price = credit_price;
   });

   adjust_liquid_balance( account.name, borrowed );

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }


void database::credit_withdraw( const asset& input, const account_object& account, const asset_credit_pool_object& pool)
{ try {
   FC_ASSERT( input.symbol == pool.credit_symbol );
   price credit_price = pool.current_price();
   asset withdrawn = input * credit_price;
   adjust_liquid_balance( account.name, -input );

   modify( pool, [&]( asset_credit_pool_object& acpo )
   {
      acpo.base_balance -= input;
      acpo.credit_balance -= withdrawn;
      acpo.last_price = credit_price;
   });

   adjust_liquid_balance( account.name, withdrawn );

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }


/**
 * Checks if the collateral asset and the debt asset is sufficiently liquid
 * to the core asset, and that the amount lent overall is less than 50% of
 * the amount available from the core to debt asset liquidity pool.
 */
bool database::credit_check( const asset& debt, const asset& collateral, const asset_credit_pool_object& credit_pool)
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   asset collateral_coin = collateral;
   asset debt_coin = debt;
   asset debt_outstanding = credit_pool.borrowed_balance;

   FC_ASSERT( debt.symbol == credit_pool.base_symbol,
      "Incorrect credit pool for requested debt asset." );

   if( collateral.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& col_pool = get_liquidity_pool( collateral.symbol );
      collateral_coin = liquid_exchange( 10 * collateral, SYMBOL_COIN, false, false);
   }
   else
   {
      collateral_coin = 10 * collateral;
   }
   
   if( debt.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& debt_pool = get_liquidity_pool( debt.symbol );
      if( debt_pool.asset_balance( debt.symbol ) >= 10 * debt )
      {
         debt_coin = liquid_acquire( 10 * debt, SYMBOL_COIN, false, false);
      }
      else
      {
         return false;
      }
      if( debt_outstanding > ( debt_pool.asset_balance( debt.symbol ) * props.market_max_credit_ratio ) / PERCENT_100 )
      {
         return false;
      }
   }
   else
   {
      debt_coin = 10 * debt;
   }

   if( collateral_coin <= debt_coin )
   {
      return false;
   }
   else
   {
      return true;
   }
} FC_CAPTURE_AND_RETHROW( (debt)(collateral)(credit_pool) ) }


/**
 * Checks whether a proposed margin position has sufficient liquidity to the 
 * core asset, and whether the debt asset has greater outstanding debt
 * than 50% of the liquidity pool has available in exchange for the core asset.
 */
bool database::margin_check( const asset& debt, const asset& position, const asset& collateral, const asset_credit_pool_object& credit_pool)
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   asset collateral_coin = collateral;
   asset position_coin = position;
   asset debt_coin = debt;
   asset debt_outstanding = credit_pool.borrowed_balance;

   FC_ASSERT( debt.symbol == credit_pool.base_symbol,
      "Incorrect credit pool for requested debt asset." );

   if( collateral.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& col_pool = get_liquidity_pool( collateral.symbol );
      collateral_coin = liquid_exchange( 10 * collateral, SYMBOL_COIN, false, false );
   }
   else
   {
      collateral_coin = 10 * collateral;
   }

   if( position.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& position_pool = get_liquidity_pool( position.symbol );
      position_coin = liquid_exchange( 10 * position, SYMBOL_COIN, false, false );
   }
   else
   {
      position_coin = 10 * position;
   }
   
   if( debt.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& debt_pool = get_liquidity_pool( debt.symbol );
      if( debt_pool.asset_balance( debt.symbol ) >= 10 * debt )
      {
         debt_coin = liquid_acquire( 10 * debt, SYMBOL_COIN, false, false );
      }
      else
      {
         return false;
      }
      if( debt_outstanding > ( debt_pool.asset_balance( debt.symbol ) * props.market_max_credit_ratio ) / PERCENT_100 )
      {
         return false;
      }
   }
   else
   {
      debt_coin = 10 * debt;
   }

   if( ( collateral_coin + position_coin ) <= debt_coin )
   {
      return false;
   }
   else
   {
      return true;
   }

} FC_CAPTURE_AND_RETHROW( (debt)(position)(collateral)(credit_pool) ) }



/** 
 * Compounds interest on all credit loans, and checks collateralization
 * ratios for all loans, and liquidates them if they are under collateralized.
 */
void database::process_credit_updates()
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& loan_idx = get_index< credit_loan_index >().indices().get< by_liquidation_spread >();
   auto loan_itr = loan_idx.begin();

   while( loan_itr != loan_idx.end() )
   {
      const asset_object& debt_asset = get_asset( loan_itr->debt_asset() );
      const asset_credit_pool_object& credit_pool = get_credit_pool( loan_itr->debt_asset(), false );
      uint16_t fixed = props.credit_min_interest;
      uint16_t variable = props.credit_variable_interest;
      share_type interest_rate = credit_pool.interest_rate( fixed, variable );
      asset total_interest = asset( 0, debt_asset.symbol );

      while( loan_itr != loan_idx.end() && 
         loan_itr->debt_asset() == debt_asset.symbol )
      {
         const asset_object& collateral_asset = get_asset( loan_itr->collateral_asset() );
         const asset_liquidity_pool_object& pool = get_liquidity_pool( loan_itr->symbol_a, loan_itr->symbol_b );
         price col_debt_price = pool.base_hour_median_price( loan_itr->collateral_asset() );

         while( loan_itr != loan_idx.end() &&
            loan_itr->debt_asset() == debt_asset.symbol &&
            loan_itr->collateral_asset() == collateral_asset.symbol )
         {
            const credit_loan_object& loan = *loan_itr;

            asset max_debt = ( loan.collateral * col_debt_price * props.credit_liquidation_ratio ) / PERCENT_100;
            price liquidation_price = price( loan.collateral, max_debt );
            asset interest = ( loan.debt * interest_rate * ( now - loan.last_updated ).to_seconds() ) / ( fc::days(365).to_seconds() * PERCENT_100 );
            total_interest += interest;

            modify( loan, [&]( credit_loan_object& c )
            {
               c.debt += interest;
               c.interest += interest;
               c.loan_price = price( c.collateral, c.debt );
               c.liquidation_price = liquidation_price;
               c.last_interest_rate = interest_rate;
               c.last_updated = now;
            });

            if( loan.liquidation_price > loan.loan_price )  // If loan falls below liquidation price
            {
               liquidate_credit_loan( loan );
            }

            ++loan_itr;
         }
      }

      modify( credit_pool, [&]( asset_credit_pool_object& c )
      {
         c.last_interest_rate = interest_rate;
         c.borrowed_balance += total_interest;
      });
   }
} FC_CAPTURE_AND_RETHROW() }


/** 
 * Compounds interest on all margin orders, and checks collateralization
 * ratios for all orders, and liquidates them if they are under collateralized.
 * Places orders into the book in liquidation if they reach their limit stop or take profit 
 * price.
 */
void database::process_margin_updates()
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& margin_idx = get_index< margin_order_index >().indices().get< by_debt_collateral_position >();
   auto margin_itr = margin_idx.begin();

   while( margin_itr != margin_idx.end() )
   {
      const asset_object& debt_asset = get_asset( margin_itr->debt_asset() );
      const asset_credit_pool_object& credit_pool = get_credit_pool( margin_itr->debt_asset(), false );
      uint16_t fixed = props.credit_min_interest;
      uint16_t variable = props.credit_variable_interest;
      share_type interest_rate = credit_pool.interest_rate( fixed, variable );
      asset total_interest = asset( 0, debt_asset.symbol );

      while( margin_itr != margin_idx.end() && 
         margin_itr->debt_asset() == debt_asset.symbol )
      {
         const asset_object& collateral_asset = get_asset( margin_itr->collateral_asset() );

         asset_symbol_type symbol_a;
         asset_symbol_type symbol_b;
         if( debt_asset.id < collateral_asset.id )
         {
            symbol_a = debt_asset.symbol;
            symbol_b = collateral_asset.symbol;
         }
         else
         {
            symbol_b = debt_asset.symbol;
            symbol_a = collateral_asset.symbol;
         }

         const asset_liquidity_pool_object& col_debt_pool = get_liquidity_pool( symbol_a, symbol_b );
         price col_debt_price = col_debt_pool.base_hour_median_price( debt_asset.symbol );

         while( margin_itr != margin_idx.end() &&
            margin_itr->debt_asset() == debt_asset.symbol &&
            margin_itr->collateral_asset() == collateral_asset.symbol )
         {
            const asset_object& position_asset = get_asset( margin_itr->position_asset() );

            asset_symbol_type symbol_a;
            asset_symbol_type symbol_b;
            if( debt_asset.id < position_asset.id )
            {
               symbol_a = debt_asset.symbol;
               symbol_b = position_asset.symbol;
            }
            else
            {
               symbol_b = debt_asset.symbol;
               symbol_a = position_asset.symbol;
            }

            const asset_liquidity_pool_object& pos_debt_pool = get_liquidity_pool( symbol_a, symbol_b );
            price pos_debt_price = pos_debt_pool.base_hour_median_price( debt_asset.symbol );
            
            while( margin_itr != margin_idx.end() &&
               margin_itr->debt_asset() == debt_asset.symbol &&
               margin_itr->collateral_asset() == collateral_asset.symbol &&
               margin_itr->position_asset() == position_asset.symbol )
            {
               const margin_order_object& margin = *margin_itr;
               asset collateral_debt_value;

               if( margin.collateral_asset() != margin.debt_asset() )
               {
                  collateral_debt_value = margin.collateral * col_debt_price;
               }
               else
               {
                  collateral_debt_value = margin.collateral;
               }

               asset position_debt_value = margin.position_balance * pos_debt_price;
               asset equity = margin.debt_balance + position_debt_value + collateral_debt_value;
               asset unrealized_value = margin.debt_balance + position_debt_value - margin.debt;
               share_type collateralization = ( PERCENT_100 * ( equity - margin.debt ) ) / margin.debt;
               
               asset interest = ( margin.debt * interest_rate * ( now - margin.last_updated ).to_seconds() ) / ( fc::days(365).to_seconds() * PERCENT_100 );
               total_interest += interest;

               modify( margin, [&]( margin_order_object& m )
               {
                  m.debt += interest;
                  m.interest += interest;
                  m.collateralization = collateralization;
                  m.unrealized_value = unrealized_value;
                  m.last_interest_rate = interest_rate;
                  m.last_updated = now;
               });

               if( margin.collateralization < props.margin_liquidation_ratio ||
                  pos_debt_price <= margin.stop_loss_price ||
                  pos_debt_price >= margin.take_profit_price )  
               {
                  close_margin_order( margin ); // If margin value falls below collateralization threshold, or stop prices are reached
               }
               else if( pos_debt_price <= margin.limit_stop_loss_price && !margin.liquidating )  
               {
                  modify( margin, [&]( margin_order_object& m )
                  {
                     m.liquidating = true;
                     m.sell_price = ~m.limit_stop_loss_price;   // If price falls below limit stop loss, reverse order and sell at limit price
                  });
                  apply_order( margin );
               }
               else if( pos_debt_price >= margin.limit_take_profit_price && !margin.liquidating )  
               {
                  modify( margin, [&]( margin_order_object& m )
                  {
                     m.liquidating = true;
                     m.sell_price = ~m.limit_take_profit_price;  // If price rises above take profit, reverse order and sell at limit price
                  });
                  apply_order( margin );
               }

               ++margin_itr;

            }     // Same Position, Collateral, and Debt
         }        // Same Collateral and Debt
      }           // Same Debt

      modify( credit_pool, [&]( asset_credit_pool_object& c )
      {
         c.last_interest_rate = interest_rate;
         c.borrowed_balance += total_interest;
      });
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Deleverages a loan that has gone under its collateralization
 * requirements, by selling the collateral to the liquidity arrays.
 */
void database::liquidate_credit_loan( const credit_loan_object& loan )
{ try {
   asset debt_liquidated = liquid_exchange( loan.collateral, loan.debt_asset(), true, true );
   const asset_credit_pool_object& credit_pool = get_credit_pool( loan.debt_asset(), false );
   if( loan.debt.amount > debt_liquidated.amount )
   {
      asset deficit = loan.debt - debt_liquidated;
      asset default_credit = network_credit_acquisition( deficit, true );
      debt_liquidated = loan.debt;
      const account_object& owner = get_account( loan.owner );
      modify( owner, [&]( account_object& a )
      {
         a.loan_default_balance += default_credit;
      });
   }

   modify( credit_pool, [&]( asset_credit_pool_object& c )
   {
      c.borrowed_balance -= loan.debt;
      c.base_balance += debt_liquidated;
   });

   remove( loan );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Acquires a debt asset using network credit asset
 * by issuing new credit to the liquidity pool of the core asset
 * and purchasing the debt asset.
 */
asset database::network_credit_acquisition( const asset& amount, bool execute )
{ try {
   asset coin_acquired;
   asset credit_acquired;
   if( amount.symbol != SYMBOL_CREDIT )
   {
      if( amount.symbol != SYMBOL_COIN )
      {
         coin_acquired = liquid_acquire( amount, SYMBOL_COIN, true, true );
      }
      else
      {
         coin_acquired = amount;
      }
      credit_acquired = liquid_acquire( coin_acquired, SYMBOL_CREDIT, true, true );
   }
   else
   {
      credit_acquired = amount;
   }
   adjust_pending_supply( credit_acquired );
   
   return credit_acquired;
} FC_CAPTURE_AND_RETHROW() }

// Look for expired transactions in the deduplication list, and remove them.
// Transactions must have expired by at least two forking windows in order to be removed.
void database::clear_expired_transactions()
{
   auto& transaction_idx = get_index< transaction_index >();
   const auto& dedupe_index = transaction_idx.indices().get< by_expiration >();
   while( ( !dedupe_index.empty() ) && ( head_block_time() > dedupe_index.begin()->expiration ) )
   {
      remove( *dedupe_index.begin() );
   }
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
   asset collateral_gathered = asset( 0, backing_asset);
   const asset_dynamic_data_object& mia_dyn = get_dynamic_data(mia.symbol);
   auto original_mia_supply = mia_dyn.total_supply;

   const auto& call_price_index = get_index< call_order_index >().indices().get< by_price >();

   // cancel all call orders and accumulate it into collateral_gathered
   auto call_itr = call_price_index.lower_bound( price::min( bitasset.backing_asset, mia.symbol ) );
   auto call_end = call_price_index.upper_bound( price::max( bitasset.backing_asset, mia.symbol ) );
   asset pays;

   while( call_itr != call_end )
   {
      pays = call_itr->debt.multiply_and_round_up( settlement_price ); // round up, in favor of global settlement fund

      if( pays > call_itr->collateral )
      {
         pays = call_itr->collateral;
      }

      collateral_gathered += pays;
      const call_order_object& order = *call_itr;
      ++call_itr;
      FC_ASSERT( fill_call_order( order, pays, order.debt, settlement_price, true, NULL_ACCOUNT, true ) );  // Fill call orders without deducting pending supply of bitasset
   }

   modify( bitasset, [&]( asset_bitasset_data_object& obj )
   {
      obj.settlement_price = asset( original_mia_supply, mia.symbol ) / collateral_gathered;     // Activate global settlement price on asset
      obj.settlement_fund = collateral_gathered;
   });

} FC_CAPTURE_AND_RETHROW( (mia)(settlement_price) ) }

void database::revive_bitasset( const asset_object& bitasset ) 
{ try {
   FC_ASSERT( bitasset.is_market_issued(),
      "Asset must be a market issued asset." );
   const asset_bitasset_data_object& bad = get_bitasset_data(bitasset.symbol);
   FC_ASSERT( bad.has_settlement(),
      "Asset must have a settlement price before it can be revived.");
   const asset_dynamic_data_object& bdd = get_dynamic_data( bitasset.symbol );
   FC_ASSERT( !bad.current_feed.settlement_price.is_null(),
      "Settlement price cannot be null to revive asset." );

   if( bdd.total_supply > 0 )    // Create + execute a "bid" with 0 additional collateral
   {
      const collateral_bid_object& pseudo_bid = create< collateral_bid_object >([&]( collateral_bid_object& bid ) 
      {
         bid.bidder = bitasset.issuer;
         bid.inv_swan_price = asset(0, bad.backing_asset) / asset( bdd.total_supply, bitasset.symbol );
      });
      execute_bid( pseudo_bid, bdd.total_supply, bad.settlement_fund.amount, bad.current_feed );
   } 
   else
   {
      FC_ASSERT( bad.settlement_fund.amount == 0,
         "Cannot have settlement fund with zero total asset supply." );
   }
      
   cancel_bids_and_revive_mpa( bitasset, bad );
} FC_CAPTURE_AND_RETHROW( (bitasset) ) }

void database::cancel_bids_and_revive_mpa( const asset_object& bitasset, const asset_bitasset_data_object& bad )
{ try {
   FC_ASSERT( bitasset.is_market_issued(),
      "Asset must be a market issued asset." );
   FC_ASSERT( bad.has_settlement(),
      "Asset must have a settlement price before it can be revived." );
   
   const auto& bid_idx = get_index< collateral_bid_index >().indices().get< by_price >();
   auto itr = bid_idx.lower_bound( boost::make_tuple( bitasset.symbol, price::max( bad.backing_asset, bitasset.symbol ) ) );
   while( itr != bid_idx.end() && itr->inv_swan_price.quote.symbol == bitasset.symbol )
   {
      const collateral_bid_object& bid = *itr;
      ++itr;
      cancel_bid( bid , true );    // cancel remaining bids
   }

   modify( bad, [&]( asset_bitasset_data_object& obj )
   {
      obj.settlement_price = price();
      obj.settlement_fund = asset( 0, bad.symbol );
   });
} FC_CAPTURE_AND_RETHROW( ( bitasset ) ) }


void database::cancel_bid( const collateral_bid_object& bid, bool create_virtual_op )
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
   remove( bid );
}


void database::execute_bid( const collateral_bid_object& bid, share_type debt_covered, share_type collateral_from_fund, const price_feed& current_feed )
{
   create< call_order_object >( [&]( call_order_object& call )
   {
      call.borrower = bid.bidder;
      call.collateral = bid.inv_swan_price.base.amount + collateral_from_fund;
      call.debt = debt_covered;
      // bid.inv_swan_price is in collateral / debt
      call.call_price = price( asset( 1, bid.inv_swan_price.base.symbol ), asset( 1, bid.inv_swan_price.quote.symbol ) );
   });

   push_virtual_operation( 
      execute_bid_operation( bid.bidder, 
      asset( bid.inv_swan_price.base.amount + collateral_from_fund, bid.inv_swan_price.base.symbol ),
      asset( debt_covered, bid.inv_swan_price.quote.symbol ) ) 
   );

   remove( bid );
}


void database::cancel_settle_order(const force_settlement_object& order, bool create_virtual_op)
{
   const account_object& account_object = get_account(order.owner);
   adjust_liquid_balance( account_object, order.balance );

   if( create_virtual_op )
   {
      asset_settle_cancel_operation vop;
      vop.settlement = order.id;
      vop.account = order.owner;
      vop.amount = order.balance;
      push_virtual_operation( vop );
   }
   remove( order );
}


void database::cancel_limit_order( const limit_order_object& order )
{
   asset refunded = order.amount_for_sale();
   adjust_liquid_balance( order.seller, refunded );
   remove(order);
}

/**
 * Liquidates the remaining position held in a margin order, and 
 * if there is sufficient debt asset, repays the loan.
 * If the order is in default, issues network credit to 
 * acquire the remaining deficit, and applies the default balance
 * to the account.
 * Returns the remaining collateral after the loan has been repaid, 
 * plus any profit denominated in the collateral asset.
 */
void database::close_margin_order( const margin_order_object& order )
{
   const account_object& owner = get_account( order.owner );
   asset collateral = order.collateral;
   asset to_repay = order.debt;
   asset debt_balance = order.debt_balance;
   asset returned_collateral;
   asset collateral_debt_value;
   asset debt_acquired;
   asset collateral_sold;
   const asset_credit_pool_object& credit_pool = get_credit_pool( order.debt_asset(), false );
   const credit_collateral_object& coll_balance = get_collateral( owner.name, order.collateral_asset() );

   if( order.position_balance.amount > 0 )   // Position contained in loan
   {
      asset proceeds = liquid_exchange( order.position_balance, order.debt_asset(), true, true );
      debt_balance += proceeds;
   }

   asset net_value = debt_balance - to_repay;

   if( net_value.amount > 0 )   // Order is net positive
   {
      if( net_value.symbol != order.collateral_asset() )
      {
         asset profit = liquid_exchange( net_value, order.collateral_asset(), true, true );
         returned_collateral = collateral + profit;
      }
      else
      {
         returned_collateral = collateral + net_value;
      }

      modify( coll_balance, [&]( credit_collateral_object& c )
      {
         c.collateral += returned_collateral;
      });
   }
   else   // Order is net negative
   {
      if( net_value.symbol != order.collateral_asset() )
      {
         collateral_debt_value = liquid_exchange( collateral, order.debt_asset(), false, true );
      }
      else
      {
         collateral_debt_value = collateral;
      }
      
      if( -net_value > collateral_debt_value )   // If position is underwater, and cannot repay sufficient debt
      {
         if( net_value.symbol != order.collateral_asset() )
         {
            debt_acquired = liquid_exchange( collateral, order.debt_asset(), true, true );
         }
         else
         {
            debt_acquired = collateral;
         }
         asset remaining = -net_value - debt_acquired;
         
         asset default_credit = network_credit_acquisition( remaining, true );   // Acquire remaining debt asset with network credit asset

         modify( owner, [&]( account_object& a )
         {
            a.loan_default_balance += default_credit;
         });
      }
      else      // Sufficient collateral to repay debt
      {
         if( net_value.symbol != order.collateral_asset() )
         {
            collateral_sold = liquid_acquire( -net_value, order.collateral_asset(), true, true );
         }
         else
         {
            collateral_sold = -net_value;
         }
         
         asset returned_collateral = collateral - collateral_sold;

         modify( coll_balance, [&]( credit_collateral_object& c )
         {
            c.collateral += returned_collateral;
         });
      }
   }

   modify( credit_pool, [&]( asset_credit_pool_object& c )
   {
      c.base_balance += to_repay;
      c.borrowed_balance -= to_repay;
   });

   remove(order);
}

/**
 * Cancels limit orders with 0 assets remaining for the recipient, 
 * Returns true if the order is cancelled.
 */
bool database::maybe_cull_small_order( const limit_order_object& order )
{
   if( order.amount_to_receive().amount == 0 )
   {
      cancel_limit_order( order );
      return true;
   }
   return false;
}

/**
 * Cancels limit orders with 0 assets remaining for the recipient, 
 * Returns true if the order is cancelled.
 */
bool database::maybe_cull_small_order( const margin_order_object& order )
{
   if( order.amount_to_receive().amount == 0 && order.liquidating )
   {
      close_margin_order( order );
      return true;
   }
   return false;
}


/**
 *  Starting with the least collateralized orders, fill them if their
 *  call price is above the max(lowest bid,call_limit).
 *  This method will return true if it filled a short or limit.
 */
bool database::check_call_orders( const asset_object& mia, bool enable_black_swan, bool for_new_limit_order )
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
   if( bitasset.current_feed.settlement_price.is_null() ) 
   {
      return false;
   }

   auto limit_index = get_index < limit_order_index >();
   const auto& limit_price_index = limit_index.indices().get<by_price>();

   auto max_price = price::max( mia.symbol, bitasset.backing_asset ); // looking for limit orders selling the most USD for the least CORE
   auto min_price = bitasset.current_feed.max_short_squeeze_price(); // stop when limit orders are selling too little USD for too much CORE

   auto limit_itr = limit_price_index.lower_bound( max_price );     // NOTE limit_price_index is sorted from greatest to least
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

      auto usd_to_buy = call_order.debt;
      if( usd_to_buy * match_price > call_order.collateral )
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
         call_receives = usd_to_buy;
         order_receives = usd_to_buy.multiply_and_round_up( match_price ); // round up, in favor of limit order
         filled_call = true; 

         if( usd_to_buy == usd_for_sale ) 
         {
            filled_limit = true;
         }
      }

      call_pays  = order_receives;
      order_pays = call_receives;

      fill_call_order( call_order, call_pays, call_receives, match_price, for_new_limit_order, limit_order.interface, false );
      
      call_collateral_itr = call_collateral_index.lower_bound( call_min );

      auto next_limit_itr = std::next( limit_itr );
      // when for_new_limit_order is true, the limit order is taker, otherwise the limit order is maker
      bool really_filled = fill_limit_order( limit_order, order_pays, order_receives, true, match_price, !for_new_limit_order, call_order.interface );
      if( really_filled ) 
      {
         limit_itr = next_limit_itr;
      }
   } // while call_itr != call_end

   return margin_called;
} FC_CAPTURE_AND_RETHROW() }


/**
 * let HB = the highest bid for the collateral (aka who will pay the most DEBT for the least collateral)
 * let SP = current median feed's Settlement Price 
 * let LC = the least collateralized call order's swan price (debt/collateral)
 * If there is no valid price feed or no bids then there is no black swan.
 * A black swan occurs if MAX(HB,SP) <= LC
 */
bool database::check_for_blackswan( const asset_object& mia, bool enable_black_swan, const asset_bitasset_data_object* bitasset_ptr )
{
   if( !mia.is_market_issued() ) return false;

   const asset_bitasset_data_object& bitasset = ( bitasset_ptr ? *bitasset_ptr : get_bitasset_data(mia.symbol));
   if( bitasset.has_settlement() ) return true;     // already force settled
   auto settle_price = bitasset.current_feed.settlement_price;
   if( settle_price.is_null() ) return false;      // no feed

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

} } //node::chain