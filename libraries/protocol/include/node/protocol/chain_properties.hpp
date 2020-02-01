#pragma once
#include <fc/uint128.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/protocol/types.hpp>
#include <node/protocol/config.hpp>
#include <node/protocol/asset.hpp>

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
namespace node { namespace protocol {

   struct chain_properties
   {
      asset                  account_creation_fee = MIN_ACCOUNT_CREATION_FEE;               ///< Minimum fee required to create a new account by staking.

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

      uint16_t               maximum_asset_feed_publishers = MAX_ASSET_FEED_PUBLISHERS;     ///< The maximum number of accounts that can publish price feeds for a bitasset.

      asset                  membership_base_price = MEMBERSHIP_FEE_BASE;                   ///< The price for standard membership per month.

      asset                  membership_mid_price = MEMBERSHIP_FEE_MID;                     ///< The price for Mezzanine membership per month.

      asset                  membership_top_price = MEMBERSHIP_FEE_TOP;                     ///< The price for top level membership per month.

      uint32_t               author_reward_percent = AUTHOR_REWARD_PERCENT;                 ///< The percentage of content rewards distributed to post authors.

      uint32_t               vote_reward_percent = VOTE_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post voters.

      uint32_t               view_reward_percent = VIEW_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post viewers.

      uint32_t               share_reward_percent = SHARE_REWARD_PERCENT;                   ///< The percentage of content rewards distributed to post sharers.

      uint32_t               comment_reward_percent = COMMENT_REWARD_PERCENT;               ///< The percentage of content rewards distributed to post commenters.

      uint32_t               storage_reward_percent = STORAGE_REWARD_PERCENT;               ///< The percentage of content rewards distributed to viewing supernodes.

      uint32_t               moderator_reward_percent = MODERATOR_REWARD_PERCENT;           ///< The percentage of content rewards distributed to board moderators.

      fc::microseconds       content_reward_decay_rate = CONTENT_REWARD_DECAY_RATE;         ///< The time over which content rewards are distributed

      fc::microseconds       content_reward_interval = CONTENT_REWARD_INTERVAL;             ///< Time taken per distribution of content rewards.

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

      uint16_t               enterprise_vote_percent_required = VOTE_THRESHOLD_PERCENT;     ///< Percentage of total voting power required to approve enterprise milestones. 

      uint64_t               maximum_asset_whitelist_authorities = MAX_ASSET_WHITELIST_AUTHORITIES;  ///< The maximum amount of whitelisted or blacklisted authorities for user issued assets 

      uint8_t                max_stake_intervals = MAX_ASSET_STAKE_INTERVALS;               ///< Maximum weeks that an asset can stake over.

      uint8_t                max_unstake_intervals = MAX_ASSET_UNSTAKE_INTERVALS;           ///< Maximum weeks that an asset can unstake over.

      asset                  max_exec_budget = MAX_EXEC_BUDGET;                             ///< Maximum budget that an executive board can claim.

