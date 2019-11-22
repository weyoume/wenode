/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 */
#pragma once
#include <node/chain/global_property_object.hpp>
//#include <node/chain/hardfork.hpp>
#include <node/chain/node_property_object.hpp>
#include <node/chain/fork_database.hpp>
#include <node/chain/block_log.hpp>
#include <node/chain/operation_notification.hpp>

#include <node/protocol/protocol.hpp>

#include <fc/signals.hpp>

#include <fc/log/logger.hpp>

#include <map>

namespace node { namespace chain {

   using node::protocol::signed_transaction;
   using node::protocol::operation;
   using node::protocol::authority;
   using node::protocol::asset;
   using node::protocol::asset_symbol_type;
   using node::protocol::price;

   class database_impl;
   class custom_operation_interpreter;

   namespace util {
      struct comment_reward_context;
   }

   inline uint8_t find_msb( const uint128_t& u )
   {
      uint64_t x;
      uint8_t places;
      x      = (u.lo ? u.lo : 1);
      places = (u.hi ?   64 : 0);
      x      = (u.hi ? u.hi : x);
      return uint8_t( boost::multiprecision::detail::find_msb(x) + places );
   }

   inline uint64_t approx_sqrt( const uint128_t& x )
   {
      if( (x.lo == 0) && (x.hi == 0) )
         return 0;

      uint8_t msb_x = find_msb(x);
      uint8_t msb_z = msb_x >> 1;

      uint128_t msb_x_bit = uint128_t(1) << msb_x;
      uint64_t  msb_z_bit = uint64_t (1) << msb_z;

      uint128_t mantissa_mask = msb_x_bit - 1;
      uint128_t mantissa_x = x & mantissa_mask;
      uint64_t mantissa_z_hi = (msb_x & 1) ? msb_z_bit : 0;
      uint64_t mantissa_z_lo = (mantissa_x >> (msb_x - msb_z)).lo;
      uint64_t mantissa_z = (mantissa_z_hi | mantissa_z_lo) >> 1;
      uint64_t result = msb_z_bit | mantissa_z;

      return result;
   }

   /**
    *   @class database
    *   @brief tracks the blockchain state in an extensible manner
    */
   class database : public chainbase::database
   {
      public:

         //=======================//
         // === NODE DATABASE === //
         //=======================//

         database();
         ~database();

         bool is_producing()const { return _is_producing; }
         void set_producing( bool p ) { _is_producing = p;  }
         bool _is_producing = false;

         bool _log_hardforks = true;

         enum validation_steps
         {
            skip_nothing                = 0,
            skip_witness_signature      = 1 << 0,  ///< used while reindexing
            skip_transaction_signatures = 1 << 1,  ///< used by non-witness nodes
            skip_transaction_dupe_check = 1 << 2,  ///< used while reindexing
            skip_fork_db                = 1 << 3,  ///< used while reindexing
            skip_block_size_check       = 1 << 4,  ///< used when applying locally generated transactions
            skip_tapos_check            = 1 << 5,  ///< used while reindexing -- note this skips expiration check as well
            skip_authority_check        = 1 << 6,  ///< used while reindexing -- disables any checking of authority on transactions
            skip_merkle_check           = 1 << 7,  ///< used while reindexing
            skip_undo_history_check     = 1 << 8,  ///< used while reindexing
            skip_witness_schedule_check = 1 << 9,  ///< used while reindexing
            skip_validate               = 1 << 10, ///< used prior to checkpoint, skips validate() call on transaction
            skip_validate_invariants    = 1 << 11, ///< used to skip database invariant check on block application
            skip_undo_block             = 1 << 12, ///< used to skip undo db on reindex
            skip_block_log              = 1 << 13  ///< used to skip block logging on reindex
         };

         /**
          * @brief Open a database, creating a new one if necessary
          *
          * Opens a database in the specified directory. If no initialized database is found the database
          * will be initialized with the default state.
          *
          * @param data_dir Path to open or create database in
          */
         void open( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size = 0, uint32_t chainbase_flags = 0 );


         /**
          * Begins a new blockchain and creates initial objects for the network.
          */
         void init_genesis();

         /**
          * @brief Rebuild object graph from block history and open detabase
          *
          * This method may be called after or instead of @ref database::open, and will rebuild the object graph by
          * replaying blockchain history. When this method exits successfully, the database will be open.
          */
         void reindex( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size = (1024l*1024l*1024l*8l) );

         /**
          * @brief wipe Delete database from disk, and potentially the raw chain as well.
          * @param include_blocks If true, delete the raw chain as well as the database.
          *
          * Will close the database before wiping. Database will be closed when this function returns.
          */
         void wipe(const fc::path& data_dir, const fc::path& shared_mem_dir, bool include_blocks);
         void close(bool rewind = true);

