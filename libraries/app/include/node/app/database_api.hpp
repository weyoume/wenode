#pragma once
#include <node/app/applied_operation.hpp>
#include <node/app/state.hpp>

#include <node/chain/database.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/history_object.hpp>

#include <node/tags/tags_plugin.hpp>

#include <node/follow/follow_plugin.hpp>
#include <node/witness/witness_plugin.hpp>

#include <fc/api.hpp>
#include <fc/optional.hpp>
#include <fc/variant_object.hpp>

#include <fc/network/ip.hpp>

#include <boost/container/flat_set.hpp>

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace node { namespace app {

using namespace node::chain;
using namespace node::protocol;
using namespace std;

struct api_context;

struct scheduled_hardfork
{
   hardfork_version     hf_version;
   fc::time_point       live_time;
};

struct withdraw_route
{
   string               from_account;
   string               to_account;
   uint16_t             percent;
   bool                 auto_stake;
};

enum withdraw_route_type
{
   incoming,
   outgoing,
   all
};

enum sort_time
{
   NO_TIME          = 'none',
   ACTIVE_TIME      = 'active',
   RAPID_TIME       = 'rapid',
   STANDARD_TIME    = 'standard',
   TOP_TIME         = 'top',
   ELITE_TIME       = 'elite'
}

enum sort_type
{
   NO_SORT             = 'none',
   VOTES_SORT          = 'votes',
   VIEWS_SORT          = 'views',
   SHARES_SORT         = 'shares',
   COMMENTS_SORT       = 'comments',
   QUALITY_SORT        = 'quality',
   POPULAR_SORT        = 'popular',
   VIRAL_SORT          = 'viral',
   DISCUSSION_SORT     = 'discussion',
   PROMINENT_SORT      = 'prominent',
   CONVERSATION_SORT   = 'conversation',
   DISCOURSE_SORT      = 'discourse'
}

enum search_query_type
{
   NO_SEARCH_TYPE     = 'none',
   ACCOUNT_SEARCH     = 'account',
   BOARD_SEARCH       = 'board',
   TAG_SEARCH         = 'tag',
   ASSET_SEARCH       = 'asset',
   POST_SEARCH        = 'post'
}

class database_api_impl;

/**
 * Defines the arguments to a discussion query.
 */
struct discussion_query 
{
   void validate()const
   {
      FC_ASSERT( filter_authors.find( account ) == filter_authors.end() );
      FC_ASSERT( filter_boards.find( board ) == filter_boards.end() );
      FC_ASSERT( filter_tags.find( tag ) == filter_tags.end() );
      FC_ASSERT( limit <= 100 );
   }

   string                  account;               // Name of the account being fetched for feed or blog queries.
   string                  board;                 // Name of the board being queried.
   string                  tag;                   // Name of the tag being querired.

   string                  sort_type;             // Sorting index type.
   string                  sort_time;             // Time preference of the sorting type.
   
   string                  feed_type;             // Type of feed being queried.
   string                  blog_type;             // Type of blog being queried.

   bool                    include_private;       // True to include private encrypted posts.
   string                  max_rating;            // Highest content rating to include in posts queried.

   uint32_t                limit = 0;

   set<string>             select_boards;         // list of boards to include
   set<string>             filter_boards;         // list of boards to filter, posts made in these boards are filtered
   
   set<string>             select_tags;           // list of tags to include
   set<string>             filter_tags;           // list of tags to filter, posts with these tags are filtered

   set<string>             select_authors;        // list of authors to include
   set<string>             filter_authors;        // list of authors to filter, posts by these authors are filtered

   set<string>             select_languages;      // list of languages to include
   set<string>             filter_languages;      // list of languages to filter, posts made in these boards are filtered

   optional<string>        start_author;
   optional<string>        start_permlink;
   optional<string>        parent_author;
   optional<string>        parent_permlink;

   uint32_t                truncate_body = 0;     // the number of bytes of the post body to return, 0 for all
};


/**
 * Defines the arguments to a network object search query.
 */
struct search_query 
{
   void validate()const
   {
      FC_ASSERT( limit <= 100 );
   }

   string                            account;    // Name of the account creating the search.

   string                            query;      // Search String being queried.

   uint32_t                          limit;      // The amount of results to include in the results.          
};

/**
 * Defines the arguments to an ad query, and provides the relevant display context
 * query from a discussion feed, or a search.
 */
struct ad_query 
{
   void validate()const
   {
      FC_ASSERT( limit <= 100 );
   }

   string                  interface;             // Name of the interface account of the ad inventory provider.

   string                  viewer;                // Name of the audience member account receiving the ad.

   string                  format_type;           // Type of ad inventory format being queried.

   discussion_query        discussion_query;         // The Discussion feed display query of the ad context.

   search_query            search_query;          // the Search display query of the ad context.
   
   uint32_t                limit = 0;
};

/**
 * @brief The database_api class implements the RPC API for the chain database.
 *
 * This API exposes accessors on the database which query state tracked by a blockchain validating node. This API is
 * read-only; all modifications to the database must be performed via transactions. Transactions are broadcast via
 * the @ref network_broadcast_api.
 */
class database_api
{
   public:
      database_api(const node::app::api_context& ctx);
      ~database_api();

      ///////////////////
      // Subscriptions //
      ///////////////////

      void set_block_applied_callback( std::function<void(const variant& block_header)> cb );

      vector<tag_api_obj> get_trending_tags( string after_tag, uint32_t limit )const;

      /**
       *  This API is a short-cut for returning all of the state required for a particular URL
       *  with a single query.
       */
      state get_state( string path )const;

      vector< account_name_type > get_active_producers()const;

      /////////////////////////////
      // Blocks and transactions //
      /////////////////////////////

      /**
       * @brief Retrieve a block header
       * @param block_num Height of the block whose header should be returned
       * @return header of the referenced block, or null if no matching block was found
       */
      optional<block_header> get_block_header(uint32_t block_num)const;

      /**
       * @brief Retrieve a full, signed block
       * @param block_num Height of the block to be returned
       * @return the referenced block, or null if no matching block was found
       */
      optional<signed_block_api_obj> get_block(uint32_t block_num)const;

      /**
       *  @brief Get sequence of operations included/generated within a particular block
       *  @param block_num Height of the block whose generated virtual operations should be returned
       *  @param only_virtual Whether to only include virtual operations in returned results (default: true)
       *  @return sequence of operations included/generated within the block
       */
      vector<applied_operation> get_ops_in_block(uint32_t block_num, bool only_virtual = true)const;

      /////////////
      // Globals //
      /////////////

      /**
       * @brief Retrieve compile-time constants
       */
      fc::variant_object get_config()const;

      /**
       * @brief Return a JSON description of object representations
       */
      std::string get_schema()const;

      /**
       * @brief Retrieve the current @ref dynamic_global_property_object
       */
      dynamic_global_property_api_obj  get_dynamic_global_properties()const;
      chain_properties                 get_chain_properties()const;
      price                            get_current_median_history_price()const;
      feed_history_api_obj             get_feed_history()const;
      witness_schedule_api_obj         get_witness_schedule()const;
      hardfork_version                 get_hardfork_version()const;
      scheduled_hardfork               get_next_scheduled_hardfork()const;
      reward_fund_api_obj              get_reward_fund( string name )const;

      //////////
      // Keys //
      //////////

      vector<set<string>> get_key_references( vector<public_key_type> key )const;

      //////////////
      // Accounts //
      //////////////

      vector< account_api_obj > get_accounts( vector< string > names ) const;

      vector< account_concise_api_obj > get_concise_accounts( vector< string > names ) const;

      vector< extended_account > get_full_accounts( vector< string > names ) const;

      vector< message_state > get_messages( vector< string > names ) const;

      vector< balance_state > get_balances( vector< string > names ) const;

      vector< key_state > get_keychains( vector< string > names) const;

      /**
       *  @return all accounts that referr to the key or account id in their owner or active authorities.
       */
      vector<account_id_type> get_account_references( account_id_type account_id )const;

      /**
       * @brief Get a list of accounts by name
       * @param account_names Names of the accounts to retrieve
       * @return The accounts holding the provided names
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<account_api_obj>> lookup_account_names(const vector<string>& account_names)const;

      /**
       * @brief Get names and IDs for registered accounts
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of account names to corresponding IDs
       */
      set<string> lookup_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get the total number of accounts registered with the blockchain
       */
      uint64_t get_account_count()const;

      vector< owner_authority_history_api_obj > get_owner_history( string account )const;

      optional< account_recovery_request_api_obj > get_recovery_request( string account ) const;

      ////////////
      // Assets //
      ////////////

      vector<extended_asset> get_assets( vector<string> assets )const;

      optional< escrow_api_obj > get_escrow( string from, uint32_t escrow_id )const;

      vector< withdraw_route > get_withdraw_routes( string account, withdraw_route_type type = outgoing )const;

      optional< account_bandwidth_api_obj > get_account_bandwidth( string account, witness::bandwidth_type type )const;

      vector< savings_withdraw_api_obj > get_savings_withdraw_from( string account )const;
      vector< savings_withdraw_api_obj > get_savings_withdraw_to( string account )const;

      vector< asset_delegation_api_obj > get_asset_delegations( string account, string from, uint32_t limit = 100 )const;
      vector< asset_delegation_expiration_api_obj > get_expiring_asset_delegations( string account, time_point from, uint32_t limit = 100 )const;


      ////////////
      // Boards //
      ////////////

      vector<extended_board> get_boards( vector< string > boards ) const;

      ///////////////
      // Witnesses //
      ///////////////

      /**
       * @brief Get a list of witnesses by ID
       * @param witness_ids IDs of the witnesses to retrieve
       * @return The witnesses corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<witness_api_obj>> get_witnesses(const vector<witness_id_type>& witness_ids)const;

      /**
       * @brief Get the witness owned by a given account
       * @param account The name of the account whose witness should be retrieved
       * @return The witness object, or null if the account does not have a witness
       */
      fc::optional< witness_api_obj > get_witness_by_account( string account_name )const;

      /**
       *  This method is used to fetch elected witnesses with pagination.
       *  @return an array of `count` witnesses sorted by current voting power after witness `from` with at most `limit' results.
       */
      vector< witness_api_obj > get_witnesses_by_voting_power( string from, uint32_t limit )const;

      /**
       *  This method is used to fetch mining witnesses with pagination.
       *  @return an array of `count` witnesses sorted by current mining power after witness `from` with at most `limit' results.
       */
      vector< witness_api_obj > get_witnesses_by_mining_power( string from, uint32_t limit )const;

      /**
       * Get Network officers according to thier type, with pagination.
       */
      vector< network_officers_api_obj >   get_development_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officers_api_obj >   get_marketing_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officers_api_obj >   get_advocacy_officers_by_voting_power( string from, uint32_t limit )const;

      vector< executive_board_api_obj >    get_executive_boards_by_voting_power( string from, uint32_t limit )const;

      vector< supernode_api_obj >          get_supernodes_by_view_weight( string from, uint32_t limit )const;

      vector< interface_api_obj >          get_interfaces_by_users( string from, uint32_t limit )const;

      vector< governance_account_api_obj > get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const;

      vector< community_enterprise_api_obj >    get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const;

      /**
       * @brief Get names and IDs for registered witnesses
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of witness names to corresponding IDs
       */
      set<account_name_type> lookup_witness_accounts( const string& lower_bound_name, uint32_t limit )const;

      /**
       * @brief Get the total number of witnesses registered with the blockchain
       */
      uint64_t get_witness_count()const;

      ////////////
      // Market //
      ////////////


      /**
       * Retrieves all of the open limit, margin, loan, and call orders from a list of accounts.
       */
      vector< order_state >                get_open_orders( vector< string > names )const;

      market_limit_orders                  get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_margin_orders                 get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_call_orders                   get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_credit_loans                  get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      vector< credit_pool_api_obj >        get_credit_pools( vector<string> assets ) const;

      vector< liquidity_pool_api_obj >     get_liquidity_pools( string buy_symbol, string sell_symbol ) const;

      /////////
      // Ads //
      /////////


      /**
       * Retrieves ad creatives, campaigns, bids, inventory, and audiences
       * for a specified list of accounts.
       */
      vector< account_ad_state >           get_account_ads( vector<string> names )const;

      /**
       * Retrieves all bids for a specified interface that include a specified audience member
       */
      vector< ad_bid_state >               get_interface_audience_bids( const ad_query& query )const;


      ////////////
      // Search //
      ////////////

      search_result_state                  get_search_query( const search_query& query )const;  

      ////////////////////////////
      // Authority / validation //
      ////////////////////////////

      /// @brief Get a hexdump of the serialized binary form of a transaction
      std::string                          get_transaction_hex(const signed_transaction& trx)const;

      annotated_signed_transaction         get_transaction( transaction_id_type trx_id )const;

      /**
       *  This API will take a partially signed transaction and a set of public keys that the owner has the ability to sign for
       *  and return the minimal subset of public keys that should add signatures to the transaction.
       */
      set<public_key_type>                 get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;

      /**
       *  This method will return the set of all public keys that could possibly sign for a given transaction.  This call can
       *  be used by wallets to filter their set of public keys to just the relevant subset prior to calling @ref get_required_signatures
       *  to get the minimum subset.
       */
      set<public_key_type> get_potential_signatures( const signed_transaction& trx )const;

      /**
       * @return true of the @ref trx has all of the required signatures, otherwise throws an exception
       */
      bool           verify_authority( const signed_transaction& trx )const;

      /*
       * @return true if the signers have enough authority to authorize an account
       */
      bool           verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;

      /**
       *  if permlink is "" then it will return all votes for author
       */
      vector<vote_state> get_active_votes( string author, string permlink )const;

      vector<view_state> get_active_views( string author, string permlink )const;

      vector<share_state> get_active_shares( string author, string permlink )const;

      vector<moderation_state> get_active_mod_tags( string author, string permlink )const;

      vector<account_vote> get_account_votes( string voter )const;


      discussion           get_content( string author, string permlink )const;
      vector<discussion>   get_content_replies( string parent, string parent_permlink )const;

      ///@{ tags API
      /** This API will return the top 1000 tags used by an author sorted by most frequently used */
      vector<pair<string,uint32_t>> get_tags_used_by_author( const string& author )const;
      vector<discussion> get_discussions_by_payout(const discussion_query& query )const;
      vector<discussion> get_post_discussions_by_payout( const discussion_query& query )const;
      vector<discussion> get_comment_discussions_by_payout( const discussion_query& query )const;
      vector<discussion> get_discussions_by_index( const discussion_query& query )const;
      vector<discussion> get_discussions_by_created( const discussion_query& query )const;
      vector<discussion> get_discussions_by_active( const discussion_query& query )const;
      vector<discussion> get_discussions_by_cashout( const discussion_query& query )const;
      vector<discussion> get_discussions_by_votes( const discussion_query& query )const;
      vector<discussion> get_discussions_by_views( const discussion_query& query )const;
      vector<discussion> get_discussions_by_shares( const discussion_query& query )const;
      vector<discussion> get_discussions_by_children( const discussion_query& query )const;
      vector<discussion> get_discussions_by_feed( const discussion_query& query )const;
      vector<discussion> get_discussions_by_blog( const discussion_query& query )const;
      vector<discussion> get_discussions_by_recommended( const discussion_query& query )const;
      vector<discussion> get_discussions_by_comments( const discussion_query& query )const;

      ///@}

      /**
       *  For each of these filters:
       *     Get root content...
       *     Get any content...
       *     Get root content in category..
       *     Get any content in category...
       *
       *  Return discussions
       *     Total Discussion Pending Payout
       *     Last Discussion Update (or reply)... think
       *     Top Discussions by Total Payout
       *
       *  Return content (comments)
       *     Pending Payout Amount
       *     Pending Payout Time
       *     Creation Date
       *
       */
      ///@{



      /**
       *  Return the active discussions with the highest cumulative pending payouts without respect to category, total
       *  pending payout means the pending payout of all children as well.
       */
      vector<discussion>   get_replies_by_last_update( account_name_type start_author, string start_permlink, uint32_t limit )const;



      /**
       *  This method is used to fetch all posts/comments by start_author that occur after before_date and start_permlink with up to limit being returned.
       *
       *  If start_permlink is empty then only before_date will be considered. If both are specified the eariler to the two metrics will be used. This
       *  should allow easy pagination.
       */
      vector<discussion>   get_discussions_by_author_before_date( string author, string start_permlink, time_point before_date, uint32_t limit )const;

      /**
       *  Account operations have sequence numbers from 0 to N where N is the most recent operation. This method
       *  returns operations in the range [from-limit, from]
       *
       *  @param from - the absolute sequence number, -1 means most recent, limit is the number of operations before from.
       *  @param limit - the maximum number of items that can be queried (0 to 1000], must be less than from
       */
      map<uint32_t, applied_operation> get_account_history( string account, uint64_t from, uint32_t limit )const;

      ////////////////////////////
      // Handlers - not exposed //
      ////////////////////////////
      void on_api_startup();

   private:
      void set_pending_payout( discussion& d )const;
      void set_url( discussion& d )const;
      discussion get_discussion( comment_id_type, uint32_t truncate_body = 0 )const;

      static bool filter_default( const comment_api_obj& c ) { return false; }
      static bool exit_default( const comment_api_obj& c )   { return false; }
      static bool tag_exit_default( const tags::tag_object& c ) { return false; }

      template<typename Index, typename StartItr>
      vector<discussion> get_discussions( const discussion_query& q,
                                          const string& board,
                                          const string& tag,
                                          comment_id_type parent,
                                          const Index& idx, 
                                          StartItr itr,
                                          uint32_t truncate_body = 0,
                                          const std::function< bool( const comment_api_obj& ) >& filter = &database_api::filter_default,
                                          const std::function< bool( const comment_api_obj& ) >& exit   = &database_api::exit_default,
                                          const std::function< bool( const tags::tag_object& ) >& tag_exit = &database_api::tag_exit_default,
                                          bool ignore_parent = false
                                          )const;
      comment_id_type get_parent( const discussion_query& q )const;

      void recursively_fetch_content( state& _state, discussion& root, set<string>& referenced_accounts )const;

      std::shared_ptr< database_api_impl >   my;
};

} }