      void validate()const
      {
         FC_ASSERT( account_creation_fee.symbol == SYMBOL_COIN,
            "Acccount creation fee must be in the core asset." );
         FC_ASSERT( account_creation_fee >= MIN_ACCOUNT_CREATION_FEE,
            "Account creation fee must be at least 1 Unit of core asset." );
         FC_ASSERT( maximum_block_size >= MIN_BLOCK_SIZE_LIMIT,
            "Maximum blocksize must be greater than minimum limit requirement." );
         FC_ASSERT( pow_target_time >= fc::minutes(1) && pow_target_time <= fc::hours(1),
            "POW target time must be between 1 minute and 1 hour." );
         FC_ASSERT( pow_decay_time >= fc::days(1) && pow_decay_time <= fc::days(30),
            "POW Decay time must be between 1 and 30 days." );
         FC_ASSERT( txn_stake_decay_time >= fc::days(1) && txn_stake_decay_time <= fc::days(30),
            "Transaction Stake Decay time must be between 1 and 30 days." );
         FC_ASSERT( escrow_bond_percent >= 0 && escrow_bond_percent <= PERCENT_100,
            "Credit interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( credit_interest_rate >= 0 && credit_interest_rate <= PERCENT_100,
            "Credit interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( credit_open_ratio >= PERCENT_100 && credit_open_ratio <= PERCENT_100 * 2,
            "Credit interest rate must be PERCENT_100 and 2 * PERCENT_100." );
         FC_ASSERT( credit_liquidation_ratio >= PERCENT_100 && credit_liquidation_ratio <= PERCENT_100 * 2,
            "Credit interest rate must be PERCENT_100 and 2 * PERCENT_100." );
         FC_ASSERT( credit_min_interest >= 0 && credit_min_interest <= PERCENT_100,
            "Credit min interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( credit_variable_interest >= 0 && credit_variable_interest <= PERCENT_100,
            "Credit variable interest rate must be between 0 and PERCENT_100." );
         FC_ASSERT( market_max_credit_ratio >= 0 && market_max_credit_ratio <= PERCENT_100,
            "Market max credit ratio must be between 0 and PERCENT_100." );
         FC_ASSERT( margin_open_ratio >= PERCENT_1 && margin_open_ratio <= PERCENT_100,
            "Margin Open Ratio must be between PERCENT_1 and PERCENT_100." );
         FC_ASSERT( margin_liquidation_ratio >= PERCENT_1 && margin_liquidation_ratio <= PERCENT_100,
            "Margin Liquidation Ratio must be between PERCENT_1 and PERCENT_100." );
         FC_ASSERT( maximum_asset_feed_publishers >= MAX_ASSET_FEED_PUBLISHERS / 10 &&
            maximum_asset_feed_publishers <= MAX_ASSET_FEED_PUBLISHERS * 10,
            "Maximum asset feed publishers must be between 10 and 1000." );
         FC_ASSERT( membership_base_price >= MEMBERSHIP_FEE_BASE / 25 && membership_base_price <= MEMBERSHIP_FEE_BASE * 100,
            "Membership base price must be between $0.10 and $250.00." );
         FC_ASSERT( membership_base_price.symbol == SYMBOL_USD,
            "Membership base price must be in the USD asset." );
         FC_ASSERT( membership_mid_price >= MEMBERSHIP_FEE_MID / 25 && membership_mid_price <= MEMBERSHIP_FEE_MID * 100,
            "Membership mid price must be between $1.00 and $2500.00." );
         FC_ASSERT( membership_mid_price.symbol == SYMBOL_USD,
            "Membership mid price must be in the USD asset." );
         FC_ASSERT( membership_top_price >= MEMBERSHIP_FEE_TOP / 25 && membership_top_price <= MEMBERSHIP_FEE_TOP * 100,
            "Membership top price must be between $10.00 and $25000.00." );
         FC_ASSERT( membership_top_price.symbol == SYMBOL_USD,
            "Membership top price must be in the USD asset." );

         FC_ASSERT( vote_reward_percent >= PERCENT_10_OF_PERCENT_1 && vote_reward_percent <= 20 * PERCENT_1,
            "Vote reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( view_reward_percent >= PERCENT_10_OF_PERCENT_1 && view_reward_percent <= 20 * PERCENT_1,
            "View reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( share_reward_percent >= PERCENT_10_OF_PERCENT_1 && share_reward_percent <= 20 * PERCENT_1,
            "Share reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( comment_reward_percent >= PERCENT_10_OF_PERCENT_1 && comment_reward_percent <= 20 * PERCENT_1,
            "Comment reward percent must be between PERCENT_10_OF_PERCENT_1 and 20 * PERCENT_1." );
         FC_ASSERT( storage_reward_percent >= PERCENT_10_OF_PERCENT_1 && storage_reward_percent <= 10 * PERCENT_1,
            "Storage reward percent must be between PERCENT_10_OF_PERCENT_1 and 10 * PERCENT_1." );
         FC_ASSERT( moderator_reward_percent >= PERCENT_10_OF_PERCENT_1 && moderator_reward_percent <= 10 * PERCENT_1,
            "Moderator reward percent must be between PERCENT_10_OF_PERCENT_1 and 10 * PERCENT_1." );

         FC_ASSERT( content_reward_decay_rate >= fc::days(1) && content_reward_decay_rate <= fc::days(365),
            "Content reward decay rate must be between 1 and 365 days." );
         FC_ASSERT( content_reward_interval >= fc::hours(1) && content_reward_interval <= fc::days(7),
            "Content reward interval must be between 1 hour and 7 days." );

         FC_ASSERT( vote_reserve_rate >= 1 && vote_reserve_rate <= 10000,
            "Vote reserve rate must be between 1 and 10000." );
         FC_ASSERT( view_reserve_rate >= 1 && view_reserve_rate <= 10000,
            "View reserve rate must be between 1 and 10000." );
         FC_ASSERT( share_reserve_rate >= 1 && share_reserve_rate <= 10000,
            "Share reserve rate must be between 1 and 10000." );
         FC_ASSERT( comment_reserve_rate >= 1 && comment_reserve_rate <= 10000,
            "Comment reserve rate must be between 1 and 10000." );

         FC_ASSERT( vote_recharge_time >= fc::days(1) && vote_recharge_time <= fc::days(365),
            "Vote Recharge time must be between 1 and 365 days." );
         FC_ASSERT( view_recharge_time >= fc::days(1) && view_recharge_time <= fc::days(365),
            "View Recharge time must be between 1 and 365 days." );
         FC_ASSERT( share_recharge_time >= fc::days(1) && share_recharge_time <= fc::days(365),
            "Share Recharge time must be between 1 and 365 days." );
         FC_ASSERT( comment_recharge_time >= fc::days(1) && comment_recharge_time <= fc::days(365),
            "Comment Recharge time must be between 1 and 365 days." );
         FC_ASSERT( curation_auction_decay_time >= fc::minutes(1) && curation_auction_decay_time <= fc::days(1),
            "Curation auction decay time must be between 1 minute and 1 day." );

         FC_ASSERT( vote_curation_decay >= 1 && vote_curation_decay <= 100000,
            "Vote curation decay must be between 1 and 100,000." );
         FC_ASSERT( view_curation_decay >= 1 && view_curation_decay <= 100000,
            "View curation decay must be between 1 and 100,000." );
         FC_ASSERT( share_curation_decay >= 1 && share_curation_decay <= 100000,
            "Share curation decay must be between 1 and 100,000." );
         FC_ASSERT( comment_curation_decay >= 1 && comment_curation_decay <= 100000,
            "Comment curation decay must be between 1 and 100,000." );

         FC_ASSERT( supernode_decay_time >= fc::days(1) && supernode_decay_time <= fc::days(365),
            "Supernode Decay time must be between 1 and 365 days." );
         FC_ASSERT( enterprise_vote_percent_required >= 0 && enterprise_vote_percent_required <= PERCENT_100,
            "Enterprise vote percent required must be between 0 and PERCENT_100." );
         FC_ASSERT( maximum_asset_whitelist_authorities >= MAX_ASSET_WHITELIST_AUTHORITIES && 
            maximum_asset_whitelist_authorities <= 10 * MAX_ASSET_WHITELIST_AUTHORITIES,
            "Executive types amount must be between 1000 and 10,000." );
         FC_ASSERT( max_stake_intervals >= MAX_ASSET_STAKE_INTERVALS && max_stake_intervals <= 100 * MAX_ASSET_STAKE_INTERVALS,
            "Max stake intervals must be between 104 and 10400." );
         FC_ASSERT( max_unstake_intervals >= MAX_ASSET_UNSTAKE_INTERVALS && max_unstake_intervals <= 100 * MAX_ASSET_UNSTAKE_INTERVALS,
            "Max unstake intervals must be between 104 and 10400." );
         FC_ASSERT( max_exec_budget.symbol == SYMBOL_CREDIT,
            "Max Excutive Budget must be in the CREDIT asset." );
         FC_ASSERT( max_exec_budget >= MAX_EXEC_BUDGET,
            "Max Excutive Budget must be less than or equal to 1,000,000 MCR." );
      };
   };
}

namespace chain {

