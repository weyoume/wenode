#pragma once

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/block.hpp>
#include <node/chain/node_object_types.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <numeric>

namespace node { namespace chain {
   
   using node::protocol::asset;
   using node::protocol::price;
   using node::protocol::price_feed;
   using node::protocol::option_strike;
   using node::protocol::asset_unit;
   using node::protocol::authority;
   using node::protocol::signed_transaction;
   using node::protocol::operation;

   using node::protocol::comment_reward_curve;
   using node::protocol::chain_properties;
   using node::protocol::digest_type;
   using node::protocol::public_key_type;
   using node::protocol::version;
   using node::protocol::hardfork_version;

   using node::protocol::x11;

   using node::protocol::share_type;
   using node::protocol::ratio_type;

   using node::protocol::block_id_type;
   using node::protocol::transaction_id_type;
   using node::protocol::chain_id_type;

   using node::protocol::account_name_type;
   using node::protocol::community_name_type;
   using node::protocol::tag_name_type;
   using node::protocol::asset_symbol_type;
   using node::protocol::graph_node_name_type;
   using node::protocol::graph_edge_name_type;
   using node::protocol::fixed_string_32;
   
   using node::protocol::encrypted_keypair_type;
   using node::protocol::date_type;
  
   using node::protocol::beneficiary_route_type;
   using node::protocol::executive_officer_set;
   using chainbase::shared_vector;

   using node::protocol::community_privacy_type;
   using node::protocol::community_federation_type;
   using node::protocol::business_structure_type;
   using node::protocol::membership_tier_type;
   using node::protocol::network_officer_role_type;
   using node::protocol::executive_role_type;
   using node::protocol::product_auction_type;
   using node::protocol::asset_property_type;
   using node::protocol::ad_format_type;
   using node::protocol::post_format_type;
   using node::protocol::ad_metric_type;
   using node::protocol::connection_tier_type;
   using node::protocol::feed_reach_type;
   using node::protocol::blog_reach_type;
   using node::protocol::sort_time_type;
   using node::protocol::sort_option_type;
   using node::protocol::post_time_type;
   using node::protocol::asset_issuer_permission_flags;
   using node::protocol::community_permission_flags;


   /**
    * Set of Network parameters that are selected by block producers.
    * 
    * Producers vote on how to set certain chain properties
    * to ensure a smooth and well functioning network, 
    * and can be responsive to changing network conditions.
    * 
    * The active set of producers will be used to
    * control the blockchain configuration by
    * selecting the median value of all properties listed.
    */
   class median_chain_property_object : public object< median_chain_property_object_type, median_chain_property_object>
   {
      median_chain_property_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         median_chain_property_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;

         comment_reward_curve   reward_curve = comment_reward_curve();                         ///< The Parameters defining the content reward distribution curve.

         asset                  account_creation_fee = MIN_ACCOUNT_CREATION_FEE;               ///< Minimum fee required to create a new account by staking.

         asset                  asset_coin_liquidity = MIN_ASSET_COIN_LIQUIDITY;               ///< Minimum COIN required to create a new asset.

         asset                  asset_usd_liquidity = MIN_ASSET_USD_LIQUIDITY;                 ///< Minimum USD required to create a new asset by.

         uint64_t               maximum_block_size = MAX_BLOCK_SIZE;                           ///< The maximum block size of the network in bytes. No Upper bound on block size limit.

         fc::microseconds       pow_target_time = POW_TARGET_TIME;                             ///< The targeted time for each proof of work

         fc::microseconds       pow_decay_time = POW_DECAY_TIME;                               ///< Time over which proof of work output is averaged over

         fc::microseconds       txn_stake_decay_time = TXN_STAKE_DECAY_TIME;                   ///< Time over which transaction stake is averaged over

         uint16_t               escrow_bond_percent = ESCROW_BOND_PERCENT;                     ///< Percentage of an escrow transfer that is deposited for dispute resolution

         uint16_t               credit_interest_rate = CREDIT_INTEREST_RATE;                   ///< The credit interest rate paid to holders of network credit assets.

         uint16_t               credit_open_ratio = CREDIT_OPEN_RATIO;                         ///< The minimum required collateralization ratio for a credit loan to be opened. 

         uint16_t               credit_liquidation_ratio = CREDIT_LIQUIDATION_RATIO;           ///< The minimum permissible collateralization ratio before a loan is liquidated. 

         uint16_t               credit_min_interest = CREDIT_MIN_INTEREST;                     ///< The minimum component of credit pool interest rates. 

         uint16_t               credit_variable_interest = CREDIT_VARIABLE_INTEREST;           ///< The variable component of credit pool interest rates, applied at equal base and borrowed balances.

         uint16_t               market_max_credit_ratio = MARKET_MAX_CREDIT_RATIO;             ///< The maximum percentage of core asset liquidity balances that can be loaned.

         uint16_t               margin_open_ratio = MARGIN_OPEN_RATIO;                         ///< The minimum required collateralization ratio for a credit loan to be opened. 

         uint16_t               margin_liquidation_ratio = MARGIN_LIQUIDATION_RATIO;           ///< The minimum permissible collateralization ratio before a loan is liquidated. 

         uint16_t               maximum_asset_feed_publishers = MAX_ASSET_FEED_PUBLISHERS;     ///< The maximum number of accounts that can publish price feeds for a stablecoin.

