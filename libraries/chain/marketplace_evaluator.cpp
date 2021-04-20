#include <node/chain/node_evaluator.hpp>
#include <node/chain/database.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <cmath>

#include <node/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
//#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace node { namespace chain {


//================================//
// === Marketplace Evaluators === //
//================================//


void product_sale_evaluator::do_apply( const product_sale_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));

   time_point now = _db.head_block_time();

   const auto& product_idx = _db.get_index< product_sale_index >().indices().get< by_product_id >();
   auto product_itr = product_idx.find( std::make_tuple( o.account, o.product_id ) );

   if( product_itr == product_idx.end() )
   {
      _db.create< product_sale_object >([&]( product_sale_object& p )
      {
         p.account = o.account;
         from_string( p.product_id, o.product_id );
         from_string( p.name, o.name );
         from_string( p.url, o.url );
         from_string( p.json, o.json );

         p.product_variants.reserve( o.product_variants.size() );
         for( auto v : o.product_variants )
         {
            p.product_variants.push_back( v );
         }

         p.product_details.reserve( o.product_details.size() );
         for( auto d : o.product_details )
         {
            p.product_details.push_back( d );
         }

         from_string( p.product_image, o.product_image );

         p.product_prices.reserve( o.product_prices.size() );
         for( auto pr : o.product_prices )
         {
            p.product_prices.push_back( pr );
         }

         p.wholesale_discount = o.wholesale_discount;

         p.stock_available.reserve( o.stock_available.size() );
         for( auto sa : o.stock_available )
         {
            p.stock_available.push_back( sa );
         }

         p.delivery_variants.reserve( o.delivery_variants.size() );
         for( auto v : o.delivery_variants )
         {
            p.delivery_variants.push_back( v );
         }

         from_string( p.delivery_details, o.delivery_details );
         
         p.delivery_prices.reserve( o.delivery_prices.size() );
         for( auto dp : o.delivery_prices )
         {
            p.delivery_prices.push_back( dp );
         }

         p.active = true;
         p.created = now;
         p.last_updated = now;
      });
   }
   else
   {
      const product_sale_object& product = *product_itr;

      _db.modify( product, [&]( product_sale_object& p )
      {
         from_string( p.name, o.name );
         from_string( p.url, o.url );
         from_string( p.json, o.json );

         p.product_variants.clear();
         p.product_variants.reserve( o.product_variants.size() );
         for( auto v : o.product_variants )
         {
            p.product_variants.push_back( v );
         }

         p.product_details.clear();
         p.product_details.reserve( o.product_details.size() );
         for( auto d : o.product_details )
         {
            p.product_details.push_back( d );
         }

         from_string( p.product_image, o.product_image );

         p.product_prices.clear();
         p.product_prices.reserve( o.product_prices.size() );
         for( auto pr : o.product_prices )
         {
            p.product_prices.push_back( pr );
         }

         p.wholesale_discount = o.wholesale_discount;

         p.stock_available.clear();
         p.stock_available.reserve( o.stock_available.size() );
         for( auto sa : o.stock_available )
         {
            p.stock_available.push_back( sa );
         }

         p.delivery_variants.clear();
         p.delivery_variants.reserve( o.delivery_variants.size() );
         for( auto v : o.delivery_variants )
         {
            p.delivery_variants.push_back( v );
         }

         from_string( p.delivery_details, o.delivery_details );
         
         p.delivery_prices.clear();
         p.delivery_prices.reserve( o.delivery_prices.size() );
         for( auto dp : o.delivery_prices )
         {
            p.delivery_prices.push_back( dp );
         }

         p.last_updated = now;
         p.active = o.active;
      });
   }

   ilog( "Account: ${a} created product sale id: ${id}",("a",o.account)("id",o.product_id));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void product_purchase_evaluator::do_apply( const product_purchase_operation& o )
{ try {
   const account_object& buyer = _db.get_account( o.buyer );
   FC_ASSERT( buyer.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.buyer));

   time_point now = _db.head_block_time();

   const product_sale_object& product = _db.get_product_sale( o.seller, o.product_id );
   const account_object& seller = _db.get_account( o.seller );

   asset payment = asset( 0, product.product_prices[0].symbol );

   flat_map< fixed_string_32, asset > price_map;

   for( size_t i = 0; i < product.product_variants.size(); i++ )
   {
      price_map[ product.product_variants[ i ] ] = product.product_prices[ i ];
   }

   flat_map< fixed_string_32, asset > delivery_map;

   for( size_t i = 0; i < product.delivery_variants.size(); i++ )
   {
      delivery_map[ product.delivery_variants[ i ] ] = product.delivery_prices[ i ];
   }

   for( size_t i = 0; i < o.order_variants.size(); i++ )
   {
      payment += price_map[ o.order_variants[ i ] ] * o.order_size[ i ];
   }

   payment += delivery_map[ o.delivery_variant ];

   const auto& escrow_idx = _db.get_index< escrow_index >().indices().get< by_from_id >();
   auto escrow_itr = escrow_idx.find( std::make_tuple( o.buyer, o.order_id ) );

   const auto& purchase_idx = _db.get_index< product_purchase_index >().indices().get< by_order_id >();
   auto purchase_itr = purchase_idx.find( std::make_tuple( o.buyer, o.order_id ) );

   if( escrow_itr == escrow_idx.end() && 
      purchase_itr == purchase_idx.end() )
   {
      _db.create< product_purchase_object >([&]( product_purchase_object& p )
      {
         p.buyer = o.buyer;
         from_string( p.order_id, o.order_id );
         p.seller = o.seller;
         from_string( p.product_id, o.product_id );

         p.order_variants.reserve( o.order_variants.size() );
         for( auto va : o.order_variants )
         {
            p.order_variants.push_back( va );
         }

         p.order_size.reserve( o.order_size.size() );
         for( auto os : o.order_size )
         {
            p.order_size.push_back( os );
         }
         
         from_string( p.memo, o.memo );
         from_string( p.json, o.json );
         p.purchase_public_key = seller.secure_public_key;
         from_string( p.shipping_address, o.shipping_address );
         p.delivery_variant = o.delivery_variant;
         from_string( p.delivery_details, o.delivery_details );
         p.order_value = payment;
         p.created = now;
         p.last_updated = now;
      });

      _db.create< escrow_object >([&]( escrow_object& esc )
      {
         esc.from = o.buyer;
         esc.to = o.seller;
         esc.from_mediator = account_name_type();
         esc.to_mediator = account_name_type();
         esc.payment = payment;
         esc.balance = asset( 0, payment.symbol );
         from_string( esc.escrow_id, o.order_id );     // Order ID used for escrow ID
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.dispute_release_time = time_point::maximum();
         esc.approvals[ o.buyer ] = false;
         esc.approvals[ o.seller ] = false;
         esc.created = now;
         esc.last_updated = now;
      });
   }
   else if( escrow_itr != escrow_idx.end() && 
      purchase_itr != purchase_idx.end() )
   {
      const escrow_object& escrow = *escrow_itr;
      const product_purchase_object& purchase = *purchase_itr;

      for( auto a : escrow.approvals )
      {
         FC_ASSERT( a.second == false, 
            "Cannot edit product purchase after escrow approvals have been made." );
      }

      _db.modify( purchase, [&]( product_purchase_object& p )
      {
         p.order_variants.clear();
         p.order_variants.reserve( o.order_variants.size() );
         for( auto va : o.order_variants )
         {
            p.order_variants.push_back( va );
         }

         p.order_size.clear();
         p.order_size.reserve( o.order_size.size() );
         for( auto os : o.order_size )
         {
            p.order_size.push_back( os );
         }

         p.order_value = payment;
         from_string( p.memo, o.memo );
         from_string( p.json, o.json );
         from_string( p.shipping_address, o.shipping_address );
         p.delivery_variant = o.delivery_variant;
         from_string( p.delivery_details, o.delivery_details );
         p.last_updated = now;
      });

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.payment = payment;
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.last_updated = now;
      });
   }

   ilog( "Buyer: ${a} purchased product id: ${id}",("a",o.buyer)("id",o.product_id));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void product_auction_sale_evaluator::do_apply( const product_auction_sale_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   time_point now = _db.head_block_time();

   product_auction_type product_auction = product_auction_type::OPEN_AUCTION;

   for( size_t i = 0; i < product_auction_values.size(); i++ )
   {
      if( o.auction_type == product_auction_values[ i ] )
      {
         product_auction = product_auction_type( i );
         break;
      }
   }

   const auto& auction_idx = _db.get_index< product_auction_sale_index >().indices().get< by_auction_id >();
   auto auction_itr = auction_idx.find( std::make_tuple( o.account, o.auction_id ) );

   const auto& bid_idx = _db.get_index< product_auction_bid_index >().indices().get< by_auction_id >();
   auto bid_itr = bid_idx.find( std::make_tuple( o.account, o.auction_id ) );

   if( auction_itr == auction_idx.end() )
   {
      FC_ASSERT( o.final_bid_time > ( now + fc::days(7) ),
         "The final bid time must be at least 7 days after head block time." );

      switch( product_auction )
      {
         case product_auction_type::OPEN_AUCTION:
         case product_auction_type::REVERSE_AUCTION:
         {
            FC_ASSERT( o.completion_time == o.final_bid_time,
               "The completion time must be at least 7 days after final bid time." );
         }
         break;
         case product_auction_type::CONCEALED_FIRST_PRICE_AUCTION:
         case product_auction_type::CONCEALED_SECOND_PRICE_AUCTION:
         {
            FC_ASSERT( o.completion_time > ( o.final_bid_time + fc::days(7) ),
               "The completion time must be at least 7 days after final bid time." );
         }
         break;
         default:
         {
            FC_ASSERT( false,
               "The product auction type is invalid." );
         }
      }

      _db.create< product_auction_sale_object >([&]( product_auction_sale_object& p )
      {
         p.account = o.account;
         from_string( p.auction_id, o.auction_id );
         p.auction_type = product_auction;
         from_string( p.name, o.name );
         from_string( p.url, o.url );
         from_string( p.json, o.json );
         from_string( p.product_details, o.product_details );
         from_string( p.product_image, o.product_image );

         p.reserve_bid = o.reserve_bid;
         p.maximum_bid = o.maximum_bid;

         p.delivery_variants.reserve( o.delivery_variants.size() );
         for( auto dv : o.delivery_variants )
         {
            p.delivery_variants.push_back( dv );
         }

         from_string( p.delivery_details, o.delivery_details );
         
         p.delivery_prices.reserve( o.delivery_prices.size() );
         for( auto dp : o.delivery_prices )
         {
            p.delivery_prices.push_back( dp );
         }

         p.final_bid_time = o.final_bid_time;
         p.completion_time = o.completion_time;
         p.winning_bidder = account_name_type();
         from_string( p.winning_bid_id, "" );
         p.created = now;
         p.last_updated = now;
      });
   }
   else
   {
      const product_auction_sale_object& auction = *auction_itr;

      if( bid_itr == bid_idx.end() )    // No Existing bids
      {
         FC_ASSERT( o.final_bid_time > ( now + fc::days(7) ),
         "The final bid time must be at least 7 days after head block time." );

         switch( product_auction )
         {
            case product_auction_type::OPEN_AUCTION:
            case product_auction_type::REVERSE_AUCTION:
            {
               FC_ASSERT( o.completion_time == o.final_bid_time,
                  "The completion time must be at least 7 days after final bid time." );
            }
            break;
            case product_auction_type::CONCEALED_FIRST_PRICE_AUCTION:
            case product_auction_type::CONCEALED_SECOND_PRICE_AUCTION:
            {
               FC_ASSERT( o.completion_time > ( o.final_bid_time + fc::days(7) ),
                  "The completion time must be at least 7 days after final bid time." );
            }
            break;
            default:
            {
               FC_ASSERT( false,
                  "The product auction type is invalid." );
            }
         }

         _db.modify( auction, [&]( product_auction_sale_object& p )
         {
            from_string( p.name, o.name );
            from_string( p.url, o.url );
            from_string( p.json, o.json );
            from_string( p.product_details, o.product_details );
            from_string( p.product_image, o.product_image );

            p.reserve_bid = o.reserve_bid;
            p.maximum_bid = o.maximum_bid;

            p.delivery_variants.clear();
            p.delivery_variants.reserve( o.delivery_variants.size() );
            for( auto dv : o.delivery_variants )
            {
               p.delivery_variants.push_back( dv );
            }

            from_string( p.delivery_details, o.delivery_details );
            
            p.delivery_prices.clear();
            p.delivery_prices.reserve( o.delivery_prices.size() );
            for( auto dp : o.delivery_prices )
            {
               p.delivery_prices.push_back( dp );
            }

            p.final_bid_time = o.final_bid_time;
            p.completion_time = o.completion_time;
            p.last_updated = now;
         });
      }
      else    // Existing bids on the auction
      {
         _db.modify( auction, [&]( product_auction_sale_object& p )
         {
            from_string( p.name, o.name );
            from_string( p.url, o.url );
            from_string( p.json, o.json );
            from_string( p.product_details, o.product_details );
            from_string( p.product_image, o.product_image );

            p.last_updated = now;
         });
      }
   }

   ilog( "Account: ${a} created product auction id: ${id}",("a",o.account)("id",o.auction_id));

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void product_auction_bid_evaluator::do_apply( const product_auction_bid_operation& o )
{ try {
   const account_object& buyer = _db.get_account( o.buyer );
   FC_ASSERT( buyer.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.buyer));

   time_point now = _db.head_block_time();

   const product_auction_sale_object& auction = _db.get_product_auction_sale( o.seller, o.auction_id );
   const account_object& seller = _db.get_account( o.seller );

   FC_ASSERT( auction.completion_time > now, 
      "Cannot create or update bids after auction is completed." );
   FC_ASSERT( o.public_bid_amount == auction.reserve_bid.amount,
      "The completion time must be at least 7 days after final bid time." );

   flat_map< fixed_string_32, asset > delivery_map;

   for( size_t i = 0; i < auction.delivery_variants.size(); i++ )
   {
      delivery_map[ auction.delivery_variants[ i ] ] = auction.delivery_prices[ i ];
   }

   asset delivery_value = delivery_map[ o.delivery_variant ];

   const auto& bid_idx = _db.get_index< product_auction_bid_index >().indices().get< by_bid_id >();
   auto bid_itr = bid_idx.find( std::make_tuple( o.buyer, o.bid_id ) );

   const auto& escrow_idx = _db.get_index< escrow_index >().indices().get< by_from_id >();
   auto escrow_itr = escrow_idx.find( std::make_tuple( o.buyer, o.bid_id ) );

   FC_ASSERT( escrow_itr == escrow_idx.end(),
      "Escrow Transfer already in progress for this bid id, select different bid id." );

   share_type bid_price = 0;

   if( bid_itr == bid_idx.end() )
   {
      FC_ASSERT( auction.final_bid_time > now,
         "Cannot create new bid after final bid time." );

      switch( auction.auction_type )
      {
         case product_auction_type::OPEN_AUCTION:
         {
            FC_ASSERT( o.public_bid_amount >= auction.reserve_bid.amount,
               "Bid amount must be greater than or equal to the auction reserve price." );
            FC_ASSERT( o.public_bid_amount <= auction.maximum_bid.amount,
               "Bid amount must be less than or equal to the maximum bid price." );
            FC_ASSERT( o.bid_price_commitment == commitment_type(),
               "Open bid must not have a bid price commitment." );
            FC_ASSERT( o.blinding_factor == blind_factor_type(),
               "Open bid must not have a blinding factor." );
            bid_price = o.public_bid_amount;
         }
         break;
         case product_auction_type::REVERSE_AUCTION:
         {
            bid_price = auction.current_reverse_price( now );

            _db.modify( auction, [&]( product_auction_sale_object& paso )
            {
               paso.last_updated = now;
               paso.completion_time = now;
            });
         }
         break;
         case product_auction_type::CONCEALED_FIRST_PRICE_AUCTION:
         case product_auction_type::CONCEALED_SECOND_PRICE_AUCTION:
         {
            FC_ASSERT( o.bid_price_commitment != commitment_type(),
               "Concealed bid must have a bid price commitment." );
            FC_ASSERT( o.blinding_factor != blind_factor_type(),
               "Concealed bid must have a blinding factor." );
         }
         break;
         default:
         {
            FC_ASSERT( false,
               "The product auction type is invalid." );
         }
      }

      _db.create< product_auction_bid_object >([&]( product_auction_bid_object& p )
      {
         p.buyer = o.buyer;
         from_string( p.bid_id, o.bid_id );
         p.seller = o.seller;
         from_string( p.auction_id, o.auction_id );
         p.bid_asset = auction.reserve_bid.symbol;
         p.bid_price_commitment = o.bid_price_commitment;
         p.blinding_factor = o.blinding_factor;
         p.public_bid_amount = bid_price;
         
         from_string( p.memo, o.memo );
         from_string( p.json, o.json );

         p.bid_public_key = seller.secure_public_key;

         from_string( p.shipping_address, o.shipping_address );
         p.delivery_variant = o.delivery_variant;
         from_string( p.delivery_details, o.delivery_details );

         p.delivery_value = delivery_value;
         p.created = now;
         p.last_updated = now;
         p.completion_time = auction.completion_time;
         p.winning_bid = false;
      });

      _db.modify( auction, [&]( product_auction_sale_object& paso )
      {
         paso.bid_count++;
      });
   }
   else
   {
      const product_auction_bid_object& bid = *bid_itr;

      FC_ASSERT( auction.completion_time > now, 
         "Cannot edit bid after auction completion time." );

      switch( auction.auction_type )
      {
         case product_auction_type::OPEN_AUCTION:
         {
            FC_ASSERT( o.public_bid_amount >= auction.reserve_bid.amount,
               "Bid amount must be greater than or equal to the auction reserve price." );
            FC_ASSERT( o.public_bid_amount <= auction.maximum_bid.amount,
               "Bid amount must be less than or equal to the auction maximum price." );
            bid_price = o.public_bid_amount;
         }
         break;
         case product_auction_type::REVERSE_AUCTION:
         {
            bid_price = auction.current_reverse_price( now );
         }
         break;
         case product_auction_type::CONCEALED_FIRST_PRICE_AUCTION:
         case product_auction_type::CONCEALED_SECOND_PRICE_AUCTION:
         {
            if( now > auction.final_bid_time )     // After bids are locked in, reveal phase.
            {
               FC_ASSERT( o.bid_price_commitment == bid.bid_price_commitment, 
                  "Cannot change bid price commitment after final bid time." );
               FC_ASSERT( o.blinding_factor == bid.blinding_factor, 
                  "Cannot change blinding factor after final bid time." );
               FC_ASSERT( o.public_bid_amount >= auction.reserve_bid.amount,
                  "Bid amount must be greater than or equal to the auction reserve price." );
               FC_ASSERT( o.public_bid_amount <= auction.maximum_bid.amount,
                  "Bid amount must be less than or equal to the auction maximum price." );

               commitment_type public_c = fc::ecc::blind( o.blinding_factor, o.public_bid_amount.value );

               FC_ASSERT( fc::ecc::verify_sum( { public_c }, { o.bid_price_commitment }, 0 ), 
                  "Public bid amount ${a} does not validate with bid price commitment.", 
                  ("a", o.public_bid_amount) );
               bid_price = o.public_bid_amount;
            }
         }
         break;
         default:
         {
            FC_ASSERT( false,
               "The product auction type is invalid." );
         }
      }

      _db.modify( bid, [&]( product_auction_bid_object& p )
      {
         p.bid_price_commitment = o.bid_price_commitment;
         p.blinding_factor = o.blinding_factor;
         p.public_bid_amount = bid_price;
         
         from_string( p.memo, o.memo );
         from_string( p.json, o.json );

         p.bid_public_key = seller.secure_public_key;

         from_string( p.shipping_address, o.shipping_address );
         p.delivery_variant = o.delivery_variant;
         from_string( p.delivery_details, o.delivery_details );

         p.delivery_value = delivery_value;
         p.last_updated = now;
      });
   }

   ilog( "Buyer: ${a} made purchase bid on product auction id: ${id}",("a",o.buyer)("id",o.auction_id));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void escrow_transfer_evaluator::do_apply( const escrow_transfer_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   time_point now = _db.head_block_time();

   FC_ASSERT( o.account == o.to || 
      o.account == o.from,
      "Account must be a party to the escrow transaction to initiate transfer proposal." );
   FC_ASSERT( o.acceptance_time > now, 
      "The escrow ratification deadline must be after head block time." );
   FC_ASSERT( o.escrow_expiration > now, 
      "The escrow expiration must be after head block time." );

   const account_object& to_account = _db.get_account( o.to );
   FC_ASSERT( to_account.active, 
      "Account: ${s} must be active to begin escrow transfer.",("s", o.to) );
   const account_object& from_account = _db.get_account( o.from );
   FC_ASSERT( from_account.active, 
      "Account: ${s} must be active to begin escrow transfer.",("s", o.from) );
   asset liquid = _db.get_liquid_balance( o.from, o.amount.symbol );
   FC_ASSERT( liquid >= o.amount,
      "Account cannot cover costs of escrow. Required: ${r} Available: ${a}", ("r",o.amount)("a",liquid) );

   const auto& escrow_idx = _db.get_index< escrow_index >().indices().get< by_from_id >();
   auto escrow_itr = escrow_idx.find( std::make_tuple( o.from, o.escrow_id ) );

   if( escrow_itr == escrow_idx.end() )
   {
      _db.create< escrow_object >([&]( escrow_object& esc )
      {
         esc.from = o.from;
         esc.to = o.to;
         esc.from_mediator = account_name_type();
         esc.to_mediator = account_name_type();
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.payment = o.amount;
         esc.balance = asset( 0, o.amount.symbol );
         from_string( esc.escrow_id, o.escrow_id );
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.dispute_release_time = time_point::maximum();
         esc.approvals[ o.from ] = false;
         esc.approvals[ o.to ] = false;
         esc.created = now;
         esc.last_updated = now;
      });
   }
   else
   {
      const escrow_object& escrow = *escrow_itr;

      for( auto a : escrow.approvals )
      {
         FC_ASSERT( a.second == false, 
            "Cannot edit escrow after approvals have been made." );
      }

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.acceptance_time = o.acceptance_time;
         esc.escrow_expiration = o.escrow_expiration;
         esc.payment = o.amount;
         from_string( esc.memo, o.memo );
         from_string( esc.json, o.json );
         esc.last_updated = now;
      });
   }

   ilog( "Account: ${a} created escrow transfer id: ${id}",("a",o.account)("id",o.escrow_id));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void escrow_approve_evaluator::do_apply( const escrow_approve_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const escrow_object& escrow = _db.get_escrow( o.escrow_from, o.escrow_id );
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   const account_object& mediator_account = _db.get_account( o.mediator );
   FC_ASSERT( mediator_account.active, 
      "Account: ${s} must be active to be assigned to escrow transfer.",("s", o.mediator) );
   const mediator_object& mediator_obj = _db.get_mediator( o.mediator );
   FC_ASSERT( mediator_obj.active, 
      "Mediator: ${s} must be active to be assigned to escrow transfer.",("s", o.mediator) );
   FC_ASSERT( o.account != escrow.to || o.mediator != o.account, 
      "Mediator: ${s} selection must not be the fund recipient of escrow transfer.",("s", o.mediator) );

   asset liquid = _db.get_liquid_balance( o.account, escrow.payment.symbol );
   // Escrow bond is a percentage paid as security in the event of dispute, and can be forfeited.
   asset escrow_bond = asset( ( escrow.payment.amount * median_props.escrow_bond_percent ) / PERCENT_100, escrow.payment.symbol );
   asset from_amount = escrow.payment + escrow_bond;

   flat_map< account_name_type, bool > approvals = escrow.approvals;

   FC_ASSERT( !approvals[ o.account ],
      "Account: ${a} has already approved the escrow.", ("a", o.account ) );

   time_point now = _db.head_block_time();

   if( escrow.disputed )
   {
      FC_ASSERT( escrow.is_mediator( o.account ),
         "Account must be an allocated mediator to approve during a dispute." );
      FC_ASSERT( o.approved,
         "Mediator cannot select to cancel a disputed escrow" );
   }
   else
   {
      FC_ASSERT( o.account == escrow.to || 
         o.account == escrow.from || 
         o.account == escrow.to_mediator || 
         o.account == escrow.from_mediator,
         "Account must be a party to the escrow transaction to approve." );
   }

   if( o.approved )
   {
      if( o.account == escrow.from )
      {
         FC_ASSERT( liquid >= from_amount,
            "Account cannot cover costs of escrow. Required: ${r} Available: ${a}", 
            ("r", escrow_bond)("a",liquid) );

         _db.adjust_liquid_balance( o.account, -from_amount );
         _db.adjust_pending_supply( from_amount );
      }
      else
      {
         FC_ASSERT( liquid >= escrow_bond,
            "Account cannot cover costs of escrow. Required: ${r} Available: ${a}",
            ("r", escrow_bond)("a",liquid) );

         _db.adjust_liquid_balance( o.account, -escrow_bond );
         _db.adjust_pending_supply( escrow_bond );
      }

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.approvals[ o.account ] = true;
         if( o.account == escrow.from )
         {
            esc.balance += from_amount;
            esc.from_mediator = o.mediator;     // Adds selected mediator to escrow
         }
         else if( o.account == escrow.to )
         {
            esc.balance += escrow_bond;
            esc.to_mediator = o.mediator;      // Adds selected mediator to escrow
         }
         else if( o.account == escrow.from_mediator )
         {
            esc.balance += escrow_bond;
         }
         else if( o.account == escrow.to_mediator )
         {
            esc.balance += escrow_bond;
         }
         esc.last_updated = now;
      });
   }
   else       // Any accounts can cancel the escrow and refund all previous payments
   {
      _db.release_escrow( escrow );
   }

   ilog( "Account: ${a} approved escrow: \n ${e} \n",("a",o.account)("e",escrow));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void escrow_dispute_evaluator::do_apply( const escrow_dispute_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const escrow_object& escrow = _db.get_escrow( o.escrow_from, o.escrow_id );
   time_point now = _db.head_block_time();

   FC_ASSERT( o.account == escrow.to || o.account == escrow.from,
      "Account must be either sender or receiver of the escrow transaction to dispute." );
   FC_ASSERT( now < escrow.escrow_expiration,
      "Disputing the escrow must happen before expiration." );
   FC_ASSERT( escrow.is_approved(),
      "The escrow must be approved by all parties and selected mediators before a dispute can be raised." );
   FC_ASSERT( !escrow.disputed,
      "The escrow is already under dispute." );

   _db.dispute_escrow( escrow );

   ilog( "Account: ${a} disputed escrow id: ${id}",
      ("a",o.account)("id",o.escrow_id));

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void escrow_release_evaluator::do_apply( const escrow_release_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const escrow_object& escrow = _db.get_escrow( o.escrow_from, o.escrow_id );
   time_point now = _db.head_block_time();
   
   FC_ASSERT( escrow.is_approved(),
      "Funds cannot be released prior to escrow approval by all participating accounts." );
   FC_ASSERT( o.account == escrow.to || 
      o.account == escrow.from || 
      o.account == escrow.to_mediator || 
      o.account == escrow.from_mediator || 
      escrow.is_mediator( o.account ),
      "Account must be a participant in the escrow transaction to release funds." );
   FC_ASSERT( escrow.approvals.at( o.account ) == true,
      "Account must have approved and deposited an escrow bond with the escrow transaction to release funds." );
   
   if( escrow.disputed )  // If there is a dispute, create release percent vote
   {
      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.release_percentages[ o.account ] = o.release_percent;
      });

      ilog( "Account: ${a} released escrow with release percent: ${p} \n ${e} \n",
         ("a",o.account)("p",o.release_percent)("e",escrow));
   }
   else
   {
      FC_ASSERT( o.account == escrow.from || 
         o.account == escrow.to,
         "Only Sender: ${f} and receiver: ${t} can release funds from a non-disputed escrow",
         ("f",escrow.from)("t", escrow.to));

      if( escrow.escrow_expiration > now )     // If escrow expires and there is no dispute, either party can release funds to either party.
      {
         if( o.account == escrow.from )        // If there is no dispute and escrow has not expired, either party can release funds to the other.
         {
            FC_ASSERT( o.release_percent == PERCENT_100,
               "Release percent should be PERCENT_100 before expiration." );
         }
         else if( o.account == escrow.to )
         {
            FC_ASSERT( o.release_percent == 0,
               "Release percent should be 0 before expiration." );
         }
      }

      _db.modify( escrow, [&]( escrow_object& esc )
      {
         esc.release_percentages[ o.account ] = o.release_percent;
      });

      _db.release_escrow( escrow );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }

} } // node::chain