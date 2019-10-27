/*
 * Copyright (c) 2017 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <fc/fixed_string.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/authority.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/witness_objects.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace node { namespace chain {
   /**
    * Assets have a globally unique symbol name that controls how they are traded and an issuer who
    * has authority over the parameters of the asset.
    * Multiple types of asset are available, each with different properties and capabilities.
    */
   class asset_object : public object< asset_object_type, asset_object >
   {
      asset_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                               id; 

         asset_symbol_type                     symbol;                 // Consensus enforced unique Ticker symbol string for this asset. 

         asset_types                           asset_type;             // The type of the asset.
         
         account_name_type                     issuer;                 // name of the account which issued this asset.

         asset_options                         options;                // Object containing the option settings of the asset

         time_point                            created;                // Time that the asset was created. 

         static bool is_valid_symbol( const string& symbol );          // True if symbol is a valid ticker symbol; false otherwise.

         bool is_market_issued()const   // True if this is a market-issued asset; false otherwise. Market issued assets cannot be issued by the asset issuer. 
         { 
            switch(asset_type)
            {
               case BITASSET_ASSET:
               case PREDICTION_ASSET:
               case LIQUIDITY_POOL_ASSET:
               case CREDIT_POOL_ASSET:
               case OPTION_ASSET:
               {
                  return true;
               }
               default:
               {
                  return false;
               }
            }
         }

         /// @return true if users may request force-settlement of this market-issued asset; false otherwise
         bool can_force_settle()const { return !(options.flags & disable_force_settle); }

         /// @return true if the issuer of this market-issued asset may globally settle the asset; false otherwise
         bool can_global_settle()const { return options.issuer_permissions & global_settle; }

         /// @return true if this asset charges a fee for the issuer on market operations; false otherwise
         bool charges_market_fees()const { return options.flags & charge_market_fee; }

         /// @return true if this asset may only be transferred to/from the issuer or market orders
         bool is_transfer_restricted()const { return options.flags & transfer_restricted; }

         bool can_override()const { return options.flags & override_authority; }

         bool allow_confidential()const { return !(options.flags & asset_issuer_permission_flags::disable_confidential); }

         void validate()const
         {
            // UIAs may not have force settlement, or global settlements
            if( !is_market_issued() )
            {
               FC_ASSERT(!(options.flags & disable_force_settle || options.flags & global_settle));
               FC_ASSERT(!(options.issuer_permissions & disable_force_settle || options.issuer_permissions & global_settle));
            }
         }
   };

    /**
    *  Tracks the asset information that changes frequently. 
    */
   class asset_dynamic_data_object : public object< asset_dynamic_data_object_type, asset_dynamic_data_object>
   {
      asset_dynamic_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_dynamic_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                        id; 

         asset_symbol_type              symbol;                        // Ticker symbol string for this asset.

         account_name_type              issuer;                        // The asset issuing account. 

         share_type                     total_supply = 0;              // The total outstanding supply of the asset

         share_type                     liquid_supply = 0;             // The current liquid supply of the asset

         share_type                     staked_supply = 0;             // The current staked supply of the asset

         share_type                     reward_supply = 0;             // The current reward supply of the asset

         share_type                     savings_supply = 0;            // The current savings supply of the asset

         share_type                     delegated_supply = 0;          // The current delegated supply of the asset
         
         share_type                     receiving_supply = 0;          // The current receiving supply supply of the asset, should equal delegated

         share_type                     pending_supply = 0;            // The current supply contained in reward funds and active order objects

         share_type                     confidential_supply = 0;       // total confidential asset supply

         share_type                     accumulated_fees = 0;          // Amount of Fees that have accumulated to be paid to the asset issuer. Denominated in this asset.

         share_type                     fee_pool = 0;                  // Amount of core asset available to pay fees. Denominated in the core asset.

         asset get_liquid_supply()const { return asset(liquid_supply, symbol); }

         asset get_reward_supply()const { return asset(reward_supply, symbol); }

         asset get_staked_supply()const { return asset(staked_supply, symbol); }

         asset get_savings_supply()const { return asset(savings_supply, symbol); }

         asset get_delegated_supply()const { return asset(delegated_supply, symbol); }

         asset get_receiving_supply()const { return asset(receiving_supply, symbol); }

         asset get_pending_supply()const { return asset(pending_supply, symbol); }

         asset get_total_supply()const { return asset(total_supply, symbol); }

         void asset_dynamic_data_object::adjust_liquid_supply(const asset& delta)
         {
            assert(delta.symbol == symbol);
            liquid_supply += delta.amount;
            total_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_staked_supply(const asset& delta)
         {
            assert(delta.symbol == symbol);
            staked_supply += delta.amount;
            total_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_reward_supply(const asset& delta)
         {
            assert(delta.symbol == symbol);
            reward_supply += delta.amount;
            total_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_savings_supply(const asset& delta)
         {
            assert(delta.symbol == symbol);
            savings_supply += delta.amount;
            total_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_delegated_supply(const asset& delta)    // Not included in total supply
         {
            assert(delta.symbol == symbol);
            delegated_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_receiving_supply(const asset& delta)    // Not included in total supply
         {
            assert(delta.symbol == symbol);
            receiving_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_pending_supply(const asset& delta)
         {
            assert(delta.symbol == symbol);
            pending_supply += delta.amount;
            total_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_fee_pool(const asset& delta)   // Must be core asset, not included in supply of this asset. Pending balance of core asset.
         {
            assert(delta.symbol == SYMBOL_COIN );
            fee_pool += delta.amount;
         }

         void asset_dynamic_data_object::adjust_pending_supply(const asset& delta)
         {
            assert(delta.symbol == symbol);
            pending_supply += delta.amount;
            total_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_accumulated_fees(const asset& delta)
         {
            assert(delta.symbol == symbol);
            accumulated_fees += delta.amount;
            total_supply += delta.amount;
         }
   };

   
   class asset_bitasset_data_object : public object < asset_bitasset_data_object_type, asset_bitasset_data_object>
   {
      asset_bitasset_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_bitasset_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                       id;

         asset_symbol_type                             symbol;                                  // The symbol of the bitasset that this object belongs to

         account_name_type                             issuer;                                  // The account name of the issuer 

         asset_symbol_type                             backing_asset = SYMBOL_COIN;             // The collateral backing asset of the bitasset

         optional<bitasset_options>                    options;                                 // The tunable options for BitAssets are stored in this field.

         flat_map<account_name_type, pair<time_point,price_feed>>  feeds;                       // Feeds published for this asset. 

         price_feed                                    current_feed;                            // Currently active price feed, median of values from the currently active feeds.

         time_point                                    current_feed_publication_time;           // Publication time of the oldest feed which was factored into current_feed.

         price                                         current_maintenance_collateralization;   // Call orders with collateralization (aka collateral/debt) not greater than this value are in margin call territory.

         share_type                                    force_settled_volume;                    // This is the volume of this asset which has been force-settled this maintanence interval
         
         share_type                                    max_force_settlement_volume(share_type current_supply)const;    // Calculate the maximum force settlement volume per maintenance interval, given the current share supply

         bool                                          has_settlement()const { return !settlement_price.is_null(); }   // True if there has been a black swan

         /**
          *  In the event of a black swan, the swan price is saved in the settlement price, and all margin positions
          *  are settled at the same price with the seized collateral being moved into the settlement fund. From this
          *  point on no further updates to the asset are permitted (no feeds, etc) and forced settlement occurs
          *  immediately when requested, using the settlement price and fund.
          */
         
         price                                         settlement_price;      // Price at which force settlements of a black swanned asset will occur
         
         share_type                                    settlement_fund;       // Amount of collateral which is available for force settlement

         bool                                          asset_cer_updated = false;       // Track whether core_exchange_rate in corresponding asset_object has updated

         bool                                          feed_cer_updated = false;// Track whether core exchange rate in current feed has updated

         bool need_to_update_cer() const
         {
            return ( ( feed_cer_updated || asset_cer_updated ) && !current_feed.core_exchange_rate.is_null() );
         }       // Whether need to update core_exchange_rate in asset_object

         /// The time when @ref current_feed would expire
         time_point feed_expiration_time()const
         {
            
            return current_feed_publication_time + *options.feed_lifetime;
         }

         bool feed_is_expired(time_point current_time)const
         { return feed_expiration_time() <= current_time; }

         /******
          * @brief calculate the median feed
          *
          * This calculates the median feed from @ref feeds, feed_lifetime
          * in @ref options, and the given parameters.
          * It may update the current_feed_publication_time, current_feed and
          * current_maintenance_collateralization member variables.
          *
          * @param current_time the current time to use in the calculations
          * @param next_maintenance_time the next chain maintenance time
          */
         void update_median_feeds(time_point current_time, time_point next_maintenance_time);
   };


   class asset_equity_data_object : public object < asset_equity_data_object_type, asset_equity_data_object>
   {
      asset_equity_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_equity_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          business_account;                     // The business account name of the issuer 

         asset_symbol_type          symbol;                               // The symbol of the equity asset of the business

         equity_options             options;                              // Tuneable options for equity assets

         asset_symbol_type          dividend_asset = SYMBOL_USD;          // The asset used to distribute dividends to asset holders

         asset                      dividend_pool = asset(0,SYMBOL_USD);  // Amount of assets pooled for distribution at the next interval
         
         time_point                 last_dividend;                        // Time that the asset last distributed a dividend.

         void adjust_pool(const asset& delta)
         {
            assert(delta.symbol == dividend_asset);
            dividend_pool += delta;
         }
   };



   /**
    * Credit assets enable business accounts to lend money by issuing
    * credit assets, backed by repurchases up to a given buyback price, or face value
    * Accounts place repurchase orders from thier buyback pool at the buyback price
    * and maintain the asset's face value.
    * Credit assets additionally pay interest to holders by issuing new credit asset supply
    * at an interest rate that linearly varies with the difference between the 
    * buyback price and the market price in the liquidity pool,
    * with a fixed component and a variable component.
    * 
    * R = Interest Rate
    * B = Buyback Price
    * M = Market price
    * Ra = Range
    * F = Fixed Component
    * V = Variable Component
    * 
    * R = V * ( -0.5 * min( 1, max(-1, (M-B)/(B*Ra) ) ) + 0.5 ) + F
    */
   class asset_credit_data_object : public object < asset_credit_data_object_type, asset_credit_data_object>
   {
      asset_credit_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_credit_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          business_account;                     // The business account name of the issuer 

         asset_symbol_type          symbol;                               // The symbol of the credit asset of the business

         credit_options             options;                              // Tuneable options for credit assets

         asset_symbol_type          buyback_asset = SYMBOL_USD;           // Symbol used to buyback credit assets

         asset                      buyback_pool = asset(0,SYMBOL_USD);   // Amount of assets pooled to buyback the asset at next interval

         price                      buyback_price = price(asset(1,SYMBOL_USD),asset(1,symbol));  // Price at which the credit asset is bought back

         asset_symbol_type          symbol_a;                             // the asset with the lower id in the buyback price pair

         asset_symbol_type          symbol_b;                             // the asset with the greater id in the buybackprice pair
         
         time_point                 last_buyback;                         // Time that the asset was last updated

         void adjust_pool(const asset& delta)
         {
            assert(delta.symbol == buyback_asset);
            buyback_pool += delta;
         }
   };

   /**
    * Asset liquidity pool contains an open balance of two assets, which any account may add liquidity too
    * Liquidity arrays utilize an extended series of Bancor Inspired formulae to provide
    * continuous, asynchronous liquidity to every asset in the network.
    * Any asset may be traded directly for any other asset in the network, 
    * by using the set of core asset liquidity arrays between them to intermediate the exchange.
    * Liquidity arrays charge a pool fee for all exchanges that they process.
    * Limit orders may match against the liquidity pool using a liquid limit exchange, creating a unified
    * orderbook between the outstanding limit orders and call orders, plus the current price of the
    * liquidity pool exchange.
    * 
    * Formulae:
    * 
    * R = Returned Asset
    * I = Input Asset
    * Pa = Price (Average)
    * Pf = Price (Final)
    * Br = Balance (Returned Asset)
    * Bi = Balance (Input Asset)
    * Bs = Balance (Supply Liquidity Pool Asset)
    * Sr = Supply Received
    * Si = Supply Input
    * 
    * 
    * R      = Br * ( 1 - ( Bi / ( I + Bi ) ) )    Amount Recieved for a given input
    * I      = Bi * ( 1 - ( R / Br ) ) - Bi        Input Required for a given return amount   
    * Pa     = ( Br * I ) / ( I * Bi + I^2 )       Average price of an exchange with a given input
    * Pf     = ( Br * Bi ) / ( Bi + I )^2          Final price after an exchange with a given input
    * I(max) = sqrt( ( Br * Bi ) / Pf ) - Bi       Maximum input to reach a given final price.
    * Sr     = Bs * (sqrt( 1 + ( I / Bi ) ) - 1 )  Supply asset obtained for funding an input asset.
    * R      = Br * ( 1 - ( 1 - ( Si / Bs ) )^2 )  Amount Received for withdrawing an input Supply asset.
    * 
    */
   class asset_liquidity_pool_object : public object< asset_liquidity_pool_object_type, asset_liquidity_pool_object >
   {
      asset_liquidity_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_liquidity_pool_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id; 

         account_name_type          issuer;                        // Name of the account which created the liquidity pool.

         asset_symbol_type          symbol_a;                      // Ticker symbol string of the asset with the lower ID. Must be core asset if one asset is core.

         asset_symbol_type          symbol_b;                      // Ticker symbol string of the asset with the higher ID.

         asset_symbol_type          symbol_liquid;                 // Ticker symbol of the pool's liquidity pool asset. 

         asset                      balance_a;                     // Balance of Asset A. Must be core asset if one asset is core.

         asset                      balance_b;                     // Balance of Asset B.

         asset                      balance_liquid;                // Outstanding supply of the liquidity asset for the asset pair.

         price                      hour_median_price;             // The median price over the past hour, at 10 minute intervals. Used for collateral calculations. 

         price                      day_median_price;              // The median price over the last day, at 10 minute intervals.

         bip::deque< price, allocator< price > >  price_history;   // Tracks the last 24 hours of median price, one per 10 minutes.

         price                      current_price()const           // Liquidity pools current price of Asset A and Asset B. Asset A is the base asset, Asset B is the quote asset.
         {
            return price(balance_a, balance_b);
         }

         price                       base_price( const asset_symbol_type& base )const   // Current price with specified asset as base
         {
            FC_ASSERT( base == symbol_a || base == symbol_b,
               "Invalid base asset price requested." );
            if( base == symbol_a )
            {
               return price( balance_a, balance_b);
            }
            else if( base == symbol_b )
            {
               return price( balance_b, balance_a );
            }
         }

         price                       base_hour_median_price( const asset_symbol_type& base )const   // hourly median price with specified asset as base
         {
            FC_ASSERT( base == symbol_a || base == symbol_b,
               "Invalid base asset price requested." );
            if( base == symbol_a )
            {
               return hour_median_price;
            }
            else if( base == symbol_b )
            {
               return ~hour_median_price;
            }
         }

         price                       base_day_median_price( const asset_symbol_type& base )const   // daily median price with specified asset as base
         {
            FC_ASSERT( base == symbol_a || base == symbol_b,
               "Invalid base asset price requested." );
            if( base == symbol_a )
            {
               return day_median_price;
            }
            else if( base == symbol_b )
            {
               return ~day_median_price;
            }
         }

         asset                       asset_balance( const asset_symbol_type& symbol )const
         {
            FC_ASSERT( symbol == symbol_a || symbol == symbol_b,
               "Invalid asset balance requested." );
            if( symbol == symbol_a )
            {
               return balance_a;
            }
            else if( symbol == symbol_b )
            {
               return balance_b;
            }
         }
   };


   class asset_credit_pool_object : public object< asset_credit_pool_object_type, asset_credit_pool_object >
   {
      asset_credit_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_credit_pool_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id; 

         account_name_type          issuer;                 // Name of the account which created the credit pool.

         asset_symbol_type          base_symbol;            // Ticker symbol string of the base asset being lent and borrowed.

         asset_symbol_type          credit_symbol;          // Ticker symbol string of the credit asset for use as collateral to borrow the base asset.

         asset                      base_balance;           // Balance of the base asset that is available for loans and redemptions. 

         asset                      borrowed_balance;       // Total amount of base asset currently lent to borrowers, accumulates compounding interest payments. 

         asset                      credit_balance;         // Balance of the credit asset redeemable for an increasing amount of base asset.

         share_type                 last_interest_rate;     // The most recently calculated interest rate when last compounded. 

         price                      last_price;             // The last price that assets were lent or withdrawn at. 

         price                      current_price()const    // Credit pool's current price of the credit asset, denominated in the base asset. Increases over time with incoming interest.
         {
            return price( base_balance + borrowed_balance, credit_balance);
         }      

         share_type                 interest_rate(uint16_t min, uint16_t var)const  // Current annualized interest rate of borrowing and lending. Updates whenever a loan is taken or repaid.  
         {
            return share_type(std::min(uint64_t(min + var * ( (borrowed_balance.amount.value + BLOCKCHAIN_PRECISION) / (base_balance.amount.value + BLOCKCHAIN_PRECISION) )), uint64_t( 50 * PERCENT_1 )));
         }
   };
   
   struct by_symbol;
   struct by_type;
   struct by_issuer;
   struct by_interest;

   typedef multi_index_container<
      asset_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member< asset_object, asset_id_type, &asset_object::id > >,
         ordered_unique< tag<by_symbol>,
            member<asset_object, asset_symbol_type, &asset_object::symbol> >,
         ordered_unique< tag<by_type>,
            composite_key< asset_object,
                const_mem_fun<asset_object, bool, &asset_object::is_market_issued>,
                member< asset_object, asset_id_type, &asset_object::id >
            >
         >,
         ordered_unique< tag<by_issuer>,
            composite_key< asset_object,
                member< asset_object, account_name_type, &asset_object::issuer >,
                member< asset_object, asset_id_type, &asset_object::id >
            >
         >
      >, 
      allocator< asset_object >
   > asset_index;

   typedef multi_index_container<
      asset_dynamic_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_dynamic_data_object, asset_dynamic_data_id_type, &asset_dynamic_data_object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_dynamic_data_object, asset_symbol_type, &asset_dynamic_data_object::symbol> >
      >, 
      allocator< asset_dynamic_data_object >
   > asset_dynamic_data_index;

   struct by_short_backing_asset;
   struct by_feed_expiration;
   struct by_cer_update;

   typedef multi_index_container<
      asset_bitasset_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_bitasset_data_object, asset_bitasset_data_id_type, &asset_bitasset_data_object::id > >,
         ordered_non_unique< tag<by_short_backing_asset>, 
            member< asset_bitasset_data_object, asset_symbol_type, &asset_bitasset_data_object::backing_asset > 
         >,
         ordered_unique< tag<by_feed_expiration>,
            composite_key< asset_bitasset_data_object,
               const_mem_fun< asset_bitasset_data_object, time_point, &asset_bitasset_data_object::feed_expiration_time >,
               member< asset_bitasset_data_object, asset_bitasset_data_id_type, &asset_bitasset_data_object::id >
            >
         >,
         ordered_non_unique< tag<by_cer_update>,
               const_mem_fun< asset_bitasset_data_object, bool, &asset_bitasset_data_object::need_to_update_cer >
         >
      >,
      allocator< asset_bitasset_data_object >
   > asset_bitasset_data_index;

   struct by_dividend_asset;

   typedef multi_index_container<
      asset_equity_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_equity_data_object, asset_equity_data_id_type, &asset_equity_data_object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_equity_data_object, asset_symbol_type, &asset_equity_data_object::symbol> >,
         ordered_unique< tag<by_dividend_asset>, 
            composite_key< asset_equity_data_object,
               member< asset_equity_data_object, asset_symbol_type, &asset_equity_data_object::dividend_asset >,
               member< asset_equity_data_object, asset_equity_data_id_type, &asset_equity_data_object::id >
            >
         >  
      >,
      allocator< asset_equity_data_object >
   > asset_equity_data_index;

   struct by_buyback_asset;

   typedef multi_index_container<
      asset_credit_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_credit_data_object, asset_credit_data_id_type, &asset_credit_data_object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_credit_data_object, asset_symbol_type, &asset_credit_data_object::symbol> >,
         ordered_unique< tag<by_buyback_asset>, 
            composite_key< asset_credit_data_object,
               member< asset_credit_data_object, asset_symbol_type, &asset_credit_data_object::buyback_asset >,
               member< asset_credit_data_object, asset_credit_data_id_type, &asset_credit_data_object::id >
            >
         >  
      >,
      allocator< asset_credit_data_object >
   > asset_credit_data_index;

   struct by_asset_pair;

   typedef multi_index_container<
      asset_liquidity_pool_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > >,
         ordered_unique< tag<by_asset_pair>,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_a >,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_b > 
            >
         >
      >, 
      allocator< asset_liquidity_pool_object >
   > asset_liquidity_pool_index;

   struct by_base_symbol;
   struct by_credit_symbol;

   typedef multi_index_container<
      asset_credit_pool_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_credit_pool_object, asset_credit_pool_id_type, &asset_credit_pool_object::id > >,
         ordered_unique< tag<by_base_symbol>, member< asset_credit_pool_object, asset_symbol_type, &asset_credit_pool_object::base_symbol > >,
         ordered_unique< tag<by_credit_symbol>, member< asset_credit_pool_object, asset_symbol_type, &asset_credit_pool_object::credit_symbol > >
      >, 
      allocator< asset_credit_pool_object >
   > asset_credit_pool_index;

} } // node::chain

FC_REFLECT( node::chain::asset_object,
                  (id)
                  (symbol)
                  (issuer)
                  (options)
                  (created)
                  );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_object, node::chain::asset_index );

FC_REFLECT( node::chain::asset_bitasset_data_object,
                  (id)
                  (symbol)
                  (feeds)
                  (collateral_asset)
                  (current_feed)
                  (current_feed_publication_time)
                  (current_maintenance_collateralization)
                  (options)
                  (force_settled_volume)
                  (settlement_price)
                  (settlement_fund)
                  (asset_cer_updated)
                  (feed_cer_updated)
                  );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_bitasset_data_object, node::chain::asset_bitasset_data_index );

FC_REFLECT( node::chain::asset_dynamic_data_object,
                  (id)
                  (symbol)
                  (total_supply)
                  (liquid_supply)
                  (staked_supply)
                  (reward_supply)
                  (savings_supply)
                  (delegated_supply)
                  (received_supply)
                  (pending_supply)
                  (confidential_supply)
                  (accumulated_fees)
                  (fee_pool) 
                  );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_dynamic_data_object, node::chain::asset_dynamic_data_index );

FC_REFLECT( node::chain::asset_liquidity_pool_object,
                  (id)
                  (symbol)
                  (total_supply)
                  (liquid_supply)
                  (staked_supply)
                  (reward_supply)
                  (savings_supply)
                  (delegated_supply)
                  (received_supply)
                  (pending_supply)
                  (confidential_supply)
                  (accumulated_fees)
                  (fee_pool) 
                  );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_liquidity_pool_object, node::chain::asset_liquidity_pool_index );

FC_REFLECT( node::chain::asset_credit_pool_object,
                  (id)
                  (symbol)
                  (total_supply)
                  (liquid_supply)
                  (staked_supply)
                  (reward_supply)
                  (savings_supply)
                  (delegated_supply)
                  (received_supply)
                  (pending_supply)
                  (confidential_supply)
                  (accumulated_fees)
                  (fee_pool) 
                  );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_credit_pool_object, node::chain::asset_credit_pool_index );