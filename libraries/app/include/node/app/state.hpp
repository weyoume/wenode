#pragma once
#include <node/app/applied_operation.hpp>
#include <node/app/node_api_objects.hpp>

#include <node/chain/global_property_object.hpp>
#include <node/chain/account_object.hpp>
#include <node/chain/node_objects.hpp>

namespace node { namespace app {
   using std::string;
   using std::vector;

   struct discussion_index
   {
      string                category;
      vector< string >      payout;
      vector< string >      payout_comments;
      vector< string >      created;
      vector< string >      responses;
      vector< string >      updated;
      vector< string >      active;
      vector< string >      cashout;
      vector< string >      best;

      vector< string >      net_votes;
      vector< string >      view_count;
      vector< string >      share_count;
      vector< string >      comment_count;
      vector< string >      vote_power;
      vector< string >      view_power;
      vector< string >      share_power;
      vector< string >      comment_power;
   };

   struct tag_index
   {
      vector< string > trending; // pending payouts
   };

   struct vote_state
   {
      string         voter;
      uint128_t      weight = 0;
      int64_t       reward = 0;
      int16_t        percent = 0;
      time_point     time;
   };

   struct view_state
   {
      string         viewer;
      uint128_t      weight = 0;
      int64_t       reward = 0;
      time_point     time;
   };

   struct share_state
   {
      string         sharer;
      uint128_t      weight = 0;
      int64_t       reward = 0;
      time_point     time;
   };

   struct moderation_state
   {
      string                     moderator;
      vector< tag_name_type >    tags;             // Set of additional string tags for sorting the post by
      uint16_t                   rating;           // Moderator updated rating as to the maturity of the content, and display sensitivity. 
      string                     details;          // Explanation as to what rule the post is in contravention of and why it was tagged.
      bool                       filter;           // True if the post should be filtered by the community or governance address subscribers.
      time_point                 time;
   };

   struct account_vote
   {
      string         author;
      string         permlink;
      uint128_t      weight = 0;
      int64_t        reward = 0;
      int16_t        percent = 0;
      time_point     time;
   };

   struct account_view
   {
      string         author;
      string         permlink;
      uint128_t      weight = 0;
      int64_t        reward = 0;
      time_point     time;
   };

   struct account_share
   {
      string         author;
      string         permlink;
      uint128_t      weight = 0;
      int64_t        reward = 0;
      time_point     time;
   };

   struct account_moderation
   {
      string                     author;
      string                     permlink;
      vector< tag_name_type >    tags;             // Set of additional string tags for sorting the post by.
      string                     rating;           // Moderator updated rating as to the maturity of the content, and display sensitivity.
      string                     details;          // Explanation as to what rule the post is in contravention of and why it was tagged.
      bool                       filter;           // True if the post should be filtered by the community or governance address subscribers.
      time_point                 time;
   };

   struct order_state
   {
      vector< limit_order_api_obj >          limit_orders;
      vector< margin_order_api_obj >         margin_orders;
      vector< call_order_api_obj >           call_orders;
      vector< credit_loan_api_obj >          loan_orders;
      vector< credit_collateral_api_obj >    collateral;
   };

   struct operation_state
   {
      map< uint64_t, applied_operation >    account_history;
      map< uint64_t, applied_operation >    connection_history;
      map< uint64_t, applied_operation >    follow_history;
      map< uint64_t, applied_operation >    activity_history;
      map< uint64_t, applied_operation >    post_history;
      map< uint64_t, applied_operation >    message_history;
      map< uint64_t, applied_operation >    vote_history;
      map< uint64_t, applied_operation >    view_history;
      map< uint64_t, applied_operation >    share_history;
      map< uint64_t, applied_operation >    moderation_history;
      map< uint64_t, applied_operation >    community_history;
      map< uint64_t, applied_operation >    ad_history;
      map< uint64_t, applied_operation >    transfer_history;
      map< uint64_t, applied_operation >    balance_history;
      map< uint64_t, applied_operation >    escrow_history;
      map< uint64_t, applied_operation >    trading_history;
      map< uint64_t, applied_operation >    liquidity_history;
      map< uint64_t, applied_operation >    credit_history;
      map< uint64_t, applied_operation >    asset_history;
      map< uint64_t, applied_operation >    network_history;
      map< uint64_t, applied_operation >    other_history;
   };

