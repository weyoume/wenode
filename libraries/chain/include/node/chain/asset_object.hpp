#pragma once
#include <fc/fixed_string.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/authority.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace node { namespace chain {

   using node::protocol::approx_sqrt;
   using node::protocol::round_sig_figs;

   /**
    * ASSET TYPES
    * 
    * CURRENCY_ASSET,         ///< Cryptocurrency that is issued by the network, starts from zero supply, issuing account is the null account, cannot be issued by any accounts. 
    * STANDARD_ASSET,         ///< Regular asset, can be transferred and staked, saved, and delegated.
    * STABLECOIN_ASSET,       ///< Asset backed by collateral that tracks the value of an external asset. Redeemable at any time with settlement.
    * EQUITY_ASSET,           ///< Asset issued by a business account that distributes a dividend from incoming revenue, and has voting power over a business accounts transactions.
    * BOND_ASSET,             ///< Asset backed by collateral that pays a coupon rate and is redeemed after expiration.
    * CREDIT_ASSET,           ///< Asset issued by a business account that is backed by repayments up to a face value, and interest payments.
    * STIMULUS_ASSET,         ///< Asset issued by a business account with expiring balances that is distributed to a set of accounts on regular intervals.
    * LIQUIDITY_POOL_ASSET,   ///< Asset that is backed by the deposits of an asset pair's liquidity pool and earns trading fees. 
    * CREDIT_POOL_ASSET,      ///< Asset that is backed by deposits of the base asset, used for borrowing funds from the pool, used as collateral to borrow base asset.
    * OPTION_ASSET,           ///< Asset that enables the execution of a trade at a specific strike price until an expiration date. 
    * PREDICTION_ASSET,       ///< Asset backed by an underlying collateral claim, on the condition that a prediction market resolve in a particular outcome.
    * UNIQUE_ASSET            ///< Asset with a supply of one, contains metadata relating to the ownership of a unique non-fungible asset.
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
         display_symbol(a), 
         details(a), 
         json(a), 
         url(a)
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

         flat_set< asset_symbol_type >   whitelist_markets;         ///< The assets that this asset may be traded against in the market.

         flat_set< asset_symbol_type >   blacklist_markets;         ///< The assets that this asset may not be traded against in the market.

         time_point                      created;                   ///< Time that the asset was created.

         time_point                      last_updated;              ///< Time that the asset details were last updated.

         bool is_market_issued()const                               ///< True if this is a market-issued asset; false otherwise. Market issued assets cannot be issued by the asset issuer.
         { 
            switch( asset_type )
            {
               case asset_property_type::STABLECOIN_ASSET:
               case asset_property_type::PREDICTION_ASSET:
               case asset_property_type::LIQUIDITY_POOL_ASSET:
               case asset_property_type::CREDIT_POOL_ASSET:
               case asset_property_type::OPTION_ASSET:
               case asset_property_type::STIMULUS_ASSET:
               {
                  return true;
               }
               default:
               {
                  return false;
               }
            }
         }

         bool is_credit_enabled()const                               ///< True if this asset can be used in credit pools, margin orders and credit loans.
         {
            if( enable_credit() )
            {
               switch( asset_type )
               {
                  case asset_property_type::STANDARD_ASSET:
                  case asset_property_type::CURRENCY_ASSET:
                  case asset_property_type::CREDIT_ASSET:
                  case asset_property_type::EQUITY_ASSET:
                  case asset_property_type::STABLECOIN_ASSET:
                  {
                     return true;
                  }
                  default:
                  {
                     return false;
                  }
               }
            }
            else
            {
               return false;
            }
         }

         bool is_liquid_enabled()const                               ///< True if this asset can be used in liquidity pools, and liquid exchange orders.
         {
            if( enable_liquid() )
            {
               switch( asset_type )
               {
                  case asset_property_type::STANDARD_ASSET:
                  case asset_property_type::CURRENCY_ASSET:
                  case asset_property_type::CREDIT_ASSET:
                  case asset_property_type::EQUITY_ASSET:
                  case asset_property_type::STABLECOIN_ASSET:
                  {
                     return true;
                  }
                  default:
                  {
                     return false;
                  }
               }
            }
            else
            {
               return false;
            }
         }

         bool is_option_enabled()const                               ///< True if this asset can be used in option pools, and option orders.
         {
            if( enable_options() )
            {
               switch( asset_type )
               {
                  case asset_property_type::STANDARD_ASSET:
                  case asset_property_type::CURRENCY_ASSET:
                  case asset_property_type::CREDIT_ASSET:
                  case asset_property_type::EQUITY_ASSET:
                  case asset_property_type::STABLECOIN_ASSET:
                  {
                     return true;
                  }
                  default:
                  {
                     return false;
                  }
               }
            }
            else
            {
               return false;
            }
         }

         bool is_unique_enabled()const                               ///< True if this asset can be used as a unique ownership asset
         {
            if( enable_unique() )
            {
               switch( asset_type )
               {
                  case asset_property_type::STANDARD_ASSET:
                  case asset_property_type::CURRENCY_ASSET:
                  case asset_property_type::EQUITY_ASSET:
                  case asset_property_type::UNIQUE_ASSET:
                  {
                     return true;
                  }
                  default:
                  {
                     return false;
                  }
               }
            }
            else
            {
               return false;
            }
         }

         bool is_temp_asset()const                                   ///< True if the asset is intended to be temporary, and can be cleared.
         {
            switch( asset_type )
            {
               case asset_property_type::BOND_ASSET:
               case asset_property_type::PREDICTION_ASSET:
               case asset_property_type::OPTION_ASSET:
               case asset_property_type::STIMULUS_ASSET:
               {
                  return true;
               }
               break;
               default:
               {
                  return false;
               }
            }
         }

         bool require_balance_whitelist()const   ///< true if Holders must be whitelisted
         { 
            return ( flags & int( asset_issuer_permission_flags::balance_whitelist ) );
         }

         bool require_trade_whitelist()const     ///< true if Traders must be whitelisted
         { 
            return ( flags & int( asset_issuer_permission_flags::trade_whitelist ) );
         }

         bool is_maker_restricted()const        ///< true if only the asset issuer can create new trade orders into the orderbook
         { 
            return ( flags & int( asset_issuer_permission_flags::maker_restricted ) );
         }

         bool issuer_accept_requests()const      ///< true if the asset issuer can accept transfer requests from any holders
         { 
            return ( flags & int( asset_issuer_permission_flags::issuer_accept_requests ) );
         }

         bool is_transfer_restricted()const     ///< true if this asset may only be transferred to/from the issuer or market orders
         { 
            return ( flags & int( asset_issuer_permission_flags::transfer_restricted ) );
         }

         bool can_request_transfer()const       ///< true if the asset can use transfer requests
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_requests ) );
         }

         bool can_recurring_transfer()const      ///< true if the asset can use recurring transfers
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_recurring ) );
         }

         bool enable_credit()const              ///< true if the asset can use credit pools, margin orders, and credit loans.
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_credit ) );
         }

         bool enable_liquid()const              ///< true if the asset can be included in liquidity pools and use liquidity pool orders.
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_liquid ) );
         }

         bool enable_options()const              ///< true if the asset can be included in option pools and generate option asset derivatives.
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_options ) );
         }

         bool enable_unique()const              ///< true if the asset can be used as a unique ownership asset
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_unique ) );
         }

         bool enable_escrow()const              ///< true if the asset can be used in escrow transfers and marketplace purchases
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_escrow ) );
         }

         bool enable_force_settle()const        ///< true if users may request force-settlement of the market-issued asset
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_force_settle ) );
         }

         bool enable_confidential()const        ///< true if the asset supports confidential transfers
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_confidential ) );
         }

         bool enable_auction()const            ///< true if the asset supports auction orders
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_auction ) );
         }

         bool is_producer_fed()const            ///< true if the top elected producers may produce price feeds for the market issued asset
         { 
            return ( flags & int( asset_issuer_permission_flags::producer_fed_asset ) );
         }

         bool can_global_settle()const         ///< true if the issuer of this market-issued asset may globally settle the stablecoin asset
         { 
            return !( flags & int( asset_issuer_permission_flags::disable_global_settle ) );
         }

         bool require_governance()const         ///< true if the governance account of the issuer must approve asset issuance and changes
         { 
            return ( flags & int( asset_issuer_permission_flags::governance_oversight ) );
         }

         bool immutable_properties()const       ///< true if the asset cannot be modified after creation
         { 
            return ( flags & int( asset_issuer_permission_flags::immutable_properties ) );
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

         share_type                     liquid_supply = 0;             ///< The current liquid supply of the asset.

         share_type                     staked_supply = 0;             ///< The current staked supply of the asset.

         share_type                     reward_supply = 0;             ///< The current reward supply of the asset.

         share_type                     savings_supply = 0;            ///< The current savings supply of the asset.

         share_type                     vesting_supply = 0;            ///< The current vesting supply of the asset.

         share_type                     delegated_supply = 0;          ///< The current delegated supply of the asset.
         
         share_type                     receiving_supply = 0;          ///< The current receiving supply supply of the asset, should equal delegated.

         share_type                     pending_supply = 0;            ///< The current supply contained in pending network objects and funds outside of account balances.

         share_type                     confidential_supply = 0;       ///< The current confidential asset supply.

         asset                          get_liquid_supply()const { return asset( liquid_supply, symbol ); }

         asset                          get_reward_supply()const { return asset( reward_supply, symbol ); }

         asset                          get_staked_supply()const { return asset( staked_supply, symbol ); }

         asset                          get_savings_supply()const { return asset( savings_supply, symbol ); }

         asset                          get_vesting_supply()const { return asset( vesting_supply, symbol ); }

         asset                          get_delegated_supply()const { return asset( delegated_supply, symbol ); }

         asset                          get_receiving_supply()const { return asset( receiving_supply, symbol ); }

         asset                          get_pending_supply()const { return asset( pending_supply, symbol ); }

         asset                          get_confidential_supply()const { return asset( confidential_supply, symbol ); }

         asset                          get_total_supply()const { return asset( ( liquid_supply + reward_supply + staked_supply + savings_supply + vesting_supply + pending_supply + confidential_supply ), symbol ); }

         asset                          get_account_balance_supply()const { return asset( ( liquid_supply + reward_supply + staked_supply + savings_supply ), symbol ); }

         void                           adjust_liquid_supply( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            liquid_supply += delta.amount;
            FC_ASSERT( liquid_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_staked_supply( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            staked_supply += delta.amount;
            FC_ASSERT( staked_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_reward_supply( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            reward_supply += delta.amount;
            FC_ASSERT( reward_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_savings_supply( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            savings_supply += delta.amount;
            FC_ASSERT( savings_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_vesting_supply( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            vesting_supply += delta.amount;
            FC_ASSERT( vesting_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_delegated_supply( const asset& delta )    ///< Not included in total supply
         { try {
            FC_ASSERT( delta.symbol == symbol );
            delegated_supply += delta.amount;
            FC_ASSERT( delegated_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_receiving_supply( const asset& delta )    ///< Not included in total supply
         { try {
            FC_ASSERT( delta.symbol == symbol );
            receiving_supply += delta.amount;
            FC_ASSERT( receiving_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_pending_supply( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            pending_supply += delta.amount;
            FC_ASSERT( pending_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          adjust_confidential_supply( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            confidential_supply += delta.amount;
            FC_ASSERT( confidential_supply >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                          clear_supply()     ///< Clears the entire asset supply, used for expiring assets - options, prediction assets
         {
            liquid_supply = 0;
            staked_supply = 0;
            reward_supply = 0;
            savings_supply = 0;
            vesting_supply = 0;
            delegated_supply = 0;
            receiving_supply = 0;
            pending_supply = 0;
            confidential_supply = 0;
         }
   };


   /**
    * Determines the Distribution Properties of Currency Assets.
    * 
    * Currency assets are distributed to network contributors, 
    * content creators, block producers.
    * 
    * Enables currency asset issuers to specify the
    * characteristics of the distribution of the cryptocurrency 
    * at creation, which cannot be altered afterwards.
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

         uint16_t                  enterprise_fund_reward_percent;      ///< Percentage of reward paid to community fund proposals.

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
    * Contains the current Pending Balance of Currency assets that are newly distributed.
    * 
    * Each Balance holds funds until they are allocated to an account
    * as a reward of the balances type.
    */
   class asset_reward_fund_object : public object< asset_reward_fund_object_type, asset_reward_fund_object >
   {
      asset_reward_fund_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_reward_fund_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         asset_symbol_type       symbol;                                                    ///< Currency symbol of the asset that the reward fund issues.

         asset                   content_reward_balance = asset( 0, symbol );               ///< Balance awaiting distribution to content creators.

         asset                   validation_reward_balance = asset( 0, symbol );            ///< Balance distributed to block validating producers.

         asset                   txn_stake_reward_balance = asset( 0, symbol );             ///< Balance distributed to block producers based on the stake weighted transactions in each block.

         asset                   work_reward_balance = asset( 0, symbol );                  ///< Balance distributed to each proof of work block producer.

         asset                   producer_activity_reward_balance = asset( 0, symbol );     ///< Balance distributed to producers that receive activity reward votes.

         asset                   supernode_reward_balance = asset( 0, symbol );             ///< Balance distributed to supernodes, based on stake weighted comment views.

         asset                   power_reward_balance = asset( 0, symbol );                 ///< Balance distributed to staked units of the currency.

         asset                   enterprise_fund_balance = asset( 0, symbol );              ///< Balance distributed to community proposal funds on the currency.

         asset                   development_reward_balance = asset( 0, symbol );           ///< Balance distributed to elected developers.

         asset                   marketing_reward_balance = asset( 0, symbol );             ///< Balance distributed to elected marketers.

         asset                   advocacy_reward_balance = asset( 0, symbol );              ///< Balance distributed to elected advocates.

         asset                   activity_reward_balance = asset( 0, symbol );              ///< Balance distributed to content creators that are active each day. 

         uint128_t               recent_content_claims = 0;                                 ///< Recently claimed content reward balance shares.

         uint128_t               recent_activity_claims = 0;                                ///< Recently claimed activity reward balance shares.

         time_point              last_updated;                                              ///< Time that the reward fund was last updated.

         void                    decay_recent_content_claims( time_point now, const median_chain_property_object& props )
         {
            uint128_t decay = ( recent_content_claims * ( now - last_updated ).count() ) / props.reward_curve.reward_duration().count();
            recent_content_claims -= decay;
            last_updated = now;
         }

         asset                   total_pending_reward_balance()const                        ///< Total of all reward balances. 
         {
            return content_reward_balance + validation_reward_balance + txn_stake_reward_balance + 
               work_reward_balance + activity_reward_balance + supernode_reward_balance + power_reward_balance + 
               enterprise_fund_balance + development_reward_balance + marketing_reward_balance + advocacy_reward_balance + 
               activity_reward_balance;
         }        

         void                    adjust_content_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            content_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_validation_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            validation_reward_balance += delta;
            FC_ASSERT( validation_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_txn_stake_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            txn_stake_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_work_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            work_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_producer_activity_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            producer_activity_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_supernode_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            supernode_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_power_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            power_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_enterprise_fund_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            enterprise_fund_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_development_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            development_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_marketing_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            marketing_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_advocacy_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            advocacy_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_activity_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol,
               "Symbol: ${s} Delta: ${d}",
               ("s",symbol)("d",delta.symbol));
            activity_reward_balance += delta;
            FC_ASSERT( content_reward_balance.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW() }
   };


   /**
    * Stablecoin Assets provide a fully collateralized instrument that tracks the price of an external asset.
    * 
    * Borrowers can lock collateral corresponding to at least 200% of the 
    * value of the position to issue new units of the stablecoin.
    * 
    * Stablecoin asset units do not present the holder with 
    * counterparty risk, and do not earn an interest rate of return.
    * 
    * Price feeds are used to determine the ratio of collateral 
    * required for creating call orders to create collateralized 
    * debt positions.
    *
    * In the event of a black swan, the swan price is saved
    * in the settlement price, and all margin positions
    * are settled at the same price with the collected collateral
    * being moved into the settlement fund. From this
    * point on, no further updates to the asset are permitted 
    * (no feeds, etc) and forced settlement occurs
    * immediately when requested, using the settlement price and fund.
    */
   class asset_stablecoin_data_object : public object < asset_stablecoin_data_object_type, asset_stablecoin_data_object>
   {
      asset_stablecoin_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_stablecoin_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                   id;

         asset_symbol_type         symbol;                                        ///< The symbol of the stablecoin that this object belongs to

         asset_symbol_type         backing_asset;                                 ///< The collateral backing asset of the stablecoin

         flat_map< account_name_type, pair< time_point, price_feed > >  feeds;    ///< Feeds published for this asset. 

         price_feed                current_feed;                                  ///< Currently active price feed, median of values from the currently active feeds.

         time_point                current_feed_publication_time;                 ///< Publication time of the oldest feed which was factored into current_feed.

         price                     current_maintenance_collateralization;         ///< Call orders with collateralization (aka collateral/debt) not greater than this value are in margin call territory.

         share_type                force_settled_volume;                          ///< This is the volume of this asset which has been force-settled this 24h interval
         
         price                     settlement_price;                              ///< Price at which force settlements of a black swanned asset will occur
         
         share_type                settlement_fund;                               ///< Amount of collateral which is available for force settlement

         fc::microseconds          feed_lifetime;                                 ///< Time before a price feed expires

         uint8_t                   minimum_feeds;                                 ///< Minimum number of unexpired feeds required to extract a median feed from

         fc::microseconds          asset_settlement_delay;                        ///< This is the delay between the time a long requests settlement and the chain evaluates the settlement

         uint16_t                  asset_settlement_offset_percent;               ///< The percentage to adjust the feed price in the short's favor in the event of a forced settlement

         uint16_t                  maximum_asset_settlement_volume;               ///< the percentage of current supply which may be force settled within each 24h interval.

         share_type                max_asset_settlement_volume( share_type total_supply )const     ///< Calculate the maximum force settlement volume per 24h interval, given the current share supply     
         {
            if( maximum_asset_settlement_volume == 0 )
            {
               return 0;
            }
            
            if( maximum_asset_settlement_volume == PERCENT_100 )
            {
               return total_supply + force_settled_volume;
            }

            fc::uint128 volume = total_supply.value + force_settled_volume.value;
            volume *= maximum_asset_settlement_volume;
            volume /= PERCENT_100;
            return share_type( volume.to_uint64());
         }

         bool                      has_settlement()const              ///< True if there has been a black swan
         {
            // ilog( "Checking Settlement price: ${p}",("p",settlement_price.to_string()));

            return !( settlement_price.is_null() );
         }   

         time_point                feed_expiration_time()const       ///< The time when current_feed would expire
         {
            return current_feed_publication_time + feed_lifetime;   
         }

         bool                      comment_feed_is_expired(time_point current_time)const
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
         void                       update_median_feeds( time_point current_time )
         {
            current_feed_publication_time = current_time;
            vector<std::reference_wrapper<const price_feed>> current_feeds;

            // find feeds that were alive at current_time
            for( const pair< account_name_type, pair< time_point, price_feed > >& f : feeds )
            {
               if( ( current_time - f.second.first ).to_seconds() < feed_lifetime.to_seconds() &&
                  f.second.first != time_point() )
               {
                  current_feeds.emplace_back(f.second.second);
                  current_feed_publication_time = std::min(current_feed_publication_time, f.second.first);
               }
            }

            // If there are no valid feeds, or the number available is less than the minimum to calculate a median...
            if( current_feeds.size() < minimum_feeds )
            {
               //... don't calculate a median, and set a null feed
               current_feed_publication_time = current_time;
               current_feed = price_feed();
               current_maintenance_collateralization = price();

               return;
            }
            if( current_feeds.size() == 1 )
            {
               current_feed = current_feeds.front();
               // Note: perhaps can defer updating current_maintenance_collateralization for better performance
               current_maintenance_collateralization = current_feed.maintenance_collateralization();
               return;
            }

            // *** Begin Median Calculations ***
            price_feed median_feed;
            const auto median_itr = current_feeds.begin() + current_feeds.size() / 2;
            
         #define CALCULATE_MEDIAN_VALUE(r, data, field_name) \
            std::nth_element( current_feeds.begin(), median_itr, current_feeds.end(), \
                              [](const price_feed& a, const price_feed& b) { \
               return a.field_name < b.field_name; \
            }); \
            median_feed.field_name = median_itr->get().field_name;

            BOOST_PP_SEQ_FOR_EACH( CALCULATE_MEDIAN_VALUE, ~, GRAPHENE_PRICE_FEED_FIELDS )
         #undef CALCULATE_MEDIAN_VALUE
            // *** End Median Calculations ***

            current_feed = median_feed;
            // Note: perhaps can defer updating current_maintenance_collateralization for better performance
            current_maintenance_collateralization = current_feed.maintenance_collateralization();
         }
   };


   /**
    * Tracks BitAssets scheduled for settlement.
    * 
    * On the settlement date the balance will be 
    * converted to the collateral asset and paid to the owner.
    */
   class asset_settlement_object : public object< asset_settlement_object_type, asset_settlement_object >
   {
      asset_settlement_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_settlement_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      owner;             ///< Name of the account that is force settling the asset.

         asset                  balance;           ///< Amount of debt asset being settled.

         time_point             settlement_date;   ///< Date of asset settlement for collateral.

         account_name_type      interface;         ///< The interface account that created the order.

         time_point             created;           ///< Time that the settlement was created.

         time_point             last_updated;      ///< Time that the settlement was last modified.

         asset_symbol_type      settlement_asset_symbol()const { return balance.symbol; }

         string                 to_string()const
         {
            return balance.to_string() + " - " + owner;
         }
   };


   /**
    * Collateral bids of collateral for debt after a black swan
    * There should only be one asset_collateral_bid_object per asset per account, and
    * only for smartcoin assets that have a global settlement_price.
    */
   class asset_collateral_bid_object : public object< asset_collateral_bid_object_type, asset_collateral_bid_object >
   {
      asset_collateral_bid_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_collateral_bid_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      bidder;           ///< Bidding Account name.

         asset                  collateral;       ///< Collateral bidded to obtain debt from a global settlement.

         asset                  debt;             ///< Debt requested for bid.

         time_point             created;          ///< Time that the bid was created.

         time_point             last_updated;     ///< Time that the bid was last adjusted.

         price                  inv_swan_price()const
         {
            return price( collateral, debt );     ///< Collateral / Debt.
         }

         asset_symbol_type      debt_type()const 
         { 
            return debt.symbol;
         }

         string                 to_string()const
         {
            return collateral.to_string() + " @ " + inv_swan_price().to_string() + " - " + bidder;
         }
   };


   /**
    * Equity assets enable a business account to raise equity capital.
    * 
    * Equity assets providing voting rights over the business account's 
    * leadership structure as to which accounts can use the authority
    * to conduct business management operations.
    * 
    * Equity asset holder also receive a weekly dividend from the incoming 
    * revenue flow to the issuing business account paid automatically.
    */
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

         account_name_type          issuer;                      ///< The business account name of the issuer.

         asset_symbol_type          symbol;                      ///< The symbol of the equity asset of the business.

         flat_map< asset_symbol_type, asset >   dividend_pool;   ///< Assets pooled for distribution at the next interval.
         
         time_point                 last_dividend;               ///< Time that the asset last distributed a dividend.

         uint16_t                   dividend_share_percent;      ///< Percentage of incoming assets added to the dividends pool.

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
    * Bond assets enable a business account to borrow funds for a fixed interest rate and duration.
    * 
    * Bond assets pay a specified coupon rate to the holders of the assets.
    * At the maturity time of the bond, it is settled for 
    * the underlying collateral value at the face value of the bond from
    * the account of the issuer. 
    * 
    * In the event that the issuer cannot pay the bond on maturity,
    * the collateral backing the bond asset is paid to the bondholders.
    * 
    * This asset represents a semi-collateralized loan 
    * from the market to the issuer with a fixed income and fixed duration.
    * 
    * The asset holds counterparty risk that its issuer may default, 
    * in exchange for a higher market rate of interest return.
    * the value of the Bond should flucuate based on the creditworthiness
    * of the issuing account. Users should not the publicly 
    * visible cash flow conditions of the issuer account before
    * purchasing the bond asset.
    * 
    * Because of the fixed interest rate, the value of the asset may diverge from the face value,
    * and most of the credit risk should be expressed in the market price of the bond.
    */
   class asset_bond_data_object : public object < asset_bond_data_object_type, asset_bond_data_object >
   {
      asset_bond_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_bond_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                          id;

         account_name_type                issuer;                    ///< The account name of the issuer. Locks collateral to issue new bond units.

         asset_symbol_type                symbol;                    ///< The symbol of the bond asset.

         asset                            value;                     ///< Face value amount of each unit of the bond. Interest is paid as a percentage of value.

         uint16_t                         collateralization;         ///< Percentage of value that is locked in collateral to back the bonds. Should be at least 10%.

         uint16_t                         coupon_rate_percent;       ///< Percentage rate of the value that is paid each month in interest to the holders.

         date_type                        maturity_date;             ///< Date at which the bond will mature. Principle value will be automatically paid from business.

         asset                            collateral_pool;           ///< Amount of collateral backing the bond assets. Distributed in case of default. 

         asset_symbol_type                collateral_asset()const { return value.symbol; }

         time_point                       maturity()const { return time_point( maturity_date ); }
   };


   /**
    * Credit assets enable business accounts to borrow funds from the market without requiring upfront collateral.
    * 
    * Credit assets are backed by future automatic repurchases up 
    * to a given buyback pricein response to incoming cash 
    * flows to the issuing business account. 
    * 
    * Accounts place repurchase orders from their buyback 
    * pool at the buyback price to maintain the asset's 
    * face value in normal market conditions.
    * 
    * Credit assets additionally pay interest to holders
    * by issuing new credit asset supply at an interest 
    * rate that linearly varies with the difference between the 
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
    * 
    * This asset represents an uncollateralized loan 
    * from the market to the issuer with a variable income and perpetual duration.
    * 
    * The asset holds counterparty risk that its issuer may default, 
    * in exchange for a higher market rate of interest return.
    * The value of the Credit should flucuate based on the creditworthiness
    * of the issuing account. Users should note the publicly 
    * visible cash flow conditions of the issuer account before
    * purchasing the credit asset.
    * 
    * Because of the variable interest rate, the value of the asset should closely approximate
    * the buyback asset value, and most of the credit risk should be expressed in the interest rate
    * instead of the market value.
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

         account_name_type      issuer;                          ///< The business account name of the issuer.

         asset_symbol_type      symbol;                          ///< The symbol of the credit asset of the business.

         asset_symbol_type      buyback_asset;                   ///< Symbol used to buyback credit assets.

         asset                  buyback_pool;                    ///< Amount of assets pooled to buyback the asset at next interval.

         price                  buyback_price;                   ///< Price at which the credit asset is bought back. Buyback asset is base.
         
         time_point             last_buyback;                    ///< Time that the last credit buyback occurred.

         uint16_t               buyback_share_percent;           ///< Percentage of incoming assets added to the buyback pool.

         uint16_t               liquid_fixed_interest_rate;      ///< Fixed component of Interest rate of the asset for liquid balances.

         uint16_t               liquid_variable_interest_rate;   ///< Variable component of Interest rate of the asset for liquid balances.

         uint16_t               staked_fixed_interest_rate;      ///< Fixed component of Interest rate of the asset for staked balances.

         uint16_t               staked_variable_interest_rate;   ///< Variable component of Interest rate of the asset for staked balances.

         uint16_t               savings_fixed_interest_rate;     ///< Fixed component of Interest rate of the asset for savings balances.

         uint16_t               savings_variable_interest_rate;  ///< Variable component of Interest rate of the asset for savings balances.

         uint16_t               var_interest_range;              ///< The percentage range from the buyback price over which to apply the variable interest rate.

         void                   adjust_pool( const asset& delta )
         {
            FC_ASSERT( delta.symbol == buyback_asset );
            buyback_pool += delta;
         }
   };


   /**
    * Stimulus Assets enable business accounts to accelerate the velocity of money in a market.
    * 
    * Stimulus assets are issued to all members of the distribution list once per month
    * and all balances expire at the end of each month.
    * 
    * Accounts can fund the redemption pool to provide value to all the beneficiaries of the 
    * asset in the redemption list and products to the distribution recipients.
    * 
    * The redemption pool acts as a subsidy for the products and services of the 
    * redemption list of accounts which can receive payments in the asset.
    * 
    * The redemption price is a specified rate at which members of the redemption list
    * can exchange the stimulus asset for the funds from the redemption pool.
    * 
    * Each month's distribution must be fully backed by the redemption pool.
    */
   class asset_stimulus_data_object : public object < asset_stimulus_data_object_type, asset_stimulus_data_object >
   {
      asset_stimulus_data_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_stimulus_data_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                              id;

         account_name_type                    issuer;                             ///< The business account name of the issuer.

         asset_symbol_type                    symbol;                             ///< The symbol of the stimulus asset.

         asset_symbol_type                    redemption_asset;                   ///< Symbol of the asset that can be redeemed in exchange the stimulus asset.

         asset                                redemption_pool;                    ///< Amount of assets pooled to redeem in exchange for the stimulus asset.

         price                                redemption_price;                   ///< Price at which the stimulus asset is redeemed. Redemption asset is base.
         
         flat_set< account_name_type >        distribution_list;                  ///< List of accounts that receive an equal balance of the stimulus asset.

         flat_set< account_name_type >        redemption_list;                    ///< List of accounts that can receive and redeem the stimulus asset.

         asset                                distribution_amount;                ///< Amount of stimulus asset distributed each interval.

         date_type                            next_distribution_date;             ///< Date that the next stimulus asset distribution will proceed.

         time_point                           expiration()const
         {
            return time_point( next_distribution_date );
         }

         void                                 adjust_pool( const asset& delta )
         {
            FC_ASSERT( delta.symbol == redemption_asset );
            redemption_pool += delta;
         }

         asset                                amount_to_distribute()const
         {
            share_type recipients = distribution_list.size();
            return asset( distribution_amount.amount * recipients, redemption_asset );
         }

         bool                                 is_redemption( const account_name_type& account )const
         {
            return std::find( redemption_list.begin(), redemption_list.end(), account ) != redemption_list.end();
         };

         bool                                 is_distribution( const account_name_type& account )const
         {
            return std::find( distribution_list.begin(), distribution_list.end(), account ) != distribution_list.end();
         };
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

         id_type                              id;

         asset_symbol_type                    symbol;                      ///< The symbol of the unique asset.

         account_name_type                    controlling_owner;           ///< Account that currently owns the asset. Controls the control list.

         asset_symbol_type                    ownership_asset;             ///< Asset that represents controlling ownership of the unique asset. Same as symbol for no liquid ownership asset.

         flat_set< account_name_type >        control_list;                ///< List of accounts that have control over access to the unique asset.

         flat_set< account_name_type >        access_list;                 ///< List of accounts that have access to the unique asset.

         asset                                access_price;                ///< Price per day for all accounts in the access list.

         share_type                           access_price_amount()const
         {
            return access_price.amount;
         }

         bool                                 is_control( const account_name_type& account )const  
         {
            return std::find( control_list.begin(), control_list.end(), account ) != control_list.end();
         };

         bool                                 is_access( const account_name_type& account )const  
         {
            return std::find( access_list.begin(), access_list.end(), account ) != access_list.end();
         }; 
   };


   /**
    * Allocates an asset according to the distribution properties.
    * 
    * The Amount of distribution is divided according to the amount
    * of assets sent to the distribution.
    * 
    * Distribution events continue for a specified amount of intervals
    * and specify an input unit and an output unit.
    * 
    * For every input unit contributed, a ratio amount of
    * output units are allocated to the contributing user.
    * 
    * The more input balances are sent, the lower the ratio becomes, until the
    * minimum ratio is reached, and no more funds can be contributed.
    */
   class asset_distribution_object : public object< asset_distribution_object_type, asset_distribution_object >
   {
      asset_distribution_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_distribution_object( Constructor&& c, allocator< Allocator > a ) :
            details(a), 
            url(a), 
            json(a)
            {
               c( *this );
            }

         id_type                                    id;

         asset_symbol_type                          distribution_asset;                    ///< Asset that is generated by the distribution.

         asset_symbol_type                          fund_asset;                            ///< Ticker symbol of the asset being accepted for distribution assets.

         shared_string                              details;                               ///< Description of the distribution process.

         shared_string                              url;                                   ///< Reference URL of the distribution process.

         shared_string                              json;                                  ///< JSON metadata of the distribution process.

         uint32_t                                   distribution_rounds;                   ///< Number of distribution rounds, total distribution amount is divided between all rounds.

         uint32_t                                   distribution_interval_days;            ///< Duration of each distribution round, in days.

         uint32_t                                   max_intervals_missed;                  ///< Number of Rounds that can be missed before the distribution is closed early.

         uint32_t                                   intervals_paid;                        ///< Number of Rounds that have been paid out to contributors.

         uint32_t                                   intervals_missed;                      ///< Number of Rounds that have been missed due to not reaching min input fund units.

         flat_set< asset_unit >                     input_fund_unit;                       ///< The integer unit ratio for distribution of incoming funds.

         flat_set< asset_unit >                     output_distribution_unit;              ///< The integer unit ratio for distribution of released funds.

         share_type                                 min_input_fund_units;                  ///< Soft Cap: Minimum fund units required for each round of the distribution process.

         share_type                                 max_input_fund_units;                  ///< Hard Cap: Maximum fund units to be accepted before closing each distribution round.

         share_type                                 min_unit_ratio;                        ///< The lowest value of unit ratio between input and output units.

         share_type                                 max_unit_ratio;                        ///< The highest possible initial value of unit ratio between input and output units.

         share_type                                 min_input_balance_units;               ///< Minimum fund units that each sender can contribute in an individual balance.

         share_type                                 max_input_balance_units;               ///< Maximum fund units that each sender can contribute in an individual balance.
         
         asset                                      total_distributed = asset( 0, distribution_asset ); ///< Amount of distribution asset generated and distributed to all fund balances.

         asset                                      total_funded = asset( 0, fund_asset ); ///< Amount of fund asset funded by all incoming fund balances.
         
         time_point                                 begin_time;                            ///< Time to begin the first distribution.

         time_point                                 next_round_time;                       ///< Time of the next distribution round.

         time_point                                 created;                               ///< Time that the distribution was created.

         time_point                                 last_updated;                          ///< Time that the distribution was last updated.

         /**
          * Minimum amount of output units that will be distributed per round.
          * [ min input fund units X max unit ratio ]
          */
         share_type                                 min_output_distribution_units()const
         {
            return min_input_fund_units * max_unit_ratio;
         }

         /**
          * Maximum amount of output units that will be distributed per round.
          * [ max input fund units X min unit ratio ]
          */
         share_type                                 max_output_distribution_units()const
         {
            return max_input_fund_units * min_unit_ratio;
         }

         asset                                      min_round_distribution()const
         {
            return asset( min_output_distribution_units() * output_unit_amount(), distribution_asset );
         }

         asset                                      max_round_distribution()const
         {
            return asset( max_output_distribution_units() * output_unit_amount(), distribution_asset );
         }

         asset                                      max_total_distribution()const
         {
            return max_round_distribution() * share_type( distribution_rounds );
         }

         asset                                      max_remaining_distribution()const
         {
            return max_round_distribution() * share_type( distribution_rounds - intervals_paid );
         }

         share_type                                 input_unit_amount()const
         {
            share_type input_sum = 0;
         
            for( auto a : input_fund_unit )
            {
               input_sum += a.units;
            }

            return input_sum;
         }

         share_type                                 output_unit_amount()const
         {
            share_type output_sum = 0;
         
            for( auto a : output_distribution_unit )
            {
               output_sum += a.units;
            }

            return output_sum;
         }
   };

   /**
    * Holds a balance of funds to be included in the next distribution interval.
    * 
    * Each interval time, all outstanding balances are divided 
    * between the recipients of the Distribution Input Fund Unit.
    */
   class asset_distribution_balance_object : public object< asset_distribution_balance_object_type, asset_distribution_balance_object >
   {
      asset_distribution_balance_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_distribution_balance_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                id;

         account_name_type                      sender;                  ///< Name of the account which sent the funds.

         asset_symbol_type                      distribution_asset;      ///< Asset that is generated by the distribution.

         asset                                  amount;                  ///< Amount of funding asset sent to the distribution, redeemed at next interval. Refunded if the soft cap is not reached.

         time_point                             created;                 ///< Time that the distribution balance was created.

         time_point                             last_updated;            ///< Time that the distribution balance was last updated.
   };
   
   struct by_symbol;
   struct by_type;
   struct by_issuer;
   struct by_interest;

   typedef multi_index_container<
      asset_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< asset_object, asset_id_type, &asset_object::id > >,
         ordered_unique< tag< by_symbol >,
            member<asset_object, asset_symbol_type, &asset_object::symbol> >,
         ordered_unique< tag< by_type >,
            composite_key< asset_object,
               const_mem_fun<asset_object, bool, &asset_object::is_market_issued>,
               member< asset_object, asset_id_type, &asset_object::id >
            >
         >,
         ordered_unique< tag< by_issuer >,
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
         ordered_unique< tag< by_id >, member< asset_dynamic_data_object, asset_dynamic_data_id_type, &asset_dynamic_data_object::id > >,
         ordered_unique< tag< by_symbol >, member<asset_dynamic_data_object, asset_symbol_type, &asset_dynamic_data_object::symbol> >
      >, 
      allocator< asset_dynamic_data_object >
   > asset_dynamic_data_index;

   typedef multi_index_container<
      asset_currency_data_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_currency_data_object, asset_currency_data_id_type, &asset_currency_data_object::id > >,
         ordered_unique< tag< by_symbol >, member<asset_currency_data_object, asset_symbol_type, &asset_currency_data_object::symbol> >
      >, 
      allocator< asset_currency_data_object >
   > asset_currency_data_index;


   struct by_name;
   struct by_symbol;


   typedef multi_index_container<
      asset_reward_fund_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< asset_reward_fund_object, asset_reward_fund_id_type, &asset_reward_fund_object::id > >,
         ordered_unique< tag< by_symbol >,
            member< asset_reward_fund_object, asset_symbol_type, &asset_reward_fund_object::symbol > >
      >,
      allocator< asset_reward_fund_object >
   > asset_reward_fund_index;


   struct by_backing_asset;
   struct by_feed_expiration;


   typedef multi_index_container<
      asset_stablecoin_data_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_stablecoin_data_object, asset_stablecoin_data_id_type, &asset_stablecoin_data_object::id > >,
         ordered_unique< tag< by_symbol >, member< asset_stablecoin_data_object, asset_symbol_type, &asset_stablecoin_data_object::symbol > >,
         ordered_non_unique< tag< by_backing_asset >, 
            member< asset_stablecoin_data_object, asset_symbol_type, &asset_stablecoin_data_object::backing_asset > 
         >,
         ordered_unique< tag< by_feed_expiration >,
            composite_key< asset_stablecoin_data_object,
               const_mem_fun< asset_stablecoin_data_object, time_point, &asset_stablecoin_data_object::feed_expiration_time >,
               member< asset_stablecoin_data_object, asset_stablecoin_data_id_type, &asset_stablecoin_data_object::id >
            >
         >
      >,
      allocator< asset_stablecoin_data_object >
   > asset_stablecoin_data_index;


   struct by_maturity;


   typedef multi_index_container<
      asset_bond_data_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_bond_data_object, asset_bond_data_id_type, &asset_bond_data_object::id > >,
         ordered_unique< tag< by_symbol >, member< asset_bond_data_object, asset_symbol_type, &asset_bond_data_object::symbol > >,
         ordered_non_unique< tag< by_backing_asset >, 
            const_mem_fun< asset_bond_data_object, asset_symbol_type, &asset_bond_data_object::collateral_asset > 
         >,
         ordered_unique< tag< by_maturity >,
            composite_key< asset_bond_data_object,
               const_mem_fun< asset_bond_data_object, time_point, &asset_bond_data_object::maturity >,
               member< asset_bond_data_object, asset_bond_data_id_type, &asset_bond_data_object::id >
            >
         >
      >,
      allocator< asset_bond_data_object >
   > asset_bond_data_index;

   struct by_expiration;
   struct by_account_asset;
   
   typedef multi_index_container<
      asset_settlement_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_settlement_object, asset_settlement_id_type, &asset_settlement_object::id > >,
         ordered_unique< tag< by_account_asset >,
            composite_key< asset_settlement_object,
               member< asset_settlement_object, account_name_type, &asset_settlement_object::owner >,
               const_mem_fun< asset_settlement_object, asset_symbol_type, &asset_settlement_object::settlement_asset_symbol >,
               member< asset_settlement_object, asset_settlement_id_type, &asset_settlement_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< asset_symbol_type >,
               std::less< asset_settlement_id_type >
            >
         >,
         ordered_unique< tag< by_expiration >,
            composite_key< asset_settlement_object,
               const_mem_fun< asset_settlement_object, asset_symbol_type, &asset_settlement_object::settlement_asset_symbol >,
               member< asset_settlement_object, time_point, &asset_settlement_object::settlement_date >,
               member< asset_settlement_object, asset_settlement_id_type, &asset_settlement_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< time_point >,
               std::less< asset_settlement_id_type >
            >
         >
      >,
      allocator < asset_settlement_object >
   > asset_settlement_index;

   struct by_price;
   struct by_account;

   typedef multi_index_container<
      asset_collateral_bid_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< asset_collateral_bid_object, asset_collateral_bid_id_type, &asset_collateral_bid_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< asset_collateral_bid_object,
               member< asset_collateral_bid_object, account_name_type, &asset_collateral_bid_object::bidder >,
               const_mem_fun< asset_collateral_bid_object, asset_symbol_type, &asset_collateral_bid_object::debt_type >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< asset_symbol_type >
            >
         >,
         ordered_unique< tag< by_price >,
            composite_key< asset_collateral_bid_object,
               const_mem_fun< asset_collateral_bid_object, asset_symbol_type, &asset_collateral_bid_object::debt_type >,
               const_mem_fun< asset_collateral_bid_object, price, &asset_collateral_bid_object::inv_swan_price >,
               member< asset_collateral_bid_object, asset_collateral_bid_id_type, &asset_collateral_bid_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::greater< price >,
               std::less< asset_collateral_bid_id_type >
            >
         >
      >,
      allocator < asset_collateral_bid_object >
   > asset_collateral_bid_index;


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
      asset_stimulus_data_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_stimulus_data_object, asset_stimulus_data_id_type, &asset_stimulus_data_object::id > >,
         ordered_unique< tag< by_symbol >, member<asset_stimulus_data_object, asset_symbol_type, &asset_stimulus_data_object::symbol> >,
         ordered_unique< tag< by_expiration >,
            composite_key< asset_stimulus_data_object,
               const_mem_fun< asset_stimulus_data_object, time_point, &asset_stimulus_data_object::expiration >,
               member< asset_stimulus_data_object, asset_stimulus_data_id_type, &asset_stimulus_data_object::id >
            >
         >
      >,
      allocator< asset_stimulus_data_object >
   > asset_stimulus_data_index;


   struct by_access_price;

   typedef multi_index_container<
      asset_unique_data_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_unique_data_object, asset_unique_data_id_type, &asset_unique_data_object::id > >,
         ordered_unique< tag< by_symbol >, member<asset_unique_data_object, asset_symbol_type, &asset_unique_data_object::symbol > >,
         ordered_unique< tag< by_access_price >,
            composite_key< asset_unique_data_object,
               const_mem_fun<asset_unique_data_object, share_type, &asset_unique_data_object::access_price_amount >,
               member< asset_unique_data_object, asset_unique_data_id_type, &asset_unique_data_object::id >
            >,
            composite_key_compare< 
               std::greater< share_type >,
               std::less< asset_unique_data_id_type >
            >
         >
      >,
      allocator< asset_unique_data_object >
   > asset_unique_data_index;

   
   struct by_symbol;
   struct by_fund_symbol;
   struct by_begin_time;
   struct by_next_round_time;


   typedef multi_index_container<
      asset_distribution_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_distribution_object, asset_distribution_id_type, &asset_distribution_object::id > >,
         ordered_unique< tag< by_symbol >, member< asset_distribution_object, asset_symbol_type, &asset_distribution_object::distribution_asset > >,
         ordered_unique< tag< by_fund_symbol >,
            composite_key< asset_distribution_object,
               member< asset_distribution_object, asset_symbol_type, &asset_distribution_object::fund_asset >,
               member< asset_distribution_object, asset_distribution_id_type, &asset_distribution_object::id > 
            >
         >,
         ordered_unique< tag< by_begin_time >,
            composite_key< asset_distribution_object,
               member< asset_distribution_object, time_point, &asset_distribution_object::begin_time >,
               member< asset_distribution_object, asset_distribution_id_type, &asset_distribution_object::id >
            >
         >,
         ordered_unique< tag< by_next_round_time >,
            composite_key< asset_distribution_object,
               member< asset_distribution_object, time_point, &asset_distribution_object::next_round_time >,
               member< asset_distribution_object, asset_distribution_id_type, &asset_distribution_object::id >
            >
         >
      >, 
      allocator< asset_distribution_object >
   > asset_distribution_index;


   struct by_account_distribution;
   struct by_distribution_account;


   typedef multi_index_container<
      asset_distribution_balance_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_distribution_balance_object, asset_distribution_balance_id_type, &asset_distribution_balance_object::id > >,
         ordered_unique< tag< by_account_distribution >,
            composite_key< asset_distribution_balance_object,
               member< asset_distribution_balance_object, account_name_type, &asset_distribution_balance_object::sender >,
               member< asset_distribution_balance_object, asset_symbol_type, &asset_distribution_balance_object::distribution_asset > 
            >
         >,
         ordered_unique< tag< by_distribution_account >,
            composite_key< asset_distribution_balance_object,
               member< asset_distribution_balance_object, asset_symbol_type, &asset_distribution_balance_object::distribution_asset >,
               member< asset_distribution_balance_object, account_name_type, &asset_distribution_balance_object::sender >
            >
         >
      >, 
      allocator< asset_distribution_balance_object >
   > asset_distribution_balance_index;


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
         (liquid_supply)
         (staked_supply)
         (reward_supply)
         (savings_supply)
         (vesting_supply)
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
         (enterprise_fund_reward_percent)
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

