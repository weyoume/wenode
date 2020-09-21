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


const limit_order_object& database::get_limit_order( const account_name_type& name, const shared_string& order_id )const
{ try {
   return get< limit_order_object, by_account >( boost::make_tuple( name, order_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(order_id) ) }

const limit_order_object* database::find_limit_order( const account_name_type& name, const shared_string& order_id )const
{
   return find< limit_order_object, by_account >( boost::make_tuple( name, order_id ) );
}

const limit_order_object& database::get_limit_order( const account_name_type& name, const string& order_id )const
{ try {
   return get< limit_order_object, by_account >( boost::make_tuple( name, order_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(order_id) ) }

const limit_order_object* database::find_limit_order( const account_name_type& name, const string& order_id )const
{
   return find< limit_order_object, by_account >( boost::make_tuple( name, order_id ) );
}

const margin_order_object& database::get_margin_order( const account_name_type& name, const shared_string& margin_id )const
{ try {
   return get< margin_order_object, by_account >( boost::make_tuple( name, margin_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(margin_id) ) }

const margin_order_object* database::find_margin_order( const account_name_type& name, const shared_string& margin_id )const
{
   return find< margin_order_object, by_account >( boost::make_tuple( name, margin_id ) );
}

const margin_order_object& database::get_margin_order( const account_name_type& name, const string& margin_id )const
{ try {
   return get< margin_order_object, by_account >( boost::make_tuple( name, margin_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(margin_id) ) }

const margin_order_object* database::find_margin_order( const account_name_type& name, const string& margin_id )const
{
   return find< margin_order_object, by_account >( boost::make_tuple( name, margin_id ) );
}

const option_order_object& database::get_option_order( const account_name_type& name, const shared_string& option_id )const
{ try {
   return get< option_order_object, by_account >( boost::make_tuple( name, option_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(option_id) ) }

const option_order_object* database::find_option_order( const account_name_type& name, const shared_string& option_id )const
{
   return find< option_order_object, by_account >( boost::make_tuple( name, option_id ) );
}

const option_order_object& database::get_option_order( const account_name_type& name, const string& option_id )const
{ try {
   return get< option_order_object, by_account >( boost::make_tuple( name, option_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(option_id) ) }

const option_order_object* database::find_option_order( const account_name_type& name, const string& option_id )const
{
   return find< option_order_object, by_account >( boost::make_tuple( name, option_id ) );
}

const auction_order_object& database::get_auction_order( const account_name_type& name, const shared_string& auction_id )const
{ try {
   return get< auction_order_object, by_account >( boost::make_tuple( name, auction_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(auction_id) ) }

const auction_order_object* database::find_auction_order( const account_name_type& name, const shared_string& auction_id )const
{
   return find< auction_order_object, by_account >( boost::make_tuple( name, auction_id ) );
}

const auction_order_object& database::get_auction_order( const account_name_type& name, const string& auction_id )const
{ try {
   return get< auction_order_object, by_account >( boost::make_tuple( name, auction_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(auction_id) ) }

const auction_order_object* database::find_auction_order( const account_name_type& name, const string& auction_id )const
{
   return find< auction_order_object, by_account >( boost::make_tuple( name, auction_id ) );
}

const call_order_object& database::get_call_order( const account_name_type& name, const asset_symbol_type& symbol )const
{ try {
   return get< call_order_object, by_account >( boost::make_tuple( name, symbol ) );
} FC_CAPTURE_AND_RETHROW( (name)(symbol) ) }

const call_order_object* database::find_call_order( const account_name_type& name, const asset_symbol_type& symbol )const
{
   return find< call_order_object, by_account >( boost::make_tuple( name, symbol ) );
}

const asset_collateral_bid_object& database::get_asset_collateral_bid( const account_name_type& name, const asset_symbol_type& symbol )const
{ try {
   return get< asset_collateral_bid_object, by_account >( boost::make_tuple( name, symbol ) );
} FC_CAPTURE_AND_RETHROW( (name)(symbol) ) }

const asset_collateral_bid_object* database::find_asset_collateral_bid( const account_name_type& name, const asset_symbol_type& symbol )const
{
   return find< asset_collateral_bid_object, by_account >( boost::make_tuple( name, symbol ) );
}

const asset_settlement_object& database::get_asset_settlement( const account_name_type& name, const asset_symbol_type& symbol )const
{ try {
   return get< asset_settlement_object, by_account_asset >( boost::make_tuple( name, symbol ) );
} FC_CAPTURE_AND_RETHROW( (name)(symbol) ) }

const asset_settlement_object* database::find_asset_settlement( const account_name_type& name, const asset_symbol_type& symbol )const
{
   return find< asset_settlement_object, by_account_asset >( boost::make_tuple( name, symbol ) );
}

bool database::apply_order( const limit_order_object& order )
{
   ilog( "Applying Limit order: \n ${o} \n",
      ("o",order));

   limit_order_id_type order_id = order.id;
   asset_symbol_type sell_asset_symbol = order.sell_asset();
   asset_symbol_type recv_asset_symbol = order.receive_asset();

   // We only need to check if the new order will match with others if it is at the front of the book

   const auto& limit_price_idx = get_index< limit_order_index >().indices().get< by_high_price >();
   const auto& margin_price_idx = get_index< margin_order_index >().indices().get< by_high_price >();
   const auto& limit_market_idx = get_index< limit_order_index >().indices().get< by_market >();
   const auto& margin_market_idx = get_index< margin_order_index >().indices().get< by_market >();

   bool match_limit = true;
   bool match_margin = true;
   bool check_pool = false;
   bool match_pool = false;

   auto limit_market_itr = limit_market_idx.lower_bound( order.get_market() );
   auto limit_market_end = limit_market_idx.upper_bound( order.get_market() );

   ilog( "|========== Orderbook ==========" );

   while( limit_market_itr != limit_market_end && limit_market_itr != limit_market_idx.end() )
   {
      ilog( "| Limit: ${a}",
         ("a",limit_market_itr->to_string()));
      ++limit_market_itr;
   }

   auto margin_market_itr = margin_market_idx.lower_bound( boost::make_tuple( false, order.get_market() ) );
   auto margin_market_end = margin_market_idx.upper_bound( boost::make_tuple( false, order.get_market() ) );

   while( margin_market_itr != margin_market_end && margin_market_itr != margin_market_idx.end() )
   {
      ilog( "| Margin: ${a}",
         ("a",margin_market_itr->to_string()));
      ++margin_market_itr;
   }

   auto limit_itr = limit_price_idx.lower_bound( order.sell_price );
   if( limit_itr != limit_price_idx.begin() )
   {
      --limit_itr;
      if( limit_itr->sell_asset() == sell_asset_symbol && limit_itr->receive_asset() == recv_asset_symbol )
      {
         match_limit = false;
      }
   }
   
   auto margin_itr = margin_price_idx.lower_bound( boost::make_tuple( false, order.sell_price ) );
   if( margin_itr != margin_price_idx.begin() )
   {
      --margin_itr;
      if( margin_itr->sell_asset() == sell_asset_symbol && margin_itr->receive_asset() == recv_asset_symbol )
      {
         match_margin = false;
      }
   }

   const asset_object& sell_asset = get_asset( sell_asset_symbol );
   const asset_object& recv_asset = get_asset( recv_asset_symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   price max_price = ~order.sell_price;   // This is the opposite side on the book
   price limit_sell = max_price;
   price margin_sell = max_price;
   price pool_price = max_price;

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
      const asset_liquidity_pool_object& pool = *liq_ptr;

      check_pool = true;
      pool_price = pool.base_price( max_price.base.symbol );
      match_pool = pool_price > max_price;
   }

   if( !match_limit && !match_margin && !match_pool )
   {
      return false;
   }

   limit_itr = limit_price_idx.lower_bound( max_price.max() );
   auto limit_end = limit_price_idx.upper_bound( max_price );
   margin_itr = margin_price_idx.lower_bound( boost::make_tuple( false, max_price.max() ) );
   auto margin_end = margin_price_idx.upper_bound( boost::make_tuple( false, max_price ) );

   if( limit_itr != limit_end )
   {
      limit_sell = limit_itr->sell_price;
   }
   if( margin_itr != margin_end )
   {
      margin_sell = margin_itr->sell_price;
   }

   // Order matching should be in favor of the taker.
   // the limit order will only match with a call order if meet all of these:
   // 1. it's buying collateral, which means sell_asset is the MIA, receive_asset is the backing asset.
   // 3. sell_asset is not globally settled
   // 4. sell_asset has a valid price feed
   // 5. the call order's collateral ratio is below or equals to MCR
   // 6. the limit order provided a good price

   bool to_check_call_orders = false;

   const asset_stablecoin_data_object* sell_abd_ptr = find_stablecoin_data( sell_asset_symbol );
   price call_match_price = max_price;    // debt / collateral

   if( sell_abd_ptr != nullptr )
   {
      if( sell_abd_ptr->backing_asset == recv_asset_symbol
         && !sell_abd_ptr->has_settlement()
         && !sell_abd_ptr->current_feed.settlement_price.is_null() )
      {
         call_match_price = ~sell_abd_ptr->current_feed.max_short_squeeze_price();
         if( ~order.sell_price <= call_match_price )
         {
            to_check_call_orders = true;        // new limit order price is good enough to match a call
         }
      }
   }

   bool finished = false;      // whether the new order is gone

   if( to_check_call_orders )  // check limit/margin/pool orders first, match the ones with better price in comparison to call orders
   {
      while( !finished && 
         (( limit_itr != limit_end && limit_sell > call_match_price ) || 
         ( margin_itr != margin_end && margin_sell > call_match_price ) || 
         ( check_pool && pool_price > call_match_price )) )
      {
         if( limit_itr != limit_end )
         {
            limit_sell = limit_itr->sell_price;
         }
         else
         {
            limit_sell = max_price;
         }
         if( margin_itr != margin_end )
         {
            margin_sell = margin_itr->sell_price;
         }
         else
         {
            margin_sell = max_price;
         }
         price book_price = std::max( limit_sell, margin_sell );

         ilog( "Limit price: ${lp} Margin Price: ${mp} Pool Price: ${pp}",
            ("lp",limit_sell.to_string())("mp",margin_sell.to_string())("pp",pool_price.to_string()));
         
         if( limit_itr != limit_end && limit_sell > call_match_price && limit_sell == book_price )
         {
            auto old_limit_itr = limit_itr;
            ++limit_itr;
            // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            finished = match( order, *old_limit_itr, old_limit_itr->sell_price ) != 2;
            
         }
         else if( margin_itr != margin_end && margin_sell > call_match_price && margin_sell == book_price )
         {
            auto old_margin_itr = margin_itr;
            ++margin_itr;
            // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            finished = match( order, *old_margin_itr, old_margin_itr->sell_price ) != 2;
         }
         else if( check_pool && pool_price > call_match_price )
         {
            finished = match( order, *liq_ptr, book_price ) != 2;
            pool_price = liq_ptr->base_price( max_price.base.symbol );
            // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
      }

      if( !finished ) // check if there are margin calls
      {
         const auto& call_idx = get_index<call_order_index>().indices().get<by_collateral>();
         auto call_min = price::min( recv_asset_symbol, sell_asset_symbol );
         while( !finished )
         {
            auto call_itr = call_idx.lower_bound( call_min );    // check call order with least collateral ratio
            if( call_itr == call_idx.end() || call_itr->debt_type() != sell_asset_symbol || 
               call_itr->collateralization() > sell_abd_ptr->current_maintenance_collateralization )
            {
               break;
            }
               
            int match_result = match( order, *call_itr, call_match_price,
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

   // check limit/margin/pool orders first, match the ones with better price in comparison to call orders

   while( !finished && ( limit_itr != limit_end || margin_itr != margin_end ))
   {
      if( limit_itr != limit_end )
      {
         limit_sell = limit_itr->sell_price;
      }
      else
      {
         limit_sell = max_price;
      }
      if( margin_itr != margin_end )
      {
         margin_sell = margin_itr->sell_price;
      }
      else
      {
         margin_sell = max_price;
      }
      price book_price = std::max( limit_sell, margin_sell );

      ilog( "Limit price: ${lp} Margin Price: ${mp} Pool Price: ${pp}",
         ("lp",limit_sell.to_string())("mp",margin_sell.to_string())("pp",pool_price.to_string()));
      
      if( limit_itr != limit_end && limit_sell == book_price )
      {
         auto old_limit_itr = limit_itr;
         ++limit_itr;
         
         // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         finished = match( order, *old_limit_itr, old_limit_itr->sell_price ) != 2;
      }
      else if( margin_itr != margin_end && margin_sell == book_price )
      {
         auto old_margin_itr = margin_itr;
         ++margin_itr;
         
         // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         finished = match( order, *old_margin_itr, old_margin_itr->sell_price ) != 2;
      }
      else if( check_pool && pool_price > book_price )
      {
         finished = match( order, *liq_ptr, book_price ) != 2;
         pool_price = liq_ptr->base_price( max_price.base.symbol );
         // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
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


bool database::apply_order( const margin_order_object& order )
{
   ilog( "Applying Margin order: \n ${o} \n",
      ("o",order));

   margin_order_id_type order_id = order.id;
   asset_symbol_type sell_asset_symbol = order.sell_asset();
   asset_symbol_type recv_asset_symbol = order.receive_asset();

   // We only need to check if the new order will match with others if it is at the front of the book
   const auto& limit_price_idx = get_index< limit_order_index >().indices().get< by_high_price >();
   const auto& margin_price_idx = get_index< margin_order_index >().indices().get< by_high_price >();
   const auto& limit_market_idx = get_index< limit_order_index >().indices().get< by_market >();
   const auto& margin_market_idx = get_index< margin_order_index >().indices().get< by_market >();

   bool match_limit = true;
   bool match_margin = true;
   bool check_pool = false;
   bool match_pool = false;

   auto limit_market_itr = limit_market_idx.lower_bound( order.get_market() );
   auto limit_market_end = limit_market_idx.upper_bound( order.get_market() );

   ilog( "|========== Orderbook ==========" );

   while( limit_market_itr != limit_market_end && limit_market_itr != limit_market_idx.end() )
   {
      ilog( "| Limit: ${a}",
         ("a",limit_market_itr->to_string()));
      ++limit_market_itr;
   }

   auto margin_market_itr = margin_market_idx.lower_bound( boost::make_tuple( false, order.get_market() ) );
   auto margin_market_end = margin_market_idx.upper_bound( boost::make_tuple( false, order.get_market() ) );

   while( margin_market_itr != margin_market_end && margin_market_itr != margin_market_idx.end() )
   {
      ilog( "| Margin: ${a}",
         ("a",margin_market_itr->to_string()));
      ++margin_market_itr;
   }

   auto limit_itr = limit_price_idx.lower_bound( order.sell_price );
   if( limit_itr != limit_price_idx.begin() )
   {
      --limit_itr;
      if( limit_itr->sell_asset() == sell_asset_symbol && limit_itr->receive_asset() == recv_asset_symbol )
      {
         match_limit = false;
         ilog( "Matched Limit Order");
      }
   }
   
   auto margin_itr = margin_price_idx.lower_bound( boost::make_tuple( false, order.sell_price ) );
   if( margin_itr != margin_price_idx.begin() )
   {
      --margin_itr;
      if( margin_itr->sell_asset() == sell_asset_symbol && margin_itr->receive_asset() == recv_asset_symbol )
      {
         match_margin = false;
         ilog( "Matched Margin Order");
      }
   }

   const asset_object& sell_asset = get_asset( sell_asset_symbol );
   const asset_object& recv_asset = get_asset( recv_asset_symbol );
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   price max_price = ~order.sell_price;                  // this is the opposite side (on the book)
   price limit_sell = max_price;
   price margin_sell = max_price;
   price pool_price = max_price;

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
      pool_price = liq_ptr->base_price( max_price.base.symbol );
      match_pool = pool_price > max_price;
   }

   if( !match_limit && !match_margin && !match_pool )
   {
      return false;
   }

   limit_itr = limit_price_idx.lower_bound( max_price.max() );
   auto limit_end = limit_price_idx.upper_bound( max_price );
   margin_itr = margin_price_idx.lower_bound( boost::make_tuple( false, max_price.max() ) );
   auto margin_end = margin_price_idx.upper_bound( boost::make_tuple( false, max_price ) );

   if( limit_itr != limit_end )
   {
      limit_sell = limit_itr->sell_price;
   }
   if( margin_itr != margin_end )
   {
      margin_sell = margin_itr->sell_price;
   }

   // Order matching should be in favor of the taker.
   // the limit order will only match with a call order if meet all of these:
   // 1. it's buying collateral, which means sell_asset is the MIA, receive_asset is the backing asset.
   // 3. sell_asset is not globally settled
   // 4. sell_asset has a valid price feed
   // 5. the call order's collateral ratio is below or equals to MCR
   // 6. the limit order provided a good price

   bool to_check_call_orders = false;

   const asset_stablecoin_data_object* sell_abd_ptr = find_stablecoin_data( sell_asset_symbol );
   price call_match_price = max_price;    // debt / collateral

   if( sell_abd_ptr != nullptr )
   {
      if( sell_abd_ptr->backing_asset == recv_asset_symbol
         && !sell_abd_ptr->has_settlement()
         && !sell_abd_ptr->current_feed.settlement_price.is_null() )
      {
         call_match_price = ~sell_abd_ptr->current_feed.max_short_squeeze_price();
         if( ~order.sell_price <= call_match_price )
         {
            to_check_call_orders = true;        // new limit order price is good enough to match a call
         }
      }
   }

   bool finished = false;      // whether the new order is gone

   if( to_check_call_orders )  // check limit/margin/pool orders first, match the ones with better price in comparison to call orders
   {
      while( !finished && 
         (( limit_itr != limit_end && limit_sell > call_match_price ) || 
         ( margin_itr != margin_end && margin_sell > call_match_price ) || 
         ( check_pool && pool_price > call_match_price )) )
      {
         if( limit_itr != limit_end )
         {
            limit_sell = limit_itr->sell_price;
         }
         else
         {
            limit_sell = max_price;
         }
         if( margin_itr != margin_end )
         {
            margin_sell = margin_itr->sell_price;
         }
         else
         {
            margin_sell = max_price;
         }
         price book_price = std::max( limit_sell, margin_sell );

         ilog( "Limit price: ${lp} Margin Price: ${mp} Pool Price: ${pp}",
            ("lp",limit_sell.to_string())("mp",margin_sell.to_string())("pp",pool_price.to_string()));
         
         if( limit_itr != limit_end && limit_sell > call_match_price && limit_sell == book_price )
         {
            auto old_limit_itr = limit_itr;
            ++limit_itr;
            // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            finished = match( order, *old_limit_itr, old_limit_itr->sell_price ) != 2;
            
         }
         else if( margin_itr != margin_end && margin_sell > call_match_price && margin_sell == book_price )
         {
            auto old_margin_itr = margin_itr;
            ++margin_itr;
            // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            finished = match( order, *old_margin_itr, old_margin_itr->sell_price ) != 2;
         }
         else if( check_pool && pool_price > call_match_price )
         {
            finished = match( order, *liq_ptr, book_price ) != 2;
            pool_price = liq_ptr->base_price( max_price.base.symbol );
            // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         }
      }

      if( !finished ) // check if there are margin calls
      {
         const auto& call_idx = get_index<call_order_index>().indices().get<by_collateral>();
         auto call_min = price::min( recv_asset_symbol, sell_asset_symbol );
         while( !finished )
         {
            auto call_itr = call_idx.lower_bound( call_min );    // check call order with least collateral ratio
            if( call_itr == call_idx.end() || call_itr->debt_type() != sell_asset_symbol || 
               call_itr->collateralization() > sell_abd_ptr->current_maintenance_collateralization )
            {
               break;
            }
               
            int match_result = match( order, *call_itr, call_match_price,
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

   // check limit/margin/pool orders first, match the ones with better price in comparison to call orders

   while( !finished && ( limit_itr != limit_end || margin_itr != margin_end ))
   {
      if( limit_itr != limit_end )
      {
         limit_sell = limit_itr->sell_price;
      }
      else
      {
         limit_sell = max_price;
      }
      if( margin_itr != margin_end )
      {
         margin_sell = margin_itr->sell_price;
      }
      else
      {
         margin_sell = max_price;
      }
      price book_price = std::max( limit_sell, margin_sell );

      ilog( "Limit price: ${lp} Margin Price: ${mp} Pool Price: ${pp}",
         ("lp",limit_sell.to_string())("mp",margin_sell.to_string())("pp",pool_price.to_string()));
      
      if( limit_itr != limit_end && limit_sell == book_price )
      {
         auto old_limit_itr = limit_itr;
         ++limit_itr;
         
         // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         finished = match( order, *old_limit_itr, old_limit_itr->sell_price ) != 2;
      }
      else if( margin_itr != margin_end && margin_sell == book_price )
      {
         auto old_margin_itr = margin_itr;
         ++margin_itr;
         
         // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
         finished = match( order, *old_margin_itr, old_margin_itr->sell_price ) != 2;
      }
      else if( check_pool && pool_price > book_price )
      {
         finished = match( order, *liq_ptr, book_price ) != 2;
         pool_price = liq_ptr->base_price( max_price.base.symbol );
         // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
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
   ilog( "Match limit orders at price: ${p} \n ${t} \n ${m} \n",
      ("p",match_price.to_string())("t",taker.to_string())("m",maker.to_string()));

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

   if( taker_for_sale < maker_for_sale * match_price )
   {
      taker_receives = taker_for_sale * match_price;
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_for_sale;
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price;
      taker_receives = maker_for_sale;
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
   FC_ASSERT( result != 0 );   // Must fill at least one order

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
   ilog( "Match margin orders at price: ${p} \n ${t} \n ${m} \n",
      ("p",match_price.to_string())("t",taker.to_string())("m",maker.to_string()));

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
   if( taker_for_sale < maker_for_sale * match_price )
   {
      taker_receives = taker_for_sale * match_price;
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_for_sale;
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price;
      taker_receives = maker_for_sale;
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
   ilog( "Match limit and margin orders at price: ${p} \n ${t} \n ${m} \n",
      ("p",match_price.to_string())("t",taker.to_string())("m",maker.to_string()));

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
   if( taker_for_sale < maker_for_sale * match_price )
   {
      taker_receives = taker_for_sale * match_price;
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_for_sale;
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price;
      taker_receives = maker_for_sale;
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
   ilog( "Match margin and limit orders at price: ${p} \n ${t} \n ${m} \n",
      ("p",match_price.to_string())("t",taker.to_string())("m",maker.to_string()));

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
   if( taker_for_sale < maker_for_sale * match_price )
   {
      taker_receives = taker_for_sale * match_price;
      if( taker_receives.amount == 0 )
      {
         return 1;
      }

      maker_receives = taker_for_sale;
      cull_taker = true;
   }
   else
   {
      maker_receives = maker_for_sale * match_price;
      taker_receives = maker_for_sale;
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
   ilog( "Match limit order to pool at price: ${p} \n Order: ${t} \n Pool: ${pool} \n",
      ("p",match_price.to_string())("t",taker.to_string())("pool",pool.to_string()));

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
   ilog( "Match margin order to pool at price: ${p} \n Order: ${t} \n Pool: ${pool} \n",
      ("p",match_price.to_string())("t",taker.to_string())("pool",pool.to_string()));

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
   ilog( "Match order to call at price: ${p} \n ${b} \n ${a} \n",
      ("p",match_price.to_string())("b",bid.to_string())("a",ask.to_string()));
   FC_ASSERT( bid.sell_asset() == ask.debt_type() );
   FC_ASSERT( bid.receive_asset() == ask.collateral_type() );
   FC_ASSERT( bid.amount_for_sale().amount > 0 && ask.debt.amount > 0 && ask.collateral.amount > 0 );

   bool cull_taker = false;

   asset taker_for_sale = bid.amount_for_sale();
   asset taker_to_buy = asset( ask.get_max_debt_to_cover( match_price, feed_price, maintenance_collateral_ratio, maintenance_collateralization ), ask.debt_type() );
   asset call_pays, call_receives, order_pays, order_receives;

   if( taker_to_buy > taker_for_sale ) // fill limit order
   {  
      order_receives = taker_for_sale * match_price; // round down here, in favor of call order

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
   ilog( "Match order to call at price: ${p} \n ${b} \n ${a} \n",
      ("p",match_price.to_string())("b",bid.to_string())("a",ask.to_string()));

   FC_ASSERT( bid.sell_asset() == ask.debt_type() );
   FC_ASSERT( bid.receive_asset() == ask.collateral_type() );
   FC_ASSERT( bid.amount_for_sale().amount > 0 && ask.debt.amount > 0 && ask.collateral.amount > 0 );

   bool cull_taker = false;

   asset taker_for_sale = bid.amount_for_sale();
   asset taker_to_buy = asset( ask.get_max_debt_to_cover( match_price, feed_price, maintenance_collateral_ratio, maintenance_collateralization ), ask.debt_type() );
   asset call_pays, call_receives, order_pays, order_receives;

   if( taker_to_buy > taker_for_sale ) // fill limit order
   {  
      order_receives = taker_for_sale * match_price; // round down here, in favor of call order

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

asset database::match( const call_order_object& call, const asset_settlement_object& settle, const price& match_price, asset max_settlement, const price& fill_price )
{ try {
   ilog( "Match call to settle at price: ${p} \n ${c} \n ${s} \n",
      ("p",match_price.to_string())("c",call.to_string())("s",settle));

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
            cancel_settle_order( settle );
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

   fill_call_order( call, call_pays, call_receives, fill_price, true, settle.interface, false );           // call order is maker
   fill_settle_order( settle, settle_pays, settle_receives, fill_price, false, call.interface );           // force settlement order is taker

   if( cull_settle_order )
   {
      cancel_settle_order( settle );
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
   ilog( "Filling Limit Order: Pays: ${p} Receives: ${r} Order: ${o}",
      ("p",pays.to_string())("r",receives.to_string())("o",order.to_string()));

   FC_ASSERT( order.amount_for_sale().symbol == pays.symbol );
   FC_ASSERT( pays.symbol != receives.symbol );
   FC_ASSERT( pays.amount > 0 );
   FC_ASSERT( receives.amount > 0 );

   const account_object& seller = get_account( order.seller );
   const asset_object& recv_asset = get_asset( receives.symbol );
   time_point now = head_block_time();

   asset issuer_fees = asset( 0, receives.symbol );
   asset trading_fees = asset( 0, receives.symbol );
   asset fees_paid = asset( 0, receives.symbol );

   if( !is_maker )  // Pay fees if we are taker order
   {
      issuer_fees = pay_issuer_fees( seller, recv_asset, receives );      // fees paid to issuer of the asset
      trading_fees = pay_trading_fees( seller, receives, match_interface, order.interface );      // fees paid to the protocol, and interfaces
      fees_paid = issuer_fees + trading_fees;
   }

   asset delta = receives - fees_paid;

   adjust_pending_supply( -delta );
   adjust_liquid_balance( order.seller, delta );

   if( pays == order.amount_for_sale() )
   {
      ilog( "Removed: ${v}",("v",order));
      remove( order );
      return true;
   }
   else
   {
      modify( order, [&]( limit_order_object& b )
      {
         b.for_sale -= pays.amount;
         b.last_updated = now;
      });

      ilog( "Updated Limit Order: ${o}",
         ("o",order.to_string()));

      if( cull_if_small )
      {
         return maybe_cull_small_order( order );
      }
      else
      {
         return false;
      }
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
   ilog( "Filling Margin Order: Pays: ${p} Receives: ${r} Order: ${o}",
      ("p",pays.to_string())("r",receives.to_string())("o",order.to_string()));

   FC_ASSERT( order.amount_for_sale().symbol == pays.symbol );
   FC_ASSERT( pays.symbol != receives.symbol );
   FC_ASSERT( pays.amount > 0 );
   FC_ASSERT( receives.amount > 0 );

   const account_object& seller = get_account( order.owner );
   const asset_object& recv_asset = get_asset( receives.symbol );
   time_point now = head_block_time();

   asset issuer_fees = asset( 0, receives.symbol );
   asset trading_fees = asset( 0, receives.symbol );
   asset fees_paid = asset( 0, receives.symbol );

   if( !is_maker )  // Pay fees if we are taker order
   {
      issuer_fees = pay_issuer_fees( seller, recv_asset, receives );      // fees paid to issuer of the asset
      trading_fees = pay_trading_fees( seller, receives, match_interface, order.interface );      // fees paid to the protocol, and interfaces
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
      m.last_updated = now;
   });

   ilog( "Updated Margin Order: ${o}",
      ("o",order.to_string()));

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
 * Executes an option exercise order against an outstanding option underlying balance.
 */
bool database::fill_option_order( const option_order_object& order, const asset& pays, 
   const asset& receives, const asset& opt, const price& fill_price )
{ try {
   ilog( "Filling Option Order: Pays: ${p} Receives: ${r} Order: ${o}",
      ("p",pays.to_string())("r",receives.to_string())("o",order.to_string()));

   FC_ASSERT( pays.symbol != receives.symbol,
      "Pays symbol and Receives symbol must not be the same." );
   FC_ASSERT( order.strike_price.strike_price == fill_price,
      "Fill price is not the same price found in option order." );
   FC_ASSERT( opt.amount % BLOCKCHAIN_PRECISION == 0, 
      "Option orders can only be filled in units of 1." );
   FC_ASSERT( pays.amount > 0 );
   FC_ASSERT( receives.amount > 0 );
   
   time_point now = head_block_time();
   bool filled = false;

   if( receives < order.underlying_amount && opt < order.option_position )
   {
      modify( order, [&]( option_order_object& ooo )
      {
         ooo.underlying_amount -= receives;
         ooo.exercise_amount -= pays;
         ooo.option_position -= opt;
         ooo.last_updated = now;
      });

      filled = false;
   } 
   else 
   {
      filled = true;
   }

   adjust_liquid_balance( order.owner, pays );
   adjust_pending_supply( -receives );

   if( filled )
   {
      ilog( "Removed: ${v}",("v",order));
      remove( order );
   }

   return filled;

} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives)(opt) ) }


/**
 * Fills a an auction order against the daily single price closing auction.
 */
bool database::fill_auction_order( const auction_order_object& order, const asset& pays, const asset& receives,
   const price& fill_price )
{ try {
   ilog( "Filling Auction Order: Pays: ${p} Receives: ${r} Order: ${o}",
      ("p",pays.to_string())("r",receives.to_string())("o",order.to_string()));

   FC_ASSERT( order.amount_for_sale().symbol == pays.symbol );
   FC_ASSERT( ( pays * order.limit_close_price ) <= receives );
   FC_ASSERT( pays.symbol != receives.symbol );
   FC_ASSERT( pays.amount > 0 );
   FC_ASSERT( receives.amount > 0 );

   adjust_pending_supply( -pays );
   adjust_liquid_balance( order.owner, receives );

   ilog( "Removed: ${v}",("v",order));
   remove( order );
   return true;
   
} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) ) }


/**
 * Fills a call order that is below maintenance collateral ratio
 * against another order.
 */
bool database::fill_call_order( const call_order_object& order, const asset& pays, const asset& receives, const price& fill_price, 
   const bool is_maker, const account_name_type& match_interface, bool global_settle = false )
{ try {
   ilog( "Filling Call Order: Pays: ${p} Receives: ${r} Order: ${o}",
      ("p",pays.to_string())("r",receives.to_string())("o",order.to_string()));

   FC_ASSERT( order.debt_type() == receives.symbol );
   FC_ASSERT( order.collateral_type() == pays.symbol );
   FC_ASSERT( order.collateral.amount >= pays.amount );
   FC_ASSERT( pays.amount > 0 );
   FC_ASSERT( receives.amount > 0 );
   const account_object& seller = get_account( order.borrower );
   const asset_object& recv_asset = get_asset( receives.symbol );
   const asset_object& pays_asset = get_asset( pays.symbol );
   time_point now = head_block_time();
   asset issuer_fees;
   asset trading_fees;
   asset fees_paid = asset( 0, pays.symbol );
   FC_ASSERT( recv_asset.is_market_issued() );

   optional<asset> collateral_freed;

   if( !is_maker )  // Pay fees if we are taker order
   {
      issuer_fees = pay_issuer_fees( seller, pays_asset, pays );      // fees paid to issuer of the asset
      trading_fees = pay_trading_fees( seller, pays, match_interface, order.interface );      // fees paid to the protocol, and interfaces
      fees_paid = issuer_fees + trading_fees;
   }

   asset total_paid = pays + fees_paid;

   modify( order, [&]( call_order_object& o )
   {
      o.debt -= receives;
      o.collateral -= total_paid;
      if( o.debt.amount == 0 )
      {
         collateral_freed = o.amount_for_sale();
         o.collateral.amount = 0;
      }
      o.last_updated = now;
   });

   ilog( "Updated Call Order: ${o}",
      ("o",order.to_string()));

   if( !global_settle )
   {
      adjust_pending_supply( -receives );    // reduce the pending supply of the stablecoin, as it has been repaid, unless globally settling
   }

   if( collateral_freed.valid() )
   {
      adjust_pending_supply( -*collateral_freed );
      adjust_liquid_balance( order.borrower, *collateral_freed );     // Return collateral when freed.
   }

   if( collateral_freed.valid() )
   {
      ilog( "Removed: ${v}",("v",order));
      remove( order );
   }
   
   return collateral_freed.valid();
} FC_CAPTURE_AND_RETHROW( (order)(pays)(receives) ) }

/**
 * Executes a force settlement order filling from it being matched against a call order.
 */
bool database::fill_settle_order( const asset_settlement_object& settle, const asset& pays, const asset& receives, 
   const price& fill_price, const bool is_maker, const account_name_type& match_interface )
{ try {
   ilog( "Filling Settle Order: Pays: ${p} Receives: ${r} Order: ${o}",
      ("p",pays.to_string())("r",receives.to_string())("o",settle.to_string()));

   FC_ASSERT( pays.symbol != receives.symbol );
   FC_ASSERT( pays.amount > 0 );
   FC_ASSERT( receives.amount > 0 );

   const asset_object& rec_asset = get_asset( receives.symbol );
   const account_object& owner = get_account( settle.owner );
   time_point now = head_block_time();
   bool filled = false;

   auto issuer_fees = pay_issuer_fees( rec_asset, receives);
   auto trading_fees = pay_trading_fees( owner, receives, match_interface, settle.interface );  // Settlement order is always taker. 
   auto fees_paid = issuer_fees + trading_fees;

   if( pays < settle.balance )
   {
      modify( settle, [&]( asset_settlement_object& s )
      {
         s.balance -= pays;
         s.last_updated = now;
      });

      filled = false;
   } 
   else 
   {
      filled = true;
   }

   asset delta = receives - fees_paid;
   adjust_liquid_balance( settle.owner, delta );
   adjust_pending_supply( -delta );

   if( filled )
   {
      ilog( "Removed: ${v}",("v",settle));
      remove( settle );
   }

   return filled;

} FC_CAPTURE_AND_RETHROW( (settle)(pays)(receives)(fill_price)(is_maker) ) }



/**
 * Matches all auction orders each day at the same price.
 * 
 * Auction orders are cleared at the price which 
 * creates the greatest executable volume.
 * 
 * Finds the imbalance between cumulative supply and demand at every 
 * price point at each order and selects the price
 * which creates the lowest imbalance.
 * The order at which the imbalance becomes negative
 * is the last order to be included in the auction.
 */
void database::process_auction_orders()
{ try {
   if( ( head_block_num() % AUCTION_INTERVAL_BLOCKS ) != 0 )    // Runs once per day
      return;

   const auto& market_idx = get_index< auction_order_index >().indices().get< by_market >();
   const auto& high_price_idx = get_index< auction_order_index >().indices().get< by_high_price >();
   pair< asset_symbol_type, asset_symbol_type > market;

   auto market_itr = market_idx.begin();

   while( market_itr != market_idx.end() )
   {
      const auction_order_object& auction = *market_itr;
      market = auction.get_market();

      ilog( "Processing Auction Orders: ${m}",
         ("m",market));

      auto auction_itr = market_idx.lower_bound( market );
      auto auction_end = market_idx.upper_bound( market );

      while( auction_itr != auction_end )
      {
         ilog( "| Auction: ${a}",
            ("a",auction_itr->to_string()));
         ++auction_itr;
      }

      price min_price = auction.limit_close_price.min();
      price max_price = auction.limit_close_price.max();

      asset buy_supply = asset( 0, min_price.base.symbol );
      asset sell_supply = asset( 0, min_price.quote.symbol );
      asset buy_demand = asset( 0, min_price.base.symbol );
      asset sell_demand = asset( 0, min_price.quote.symbol );

      asset new_buy_supply = asset( 0, min_price.base.symbol );
      asset new_sell_supply = asset( 0, min_price.quote.symbol );
      asset new_buy_demand = asset( 0, min_price.base.symbol );
      asset new_sell_demand = asset( 0, min_price.quote.symbol );

      auto buy_begin = high_price_idx.lower_bound( max_price );
      auto buy_end = high_price_idx.upper_bound( min_price );

      auto sell_begin = high_price_idx.lower_bound( ~min_price );
      auto sell_end = high_price_idx.upper_bound( ~max_price );

      auto buy_itr = buy_begin;
      auto sell_itr = sell_begin;

      bool finished = false;

      while( buy_itr != buy_end && 
         sell_itr != sell_end && 
         buy_itr->get_market() == market && 
         sell_itr->get_market() == market &&
         !finished )
      {
         ilog( "Finding Auction Price: Buy supply: ${bs} Sell Supply: ${ss} Buy Demand: ${bd} Sell Demand: ${sd}",
            ("bs",buy_supply.to_string())("ss",sell_supply.to_string())("bd",buy_demand.to_string())("sd",sell_demand.to_string()));

         // If buy is lower, add to buy side, or sell side if sell is lower.

         if( new_buy_supply.amount < ( new_sell_supply * sell_itr->limit_close_price ).amount )
         {
            new_buy_supply += buy_itr->amount_for_sale();
            new_sell_demand = new_buy_supply * buy_itr->limit_close_price;
            ++buy_itr;
         }
         else
         {
            new_sell_supply += sell_itr->amount_for_sale();
            new_buy_demand = new_sell_supply * sell_itr->limit_close_price;
            ++sell_itr;
         }

         // Find the point where prices converge at equal supply and demand volume.

         if( ( new_buy_demand >= new_buy_supply || new_sell_demand >= new_sell_supply ) && 
            new_buy_supply.amount.value > 0 && new_sell_supply.amount.value > 0 && 
            new_buy_demand.amount.value > 0 && new_sell_demand.amount.value > 0 )
         {
            finished = true;
         }
         else
         {
            buy_supply = new_buy_supply;
            sell_supply = new_sell_supply;
            buy_demand = new_buy_demand;
            sell_demand = new_sell_demand;
         }
      }

      if( buy_supply.amount > 0 && sell_supply.amount > 0 )
      {
         price clearing_price = buy_supply / sell_supply;
         ilog( "Found Auction Clearing Price: ${p}",
            ("p",clearing_price.to_string()) );
         asset buy_remaining = buy_supply;
         asset sell_remaining = sell_supply;
         asset pays;
         asset receives;

         buy_itr = buy_begin;
      
         while( buy_itr != buy_end && 
            buy_itr->limit_close_price >= clearing_price &&
            sell_remaining.amount > 0 )
         {
            const auction_order_object& order = *buy_itr;
            pays = order.amount_for_sale();
            receives = pays * clearing_price;

            if( receives.amount > sell_remaining.amount )
            {
               receives = sell_remaining;
            }
            
            ++buy_itr;
            
            fill_auction_order( order, pays, receives, clearing_price );
            sell_remaining -= receives;
         }

         sell_itr = sell_begin;

         while( sell_itr != sell_end &&
            sell_itr->limit_close_price >= ~clearing_price &&
            buy_remaining.amount > 0 )
         {
            const auction_order_object& order = *sell_itr;
            pays = order.amount_for_sale();
            receives = pays * clearing_price;

            if( receives.amount > buy_remaining.amount )
            {
               receives = buy_remaining;
            }

            ++sell_itr;
            
            fill_auction_order( order, pays, receives, clearing_price );
            buy_remaining -= receives;
         }
      }
      
      market_itr = market_idx.upper_bound( market );
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Activates an option asset by trading the quote asset for the
 * underlying asset at the sprecifed strike price.
 */
bool database::exercise_option( const asset& option, const account_object& account )
{ try {
   ilog("Exercising Option asset: ${s}",("s",option));
   FC_ASSERT( option.amount % BLOCKCHAIN_PRECISION == 0, 
      "Option assets can only be exercised in units of 1." );
   
   option_strike strike = option_strike::from_string( option.symbol );
   time_point now = head_block_time();

   FC_ASSERT( strike.expiration() >= now, 
      "Option assets can only be exercised before their expiration date." );

   const asset_option_pool_object& pool = get_option_pool( strike.strike_price.base.symbol, strike.strike_price.quote.symbol );

   FC_ASSERT( std::find( pool.call_strikes.begin(), pool.call_strikes.end(), strike ) != pool.call_strikes.end() ||
      std::find( pool.put_strikes.begin(), pool.put_strikes.end(), strike ) != pool.put_strikes.end(),
      "Option pool chain sheet does not support the specified option stike: ${s}.", ("s", strike.option_symbol() ) );

   const auto& option_index = get_index< option_order_index >().indices().get< by_symbol >();

   auto option_itr = option_index.lower_bound( option.symbol );
   auto option_end = option_index.upper_bound( option.symbol );
   
   asset rec_total;
   asset pays_total;

   if( strike.call )    // Call Option pays base asset and receives quote asset.
   {
      rec_total = asset( option.amount * strike.multiple, strike.strike_price.quote.symbol );
      pays_total = rec_total * strike.strike_price;
   }
   else                 // Put Option pays quote asset and receives base asset.
   {
      pays_total = asset( option.amount * strike.multiple, strike.strike_price.quote.symbol );
      rec_total = pays_total * strike.strike_price;
   }

   asset opt_remaining = option;
   asset rec_remaining = rec_total;
   asset pays_remaining = pays_total;
   
   asset rec = asset( 0, rec_total.symbol );
   asset pays = asset( 0, pays_total.symbol );
   asset opt = asset( 0, option.symbol );

   asset received = asset( 0, rec_total.symbol );
   asset paid = asset( 0, pays_total.symbol );
   asset exercised = asset( 0, option.symbol );

   bool finished = false;

   while( option_itr != option_end && !finished )
   {
      rec = option_itr->amount_for_sale();
      pays = option_itr->amount_to_receive();
      opt = option_itr->option_position;

      if( opt >= opt_remaining || rec >= rec_remaining || pays >= pays_remaining )
      {
         opt = opt_remaining;
         rec = rec_remaining;
         pays = pays_remaining;
      }

      const option_order_object& order = *option_itr;
      ++option_itr;
      fill_option_order( order, pays, rec, opt, strike.strike_price );

      received += rec;
      rec_remaining -= rec;
      paid += pays;
      pays_remaining -= pays;
      exercised += opt;
      opt_remaining -= opt;

      if( rec_remaining.amount == 0 && pays_remaining.amount == 0 && opt_remaining.amount == 0 )
      {
         finished = true;
      }
   }

   adjust_liquid_balance( account.name, -exercised );
   adjust_liquid_balance( account.name, -paid );
   adjust_liquid_balance( account.name, received );
   
   return finished;
   
} FC_CAPTURE_AND_RETHROW() }



void database::cancel_bid( const asset_collateral_bid_object& bid )
{
   adjust_liquid_balance( bid.bidder, bid.collateral );

   ilog( "Removed: ${v}",("v",bid));
   remove( bid );
}

/**
 * Converts a processed collateral bid into a call order
 * with the requested debt and collateral values, plus collateral dispursed from
 * the settlement fund of the stablecoin.
 */
void database::execute_bid( const asset_collateral_bid_object& bid, share_type debt, 
   share_type collateral_from_fund, const price_feed& current_feed )
{
   const call_order_object& call = create< call_order_object >( [&]( call_order_object& call )
   {
      call.borrower = bid.bidder;
      call.collateral = asset( bid.collateral.amount + collateral_from_fund, bid.collateral.symbol );
      call.debt = asset( debt, bid.debt.symbol );
   });

   ilog( "Executed Collateral Bid and created Call: ${c}",
      ("c",call.to_string()));

   execute_bid_operation ebo;

   ebo.bidder = bid.bidder;
   ebo.collateral = asset( bid.collateral.amount + collateral_from_fund, bid.collateral.symbol );
   ebo.debt = asset( debt, bid.debt.symbol );

   push_virtual_operation( ebo );
   remove( bid );
}


void database::cancel_settle_order( const asset_settlement_object& order )
{
   adjust_liquid_balance( order.owner, order.balance );
   ilog( "Removed: ${v}",("v",order));
   remove( order );
}


void database::cancel_limit_order( const limit_order_object& order )
{
   asset refunded = order.amount_for_sale();
   adjust_liquid_balance( order.seller, refunded );
   ilog( "Removed: ${v}",("v",order));
   remove( order );
}

/**
 * Liquidates the remaining position held in a margin order.
 * 
 * If there is sufficient debt assetremaining, repays the loan.
 * If the order is in default, issues network credit to 
 * acquire the remaining deficit, and applies the default balance
 * to the account.
 * Returns the remaining collateral after the loan has been repaid, 
 * plus any profit denominated in the collateral asset.
 */
void database::close_margin_order( const margin_order_object& order )
{
   ilog( "Closing Margin order: ${s}",
      ("s",order));
   const account_object& owner = get_account( order.owner );
   asset collateral = order.collateral;
   asset to_repay = order.debt;
   asset interest = order.interest;
   asset debt_balance = order.debt_balance;
   asset returned_collateral;
   asset collateral_debt_value;
   asset debt_acquired;
   asset collateral_sold;
   const asset_credit_pool_object& credit_pool = get_credit_pool( order.debt_asset(), false );
   const credit_collateral_object& coll_balance = get_collateral( owner.name, order.collateral_asset() );
   time_point now = head_block_time();

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
         c.last_updated = now;
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
            c.last_updated = now;
         });
      }
   }

   asset interest_fees = ( interest * INTEREST_FEE_PERCENT ) / PERCENT_100;

   modify( credit_pool, [&]( asset_credit_pool_object& c )
   {
      c.base_balance += ( to_repay - interest_fees );
      c.borrowed_balance -= to_repay;
   });

   ilog( "Removed: ${v}",("v",order));
   remove( order );
}

void database::close_auction_order( const auction_order_object& order )
{
   asset refunded = order.amount_for_sale();
   adjust_liquid_balance( order.owner, refunded );
   ilog( "Removed: ${v}",("v",order));
   remove( order );
}


void database::close_option_order( const option_order_object& order )
{
   asset refunded = order.amount_for_sale();
   adjust_liquid_balance( order.owner, refunded );
   ilog( "Removed: ${v}",("v",order));
   remove( order );
}

/**
 * Cancels limit orders with 0 assets remaining for the recipient, 
 * Returns true if the order is cancelled.
 */
bool database::maybe_cull_small_order( const limit_order_object& order )
{
   ilog( "Maybe cull small order: ${o}",
      ("o",order.to_string()));
      
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
   ilog( "Maybe cull small order: ${o}",
      ("o",order.to_string()));

   if( order.liquidating )
   {
      if( order.amount_to_receive().amount == 0  )
      {
         close_margin_order( order );
         return true;
      }
   }
   return order.filled();
}


/**
 *  Starting with the least collateralized orders, fill them if their
 *  call price is above the max(lowest bid,call_limit).
 *  This method will return true if it filled a short or limit.
 */
bool database::check_call_orders( const asset_object& mia, bool enable_black_swan, bool for_new_limit_order )
{ try {
   ilog( "Checking Call orders: ${s}",
      ("s",mia.symbol));

   if( !mia.is_market_issued() )
   {
      return false;
   }

   const asset_stablecoin_data_object& stablecoin = get_stablecoin_data( mia.symbol );

   if( check_for_blackswan( mia, enable_black_swan, &stablecoin ) )
   {
      return false;
   }
   if( stablecoin.current_feed.settlement_price.is_null() )
   {
      return false;
   }

   const auto& limit_idx = get_index< limit_order_index >().indices().get< by_high_price >();
   const auto& call_idx = get_index< call_order_index >().indices().get< by_collateral >();

   price max_price = price::max( mia.symbol, stablecoin.backing_asset );      // looking for limit orders selling the most USD for the least CORE
   price min_price = stablecoin.current_feed.max_short_squeeze_price();       // stop when limit orders are selling too little USD for too much CORE

   auto limit_itr = limit_idx.lower_bound( max_price );                       // limit_price_index is sorted from greatest to least
   auto limit_end = limit_idx.upper_bound( min_price );

   if( limit_itr == limit_end ) 
   {
      return false;
   }

   price call_min = price::min( stablecoin.backing_asset, mia.symbol );
   price call_max = price::max( stablecoin.backing_asset, mia.symbol );

   auto call_itr = call_idx.begin();
   auto call_end = call_itr;
   
   call_itr = call_idx.lower_bound( call_min );
   call_end = call_idx.upper_bound( call_max );
   
   bool margin_called = false;

   uint64_t head_num = head_block_num();

   while( !check_for_blackswan( mia, enable_black_swan, &stablecoin ) && 
      limit_itr != limit_end && 
      limit_itr != limit_idx.end() &&
      call_itr != call_idx.end() &&
      call_itr != call_end )
   {
      const call_order_object& call_order = *call_itr;
      ilog( "Checking Call orders: ${c}",
         ("c",call_order.to_string()));

      if( stablecoin.current_maintenance_collateralization < call_order.collateralization() )
      {
         return margin_called;
      }
         
      const limit_order_object& limit_order = *limit_itr;
      price match_price = limit_order.sell_price;
      
      margin_called = true;

      auto usd_to_buy = call_order.debt;
      if( usd_to_buy * match_price > call_order.collateral )
      {
         elog( "black swan detected on asset ${symbol} (${id}) at block ${b}",
         ("id",mia.symbol)("symbol",mia.symbol)("b",head_num) );
         edump((enable_black_swan));
         FC_ASSERT( enable_black_swan );
         globally_settle_asset( mia, stablecoin.current_feed.settlement_price );
         return true;
      }

      usd_to_buy.amount = call_order.get_max_debt_to_cover( 
         match_price, 
         stablecoin.current_feed.settlement_price, 
         stablecoin.current_feed.maintenance_collateral_ratio, 
         stablecoin.current_maintenance_collateralization );
      
      asset usd_for_sale = limit_order.amount_for_sale();
      asset call_pays, call_receives, order_pays, order_receives;

      if( usd_to_buy > usd_for_sale ) // fill order
      {  
         order_receives = usd_for_sale * match_price; // round down, in favor of call order
         call_receives = order_receives.multiply_and_round_up( match_price );
      } 
      else // fill call
      { 
         call_receives = usd_to_buy;
         order_receives = usd_to_buy.multiply_and_round_up( match_price ); // round up, in favor of limit order
      }

      call_pays = order_receives;
      order_pays = call_receives;

      account_name_type limit_interface;
      account_name_type call_interface;

      if( limit_order.interface.size() )
      {
         limit_interface = limit_order.interface;
      }
      if( call_order.interface.size() )
      {
         call_interface = call_order.interface;
      }

      bool call_filled = fill_call_order( call_order, call_pays, call_receives, match_price, for_new_limit_order, limit_interface, false );
      bool limit_filled = fill_limit_order( limit_order, order_pays, order_receives, true, match_price, !for_new_limit_order, call_interface );

      if( call_filled ) 
      {
         ilog( "Call Order filled" );
      }

      if( limit_filled ) 
      {
         ilog( "Limit Order filled" );
         ++limit_itr;
      }

      call_itr = call_idx.lower_bound( call_min );
   }

   return margin_called;
} FC_CAPTURE_AND_RETHROW() }


/**
 * let HB = the highest bid for the collateral (aka who will pay the most DEBT for the least collateral)
 * let SP = current median feed's Settlement Price 
 * let LC = the least collateralized call order's swan price (debt/collateral)
 * If there is no valid price feed or no bids then there is no black swan.
 * A black swan occurs if MAX(HB,SP) <= LC
 */
bool database::check_for_blackswan( const asset_object& mia, bool enable_black_swan, const asset_stablecoin_data_object* stablecoin_ptr )
{
   ilog("Checking for Black Swan: ${s}",
      ("s",mia.symbol));

   if( !mia.is_market_issued() )       // Asset must be market issued
   {
      return false;
   } 

   const asset_stablecoin_data_object& stablecoin = ( stablecoin_ptr ? *stablecoin_ptr : get_stablecoin_data( mia.symbol ));
   if( stablecoin.has_settlement() )
   {
      return true;     // already force settled
   }
   auto settle_price = stablecoin.current_feed.settlement_price;

   if( settle_price.is_null() )
   {
      return false;      // no feed
   } 

   const call_order_object* call_ptr = nullptr; // place holder for the call order with least collateral ratio

   asset_symbol_type debt_asset_symbol = mia.symbol;
   auto call_min = price::min( stablecoin.backing_asset, debt_asset_symbol );

   const auto& call_idx = get_index< call_order_index >().indices().get< by_collateral >();
   auto call_itr = call_idx.lower_bound( call_min );
   if( call_itr == call_idx.end() ) // no call order
   {
      return false;
   } 
   call_ptr = &(*call_itr);
    
   if( call_ptr->debt_type() != debt_asset_symbol ) 
   {
      return false; // no call order
   }

   price highest = settle_price;
   
   highest = stablecoin.current_feed.max_short_squeeze_price();

   const auto& limit_index = get_index< limit_order_index >();
   const auto& limit_price_index = limit_index.indices().get< by_high_price >();

   // looking for limit orders selling the most USD for the least CORE
   price highest_possible_bid = price::max( mia.symbol, stablecoin.backing_asset );

   // stop when limit orders are selling too little USD for too much CORE
   price lowest_possible_bid  = price::min( mia.symbol, stablecoin.backing_asset );

   FC_ASSERT( highest_possible_bid.base.symbol == lowest_possible_bid.base.symbol );
   // limit_price_index is sorted from greatest to least

   auto limit_itr = limit_price_index.lower_bound( highest_possible_bid );
   auto limit_end = limit_price_index.upper_bound( lowest_possible_bid );

   if( limit_itr != limit_end ) 
   {
      FC_ASSERT( highest.base.symbol == limit_itr->sell_price.base.symbol );
      highest = std::max( limit_itr->sell_price, highest );
   }

   price least_collateral = call_ptr->collateralization();

   if( ~least_collateral >= highest  )    // Least collateralized order's Inverse Swan price is great than Max short squeeze price
   {
      wdump( (*call_ptr) );
      elog( "Black Swan detected on asset ${symbol} (${id}) at block ${b}: \n"
            "   Least collateralized call: ${lc}  ${~lc}\n"
            "   Highest Bid:               ${hb}  ${~hb}\n"
            "   Settle Price:              ${~sp}  ${sp}\n"
            "   Max:                       ${~h}  ${h}\n",
         ("id",mia.id)("symbol",mia.symbol)("b",head_block_num())
         ("lc",least_collateral.to_real())("~lc",(~least_collateral).to_real())
         ("hb",limit_itr->sell_price.to_real())("~hb",(~limit_itr->sell_price).to_real())
         ("sp",settle_price.to_real())("~sp",(~settle_price).to_real())
         ("h",highest.to_real())("~h",(~highest).to_real()) );
      edump((enable_black_swan));

      FC_ASSERT( enable_black_swan, 
         "Black swan was detected during a margin update which is not allowed to trigger a blackswan" );

      if( ~least_collateral <= settle_price )
      {
         globally_settle_asset( mia, settle_price );     // global settle at feed price if possible
      }
      else
      {
         globally_settle_asset( mia, ~least_collateral );
      }
         
      return true;
   } 
   return false;
}


/** 
 * Pays protocol trading fees on taker orders.
 * 
 * taker: The account that is the taker on the trade
 * receives: The asset object being received from the trade
 * maker_int: The owner account of the interface of the maker of the trade
 * taker_int: The owner account of the interface of the taker of the trade
 */
asset database::pay_trading_fees( const account_object& taker, const asset& receives, 
   const account_name_type& maker_int, const account_name_type& taker_int )
{ try {
   asset total_fees = ( receives * TRADING_FEE_PERCENT ) / PERCENT_100;
   asset network_fee = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset maker_interface_share = ( total_fees * MAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset taker_interface_share = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset maker_paid = asset( 0, receives.symbol );
   asset taker_paid = asset( 0, receives.symbol );

   if( maker_int.size() )
   {
      const account_object& m_int_acc = get_account( maker_int );
      const interface_object& m_interface = get_interface( maker_int );

      FC_ASSERT( m_int_acc.active && m_interface.active, 
         "Maker Interface: ${i} must be active",
         ("i",maker_int) );

      maker_paid = pay_fee_share( m_int_acc, maker_interface_share, true );
   }
   else
   {
      network_fee += maker_interface_share;
   }
   
   if( taker_int.size() )
   {
      const account_object& t_int_acc = get_account( taker_int );
      const interface_object& t_interface = get_interface( taker_int );

      FC_ASSERT( t_int_acc.active && t_interface.active, 
         "Taker Interface: ${i} must be active",
         ("i",taker_int) );

      taker_paid = pay_fee_share( t_int_acc, taker_interface_share, true );
   }
   else
   {
      network_fee += taker_interface_share;
   }
   
   pay_network_fees( taker, network_fee );

   asset total_paid = network_fee + maker_paid + taker_paid;

   ilog( "Account: ${a} paid trading fees: ${p}",
      ("a",taker.name)("p",total_paid.to_string()));
   return total_paid;
} FC_CAPTURE_AND_RETHROW() }

} } //node::chain