   struct transfer_state
   {
      map< account_name_type, transfer_request_api_obj >                   incoming_requests;
      map< account_name_type, transfer_request_api_obj >                   outgoing_requests;
      map< account_name_type, transfer_recurring_api_obj >                 incoming_recurring_transfers;
      map< account_name_type, transfer_recurring_api_obj >                 outgoing_recurring_transfers;
      map< account_name_type, transfer_recurring_request_api_obj >         incoming_recurring_transfer_requests;
      map< account_name_type, transfer_recurring_request_api_obj >         outgoing_recurring_transfer_requests;
   };

   struct community_state
   {
      map< community_name_type, community_request_api_obj >                        pending_requests;
      map< community_name_type, community_invite_api_obj >                         incoming_invites;
      map< community_name_type, community_invite_api_obj >                         outgoing_invites;
      map< community_name_type, int64_t >                                      incoming_moderator_votes;
      map< community_name_type, map<account_name_type, uint16_t > >            outgoing_moderator_votes;
      vector< community_name_type >                                            founded_communities;
      vector< community_name_type >                                            admin_communities;
      vector< community_name_type >                                            moderator_communities;
      vector< community_name_type >                                            member_communities;
   };

   struct connection_state
   {
      map< account_name_type, connection_api_obj >                         connections;
      map< account_name_type, connection_api_obj >                         friends;
      map< account_name_type, connection_api_obj >                         companions;
      map< account_name_type, connection_request_api_obj >                 incoming_requests;
      map< account_name_type, connection_request_api_obj >                 outgoing_requests;
   };

   struct business_account_state
   {
      account_business_api_obj                                             business;
      vector< account_name_type >                                          member_businesses;
      vector< account_name_type >                                          officer_businesses;
      vector< account_name_type >                                          executive_businesses;
      map< account_name_type, account_request_api_obj >                    incoming_requests;
      map< account_name_type, account_invite_api_obj >                     incoming_invites;
      map< account_name_type, account_request_api_obj >                    outgoing_requests;
      map< account_name_type, account_invite_api_obj >                     outgoing_invites;
   };

   struct network_state
   {
      producer_api_obj                                                     producer;
      network_officer_api_obj                                              network_officer;
      executive_board_api_obj                                              executive_board;
      interface_api_obj                                                    interface;
      supernode_api_obj                                                    supernode;
      governance_account_api_obj                                           governance_account;
      vector< community_enterprise_api_obj >                               enterprise_proposals;

      map< account_name_type, uint16_t >                                   producer_votes;
      map< string, map< account_name_type, uint16_t > >                    network_officer_votes;
      map< account_name_type, uint16_t >                                   executive_board_votes;
      map< account_name_type, map< string, pair< account_name_type, uint16_t > > >   account_executive_votes;
      map< account_name_type, map< account_name_type, uint16_t > >         account_officer_votes;
      map< account_name_type, map< string, uint16_t > >                    enterprise_approvals;
   };

   struct discussion : public comment_api_obj 
   {
      discussion( const comment_object& o ):comment_api_obj(o){}
      discussion(){}

      string                        url;                 // /category/@rootauthor/root_permlink#author/permlink
      string                        root_title;
      vector< vote_state >          active_votes;
      vector< view_state >          active_views;
      vector< share_state >         active_shares;
      vector< moderation_state >    active_mod_tags;
      vector< string >              replies;                    // author/slug mapping
      uint32_t                      body_length = 0;
      blog_api_obj                  blog;                      // Details Injected if using get_discussions_by_blog.
      feed_api_obj                  feed;                      // Details injected if using get_discussions_by_feed.
   };

   struct extended_account : public account_api_obj
   {
      extended_account(){}
      extended_account( const account_object& a, const database& db ):account_api_obj( a, db ){}