         //////////////////// db_block.cpp ////////////////////

         /**
          *  @return true if the block is in our fork DB or saved to disk as
          *  part of the official chain, otherwise return false
          */
         bool                                   is_known_block( const block_id_type& id )const;
         bool                                   is_known_transaction( const transaction_id_type& id )const;
         
         uint128_t                              pow_difficulty()const;
         block_id_type                          find_block_id_for_num( uint32_t block_num )const;
         block_id_type                          get_block_id_for_num( uint32_t block_num )const;
         optional<signed_block>                 fetch_block_by_id( const block_id_type& id )const;
         optional<signed_block>                 fetch_block_by_number( uint32_t num )const;
         const signed_transaction               get_recent_transaction( const transaction_id_type& trx_id )const;
         std::vector<block_id_type>             get_block_ids_on_fork(block_id_type head_of_fork) const;

         chain_id_type                          get_chain_id()const;
         const dynamic_global_property_object&  get_dynamic_global_properties()const;
         time_point                             head_block_time()const;
         time_point                             next_maintenance_time()const;
         uint32_t                               head_block_num()const;
         block_id_type                          head_block_id()const;
         const witness_schedule_object&         get_witness_schedule()const;
         const chain_properties&                chain_properties()const;
         const price&                           get_usd_price()const;
         const reward_fund_object&              get_reward_fund() const;
         const comment_metrics_object&          get_comment_metrics() const;

         const asset&                           asset_to_USD( const asset& a) const;
         const asset&                           USD_to_asset( const asset& a) const;
         
         const node_property_object&            get_node_properties()const;
         const feed_history_object&             get_feed_history()const;

         const asset_object& get_core_asset() const;
         const asset_object* find_core_asset() const;

         const asset_object& get_asset( const asset_symbol_type& symbol ) const;
         const asset_object* find_asset( const asset_symbol_type& symbol ) const;

         const asset_dynamic_data_object& get_core_dynamic_data() const;
         const asset_dynamic_data_object* find_core_dynamic_data() const;

         const asset_dynamic_data_object& get_dynamic_data( const asset_symbol_type& symbol ) const;
         const asset_dynamic_data_object* find_dynamic_data( const asset_symbol_type& symbol ) const;

         const asset_bitasset_data_object& get_bitasset_data( const asset_symbol_type& symbol ) const;
         const asset_bitasset_data_object* find_bitasset_data( const asset_symbol_type& symbol ) const;

         const asset_equity_data_object& get_equity_data( const asset_symbol_type& symbol ) const;
         const asset_equity_data_object* find_equity_data( const asset_symbol_type& symbol ) const;

         const asset_credit_data_object& get_credit_data( const asset_symbol_type& symbol ) const;
         const asset_credit_data_object* find_credit_data( const asset_symbol_type& symbol ) const;

         const witness_object& get_witness( const account_name_type& name ) const;
         const witness_object* find_witness( const account_name_type& name ) const;

         const block_validation_object& get_block_validation( const account_name_type& producer, uint32_t height ) const;
         const block_validation_object* find_block_validation( const account_name_type& producer, uint32_t height ) const;

         const account_object& get_account(  const account_name_type& name )const;
         const account_object* find_account( const account_name_type& name )const;

         const account_following_object& get_account_following( const account_name_type& account )const;
         const account_following_object* find_account_following( const account_name_type& account )const;

         const tag_following_object& get_tag_following( const tag_name_type& tag )const;
         const tag_following_object* find_tag_following( const tag_name_type& tag )const;

         const account_business_object& get_account_business( const account_name_type& account )const;
         const account_business_object* find_account_business( const account_name_type& account )const;

         const account_executive_vote_object& get_account_executive_vote( const account_name_type& account, const account_name_type& business, const account_name_type& executive )const;
         const account_executive_vote_object* find_account_executive_vote( const account_name_type& account, const account_name_type& business, const account_name_type& executive )const;

         const account_officer_vote_object& get_account_officer_vote( const account_name_type& account, const account_name_type& business, const account_name_type& officer )const;
         const account_officer_vote_object* find_account_officer_vote( const account_name_type& account, const account_name_type& business, const account_name_type& officer )const;

         const account_member_request_object& get_account_member_request( const account_name_type& account, const account_name_type& business )const;
         const account_member_request_object* find_account_member_request( const account_name_type& account, const account_name_type& business )const;

         const account_member_invite_object& get_account_member_invite( const account_name_type& member, const account_name_type& business )const;
         const account_member_invite_object* find_account_member_invite( const account_name_type& member, const account_name_type& business )const;

         const account_member_key_object& get_account_member_key( const account_name_type& account )const;
         const account_member_key_object* find_account_member_key( const account_name_type& account )const;

