#pragma once
#include <node/app/applied_operation.hpp>
#include <node/app/state.hpp>

#include <node/chain/database.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/history_object.hpp>
#include <node/producer/producer_plugin.hpp>
#include <node/tags/tags_plugin.hpp>


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


class database_api_impl;

/**
 * Defines the arguments to a discussion query.
 */
struct discussion_query 
{
   void validate()const
   {
      FC_ASSERT( filter_authors.find( account ) == filter_authors.end() );
      FC_ASSERT( filter_communities.find( community ) == filter_communities.end() );
      FC_ASSERT( filter_tags.find( tag ) == filter_tags.end() );
      FC_ASSERT( limit <= 100 );
      FC_ASSERT( max_rating >= 1 && max_rating <= 9 );
   }

   string                  account = INIT_ACCOUNT;       ///< Name of the account being fetched for feed or blog queries.

   string                  community = string();         ///< Name of the community being queried.

   string                  tag = string();               ///< Name of the tag being querired.

   string                  sort_option = "quality";      ///< Sorting index type.

   string                  sort_time = "standard";       ///< Time preference of the sorting type.
   
   string                  feed_type = "follow";         ///< Type of feed being queried.

   string                  blog_type = "account";        ///< Type of blog being queried.

   string                  post_include_time = "all";    ///< Time limit for including posts.

   bool                    include_private = false;      ///< True to include private encrypted posts.

   uint16_t                max_rating = 9;               ///< Highest content rating to include in posts queried.
   
   uint32_t                limit = 20;                   ///< Amount of discussions to return.

   set<string>             select_communities;           ///< list of communities to include.

   set<string>             filter_communities;           ///< list of communities to filter, posts made in these communities are filtered.
   
   set<string>             select_tags;                  ///< list of tags to include.

   set<string>             filter_tags;                  ///< list of tags to filter, posts with these tags are filtered.

   set<string>             select_authors;               ///< list of authors to include.

   set<string>             filter_authors;               ///< list of authors to filter, posts by these authors are filtered.

   set<string>             select_languages;             ///< list of languages to include.

   set<string>             filter_languages;             ///< list of languages to filter, posts made in these communities are filtered.

   optional<string>        start_author;                 ///< The Author of the first post to include in the ranking.

   optional<string>        start_permlink;               ///< The Permlink of the first post to include in the ranking.

   optional<string>        parent_author;                ///< The Author of the parent post to to query comments from.

   optional<string>        parent_permlink;              ///< The Permlink of the parent post to to query comments from.

   uint32_t                truncate_body = 0;            ///< the number of bytes of the post body to return, 0 for all.
};


/**
 * Defines the arguments to a network object search query.
 */
struct search_query 
{
   void validate()const
   {
      FC_ASSERT( limit <= 100 );
      FC_ASSERT( margin_percent <= uint16_t( PERCENT_100 ) );
   }

   string                            account;                            ///< Name of the account creating the search.

   string                            query;                              ///< Search String being queried.

   uint32_t                          limit = 20;                         ///< The amount of results to include in the results.

   uint16_t                          margin_percent = PERCENT_1 * 25;    ///< Search result must match within this percentage to be included.

   bool                              include_accounts = true;            ///< Set True to include account results.

   bool                              include_communities = true;         ///< Set True to include community results.

   bool                              include_tags = true;                ///< Set True to include tag results.

   bool                              include_assets = true;              ///< Set True to include asset results.

   bool                              include_posts = true;               ///< Set True to include post results by title.
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

   string                  interface;             ///< Name of the interface account of the ad inventory provider.

   string                  viewer;                ///< Name of the audience member account receiving the ad.

   string                  ad_metric;             ///< Type of ad price metric being queried.

   string                  ad_format;             ///< Type of ad inventory format being queried.
   
   uint32_t                limit = 0;
};


/**
 * Defines the arguments to a graph query.
 */
struct graph_query 
{
   void validate()const
   {
      FC_ASSERT( limit <= 1000 );
   }