      account_following_api_obj                         following;
      connection_state                                  connections;
      balance_state                                     balances;
      order_state                                       orders;
      business_account_state                            business;
      key_state                                         keychain;
      message_state                                     messages;
      transfer_state                                    transfers;
      community_state                                       communities;
      network_state                                     network;
      account_ad_state                                  active_ads; 
      vector< pair< account_name_type, uint32_t > >     top_shared;
      account_permission_api_obj                        permissions;
      vector< pair< tag_name_type, uint32_t > >         tags_usage;
      operation_state                                   operations;
   };

   struct extended_community : public community_api_obj
   {
      extended_community(){}
      extended_community( const community_object& b ):community_api_obj( b ){}

      vector< account_name_type >                       subscribers;                 // List of accounts that subscribe to the posts made in the community.
      vector< account_name_type >                       members;                     // List of accounts that are permitted to post in the community. Can invite and accept on public communities
      vector< account_name_type >                       moderators;                  // Accounts able to filter posts. Can invite and accept on private communities.
      vector< account_name_type >                       administrators;              // Accounts able to add and remove moderators and update community details. Can invite and accept on Exclusive communities. 
      vector< account_name_type >                       blacklist;                   // Accounts that are not able to post in this community, or request to join.
      map< account_name_type, int64_t >                 mod_weight;                  // Map of all moderator voting weights for distributing rewards. 
      int64_t                                           total_mod_weight = 0;        // Total of all moderator weights. 
      map< account_name_type, community_request_api_obj >   requests;
      map< account_name_type, community_invite_api_obj >    invites;
   };

   struct extended_asset : public asset_api_obj
   {
      extended_asset(){}
      extended_asset( const asset_object& a ):asset_api_obj( a ){}

      int64_t                                  total_supply;              // The total outstanding supply of the asset
      int64_t                                  liquid_supply;             // The current liquid supply of the asset
      int64_t                                  staked_supply;             // The current staked supply of the asset
      int64_t                                  reward_supply;             // The current reward supply of the asset
      int64_t                                  savings_supply;            // The current savings supply of the asset
      int64_t                                  delegated_supply;          // The current delegated supply of the asset
      int64_t                                  receiving_supply;          // The current receiving supply supply of the asset, should equal delegated
      int64_t                                  pending_supply;            // The current supply contained in reward funds and active order objects
      int64_t                                  confidential_supply;       // total confidential asset supply
      bitasset_data_api_obj                    bitasset;
      equity_data_api_obj                      equity; 
      credit_data_api_obj                      credit;
      credit_pool_api_obj                      credit_pool;
      map< string, liquidity_pool_api_obj >    liquidity_pools;
   };

   struct message_state
   {
      vector< message_api_obj >                                inbox;
      vector< message_api_obj >                                outbox;
      map< account_name_type, vector< message_api_obj > >      conversations;
   };

   struct balance_state
   {
      map< asset_symbol_type, account_balance_api_obj >       balances;
   };

   struct key_state
   {
      map< account_name_type, encrypted_keypair_type >       connection_keys;
      map< account_name_type, encrypted_keypair_type >       friend_keys;
      map< account_name_type, encrypted_keypair_type >       companion_keys;
      map< community_name_type, encrypted_keypair_type >         community_keys;
      map< account_name_type, encrypted_keypair_type >       business_keys;
   };

   struct market_limit_orders
   {
      vector<limit_order_api_obj>             limit_bids;
      vector<limit_order_api_obj>             limit_asks;
   };

   struct market_margin_orders
   {
      vector<margin_order_api_obj>            margin_bids;
      vector<margin_order_api_obj>            margin_asks;
   };

   struct market_call_orders
   {
      vector<call_order_api_obj>              calls;
      price                                   settlement_price;
   };

   struct market_credit_loans
   {
      vector<credit_loan_api_obj>             loan_bids;
      vector<credit_loan_api_obj>             loan_asks;
   };

   struct market_state
   {
      market_limit_orders                     limit_orders;
      market_margin_orders                    margin_orders;
      market_call_orders                      call_orders;
      vector< liquidity_pool_api_obj >        liquidity_pools;
      vector< credit_pool_api_obj >           credit_pools;
      market_credit_loans                     credit_loans;
   };

