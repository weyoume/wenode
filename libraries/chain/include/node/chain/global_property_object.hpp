#pragma once
#include <fc/uint128.hpp>

#include <node/chain/node_object_types.hpp>

#include <node/protocol/asset.hpp>

namespace node { namespace chain {

   using node::protocol::asset;
   using node::protocol::price;

   /**
    * @class dynamic_global_property_object
    * @brief Maintains global state information
    * @ingroup object
    * @ingroup implementation
    *
    * This is an implementation detail. The values here are calculated during normal chain operations and reflect the
    * current values of global blockchain properties.
    */
   class dynamic_global_property_object : public object< dynamic_global_property_object_type, dynamic_global_property_object>
   {
      public:
         template< typename Constructor, typename Allocator >
         dynamic_global_property_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         dynamic_global_property_object(){}

         id_type                id;

         uint32_t               head_block_number = 0;                                    // Number of the current highest block in the network that the node has processed. 

         block_id_type          head_block_id;                                            // The block id of the most recently produced block.

         uint32_t               last_irreversible_block_num = 0;                          // The number of the last block that has been produced on top of by at least #IRREVERSIBLE_THRESHOLD of block producers.

         block_id_type          last_irreversible_block_id;                               // The block id of the last irreversible block.

         uint32_t               last_committed_block_num = 0;                             // The number of the last block that has been committed by #IRREVERSIBLE_THRESHOLD of block producers.

         block_id_type          last_committed_block_id;                                  // The block id of the last irreversible block.
  
         account_name_type      current_producer;                                         // The account name of the current block producing witness or miner. 

         time_point             time;                                                     // Current blockchain time in microseconds;

         time_point             next_maintenance_time;                                    // Time in microseconds of the next maintenance period.

         asset                  accumulated_network_revenue = asset(0, SYMBOL_COIN);      // Counter for the total of all core assets burned as network revenue. 

         asset                  membership_base_price = MEMBERSHIP_FEE_BASE;              // The price for standard membership per month.

         asset                  membership_mid_price = MEMBERSHIP_FEE_MID;                // The price for Mezzanine membership per month.

         asset                  membership_top_price = MEMBERSHIP_FEE_TOP;                // The price for top level membership per month.

         price                  current_median_equity_price;                              // The current price of Equity Asset in Coin asset.

         price                  current_median_usd_price;                                 // The current price of the USd asst in the Coin asset.

         uint128_t              total_voting_power;                                       // Current outstanding supply of voting power in both equity and staked coin balances.

         uint128_t              total_pow = 0;                                            // The total POW accumulated

         uint16_t               credit_interest_rate = 0;                                 // The median witness elected interest rate that credit asset holders receive for lending to the protocol.

         uint16_t               credit_open_ratio = CREDIT_OPEN_RATIO;                    // The minimum required collateralization ratio for a credit loan to be opened. 

         uint16_t               credit_liquidation_ratio = CREDIT_LIQUIDATION_RATIO;      // The minimum permissible collateralization ratio before a loan is liquidated. 

         uint16_t               credit_min_interest = CREDIT_MIN_INTEREST;                // The minimum component of credit pool interest rates. 

         uint16_t               credit_variable_interest = CREDIT_VARIABLE_INTEREST;      // The variable component of credit pool interest rates, applied at equal base and borrowed balances.

         uint16_t               market_max_credit_ratio = MARKET_MAX_CREDIT_RATIO;        // The maximum percentage of core asset liquidity balances that can be loaned.

         uint16_t               margin_open_ratio = MARGIN_OPEN_RATIO;                    // The minimum required collateralization ratio for a credit loan to be opened. 

         uint16_t               margin_liquidation_ratio = MARGIN_LIQUIDATION_RATIO;      // The minimum permissible collateralization ratio before a loan is liquidated. 

         uint32_t               maximum_block_size = 0;                                   // The current median witness elected maximum block size in bytes, limited to at least #MIN_BLOCK_SIZE_LIMIT. 

         uint64_t               current_aslot = 0;                                        // The current absolute slot number. Equal to the total number of slots since genesis.

         uint64_t               maximum_asset_whitelist_authorities = MAX_ASSET_WHITELIST_AUTHORITIES;  // The maximum amount of whitelisted or blacklisted authorities for user issued assets 

