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
   using node::protocol::option_strike;
   using node::protocol::asset_unit;
   using node::protocol::authority;
   using node::protocol::signed_transaction;
   using node::protocol::operation;

   using node::protocol::chain_properties;
   using node::protocol::digest_type;
   using node::protocol::public_key_type;
   using node::protocol::version;
   using node::protocol::hardfork_version;

   using node::protocol::price;
   using node::protocol::asset;
   using node::protocol::option_strike;
   using node::protocol::price_feed;
   using node::protocol::asset_unit;

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
   
   using node::protocol::encrypted_keypair_type;
   using node::protocol::date_type;
  
   using node::protocol::beneficiary_route_type;
   using node::protocol::executive_officer_set;
   using chainbase::shared_vector;

   using node::protocol::community_privacy_type;
   using node::protocol::business_structure_type;
   using node::protocol::membership_tier_type;
   using node::protocol::network_officer_role_type;
   using node::protocol::executive_role_type;
   using node::protocol::proposal_distribution_type;
   using node::protocol::product_sale_type;
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

         uint16_t               maximum_asset_feed_publishers = MAX_ASSET_FEED_PUBLISHERS;     ///< The maximum number of accounts that can publish price feeds for a stablecoin.

         asset                  membership_base_price = MEMBERSHIP_FEE_BASE;                   ///< The price for standard membership per month.

         asset                  membership_mid_price = MEMBERSHIP_FEE_MID;                     ///< The price for Mezzanine membership per month.

         asset                  membership_top_price = MEMBERSHIP_FEE_TOP;                     ///< The price for top level membership per month.

         uint32_t               author_reward_percent = AUTHOR_REWARD_PERCENT;                 ///< The percentage of content rewards distributed to post authors.

         uint32_t               vote_reward_percent = VOTE_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post voters.

         uint32_t               view_reward_percent = VIEW_REWARD_PERCENT;                     ///< The percentage of content rewards distributed to post viewers.

         uint32_t               share_reward_percent = SHARE_REWARD_PERCENT;                   ///< The percentage of content rewards distributed to post sharers.

         uint32_t               comment_reward_percent = COMMENT_REWARD_PERCENT;               ///< The percentage of content rewards distributed to post commenters.

         uint32_t               storage_reward_percent = STORAGE_REWARD_PERCENT;               ///< The percentage of content rewards distributed to viewing supernodes.

         uint32_t               moderator_reward_percent = MODERATOR_REWARD_PERCENT;           ///< The percentage of content rewards distributed to community moderators.

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

         uint16_t               enterprise_vote_percent_required = ENTERPRISE_VOTE_THRESHOLD_PERCENT;     ///< Percentage of total voting power required to approve enterprise milestones. 

         uint64_t               maximum_asset_whitelist_authorities = MAX_ASSET_WHITELIST_AUTHORITIES;  ///< The maximum amount of whitelisted or blacklisted authorities for user issued assets 

         uint8_t                max_stake_intervals = MAX_ASSET_STAKE_INTERVALS;               ///< Maximum weeks that an asset can stake over.

         uint8_t                max_unstake_intervals = MAX_ASSET_UNSTAKE_INTERVALS;           ///< Maximum weeks that an asset can unstake over.

         asset                  max_exec_budget = MAX_EXEC_BUDGET;                             ///< Maximum budget that an executive board can claim.
   };

   
   /**
    * Holds asset collateral balance for lending.
    * 
    * Use to support a credit borrowing order from the asset's credit pool, 
    * or a margin order by substracting collateral from 
    * the object to include in a margin order's collateral.
    */
   class credit_collateral_object : public object< credit_collateral_object_type, credit_collateral_object >
   {
      credit_collateral_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         credit_collateral_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;               ///< Collateral owners account name.

         asset_symbol_type          symbol;              ///< Asset symbol being collateralized. 

         asset                      collateral;          ///< Asset balance that is being locked in for loan backing for loan or margin orders.

         time_point                 created;             ///< Time that collateral was created.

         time_point                 last_updated;        ///< Time that the order was last modified.
   };


   /**
    * Credit collateral object holds assets in collateral for use to support a
    * credit borrowing order from the asset's credit pool, 
    * or a margin order by substracting collateral from 
    * the object to include in a margin order's collateral.
    */
   class credit_loan_object : public object< credit_loan_object_type, credit_loan_object >
   {
      credit_loan_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         credit_loan_object( Constructor&& c, allocator< Allocator > a ) :
         loan_id(a)
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;                   ///< Collateral owner's account name

         shared_string              loan_id;                 ///< UUIDV4 for the loan to uniquely identify it for reference. 

         asset                      debt;                    ///< Amount of an asset borrowed. Limit of 75% of collateral value. Increases with interest charged.

         asset                      interest;                ///< Total Amount of interest accrued on the loan. 

         asset                      collateral;              ///< Amount of an asset to use as collateral for the loan. 

         price                      loan_price;              ///< Collateral / Debt. Must be higher than liquidation price to remain solvent. 

         price                      liquidation_price;       ///< Collateral / max_debt value. Rises when collateral/debt market price falls.

         asset_symbol_type          symbol_a;                ///< The symbol of asset A in the debt / collateral exchange pair.

         asset_symbol_type          symbol_b;                ///< The symbol of asset B in the debt / collateral exchange pair.

         share_type                 last_interest_rate;      ///< Updates the interest rate of the loan hourly. 

         time_point                 created;                 ///< Time that the loan was taken out.

         time_point                 last_updated;            ///< Time that the loan details was last updated.

         time_point                 last_interest_time;      ///< Time that interest was last compounded. 

         bool                       flash_loan = false;      ///< True when the loan is a flash loan, which has no collateral and must be repaid within same txn.

         asset_symbol_type          debt_asset()const { return debt.symbol; }

         asset_symbol_type          collateral_asset()const { return collateral.symbol; }

         price                      liquidation_spread()const { return loan_price - liquidation_price; }

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return debt.symbol < collateral.symbol ?
               std::make_pair( debt.symbol, collateral.symbol ) :
               std::make_pair( collateral.symbol, debt.symbol );
         }
   };


   class escrow_object : public object< escrow_object_type, escrow_object >
   {
      escrow_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         escrow_object( Constructor&& c, allocator< Allocator > a ) :
         escrow_id(a), memo(a), json(a)
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

         shared_string                             json;                   ///< Additonal JSON object attribute details.

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


   class product_object : public object< product_object_type, product_object >
   {
      product_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         product_object( Constructor&& c, allocator< Allocator > a ) :
         product_id(a), 
         name(a), 
         url(a), 
         json(a), 
         product_variants( a.get_segment_manager() ), 
         product_details( a.get_segment_manager() ), 
         product_images( a.get_segment_manager() ), 
         product_prices( a.get_segment_manager() ), 
         stock_available( a.get_segment_manager() ), 
         delivery_variants( a.get_segment_manager() ), 
         delivery_details( a.get_segment_manager() ), 
         delivery_prices( a.get_segment_manager() )
         {
            c( *this );
         }

         id_type                               id;

         account_name_type                     account;                ///< The Seller of the product.

         shared_string                         product_id;             ///< The name of the product.

         shared_string                         name;                   ///< The name of the product. Unique for each account.

         product_sale_type                     sale_type;              ///< The type of sale to be used for the product.

         shared_string                         url;                    ///< Reference URL of the product or seller.

         shared_string                         json;                   ///< JSON metadata attributes of the product.

         shared_vector< shared_string >        product_variants;       ///< The collection of product variants. Each map must have a key for each variant.

         shared_vector< shared_string >        product_details;        ///< The Description details of each variant of the product.

         shared_vector< shared_string >        product_images;         ///< IPFS references to images of each product variant.

         shared_vector< asset >                product_prices;         ///< The price (or min auction price) for each variant of the product.

         shared_vector< uint32_t >             stock_available;        ///< The available stock of each variant of the product.

         shared_vector< shared_string >        delivery_variants;      ///< The types of product delivery available to purchasers.

         shared_vector< shared_string >        delivery_details;       ///< The Description details of each variant of the delivery.

         shared_vector< asset >                delivery_prices;        ///< The price for each variant of delivery.

         time_point                            created;                ///< Time that the order was created.

         time_point                            last_updated;           ///< Time that the order was last updated, approved, or disputed.
   };


   class purchase_order_object : public object< purchase_order_object_type, purchase_order_object >
   {
      purchase_order_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         purchase_order_object( Constructor&& c, allocator< Allocator > a ) :
         order_id(a), 
         product_id(a), 
         order_variants( a.get_segment_manager() ), 
         order_size( a.get_segment_manager() ), 
         memo(a), 
         json(a), 
         shipping_address(a),
         delivery_variant(a), 
         delivery_details(a)
         {
            c( *this );
         }

         id_type                           id;

         account_name_type                 buyer;                  ///< The Buyer of the product.

         shared_string                     order_id;               ///< uuidv4 referring to the purchase order.

         account_name_type                 seller;                 ///< The Seller of the product.

         shared_string                     product_id;             ///< uuidv4 referring to the product.

         shared_vector< shared_string >    order_variants;         ///< Variants of product ordered in the purchase.

         shared_vector< uint32_t >         order_size;             ///< The number of each product variant ordered. 

         shared_string                     memo;                   ///< The memo for the transaction, encryption on the memo is advised.

         shared_string                     json;                   ///< Additonal JSON object attribute details.

         shared_string                     shipping_address;       ///< The shipping address requested, encrypted with the secure key of the seller.

         shared_string                     delivery_variant;       ///< The type of product delivery selected.

         shared_string                     delivery_details;       ///< The Description details of the delivery.

         asset                             order_value;            ///< The total value of the order.

         time_point                        created;                ///< Time that the order was created.

         time_point                        last_updated;           ///< Time that the order was last updated, approved, or disputed.

         bool                              completed = false;      ///< True when the purchase order has been completed, false when outstanding. 
   };


   class savings_withdraw_object : public object< savings_withdraw_object_type, savings_withdraw_object >
   {
      savings_withdraw_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         savings_withdraw_object( Constructor&& c, allocator< Allocator > a ) :
         memo(a), request_id(a)
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       from;         ///< Account that is withdrawing savings balance

         account_name_type       to;           ///< Account to direct withdrawn assets to

         shared_string           memo;         ///< Reference memo for the transaction

         shared_string           request_id;   ///< uuidv4 reference of the withdrawl instance

         asset                   amount;       ///< Amount to withdraw
         
         time_point              complete;     ///< Time that the withdrawal will complete
   };


   /**
    * A route to send unstaked assets.
    */
   class unstake_asset_route_object : public object< unstake_asset_route_object_type, unstake_asset_route_object >
   {
      unstake_asset_route_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         unstake_asset_route_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       from;                  ///< Account that is unstaking the asset balance

         account_name_type       to;                    ///< Account name that receives the unstaked assets

         asset_symbol_type       symbol;                ///< Asset to be unstaked

         uint16_t                percent = 0;           ///< Percentage of unstaking asset that should be directed to this route.

         bool                    auto_stake = false;    ///< Automatically stake the asset on the receiving account
   };


   class decline_voting_rights_request_object : public object< decline_voting_rights_request_object_type, decline_voting_rights_request_object >
   {
      decline_voting_rights_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         decline_voting_rights_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type             id;

         account_name_type   account;
         
         time_point          effective_date;
   };


   enum curve_id
   {
      quadratic,                   ///< Returns the square of the reward, with a constant added
      quadratic_curation,          ///< Returns an amount converging to linear with reward
      linear,                      ///< Returns exactly the reward, without using constant
      square_root,                 ///< returns exactly the square root of reward
      convergent_semi_quadratic    ///< Returns an amount converging to the reward, to the power of 1.5, which decays over the time period specified
   };


   class reward_fund_object : public object< reward_fund_object_type, reward_fund_object >
   {
      reward_fund_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         reward_fund_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         asset_symbol_type       symbol;                                                    ///< Currency symbol of the asset that the reward fund issues.

         asset                   content_reward_balance;                                    ///< Balance awaiting distribution to content creators.

         asset                   validation_reward_balance;                                 ///< Balance distributed to block validating producers. 

         asset                   txn_stake_reward_balance;                                  ///< Balance distributed to block producers based on the stake weighted transactions in each block.

         asset                   work_reward_balance;                                       ///< Balance distributed to each proof of work block producer. 

         asset                   producer_activity_reward_balance;                          ///< Balance distributed to producers that receive activity reward votes.

         asset                   supernode_reward_balance;                                  ///< Balance distributed to supernodes, based on stake weighted comment views.

         asset                   power_reward_balance;                                      ///< Balance distributed to staked units of the currency.

         asset                   community_fund_balance;                                    ///< Balance distributed to community proposal funds on the currency. 

         asset                   development_reward_balance;                                ///< Balance distributed to elected developers. 

         asset                   marketing_reward_balance;                                  ///< Balance distributed to elected marketers. 

         asset                   advocacy_reward_balance;                                   ///< Balance distributed to elected advocates. 

         asset                   activity_reward_balance;                                   ///< Balance distributed to content creators that are active each day. 

         asset                   premium_partners_fund_balance;                             ///< Receives income from memberships, distributed to premium creators. 

         asset                   total_pending_reward_balance;                              ///< Total of all reward balances. 

         uint128_t               recent_content_claims = 0;                                 ///< Recently claimed content reward balance shares.

         uint128_t               recent_activity_claims = 0;                                ///< Recently claimed activity reward balance shares.

         uint128_t               content_constant = CONTENT_CONSTANT;                       ///< Contstant added to content claim shares.

         fc::microseconds        content_reward_decay_rate = CONTENT_REWARD_DECAY_RATE;     ///< Time taken to distribute all content rewards.

         fc::microseconds        content_reward_interval = CONTENT_REWARD_INTERVAL;         ///< Time between each individual distribution of content rewards. 

         curve_id                author_reward_curve = convergent_semi_quadratic;           ///< Type of reward curve used for author content reward calculation. 

         curve_id                curation_reward_curve = convergent_semi_quadratic;         ///< Type of reward curve used for curation content reward calculation.

         time_point              last_updated;                                              ///< Time that the reward fund was last updated. 

         void                    adjust_content_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            content_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_validation_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            validation_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_txn_stake_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            txn_stake_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_work_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            work_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_producer_activity_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            activity_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_supernode_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            supernode_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_power_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            power_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_community_fund_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            community_fund_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_development_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            development_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_marketing_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            marketing_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_advocacy_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            advocacy_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_activity_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            activity_reward_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }

         void                    adjust_premium_partners_fund_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            premium_partners_fund_balance += delta;
            total_pending_reward_balance += delta;
         } FC_CAPTURE_AND_RETHROW() }
   };


   typedef multi_index_container<
      median_chain_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< median_chain_property_object, median_chain_property_id_type, &median_chain_property_object::id > >
      >,
      allocator< median_chain_property_object >
   > median_chain_property_index;


   struct by_expiration;
   struct by_request_id;
   struct by_from_account;


   struct by_end;
   struct by_begin;
   struct by_next_transfer;
   struct by_transfer_id;
   struct by_to_account;

   struct by_price;
   struct by_account;
   struct by_owner;
   struct by_conversion_date;
   struct by_volume_weight;
   struct by_withdraw_route;
   struct by_destination;

   typedef multi_index_container<
      unstake_asset_route_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< unstake_asset_route_object, unstake_asset_route_id_type, &unstake_asset_route_object::id > >,
         ordered_unique< tag< by_withdraw_route >,
            composite_key< unstake_asset_route_object,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::from >,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::to >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type > 
            >
         >,
         ordered_unique< tag< by_destination >,
            composite_key< unstake_asset_route_object,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::to >,
               member< unstake_asset_route_object, unstake_asset_route_id_type, &unstake_asset_route_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< unstake_asset_route_id_type >
            >
         >
      >,
      allocator< unstake_asset_route_object >
   > unstake_asset_route_index;

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

   struct by_product_id;
   struct by_last_updated;

   typedef multi_index_container<
      product_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< product_object, product_id_type, &product_object::id > >,
         ordered_unique< tag< by_product_id >,
            composite_key< product_object,
               member< product_object, account_name_type,  &product_object::account >,
               member< product_object, shared_string, &product_object::product_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< product_object,
               member< product_object, time_point, &product_object::last_updated >,
               member< product_object, product_id_type, &product_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< product_id_type >
            >
         >
      >,
      allocator< product_object >
   > product_index;

   struct by_order_id;
   struct by_buyer_product_id;


   typedef multi_index_container<
      purchase_order_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< purchase_order_object, purchase_order_id_type, &purchase_order_object::id > >,
         ordered_unique< tag< by_order_id >,
            composite_key< purchase_order_object,
               member< purchase_order_object, account_name_type, &purchase_order_object::buyer >,
               member< purchase_order_object, shared_string, &purchase_order_object::order_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_product_id >,
            composite_key< purchase_order_object,
               member< purchase_order_object, account_name_type, &purchase_order_object::seller >,
               member< purchase_order_object, shared_string, &purchase_order_object::product_id >,
               member< purchase_order_object, purchase_order_id_type, &purchase_order_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less,
               std::less< purchase_order_id_type >
            >
         >,
         ordered_unique< tag< by_buyer_product_id >,
            composite_key< purchase_order_object,
               member< purchase_order_object, account_name_type, &purchase_order_object::buyer >,
               member< purchase_order_object, account_name_type, &purchase_order_object::seller >,
               member< purchase_order_object, shared_string, &purchase_order_object::product_id >,
               member< purchase_order_object, purchase_order_id_type, &purchase_order_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >,
               strcmp_less,
               std::less< purchase_order_id_type >
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< purchase_order_object,
               member< purchase_order_object, time_point, &purchase_order_object::last_updated >,
               member< purchase_order_object, purchase_order_id_type, &purchase_order_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< purchase_order_id_type >
            >
         >
      >,
      allocator< purchase_order_object >
   > purchase_order_index;


   struct by_request_id;
   struct by_to_complete;
   struct by_complete_from_request_id;

   typedef multi_index_container<
      savings_withdraw_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id > >,
         ordered_unique< tag< by_request_id >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, shared_string, &savings_withdraw_object::request_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_to_complete >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::to >,
               member< savings_withdraw_object, time_point,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< time_point >,
               std::less< savings_withdraw_id_type >
            >
         >,
         ordered_unique< tag< by_complete_from_request_id >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, time_point,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, shared_string, &savings_withdraw_object::request_id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >
      >,
      allocator< savings_withdraw_object >
   > savings_withdraw_index;

   struct by_account;
   struct by_effective_date;

   typedef multi_index_container<
      decline_voting_rights_request_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< decline_voting_rights_request_object, decline_voting_rights_request_id_type, &decline_voting_rights_request_object::id > >,
         ordered_unique< tag< by_account >,
            member< decline_voting_rights_request_object, account_name_type, &decline_voting_rights_request_object::account >
         >,
         ordered_unique< tag< by_effective_date >,
            composite_key< decline_voting_rights_request_object,
               member< decline_voting_rights_request_object, time_point, &decline_voting_rights_request_object::effective_date >,
               member< decline_voting_rights_request_object, account_name_type, &decline_voting_rights_request_object::account >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< account_name_type > 
            >
         >
      >,
      allocator< decline_voting_rights_request_object >
   > decline_voting_rights_request_index;

   struct by_name;
   struct by_symbol;

   typedef multi_index_container<
      reward_fund_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< reward_fund_object, reward_fund_id_type, &reward_fund_object::id > >,
         ordered_unique< tag< by_symbol >,
            member< reward_fund_object, asset_symbol_type, &reward_fund_object::symbol > >
      >,
      allocator< reward_fund_object >
   > reward_fund_index;

   struct by_owner_symbol;

   typedef multi_index_container<
      credit_collateral_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< credit_collateral_object, credit_collateral_id_type, &credit_collateral_object::id > >,
         ordered_unique< tag< by_owner_symbol >,
            composite_key< credit_collateral_object,
               member< credit_collateral_object, account_name_type, &credit_collateral_object::owner >,
               member< credit_collateral_object, asset_symbol_type, &credit_collateral_object::symbol >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< asset_symbol_type >
            >
         >,
         ordered_unique< tag< by_owner >,
            composite_key< credit_collateral_object,
               member< credit_collateral_object, account_name_type, &credit_collateral_object::owner >,
               member< credit_collateral_object, credit_collateral_id_type, &credit_collateral_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< credit_collateral_id_type >
            >
         >
      >,
      allocator < credit_collateral_object >
   > credit_collateral_index;

   struct by_loan_id;
   struct by_owner;
   struct by_owner_price;
   struct by_owner_debt;
   struct by_owner_collateral;
   struct by_last_updated; 
   struct by_debt;
   struct by_collateral;
   struct by_liquidation_spread;
   struct by_flash_loan;

   typedef multi_index_container<
      credit_loan_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id > >,
         ordered_unique< tag< by_owner >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag<by_loan_id>,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               member< credit_loan_object, shared_string, &credit_loan_object::loan_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_owner_price >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               member< credit_loan_object, price, &credit_loan_object::loan_price >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< price >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_owner_debt >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< asset_symbol_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_owner_collateral >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< asset_symbol_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_liquidation_spread >,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset >,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset >,
               const_mem_fun< credit_loan_object, price, &credit_loan_object::liquidation_spread >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< asset_symbol_type >,
               std::less< price >,
               std::less< credit_loan_id_type >
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< credit_loan_object,
               member< credit_loan_object, time_point, &credit_loan_object::last_updated >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_debt >,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_collateral >,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< credit_loan_id_type >
            >
         >,
         ordered_unique< tag< by_flash_loan >,
            composite_key< credit_loan_object,
               member< credit_loan_object, bool, &credit_loan_object::flash_loan >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::greater< bool >,
               std::less< credit_loan_id_type >
            >
         >
      >, 
      allocator < credit_loan_object >
   > credit_loan_index;

} } // node::chain