   struct account_ad_state
   {
      vector< ad_creative_api_obj >           creatives;
      vector< ad_campaign_api_obj >           campaigns;
      vector< ad_audience_api_obj >           audiences;
      vector< ad_inventory_api_obj >          inventories;
      vector< ad_bid_api_obj >                created_bids;
      vector< ad_bid_api_obj >                account_bids;
      vector< ad_bid_api_obj >                creative_bids;
      vector< ad_bid_state >                  incoming_bids;
   };

   struct search_result_state
   {
      vector< account_api_obj >               accounts;
      vector< community_api_obj >                 communities;
      vector< tag_following_api_obj >         tags;
      vector< asset_api_obj >                 assets;
      vector< discussion >                    posts;
   };

   struct ad_bid_state : public ad_bid_api_obj
   {
      ad_bid_state(){}
      ad_bid_state( const ad_bid_object& a ):ad_bid_api_obj( a ){}

      ad_creative_api_obj                     creative;
      ad_campaign_api_obj                     campaign;
      ad_inventory_api_obj                    inventory;
      ad_audience_api_obj                     audience;
   };

   struct state 
   {
      string                                  current_route;
      dynamic_global_property_api_obj         props;
      app::tag_index                          tag_idx;
      map< string, extended_account >         accounts;
      map< string, extended_community >           communities;
      map< string, tag_following_api_obj >    tags;
      map< string, discussion_index >         discussion_idx;
      map< string, tag_api_obj >              tag_stats;
      map< string, discussion >               content;
      map< string, producer_api_obj >         voting_producers;
      map< string, producer_api_obj >         mining_producers;
      map< string, vector< string > >         blogs;
      map< string, vector< string > >         feeds;
      map< string, vector< string > >         comments;
      map< string, vector< string > >         recent_replies;
      producer_schedule_api_obj               producer_schedule;
      string                                  error;
   };

} }

FC_REFLECT( node::app::discussion_index,
         (category)
         (payout)
         (payout_comments)
         (created)
         (responses)
         (updated)
         (active)
         (cashout)
         (best)
         (net_votes)
         (view_count)
         (share_count)
         (comment_count)
         (vote_power)
         (view_power)
         (share_power)
         (comment_power)
         );

FC_REFLECT( node::app::tag_index,
         (trending) 
         );

FC_REFLECT( node::app::vote_state,
         (voter)
         (weight)
         (reward)
         (percent)
         (time) 
         );

FC_REFLECT( node::app::view_state,
         (voter)
         (weight)
         (reward)
         (time) 
         );

FC_REFLECT( node::app::share_state,
         (voter)
         (weight)
         (reward)
         (time) 
         );

FC_REFLECT( node::app::moderation_state,
         (moderator)
         (tags)
         (rating)
         (details)
         (filter)
         (time) 
         );

FC_REFLECT( node::app::account_vote,
         (author)
         (permlink)
         (weight)
         (reward)
         (percent)
         (time)
         );

FC_REFLECT( node::app::account_view,
         (author)
         (permlink)
         (weight)
         (reward)
         (time)
         );

FC_REFLECT( node::app::account_share,
         (author)
         (permlink)
         (weight)
         (reward)
         (time)
         );

FC_REFLECT( node::app::account_moderation,
         (author)
         (permlink)
         (tags)
         (rating)
         (details)
         (filter)
         (time)
         );

FC_REFLECT( node::app::order_state,
         (limit_orders)
         (margin_orders)
         (call_orders)
         (loan_orders)
         (collateral)
         );

FC_REFLECT( node::app::operation_state,
         (account_history)
         (connection_history)
         (follow_history)
         (activity_history)
         (post_history)
         (message_history)
         (vote_history)
         (view_history)
         (share_history)
         (moderation_history)
         (community_history)
         (ad_history)
         (transfer_history)
         (balance_history)
         (escrow_history)
         (trading_history)
         (liquidity_history)
         (credit_history)
         (asset_history)
         (network_history)
         (other_history)
         );

FC_REFLECT( node::app::transfer_state,
         (incoming_requests)
         (outgoing_requests)
         (incoming_recurring_transfers)
         (outgoing_recurring_transfers)
         (incoming_recurring_transfer_requests)
         (outgoing_recurring_transfer_requests)
         );

