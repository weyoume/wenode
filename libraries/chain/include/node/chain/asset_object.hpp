#pragma once
#include <fc/fixed_string.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/authority.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/producer_objects.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace node { namespace chain {

   /**
    * ASSET TYPES
    * 
    * CURRENCY_ASSET,         // Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
    * STANDARD_ASSET,         // Regular asset, can be transferred and staked, saved, and delegated.
    * EQUITY_ASSET,           // Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions. 
    * CREDIT_ASSET,           // Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
    * BITASSET_ASSET,         // Asset based by collateral and track the value of an external asset.
    * LIQUIDITY_POOL_ASSET,   // Liquidity pool asset that is backed by the deposits of an asset pair's liquidity pool and earns trading fees. 
    * CREDIT_POOL_ASSET,      // Credit pool asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
    * OPTION_ASSET,           // Asset that enables the execution of a trade at a specific price until an expiration date. 
    * PREDICTION_ASSET,       // Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
    * GATEWAY_ASSET,          // Asset backed by deposits with an exchange counterparty of another asset or currency. 
    * UNIQUE_ASSET,           // Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset. 
    */

   /**
    * A Tradeable digital asset.
    * 
    * Assets have a globally unique symbol name that controls
    * how they are traded and an issuer who
    * has authority over the parameters of the asset.
    * Multiple types of asset are available, 
    * each with different properties and capabilities.
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

         id_type                         id; 

         asset_symbol_type               symbol;                    ///< Consensus enforced unique Ticker symbol string for this asset.

         asset_property_type             asset_type;                ///< The type of the asset.
         
         account_name_type               issuer;                    ///< name of the account which issued this asset.

         shared_string                   display_symbol;            ///< Non-consensus display name for interface reference.

         shared_string                   details;                   ///< Data that describes the purpose of this asset.

         shared_string                   json;                      ///< Additional JSON metadata of this asset.

         shared_string                   url;                       ///< Reference URL for the asset. 
         
         share_type                      max_supply;                ///< The maximum supply of this asset which may exist at any given time. 

         uint8_t                         stake_intervals;           ///< Weeks required to stake the asset.

         uint8_t                         unstake_intervals;         ///< Weeks require to unstake the asset.

         uint16_t                        market_fee_percent;        ///< Percentage of the total traded will be paid to the issuer of the asset.

         uint16_t                        market_fee_share_percent;  ///< Percentage of the market fee that will be shared with the account's referrers.

         share_type                      max_market_fee;            ///< Market fee charged on a trade is capped to this value.
         
         uint32_t                        issuer_permissions;        ///< The flags which the issuer has permission to update.

         uint32_t                        flags;                     ///< The currently active flags on this permission.

         flat_set< account_name_type >   whitelist_authorities;     ///< Accounts able to transfer this asset if the flag is set and whitelist is non-empty.

         flat_set< account_name_type >   blacklist_authorities;     ///< Accounts which cannot transfer or recieve this asset.

         flat_set< asset_symbol_type >   whitelist_markets;         ///< The assets that this asset may be traded against in the market

         flat_set< asset_symbol_type >   blacklist_markets;         ///< The assets that this asset may not be traded against in the market.

         time_point                      created;                   ///< Time that the asset was created.

         time_point                      last_updated;              ///< Time that the asset details were last updated.

         static bool is_valid_symbol( const string& symbol );       ///< True if symbol is a valid ticker symbol; false otherwise.

         bool is_market_issued()const                               ///< True if this is a market-issued asset; false otherwise. Market issued assets cannot be issued by the asset issuer. 
         { 
            switch( asset_type )
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

         bool require_balance_whitelist()const   ///< true if Holders must be whitelisted
         { 
            return ( flags & asset_issuer_permission_flags::balance_whitelist );
         }

         bool require_trade_whitelist()const     ///< true if Traders must be whitelisted
         { 
            return ( flags & asset_issuer_permission_flags::trade_whitelist );
         }

         bool is_maker_restricted()const        ///< true if only the asset issuer can create new trade orders into the orderbook
         { 
            return ( flags & asset_issuer_permission_flags::maker_restricted );
         }

         bool issuer_accept_requests()const      ///< true if the asset issuer can accept transfer requests from any holders
         { 
            return ( flags & asset_issuer_permission_flags::maker_restricted );
         }

         bool is_transfer_restricted()const     ///< true if this asset may only be transferred to/from the issuer or market orders
         { 
            return ( flags & asset_issuer_permission_flags::transfer_restricted );
         }

         bool can_request_transfer()const       ///< true if the asset can use transfer requests
         { 
            return !( flags & asset_issuer_permission_flags::disable_requests );
         }

         bool can_recurring_transfer()const      ///< true if the asset can use recurring transfers
         { 
            return !( flags & asset_issuer_permission_flags::disable_recurring );
         }

         bool enable_credit()const              ///< true if the asset can use credit pools, margin orders, and credit loans
         { 
            return !( flags & asset_issuer_permission_flags::disable_credit );
         }

         bool enable_liquid()const              ///< true if the asset can be included in liquidity pools and use liquidity pool orders
         { 
            return !( flags & asset_issuer_permission_flags::disable_liquid );
         }

         bool enable_options()const              ///< true if the asset can be included in option pools and generate option asset derivatives
         { 
            return !( flags & asset_issuer_permission_flags::disable_options );
         }

         bool enable_escrow()const              ///< true if the asset can be used in escrow transfers and marketplace purchases
         { 
            return !( flags & asset_issuer_permission_flags::disable_escrow );
         }

         bool enable_escrow()const              ///< true if the asset can be used in escrow transfers and marketplace purchases
         { 
            return !( flags & asset_issuer_permission_flags::disable_escrow );
         }

         bool enable_force_settle()const        ///< true if users may request force-settlement of the market-issued asset
         { 
            return !( flags & asset_issuer_permission_flags::disable_force_settle );
         }

         bool enable_confidential()const        ///< true if the asset supports confidential transfers
         { 
            return !( flags & asset_issuer_permission_flags::disable_confidential );
         }

         bool enable_auction()const            ///< true if the asset supports auction orders
         { 
            return !( flags & asset_issuer_permission_flags::disable_auction );
         }

         bool is_producer_fed()const            ///< true if the top elected producers may produce price feeds for the market issued asset
         { 
            return ( flags & asset_issuer_permission_flags::producer_fed_asset );
         }

         bool can_global_settle()const         ///< true if the issuer of this market-issued asset may globally settle the asset
         { 
            return ( flags & asset_issuer_permission_flags::global_settle );
         }

         bool require_governance()const         ///< true if the governance account of the issuer must approve asset issuance and changes
         { 
            return ( flags & asset_issuer_permission_flags::governance_oversight );
         }

         bool immutable_properties()const       ///< true if the asset cannot be modified after creation
         { 
            return ( flags & asset_issuer_permission_flags::immutable_properties );
         }
         
         void validate()const                    ///< UIAs may not have force settlement, or global settlements
         {
            if( !is_market_issued() )
            {
               FC_ASSERT(!(flags & disable_force_settle || flags & global_settle));
               FC_ASSERT(!(issuer_permissions & disable_force_settle || issuer_permissions & global_settle));
            }
         }
   };

    /**
    * Tracks the asset information that changes frequently.
    * 
    * Manages the supply statistics of each asset, 
    * and all the types of assets in liquid, staked, reward fund,
    * savings, delegated, and receiving balances.
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

         asset_symbol_type              symbol;                        ///< Ticker symbol string for this asset.

         account_name_type              issuer;                        ///< The asset issuing account. 

         share_type                     total_supply = 0;              ///< The total outstanding supply of the asset

         share_type                     liquid_supply = 0;             ///< The current liquid supply of the asset

         share_type                     staked_supply = 0;             ///< The current staked supply of the asset

         share_type                     reward_supply = 0;             ///< The current reward supply of the asset

         share_type                     savings_supply = 0;            ///< The current savings supply of the asset

         share_type                     delegated_supply = 0;          ///< The current delegated supply of the asset
         
         share_type                     receiving_supply = 0;          ///< The current receiving supply supply of the asset, should equal delegated

         share_type                     pending_supply = 0;            ///< The current supply contained in reward funds and active order objects

         share_type                     confidential_supply = 0;       ///< total confidential asset supply

         asset get_liquid_supply()const { return asset( liquid_supply, symbol ); }

         asset get_reward_supply()const { return asset( reward_supply, symbol ); }

         asset get_staked_supply()const { return asset( staked_supply, symbol ); }

         asset get_savings_supply()const { return asset( savings_supply, symbol ); }

         asset get_delegated_supply()const { return asset( delegated_supply, symbol ); }

         asset get_receiving_supply()const { return asset( receiving_supply, symbol ); }

         asset get_pending_supply()const { return asset( pending_supply, symbol ); }

         asset get_confidential_supply()const { return asset( confidential_supply, symbol ); }

         asset get_total_supply()const { return asset( total_supply, symbol ); }

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

         void asset_dynamic_data_object::adjust_delegated_supply(const asset& delta)    ///< Not included in total supply
         {
            assert(delta.symbol == symbol);
            delegated_supply += delta.amount;
         }

         void asset_dynamic_data_object::adjust_receiving_supply(const asset& delta)    ///< Not included in total supply
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
   };

   /**
    * Manages the details of a market issued bitasset.
    * 
    * Enables asset feed providers to upload price feeds to 
    * determine the ratio of collateral required for creating 
    * call orders to create collateralized debt positions of the 
    * @ref backing_asset
    *
    * In the event of a black swan, the swan price is saved
    * in the settlement price, and all margin positions
    * are settled at the same price with the collected collateral
    * being moved into the settlement fund. From this
    * point on, no further updates to the asset are permitted 
    * (no feeds, etc) and forced settlement occurs
    * immediately when requested, using the settlement price and fund.
    */
   class asset_bitasset_data_object : public object < asset_bitasset_data_object_type, asset_bitasset_data_object>
   {
      asset_bitasset_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_bitasset_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                   id;

         asset_symbol_type         symbol;                                        ///< The symbol of the bitasset that this object belongs to

         account_name_type         issuer;                                        ///< The account name of the issuer 

         asset_symbol_type         backing_asset;                                 ///< The collateral backing asset of the bitasset

         flat_map< account_name_type, pair< time_point, price_feed > >  feeds;    ///< Feeds published for this asset. 

         price_feed                current_feed;                                  ///< Currently active price feed, median of values from the currently active feeds.

         time_point                current_feed_publication_time;                 ///< Publication time of the oldest feed which was factored into current_feed.

         price                     current_maintenance_collateralization;         ///< Call orders with collateralization (aka collateral/debt) not greater than this value are in margin call territory.

         share_type                force_settled_volume;                          ///< This is the volume of this asset which has been force-settled this 24h interval
         
         price                     settlement_price;                              ///< Price at which force settlements of a black swanned asset will occur
         
         asset                     settlement_fund;                               ///< Amount of collateral which is available for force settlement

         fc::microseconds          feed_lifetime;                                 ///< Time before a price feed expires

         uint8_t                   minimum_feeds;                                 ///< Minimum number of unexpired feeds required to extract a median feed from

         fc::microseconds          force_settlement_delay;                        ///< This is the delay between the time a long requests settlement and the chain evaluates the settlement

         uint16_t                  force_settlement_offset_percent;               ///< The percentage to adjust the feed price in the short's favor in the event of a forced settlement

         uint16_t                  maximum_force_settlement_volume;               ///< the percentage of current supply which may be force settled within each 24h interval.

         share_type                max_force_settlement_volume( share_type total_supply )const     ///< Calculate the maximum force settlement volume per 24h interval, given the current share supply     
         {
            if( maximum_force_settlement_volume == 0 )
            {
               return 0;
            }
            
            if( maximum_force_settlement_volume == PERCENT_100 )
            {
               return total_supply + force_settled_volume;
            }

            fc::uint128 volume = total_supply.value + force_settled_volume.value;
            volume *= maximum_force_settlement_volume;
            volume /= PERCENT_100;
            return volume.to_uint64();
         }

         bool                                          has_settlement()const    ///< True if there has been a black swan
         { 
            return !settlement_price.is_null();    
         }   

         time_point                                    feed_expiration_time()const    ///< The time when current_feed would expire
         {
            return current_feed_publication_time + feed_lifetime;   
         }

         bool                                          feed_is_expired(time_point current_time)const
         { 
            return feed_expiration_time() <= current_time; 
         }

         /**
          * Calculates the median feed from feeds, feed_lifetime.
          * 
          * It may update the current_feed_publication_time, current_feed and
          * current_maintenance_collateralization member variables.
          * @param current_time the current time to use in the calculations
          */
         void update_median_feeds( time_point current_time );
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

         account_name_type          business_account;            ///< The business account name of the issuer.

         asset_symbol_type          symbol;                      ///< The symbol of the equity asset of the business.

         asset_symbol_type          dividend_asset;              ///< The asset used to distribute dividends to asset holders.

         asset                      dividend_pool;               ///< Amount of assets pooled for distribution at the next interval.
         
         time_point                 last_dividend;               ///< Time that the asset last distributed a dividend.

         uint16_t                   dividend_share_percent;      ///< Percentage of incoming assets added to the dividends pool.

         uint16_t                   liquid_dividend_percent;     ///< Percentage of equity dividends distributed to liquid balances.

         uint16_t                   staked_dividend_percent;     ///< Percentage of equity dividends distributed to staked balances.

         uint16_t                   savings_dividend_percent;    ///< Percentage of equity dividends distributed to savings balances.

         uint16_t                   liquid_voting_rights;        ///< Amount of votes per asset conveyed to liquid holders of the asset.

         uint16_t                   staked_voting_rights;        ///< Amount of votes per asset conveyed to staked holders of the asset.

         uint16_t                   savings_voting_rights;       ///< Amount of votes per asset conveyed to savings holders of the asset

         fc::microseconds           min_active_time;             ///< Time that account must have a recent activity reward within to earn dividend.

         share_type                 min_balance;                 ///< Minimum amount of equity required to earn dividends.

         uint16_t                   min_producers;               ///< Minimum amount of producer votes required to earn dividends.

         uint16_t                   boost_balance;               ///< Amount of equity balance to earn double dividends.

         uint16_t                   boost_activity;              ///< Amount of recent activity rewards required to earn double dividends.

         uint16_t                   boost_producers;             ///< Amount of producer votes required to earn double dividends.

         uint16_t                   boost_top;                   ///< Percent bonus earned by Top membership accounts.

         void adjust_pool( const asset& delta )
         {
            assert( delta.symbol == dividend_asset );
            dividend_pool += delta;
         };
   };


   /**
    * Credit assets enable business accounts to borrow funds.
    * 
    * By issuing credit assets, backed by repurchases up to a given buyback price, or face value
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

         id_type                id;

         account_name_type      business_account;                ///< The business account name of the issuer.

         asset_symbol_type      symbol;                          ///< The symbol of the credit asset of the business.

         asset_symbol_type      buyback_asset;                   ///< Symbol used to buyback credit assets.

         asset                  buyback_pool;                    ///< Amount of assets pooled to buyback the asset at next interval.

         price                  buyback_price;                   ///< Price at which the credit asset is bought back.
         
         time_point             last_buyback;                    ///< Time that the asset was last updated.

         uint32_t               buyback_share_percent;           ///< Percentage of incoming assets added to the buyback pool

         uint32_t               liquid_fixed_interest_rate;      ///< Fixed component of Interest rate of the asset for liquid balances.

         uint32_t               liquid_variable_interest_rate;   ///< Variable component of Interest rate of the asset for liquid balances.

         uint32_t               staked_fixed_interest_rate;      ///< Fixed component of Interest rate of the asset for staked balances.

         uint32_t               staked_variable_interest_rate;   ///< Variable component of Interest rate of the asset for staked balances.

         uint32_t               savings_fixed_interest_rate;     ///< Fixed component of Interest rate of the asset for savings balances.

         uint32_t               savings_variable_interest_rate;  ///< Variable component of Interest rate of the asset for savings balances.

         uint32_t               var_interest_range;              ///< The percentage range from the buyback price over which to apply the variable interest rate.

         void adjust_pool( const asset& delta )
         {
            assert( delta.symbol == buyback_asset );
            buyback_pool += delta;
         }
   };

   /**
    * Asset liquidity pools contain an open market making balance of two assets.
    *  
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

         account_name_type          issuer;                        ///< Name of the account which created the liquidity pool.

         asset_symbol_type          symbol_a;                      ///< Ticker symbol string of the asset with the lower ID. Must be core asset if one asset is core.

         asset_symbol_type          symbol_b;                      ///< Ticker symbol string of the asset with the higher ID.

         asset_symbol_type          symbol_liquid;                 ///< Ticker symbol of the pool's liquidity pool asset. 

         asset                      balance_a;                     ///< Balance of Asset A. Must be core asset if one asset is core.

         asset                      balance_b;                     ///< Balance of Asset B.

         asset                      balance_liquid;                ///< Outstanding supply of the liquidity asset for the asset pair.

         price                      hour_median_price;             ///< The median price over the past hour, at 10 minute intervals. Used for collateral calculations. 

         price                      day_median_price;              ///< The median price over the last day, at 10 minute intervals.

         bip::deque< price, allocator< price > >  price_history;   ///< Tracks the last 24 hours of median price, one per 10 minutes.

         price                      current_price()const           ///< Liquidity pools current price of Asset A and Asset B. Asset A is the base asset, Asset B is the quote asset.
         {
            return price( balance_a, balance_b );
         }

         price                       base_price( const asset_symbol_type& base )const   ///< Current price with specified asset as base
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

         price                       base_hour_median_price( const asset_symbol_type& base )const   ///< hourly median price with specified asset as base
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

         price                       base_day_median_price( const asset_symbol_type& base )const   ///< daily median price with specified asset as base
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

   /**
    * Holds a reserve of an asset for lending to borrowers.
    * 
    * Funds can be lent to the pool in exchange for the credit pool
    * asset, and earn interest from borrowers when funds are used for
    * margin trading and borrowing orders.
    * 
    * The interest rate charged varies with the ratio of funds 
    * available to the funds currently loaned out. 
    * Interest rates lower when more funds are available in the 
    * @ref base_balance and increase when the @ref borrowed_balance becomes higher. 
    * 
    * Each asset specifies a minimum interest rate,
    * and a variable interest rate that enable 
    * the rate to float accoridng to market credit conditions.
    */
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

         account_name_type          issuer;                 ///< Name of the account which created the credit pool.

         asset_symbol_type          base_symbol;            ///< Ticker symbol string of the base asset being lent and borrowed.

         asset_symbol_type          credit_symbol;          ///< Ticker symbol string of the credit asset for use as collateral to borrow the base asset.

         asset                      base_balance;           ///< Balance of the base asset that is available for loans and redemptions. 

         asset                      borrowed_balance;       ///< Total amount of base asset currently lent to borrowers, accumulates compounding interest payments. 

         asset                      credit_balance;         ///< Balance of the credit asset redeemable for an increasing amount of base asset.

         share_type                 last_interest_rate;     ///< The most recently calculated interest rate when last compounded. 

         price                      last_price;             ///< The last price that assets were lent or withdrawn at. 

         price                      current_price()const    ///< Credit pool's current price of the credit asset, denominated in the base asset. Increases over time with incoming interest.
         {
            return price( base_balance + borrowed_balance, credit_balance );
         }      

         share_type                 interest_rate( uint16_t min, uint16_t var )const  ///< Current annualized interest rate of borrowing and lending. Updates whenever a loan is taken or repaid.  
         {
            return share_type( std::min( uint64_t( 
               min + var * ( ( borrowed_balance.amount.value + BLOCKCHAIN_PRECISION ) / 
               ( base_balance.amount.value + BLOCKCHAIN_PRECISION ) ) ), uint64_t( 50 * PERCENT_1 ) ) );
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

   struct by_backing_asset;
   struct by_feed_expiration;

   typedef multi_index_container<
      asset_bitasset_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_bitasset_data_object, asset_bitasset_data_id_type, &asset_bitasset_data_object::id > >,
         ordered_unique< tag<by_symbol>, member< asset_bitasset_data_object, asset_symbol_type, &asset_bitasset_data_object::symbol > >,
         ordered_non_unique< tag<by_backing_asset>, 
            member< asset_bitasset_data_object, asset_symbol_type, &asset_bitasset_data_object::backing_asset > 
         >,
         ordered_unique< tag<by_feed_expiration>,
            composite_key< asset_bitasset_data_object,
               const_mem_fun< asset_bitasset_data_object, time_point, &asset_bitasset_data_object::feed_expiration_time >,
               member< asset_bitasset_data_object, asset_bitasset_data_id_type, &asset_bitasset_data_object::id >
            >
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
   struct by_symbol_a;
   struct by_symbol_b;

   typedef multi_index_container<
      asset_liquidity_pool_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > >,
         ordered_unique< tag<by_asset_pair>,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_a >,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_b > 
            >
         >,
         ordered_unique< tag<by_symbol_a>,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_a >,
               member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > 
            >
         >,
         ordered_unique< tag<by_symbol_b>,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_b >,
               member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > 
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

} } ///< node::chain