         const account_balance_object& get_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const;
         const account_balance_object* find_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const;

         const account_permission_object& get_account_permissions( const account_name_type& account )const;
         const account_permission_object* find_account_permissions( const account_name_type& account )const;

         const account_authority_object& get_account_authority( const account_name_type& account )const;
         const account_authority_object* find_account_authority( const account_name_type& account )const;

         const network_officer_object& get_network_officer( const account_name_type& account )const;
         const network_officer_object* find_network_officer( const account_name_type& account )const;

         const executive_board_object& get_executive_board( const account_name_type& account )const;
         const executive_board_object* find_executive_board( const account_name_type& account )const;

         const supernode_object& get_supernode( const account_name_type& account )const;
         const supernode_object* find_supernode( const account_name_type& account )const;

         const interface_object& get_interface( const account_name_type& account )const;
         const interface_object* find_interface( const account_name_type& account )const;

         const governance_account_object& get_governance_account( const account_name_type& name )const;
         const governance_account_object* find_governance_account( const account_name_type& name )const;

         const community_enterprise_object& get_community_enterprise( const account_name_type& creator, const shared_string& enterprise_id )const;
         const community_enterprise_object* find_community_enterprise( const account_name_type& creator, const shared_string& enterprise_id )const;

         const enterprise_approval_object& get_enterprise_approval( const account_name_type& creator, const shared_string& enterprise_id, const account_name_type& account )const;
         const enterprise_approval_object* find_enterprise_approval( const account_name_type& creator, const shared_string& enterprise_id, const account_name_type& account )const;

         const board_object& get_board( const board_name_type& board )const;
         const board_object* find_board( const board_name_type& board )const;

         const board_member_object& get_board_member( const board_name_type& board )const;
         const board_member_object* find_board_member( const board_name_type& board )const;

         const board_member_key_object& get_board_member_key( const account_name_type& member, const board_name_type& board )const;
         const board_member_key_object* find_board_member_key( const account_name_type& member, const board_name_type& board )const;

         const governance_account_object& get_governance_account( const account_name_type& name )const;
         const governance_account_object* find_governance_account( const account_name_type& name )const;

         const comment_object& get_comment(  const account_name_type& author, const shared_string& permlink )const;
         const comment_object* find_comment( const account_name_type& author, const shared_string& permlink )const;

         const comment_object& get_comment(  const account_name_type& author, const string& permlink )const;
         const comment_object* find_comment( const account_name_type& author, const string& permlink )const;

         const comment_vote_object& get_comment_vote( const account_name_type& voter, const comment_id_type& vote_id )const;
         const comment_vote_object* find_comment_vote( const account_name_type& voter, const comment_id_type& vote_id )const;

         const comment_view_object& get_comment_view( const account_name_type& viewer, const comment_id_type& view_id )const;
         const comment_view_object* find_comment_view( const account_name_type& viewer, const comment_id_type& view_id )const;

         const comment_share_object& get_comment_share( const account_name_type& sharer, const comment_id_type& share_id )const;
         const comment_share_object* find_comment_share( const account_name_type& sharer, const comment_id_type& share_id )const;

         const ad_creative_object& get_ad_creative( const account_name_type& account, const shared_string& creative_id )const;
         const ad_creative_object* find_ad_creative( const account_name_type& account, const shared_string& creative_id )const;

         const ad_campaign_object& get_ad_campaign( const account_name_type& account, const shared_string& campaign_id )const;
         const ad_campaign_object* find_ad_campaign( const account_name_type& account, const shared_string& campaign_id )const;

         const ad_inventory_object& get_ad_inventory( const account_name_type& account, const shared_string& inventory_id )const;
         const ad_inventory_object* find_ad_inventory( const account_name_type& account, const shared_string& inventory_id )const;

         const ad_audience_object& get_ad_audience( const account_name_type& account, const shared_string& audience_id )const;
         const ad_audience_object* find_ad_audience( const account_name_type& account, const shared_string& audience_id )const;

         const ad_bid_object& get_ad_bid( const account_name_type& account, const shared_string& bid_id )const;
         const ad_bid_object* find_ad_bid( const account_name_type& account, const shared_string& bid_id )const;

         const asset_liquidity_pool_object& get_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const;
         const asset_liquidity_pool_object* find_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const;

         const asset_liquidity_pool_object& get_liquidity_pool( const asset_symbol_type& symbol )const;
         const asset_liquidity_pool_object* find_liquidity_pool( const asset_symbol_type& symbol )const;

         const asset_credit_pool_object& get_credit_pool( const asset_symbol_type& symbol, bool credit_asset )const;
         const asset_credit_pool_object* find_credit_pool( const asset_symbol_type& symbol, bool credit_asset )const;

