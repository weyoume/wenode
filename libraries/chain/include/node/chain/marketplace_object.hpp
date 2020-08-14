#pragma once

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/block.hpp>
#include <node/chain/node_object_types.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <numeric>

namespace node { namespace chain {

   /**
    * Enables a merchant to list a product for sale.
    * 
    * Users can search for products and make purchase orders for 
    * products from merchants.
    * 
    * Manages stock available and product variants and prices.
    * Product objects can be sold according to a variety of sale types,
    * which offer variable price mechanisms.
    */
   class product_sale_object : public object< product_sale_object_type, product_sale_object >
   {
      product_sale_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         product_sale_object( Constructor&& c, allocator< Allocator > a ) :
            product_id(a), 
            name(a), 
            url(a), 
            json(a), 
            product_variants( a.get_segment_manager() ), 
            product_details( a ), 
            product_image( a ), 
            product_prices( a.get_segment_manager() ), 
            stock_available( a.get_segment_manager() ), 
            delivery_variants( a.get_segment_manager() ), 
            delivery_details( a ), 
            delivery_prices( a.get_segment_manager() )
            {
               c( *this );
            }

         id_type                               id;

         account_name_type                     account;                ///< The Seller of the product.

         shared_string                         product_id;             ///< The name of the product.

         shared_string                         name;                   ///< The name of the product. Unique for each account.

         shared_string                         url;                    ///< Reference URL of the product or seller.

         shared_string                         json;                   ///< JSON metadata attributes of the product.

         shared_vector< fixed_string_32 >      product_variants;       ///< The collection of product variants. Each map must have a key for each variant.

         shared_string                         product_details;        ///< The Description details of each variant of the product.

         shared_string                         product_image;         ///< IPFS references to images of each product variant.

         shared_vector< asset >                product_prices;         ///< The price for each variant of the product.

         flat_map< uint32_t, uint16_t >        wholesale_discount;     ///< Discount percentages that are applied when quantity is above a given size.
         
         shared_vector< uint32_t >             stock_available;        ///< The available stock of each variant of the product.

         shared_vector< fixed_string_32 >      delivery_variants;      ///< The types of product delivery available to purchasers.

         shared_string                         delivery_details;       ///< The Description details of each variant of the delivery.

         shared_vector< asset >                delivery_prices;        ///< The price for each variant of delivery.

         time_point                            created;                ///< Time that the order was created.

         time_point                            last_updated;           ///< Time that the order was last updated, approved, or disputed.

         bool                                  active;                 ///< True when the product is active and able to be sold, false when discontinued.
   };


   /**
    * Tracks a purchase order to buy a product from a seller.
    * 
    * Lists an encrypted shipping address, 
    * and lists a product variant being be purchased.
    * Tracks the order value of the purchase, 
    * and marks as completed when delivered.
    */
   class product_purchase_object : public object< product_purchase_object_type, product_purchase_object >
   {
      product_purchase_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         product_purchase_object( Constructor&& c, allocator< Allocator > a ) :
            order_id(a), 
            product_id(a), 
            order_variants( a.get_segment_manager() ), 
            order_size( a.get_segment_manager() ), 
            memo(a), 
            json(a), 
            shipping_address(a),
            delivery_details(a)
            {
               c( *this );
            }

         id_type                           id;

         account_name_type                 buyer;                  ///< The Buyer of the product.

         shared_string                     order_id;               ///< uuidv4 referring to the purchase order.

         account_name_type                 seller;                 ///< The Seller of the product.

         shared_string                     product_id;             ///< uuidv4 referring to the product.

         shared_vector< fixed_string_32 >  order_variants;         ///< Variants of product ordered in the purchase.

         shared_vector< uint32_t >         order_size;             ///< The number of each product variant ordered.

         shared_string                     memo;                   ///< The memo for the transaction, encryption on the memo is advised.

         shared_string                     json;                   ///< Additional JSON object attribute details.

         public_key_type                   purchase_public_key;    ///< the Public key used to encrypt the memo and shipping address. 

         shared_string                     shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.

         fixed_string_32                   delivery_variant;       ///< The type of product delivery selected.

         shared_string                     delivery_details;       ///< The Description details of the delivery.

         asset                             order_value;            ///< The total value of the order.

         time_point                        created;                ///< Time that the order was created.

         time_point                        last_updated;           ///< Time that the order was last updated, approved, or disputed.
   };