FC_REFLECT( node::chain::asset_reward_fund_object,
         (id)
         (symbol)
         (content_reward_balance)
         (validation_reward_balance)
         (txn_stake_reward_balance)
         (work_reward_balance)
         (producer_activity_reward_balance)
         (supernode_reward_balance)
         (power_reward_balance)
         (enterprise_fund_balance)
         (development_reward_balance)
         (marketing_reward_balance)
         (advocacy_reward_balance)
         (activity_reward_balance)
         (recent_content_claims)
         (recent_activity_claims)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_reward_fund_object, node::chain::asset_reward_fund_index );

FC_REFLECT( node::chain::asset_stablecoin_data_object,
         (id)
         (symbol)
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
         (asset_settlement_delay)
         (asset_settlement_offset_percent)
         (maximum_asset_settlement_volume)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_stablecoin_data_object, node::chain::asset_stablecoin_data_index );

FC_REFLECT( node::chain::asset_settlement_object, 
         (id)
         (owner)
         (balance)
         (settlement_date)
         (interface)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_settlement_object, node::chain::asset_settlement_index );

FC_REFLECT( node::chain::asset_collateral_bid_object, 
         (id)
         (bidder)
         (collateral)
         (debt)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_collateral_bid_object, node::chain::asset_collateral_bid_index );