         const credit_collateral_object& get_collateral( const account_name_type& owner, const asset_symbol_type& symbol  )const;
         const credit_collateral_object* find_collateral( const account_name_type& owner, const asset_symbol_type& symbol )const;

         const credit_loan_object& get_loan( const account_name_type& owner, shared_string& loan_id  )const;
         const credit_loan_object* find_loan( const account_name_type& owner, shared_string& loan_id )const;

         const escrow_object& get_escrow(  const account_name_type& name, uint32_t escrow_id )const;
         const escrow_object* find_escrow( const account_name_type& name, uint32_t escrow_id )const;

         const limit_order_object& get_limit_order(  const account_name_type& owner, shared_string& order_id )const;
         const limit_order_object* find_limit_order( const account_name_type& owner, shared_string& order_id )const;

         const transfer_request_object& get_transfer_request( const account_name_type& name, shared_string& request_id )const;
         const transfer_request_object* find_transfer_request( const account_name_type& name, shared_string& request_id )const;

         const transfer_recurring_request_object& get_transfer_recurring_request( const account_name_type& name, shared_string& request_id )const;
         const transfer_recurring_request_object* find_transfer_recurring_request( const account_name_type& name, shared_string& request_id )const;

         const call_order_object& get_call_order( const account_name_type& name, const asset_symbol_type& symbol )const;
         const call_order_object* find_call_order( const account_name_type& name, const asset_symbol_type& symbol )const;

         const margin_order_object& get_margin_order(  const account_name_type& name, shared_string& margin_id )const;
         const margin_order_object* find_margin_order( const account_name_type& name, shared_string& margin_id )const;

         const savings_withdraw_object& get_savings_withdraw(  const account_name_type& owner, uint32_t request_id )const;
         const savings_withdraw_object* find_savings_withdraw( const account_name_type& owner, uint32_t request_id )const;

         annotated_signed_transaction get_transaction( const transaction_id_type& id )const;
         
         const hardfork_property_object&        get_hardfork_property_object()const;

         const time_point                       calculate_discussion_payout_time( const comment_object& comment )const;

         node_property_object& node_properties();

         uint32_t last_non_undoable_block_num() const;
         //////////////////// db_init.cpp ////////////////////

         void initialize_evaluators();
         void set_custom_operation_interpreter( const std::string& id, std::shared_ptr< custom_operation_interpreter > registry );
         std::shared_ptr< custom_operation_interpreter > get_custom_json_evaluator( const std::string& id );

         /// Reset the object graph in-memory
         void initialize_indexes();
         void init_schema();
         
         /**
          *  This method validates transactions without adding it to the pending state.
          *  @throw if an error occurs
          */
         void validate_transaction( const signed_transaction& trx );

         /** when popping a block, the transactions that were removed get cached here so they
          * can be reapplied at the proper time */
         std::deque< signed_transaction >       _popped_tx;

         bool has_hardfork( uint32_t hardfork )const;

         /* For testing and debugging only. Given a hardfork
            with id N, applies all hardforks with id <= N */
         void set_hardfork( uint32_t hardfork, bool process_now = true );

         

         const std::string& get_json_schema() const;

         void set_flush_interval( uint32_t flush_blocks );
         void show_free_memory( bool force );
         

         /**
          *  Calculate the percent of block production slots that were missed in the
          *  past 128 blocks, not including the current block.
          */
         uint32_t witness_participation_rate()const;

         void                                   add_checkpoints( const flat_map<uint32_t,block_id_type>& checkpts );
         const flat_map<uint32_t,block_id_type> get_checkpoints()const { return _checkpoints; }
         bool                                   before_last_checkpoint()const;

         bool push_block( const signed_block& b, uint32_t skip = skip_nothing );
         void push_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );
         void _maybe_warn_multiple_production( uint32_t height )const;
         bool _push_block( const signed_block& b );
         void _push_transaction( const signed_transaction& trx );

         signed_block generate_block(
            const fc::time_point when,
            const account_name_type& witness_owner,
            const fc::ecc::private_key& block_signing_private_key,
            uint32_t skip
            );
         signed_block _generate_block(
            const fc::time_point when,
            const account_name_type& witness_owner,
            const fc::ecc::private_key& block_signing_private_key
            );

         void pop_block();
         void clear_pending();

         /**
          *  This method is used to track applied operations during the evaluation of a block, these
          *  operations should include any operation actually included in a transaction as well
          *  as any implied/virtual operations that resulted, such as filling an order.
          *  The applied operations are cleared after post_apply_operation.
          */
         void notify_pre_apply_operation( operation_notification& note );
         void notify_post_apply_operation( const operation_notification& note );
         void push_virtual_operation( const operation& op, bool force = false ); // vops are not needed for low mem. Force will push them on low mem.
         void notify_pre_apply_block( const signed_block& block );
         void notify_applied_block( const signed_block& block );
         void notify_on_pending_transaction( const signed_transaction& tx );
         void notify_on_pre_apply_transaction( const signed_transaction& tx );
         void notify_on_applied_transaction( const signed_transaction& tx );