         asset                  membership_base_price = MEMBERSHIP_FEE_BASE;                   ///< The price for standard membership per month.

         asset                  membership_mid_price = MEMBERSHIP_FEE_MID;                     ///< The price for Mezzanine membership per month.

         asset                  membership_top_price = MEMBERSHIP_FEE_TOP;                     ///< The price for top level membership per month.

         uint32_t               vote_reserve_rate = VOTE_RESERVE_RATE;                         ///< The number of votes regenerated per day.

         uint32_t               view_reserve_rate = VIEW_RESERVE_RATE;                         ///< The number of views regenerated per day.

         uint32_t               share_reserve_rate = SHARE_RESERVE_RATE;                       ///< The number of shares regenerated per day.

         uint32_t               comment_reserve_rate = COMMENT_RESERVE_RATE;                   ///< The number of comments regenerated per day.

         fc::microseconds       vote_recharge_time = VOTE_RECHARGE_TIME;                       ///< Time taken to fully recharge voting power.

         fc::microseconds       view_recharge_time = VIEW_RECHARGE_TIME;                       ///< Time taken to fully recharge viewing power.

         fc::microseconds       share_recharge_time = SHARE_RECHARGE_TIME;                     ///< Time taken to fully recharge sharing power.

         fc::microseconds       comment_recharge_time = COMMENT_RECHARGE_TIME;                 ///< Time taken to fully recharge commenting power.

         fc::microseconds       curation_auction_decay_time = CURATION_AUCTION_DECAY_TIME;     ///< time of curation reward decay after a post is created. 

         double                 vote_curation_decay = VOTE_CURATION_DECAY;                     ///< Number of votes for the half life of voting curation reward decay.

         double                 view_curation_decay = VIEW_CURATION_DECAY;                     ///< Number of views for the half life of viewer curation reward decay.

         double                 share_curation_decay = SHARE_CURATION_DECAY;                   ///< Number of shares for the half life of sharing curation reward decay.

         double                 comment_curation_decay = COMMENT_CURATION_DECAY;               ///< Number of comments for the half life of comment curation reward decay.

         fc::microseconds       supernode_decay_time = SUPERNODE_DECAY_TIME;                   ///< Amount of time to average the supernode file weight over. 

         uint16_t               enterprise_vote_percent_required = ENTERPRISE_VOTE_THRESHOLD_PERCENT;     ///< Percentage of total voting power required to approve enterprise milestones. 

         uint64_t               maximum_asset_whitelist_authorities = MAX_ASSET_WHITELIST_AUTHORITIES;  ///< The maximum amount of whitelisted or blacklisted authorities for user issued assets 

         uint8_t                max_stake_intervals = MAX_ASSET_STAKE_INTERVALS;               ///< Maximum weeks that an asset can stake over.

         uint8_t                max_unstake_intervals = MAX_ASSET_UNSTAKE_INTERVALS;           ///< Maximum weeks that an asset can unstake over.

         asset                  max_exec_budget = MAX_EXEC_BUDGET;                             ///< Maximum budget that an executive board can claim.
   };


   typedef multi_index_container<
      median_chain_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< median_chain_property_object, median_chain_property_id_type, &median_chain_property_object::id > >
      >,
      allocator< median_chain_property_object >
   > median_chain_property_index;


} } // node::chain


#include <node/chain/producer_objects.hpp>
#include <node/chain/network_object.hpp>
#include <node/chain/transfer_object.hpp>
#include <node/chain/balance_object.hpp>
#include <node/chain/asset_object.hpp>
#include <node/chain/trading_object.hpp>
#include <node/chain/pool_object.hpp>
#include <node/chain/comment_object.hpp>
#include <node/chain/account_object.hpp>
#include <node/chain/community_object.hpp>
#include <node/chain/ad_object.hpp>
#include <node/chain/graph_object.hpp>
#include <node/chain/marketplace_object.hpp>


FC_REFLECT( node::chain::median_chain_property_object,
         (id)
         (reward_curve)
         (account_creation_fee)
         (asset_coin_liquidity)
         (asset_usd_liquidity)
         (maximum_block_size)
         (pow_target_time)
         (pow_decay_time)
         (txn_stake_decay_time)
         (escrow_bond_percent)
         (credit_interest_rate)
         (credit_open_ratio)
         (credit_liquidation_ratio)
         (credit_min_interest)
         (credit_variable_interest)
         (market_max_credit_ratio)
         (margin_open_ratio)
         (margin_liquidation_ratio)
         (maximum_asset_feed_publishers)
         (membership_base_price)
         (membership_mid_price)
         (membership_top_price)
         (vote_reserve_rate)
         (view_reserve_rate)
         (share_reserve_rate)
         (comment_reserve_rate)
         (vote_recharge_time)
         (view_recharge_time)
         (share_recharge_time)
         (comment_recharge_time)
         (curation_auction_decay_time)
         (vote_curation_decay)
         (view_curation_decay)
         (share_curation_decay)
         (comment_curation_decay)
         (supernode_decay_time)
         (enterprise_vote_percent_required)
         (maximum_asset_whitelist_authorities)
         (max_stake_intervals)
         (max_unstake_intervals)
         (max_exec_budget)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::median_chain_property_object, node::chain::median_chain_property_index );