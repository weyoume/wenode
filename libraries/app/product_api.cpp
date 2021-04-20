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


   //=====================//
   // === Product API === //
   //=====================//


product_sale_api_obj database_api::get_product_sale( string seller, string product_id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_product_sale( seller, product_id );
   });
}


/**
 * Retrieves a list of products and their purchase orders by ID.
 */
product_sale_api_obj database_api_impl::get_product_sale( string seller, string product_id )const
{
   product_sale_api_obj results;
   
   const auto& product_idx = _db.get_index< product_sale_index >().indices().get< by_product_id >();
   
   auto product_itr = product_idx.find( boost::make_tuple( seller, product_id ) );
   if( product_itr != product_idx.end() )
   {
      results = product_sale_api_obj( *product_itr );
   }

   return results;
}


product_auction_sale_api_obj database_api::get_product_auction_sale( string seller, string auction_id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_product_auction_sale( seller, auction_id );
   });
}


product_auction_sale_api_obj database_api_impl::get_product_auction_sale( string seller, string auction_id )const
{
   product_auction_sale_api_obj results;
   
   const auto& product_idx = _db.get_index< product_auction_sale_index >().indices().get< by_auction_id >();
   
   auto product_itr = product_idx.find( boost::make_tuple( seller, auction_id ) );
   if( product_itr != product_idx.end() )
   {
      results = product_auction_sale_api_obj( *product_itr );
   }

   return results;
}


vector< account_product_state > database_api::get_account_products( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_products( names );
   });
}


/**
 * Retrieves a list of products and their purchase orders according to the sellers.
 */
vector< account_product_state > database_api_impl::get_account_products( vector< string > names )const
{
   vector< account_product_state > results;
   
   const auto& product_idx = _db.get_index< product_sale_index >().indices().get< by_product_id >();
   const auto& seller_purchase_idx = _db.get_index< product_purchase_index >().indices().get< by_product_id >();
   const auto& buyer_purchase_idx = _db.get_index< product_purchase_index >().indices().get< by_order_id >();

   const auto& auction_idx = _db.get_index< product_auction_sale_index >().indices().get< by_auction_id >();
   const auto& seller_bid_idx = _db.get_index< product_auction_bid_index >().indices().get< by_auction_id >();
   const auto& buyer_bid_idx = _db.get_index< product_auction_bid_index >().indices().get< by_bid_id >();
   
   auto product_itr = product_idx.begin();
   auto buyer_purchase_itr = buyer_purchase_idx.begin();
   auto seller_purchase_itr = seller_purchase_idx.begin();

   auto auction_itr = auction_idx.begin();
   auto seller_bid_itr = seller_bid_idx.begin();
   auto buyer_bid_itr = buyer_bid_idx.begin();

   for( auto acc : names )
   {
      account_product_state pstate;
      product_itr = product_idx.lower_bound( acc );

      while( product_itr != product_idx.end() && 
         product_itr->account == acc )
      {
         pstate.seller_products.push_back( product_sale_api_obj( *product_itr ) );
         ++product_itr;
      }

      seller_purchase_itr = seller_purchase_idx.lower_bound( acc );

      while( seller_purchase_itr != seller_purchase_idx.end() && 
         seller_purchase_itr->seller == acc )
      {
         pstate.seller_orders.push_back( product_purchase_api_obj( *seller_purchase_itr ) );
         ++seller_purchase_itr;
      }

      buyer_purchase_itr = buyer_purchase_idx.lower_bound( acc );

      while( buyer_purchase_itr != buyer_purchase_idx.end() && 
         buyer_purchase_itr->buyer == acc )
      {
         pstate.buyer_orders.push_back( product_purchase_api_obj( *buyer_purchase_itr ) );
         ++buyer_purchase_itr;
      }

      flat_set< pair < account_name_type, string > > buyer_products;

      for( auto product : pstate.buyer_orders )
      {
         buyer_products.insert( std::make_pair( product.seller, product.product_id ) );
      }

      for( auto product : buyer_products )
      {
         product_itr = product_idx.find( boost::make_tuple( product.first, product.second ) );
         if( product_itr != product_idx.end() )
         {
            pstate.buyer_products.push_back( product_sale_api_obj( *product_itr ) );
         }
      }

      auction_itr = auction_idx.lower_bound( acc );

      while( auction_itr != auction_idx.end() && 
         auction_itr->account == acc )
      {
         pstate.seller_auctions.push_back( product_auction_sale_api_obj( *auction_itr ) );
         ++auction_itr;
      }

      seller_bid_itr = seller_bid_idx.lower_bound( acc );

      while( seller_bid_itr != seller_bid_idx.end() && 
         seller_bid_itr->seller == acc )
      {
         pstate.seller_bids.push_back( product_auction_bid_api_obj( *seller_bid_itr ) );
         ++seller_bid_itr;
      }

      buyer_bid_itr = buyer_bid_idx.lower_bound( acc );

      while( buyer_bid_itr != buyer_bid_idx.end() && 
         buyer_bid_itr->buyer == acc )
      {
         pstate.buyer_bids.push_back( product_auction_bid_api_obj( *buyer_bid_itr ) );
         ++buyer_bid_itr;
      }

      flat_set< pair < account_name_type, string > > buyer_auctions;

      for( auto bid : pstate.buyer_bids )
      {
         buyer_auctions.insert( std::make_pair( bid.seller, bid.bid_id ) );
      }

      for( auto auction : buyer_auctions )
      {
         auction_itr = auction_idx.find( boost::make_tuple( auction.first, auction.second ) );
         if( auction_itr != auction_idx.end() )
         {
            pstate.buyer_auctions.push_back( product_auction_sale_api_obj( *auction_itr ) );
         }
      }

      results.push_back( pstate );
   }
   
   return results;
}


} } // node::app