   set< string >                  select_accounts;                           ///< Name of the graph node and edge creator accounts to include.

   set< string >                  filter_accounts;                           ///< Name of the graph node and edge creator accounts to not include.

   bool                           include_private = false;                   ///< True to include private graphs.


   set< string >                  intersect_select_node_types;               ///< Name of the graph node types to include.

   set< string >                  intersect_filter_node_types;               ///< Name of the graph node types to not include.

   set< string >                  union_select_node_types;                   ///< Name of the graph node types to include.

   set< string >                  union_filter_node_types;                   ///< Name of the graph node types to not include.


   vector< string >               node_intersect_select_attributes;          ///< Nodes are included if they include all attributes.

   vector< string >               node_union_select_attributes;              ///< Nodes are included if they include any of these attributes.

   vector< string >               node_intersect_filter_attributes;          ///< Nodes are not included if they include all attributes.

   vector< string >               node_union_filter_attributes;              ///< Nodes are not included if they include any of these attributes.


   vector< string >               node_intersect_select_values;              ///< Nodes are included if they include all values for intersect select attributes.

   vector< string >               node_union_select_values;                  ///< Nodes are included if they include any of these union select attributes.

   vector< string >               node_intersect_filter_values;              ///< Nodes are not included if they include all intersect filter attributes.

   vector< string >               node_union_filter_values;                  ///< Nodes are not included if they include any of these union filter attributes.


   set< string >                  intersect_select_edge_types;               ///< Name of the graph edge types to include.

   set< string >                  intersect_filter_edge_types;               ///< Name of the graph edge types to not include.

   set< string >                  union_select_edge_types;                   ///< Name of the graph edge types to include.

   set< string >                  union_filter_edge_types;                   ///< Name of the graph edge types to not include.


   vector< string >               edge_intersect_select_attributes;          ///< Edges are included if they include all attributes.

   vector< string >               edge_union_select_attributes;              ///< Edges are included if they include any of these attributes.

   vector< string >               edge_intersect_filter_attributes;          ///< Edges are not included if they include all attributes.

   vector< string >               edge_union_filter_attributes;              ///< Edges are not included if they include any of these attributes.
   

   vector< string >               edge_intersect_select_values;              ///< Edges are included if they include all values for intersect select attributes.

   vector< string >               edge_union_select_values;                  ///< Edges are included if they include any of these union select attributes.

   vector< string >               edge_intersect_filter_values;              ///< Edges are not included if they include all intersect filter attributes.

   vector< string >               edge_union_filter_values;                  ///< Edges are not included if they include any of these union filter attributes.

   
   uint32_t                       limit = 0;
};


/**
 * Defines the arguments to a graph query.
 */
struct confidential_query 
{
   void validate()const
   {
      FC_ASSERT( limit <= 1000 );
   }

   set< commitment_type >         select_commitments;           ///< Commitments to include balances.

   set< string >                  select_account_auths;         ///< Account auths to include balances.

   set< string >                  select_key_auths;             ///< Key auths to include balances.
   
   uint32_t                       limit = 0;
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


      //=================//
      // === Globals === //
      //=================//


      fc::variant_object                              get_config()const;

      dynamic_global_property_api_obj                 get_dynamic_global_properties()const;

      median_chain_property_api_obj                   get_median_chain_properties()const;

      producer_schedule_api_obj                       get_producer_schedule()const;

      hardfork_version                                get_hardfork_version()const;

      scheduled_hardfork                              get_next_scheduled_hardfork()const;


      //===================//
      // === Accounts ==== //
      //===================//


      vector< account_api_obj >                       get_accounts( vector< string > names )const;

      vector< account_api_obj >                       get_accounts_by_followers( string from, uint32_t limit )const;

      vector< account_concise_api_obj >               get_concise_accounts( vector< string > names )const;

      vector< extended_account >                      get_full_accounts( vector< string > names )const;

      map< uint32_t, applied_operation >              get_account_history( string account, uint64_t from, uint32_t limit )const;