   /**
    * Enables a merchant to list a product for auction.
    * 
    * Users can search for products and make auction bids for 
    * products from merchants.
    * 
    * Manages the type of auction, and creates an escrow transfer 
    * with the winning bidder on completion.
    * Product objects can be sold according to a variety of sale types,
    * which offer variable price mechanisms.
    */
   class product_auction_sale_object : public object< product_auction_sale_object_type, product_auction_sale_object >
   {
      product_auction_sale_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         product_auction_sale_object( Constructor&& c, allocator< Allocator > a ) :
            auction_id(a), 
            name(a), 
            url(a), 
            json(a), 
            product_details(a), 
            product_image(a), 
            delivery_variants( a.get_segment_manager() ), 
            delivery_details(a), 
            delivery_prices( a.get_segment_manager() ),
            winning_bid_id(a)
            {
               c( *this );
            }

         id_type                               id;

         account_name_type                     account;                ///< The Seller of the product.

         shared_string                         auction_id;             ///< The uuidv4 identifying the auction.

         product_auction_type                  auction_type;           ///< The type of sale to be used for the product.

         shared_string                         name;                   ///< The name of the product. Unique for each account.

         shared_string                         url;                    ///< Reference URL of the product or seller.

         shared_string                         json;                   ///< JSON metadata attributes of the product.

         shared_string                         product_details;        ///< The Description details of each variant of the product.

         shared_string                         product_image;          ///< IPFS references to image of each product variant.

         asset                                 reserve_bid;            ///< The min auction bid, or minimum price of a reverse auction at final bid time.

         asset                                 maximum_bid;            ///< The max auction bid. Auction will immediately conclude if this price is bidded. Starting price of reverse auction.

         shared_vector< fixed_string_32 >      delivery_variants;      ///< The types of product delivery available to purchasers.

         shared_string                         delivery_details;       ///< The Description details of each variant of the delivery.

         shared_vector< asset >                delivery_prices;        ///< The price for each variant of delivery.

         time_point                            final_bid_time;         ///< No more bids will be accepted after this time. Concealed bids must be revealed before completion time.

         time_point                            completion_time;        ///< Time that the auction will select the winning bidder, or end if no bids.

         uint32_t                              bid_count = 0;          ///< Number of bids placed on the auction.

         account_name_type                     winning_bidder;         ///< Account name of the winning bidder.

         shared_string                         winning_bid_id;         ///< The uuidv4 identifying the auction.

         bool                                  completed = false;      ///< True when the auction is completed.

         time_point                            created;                ///< Time that the order was created.

         time_point                            last_updated;           ///< Time that the order was last updated.

         share_type                            current_reverse_price( time_point now )const
         {
            if( now >= final_bid_time )
            {
               return reserve_bid.amount;
            }
            else if( now <= created )
            {
               return maximum_bid.amount;
            }
            else
            {
               fc::microseconds duration = final_bid_time - created;
               fc::microseconds elapsed = now - created;
               share_type min_price = reserve_bid.amount;
               share_type max_price = maximum_bid.amount;

               return min_price + ( ( ( max_price - min_price ) * ( duration - elapsed ).to_seconds() ) / duration.to_seconds() );
            }
         }

         asset_symbol_type                   bid_asset()const 
         {
            return reserve_bid.symbol;
         }
   };


   /**
    * Tracks a purchase order to buy a product from a seller.
    * 
    * Lists an encrypted shipping address, 
    * and lists a product variant being be purchased.
    * Tracks the order value of the purchase, 
    * and marks as completed when delivered.
    */
   class product_auction_bid_object : public object< product_auction_bid_object_type, product_auction_bid_object >
   {
      product_auction_bid_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         product_auction_bid_object( Constructor&& c, allocator< Allocator > a ) :
            bid_id(a), 
            auction_id(a), 
            memo(a), 
            json(a), 
            shipping_address(a),
            delivery_details(a)
            {
               c( *this );
            }

         id_type                           id;

         account_name_type                 buyer;                  ///< The Buyer of the product.

         shared_string                     bid_id;                 ///< uuidv4 referring to the auction bid.

         account_name_type                 seller;                 ///< The Seller of the product.

         shared_string                     auction_id;             ///< The uuidv4 identifying the auction.

         asset_symbol_type                 bid_asset;              ///< The Symbol of the asset being bidded.

         commitment_type                   bid_price_commitment;   ///< Concealed value of the bid price amount.

         blind_factor_type                 blinding_factor;        ///< Factor to blind the bid price.

         share_type                        public_bid_amount;      ///< Set to 0 initially for concealed bid, revealed to match commitment. Revealed in initial bid if open.

         shared_string                     memo;                   ///< The memo for the transaction, encryption on the memo is advised.