FC_REFLECT( node::chain::asset_equity_data_object,
         (id)
         (issuer)
         (symbol)
         (dividend_pool)
         (last_dividend)
         (dividend_share_percent)
         (min_active_time)
         (min_balance)
         (min_producers)
         (boost_balance)
         (boost_activity)
         (boost_producers)
         (boost_top)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_equity_data_object, node::chain::asset_equity_data_index );

FC_REFLECT( node::chain::asset_bond_data_object,
         (id)
         (issuer)
         (symbol)
         (value)
         (collateralization)
         (coupon_rate_percent)
         (maturity_date)
         (collateral_pool)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_bond_data_object, node::chain::asset_bond_data_index );

FC_REFLECT( node::chain::asset_credit_data_object,
         (id)
         (issuer)
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

FC_REFLECT( node::chain::asset_stimulus_data_object,
         (id)
         (issuer)
         (symbol)
         (redemption_asset)
         (redemption_pool)
         (redemption_price)
         (distribution_list)
         (redemption_list)
         (distribution_amount)
         (next_distribution_date)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_stimulus_data_object, node::chain::asset_stimulus_data_index );

FC_REFLECT( node::chain::asset_unique_data_object,
         (id)
         (symbol)
         (controlling_owner)
         (ownership_asset)
         (control_list)
         (access_list)
         (access_price)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_unique_data_object, node::chain::asset_unique_data_index );

FC_REFLECT( node::chain::asset_distribution_object,
         (id)
         (distribution_asset)
         (fund_asset)
         (details)
         (url)
         (json)
         (distribution_rounds)
         (distribution_interval_days)
         (max_intervals_missed)
         (intervals_paid)
         (intervals_missed)
         (input_fund_unit)
         (output_distribution_unit)
         (min_input_fund_units)
         (max_input_fund_units)
         (min_unit_ratio)
         (max_unit_ratio)
         (min_input_balance_units)
         (max_input_balance_units)
         (total_distributed)
         (total_funded)
         (begin_time)
         (next_round_time)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_distribution_object, node::chain::asset_distribution_index );

FC_REFLECT( node::chain::asset_distribution_balance_object,
         (id)
         (sender)
         (distribution_asset)
         (amount)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_distribution_balance_object, node::chain::asset_distribution_balance_index );