#include <node/chain/producer_objects.hpp>
#include <node/chain/network_object.hpp>
#include <node/chain/transfer_object.hpp>
#include <node/chain/asset_object.hpp>
#include <node/chain/trading_object.hpp>
#include <node/chain/comment_object.hpp>
#include <node/chain/account_object.hpp>
#include <node/chain/community_object.hpp>
#include <node/chain/ad_object.hpp>
#include <node/chain/graph_object.hpp>

FC_REFLECT( node::chain::median_chain_property_object,
         (id)
         (account_creation_fee)
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
         (author_reward_percent)
         (vote_reward_percent)
         (view_reward_percent)
         (share_reward_percent)
         (comment_reward_percent)
         (storage_reward_percent)
         (moderator_reward_percent)
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

FC_REFLECT( node::chain::credit_collateral_object,
         (id)
         (owner)
         (symbol)
         (collateral)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::credit_collateral_object, node::chain::credit_collateral_index );   

FC_REFLECT( node::chain::credit_loan_object,
         (id)
         (owner)
         (loan_id)
         (debt)
         (interest)
         (collateral)
         (loan_price)
         (liquidation_price)
         (symbol_a)
         (symbol_b)
         (last_interest_rate)
         (created)
         (last_updated)
         (last_interest_time)
         (flash_loan)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::credit_loan_object, node::chain::credit_loan_index );

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

FC_REFLECT( node::chain::product_object,
         (id)
         (account)
         (product_id)
         (name)
         (sale_type)
         (url)
         (json)
         (product_variants)
         (product_details)
         (product_images)
         (product_prices)
         (stock_available)
         (delivery_variants)
         (delivery_details)
         (delivery_prices)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::product_object, node::chain::product_index );

FC_REFLECT( node::chain::purchase_order_object,
         (id)
         (buyer)
         (order_id)
         (seller)
         (product_id)
         (order_variants)
         (order_size)
         (memo)
         (json)
         (shipping_address)
         (delivery_variant)
         (delivery_details)
         (order_value)
         (created)
         (last_updated)
         (completed)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::purchase_order_object, node::chain::purchase_order_index );

FC_REFLECT( node::chain::savings_withdraw_object,
         (id)
         (from)
         (to)
         (memo)
         (request_id)
         (amount)
         (complete)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::savings_withdraw_object, node::chain::savings_withdraw_index );

FC_REFLECT( node::chain::unstake_asset_route_object,
         (id)
         (from)
         (to)
         (symbol)
         (percent)
         (auto_stake)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::unstake_asset_route_object, node::chain::unstake_asset_route_index );

FC_REFLECT( node::chain::decline_voting_rights_request_object,
         (id)
         (account)
         (effective_date) 
         );
            
CHAINBASE_SET_INDEX_TYPE( node::chain::decline_voting_rights_request_object, node::chain::decline_voting_rights_request_index );

FC_REFLECT_ENUM( node::chain::curve_id,
         (quadratic)
         (quadratic_curation)
         (linear)
         (square_root)
         (convergent_semi_quadratic)
         );

FC_REFLECT( node::chain::reward_fund_object,
         (id)
         (content_reward_balance)
         (validation_reward_balance) 
         (txn_stake_reward_balance) 
         (work_reward_balance)
         (producer_activity_reward_balance) 
         (supernode_reward_balance)
         (power_reward_balance)
         (community_fund_balance)
         (development_reward_balance)
         (marketing_reward_balance)
         (advocacy_reward_balance)
         (activity_reward_balance)
         (premium_partners_fund_balance)
         (total_pending_reward_balance)
         (recent_content_claims)
         (recent_activity_claims)
         (content_constant)
         (content_reward_decay_rate)
         (author_reward_curve)
         (curation_reward_curve)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::reward_fund_object, node::chain::reward_fund_index );