         uint8_t                max_stake_intervals = MAX_ASSET_STAKE_INTERVALS;          // Maximum weeks that an asset can stake over.

         uint8_t                max_unstake_intervals = MAX_ASSET_UNSTAKE_INTERVALS;      // Maximum weeks that an asset can unstake over.

         fc::uint128_t          recent_slots_filled;                                      // parameter used to compute witness participation.

         uint8_t                participation_count = 0;                                  // Divide by 128 to compute participation percentage

         uint32_t               asset_count = 0;                                          // The number of active assets on the network.

         uint32_t               account_count = 0;                                        // The total number of accounts created. 

         fc::microseconds       content_reward_decay_rate = CONTENT_REWARD_DECAY_RATE;

         fc::microseconds       content_reward_interval = CONTENT_REWARD_INTERVAL;        

         uint32_t               vote_reserve_rate = VOTE_RESERVE_RATE;                    // The number of votes regenerated per day.

         uint32_t               view_reserve_rate = VIEW_RESERVE_RATE;                    // The number of views regenerated per day.

         uint32_t               share_reserve_rate = SHARE_RESERVE_RATE;                  // The number of shares regenerated per day.

         uint32_t               comment_reserve_rate = COMMENT_RESERVE_RATE;              // The number of comments regenerated per day.

         fc::microseconds       vote_recharge_time = VOTE_RECHARGE_TIME;                  // Time taken to fully recharge voting power.

         fc::microseconds       view_recharge_time = VIEW_RECHARGE_TIME;                  // Time taken to fully recharge viewing power.

         fc::microseconds       share_recharge_time = SHARE_RECHARGE_TIME;                // Time taken to fully recharge sharing power.

         fc::microseconds       comment_recharge_time = COMMENT_RECHARGE_TIME;            // Time taken to fully recharge commenting power.

         fc::microseconds       curation_auction_decay_time = CURATION_AUCTION_DECAY_TIME;// time of curation reward decay after a post is created. 

         double                 vote_curation_decay = VOTE_CURATION_DECAY;                // Number of votes for the half life of voting curation reward decay.

         double                 view_curation_decay = VIEW_CURATION_DECAY;                // Number of views for the half life of viewer curation reward decay.

         double                 share_curation_decay = SHARE_CURATION_DECAY;              // Number of shares for the half life of sharing curation reward decay.

         double                 comment_curation_decay = COMMENT_CURATION_DECAY;          // Number of comments for the half life of comment curation reward decay.

         fc::microseconds       supernode_decay_time = SUPERNODE_DECAY_TIME;              // Amount of time to average the supernode file weight over. 

         uint16_t               enterprise_vote_percent_required = VOTE_THRESHOLD_PERCENT;   // Percentage of total voting power required to approve enterprise milestones. 

         uint16_t               executive_types_amount = EXECUTIVE_TYPES_AMOUNT;          // Number of roles on a business account executive board.

         uint32_t               dynamic_flags = 0;

         

         enum dynamic_flag_bits
         {
            /**
             * If maintenance_flag is set, then the head block is a
             * maintenance block.  This means
             * get_time_slot(1) - head_block_time() will have a gap
             * due to maintenance duration.
             *
             * This flag answers the question, "Was maintenance
             * performed in the last call to apply_block()?"
             */
            maintenance_flag = 0x01
         };
   };

   typedef multi_index_container<
      dynamic_global_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< dynamic_global_property_object, dynamic_global_property_object::id_type, &dynamic_global_property_object::id > >
      >,
      allocator< dynamic_global_property_object >
   > dynamic_global_property_index;

} } // node::chain

FC_REFLECT( node::chain::dynamic_global_property_object,
         (id)
         (head_block_number)
         (head_block_id)
         (time)
         (current_producer)
         (next_maintenance_time)
         (total_pow)
         (num_pow_witnesses)
         (credit_interest_rate)
         (maximum_block_size)
         (current_aslot)
         (recent_slots_filled)
         (participation_count)
         (last_irreversible_block_num)
         (vote_power_reserve_rate)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::dynamic_global_property_object, node::chain::dynamic_global_property_index );