FC_REFLECT( node::chain::asset_object,
         (id)
         (symbol)
         (asset_type)
         (issuer)
         (options)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_object, node::chain::asset_index );

FC_REFLECT( node::chain::asset_dynamic_data_object,
         (id)
         (symbol)
         (issuer)
         (total_supply)
         (liquid_supply)
         (staked_supply)
         (reward_supply)
         (savings_supply)
         (delegated_supply)
         (receiving_supply)
         (pending_supply)
         (confidential_supply)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_dynamic_data_object, node::chain::asset_dynamic_data_index );

FC_REFLECT( node::chain::asset_bitasset_data_object,
         (id)
         (symbol)
         (issuer)
         (backing_asset)
         (options)
         (feeds)
         (current_feed)
         (current_feed_publication_time)
         (current_maintenance_collateralization)
         (force_settled_volume)
         (settlement_price)
         (settlement_fund)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_bitasset_data_object, node::chain::asset_bitasset_data_index );

FC_REFLECT( node::chain::asset_equity_data_object,
         (id)
         (business_account)
         (symbol)
         (options)
         (dividend_asset)
         (dividend_pool)
         (last_dividend)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_equity_data_object, node::chain::asset_equity_data_index );

FC_REFLECT( node::chain::asset_credit_data_object,
         (id)
         (business_account)
         (symbol)
         (options)
         (buyback_asset)
         (buyback_pool)
         (buyback_price)
         (symbol_a)
         (symbol_b)
         (last_buyback)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_credit_data_object, node::chain::asset_credit_data_index );

FC_REFLECT( node::chain::asset_liquidity_pool_object,
         (id)
         (issuer)
         (symbol_a)
         (symbol_b)
         (symbol_liquid)
         (balance_a)
         (balance_b)
         (balance_liquid)
         (hour_median_price)
         (day_median_price)
         (price_history)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_liquidity_pool_object, node::chain::asset_liquidity_pool_index );

FC_REFLECT( node::chain::asset_credit_pool_object,
         (id)
         (issuer)
         (base_symbol)
         (credit_symbol)
         (base_balance)
         (borrowed_balance)
         (credit_balance)
         (last_interest_rate)
         (last_price)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_credit_pool_object, node::chain::asset_credit_pool_index );