         /**
          *  This signal is emitted for plugins to process every operation after it has been fully applied.
          */
         fc::signal<void(const operation_notification&)> pre_apply_operation;
         fc::signal<void(const operation_notification&)> post_apply_operation;

         fc::signal<void(const signed_block&)>           pre_apply_block;

         /**
          *  This signal is emitted after all operations and virtual operation for a
          *  block have been applied but before the get_applied_operations() are cleared.
          *
          *  You may not yield from this callback because the blockchain is holding
          *  the write lock and may be in an "inconstant state" until after it is
          *  released.
          */
         fc::signal<void(const signed_block&)>           applied_block;

         /**
          * This signal is emitted any time a new transaction is added to the pending
          * block state.
          */
         fc::signal<void(const signed_transaction&)>     on_pending_transaction;

         /**
          * This signla is emitted any time a new transaction is about to be applied
          * to the chain state.
          */
         fc::signal<void(const signed_transaction&)>     on_pre_apply_transaction;

         /**
          * This signal is emitted any time a new transaction has been applied to the
          * chain state.
          */
         fc::signal<void(const signed_transaction&)>     on_applied_transaction;

         //////////////////// db_witness_schedule.cpp ////////////////////

         /**
          * @brief Get the witness scheduled for block production in a slot.
          *
          * slot_num always corresponds to a time in the future.
          *
          * If slot_num == 1, returns the next scheduled witness.
          * If slot_num == 2, returns the next scheduled witness after
          * 1 block gap.
          *
          * Use the get_slot_time() and get_slot_at_time() functions
          * to convert between slot_num and timestamp.
          *
          * Passing slot_num == 0 returns NULL_WITNESS
          */
         account_name_type get_scheduled_witness(uint32_t slot_num)const;

         /**
          * Get the time at which the given slot occurs.
          *
          * If slot_num == 0, return time_point().
          *
          * If slot_num == N for N > 0, return the Nth next
          * block-interval-aligned time greater than head_block_time().
          */
         fc::time_point get_slot_time(uint32_t slot_num)const;

         /**
          * Get the last slot which occurs AT or BEFORE the given time.
          *
          * The return value is the greatest value N such that
          * get_slot_time( N ) <= when.
          *
          * If no such N exists, return 0.
          */
         uint32_t get_slot_at_time(fc::time_point when)const;

         share_type update_witness( const witness_object& witness, const witness_schedule_object& wso, const dynamic_global_property_object& props );

         void update_witness_set();

         void update_board_moderators( const board_member_object& board );

         void update_board_moderator_set();

         void update_business_account( const account_business_object& business, const dynamic_global_property_object& props );

         void update_business_account_set();

         void adjust_interface_users( const interface_object& interface, bool adjust = true );

         /** clears all vote records for a particular account */
         void clear_network_votes( const account_object& a );   //

         void process_funds();
         
         void process_equity_rewards();

         void process_power_rewards();
         
         void update_proof_of_work_target();
         
         void process_txn_stake_rewards();
         
         void process_validation_rewards();
         
         void process_producer_activity_rewards();

         void update_network_officer( const network_officer_object& network_officer, 
            const witness_schedule_object& wso, const dynamic_global_property_object& props );

         void process_network_officer_rewards();

         void process_supernode_rewards();

         void adjust_view_weight( const supernode_object& supernode, share_type delta, bool adjust = true );

         void process_community_enterprise_fund();
         
         void update_enterprise( const community_enterprise_object& enterprise, 
            const witness_schedule_object& wso, const dynamic_global_property_object& props );

         void process_executive_board_budgets();

         void update_executive_board( const executive_board_object& executive_board, 
            const witness_schedule_object& wso, const dynamic_global_property_object& props );

         void update_governance_account_set();

         void update_governance_account( const governance_account_object& network_officer, 
            const witness_schedule_object& wso, const dynamic_global_property_object& props );

         share_type get_equity_shares( const account_balance_object& balance, const asset_equity_data_object& equity );

         void claim_proof_of_work_reward( const account_name_type& miner );

         asset calculate_issuer_fee( const asset_object& trade_asset, const asset& trade_amount );

         asset pay_issuer_fees( const asset_object& recv_asset, const asset& receives );

         asset pay_issuer_fees( const account_object& seller, const asset_object& recv_asset, const asset& receives );

         asset pay_network_fees( const account_object& payer, const asset& amount );

         asset pay_network_fees( const asset& amount );