      vector< message_state >                         get_messages( vector< string > names )const;

      vector< balance_state >                         get_balances( vector< string > names )const;

      vector< confidential_balance_api_obj >          get_confidential_balances( const confidential_query& query )const;

      vector< key_state >                             get_keychains( vector< string > names)const;

      set< string >                                   lookup_accounts( string lower_bound_name, uint32_t limit )const;

      uint64_t                                        get_account_count()const;

      vector< owner_authority_history_api_obj >       get_owner_history( string account )const;

      optional< account_recovery_request_api_obj >    get_recovery_request( string account )const;

      optional< account_bandwidth_api_obj >           get_account_bandwidth( string account, producer::bandwidth_type type )const;


      //================//
      // === Assets === //
      //================//


      vector< extended_asset >                        get_assets( vector< string > assets )const;

      optional< escrow_api_obj >                      get_escrow( string from, string escrow_id )const;


      //=====================//
      // === Communities === //
      //=====================//


      vector< extended_community >                    get_communities( vector< string > communities )const;

      vector< extended_community >                    get_communities_by_subscribers( string from, uint32_t limit )const;


      //=================//
      // === Network === //
      //=================//


      vector< account_network_state >                 get_account_network_state( vector< string > names )const;

      vector< account_name_type >                     get_active_producers()const;

      vector< producer_api_obj >                      get_producers_by_voting_power( string from, uint32_t limit )const;

      vector< producer_api_obj >                      get_producers_by_mining_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >               get_development_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >               get_marketing_officers_by_voting_power( string from, uint32_t limit )const;

      vector< network_officer_api_obj >               get_advocacy_officers_by_voting_power( string from, uint32_t limit )const;

      vector< executive_board_api_obj >               get_executive_boards_by_voting_power( string from, uint32_t limit )const;

      vector< supernode_api_obj >                     get_supernodes_by_view_weight( string from, uint32_t limit )const;

      vector< interface_api_obj >                     get_interfaces_by_users( string from, uint32_t limit )const;

      vector< governance_account_api_obj >            get_governance_accounts_by_subscriber_power( string from, uint32_t limit )const;

      vector< community_enterprise_api_obj >          get_enterprise_by_voting_power( string from, string from_id, uint32_t limit )const;


      //================//
      // === Market === //
      //================//


      vector< order_state >                           get_open_orders( vector< string > names )const;

      market_limit_orders                             get_limit_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_margin_orders                            get_margin_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_option_orders                            get_option_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_call_orders                              get_call_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_auction_orders                           get_auction_orders( string buy_symbol, string sell_symbol, uint32_t limit )const;

      market_credit_loans                             get_credit_loans( string buy_symbol, string sell_symbol, uint32_t limit )const;

      vector< credit_pool_api_obj >                   get_credit_pools( vector< string > assets )const;

      vector< liquidity_pool_api_obj >                get_liquidity_pools( string buy_symbol, string sell_symbol )const;

      vector< option_pool_api_obj >                   get_option_pools( string buy_symbol, string sell_symbol )const;

      market_state                                    get_market_state( string buy_symbol, string sell_symbol )const;


      //=============//
      // === Ads === //
      //=============//


      vector< account_ad_state >                      get_account_ads( vector< string > names )const;

      vector< ad_bid_state >                          get_interface_audience_bids( const ad_query& query )const;


      //==================//
      // === Products === //
      //==================//


      product_api_obj                                 get_product( string seller, string product_id )const;

      vector< account_product_state >                 get_account_products( vector< string > names )const;


      //=====================//
      // === Graph Data  === //
      //=====================//


      graph_data_state                                get_graph_query( const graph_query& query )const;

      vector< graph_node_property_api_obj >           get_graph_node_properties( vector< string > names )const;

      vector< graph_edge_property_api_obj >           get_graph_edge_properties( vector< string > names )const;

      
      //================//
      // === Search === //
      //================//


      search_result_state                             get_search_query( const search_query& query )const;


