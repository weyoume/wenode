#pragma once
#include <node/chain/global_property_object.hpp>
#include <node/chain/hardfork.hpp>
#include <node/chain/node_property_object.hpp>
#include <node/chain/fork_database.hpp>
#include <node/chain/block_log.hpp>
#include <node/chain/operation_notification.hpp>
#include <node/protocol/protocol.hpp>
#include <node/chain/node_objects.hpp>

#include <fc/signals.hpp>

#include <fc/log/logger.hpp>

#include <map>

namespace node { namespace chain {

   class database_impl;
   class custom_operation_interpreter;

   namespace util {
      struct comment_reward_context;
   }


   /**
    * Database: Tracks the Blockchain State.
    * 
    * The Database uses a Structure of 
    * Multi-Indexed containers of Objects, 
    * which can be updated using a series of Evaluators, 
    * which accept input from Signed Transactions that contain
    * a set of Operations. Blocks contain a set of Signed Transactions
    * and are broadcast by all network nodes, and signed by a group of 
    * elected block producers. 
    */
   class database : public chainbase::database
   {
      public:

         //=======================//
         // === NODE DATABASE === //
         //=======================//
         
         
         database();
         ~database();

         /**
          * @brief Open a database, creating a new one if necessary.
          *
          * Opens a database in the specified directory. If no initialized database is found the database
          * will be initialized with the default state.
          *
          * @param data_dir Path to open or create database in
          * @param shared_mem_dir Path to open or create shared memory
          * @param shared_file_size Size in bytes of the share memory file
          * @param chainbase_flags Options to configure database
          */
         void                                   open( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size, uint32_t chainbase_flags );

         /**
          * Begins a new blockchain and creates initial objects for the network using a specified initial public key.
          */
         void                                   init_genesis();

         /**
          * @brief Rebuild object graph from block history and open database
          * 
          * This method may be called after or instead of @ref database::open, and will rebuild the object graph by
          * replaying blockchain history. When this method exits successfully, the database will be open.
          * 
          * @param data_dir Path to reindex database in
          * @param shared_mem_dir Path to reindex shared memory
          * @param shared_file_size Size in bytes of the share memory file
          */
         void                                   reindex( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size = (1024l*1024l*1024l*8l) );

         /**
          * @brief wipe Delete database from disk, and potentially the raw chain as well.
          * 
          * @param data_dir Path to wipe database from
          * @param shared_mem_dir Path to wipe shared memory from
          * @param include_blocks If true, delete the raw chain as well as the database.
          *
          * Will close the database before wiping. Database will be closed when this function returns.
          */
         void                                   wipe( const fc::path& data_dir, const fc::path& shared_mem_dir, bool include_blocks );

         void                                   close(bool rewind = true);

         bool                                   is_known_block( const block_id_type& id )const;

         bool                                   is_known_transaction( const transaction_id_type& id )const;

         block_id_type                          find_block_id_for_num( uint64_t block_num )const;

         block_id_type                          get_block_id_for_num( uint64_t block_num )const;

         optional<signed_block>                 fetch_block_by_id( const block_id_type& id )const;

         optional<signed_block>                 fetch_block_by_number( uint64_t num )const;

         const signed_transaction               get_recent_transaction( const transaction_id_type& trx_id )const;

         annotated_signed_transaction           get_transaction( const transaction_id_type& id )const;
         
         std::vector<block_id_type>             get_block_ids_on_fork( block_id_type head_of_fork )const;

         chain_id_type                          get_chain_id()const;

         const dynamic_global_property_object&  get_dynamic_global_properties()const;

         time_point                             head_block_time()const;

         uint64_t                               head_block_num()const;

         block_id_type                          head_block_id()const;
         
         const hardfork_property_object&        get_hardfork_property_object()const;

         node_property_object&                  node_properties();

         const node_property_object&            get_node_properties()const;

         uint64_t                               last_non_undoable_block_num()const;

         const transaction_id_type&             get_current_transaction_id()const;

         uint16_t                               get_current_op_in_trx()const;

         vector< account_name_type >            shuffle_accounts( vector< account_name_type > accounts )const;

         void                                   add_checkpoints( const flat_map<uint64_t,block_id_type>& checkpts );

         bool                                   before_last_checkpoint()const;
         
         const flat_map<uint64_t,block_id_type> get_checkpoints()const { return _checkpoints; }

         bool                                   push_block( const signed_block& b, uint32_t skip = skip_nothing );

         void                                  _maybe_warn_multiple_production( uint64_t height )const;

         bool                                  _push_block( const signed_block& b );

         void                                   push_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );

         void                                  _push_transaction( const signed_transaction& trx );

         signed_block                           generate_block( const fc::time_point when, const account_name_type& producer_owner, const fc::ecc::private_key& block_signing_private_key, uint32_t skip );

         signed_block                          _generate_block( const fc::time_point when, const account_name_type& producer_owner, const fc::ecc::private_key& block_signing_private_key);

         void                                   pop_block();

         void                                   clear_pending();

         /**
          * This method is used to track applied operations during the evaluation of a block, these
          * operations should include any operation actually included in a transaction as well
          * as any implied/virtual operations that resulted, such as filling an order.
          * The applied operations are cleared after post_apply_operation.
          */
         void                                   notify_pre_apply_operation( operation_notification& note );

         void                                   notify_post_apply_operation( const operation_notification& note );

         void                                   push_virtual_operation( const operation& op, bool force = false ); // vops are not needed for low mem. Force will push them on low mem.
         
         void                                   notify_pre_apply_block( const signed_block& block );

         void                                   notify_applied_block( const signed_block& block );

         void                                   notify_on_pending_transaction( const signed_transaction& tx );

         void                                   notify_on_pre_apply_transaction( const signed_transaction& tx );

         void                                   notify_on_applied_transaction( const signed_transaction& tx );
         
         void                                   initialize_evaluators();

         void                                   set_custom_operation_interpreter( const std::string& id, std::shared_ptr< custom_operation_interpreter > registry );

         std::shared_ptr< custom_operation_interpreter > get_custom_json_evaluator( const std::string& id );

         void                                   initialize_indexes();

         const std::string&                     get_json_schema()const;
         
         /**
          * This method validates transactions without adding it to the pending state.
          */
         void                                   validate_transaction( const signed_transaction& trx );

         void                                   set_flush_interval( uint32_t flush_blocks );

         void                                   show_free_memory( bool force );