FC_REFLECT( node::app::community_state,
         (pending_requests)
         (incoming_invites)
         (outgoing_invites)
         (incoming_moderator_votes)
         (outgoing_moderator_votes)
         (founded_communities)
         (admin_communities)
         (moderator_communities)
         (member_communities)
         );

FC_REFLECT( node::app::connection_state,
         (connections)
         (friends)
         (companions)
         (incoming_requests)
         (outgoing_requests)
         );

FC_REFLECT( node::app::business_account_state,
         (business)
         (member_businesses)
         (officer_businesses)
         (executive_businesses)
         (incoming_requests)
         (incoming_invites)
         (outgoing_requests)
         (outgoing_invites)
         );

FC_REFLECT( node::app::network_state,
         (producer)
         (network_officer)
         (executive_board)
         (interface)
         (supernode)
         (governance_account)
         (enterprise_proposals)
         (producer_votes)
         (network_officer_votes)
         (executive_board_votes)
         (account_executive_votes)
         (account_officer_votes)
         (enterprise_approvals)
         );

FC_REFLECT_DERIVED( node::app::discussion, (node::app::comment_api_obj), 
         (url)
         (root_title)
         (active_votes)
         (active_views)
         (active_shares)
         (active_mod_tags)
         (replies)
         (body_length)
         (blog)
         (feed)
         );

FC_REFLECT_DERIVED( node::app::extended_account, ( node::app::account_api_obj ),
         (following)
         (connections)
         (balances)
         (orders)
         (business)
         (keychain)
         (messages)
         (transfers)
         (communities)
         (network)
         (active_ads)
         (top_shared)
         (permissions)
         (tags_usage)
         (operations)
         );

FC_REFLECT_DERIVED( node::app::extended_community, ( node::app::community_api_obj ),
         (subscribers)
         (members)
         (moderators)
         (administrators)
         (blacklist)
         (mod_weight)
         (total_mod_weight)
         (requests)
         (invites)
         );

FC_REFLECT_DERIVED( node::app::extended_asset, ( node::app::asset_api_obj ),
         (total_supply)
         (liquid_supply)
         (staked_supply)
         (reward_supply)
         (savings_supply)
         (delegated_supply)
         (receiving_supply)
         (pending_supply)
         (confidential_supply)
         (bitasset)
         (equity)
         (credit)
         (credit_pool)
         (liquidity_pools)
         );

FC_REFLECT( node::app::message_state,
         (inbox)
         (outbox)
         (conversations)
         );

FC_REFLECT( node::app::key_state,
         (connection_keys)
         (friend_keys)
         (companion_keys)
         (community_keys)
         (business_keys)
         );

FC_REFLECT( node::app::market_limit_orders,
         (limit_bids)
         (limit_asks)
         );

FC_REFLECT( node::app::market_margin_orders,
         (margin_bids)
         (margin_asks)
         );

FC_REFLECT( node::app::market_call_orders,
         (calls)
         (settlement_price)
         );

FC_REFLECT( node::app::market_credit_loans,
         (loan_bids)
         (loan_asks)
         );

FC_REFLECT( node::app::market_state,
         (limit_orders)
         (margin_orders)
         (call_orders)
         (liquidity_pools)
         (credit_pools)
         (credit_loans)
         );

FC_REFLECT( node::app::account_ad_state,
         (creatives)
         (campaigns)
         (audiences)
         (inventories)
         (created_bids)
         (account_bids)
         (creative_bids)
         (incoming_bids)
         );

FC_REFLECT( node::app::search_result_state,
         (accounts)
         (communities)
         (tags)
         (assets)
         (posts)
         );

FC_REFLECT_DERIVED( node::app::ad_bid_state, ( node::app::ad_bid_api_obj ),
         (creative)
         (campaign)
         (inventory)
         (audience)
         );

FC_REFLECT( node::app::state, 
         (current_route)
         (props)
         (tag_idx)
         (accounts)
         (communities)
         (tags)
         (discussion_idx)
         (tag_stats)
         (content)
         (voting_producers)
         (mining_producers)
         (blogs)
         (feeds)
         (comments)
         (recent_replies)
         (producer_schedule)
         (error)
         );