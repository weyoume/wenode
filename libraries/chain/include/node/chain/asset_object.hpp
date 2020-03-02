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
         asset_object( Constructor&& c, allocator< Allocator > a ) :
         display_symbol(a), details(a), json(a), url(a)
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

         flat_set< account_name_type >   blacklist_authorities;     ///< Accounts which cannot transfer or receive this asset.

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
            return ( flags & asset_issuer_permission_flags::issuer_accept_requests );
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

         asset                          get_liquid_supply()const { return asset( liquid_supply, symbol ); }

         asset                          get_reward_supply()const { return asset( reward_supply, symbol ); }

         asset                          get_staked_supply()const { return asset( staked_supply, symbol ); }

         asset                          get_savings_supply()const { return asset( savings_supply, symbol ); }

         asset                          get_delegated_supply()const { return asset( delegated_supply, symbol ); }

         asset                          get_receiving_supply()const { return asset( receiving_supply, symbol ); }

         asset                          get_pending_supply()const { return asset( pending_supply, symbol ); }

         asset                          get_confidential_supply()const { return asset( confidential_supply, symbol ); }

         asset                          get_total_supply()const { return asset( total_supply, symbol ); }

         void                           adjust_liquid_supply(const asset& delta)
         { try {
            FC_ASSERT( delta.symbol == symbol );
            liquid_supply += delta.amount;
            total_supply += delta.amount;
            FC_ASSERT( liquid_supply >= 0 );
            FC_ASSERT( total_supply >= 0 );

         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_staked_supply(const asset& delta)
         { try {
            FC_ASSERT( delta.symbol == symbol );
            staked_supply += delta.amount;
            total_supply += delta.amount;
            FC_ASSERT( staked_supply >= 0 );
            FC_ASSERT( total_supply >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_reward_supply(const asset& delta)
         { try {
            FC_ASSERT( delta.symbol == symbol );
            reward_supply += delta.amount;
            total_supply += delta.amount;
            FC_ASSERT( reward_supply >= 0 );
            FC_ASSERT( total_supply >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_savings_supply(const asset& delta)
         { try {
            FC_ASSERT( delta.symbol == symbol );
            savings_supply += delta.amount;
            total_supply += delta.amount;
            FC_ASSERT( savings_supply >= 0 );
            FC_ASSERT( total_supply >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_delegated_supply(const asset& delta)    ///< Not included in total supply
         { try {
            FC_ASSERT( delta.symbol == symbol );
            delegated_supply += delta.amount;
            FC_ASSERT( delegated_supply >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_receiving_supply(const asset& delta)    ///< Not included in total supply
         { try {
            FC_ASSERT( delta.symbol == symbol );
            receiving_supply += delta.amount;
            FC_ASSERT( receiving_supply >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_pending_supply(const asset& delta)
         { try {
            FC_ASSERT( delta.symbol == symbol );
            pending_supply += delta.amount;
            total_supply += delta.amount;
            FC_ASSERT( pending_supply >= 0 );
            FC_ASSERT( total_supply >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }
   };


   /**
    * Manages the details of a currency asset.
    * 
    * Enables currency asset issuers to specify the
    * characteristics of the distribution of the cryptocurrency 
    * at creation, which cannot be altered afterwards. 
    * 
    * Currency assets are distributed to network contributors,
    * and each have thier own reward fund objects.
    */
   class asset_currency_data_object : public object < asset_currency_data_object_type, asset_currency_data_object>
   {
      asset_currency_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_currency_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                   id;

         asset_symbol_type         symbol;                             ///< The symbol of the currency asset.

         asset                     block_reward;                       ///< The value of the initial reward paid into the reward fund every block.

         uint16_t                  block_reward_reduction_percent;     ///< The percentage by which the block reward is reduced each period. 0 for no reduction.

         uint16_t                  block_reward_reduction_days;        ///< Number of days between reduction events. 0 for no reduction.

         uint16_t                  content_reward_percent;             ///< Percentage of reward paid to content creators.

         asset_symbol_type         equity_asset;                       ///< Asset that will receive equity rewards.

         uint16_t                  equity_reward_percent;              ///< Percentage of reward paid to staked equity asset holders.

         uint16_t                  producer_reward_percent;            ///< Percentage of reward paid to block producers.

         uint16_t                  supernode_reward_percent;           ///< Percentage of reward paid to supernode operators.

         uint16_t                  power_reward_percent;               ///< Percentage of reward paid to staked currency asset holders.

         uint16_t                  community_fund_reward_percent;      ///< Percentage of reward paid to community fund proposals.

         uint16_t                  development_reward_percent;         ///< Percentage of reward paid to elected developers.

         uint16_t                  marketing_reward_percent;           ///< Percentage of reward paid to elected marketers.

         uint16_t                  advocacy_reward_percent;            ///< Percentage of reward paid to elected advocates.

         uint16_t                  activity_reward_percent;            ///< Percentage of reward paid to active accounts each day.

         uint16_t                  producer_block_reward_percent;      ///< Percentage of producer reward paid to the producer of each individual block.

         uint16_t                  validation_reward_percent;          ///< Percentage of producer reward paid to validators of blocks.

         uint16_t                  txn_stake_reward_percent;           ///< Percentage of producer reward paid to producers according to transaction stake weight of blocks.

         uint16_t                  work_reward_percent;                ///< Percentage of producer reward paid to proof of work mining producers for each proof.

         uint16_t                  producer_activity_reward_percent;   ///< Percentage of producer reward paid to the highest voted producer in activity rewards.
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
            return share_type( volume.to_uint64());
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

         flat_map< asset_symbol_type, asset >   dividend_pool;   ///< Assets pooled for distribution at the next interval.
         
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

         share_type                 boost_balance;               ///< Amount of equity balance to earn double dividends.

         share_type                 boost_activity;              ///< Amount of recent activity rewards required to earn double dividends.

         share_type                 boost_producers;             ///< Amount of producer votes required to earn double dividends.

         uint16_t                   boost_top;                   ///< Percent bonus earned by Top membership accounts.

         void                       adjust_pool( const asset& delta )
         { try {
            if( dividend_pool[ delta.symbol ].amount >= 0 && dividend_pool[ delta.symbol ].symbol == delta.symbol )
            {
               dividend_pool[ delta.symbol ] += delta;
            }
            else
            {
               FC_ASSERT( delta.amount >= 0 );
               dividend_pool[ delta.symbol ] = delta; 
            }
            FC_ASSERT( dividend_pool[ delta.symbol ].amount >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) };
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

         price                  buyback_price;                   ///< Price at which the credit asset is bought back. Buyback asset is base.
         
         time_point             last_buyback;                    ///< Time that the last credit buyback occurred.

         uint32_t               buyback_share_percent;           ///< Percentage of incoming assets added to the buyback pool

         uint32_t               liquid_fixed_interest_rate;      ///< Fixed component of Interest rate of the asset for liquid balances.

         uint32_t               liquid_variable_interest_rate;   ///< Variable component of Interest rate of the asset for liquid balances.

         uint32_t               staked_fixed_interest_rate;      ///< Fixed component of Interest rate of the asset for staked balances.

         uint32_t               staked_variable_interest_rate;   ///< Variable component of Interest rate of the asset for staked balances.

         uint32_t               savings_fixed_interest_rate;     ///< Fixed component of Interest rate of the asset for savings balances.

         uint32_t               savings_variable_interest_rate;  ///< Variable component of Interest rate of the asset for savings balances.

         uint32_t               var_interest_range;              ///< The percentage range from the buyback price over which to apply the variable interest rate.

         void                   adjust_pool( const asset& delta )
         {
            FC_ASSERT( delta.symbol == buyback_asset );
            buyback_pool += delta;
         }
   };


   /**
    * Unique assets enable a single unit of an asset
    * to be transferred between accounts to represent 
    * a non-fungible asset.
    * 
    * Unique Assets can be used to represent a physical item
    * of property, and contain the metadata of the object
    * and an access list for who can access the item.
    * 
    * Unique assets can be transferred by the holder
    * of a majority of a nominated ownership asset.
    */
   class asset_unique_data_object : public object < asset_unique_data_object_type, asset_unique_data_object>
   {
      asset_unique_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_unique_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                id;

         asset_symbol_type                      symbol;                 ///< The symbol of the unique asset.

         account_name_type                      issuer;                 ///< The account name of the issuer.

         account_name_type                      controlling_owner;      ///< Account that currently owns the asset. Controls the control list.

         asset_symbol_type                      ownership_asset;        ///< Asset that represents controlling ownership of the unique asset. Same as symbol for no liquid ownership asset.

         flat_set< account_name_type >          control_list;           ///< List of accounts that have control over access to the unique asset.

         flat_set< account_name_type >          access_list;            ///< List of accounts that have access to the unique asset. 

         flat_map< asset_symbol_type, asset >   revenue_pool;           ///< Assets pooled for distribution to the ownership asset holders. 

         asset                                  access_price;           ///< Price per day for all accounts in the access list.
         
         time_point                             last_distribution;      ///< Time that the revenue pool was distributed to ownership asset holders.

         void                                   adjust_pool( const asset& delta )
         { try {
            if( revenue_pool[ delta.symbol ].amount >= 0 && revenue_pool[ delta.symbol ].symbol == delta.symbol )
            {
               revenue_pool[ delta.symbol ] += delta;
            }
            else
            {
               FC_ASSERT( delta.amount >= 0 );
               revenue_pool[ delta.symbol ] = delta; 
            }
            FC_ASSERT( revenue_pool[ delta.symbol ].amount >= 0 );
            
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) };
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
    * R      = Br * ( 1 - ( Bi / ( I + Bi ) ) )    Amount received for a given input
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

         std::deque< price >        price_history;                 ///< Tracks the last 24 hours of median price, one per 10 minutes.

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
            else
            {
               return price();
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
            else
            {
               return price();
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
            else
            {
               return price();
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
            else
            {
               return asset();
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
               min + var * ( ( borrowed_balance.amount.value + BLOCKCHAIN_PRECISION.value ) / 
               ( base_balance.amount.value + BLOCKCHAIN_PRECISION.value ) ) ), uint64_t( 50 * PERCENT_1 ) ) );
         }
   };


   /**
    * Manages the option chain sheet of an asset trading pair.
    * 
    * Each asset pair creates 21 option strike prices each month
    * that can be used to issue option assets. Option assets are opened
    * for 12 months in advance of the current month, 
    * and each expiration adds a new month in the year ahead.
    * Strike prices are determined by rounding to the nearest significant figure
    * and incrementing by 5% intervals up and down the price book.
    */
   class asset_option_pool_object : public object< asset_option_pool_object_type, asset_option_pool_object >
   {
      asset_option_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_option_pool_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                            id; 

         account_name_type                  issuer;            ///< Name of the account which created the option pool.

         asset_symbol_type                  base_symbol;       ///< Ticker symbol of the base asset of the trading pair.

         asset_symbol_type                  quote_symbol;      ///< Ticker symbol of the quote asset of the trading pair.

         vector< option_strike >            call_strikes;      ///< Available strike price and expirations of call options to buy the quote asset.

         vector< option_strike >            put_strikes;       ///< Available strike price and expirations of put options to sell the quote asset.

         flat_map< option_strike, asset >   open_interest;     ///< Outstanding supply of options at each strike price and expiration.
   };


   /**
    * Manages a prediction market.
    * 
    * Enables a trading market on the likelyhood of
    * a set of public events occuring on or before a specified time
    * period in the future.
    * 
    * Each prediction market has multiple outcome assets,
    * each representing a full unit of the underlying asset 
    * in the event that an outcome occurs.
    * 
    * Funding the prediction pool returns one unit of each outcome
    * asset to the owner, which can be sold.
    * 
    * All funds in the pool are able to be withdrawn after the event
    * has concluded by the holders of the realized outcome asset.
    * 
    * One unit of all outcome assets can be combined and returned to
    * withdraw one unit of the underlying asset. Under normal market conditions, 
    * all asset prices should sum to the value of one unit of the 
    * underlying funding asset.
    * 
    * Prediction markets must include to be valid:
    * - A Public event.
    * - With a series of at least 2 possible outcomes.
    * - That can be reasonably determined on or before a fixed time period.
    * - Where only one outcome can possibly occur.
    * - A public source of information that can be used to verify the outcome.
    * 
    * Each prediction market contains an additional outcome 
    * for invalid or none of the above, in the case that
    * there is a resolution error.
    * 
    * The Security bond is placed by the issuer as a
    * demonstration of confidence in the market, and is lost if the 
    * market is not resolved by the completion time.
    */
   class asset_prediction_pool_object : public object< asset_prediction_pool_object_type, asset_prediction_pool_object >
   {
      asset_prediction_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_prediction_pool_object( Constructor&& c, allocator< Allocator > a ) :
         details(a), outcome_details(a)
         {
            c( *this );
         }

         id_type                            id;

         account_name_type                  issuer;              ///< Name of the account which created the prediction market pool.

         asset_symbol_type                  prediction_symbol;   ///< Ticker symbol of the prediction pool primary asset.

         asset_symbol_type                  collateral_symbol;   ///< Ticker symbol of the collateral asset backing the prediction market.

         asset                              collateral_pool;     ///< Funds accumulated by outcome asset positions for distribution to winning outcome. 

         vector< asset_symbol_type >        outcome_assets;      ///< Available strike price and expirations of call options to buy the quote asset.

         shared_string                      details;             ///< Description of the market, how it will be resolved using known public data sources.

         flat_map< asset_symbol_type, shared_string >   outcome_details;       ///< Description of each outcome and the resolution conditions for each asset. 

         time_point                         min_closing_time;    ///< Time after which the market can be closed.

         time_point                         max_closing_time;    ///< Time by which the market must be closed with a selected outcome.

         asset                              prediction_bond;     ///< Security deposit placed by the issuer on the market.
   };


   class asset_distribution_object : public object< asset_distribution_object_type, asset_distribution_object >
   {
      asset_distribution_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_distribution_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                id;

         account_name_type                      issuer;                       ///< Name of the account which created the distribution market pool.

         asset_symbol_type                      distribution_symbol;          ///< Asset that is generated by the distribution.

         asset_symbol_type                      fund_symbol;                  ///< Ticker symbol of the asset being accepted for distribution assets.

         asset                                  fund_pool;                    ///< Pool of funds received.

         shared_string                          details;                      ///< Description of the distribution process.

         uint32_t                               min_input_fund_units;         ///< Minimum funds required for each round of the distribution process.

         uint32_t                               max_input_fund_units;         ///< Maximum funds to be accepted before closing each distribution round.

         uint32_t                               distribution_rounds;          ///< Number of distribution rounds, total distribution amount is divided between all rounds.

         uint32_t                               distribution_interval_days;   ///< Duration of each distribution round, in days.

         vector< asset_unit >                   input_fund_unit;              ///< The integer unit ratio for distribution of incoming funds.

         vector< asset_unit >                   output_distribution_unit;     ///< The integer unit ratio for distribution of released funds.

         uint16_t                               min_unit_ratio;               ///< The lowest value of unit ratio between input and output units.

         uint16_t                               max_unit_ratio;               ///< The highest possible initial value of unit ratio between input and output units. 
         
         time_point                             begin_time;                   ///< Time to begin the first distribution.
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

   typedef multi_index_container<
      asset_currency_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_currency_data_object, asset_currency_data_id_type, &asset_currency_data_object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_currency_data_object, asset_symbol_type, &asset_currency_data_object::symbol> >
      >, 
      allocator< asset_currency_data_object >
   > asset_currency_data_index;

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

   typedef multi_index_container<
      asset_equity_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_equity_data_object, asset_equity_data_id_type, &asset_equity_data_object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_equity_data_object, asset_symbol_type, &asset_equity_data_object::symbol> >
      >,
      allocator< asset_equity_data_object >
   > asset_equity_data_index;

   typedef multi_index_container<
      asset_credit_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_credit_data_object, asset_credit_data_id_type, &asset_credit_data_object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_credit_data_object, asset_symbol_type, &asset_credit_data_object::symbol> >
      >,
      allocator< asset_credit_data_object >
   > asset_credit_data_index;

   typedef multi_index_container<
      asset_unique_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_unique_data_object, asset_unique_data_id_type, &asset_unique_data_object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_unique_data_object, asset_symbol_type, &asset_unique_data_object::symbol> >
      >,
      allocator< asset_unique_data_object >
   > asset_unique_data_index;

   struct by_asset_pair;
   struct by_symbol_a;
   struct by_symbol_b;

   typedef multi_index_container<
      asset_liquidity_pool_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > >,
         ordered_unique< tag< by_asset_pair >,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_a >,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_b > 
            >
         >,
         ordered_unique< tag< by_symbol_a >,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_a >,
               member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > 
            >
         >,
         ordered_unique< tag< by_symbol_b >,
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

   struct by_quote_symbol;

   typedef multi_index_container<
      asset_option_pool_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_option_pool_object, asset_option_pool_id_type, &asset_option_pool_object::id > >,
         ordered_unique< tag<by_base_symbol>,
            composite_key< asset_option_pool_object,
               member< asset_option_pool_object, asset_symbol_type, &asset_option_pool_object::base_symbol >,
               member< asset_option_pool_object, asset_option_pool_id_type, &asset_option_pool_object::id > 
            >
         >,
         ordered_unique< tag<by_quote_symbol>,
            composite_key< asset_option_pool_object,
               member< asset_option_pool_object, asset_symbol_type, &asset_option_pool_object::quote_symbol >,
               member< asset_option_pool_object, asset_option_pool_id_type, &asset_option_pool_object::id > 
            >
         >
      >, 
      allocator< asset_option_pool_object >
   > asset_option_pool_index;

   struct by_prediction_symbol;
   struct by_collateral_symbol;

   typedef multi_index_container<
      asset_prediction_pool_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_prediction_pool_object, asset_prediction_pool_id_type, &asset_prediction_pool_object::id > >,
         ordered_unique< tag<by_prediction_symbol>, member< asset_prediction_pool_object, asset_symbol_type, &asset_prediction_pool_object::prediction_symbol > >,
         ordered_unique< tag<by_collateral_symbol>,
            composite_key< asset_prediction_pool_object,
               member< asset_prediction_pool_object, asset_symbol_type, &asset_prediction_pool_object::collateral_symbol >,
               member< asset_prediction_pool_object, asset_prediction_pool_id_type, &asset_prediction_pool_object::id > 
            >
         >
      >, 
      allocator< asset_prediction_pool_object >
   > asset_prediction_pool_index;

   struct by_distribution_symbol;
   struct by_fund_symbol;

   typedef multi_index_container<
      asset_distribution_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_distribution_object, asset_distribution_id_type, &asset_distribution_object::id > >,
         ordered_unique< tag<by_distribution_symbol>, member< asset_distribution_object, asset_symbol_type, &asset_distribution_object::distribution_symbol > >,
         ordered_unique< tag<by_fund_symbol>,
            composite_key< asset_distribution_object,
               member< asset_distribution_object, asset_symbol_type, &asset_distribution_object::fund_symbol >,
               member< asset_distribution_object, asset_distribution_id_type, &asset_distribution_object::id > 
            >
         >
      >, 
      allocator< asset_distribution_object >
   > asset_distribution_index;

} } ///< node::chain

FC_REFLECT( node::chain::asset_object,
         (id)
         (symbol)
         (asset_type)
         (issuer)
         (display_symbol)
         (details)
         (json)
         (url)
         (max_supply)
         (stake_intervals)
         (unstake_intervals)
         (market_fee_percent)
         (market_fee_share_percent)
         (max_market_fee)
         (issuer_permissions)
         (flags)
         (whitelist_authorities)
         (blacklist_authorities)
         (whitelist_markets)
         (blacklist_markets)
         (created)
         (last_updated)
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

FC_REFLECT( node::chain::asset_currency_data_object,
         (id)
         (symbol)
         (block_reward)
         (block_reward_reduction_percent)
         (block_reward_reduction_days)
         (content_reward_percent)
         (equity_asset)
         (equity_reward_percent)
         (producer_reward_percent)
         (supernode_reward_percent)
         (power_reward_percent)
         (community_fund_reward_percent)
         (development_reward_percent)
         (marketing_reward_percent)
         (advocacy_reward_percent)
         (activity_reward_percent)
         (producer_block_reward_percent)
         (validation_reward_percent)
         (txn_stake_reward_percent)
         (work_reward_percent)
         (producer_activity_reward_percent)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_currency_data_object, node::chain::asset_currency_data_index );

FC_REFLECT( node::chain::asset_bitasset_data_object,
         (id)
         (symbol)
         (issuer)
         (backing_asset)
         (feeds)
         (current_feed)
         (current_feed_publication_time)
         (current_maintenance_collateralization)
         (force_settled_volume)
         (settlement_price)
         (settlement_fund)
         (feed_lifetime)
         (minimum_feeds)
         (force_settlement_delay)
         (force_settlement_offset_percent)
         (maximum_force_settlement_volume)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_bitasset_data_object, node::chain::asset_bitasset_data_index );

FC_REFLECT( node::chain::asset_equity_data_object,
         (id)
         (business_account)
         (symbol)
         (dividend_pool)
         (last_dividend)
         (dividend_share_percent)
         (liquid_dividend_percent)
         (staked_dividend_percent)
         (savings_dividend_percent)
         (liquid_voting_rights)
         (staked_voting_rights)
         (savings_voting_rights)
         (min_active_time)
         (min_balance)
         (min_producers)
         (boost_balance)
         (boost_activity)
         (boost_producers)
         (boost_top)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_equity_data_object, node::chain::asset_equity_data_index );

FC_REFLECT( node::chain::asset_credit_data_object,
         (id)
         (business_account)
         (symbol)
         (buyback_asset)
         (buyback_pool)
         (buyback_price)
         (last_buyback)
         (buyback_share_percent)
         (liquid_fixed_interest_rate)
         (liquid_variable_interest_rate)
         (staked_fixed_interest_rate)
         (staked_variable_interest_rate)
         (savings_fixed_interest_rate)
         (savings_variable_interest_rate)
         (var_interest_range)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_credit_data_object, node::chain::asset_credit_data_index );

FC_REFLECT( node::chain::asset_unique_data_object,
         (id)
         (symbol)
         (issuer)
         (controlling_owner)
         (ownership_asset)
         (control_list)
         (access_list)
         (revenue_pool)
         (access_price)
         (last_distribution)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_unique_data_object, node::chain::asset_unique_data_index );

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

FC_REFLECT( node::chain::asset_option_pool_object,
         (id)
         (issuer)
         (base_symbol)
         (quote_symbol)
         (call_strikes)
         (put_strikes)
         (open_interest)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_option_pool_object, node::chain::asset_option_pool_index );

FC_REFLECT( node::chain::asset_prediction_pool_object,
         (id)
         (issuer)
         (prediction_symbol)
         (collateral_symbol)
         (collateral_pool)
         (outcome_assets)
         (details)
         (outcome_details)
         (min_closing_time)
         (max_closing_time)
         (prediction_bond)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_prediction_pool_object, node::chain::asset_prediction_pool_index );

FC_REFLECT( node::chain::asset_distribution_object,
         (id)
         (issuer)
         (distribution_symbol)
         (fund_symbol)
         (fund_pool)
         (details)
         (min_input_fund_units)
         (max_input_fund_units)
         (distribution_rounds)
         (distribution_interval_days)
         (input_fund_unit)
         (output_distribution_unit)
         (min_unit_ratio)
         (max_unit_ratio)
         (begin_time)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_distribution_object, node::chain::asset_distribution_index );