         shared_string                     json;                   ///< Additional JSON object attribute details.

         public_key_type                   bid_public_key;         ///< the Public key used to encrypt the memo and shipping address. 

         shared_string                     shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.

         fixed_string_32                   delivery_variant;       ///< The type of product delivery selected.

         shared_string                     delivery_details;       ///< The Description details of the delivery.

         asset                             delivery_value;         ///< The cost of the delivery if the bid is successful.

         time_point                        created;                ///< Time that the order was created.

         time_point                        last_updated;           ///< Time that the order was last updated, approved, or disputed.

         time_point                        completion_time;        ///< Time that the auction will select the winning bidder, or end if no bids.

         bool                              winning_bid;            ///< True when the bid wins its auction, false otherwise.

         asset                             order_value()const      ///< The total value of the order.
         {
            return asset( public_bid_amount, bid_asset ) + delivery_value;
         }
   };


   /**
    * Enables a conditional transfer between two accounts, with mediation.
    * 
    * Seller account deposits funds when they approve the escrow
    * When either the from or to account account release the funds,
    * the funds are either transferred in full or refunded.
    * 
    * If there is an issue in the escrow transfer, the mediation
    * object is placed into dispute mode which allocates an additional 5
    * mediators which are able to vote on how to allocate the funds.
    */
   class escrow_object : public object< escrow_object_type, escrow_object >
   {
      escrow_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         escrow_object( Constructor&& c, allocator< Allocator > a ) :
         escrow_id(a), 
         memo(a), 
         json(a)
         {
            c( *this );
         }

         id_type                                   id;

         account_name_type                         from;                   ///< Account sending funds.

         account_name_type                         to;                     ///< Account receiving funds.

         account_name_type                         from_mediator;          ///< Representative of the sending account.

         account_name_type                         to_mediator;            ///< Representative of the receiving account.

         asset                                     payment;                ///< Total payment to be transferred.

         asset                                     balance;                ///< Current funds deposited in the escrow.

         shared_string                             escrow_id;              ///< uuidv4 referring to the escrow payment.

         shared_string                             memo;                   ///< Details of the transaction for reference.

         shared_string                             json;                   ///< Additional JSON object attribute details.

         time_point                                acceptance_time;        ///< time that the transfer must be approved by.

         time_point                                escrow_expiration;      ///< Time that the escrow is able to be claimed by either TO or FROM.

         time_point                                dispute_release_time;   ///< Time that the balance is distributed to median release percentage.

         flat_set< account_name_type >             mediators;              ///< Set of accounts able to mediate the dispute.

         flat_map< account_name_type, uint16_t >   release_percentages;    ///< Declared release percentages of all accounts.

         flat_map< account_name_type, bool >       approvals;              ///< Map of account approvals, paid into balance.

         time_point                                created;                ///< Time that the order was created.

         time_point                                last_updated;           ///< Time that the order was last updated, approved, or disputed.

         bool                                      disputed = false;       ///< True when escrow is in dispute.

         bool                                      from_approved()const    ///< True when the from account has approved.
         {
            return approvals.at( from );
         };

         bool                                      to_approved()const      ///< True when the to account has approved
         {
            return approvals.at( to );
         };

         bool                                      from_mediator_approved()const    ///< True when the mediator added by from account has approved
         {
            if( from_mediator != account_name_type() )
            {
               return approvals.at( from_mediator );
            }
            else
            {
               return false;
            }
         };

         bool                                      to_mediator_approved()const    ///< True when the mediator added by to account has approved
         {
            if( to_mediator != account_name_type() )
            {
               return approvals.at( to_mediator );
            }
            else
            {
               return false;
            }
         };

         bool                                      is_approved()const      ///< True when all participating accounts have approved
         { 
            return from_approved() && to_approved() && from_mediator_approved() && to_mediator_approved();
         };

         bool                                      is_mediator( const account_name_type& account )const    ///< True when an acocunt is a mediator
         {
            return std::find( mediators.begin(), mediators.end(), account ) != mediators.end();
         };
   };


   struct by_product_id;
   struct by_last_updated;

   typedef multi_index_container<
      product_sale_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< product_sale_object, product_sale_id_type, &product_sale_object::id > >,
         ordered_unique< tag< by_product_id >,
            composite_key< product_sale_object,
               member< product_sale_object, account_name_type, &product_sale_object::account >,
               member< product_sale_object, shared_string, &product_sale_object::product_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< product_sale_object,
               member< product_sale_object, time_point, &product_sale_object::last_updated >,
               member< product_sale_object, product_sale_id_type, &product_sale_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< product_sale_id_type >
            >
         >
      >,
      allocator< product_sale_object >
   > product_sale_index;


