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
};

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
};

enum post_time_type
{
   ALL_TIME         = 'all',
   LAST_HOUR        = 'hour',
   LAST_DAY         = 'day',
   LAST_WEEK        = 'week',
   LAST_MONTH       = 'month',
   LAST_YEAR        = 'year'
};

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

   string                  post_include_time;     // Time limit for including posts.

   bool                    include_private;       // True to include private encrypted posts.

   string                  max_rating;            // Highest content rating to include in posts queried.
   
   uint32_t                limit = 0;             // Amount of discussions to return

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

   discussion_query        discussion_query;      // The Discussion feed display query of the ad context.

   search_query            search_query;          // the Search display query of the ad context.
   
   uint32_t                limit = 0;
};

/**
 * The database_api class implements the RPC API for the chain database.
 *
 * This API exposes accessors on the database which query state 
 * tracked by a blockchain validating node. 
 * This API is read-only; all modifications to the database 
 * must be performed via transactions. Transactions are broadcast via
 * the network_broadcast_api.
 */
class database_api
{
   public:
      database_api( const node::app::api_context& ctx );
      ~database_api();


      //=======================//
      // === Subscriptions === //
      //=======================//
   

      void                                set_block_applied_callback( std::function< void(const variant& block_header ) > cb );


      //=================================//
      // === Blocks and transactions === //
      //=================================//


      optional< block_header >            get_block_header( uint32_t block_num )const;

      optional< signed_block_api_obj >    get_block( uint32_t block_num )const;

      vector< applied_operation >         get_ops_in_block( uint32_t block_num, bool only_virtual = true)const;


      //=================//
      // === Globals === //
      //=================//


      fc::variant_object                get_config()const;

      dynamic_global_property_api_obj   get_dynamic_global_properties()const;

      chain_properties                  get_chain_properties()const;

      witness_schedule_api_obj          get_witness_schedule()const;

      hardfork_version                  get_hardfork_version()const;

      scheduled_hardfork                get_next_scheduled_hardfork()const;

      reward_fund_api_obj               get_reward_fund()const;


      //===================//
      // === Accounts ==== //
      //===================//


      vector< account_api_obj >                       get_accounts( vector< string > names ) const;

      vector< account_api_obj >                       get_accounts_by_followers( string from, uint32_t limit ) const;

      vector< account_concise_api_obj >               get_concise_accounts( vector< string > names ) const;

      vector< extended_account >                      get_full_accounts( vector< string > names ) const;

      map< uint32_t, applied_operation >              get_account_history( string account, uint64_t from, uint32_t limit )const;

      vector< message_state >                         get_messages( vector< string > names ) const;

      vector< balance_state >                         get_balances( vector< string > names ) const;

      vector< key_state >                             get_keychains( vector< string > names) const;

      vector< optional< account_api_obj > >           lookup_account_names( vector< string > account_names )const;

      set< string >                                   lookup_accounts( string lower_bound_name, uint32_t limit )const;

      uint64_t                                        get_account_count()const;

      vector< owner_authority_history_api_obj >       get_owner_history( string account )const;

      optional< account_recovery_request_api_obj >    get_recovery_request( string account ) const;

      optional< account_bandwidth_api_obj >           get_account_bandwidth( string account, witness::bandwidth_type type )const;


      //================//
      // === Assets === //
      //================//


      vector< extended_asset >                        get_assets( vector< string > assets )const;

      uint64_t                                        get_asset_count()const;

      optional< escrow_api_obj >                      get_escrow( string from, string escrow_id )const;

      vector< withdraw_route >                        get_withdraw_routes( string account, withdraw_route_type type = outgoing )const;

      vector< savings_withdraw_api_obj >              get_savings_withdraw_from( string account )const;

      vector< savings_withdraw_api_obj >              get_savings_withdraw_to( string account )const;

      vector< asset_delegation_api_obj >              get_asset_delegations( string account, string from, uint32_t limit = 100 )const;

      vector< asset_delegation_expiration_api_obj >   get_expiring_asset_delegations( string account, time_point from, uint32_t limit = 100 )const;


      //================//
      // === Boards === //
      //================//


      vector< extended_board >                        get_boards( vector< string > boards )const;

      vector< extended_board >                        get_boards_by_subscribers( string from, uint32_t limit )const;

      uint64_t                                        get_board_count()const;


      //=================//
      // === Network === //
      //=================//


      vector< account_name_type >                     get_active_producers()const;

      vector< optional< witness_api_obj > >           get_witnesses( vector< witness_id_type > witness_ids )const;

      set< account_name_type >                        lookup_witness_accounts( string lower_bound_name, uint32_t limit )const;

      uint64_t                                        get_witness_count()const;

      fc::optional< witness_api_obj >                 get_witness_by_account( string account_name )const;