         bool                                   has_hardfork( uint32_t hardfork )const;

         /**
          * For testing and debugging only. Given a hardfork with id N, applies all hardforks with id <= N 
          */
         void                                   set_hardfork( uint32_t hardfork, bool process_now = true );

         void                                   check_namespace( const account_name_type& name );

         void                                   validate_invariants()const;

         /**
          *  This signal is emitted for plugins to process every operation after it has been fully applied.
          */
         fc::signal<void( const operation_notification&)> pre_apply_operation;
         fc::signal<void( const operation_notification&)> post_apply_operation;

         fc::signal<void( const signed_block&)>           pre_apply_block;

         /**
          *  This signal is emitted after all operations and virtual operation for a
          *  block have been applied but before the get_applied_operations() are cleared.
          *
          *  You may not yield from this callback because the blockchain is holding
          *  the write lock and may be in an "inconstant state" until after it is
          *  released.
          */
         fc::signal<void( const signed_block&)>           applied_block;

         /**
          * This signal is emitted any time a new transaction is added to the pending
          * block state.
          */
         fc::signal<void( const signed_transaction&)>     on_pending_transaction;

         /**
          * This signla is emitted any time a new transaction is about to be applied
          * to the chain state.
          */
         fc::signal<void( const signed_transaction&)>     on_pre_apply_transaction;

         /**
          * This signal is emitted any time a new transaction has been applied to the
          * chain state.
          */
         fc::signal<void( const signed_transaction&)>     on_applied_transaction;

         //////////////////// db_producer_schedule.cpp ////////////////////

         /** 
          * When popping a block, the transactions that were removed get cached here so they can be reapplied at the proper time.
          */
         std::deque< signed_transaction >       _popped_tx;

         bool                                   is_producing()const { return _is_producing; }

         void                                   set_producing( bool p ) { _is_producing = p;  }

         bool                                   _is_producing = false;

         bool                                   _log_hardforks = true;

         enum validation_steps
         {
            skip_nothing                    = 0,
            skip_producer_signature         = 1 << 0,  ///< used while reindexing
            skip_transaction_signatures     = 1 << 1,  ///< used by non-producer nodes
            skip_transaction_dupe_check     = 1 << 2,  ///< used while reindexing
            skip_fork_db                    = 1 << 3,  ///< used while reindexing
            skip_block_size_check           = 1 << 4,  ///< used when applying locally generated transactions
            skip_tapos_check                = 1 << 5,  ///< used while reindexing -- note this skips expiration check as well
            skip_authority_check            = 1 << 6,  ///< used while reindexing -- disables any checking of authority on transactions
            skip_merkle_check               = 1 << 7,  ///< used while reindexing
            skip_undo_history_check         = 1 << 8,  ///< used while reindexing
            skip_producer_schedule_check    = 1 << 9,  ///< used while reindexing
            skip_validate                   = 1 << 10, ///< used prior to checkpoint, skips validate() call on transaction
            skip_validate_invariants        = 1 << 11, ///< used to skip database invariant check on block application
            skip_undo_block                 = 1 << 12, ///< used to skip undo db on reindex
            skip_block_log                  = 1 << 13  ///< used to skip block logging on reindex
         };


         //==========================//
         // === ACCOUNT DATABASE === //
         //==========================//

         

         const account_object&                        get_account(  const account_name_type& name )const;
         const account_object*                        find_account( const account_name_type& name )const;

         const account_authority_object&              get_account_authority( const account_name_type& account )const;
         const account_authority_object*              find_account_authority( const account_name_type& account )const;

         const account_permission_object&             get_account_permissions( const account_name_type& account )const;
         const account_permission_object*             find_account_permissions( const account_name_type& account )const;

         const account_verification_object&           get_account_verification( const account_name_type& verifier_account, const account_name_type& verified_account )const;
         const account_verification_object*           find_account_verification( const account_name_type& verifier_account, const account_name_type& verified_account )const;

         const account_following_object&              get_account_following( const account_name_type& account )const;
         const account_following_object*              find_account_following( const account_name_type& account )const;

         const account_tag_following_object&          get_account_tag_following( const tag_name_type& tag )const;
         const account_tag_following_object*          find_account_tag_following( const tag_name_type& tag )const;

         void process_membership_updates();

         asset pay_membership_fees( const account_object& member, const asset& payment, const account_object& interface );
         asset pay_membership_fees( const account_object& member, const asset& payment );

         void update_account_reputations();

         asset claim_activity_reward( const account_object& account, const producer_object& producer, asset_symbol_type currency_symbol );

         void update_owner_authority( const account_object& account, const authority& owner_authority );

         void account_recovery_processing();

         void process_account_decline_voting();

         void clear_account_votes( const account_name_type& a );



         //===========================//
         // === BUSINESS DATABASE === //
         //===========================//



         const business_object&                       get_business( const account_name_type& account )const;
         const business_object*                       find_business( const account_name_type& account )const;

         const business_permission_object&            get_business_permission( const account_name_type& account )const;
         const business_permission_object*            find_business_permission( const account_name_type& account )const;

         const business_executive_object&             get_business_executive( const account_name_type& business, const account_name_type& executive )const;
         const business_executive_object*             find_business_executive( const account_name_type& business, const account_name_type& executive )const;
         
         const business_executive_vote_object&        get_business_executive_vote( const account_name_type& director, const account_name_type& business )const;
         const business_executive_vote_object*        find_business_executive_vote( const account_name_type& director, const account_name_type& business )const;
         
         const business_director_object&              get_business_director( const account_name_type& business, const account_name_type& director )const;
         const business_director_object*              find_business_director( const account_name_type& business, const account_name_type& director )const;
         
         const business_director_vote_object&         get_business_director_vote( const account_name_type& account, const account_name_type& business, const account_name_type& director )const;
         const business_director_vote_object*         find_business_director_vote( const account_name_type& account, const account_name_type& business, const account_name_type& director )const;

         void update_business( const business_object& business );

         void update_business_set();

         void disolve_business( const business_object& business );

         void update_business_director_votes( const account_object& account, const account_name_type& business );
         void update_business_director_votes( const account_object& account, const account_name_type& business, const account_object& director, uint16_t input_vote_rank );



         //=============================//
         // === GOVERNANCE DATABASE === //
         //=============================//



         const governance_object&                       get_governance( const account_name_type& account )const;
         const governance_object*                       find_governance( const account_name_type& account )const;