      //=================================//
      // === Blocks and Transactions === //
      //=================================//


      optional< block_header >                        get_block_header( uint64_t block_num )const;

      optional< signed_block_api_obj >                get_block( uint64_t block_num )const;

      vector< applied_operation >                     get_ops_in_block( uint64_t block_num, bool only_virtual = true)const;

      std::string                                     get_transaction_hex( const signed_transaction& trx )const;

      annotated_signed_transaction                    get_transaction( transaction_id_type trx_id )const;

      set<public_key_type>                            get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;

      set<public_key_type>                            get_potential_signatures( const signed_transaction& trx )const;

      bool                                            verify_authority( const signed_transaction& trx )const;

      bool                                            verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;


      //======================//
      // === Posts + Tags === //
      //======================//


      vector< vote_state >                            get_active_votes( string author, string permlink )const;

      vector< view_state >                            get_active_views( string author, string permlink )const;

      vector< share_state >                           get_active_shares( string author, string permlink )const;

      vector< moderation_state >                      get_active_mod_tags( string author, string permlink )const;

      vector< account_vote >                          get_account_votes( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< account_view >                          get_account_views( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< account_share >                         get_account_shares( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< account_moderation >                    get_account_moderation( string account, string from_author, string from_permlink, uint32_t limit )const;

      vector< tag_following_api_obj >                 get_tag_followings( vector< string > tags )const;

      vector< tag_api_obj >                           get_top_tags( string after_tag, uint32_t limit )const;

      vector< pair< tag_name_type, uint32_t > >       get_tags_used_by_author( string author )const;


      //=====================//
      // === Discussions === //
      //=====================//


      discussion                                      get_content( string author, string permlink )const;
      
      vector< discussion >                            get_content_replies( string parent, string parent_permlink )const;

      vector< discussion >                            get_replies_by_last_update( account_name_type start_author, string start_permlink, uint32_t limit )const;

      vector< discussion >                            get_discussions_by_sort_rank( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_feed( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_blog( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_recommended( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_comments( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_payout(const discussion_query& query )const;

      vector< discussion >                            get_post_discussions_by_payout( const discussion_query& query )const;

      vector< discussion >                            get_comment_discussions_by_payout( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_created( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_active( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_votes( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_views( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_shares( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_children( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_vote_power( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_view_power( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_share_power( const discussion_query& query )const;

      vector< discussion >                            get_discussions_by_comment_power( const discussion_query& query )const;



      //===============//
      // === State === //
      //===============//


      state                                           get_state( string path )const;


      //=======================//
      // === Subscriptions === //
      //=======================//
   

      void                                            set_block_applied_callback( std::function< void(const variant& block_header ) > cb );


      //=========================//
      // === Signal Handlers === //
      //=========================//


      void                                            on_api_startup();

   private:
      discussion get_discussion( comment_id_type id, uint32_t truncate_body )const;

      static bool filter_default( const comment_api_obj& c ) { return false; }
      static bool exit_default( const comment_api_obj& c ) { return false; }
      static bool tag_exit_default( const tags::tag_object& c ) { return false; }

      template< typename Index, typename StartItr >
      vector< discussion > get_discussions( 
         const discussion_query& q,
         const string& community,
         const string& tag,
         comment_id_type parent,
         const Index& idx, 
         StartItr itr,
         uint32_t truncate_body,
         const std::function< bool( const comment_api_obj& ) >& filter,
         const std::function< bool( const comment_api_obj& ) >& exit,
         const std::function< bool( const tags::tag_object& ) >& tag_exit,
         bool ignore_parent
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

FC_REFLECT( node::app::discussion_query,
         (account)
         (community)
         (tag)
         (sort_option)
         (sort_time)
         (feed_type)
         (blog_type)
         (include_private)
         (max_rating)
         (limit)
         (select_communities)
         (filter_communities)
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
         (ad_format)
         (ad_metric)
         (limit)
         );

FC_REFLECT( node::app::graph_query,
         (select_accounts)
         (filter_accounts)
         (include_private)
         (intersect_select_node_types)
         (intersect_filter_node_types)
         (union_select_node_types)
         (union_filter_node_types)
         (node_intersect_select_attributes)
         (node_union_select_attributes)
         (node_intersect_filter_attributes)
         (node_union_filter_attributes)
         (node_intersect_select_values)
         (node_union_select_values)
         (node_intersect_filter_values)
         (node_union_filter_values)
         (intersect_select_edge_types)
         (intersect_filter_edge_types)
         (union_select_edge_types)
         (union_filter_edge_types)
         (edge_intersect_select_attributes)
         (edge_union_select_attributes)
         (edge_intersect_filter_attributes)
         (edge_union_filter_attributes)
         (edge_intersect_select_values)
         (edge_union_select_values)
         (edge_intersect_filter_values)
         (edge_union_filter_values)
         (limit)
         );

FC_REFLECT( node::app::confidential_query,
         (select_commitments)
         (select_account_auths)
         (select_key_auths)
         (limit)
         );

FC_API( node::app::database_api,

         // Globals

         (get_config)
         (get_dynamic_global_properties)
         (get_median_chain_properties)
         (get_producer_schedule)
         (get_hardfork_version)
         (get_next_scheduled_hardfork)

         // Accounts

         (get_accounts)
         (get_accounts_by_followers)
         (get_concise_accounts)
         (get_full_accounts)
         (get_account_history)
         (get_messages)
         (get_balances)
         (get_confidential_balances)
         (get_keychains)
         (lookup_accounts)
         (get_account_count)
         (get_owner_history)
         (get_recovery_request)
         (get_account_bandwidth)

         // Assets

         (get_assets)
         (get_escrow)

         // Communities

         (get_communities)
         (get_communities_by_subscribers)

         // Network

         (get_account_network_state)
         (get_active_producers)
         (get_producers_by_voting_power)
         (get_producers_by_mining_power)
         (get_development_officers_by_voting_power)
         (get_marketing_officers_by_voting_power)
         (get_advocacy_officers_by_voting_power)
         (get_executive_boards_by_voting_power)
         (get_supernodes_by_view_weight)
         (get_interfaces_by_users)
         (get_governance_accounts_by_subscriber_power)
         (get_enterprise_by_voting_power)
         
         
         // Market

         (get_open_orders)
         (get_limit_orders)
         (get_margin_orders)
         (get_option_orders)
         (get_call_orders)
         (get_auction_orders)
         (get_credit_loans)
         (get_credit_pools)
         (get_liquidity_pools)
         (get_option_pools)
         (get_market_state)

         // Ads

         (get_account_ads)
         (get_interface_audience_bids)

         // Products

         (get_product)
         (get_account_products)

         // Graph
         
         (get_graph_query)
         (get_graph_node_properties)
         (get_graph_edge_properties)

         // Search 

         (get_search_query)

         // Blocks and transactions

         (get_block_header)
         (get_block)
         (get_ops_in_block)
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
         (get_account_views)
         (get_account_shares)
         (get_account_moderation)
         (get_tag_followings)
         (get_top_tags)
         (get_tags_used_by_author)

         // Discussions

         (get_content)
         (get_content_replies)
         (get_replies_by_last_update)
         (get_discussions_by_sort_rank)
         (get_discussions_by_feed)
         (get_discussions_by_blog)
         (get_discussions_by_recommended)
         (get_discussions_by_comments)

         (get_discussions_by_payout)
         (get_post_discussions_by_payout)
         (get_comment_discussions_by_payout)

         (get_discussions_by_created)
         (get_discussions_by_active)
         (get_discussions_by_votes)
         (get_discussions_by_views)
         (get_discussions_by_shares)
         (get_discussions_by_children)
         (get_discussions_by_vote_power)
         (get_discussions_by_view_power)
         (get_discussions_by_share_power)
         (get_discussions_by_comment_power)
         
         // State 

         (get_state)

         // Subscriptions

         (set_block_applied_callback)

         );