   struct by_order_id;
   struct by_buyer_product_id;


   typedef multi_index_container<
      product_purchase_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< product_purchase_object, product_purchase_id_type, &product_purchase_object::id > >,
         ordered_unique< tag< by_order_id >,
            composite_key< product_purchase_object,
               member< product_purchase_object, account_name_type, &product_purchase_object::buyer >,
               member< product_purchase_object, shared_string, &product_purchase_object::order_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_product_id >,
            composite_key< product_purchase_object,
               member< product_purchase_object, account_name_type, &product_purchase_object::seller >,
               member< product_purchase_object, shared_string, &product_purchase_object::product_id >,
               member< product_purchase_object, product_purchase_id_type, &product_purchase_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less,
               std::less< product_purchase_id_type >
            >
         >,
         ordered_unique< tag< by_buyer_product_id >,
            composite_key< product_purchase_object,
               member< product_purchase_object, account_name_type, &product_purchase_object::buyer >,
               member< product_purchase_object, account_name_type, &product_purchase_object::seller >,
               member< product_purchase_object, shared_string, &product_purchase_object::product_id >,
               member< product_purchase_object, product_purchase_id_type, &product_purchase_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >,
               strcmp_less,
               std::less< product_purchase_id_type >
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< product_purchase_object,
               member< product_purchase_object, time_point, &product_purchase_object::last_updated >,
               member< product_purchase_object, product_purchase_id_type, &product_purchase_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< product_purchase_id_type >
            >
         >
      >,
      allocator< product_purchase_object >
   > product_purchase_index;

   struct by_auction_id;
   struct by_last_updated;
   struct by_completion_time;

   typedef multi_index_container<
      product_auction_sale_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< product_auction_sale_object, product_auction_sale_id_type, &product_auction_sale_object::id > >,
         ordered_unique< tag< by_auction_id >,
            composite_key< product_auction_sale_object,
               member< product_auction_sale_object, account_name_type,  &product_auction_sale_object::account >,
               member< product_auction_sale_object, shared_string, &product_auction_sale_object::auction_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_completion_time >,
            composite_key< product_auction_sale_object,
               member< product_auction_sale_object, bool, &product_auction_sale_object::completed >,
               member< product_auction_sale_object, time_point, &product_auction_sale_object::completion_time >,
               member< product_auction_sale_object, product_auction_sale_id_type, &product_auction_sale_object::id >
            >,
            composite_key_compare<
               std::less< bool >,
               std::less< time_point >,
               std::less< product_auction_sale_id_type >
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< product_auction_sale_object,
               member< product_auction_sale_object, time_point, &product_auction_sale_object::last_updated >,
               member< product_auction_sale_object, product_auction_sale_id_type, &product_auction_sale_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >,
               std::less< product_auction_sale_id_type >
            >
         >
      >,
      allocator< product_auction_sale_object >
   > product_auction_sale_index;


   struct by_bid_id;
   struct by_buyer_auction_id;
   struct by_highest_bid; 


   typedef multi_index_container<
      product_auction_bid_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< product_auction_bid_object, product_auction_bid_id_type, &product_auction_bid_object::id > >,
         ordered_unique< tag< by_bid_id >,
            composite_key< product_auction_bid_object,
               member< product_auction_bid_object, account_name_type, &product_auction_bid_object::buyer >,
               member< product_auction_bid_object, shared_string, &product_auction_bid_object::bid_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_auction_id >,
            composite_key< product_auction_bid_object,
               member< product_auction_bid_object, account_name_type, &product_auction_bid_object::seller >,
               member< product_auction_bid_object, shared_string, &product_auction_bid_object::auction_id >,
               member< product_auction_bid_object, product_auction_bid_id_type, &product_auction_bid_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less,
               std::less< product_auction_bid_id_type >
            >
         >,
         ordered_unique< tag< by_highest_bid >,
            composite_key< product_auction_bid_object,
               member< product_auction_bid_object, account_name_type, &product_auction_bid_object::seller >,
               member< product_auction_bid_object, shared_string, &product_auction_bid_object::auction_id >,
               member< product_auction_bid_object, share_type, &product_auction_bid_object::public_bid_amount >,
               member< product_auction_bid_object, product_auction_bid_id_type, &product_auction_bid_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less,
               std::greater< share_type >,
               std::less< product_auction_bid_id_type >
            >
         >,
         ordered_unique< tag< by_buyer_auction_id >,
            composite_key< product_auction_bid_object,
               member< product_auction_bid_object, account_name_type, &product_auction_bid_object::buyer >,
               member< product_auction_bid_object, account_name_type, &product_auction_bid_object::seller >,
               member< product_auction_bid_object, shared_string, &product_auction_bid_object::auction_id >,
               member< product_auction_bid_object, product_auction_bid_id_type, &product_auction_bid_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >,
               strcmp_less,
               std::less< product_auction_bid_id_type >
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< product_auction_bid_object,
               member< product_auction_bid_object, time_point, &product_auction_bid_object::last_updated >,
               member< product_auction_bid_object, product_auction_bid_id_type, &product_auction_bid_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< product_auction_bid_id_type >
            >
         >
      >,
      allocator< product_auction_bid_object >
   > product_auction_bid_index;