         asset pay_trading_fees( const account_object& taker, const asset& receives, const account_name_type& maker_int, 
            const account_name_type& taker_int );

         asset pay_advertising_delivery( const account_object& provider, const account_object& demand, 
            const account_object& bidder, const account_object& delivery, flat_set< const account_object* > audience, const asset& value );

         asset pay_fee_share( const account_object& payee, const asset& amount );

         asset pay_multi_fee_share( flat_set< const account_object* > payees, const asset& amount );

         void cancel_ad_bid( const ad_bid_object& bid );

         void cancel_community_enterprise( const community_enterprise_object& e );

         void validate_invariants()const;

         

         //==========================//
         // === ACCOUNT DATABASE === //
         //==========================//



         void process_membership_updates();

         void update_account_reputations();

         asset pay_membership_fees( const account_object& member, const asset& payment, const account_object& interface );

         asset claim_activity_reward( const account_object& account, const witness_object& witness );

         void update_owner_authority( const account_object& account, const authority& owner_authority );

         void update_witness_votes(const account_object& account );
         void update_witness_votes(const account_object& account, const account_name_type& witness, uint16_t vote_rank );

         void update_network_officer_votes(const account_object& account );
         void update_network_officer_votes(const account_object& account, const account_name_type& officer, 
            network_officer_types officer_type, uint16_t vote_rank );

         void update_executive_board_votes(const account_object& account );
         void update_executive_board_votes(const account_object& account, const account_name_type& executive, uint16_t vote_rank );

         void update_board_moderator_votes(const account_object& account, const board_name_type& board );
         void update_board_moderator_votes(const account_object& account, const board_name_type& board, 
            const account_name_type& moderator, uint16_t vote_rank );

         void update_enterprise_votes(const account_object& account );
         void update_enterprise_votes(const account_object& account, const account_name_type& creator, const shared_string& enterprise_id, uint16_t vote_rank );

         void update_account_executive_votes( const account_object& account, const account_name_type& business );
         void update_account_executive_votes( const account_object& account, const account_name_type& business, const account_object& executive,
            executive_types role, uint16_t input_vote_rank );

         void update_account_officer_votes( const account_object& account, const account_name_type& business );
         void update_account_officer_votes( const account_object& account, const account_name_type& business, const account_object& officer, uint16_t input_vote_rank );

         void account_recovery_processing();

         void process_decline_voting_rights();

         void clear_witness_votes( const account_object& a );



         //==========================//
         // === COMMENT DATABASE === //
         //==========================//



         share_type pay_voters( const comment_object& c, const share_type& max_rewards );

         share_type pay_viewers( const comment_object& c, const share_type& max_rewards );

         share_type pay_sharers( const comment_object& c, const share_type& max_rewards );

         share_type pay_commenters( const comment_object& c, const share_type& max_rewards );

         share_type pay_moderators( const comment_object& c, const share_type& max_rewards );

         share_type pay_storage( const comment_object& c, const share_type& max_rewards );

         share_type pay_content_rewards( share_type reward );

         share_type distribute_comment_reward( util::comment_reward_context& ctx, const comment_object& comment );

         void process_comment_cashout();

         void update_comment_metrics();

         void add_comment_to_feeds( const comment_object& comment );

         void share_comment_to_feeds( const account_name_type& sharer, const feed_types& reach, const comment_object& comment );

         void share_comment_to_board( const account_name_type& sharer, 
            const board_name_type& board, const comment_object& comment );

         void share_comment_to_tag( const account_name_type& sharer, 
            const tag_name_type& tag, const comment_object& comment );

         void clear_comment_feeds( const comment_object& comment );

         void remove_shared_comment( const account_name_type& sharer, const comment_object& comment );

         void update_account_in_feed( const account_name_type& account, const account_name_type& followed );

         void update_tag_in_feed( const account_name_type& account, const tag_name_type& tag );

         void update_board_in_feed( const account_name_type& account, const board_name_type& board );

         void adjust_total_payout( const comment_object& a, const asset& USD, const asset& curator_USD_value, const asset& beneficiary_value );

         void adjust_reward_shares( const comment_object& comment, fc::uint128_t old_reward_shares, fc::uint128_t new_reward_shares );



         //========================//
         // === ASSET DATABASE === //
         //========================//



         void process_asset_staking();

         void process_recurring_transfers();

         void process_savings_withdraws();

         void expire_escrow_ratification();

         void update_median_liquidity();
         
         void process_credit_buybacks();
         
         void process_credit_interest();

         void update_expired_feeds();

         void clear_expired_delegations();
         
         void update_core_exchange_rates();
         
         void update_maintenance_flag( bool new_maintenance_flag );

         void adjust_liquid_balance( const account_name_type& a, const asset& delta );
         void adjust_liquid_balance( const account_object& a, const asset& delta );