   using node::protocol::asset;
   using node::protocol::price;

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
      public:
         template< typename Constructor, typename Allocator >
         median_chain_property_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         median_chain_property_object(){}

         id_type                id;

         asset                  account_creation_fee = MIN_ACCOUNT_CREATION_FEE;               ///< Minimum fee required to create a new account by staking.

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

         uint16_t               maximum_asset_feed_publishers = MAX_ASSET_FEED_PUBLISHERS;     ///< The maximum number of accounts that can publish price feeds for a bitasset.

         asset                  membership_base_price = MEMBERSHIP_FEE_BASE;                   ///< The price for standard membership per month.

         asset                  membership_mid_price = MEMBERSHIP_FEE_MID;                     ///< The price for Mezzanine membership per month.

         asset                  membership_top_price = MEMBERSHIP_FEE_TOP;                     ///< The price for top level membership per month.

         uint32_t               author_reward_percent = AUTHOR_REWARD_PERCENT;                 ///< The percentage of content rewards distributed to post authors.

         uint32_t               vote_reward_percent = VOTE_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post voters.

         uint32_t               view_reward_percent = VIEW_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post viewers.

         uint32_t               share_reward_percent = SHARE_REWARD_PERCENT;                   ///< The percentage of content rewards distributed to post sharers.