         const governance_permission_object&            get_governance_permission( const account_name_type& account )const;
         const governance_permission_object*            find_governance_permission( const account_name_type& account )const;

         const governance_executive_object&             get_governance_executive( const account_name_type& governance, const account_name_type& executive )const;
         const governance_executive_object*             find_governance_executive( const account_name_type& governance, const account_name_type& executive )const;
         
         const governance_executive_vote_object&        get_governance_executive_vote( const account_name_type& director, const account_name_type& governance )const;
         const governance_executive_vote_object*        find_governance_executive_vote( const account_name_type& director, const account_name_type& governance )const;
         
         const governance_director_object&              get_governance_director( const account_name_type& governance, const account_name_type& director )const;
         const governance_director_object*              find_governance_director( const account_name_type& governance, const account_name_type& director )const;
         
         const governance_director_vote_object&         get_governance_director_vote( const account_name_type& account, const account_name_type& governance )const;
         const governance_director_vote_object*         find_governance_director_vote( const account_name_type& account, const account_name_type& governance )const;

         const governance_member_object&                get_governance_member( const account_name_type& account )const;
         const governance_member_object*                find_governance_member( const account_name_type& account )const;

         const governance_member_request_object&        get_governance_member_request( const account_name_type& account )const;
         const governance_member_request_object*        find_governance_member_request( const account_name_type& account )const;