         void adjust_staked_balance( const account_name_type& a, const asset& delta );
         void adjust_staked_balance( const account_object& a, const asset& delta );

         void adjust_savings_balance( const account_name_type& a, const asset& delta );
         void adjust_savings_balance( const account_object& a, const asset& delta );
         
         void adjust_reward_balance( const account_name_type& a, const asset& delta );
         void adjust_reward_balance( const account_object& a, const asset& delta );

         void adjust_delegated_balance( const account_name_type& a, const asset& delta );
         void adjust_delegated_balance( const account_object& a, const asset& delta );

         void adjust_receiving_balance( const account_name_type& a, const asset& delta );
         void adjust_receiving_balance( const account_object& a, const asset& delta );

         void adjust_pending_supply( const asset& delta );

         asset get_liquid_balance( const account_object& a, const asset_symbol_type& symbol)const;
         asset get_liquid_balance( const account_name_type& a, const asset_symbol_type& symbol)const;

         asset get_staked_balance( const account_object& a, const asset_symbol_type& symbol )const;
         asset get_staked_balance( const account_name_type& a, const asset_symbol_type& symbol)const;

         asset get_reward_balance( const account_object& a, const asset_symbol_type& symbol )const;
         asset get_reward_balance( const account_name_type& a, const asset_symbol_type& symbol)const;

         asset get_savings_balance( const account_object& a, const asset_symbol_type& symbol )const;
         asset get_savings_balance( const account_name_type& a, const asset_symbol_type& symbol)const;

         asset get_delegated_balance( const account_object& a, const asset_symbol_type& symbol )const;
         asset get_delegated_balance( const account_name_type& a, const asset_symbol_type& symbol)const;

         asset get_receiving_balance( const account_object& a, const asset_symbol_type& symbol )const;
         asset get_receiving_balance( const account_name_type& a, const asset_symbol_type& symbol)const;

         share_type get_voting_power( const account_object& a )const;
         share_type get_voting_power( const account_name_type& a )const;
         share_type get_voting_power( const account_object& a, const price& equity_price )const;
         share_type get_voting_power( const account_name_type& a, const price& equity_price )const;

         share_type get_proxied_voting_power( const account_object& a, const price& equity_price )const;
         share_type get_proxied_voting_power( const account_name_type& a, const price& equity_price )const;

         share_type get_equity_voting_power( const account_object& a, const account_business_object& b )const;
         share_type get_equity_voting_power( const account_name_type& a, const account_business_object& b )const;

         string to_pretty_string( const asset& a )const;



         //==========================//
         // === TRADING DATABASE === //
         //==========================//

         

         bool apply_order(const limit_order_object& new_order_object );
         bool apply_order(const margin_order_object& new_order_object );

         int match( const limit_order_object& taker, const limit_order_object& maker, const price& match_price );
         int match( const margin_order_object& taker, const margin_order_object& maker, const price& match_price );
         int match( const limit_order_object& taker, const margin_order_object& maker, const price& match_price );
         int match( const margin_order_object& taker, const limit_order_object& maker, const price& match_price );
         int match( const limit_order_object& order, const asset_liquidity_pool_object& pool, const price& match_price );
         int match( const margin_order_object& order, const asset_liquidity_pool_object& pool, const price& match_price );
         int match( const limit_order_object& bid, const call_order_object& ask, const price& match_price,
                     const price& feed_price, const uint16_t maintenance_collateral_ratio,
                     const optional<price>& maintenance_collateralization );
         int match( const margin_order_object& bid, const call_order_object& ask, const price& match_price,
                     const price& feed_price, const uint16_t maintenance_collateral_ratio,
                     const optional<price>& maintenance_collateralization );
         asset match( const call_order_object& call, const force_settlement_object& settle, const price& match_price, 
            asset max_settlement, const price& fill_price );

         bool fill_limit_order( const limit_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface );

         bool fill_margin_order( const margin_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface );

         bool fill_call_order( const call_order_object& order, const asset& pays, const asset& receives,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface, bool global_settle );
         
         bool fill_settle_order( const force_settlement_object& settle, const asset& pays, const asset& receives,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface );

         void liquid_fund( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool);

         void liquid_withdraw( const asset& input, const asset_symbol_type& receive, 
            const account_object& account, const asset_liquidity_pool_object& pool);

         asset liquid_exchange( const asset& input, const asset_symbol_type& receive, bool execute = true, bool apply_fees = true );
         void liquid_exchange( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool, 
            const account_object& int_account);
         
         asset liquid_acquire( const asset& receive, const asset_symbol_type& input, bool execute = true, bool apply_fees = true );
         void liquid_acquire( const asset& receive, const account_object& account, const asset_liquidity_pool_object& pool, 
            const account_object& int_account);