FC_REFLECT( node::app::scheduled_hardfork, (hf_version)(live_time) );
FC_REFLECT( node::app::withdraw_route, (from_account)(to_account)(percent)(auto_stake) );
FC_REFLECT( node::app::discussion_query, (tag)(filter_tags)(select_tags)(select_authors)(truncate_body)(start_author)(start_permlink)(parent_author)(parent_permlink)(limit) );

FC_REFLECT_ENUM( node::app::withdraw_route_type, (incoming)(outgoing)(all) );

FC_API(node::app::database_api,
   // Subscriptions
   (set_block_applied_callback)

   // tags
   (get_trending_tags)
   (get_tags_used_by_author)
   (get_discussions_by_payout)
   (get_post_discussions_by_payout)
   (get_comment_discussions_by_payout)
   (get_discussions_by_trending)
   (get_discussions_by_created)
   (get_discussions_by_active)
   (get_discussions_by_cashout)
   (get_discussions_by_votes)
   (get_discussions_by_children)
   (get_discussions_by_hot)
   (get_discussions_by_feed)
   (get_discussions_by_blog)
   (get_discussions_by_comments)
   (get_discussions_by_promoted)

   // Blocks and transactions
   (get_block_header)
   (get_block)
   (get_ops_in_block)
   (get_state)

   // Globals
   (get_config)
   (get_dynamic_global_properties)
   (get_chain_properties)
   (get_feed_history)
   (get_current_median_history_price)
   (get_witness_schedule)
   (get_hardfork_version)
   (get_next_scheduled_hardfork)
   (get_reward_fund)

   // Keys
   (get_key_references)

   // Accounts
   (get_accounts)
   (get_account_references)
   (lookup_account_names)
   (lookup_accounts)
   (get_account_count)
   (get_conversion_requests)
   (get_account_history)
   (get_owner_history)
   (get_recovery_request)
   (get_escrow)
   (get_withdraw_routes)
   (get_account_bandwidth)
   (get_savings_withdraw_from)
   (get_savings_withdraw_to)
   (get_asset_delegations)
   (get_expiring_asset_delegations)

   // Market
   (get_order_book)
   (get_open_orders)
   (get_liquidity_queue)

   // Authority / validation
   (get_transaction_hex)
   (get_transaction)
   (get_required_signatures)
   (get_potential_signatures)
   (verify_authority)
   (verify_account_authority)

   // votes
   (get_active_votes)
   (get_account_votes)

   // content
   (get_content)
   (get_content_replies)
   (get_discussions_by_author_before_date)
   (get_replies_by_last_update)


   // Witnesses
   (get_witnesses)
   (get_witness_by_account)
   (get_witnesses_by_voting_power)
   (lookup_witness_accounts)
   (get_witness_count)
   (get_active_witnesses)
   (get_miner_queue)
)