         const governance_resolution_object&            get_governance_resolution( const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const;
         const governance_resolution_object*            find_governance_resolution( const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const;
         
         const governance_resolution_object&            get_governance_resolution( const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const;
         const governance_resolution_object*            find_governance_resolution( const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const;

         const governance_resolution_vote_object&       get_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const;
         const governance_resolution_vote_object*       find_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const string& resolution_id, const string& ammendment_id )const;

         const governance_resolution_vote_object&       get_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const;
         const governance_resolution_vote_object*       find_governance_resolution_vote( const account_name_type& account, const account_name_type& governance, const shared_string& resolution_id, const shared_string& ammendment_id )const;

         void update_governance( const governance_object& governance );

         void update_governance_set();

         void disolve_governance( const governance_object& governance );


         //==========================//
         // === NETWORK DATABASE === //
         //==========================//


         
         const network_officer_object&          get_network_officer( const account_name_type& account )const;
         const network_officer_object*          find_network_officer( const account_name_type& account )const;

         const network_officer_vote_object&     get_network_officer_vote( const account_name_type& account, const account_name_type& officer )const;
         const network_officer_vote_object*     find_network_officer_vote( const account_name_type& account, const account_name_type& officer )const;

         const supernode_object&                get_supernode( const account_name_type& account )const;
         const supernode_object*                find_supernode( const account_name_type& account )const;

         const interface_object&                get_interface( const account_name_type& account )const;
         const interface_object*                find_interface( const account_name_type& account )const;

         const mediator_object&                 get_mediator( const account_name_type& account )const;
         const mediator_object*                 find_mediator( const account_name_type& account )const;

         const enterprise_object&               get_enterprise( const account_name_type& account, const shared_string& enterprise_id )const;
         const enterprise_object*               find_enterprise( const account_name_type& account, const shared_string& enterprise_id )const;

         const enterprise_object&               get_enterprise( const account_name_type& account, const string& enterprise_id )const;
         const enterprise_object*               find_enterprise( const account_name_type& account, const string& enterprise_id )const;

         const enterprise_vote_object&          get_enterprise_vote( const account_name_type& account, const shared_string& enterprise_id, const account_name_type& voter )const;
         const enterprise_vote_object*          find_enterprise_vote( const account_name_type& account, const shared_string& enterprise_id, const account_name_type& voter )const;

         const enterprise_vote_object&          get_enterprise_vote( const account_name_type& account, const string& enterprise_id, const account_name_type& voter )const;
         const enterprise_vote_object*          find_enterprise_vote( const account_name_type& account, const string& enterprise_id, const account_name_type& voter )const;

         void network_officer_update_votes( const account_object& account );
         void network_officer_update_votes( const account_object& account, const account_name_type& officer, network_officer_role_type officer_type, uint16_t vote_rank );

         void update_enterprise_votes( const account_object& account );
         void update_enterprise_votes( const account_object& account, const account_name_type& creator, 
            string enterprise_id, uint16_t vote_rank );

         void adjust_view_weight( const supernode_object& supernode, share_type delta, bool adjust );

         void adjust_interface_users( const interface_object& interface, bool adjust );

         void network_officer_update( const network_officer_object& network_officer, 
            const producer_schedule_object& pso, const dynamic_global_property_object& props );

         void process_network_officer_rewards();

         void process_supernode_rewards();

         void update_enterprise( const enterprise_object& enterprise, 
            const producer_schedule_object& pso, const dynamic_global_property_object& props );

         void process_enterprise_fund();

         asset pay_fee_share( const account_object& payee, const asset& amount, bool recursive );

         asset pay_multi_fee_share( flat_set< const account_object* > payees, const asset& amount, bool recursive );

         asset pay_network_fees( const account_object& payer, const asset& amount );

         asset pay_network_fees( const asset& amount );
         
         asset network_credit_acquisition( const asset& amount, bool execute );



         //==========================//
         // === COMMENT DATABASE === //
         //==========================//



         const comment_object&                          get_comment(  const account_name_type& author, const shared_string& permlink )const;
         const comment_object*                          find_comment( const account_name_type& author, const shared_string& permlink )const;

         const comment_object&                          get_comment(  const account_name_type& author, const string& permlink )const;
         const comment_object*                          find_comment( const account_name_type& author, const string& permlink )const;

         const comment_vote_object&                     get_comment_vote( const account_name_type& voter, const comment_id_type& vote_id )const;
         const comment_vote_object*                     find_comment_vote( const account_name_type& voter, const comment_id_type& vote_id )const;

         const comment_view_object&                     get_comment_view( const account_name_type& viewer, const comment_id_type& view_id )const;
         const comment_view_object*                     find_comment_view( const account_name_type& viewer, const comment_id_type& view_id )const;

         const comment_share_object&                    get_comment_share( const account_name_type& sharer, const comment_id_type& share_id )const;
         const comment_share_object*                    find_comment_share( const account_name_type& sharer, const comment_id_type& share_id )const;

         const comment_moderation_object&               get_comment_moderation( const account_name_type& moderator, const comment_id_type& id )const;
         const comment_moderation_object*               find_comment_moderation( const account_name_type& moderator, const comment_id_type& id )const;

         const list_object&                             get_list( const account_name_type& creator, const shared_string& list_id )const;
         const list_object*                             find_list( const account_name_type& creator, const shared_string& list_id )const;
         
         const list_object&                             get_list( const account_name_type& creator, const string& list_id )const;
         const list_object*                             find_list( const account_name_type& creator, const string& list_id )const;
         
         const poll_object&                             get_poll( const account_name_type& creator, const shared_string& poll_id )const;
         const poll_object*                             find_poll( const account_name_type& creator, const shared_string& poll_id )const;

         const poll_object&                             get_poll( const account_name_type& creator, const string& poll_id )const;
         const poll_object*                             find_poll( const account_name_type& creator, const string& poll_id )const;

         const poll_vote_object&                        get_poll_vote( const account_name_type& voter, const account_name_type& creator, const shared_string& poll_id )const;
         const poll_vote_object*                        find_poll_vote( const account_name_type& voter, const account_name_type& creator, const shared_string& poll_id )const;
         
         const poll_vote_object&                        get_poll_vote( const account_name_type& voter, const account_name_type& creator, const string& poll_id )const;
         const poll_vote_object*                        find_poll_vote( const account_name_type& voter, const account_name_type& creator, const string& poll_id )const;

         const premium_purchase_object&                 get_premium_purchase( const account_name_type& account, const comment_id_type& id )const;
         const premium_purchase_object*                 find_premium_purchase( const account_name_type& account, const comment_id_type& id )const;

         const premium_purchase_key_object&             get_premium_purchase_key( const account_name_type& provider, const account_name_type& account, const comment_id_type& id )const;
         const premium_purchase_key_object*             find_premium_purchase_key( const account_name_type& provider, const account_name_type& account, const comment_id_type& id )const;

         const comment_metrics_object&                  get_comment_metrics()const;
         
         asset get_comment_reward( const comment_object& comment, const util::comment_reward_context& ctx )const;

         asset pay_voters( const comment_object& c, const asset& max_rewards );

         asset pay_viewers( const comment_object& c, const asset& max_rewards );

         asset pay_sharers( const comment_object& c, const asset& max_rewards );

         asset pay_commenters( const comment_object& c, const asset& max_rewards );

         asset pay_storage( const comment_object& c, const asset& max_rewards );

         asset pay_moderators( const comment_object& c, const asset& max_rewards );

         asset pay_content_rewards( asset reward );

         asset distribute_comment_reward( const comment_object& c, util::comment_reward_context& ctx );

         void process_comment_cashout();

         void update_comment_metrics();

         void update_message_counter();

         void add_comment_to_feeds( const comment_object& comment );

         void share_comment_to_feeds( const account_name_type& sharer, const feed_reach_type& reach, const comment_object& comment );

         void share_comment_to_community( const account_name_type& sharer, 
            const community_name_type& community, const comment_object& comment );

         void share_comment_to_tag( const account_name_type& sharer, 
            const tag_name_type& tag, const comment_object& comment );

         void clear_comment_feeds( const comment_object& comment );

         void remove_shared_comment( const account_name_type& sharer, const comment_object& comment );

         void update_account_in_feed( const account_name_type& account, const account_name_type& followed );

         void update_tag_in_feed( const account_name_type& account, const tag_name_type& tag );

         void update_community_in_feed( const account_name_type& account, const community_name_type& community );

         void adjust_total_payout( const comment_object& a, const asset& USD, const asset& curator_USD_value, const asset& beneficiary_value );

         void deliver_premium_purchase( const premium_purchase_object& purchase, const account_name_type& interface, const account_name_type& supernode );

         void cancel_premium_purchase( const premium_purchase_object& purchase );



         //============================//
         // === COMMUNITY DATABASE === //
         //============================//



         const community_object&                        get_community( const community_name_type& community )const;
         const community_object*                        find_community( const community_name_type& community )const;

         const community_permission_object&             get_community_permission( const community_name_type& community )const;
         const community_permission_object*             find_community_permission( const community_name_type& community )const;

         const community_event_object&                  get_community_event( const community_name_type& community, const shared_string& event_id )const;
         const community_event_object*                  find_community_event( const community_name_type& community, const shared_string& event_id )const;

         const community_event_object&                  get_community_event( const community_name_type& community, const string& event_id )const;
         const community_event_object*                  find_community_event( const community_name_type& community, const string& event_id )const;

         const community_directive_object&              get_community_directive( const account_name_type& account, const shared_string& directive_id )const;
         const community_directive_object*              find_community_directive( const account_name_type& account, const shared_string& directive_id )const;

         const community_directive_object&              get_community_directive( const account_name_type& account, const string& directive_id )const;
         const community_directive_object*              find_community_directive( const account_name_type& account, const string& directive_id )const;

         const community_directive_member_object&       get_community_directive_member( const account_name_type& member, const community_name_type& community )const;
         const community_directive_member_object*       find_community_directive_member( const account_name_type& member, const community_name_type& community )const;
         
         void update_community_member_votes( const account_object& account, const community_name_type& community );
         void update_community_member_votes( const account_object& account, const community_name_type& community, const account_name_type& moderator, uint16_t vote_rank );
         
         void update_community_moderators( const community_name_type& community );

         void update_community_moderator_set();

         void process_community_premium_membership( const community_member_object& member );

         void process_community_federation( const community_federation_object& federation );

         void remove_community_federation( const community_federation_object& federation );

         bool check_community_verification( const account_name_type& account, const community_permission_object& community, uint16_t depth = 0 );



         //=====================//
         // === AD DATABASE === //
         //=====================//



         const ad_creative_object&                      get_ad_creative( const account_name_type& account, const shared_string& creative_id )const;
         const ad_creative_object*                      find_ad_creative( const account_name_type& account, const shared_string& creative_id )const;

         const ad_creative_object&                      get_ad_creative( const account_name_type& account, const string& creative_id )const;
         const ad_creative_object*                      find_ad_creative( const account_name_type& account, const string& creative_id )const;

         const ad_campaign_object&                      get_ad_campaign( const account_name_type& account, const shared_string& campaign_id )const;
         const ad_campaign_object*                      find_ad_campaign( const account_name_type& account, const shared_string& campaign_id )const;

         const ad_campaign_object&                      get_ad_campaign( const account_name_type& account, const string& campaign_id )const;
         const ad_campaign_object*                      find_ad_campaign( const account_name_type& account, const string& campaign_id )const;

         const ad_inventory_object&                     get_ad_inventory( const account_name_type& account, const shared_string& inventory_id )const;
         const ad_inventory_object*                     find_ad_inventory( const account_name_type& account, const shared_string& inventory_id )const;

         const ad_inventory_object&                     get_ad_inventory( const account_name_type& account, const string& inventory_id )const;
         const ad_inventory_object*                     find_ad_inventory( const account_name_type& account, const string& inventory_id )const;

         const ad_audience_object&                      get_ad_audience( const account_name_type& account, const shared_string& audience_id )const;
         const ad_audience_object*                      find_ad_audience( const account_name_type& account, const shared_string& audience_id )const;

         const ad_audience_object&                      get_ad_audience( const account_name_type& account, const string& audience_id )const;
         const ad_audience_object*                      find_ad_audience( const account_name_type& account, const string& audience_id )const;

         const ad_bid_object&                           get_ad_bid( const account_name_type& account, const shared_string& bid_id )const;
         const ad_bid_object*                           find_ad_bid( const account_name_type& account, const shared_string& bid_id )const;

         const ad_bid_object&                           get_ad_bid( const account_name_type& account, const string& bid_id )const;
         const ad_bid_object*                           find_ad_bid( const account_name_type& account, const string& bid_id )const;

         asset pay_advertising_delivery( const account_object& provider, const account_object& demand, const account_object& audience, const asset& value );

         void deliver_ad_bid( const ad_bid_object& bid, const account_object& viewer );

         void cancel_ad_bid( const ad_bid_object& bid );



         //========================//
         // === GRAPH DATABASE === //
         //========================//



         const graph_node_object&                       get_graph_node( const account_name_type& account, const shared_string& node_id )const;
         const graph_node_object*                       find_graph_node( const account_name_type& account, const shared_string& node_id )const;

         const graph_node_object&                       get_graph_node( const account_name_type& account, const string& node_id )const;
         const graph_node_object*                       find_graph_node( const account_name_type& account, const string& node_id )const;

         const graph_edge_object&                       get_graph_edge( const account_name_type& account, const shared_string& edge_id )const;
         const graph_edge_object*                       find_graph_edge( const account_name_type& account, const shared_string& edge_id )const;

         const graph_edge_object&                       get_graph_edge( const account_name_type& account, const string& edge_id )const;
         const graph_edge_object*                       find_graph_edge( const account_name_type& account, const string& edge_id )const;

         const graph_node_property_object&              get_graph_node_property( const graph_node_name_type& node_type )const;
         const graph_node_property_object*              find_graph_node_property( const graph_node_name_type& node_type )const;

         const graph_edge_property_object&              get_graph_edge_property( const graph_edge_name_type& edge_type )const;
         const graph_edge_property_object*              find_graph_edge_property( const graph_edge_name_type& edge_type )const;



         //===========================//
         // === TRANSFER DATABASE === //
         //===========================//



         const transfer_request_object&                 get_transfer_request( const account_name_type& name, const shared_string& request_id )const;
         const transfer_request_object*                 find_transfer_request( const account_name_type& name, const shared_string& request_id )const;

         const transfer_request_object&                 get_transfer_request( const account_name_type& name, const string& request_id )const;
         const transfer_request_object*                 find_transfer_request( const account_name_type& name, const string& request_id )const;

         const transfer_recurring_object&               get_transfer_recurring( const account_name_type& name, const shared_string& transfer_id )const;
         const transfer_recurring_object*               find_transfer_recurring( const account_name_type& name, const shared_string& transfer_id )const;

         const transfer_recurring_object&               get_transfer_recurring( const account_name_type& name, const string& transfer_id )const;
         const transfer_recurring_object*               find_transfer_recurring( const account_name_type& name, const string& transfer_id )const;

         const transfer_recurring_request_object&       get_transfer_recurring_request( const account_name_type& name, const shared_string& request_id )const;
         const transfer_recurring_request_object*       find_transfer_recurring_request( const account_name_type& name, const shared_string& request_id )const;

         const transfer_recurring_request_object&       get_transfer_recurring_request( const account_name_type& name, const string& request_id )const;
         const transfer_recurring_request_object*       find_transfer_recurring_request( const account_name_type& name, const string& request_id )const;

         void process_recurring_transfers();



         //==========================//
         // === BALANCE DATABASE === //
         //==========================//



         const account_balance_object&                  get_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const;
         const account_balance_object*                  find_account_balance( const account_name_type& owner, const asset_symbol_type& symbol )const;

         const savings_withdraw_object&                 get_savings_withdraw( const account_name_type& owner, const shared_string& request_id )const;
         const savings_withdraw_object*                 find_savings_withdraw( const account_name_type& owner, const shared_string& request_id )const;

         const savings_withdraw_object&                 get_savings_withdraw( const account_name_type& owner, const string& request_id )const;
         const savings_withdraw_object*                 find_savings_withdraw( const account_name_type& owner, const string& request_id )const;

         const confidential_balance_object&             get_confidential_balance( const digest_type& hash )const;
         const confidential_balance_object*             find_confidential_balance( const digest_type& hash )const;

         const asset_delegation_object&                 get_asset_delegation( const account_name_type& delegator, const account_name_type& delegatee, const asset_symbol_type& symbol )const;
         const asset_delegation_object*                 find_asset_delegation( const account_name_type& delegator, const account_name_type& delegatee, const asset_symbol_type& symbol )const;

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
         void adjust_confidential_supply( const asset& delta );

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

         share_type get_voting_power( const account_object& a, const asset_symbol_type& currency )const;
         share_type get_voting_power( const account_name_type& a, const asset_symbol_type& currency )const;
         share_type get_voting_power( const account_object& a, const price& equity_price )const;
         share_type get_voting_power( const account_name_type& a, const price& equity_price )const;

         share_type get_proxied_voting_power( const account_object& a, const price& equity_price )const;
         share_type get_proxied_voting_power( const account_name_type& a, const price& equity_price )const;

         share_type get_equity_voting_power( const account_object& a, const business_object& b )const;
         share_type get_equity_voting_power( const account_name_type& a, const business_object& b )const;
         share_type get_equity_voting_power( const account_object& a, const governance_object& b )const;
         share_type get_equity_voting_power( const account_name_type& a, const governance_object& b )const;

         void process_savings_withdraws();

         void clear_expired_delegations();

         void clear_asset_balances( const asset_symbol_type& symbol );

         string to_pretty_string( const asset& a )const;



         //==============================//
         // === MARKETPLACE DATABASE === //
         //==============================//



         const product_sale_object&               get_product_sale( const account_name_type& name, const shared_string& product_id )const;
         const product_sale_object*               find_product_sale( const account_name_type& name, const shared_string& product_id )const;

         const product_sale_object&               get_product_sale( const account_name_type& name, const string& product_id )const;
         const product_sale_object*               find_product_sale( const account_name_type& name, const string& product_id )const;

         const product_purchase_object&           get_product_purchase( const account_name_type& name, const shared_string& order_id )const;
         const product_purchase_object*           find_product_purchase( const account_name_type& name, const shared_string& order_id )const;

         const product_purchase_object&           get_product_purchase( const account_name_type& name, const string& order_id )const;
         const product_purchase_object*           find_product_purchase( const account_name_type& name, const string& order_id )const;

         const product_auction_sale_object&       get_product_auction_sale( const account_name_type& name, const shared_string& auction_id )const;
         const product_auction_sale_object*       find_product_auction_sale( const account_name_type& name, const shared_string& auction_id )const;

         const product_auction_sale_object&       get_product_auction_sale( const account_name_type& name, const string& auction_id )const;
         const product_auction_sale_object*       find_product_auction_sale( const account_name_type& name, const string& auction_id )const;

         const product_auction_bid_object&        get_product_auction_bid( const account_name_type& name, const shared_string& bid_id )const;
         const product_auction_bid_object*        find_product_auction_bid( const account_name_type& name, const shared_string& bid_id )const;

         const product_auction_bid_object&        get_product_auction_bid( const account_name_type& name, const string& bid_id )const;
         const product_auction_bid_object*        find_product_auction_bid( const account_name_type& name, const string& bid_id )const;

         const escrow_object&                     get_escrow( const account_name_type& name, const shared_string& escrow_id )const;
         const escrow_object*                     find_escrow( const account_name_type& name, const shared_string& escrow_id )const;

         const escrow_object&                     get_escrow( const account_name_type& name, const string& escrow_id )const;
         const escrow_object*                     find_escrow( const account_name_type& name, const string& escrow_id )const;

         void process_product_auctions();

         void process_escrow_transfers();

         void dispute_escrow( const escrow_object& escrow );

         void release_escrow( const escrow_object& escrow );



         //==========================//
         // === TRADING DATABASE === //
         //==========================//



         const limit_order_object&                get_limit_order(  const account_name_type& owner, const shared_string& order_id )const;
         const limit_order_object*                find_limit_order( const account_name_type& owner, const shared_string& order_id )const;

         const limit_order_object&                get_limit_order(  const account_name_type& owner, const string& order_id )const;
         const limit_order_object*                find_limit_order( const account_name_type& owner, const string& order_id )const;

         const margin_order_object&               get_margin_order( const account_name_type& name, const shared_string& margin_id )const;
         const margin_order_object*               find_margin_order( const account_name_type& name, const shared_string& margin_id )const;

         const margin_order_object&               get_margin_order( const account_name_type& name, const string& margin_id )const;
         const margin_order_object*               find_margin_order( const account_name_type& name, const string& margin_id )const;

         const option_order_object&               get_option_order( const account_name_type& name, const shared_string& option_id )const;
         const option_order_object*               find_option_order( const account_name_type& name, const shared_string& option_id )const;

         const option_order_object&               get_option_order( const account_name_type& name, const string& option_id )const;
         const option_order_object*               find_option_order( const account_name_type& name, const string& option_id )const;

         const auction_order_object&              get_auction_order( const account_name_type& name, const shared_string& auction_id )const;
         const auction_order_object*              find_auction_order( const account_name_type& name, const shared_string& auction_id )const;

         const auction_order_object&              get_auction_order( const account_name_type& name, const string& auction_id )const;
         const auction_order_object*              find_auction_order( const account_name_type& name, const string& auction_id )const;

         const call_order_object&                 get_call_order( const account_name_type& name, const asset_symbol_type& symbol )const;
         const call_order_object*                 find_call_order( const account_name_type& name, const asset_symbol_type& symbol )const;

         const asset_collateral_bid_object&       get_asset_collateral_bid( const account_name_type& name, const asset_symbol_type& symbol )const;
         const asset_collateral_bid_object*       find_asset_collateral_bid( const account_name_type& name, const asset_symbol_type& symbol )const;

         const asset_settlement_object&           get_asset_settlement( const account_name_type& name, const asset_symbol_type& symbol )const;
         const asset_settlement_object*           find_asset_settlement( const account_name_type& name, const asset_symbol_type& symbol )const;

         bool apply_order( const limit_order_object& new_order_object );
         bool apply_order( const margin_order_object& new_order_object );

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
         asset match( const call_order_object& call, const asset_settlement_object& settle, const price& match_price, 
            asset max_settlement, const price& fill_price );

         bool fill_limit_order( const limit_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface );

         bool fill_margin_order( const margin_order_object& order, const asset& pays, const asset& receives, bool cull_if_small,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface );

         bool fill_option_order( const option_order_object& order, const asset& pays, const asset& receives, const asset& opt,
            const price& fill_price );

         bool fill_auction_order( const auction_order_object& order, const asset& pays, const asset& receives,
            const price& fill_price );

         bool fill_call_order( const call_order_object& order, const asset& pays, const asset& receives,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface, bool global_settle );
         
         bool fill_settle_order( const asset_settlement_object& settle, const asset& pays, const asset& receives,
            const price& fill_price, const bool is_maker, const account_name_type& match_interface );

         void process_auction_orders();

         bool exercise_option( const asset& option, const account_object& account );

         void cancel_bid( const asset_collateral_bid_object& bid );

         void execute_bid( const asset_collateral_bid_object& bid, share_type debt, 
            share_type collateral_from_fund, const price_feed& current_feed );

         void cancel_settle_order( const asset_settlement_object& order );

         void cancel_limit_order( const limit_order_object& order );
         
         void close_margin_order( const margin_order_object& order );

         void close_auction_order( const auction_order_object& order );

         void close_option_order( const option_order_object& order );

         bool maybe_cull_small_order( const limit_order_object& order );

         bool maybe_cull_small_order( const margin_order_object& order );

         bool check_call_orders( const asset_object& mia, bool enable_black_swan, bool for_new_limit_order );

         bool check_for_blackswan( const asset_object& mia, bool enable_black_swan, const asset_stablecoin_data_object* stablecoin_ptr );

         asset pay_trading_fees( const account_object& taker, const asset& receives, const account_name_type& maker_int, const account_name_type& taker_int );



         //=======================//
         // === POOL DATABASE === //
         //=======================//



         const asset_liquidity_pool_object&                  get_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const;
         const asset_liquidity_pool_object*                  find_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const;

         const asset_liquidity_pool_object&                  get_liquidity_pool( const asset_symbol_type& symbol )const;
         const asset_liquidity_pool_object*                  find_liquidity_pool( const asset_symbol_type& symbol )const;

         const asset_credit_pool_object&                     get_credit_pool( const asset_symbol_type& symbol, bool credit_asset )const;
         const asset_credit_pool_object*                     find_credit_pool( const asset_symbol_type& symbol, bool credit_asset )const;

         const credit_collateral_object&                     get_collateral( const account_name_type& owner, const asset_symbol_type& symbol )const;
         const credit_collateral_object*                     find_collateral( const account_name_type& owner, const asset_symbol_type& symbol )const;

         const credit_loan_object&                           get_loan( const account_name_type& owner, const shared_string& loan_id )const;
         const credit_loan_object*                           find_loan( const account_name_type& owner, const shared_string& loan_id )const;

         const credit_loan_object&                           get_loan( const account_name_type& owner, const string& loan_id )const;
         const credit_loan_object*                           find_loan( const account_name_type& owner, const string& loan_id )const;

         const asset_option_pool_object&                     get_option_pool( const asset_symbol_type& base_symbol, const asset_symbol_type& quote_symbol )const;
         const asset_option_pool_object*                     find_option_pool( const asset_symbol_type& base_symbol, const asset_symbol_type& quote_symbol )const;

         const asset_option_pool_object&                     get_option_pool( const asset_symbol_type& symbol )const;
         const asset_option_pool_object*                     find_option_pool( const asset_symbol_type& symbol )const;

         const asset_prediction_pool_object&                 get_prediction_pool( const asset_symbol_type& symbol )const;
         const asset_prediction_pool_object*                 find_prediction_pool( const asset_symbol_type& symbol )const;

         const asset_prediction_pool_resolution_object&      get_prediction_pool_resolution( const account_name_type& name, const asset_symbol_type& symbol )const;
         const asset_prediction_pool_resolution_object*      find_prediction_pool_resolution( const account_name_type& name, const asset_symbol_type& symbol )const;
         
         void liquid_fund( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool );

         void liquid_withdraw( const asset& input, const asset_symbol_type& receive, 
            const account_object& account, const asset_liquidity_pool_object& pool );

         asset liquid_exchange( const asset& input, const asset_symbol_type& receive, bool execute, bool apply_fees );
         void liquid_exchange( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool, 
            const account_object& int_account );
         void liquid_exchange( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool );
         
         asset liquid_acquire( const asset& receive, const asset_symbol_type& input, bool execute, bool apply_fees );
         void liquid_acquire( const asset& receive, const account_object& account, const asset_liquidity_pool_object& pool, 
            const account_object& int_account );
         void liquid_acquire( const asset& receive, const account_object& account, const asset_liquidity_pool_object& pool );

         pair< asset, asset > liquid_limit_exchange( const asset& input, const price& limit_price, 
            const asset_liquidity_pool_object& pool, bool execute, bool apply_fees );
         void liquid_limit_exchange( const asset& input, const price& limit_price, const account_object& account, 
            const asset_liquidity_pool_object& pool, const account_object& int_account );
         void liquid_limit_exchange( const asset& input, const price& limit_price, const account_object& account, 
            const asset_liquidity_pool_object& pool );

         void update_median_liquidity();

         void credit_lend( const asset& input, const account_object& account, const asset_credit_pool_object& pool);

         void credit_withdraw( const asset& input, const account_object& account, const asset_credit_pool_object& pool);

         bool credit_check( const asset& debt, const asset& collateral, const asset_credit_pool_object& credit_pool);

         bool margin_check( const asset& debt, const asset& position, const asset& collateral, const asset_credit_pool_object& credit_pool);

         void process_credit_updates();

         void process_margin_updates();

         void liquidate_credit_loan( const credit_loan_object& loan );

         void process_option_pools();

         void process_prediction_pools();

         void close_prediction_pool( const asset_prediction_pool_object& pool );



         //========================//
         // === ASSET DATABASE === //
         //========================//



         const asset_object&                                 get_asset( const asset_symbol_type& symbol )const;
         const asset_object*                                 find_asset( const asset_symbol_type& symbol )const;

         const asset_dynamic_data_object&                    get_dynamic_data( const asset_symbol_type& symbol )const;
         const asset_dynamic_data_object*                    find_dynamic_data( const asset_symbol_type& symbol )const;

         const asset_currency_data_object&                   get_currency_data( const asset_symbol_type& symbol )const;
         const asset_currency_data_object*                   find_currency_data( const asset_symbol_type& symbol )const;

         const asset_reward_fund_object&                     get_reward_fund( const asset_symbol_type& symbol )const;
         const asset_reward_fund_object*                     find_reward_fund( const asset_symbol_type& symbol )const;

         const asset_stablecoin_data_object&                 get_stablecoin_data( const asset_symbol_type& symbol )const;
         const asset_stablecoin_data_object*                 find_stablecoin_data( const asset_symbol_type& symbol )const;

         const asset_equity_data_object&                     get_equity_data( const asset_symbol_type& symbol )const;
         const asset_equity_data_object*                     find_equity_data( const asset_symbol_type& symbol )const;

         const asset_bond_data_object&                       get_bond_data( const asset_symbol_type& symbol )const;
         const asset_bond_data_object*                       find_bond_data( const asset_symbol_type& symbol )const;

         const asset_credit_data_object&                     get_credit_data( const asset_symbol_type& symbol )const;
         const asset_credit_data_object*                     find_credit_data( const asset_symbol_type& symbol )const;

         const asset_stimulus_data_object&                   get_stimulus_data( const asset_symbol_type& symbol )const;
         const asset_stimulus_data_object*                   find_stimulus_data( const asset_symbol_type& symbol )const;

         const asset_unique_data_object&                     get_unique_data( const asset_symbol_type& symbol )const;
         const asset_unique_data_object*                     find_unique_data( const asset_symbol_type& symbol )const;

         const asset_distribution_object&                    get_asset_distribution( const asset_symbol_type& symbol )const;
         const asset_distribution_object*                    find_asset_distribution( const asset_symbol_type& symbol )const;

         const asset_distribution_balance_object&            get_asset_distribution_balance( const account_name_type& name, const asset_symbol_type& symbol )const;
         const asset_distribution_balance_object*            find_asset_distribution_balance( const account_name_type& name, const asset_symbol_type& symbol )const;

         void process_funds();

         const price& get_usd_price()const;

         asset asset_to_USD( const price& p, const asset& a )const;

         asset asset_to_USD( const asset& a )const;

         asset USD_to_asset( const price& p, const asset& a )const;

         asset USD_to_asset( const asset& a )const;

         void process_stablecoins();

         void process_power_rewards();

         share_type get_equity_shares( const account_balance_object& balance, const asset_equity_data_object& equity );

         void process_equity_rewards();
         
         void process_asset_staking();

         void process_bond_interest();

         void process_bond_assets();
         
         void process_credit_buybacks();
         
         void process_credit_interest();

         void process_stimulus_assets();

         void process_unique_assets();

         void process_asset_distribution();

         void update_expired_feeds();

         void process_bids( const asset_stablecoin_data_object& bad );

         void globally_settle_asset( const asset_object& mia, const price& settlement_price );
         
         void revive_stablecoin( const asset_object& stablecoin );

         void cancel_bids_and_revive_mpa( const asset_object& stablecoin, const asset_stablecoin_data_object& bad );

         asset calculate_issuer_fee( const asset_object& trade_asset, const asset& trade_amount );

         asset pay_issuer_fees( const asset_object& recv_asset, const asset& receives );

         asset pay_issuer_fees( const account_object& seller, const asset_object& recv_asset, const asset& receives );



         //===========================//
         // === PRODUCER DATABASE === //
         //===========================//



         const producer_object&                get_producer( const account_name_type& name )const;
         const producer_object*                find_producer( const account_name_type& name )const;

         const producer_vote_object&           get_producer_vote( const account_name_type& account, const account_name_type& producer )const;
         const producer_vote_object*           find_producer_vote( const account_name_type& account, const account_name_type& producer )const;

         const block_validation_object&        get_block_validation( const account_name_type& producer, uint64_t height )const;
         const block_validation_object*        find_block_validation( const account_name_type& producer, uint64_t height )const;

         const producer_schedule_object&       get_producer_schedule()const;

         x11                                   pow_difficulty()const;

         const median_chain_property_object&   get_median_chain_properties()const;

         uint32_t                              producer_participation_rate()const;

         /**
          * Get the producer scheduled for block production in a slot.
          *
          * slot_num always corresponds to a time in the future.
          *
          * If slot_num == 1, returns the next scheduled producer.
          * If slot_num == 2, returns the next scheduled producer after
          * 1 block gap.
          *
          * Use the get_slot_time() and get_slot_at_time() functions
          * to convert between slot_num and timestamp.
          *
          * Passing slot_num == 0 returns NULL_PRODUCER
          */
         account_name_type                      get_scheduled_producer(uint64_t slot_num)const;

         /**
          * Get the time at which the given slot occurs.
          *
          * If slot_num == 0, return time_point().
          *
          * If slot_num == N for N > 0, return the Nth next
          * block-interval-aligned time greater than head_block_time().
          */
         fc::time_point                         get_slot_time(uint64_t slot_num)const;

         /**
          * Get the last slot which occurs AT or BEFORE the given time.
          *
          * The return value is the greatest value N such that
          * get_slot_time( N ) <= when.
          *
          * If no such N exists, return 0.
          */
         uint64_t                               get_slot_at_time(fc::time_point when)const;

         void update_producer_set();

         void update_producer_votes( const account_object& account );
         void update_producer_votes( const account_object& account, const account_name_type& producer, uint16_t vote_rank );

         void process_update_producer_set();

         share_type update_producer( const producer_object& producer, const producer_schedule_object& pso, const dynamic_global_property_object& props, const median_chain_property_object& median_props );
         
         void update_proof_of_work_target();
         
         void claim_proof_of_work_reward( const account_name_type& miner );

         void process_txn_stake_rewards();

         void process_validation_rewards();

         void process_producer_activity_rewards();

      private:
         optional< chainbase::database::session > _pending_tx_session;

         void apply_block( const signed_block& next_block, uint32_t skip = skip_nothing );

         void apply_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );

         void _apply_block( const signed_block& next_block );

         void _apply_transaction( const signed_transaction& trx );

         void apply_operation( const operation& op );

         void update_stake( const signed_transaction& trx );

         void check_flash_loans();

         const producer_object& validate_block_header( uint32_t skip, const signed_block& next_block )const;

         void create_block_summary( const signed_block& next_block);

         void update_global_dynamic_data( const signed_block& b );

         void update_signing_producer( const producer_object& signing_producer, const signed_block& new_block);

         void update_transaction_stake( const producer_object& signing_producer, const uint128_t& transaction_stake);

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

         std::unique_ptr< database_impl >         _my;

         vector< signed_transaction >             _pending_tx;

         fork_database                            _fork_db;

         fc::time_point                           _hardfork_times[ NUM_HARDFORKS + 1 ];

         protocol::hardfork_version               _hardfork_versions[ NUM_HARDFORKS + 1 ];

         block_log                                _block_log;

         // this function needs access to _plugin_index_signal
         template< typename MultiIndexType >
         friend void add_plugin_index( database& db );

         fc::signal< void() >                     _plugin_index_signal;

         transaction_id_type                      _current_trx_id;

         uint64_t                                 _current_block_num = 0;

         uint16_t                                 _current_trx_in_block = 0;

         uint16_t                                 _current_op_in_trx = 0;

         uint16_t                                 _current_virtual_op = 0;

         uint128_t                                _current_trx_stake_weight = 0;

         flat_map<uint64_t,block_id_type>         _checkpoints;

         node_property_object                     _node_property_object;

         uint32_t                                 _flush_blocks = 0;

         uint64_t                                 _next_flush_block = 0;

         uint32_t                                 _last_free_gb_printed = 0;

         flat_map< std::string, std::shared_ptr< custom_operation_interpreter > >   _custom_operation_interpreters;

         std::string                              _json_schema;
   };

} }