         pair< asset, asset > liquid_limit_exchange( const asset& input, const price& limit_price, 
            const asset_liquidity_pool_object& pool, bool execute = true, bool apply_fees = true );
         void liquid_limit_exchange( const asset& input, const price& limit_price, const account_object& account, 
            const asset_liquidity_pool_object& pool, const account_object& int_account );

         void credit_lend( const asset& input, const account_object& account, const asset_credit_pool_object& pool);

         void credit_withdraw( const asset& input, const account_object& account, const asset_credit_pool_object& pool);

         bool credit_check( const asset& debt, const asset& collateral, const asset_credit_pool_object& credit_pool);

         bool margin_check( const asset& debt, const asset& position, const asset& collateral, const asset_credit_pool_object& credit_pool);

         void process_credit_updates();

         void process_margin_updates();

         void liquidate_credit_loan( const credit_loan_object& loan );

         asset network_credit_acquisition( const asset& amount, bool execute );

         bool check_call_orders( const asset_object& mia, bool enable_black_swan, bool for_new_limit_order );

         bool check_for_blackswan( const asset_object& mia, bool enable_black_swan, const asset_bitasset_data_object* bitasset_ptr );

         void globally_settle_asset( const asset_object& mia, const price& settlement_price );

         void revive_bitasset( const asset_object& bitasset );

         void cancel_bids_and_revive_mpa( const asset_object& bitasset, const asset_bitasset_data_object& bad );

         void cancel_bid(const collateral_bid_object& bid, bool create_virtual_op);

         void execute_bid( const collateral_bid_object& bid, share_type debt_covered, 
            share_type collateral_from_fund, const price_feed& current_feed );

         void cancel_settle_order(const force_settlement_object& order, bool create_virtual_op );

         void cancel_limit_order( const limit_order_object& order );

         bool maybe_cull_small_order( const limit_order_object& order );
         
         void close_margin_order( const margin_order_object& order );

         bool maybe_cull_small_order( const margin_order_object& order );
         

#ifdef IS_TEST_NET
         bool liquidity_rewards_enabled = true;
         bool skip_price_feed_limit_check = true;
         bool skip_transaction_delta_check = true;
#endif

      protected:
         //Mark pop_undo() as protected -- we do not want outside calling pop_undo(); it should call pop_block() instead
         //void pop_undo() { object_database::pop_undo(); }
         void notify_changed_objects();

      private:
         optional< chainbase::database::session > _pending_tx_session;

         void apply_block( const signed_block& next_block, uint32_t skip = skip_nothing );
         void apply_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );
         void _apply_block( const signed_block& next_block );
         void _apply_transaction( const signed_transaction& trx );
         void apply_operation( const operation& op );
         void update_stake(const signed_transaction& trx );

         ///Steps involved in applying a new block

         const witness_object& validate_block_header( uint32_t skip, const signed_block& next_block )const;
         void create_block_summary(const signed_block& next_block);

         void update_global_dynamic_data( const signed_block& b );
         void update_signing_witness(const witness_object& signing_witness, const signed_block& new_block);
         void update_transaction_stake(const witness_object& signing_witness, const uint128_t& transaction_stake);
         void update_last_irreversible_block();
         void clear_expired_transactions();
         void clear_expired_operations();
         void update_withdraw_permissions();
         void clear_expired_htlcs();

         void process_header_extensions( const signed_block& next_block );

         void init_hardforks();
         void process_hardforks();
         void apply_hardfork( uint32_t hardfork );

         ///@}

         std::unique_ptr< database_impl > _my;

         vector< signed_transaction >  _pending_tx;
         fork_database                 _fork_db;
         fc::time_point                _hardfork_times[ NUM_HARDFORKS + 1 ];
         protocol::hardfork_version    _hardfork_versions[ NUM_HARDFORKS + 1 ];

         block_log                     _block_log;

         // this function needs access to _plugin_index_signal
         template< typename MultiIndexType >
         friend void add_plugin_index( database& db );

         fc::signal< void() >          _plugin_index_signal;

         transaction_id_type           _current_trx_id;
         uint32_t                      _current_block_num           = 0;
         uint16_t                      _current_trx_in_block        = 0;
         uint16_t                      _current_op_in_trx           = 0;
         uint16_t                      _current_virtual_op          = 0;
         uint128_t                     _current_trx_stake_weight    = 0;

         flat_map<uint32_t,block_id_type>  _checkpoints;

         node_property_object              _node_property_object;

         uint32_t                      _flush_blocks = 0;
         uint32_t                      _next_flush_block = 0;

         uint32_t                      _last_free_gb_printed = 0;

         flat_map< std::string, std::shared_ptr< custom_operation_interpreter > >   _custom_operation_interpreters;
         std::string                       _json_schema;
   };

} }