         uint32_t               comment_reward_percent = COMMENT_REWARD_PERCENT;               ///< The percentage of content rewards distributed to post commenters.

         uint32_t               storage_reward_percent = STORAGE_REWARD_PERCENT;               ///< The percentage of content rewards distributed to viewing supernodes.

         uint32_t               moderator_reward_percent = MODERATOR_REWARD_PERCENT;           ///< The percentage of content rewards distributed to board moderators.

         fc::microseconds       content_reward_decay_rate = CONTENT_REWARD_DECAY_RATE;         ///< The time over which content rewards are distributed

         fc::microseconds       content_reward_interval = CONTENT_REWARD_INTERVAL;             ///< Time taken per distribution of content rewards.

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

         uint16_t               enterprise_vote_percent_required = VOTE_THRESHOLD_PERCENT;     ///< Percentage of total voting power required to approve enterprise milestones. 

         uint64_t               maximum_asset_whitelist_authorities = MAX_ASSET_WHITELIST_AUTHORITIES;  ///< The maximum amount of whitelisted or blacklisted authorities for user issued assets 

         uint8_t                max_stake_intervals = MAX_ASSET_STAKE_INTERVALS;               ///< Maximum weeks that an asset can stake over.

         uint8_t                max_unstake_intervals = MAX_ASSET_UNSTAKE_INTERVALS;           ///< Maximum weeks that an asset can unstake over.

         asset                  max_exec_budget = MAX_EXEC_BUDGET;                             ///< Maximum budget that an executive board can claim.
      };

      typedef multi_index_container<
         median_chain_property_object,
         indexed_by<
            ordered_unique< tag< by_id >,
               member< median_chain_property_object, median_chain_property_object::id_type, &median_chain_property_object::id > >
         >,
         allocator< median_chain_property_object >
      > median_chain_property_index;

} } // node::chain


FC_REFLECT( node::protocol::chain_properties,
         (account_creation_fee)
         (maximum_block_size)
         (pow_target_time)
         (pow_decay_time)
         (txn_stake_decay_time)
         (credit_interest_rate)
         (credit_open_ratio)
         (credit_liquidation_ratio)
         (credit_min_interest)
         (credit_variable_interest)
         (market_max_credit_ratio)
         (margin_open_ratio)
         (margin_liquidation_ratio)
         (interest_compound_interval)
         (maximum_asset_feed_publishers)
         (membership_base_price)
         (membership_mid_price)
         (membership_top_price)
         (content_reward_decay_rate)
         (content_reward_interval)
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

FC_REFLECT( node::chain::median_chain_property_object,
         (id)
         (account_creation_fee)
         (maximum_block_size)
         (pow_target_time)
         (pow_decay_time)
         (txn_stake_decay_time)
         (credit_interest_rate)
         (credit_open_ratio)
         (credit_liquidation_ratio)
         (credit_min_interest)
         (credit_variable_interest)
         (market_max_credit_ratio)
         (margin_open_ratio)
         (margin_liquidation_ratio)
         (interest_compound_interval)
         (maximum_asset_feed_publishers)
         (membership_base_price)
         (membership_mid_price)
         (membership_top_price)
         (content_reward_decay_rate)
         (content_reward_interval)
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