   struct by_from_id;
   struct by_to;
   struct by_acceptance_time;
   struct by_dispute_release_time;
   struct by_balance;

   typedef multi_index_container<
      escrow_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< escrow_object, escrow_id_type, &escrow_object::id > >,
         ordered_unique< tag< by_from_id >,
            composite_key< escrow_object,
               member< escrow_object, account_name_type,  &escrow_object::from >,
               member< escrow_object, shared_string, &escrow_object::escrow_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >,
         ordered_unique< tag< by_to >,
            composite_key< escrow_object,
               member< escrow_object, account_name_type, &escrow_object::to >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< escrow_id_type >
            >
         >,
         ordered_unique< tag< by_acceptance_time >,
            composite_key< escrow_object,
               const_mem_fun< escrow_object, bool, &escrow_object::is_approved >,
               member< escrow_object, time_point, &escrow_object::acceptance_time >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >,
            composite_key_compare< 
               std::less< bool >,
               std::less< time_point >,
               std::less< escrow_id_type >
            >
         >,
         ordered_unique< tag< by_dispute_release_time >,
            composite_key< escrow_object,
               member< escrow_object, bool, &escrow_object::disputed >,
               member< escrow_object, time_point, &escrow_object::dispute_release_time >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >,
            composite_key_compare< 
               std::less< bool >,
               std::less< time_point >,
               std::less< escrow_id_type >
            >
         >,
         ordered_unique< tag< by_balance >,
            composite_key< escrow_object,
               member< escrow_object, asset, &escrow_object::balance >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >,
            composite_key_compare<
               std::greater< asset >,
               std::less< escrow_id_type >
            >
         >
      >,
      allocator< escrow_object >
   > escrow_index;

} } // node::chain

FC_REFLECT( node::chain::product_sale_object,
         (id)
         (account)
         (product_id)
         (name)
         (url)
         (json)
         (product_variants)
         (product_details)
         (product_image)
         (product_prices)
         (wholesale_discount)
         (stock_available)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (created)
         (last_updated)
         (active)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::product_sale_object, node::chain::product_sale_index );

FC_REFLECT( node::chain::product_purchase_object,
         (id)
         (buyer)
         (order_id)
         (seller)
         (product_id)
         (order_variants)
         (order_size)
         (memo)
         (json)
         (purchase_public_key)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         (order_value)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::product_purchase_object, node::chain::product_purchase_index );

FC_REFLECT( node::chain::product_auction_sale_object,
         (id)
         (account)
         (auction_id)
         (auction_type)
         (name)
         (url)
         (json)
         (product_details)
         (product_image)
         (reserve_bid)
         (maximum_bid)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (final_bid_time)
         (completion_time)
         (bid_count)
         (winning_bidder)
         (winning_bid_id)
         (completed)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::product_auction_sale_object, node::chain::product_auction_sale_index );

FC_REFLECT( node::chain::product_auction_bid_object,
         (id)
         (buyer)
         (bid_id)
         (seller)
         (auction_id)
         (bid_asset)
         (bid_price_commitment)
         (blinding_factor)
         (public_bid_amount)
         (memo)
         (json)
         (bid_public_key)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         (delivery_value)
         (created)
         (last_updated)
         (completion_time)
         (winning_bid)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::product_auction_bid_object, node::chain::product_auction_bid_index );

FC_REFLECT( node::chain::escrow_object,
         (id)
         (from)
         (to)
         (from_mediator)
         (to_mediator)
         (payment)
         (balance)
         (escrow_id)
         (memo)
         (json)
         (acceptance_time)
         (escrow_expiration)
         (dispute_release_time)
         (mediators)
         (release_percentages)
         (approvals)
         (created)
         (last_updated)
         (disputed)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::escrow_object, node::chain::escrow_index );