      vector< witness_api_obj >                       get_witnesses_by_voting_power( string from, uint32_t limit )const;

      vector< witness_api_obj >                       get_witnesses_by_mining_power( string from, uint32_t limit )const;

      vector< network_officers_api_obj >              get_development_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officers_api_obj >              get_marketing_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officers_api_obj >              get_advocacy_officers_by_voting_power( string from, uint32_t limit )const;

      vector< executive_board_api_obj >               get_executive_boards_by_voting_power( string from, uint32_t limit )const;

      vector< supernode_api_obj >                     get_supernodes_by_view_weight( string from, uint32_t limit )const;

      vector< interface_api_obj >                     get_interfaces_by_users( string from, uint32_t limit )const;

      vector< governance_account_api_obj >            get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const;

      vector< community_enterprise_api_obj >          get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const;


      //================//
      // === Market === //
      //================//


      vector< order_state >                get_open_orders( vector< string > names )const;

      market_limit_orders                  get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_margin_orders                 get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_call_orders                   get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      market_credit_loans                  get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit ) const;

      vector< credit_pool_api_obj >        get_credit_pools( vector< string > assets ) const;

      vector< liquidity_pool_api_obj >     get_liquidity_pools( string buy_symbol, string sell_symbol ) const;

      market_state                         get_market_state( string buy_symbol, string sell_symbol )const;


      //=============//
      // === Ads === //
      //=============//


      vector< account_ad_state >           get_account_ads( vector< string > names )const;

      vector< ad_bid_state >               get_interface_audience_bids( const ad_query& query )const;


      //================//
      // === Search === //
      //================//


      search_result_state                  get_search_query( const search_query& query )const;  


      //================================//
      // === Authority / validation === //
      //================================//


      std::string                          get_transaction_hex( const signed_transaction& trx )const;

      annotated_signed_transaction         get_transaction( transaction_id_type trx_id )const;

      set<public_key_type>                 get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;

      set<public_key_type>                 get_potential_signatures( const signed_transaction& trx )const;

      bool                                 verify_authority( const signed_transaction& trx )const;

      bool                                 verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;


      //======================//
      // === Posts + Tags === //
      //======================//


      vector< vote_state >                 get_active_votes( string author, string permlink )const;

      vector< view_state >                 get_active_views( string author, string permlink )const;

      vector< share_state >                get_active_shares( string author, string permlink )const;

      vector< moderation_state >           get_active_mod_tags( string author, string permlink )const;

      vector< account_vote >               get_account_votes( string account )const;

      vector< account_view >               get_account_views( string account )const;

      vector< account_share >              get_account_shares( string account )const;

      vector< account_moderation >         get_account_moderation( string account )const;

      vector< tag_following_api_obj >      get_tag_followings( vector< string > tags )const;

      vector< tag_api_obj >                get_top_tags( string after_tag, uint32_t limit )const;

      vector< pair< string,uint32_t > >    get_tags_used_by_author( string author )const;


      //=====================//
      // === Discussions === //
      //=====================//


      discussion                           get_content( string author, string permlink )const;
      
      vector< discussion >                 get_content_replies( string parent, string parent_permlink )const;

      vector< discussion >                 get_replies_by_last_update( account_name_type start_author, string start_permlink, uint32_t limit )const;

      vector< discussion >                 get_discussions_by_payout(const discussion_query& query )const;

      vector< discussion >                 get_post_discussions_by_payout( const discussion_query& query )const;

      vector< discussion >                 get_comment_discussions_by_payout( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_index( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_created( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_active( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_cashout( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_votes( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_views( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_shares( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_children( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_vote_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_view_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_share_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_comment_power( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_feed( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_blog( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_recommended( const discussion_query& query )const;

      vector< discussion >                 get_discussions_by_comments( const discussion_query& query )const;
      
      vector< discussion >                 get_discussions_by_author_before_date( string author, string start_permlink, time_point before_date, uint32_t limit )const;


      //===============//
      // === State === //
      //===============//


      state                                get_state( string path )const;

      //=========================//
      // === Signal Handlers === //
      //=========================//


      void on_api_startup();

   private:
      void set_pending_payout( discussion& d )const;
      void set_url( discussion& d )const;
      discussion get_discussion( comment_id_type, uint32_t truncate_body = 0 )const;

      static bool filter_default( const comment_api_obj& c ) { return false; }
      static bool exit_default( const comment_api_obj& c )   { return false; }
      static bool tag_exit_default( const tags::tag_object& c ) { return false; }

      template< typename Index, typename StartItr >
      vector< discussion > get_discussions( 
         const discussion_query& q,
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


FC_REFLECT( node::app::scheduled_hardfork,
         (hf_version)
         (live_time)
         );

FC_REFLECT( node::app::withdraw_route,
         (from_account)
         (to_account)
         (percent)
         (auto_stake)
         );

FC_REFLECT_ENUM( node::app::withdraw_route_type,
         (incoming)
         (outgoing)
         (all)
         );

FC_REFLECT_ENUM( node::app::sort_time,
         (NO_TIME)
         (ACTIVE_TIME)
         (RAPID_TIME)
         (STANDARD_TIME)
         (TOP_TIME)
         (ELITE_TIME)
         );

FC_REFLECT_ENUM( node::app::sort_type,
         (NO_SORT)
         (VOTES_SORT)
         (VIEWS_SORT)
         (SHARES_SORT)
         (COMMENTS_SORT)
         (QUALITY_SORT)
         (POPULAR_SORT)
         (VIRAL_SORT)
         (DISCUSSION_SORT)
         (PROMINENT_SORT)
         (CONVERSATION_SORT)
         (DISCOURSE_SORT)
         );

FC_REFLECT( node::app::discussion_query,
         (account)
         (board)
         (tag)
         (sort_type)
         (sort_time)
         (feed_type)
         (blog_type)
         (include_private)
         (max_rating)
         (limit)
         (select_boards)
         (filter_boards)
         (select_tags)
         (filter_tags)
         (select_authors)
         (filter_authors)
         (select_languages)
         (filter_languages)
         (start_author)
         (start_permlink)
         (parent_author)
         (parent_permlink)
         (truncate_body)
         );

FC_REFLECT( node::app::search_query,
         (account)
         (query)
         (limit)
         );

FC_REFLECT( node::app::ad_query,
         (interface)
         (viewer)
         (format_type)
         (discussions_query)
         (search_query)
         (limit)
         );

FC_API( node::app::database_api,

         // Subscriptions

         (set_block_applied_callback)

         // Blocks and transactions

         (get_block_header)
         (get_block)
         (get_ops_in_block)
         
         // Globals

         (get_config)
         (get_dynamic_global_properties)
         (get_chain_properties)
         (get_witness_schedule)
         (get_hardfork_version)
         (get_next_scheduled_hardfork)
         (get_reward_fund)
         (get_state)

         // Accounts

         (get_accounts)
         (get_concise_accounts)
         (get_full_accounts)
         (get_account_history)
         (get_messages)
         (get_balances)
         (get_keychains)
         (lookup_account_names)
         (lookup_accounts)
         (get_account_count)
         (get_owner_history)
         (get_recovery_request)

         // Assets

         (get_assets)
         (get_escrow)
         (get_withdraw_routes)
         (get_account_bandwidth)
         (get_savings_withdraw_from)
         (get_savings_withdraw_to)
         (get_asset_delegations)
         (get_expiring_asset_delegations)

         // Boards

         (get_boards)
         (get_board_count)

         // Network

         (get_active_producers)
         (get_witnesses)
         (get_witness_by_account)
         (get_witnesses_by_voting_power)
         (get_witnesses_by_mining_power)
         (get_development_officers_by_voting_power)
         (get_marketing_officers_by_voting_power)
         (get_advocacy_officers_by_voting_power)
         (get_executive_boards_by_voting_power)
         (get_supernodes_by_view_weight)
         (get_interfaces_by_users)
         (get_governance_accounts_by_subscriber_power)
         (get_enterprise_by_voting_power)
         (lookup_witness_accounts)
         (get_witness_count)

         // Market

         (get_open_orders)
         (get_limit_orders)
         (get_margin_orders)
         (get_call_orders)
         (get_credit_loans)
         (get_credit_pools)
         (get_liquidity_pools)

         // Ads

         (get_account_ads)
         (get_interface_audience_bids)

         // Search 

         (get_search_query)

         // Authority / validation

         (get_transaction_hex)
         (get_transaction)
         (get_required_signatures)
         (get_potential_signatures)
         (verify_authority)
         (verify_account_authority)

         // Posts + Tags

         (get_active_votes)
         (get_active_views)
         (get_active_shares)
         (get_active_mod_tags)
         (get_account_votes)
         (get_top_tags)
         (get_tags_used_by_author)

         // Discussions

         (get_content)
         (get_content_replies)
         (get_discussions_by_payout)
         (get_post_discussions_by_payout)
         (get_comment_discussions_by_payout)
         (get_discussions_by_index)
         (get_discussions_by_created)
         (get_discussions_by_active)
         (get_discussions_by_cashout)
         (get_discussions_by_votes)
         (get_discussions_by_views)
         (get_discussions_by_shares)
         (get_discussions_by_children)
         (get_discussions_by_feed)
         (get_discussions_by_blog)
         (get_discussions_by_recommended)
         (get_discussions_by_comments)
         (get_replies_by_last_update)
         (get_discussions_by_author_before_date)
         );