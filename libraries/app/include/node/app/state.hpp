#pragma once
#include <node/app/applied_operation.hpp>
#include <node/app/node_api_objects.hpp>
#include <node/chain/global_property_object.hpp>
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
      int64_t        reward = 0;
      int16_t        percent = 0;
      time_point     time;
   };

   struct view_state
   {
      string         viewer;
      uint128_t      weight = 0;
      int64_t        reward = 0;
      time_point     time;
   };

   struct share_state
   {
      string         sharer;
      uint128_t      weight = 0;
      int64_t        reward = 0;
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

   struct comment_interaction_state
   {
      vector< vote_state >            votes;
      vector< view_state >            views;
      vector< share_state >           shares;
      vector< moderation_state >      moderation;
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

   struct account_interaction_state
   {
      vector< account_vote >            votes;
      vector< account_view >            views;
      vector< account_share >           shares;
      vector< account_moderation >      moderation;
   };

   struct order_state
   {
      vector< limit_order_api_obj >             limit_orders;
      vector< margin_order_api_obj >            margin_orders;
      vector< call_order_api_obj >              call_orders;
      vector< option_order_api_obj >            option_orders;
      vector< credit_loan_api_obj >             loan_orders;
      vector< credit_collateral_api_obj >       collateral;
   };


   struct withdraw_route
   {
      string               from;
      string               to;
      uint16_t             percent;
      bool                 auto_stake;
   };

   enum withdraw_route_type
   {
      incoming,
      outgoing,
      all
   };

   struct balance_state
   {
      map< asset_symbol_type, account_balance_api_obj >                     balances;
      vector< confidential_balance_api_obj >                                confidential_balances;
      map< asset_symbol_type, distribution_balance_api_obj >                distribution_balances;
      map< asset_symbol_type, prediction_pool_resolution_api_obj >          prediction_resolutions;

      vector< withdraw_route >                                              withdraw_routes;
      vector< savings_withdraw_api_obj >                                    savings_withdrawals_from;
      vector< savings_withdraw_api_obj >                                    savings_withdrawals_to;
      vector< asset_delegation_api_obj >                                    delegations_from;
      vector< asset_delegation_api_obj >                                    delegations_to;
      vector< asset_delegation_expiration_api_obj >                         expirations_from;
      vector< asset_delegation_expiration_api_obj >                         expirations_to;
   };

   struct key_state
   {
      map< account_name_type, encrypted_keypair_type >       connection_keys;
      map< account_name_type, encrypted_keypair_type >       friend_keys;
      map< account_name_type, encrypted_keypair_type >       companion_keys;
      map< community_name_type, encrypted_keypair_type >     community_keys;
      map< account_name_type, encrypted_keypair_type >       business_keys;
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
      map< uint64_t, applied_operation >    graph_history;
      map< uint64_t, applied_operation >    transfer_history;
      map< uint64_t, applied_operation >    balance_history;
      map< uint64_t, applied_operation >    product_history;
      map< uint64_t, applied_operation >    escrow_history;
      map< uint64_t, applied_operation >    trading_history;
      map< uint64_t, applied_operation >    liquidity_history;
      map< uint64_t, applied_operation >    credit_history;
      map< uint64_t, applied_operation >    option_history;
      map< uint64_t, applied_operation >    prediction_history;
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
      map< community_name_type, community_request_api_obj >                    pending_requests;
      map< community_name_type, community_invite_api_obj >                     incoming_invites;
      map< community_name_type, community_invite_api_obj >                     outgoing_invites;
      map< community_name_type, int64_t >                                      incoming_moderator_votes;
      map< community_name_type, map<account_name_type, uint16_t > >            outgoing_moderator_votes;
      vector< community_name_type >                                            founded_communities;
      vector< community_name_type >                                            admin_communities;
      vector< community_name_type >                                            moderator_communities;
      vector< community_name_type >                                            member_communities;
   };

   struct connection_state
   {
      map< account_name_type, account_connection_api_obj >                         connections;
      map< account_name_type, account_connection_api_obj >                         friends;
      map< account_name_type, account_connection_api_obj >                         companions;
      map< account_name_type, account_connection_request_api_obj >                 incoming_requests;
      map< account_name_type, account_connection_request_api_obj >                 outgoing_requests;
      map< account_name_type, account_verification_api_obj >               incoming_verifications;
      map< account_name_type, account_verification_api_obj >               outgoing_verifications;
   };

   struct business_account_state : public account_business_api_obj
   {
      business_account_state(){}
      business_account_state( const account_business_object& a ):account_business_api_obj( a ){}

      map< account_name_type, account_request_api_obj >                                          incoming_requests;
      map< account_name_type, account_invite_api_obj >                                           incoming_invites;
      map< account_name_type, map< account_name_type, account_executive_vote_api_obj > >         incoming_executive_votes;
      map< account_name_type, map< account_name_type, account_officer_vote_api_obj > >           incoming_officer_votes;

      map< account_name_type, account_request_api_obj >                                          outgoing_requests;
      map< account_name_type, account_invite_api_obj >                                           outgoing_invites;
      map< account_name_type, map< account_name_type, account_executive_vote_api_obj > >         outgoing_executive_votes;
      map< account_name_type, map< account_name_type, account_officer_vote_api_obj > >           outgoing_officer_votes;
   };


   struct account_network_state
   {
      producer_api_obj                                                     producer;
      network_officer_api_obj                                              network_officer;
      executive_board_api_obj                                              executive_board;
      interface_api_obj                                                    interface;
      supernode_api_obj                                                    supernode;
      governance_account_api_obj                                           governance_account;
      vector< enterprise_api_obj >                               enterprise_proposals;
      vector< block_validation_api_obj >                                   block_validations;

      map< account_name_type, producer_vote_api_obj >                      incoming_producer_votes;
      map< account_name_type, network_officer_vote_api_obj >               incoming_network_officer_votes;
      map< account_name_type, executive_board_vote_api_obj >               incoming_executive_board_votes;
      map< account_name_type, governance_subscription_api_obj >            incoming_governance_subscriptions;
      map< account_name_type, map< string, enterprise_vote_api_obj > > incoming_enterprise_votes;
      map< account_name_type, commit_violation_api_obj >                   incoming_commit_violations;

      map< account_name_type, producer_vote_api_obj >                      outgoing_producer_votes;
      map< account_name_type, network_officer_vote_api_obj >               outgoing_network_officer_votes;
      map< account_name_type, executive_board_vote_api_obj >               outgoing_executive_board_votes;
      map< account_name_type, governance_subscription_api_obj >            outgoing_governance_subscriptions;
      map< account_name_type, map< string, enterprise_vote_api_obj > > outgoing_enterprise_votes;
      map< account_name_type, commit_violation_api_obj >                   outgoing_commit_violations;
   };

   struct discussion : public comment_api_obj 
   {
      discussion( const comment_object& o ):comment_api_obj(o){}
      discussion(){}

      string                        url;                       // /category/@rootauthor/root_permlink#author/permlink
      string                        root_title;
      vector< vote_state >          active_votes;
      vector< view_state >          active_views;
      vector< share_state >         active_shares;
      vector< moderation_state >    active_mod_tags;
      vector< string >              replies;                   // author/slug mapping.
      uint32_t                      body_length = 0;
      uint16_t                      median_rating = 1;         // The median of all moderation tag ratings.
      blog_api_obj                  blog;                      // Details Injected if using get_discussions_by_blog.
      feed_api_obj                  feed;                      // Details injected if using get_discussions_by_feed.
   };


   struct list_state
   {
      account_name_type                         creator;             ///< Name of the account that created the list.
      string                                    list_id;             ///< uuidv4 referring to the list.
      string                                    name;                ///< Name of the list, unique for each account.
      vector< account_api_obj >                 accounts;            ///< Account within the list.
      vector< comment_api_obj >                 comments;            ///< Comment within the list.
      vector< community_api_obj >               communities;         ///< Community within the list.
      vector< asset_api_obj >                   assets;              ///< Asset within the list.
      vector< product_sale_api_obj >            products;            ///< Product within the list.
      vector< product_auction_sale_api_obj >    auctions;            ///< Auction within the list.
      vector< graph_node_api_obj >              nodes;               ///< Graph node within the list.
      vector< graph_edge_api_obj >              edges;               ///< Graph edge within the list.
      vector< graph_node_property_api_obj >     node_types;          ///< Graph node property within the list.
      vector< graph_edge_property_api_obj >     edge_types;          ///< Graph edge property within the list.
   };


   struct account_list_state
   {
      vector< list_state >                        lists;
   };


   struct poll_state : public poll_api_obj
   {
      poll_state( const poll_object& a ):poll_api_obj( a ){}
      poll_state(){}

      map< fixed_string_32, uint32_t >        vote_count;
      vector< poll_vote_api_obj >             votes;
   };


   struct account_poll_state
   {
      vector< poll_state >                    polls;
   };


   struct ad_bid_state : public ad_bid_api_obj
   {
      ad_bid_state( const ad_bid_object& a ):ad_bid_api_obj( a ){}
      ad_bid_state(){}

      ad_creative_api_obj                     creative;
      ad_campaign_api_obj                     campaign;
      ad_inventory_api_obj                    inventory;
      ad_audience_api_obj                     audience;
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

   struct account_product_state
   {
      vector< product_sale_api_obj >            seller_products;
      vector< product_purchase_api_obj >        seller_orders;
      vector< product_auction_sale_api_obj >    seller_auctions;
      vector< product_auction_bid_api_obj >     seller_bids;

      vector< product_sale_api_obj >            buyer_products;
      vector< product_purchase_api_obj >        buyer_orders;
      vector< product_auction_sale_api_obj >    buyer_auctions;
      vector< product_auction_bid_api_obj >     buyer_bids;
   };

   struct graph_data_state
   {
      vector< graph_node_api_obj >              nodes;
      vector< graph_edge_api_obj >              edges;
   };

   struct message_state
   {
      vector< message_api_obj >                                inbox;
      vector< message_api_obj >                                outbox;
      map< account_name_type, vector< message_api_obj > >      conversations;
   };

   struct extended_account : public account_api_obj
   {
      extended_account( const account_object& a, const database& db ):account_api_obj( a, db ){}
      extended_account(){}

      account_following_api_obj                         following;
      connection_state                                  connections;
      business_account_state                            business;
      balance_state                                     balances;
      order_state                                       orders;
      key_state                                         keychain;
      message_state                                     messages;
      transfer_state                                    transfers;
      community_state                                   communities;
      account_network_state                             network;
      account_ad_state                                  ads;
      account_product_state                             products;
      
      vector< pair< account_name_type, uint32_t > >     top_shared;
      account_permission_api_obj                        permissions;
      vector< pair< tag_name_type, uint32_t > >         tags_usage;
      account_recovery_request_api_obj                  recovery;
      vector< owner_authority_history_api_obj >         owner_history;
      operation_state                                   operations;
   };

   struct extended_community : public community_api_obj
   {
      extended_community( const community_object& b ):community_api_obj( b ){}
      extended_community(){}

      vector< account_name_type >                             subscribers;                 // List of accounts that subscribe to the posts made in the community.
      vector< account_name_type >                             members;                     // List of accounts that are permitted to post in the community. Can invite and accept on public communities
      vector< account_name_type >                             moderators;                  // Accounts able to filter posts. Can invite and accept on private communities.
      vector< account_name_type >                             administrators;              // Accounts able to add and remove moderators and update community details. Can invite and accept on Exclusive communities. 
      vector< account_name_type >                             blacklist;                   // Accounts that are not able to post in this community, or request to join.
      vector< community_name_type >                           public_federations;          ///< Communities that have a Public Federation with this community.
      vector< community_name_type >                           member_federations;          ///< Communities that have a Member Federation with this community.
      vector< community_name_type >                           moderator_federations;       ///< Communities that have a Moderator Federation with this community.
      vector< community_name_type >                           admin_federations;           ///< Communities that have a Admin Federation with this community.
      vector< community_event_api_obj >                       events;                      // Events for the community.
      map< account_name_type, int64_t >                       mod_weight;                  // Map of all moderator voting weights for distributing rewards. 
      int64_t                                                 total_mod_weight;            // Total of all moderator weights.
      map< account_name_type, community_request_api_obj >     requests;                    // Requests to join the community.
      map< account_name_type, community_invite_api_obj >      invites;                     // Invitations to join the community.
   };

   struct extended_asset : public asset_api_obj
   {
      extended_asset( const asset_object& a ):asset_api_obj( a ){}
      extended_asset(){}

      int64_t                                                        total_supply;              // The total outstanding supply of the asset
      int64_t                                                        liquid_supply;             // The current liquid supply of the asset
      int64_t                                                        staked_supply;             // The current staked supply of the asset
      int64_t                                                        reward_supply;             // The current reward supply of the asset
      int64_t                                                        savings_supply;            // The current savings supply of the asset
      int64_t                                                        vesting_supply;            // The Current vesting supply of the asset
      int64_t                                                        delegated_supply;          // The current delegated supply of the asset
      int64_t                                                        receiving_supply;          // The current receiving supply supply of the asset, should equal delegated
      int64_t                                                        pending_supply;            // The current supply contained in reward funds and active order objects
      int64_t                                                        confidential_supply;       // Total confidential asset supply
      
      currency_data_api_obj                                          currency;
      stablecoin_data_api_obj                                        stablecoin;
      equity_data_api_obj                                            equity;
      bond_data_api_obj                                              bond;
      credit_data_api_obj                                            credit;
      stimulus_data_api_obj                                          stimulus;
      unique_data_api_obj                                            unique;
      credit_pool_api_obj                                            credit_pool;
      reward_fund_api_obj                                            reward_fund;

      map< asset_symbol_type, liquidity_pool_api_obj >               liquidity_pools;
      map< asset_symbol_type, option_pool_api_obj >                  option_pools;
      prediction_pool_api_obj                                        prediction;
      map< asset_symbol_type, prediction_pool_resolution_api_obj >   resolutions;
      distribution_api_obj                                           distribution;
      map< account_name_type, distribution_balance_api_obj >         distribution_balances;
   };

   struct market_limit_orders
   {
      vector< limit_order_api_obj >           limit_bids;
      vector< limit_order_api_obj >           limit_asks;
   };

   struct market_margin_orders
   {
      vector< margin_order_api_obj >          margin_bids;
      vector< margin_order_api_obj >          margin_asks;
   };

   struct market_auction_orders
   {
      vector< auction_order_api_obj >         product_auction_bids;
      vector< auction_order_api_obj >         auction_asks;
   };

   struct market_call_orders
   {
      vector< call_order_api_obj >            calls;
      price                                   settlement_price;
   };

   struct market_option_orders
   {
      vector< option_order_api_obj >          option_calls;
      vector< option_order_api_obj >          option_puts;
   };

   struct market_credit_loans
   {
      vector< credit_loan_api_obj >           loan_bids;
      vector< credit_loan_api_obj >           loan_asks;
   };

   struct market_state
   {
      market_limit_orders                     limit_orders;
      market_margin_orders                    margin_orders;
      market_auction_orders                   auction_orders;
      market_call_orders                      call_orders;
      market_option_orders                    option_orders;
      vector< liquidity_pool_api_obj >        liquidity_pools;
      vector< credit_pool_api_obj >           credit_pools;
      vector< option_pool_api_obj >           option_pools;
      market_credit_loans                     credit_loans;
   };

   struct search_result_state
   {
      vector< account_api_obj >               accounts;
      vector< community_api_obj >             communities;
      vector< account_tag_following_api_obj >         tags;
      vector< asset_api_obj >                 assets;
      vector< discussion >                    posts;
   };

   struct state 
   {
      string                                  current_route;
      dynamic_global_property_api_obj         props;
      app::tag_index                          tag_idx;
      map< string, extended_account >         accounts;
      map< string, extended_community >       communities;
      map< string, account_tag_following_api_obj >    tags;
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
         (viewer)
         (weight)
         (reward)
         (time) 
         );

FC_REFLECT( node::app::share_state,
         (sharer)
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

FC_REFLECT( node::app::comment_interaction_state,
         (votes)
         (views)
         (shares)
         (moderation)
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

FC_REFLECT( node::app::account_interaction_state,
         (votes)
         (views)
         (shares)
         (moderation)
         );

FC_REFLECT( node::app::order_state,
         (limit_orders)
         (margin_orders)
         (call_orders)
         (option_orders)
         (loan_orders)
         (collateral)
         );

FC_REFLECT( node::app::withdraw_route,
         (from)
         (to)
         (percent)
         (auto_stake)
         );

FC_REFLECT_ENUM( node::app::withdraw_route_type,
         (incoming)
         (outgoing)
         (all)
         );

FC_REFLECT( node::app::balance_state,
         (balances)
         (confidential_balances)
         (distribution_balances)
         (prediction_resolutions)
         (withdraw_routes)
         (savings_withdrawals_from)
         (savings_withdrawals_to)
         (delegations_from)
         (delegations_to)
         (expirations_from)
         (expirations_to)
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
         (graph_history)
         (transfer_history)
         (balance_history)
         (product_history)
         (escrow_history)
         (trading_history)
         (liquidity_history)
         (credit_history)
         (option_history)
         (prediction_history)
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
         (incoming_verifications)
         (outgoing_verifications)
         );

FC_REFLECT( node::app::business_account_state,
         (incoming_requests)
         (incoming_invites)
         (incoming_executive_votes)
         (incoming_officer_votes)
         (outgoing_requests)
         (outgoing_invites)
         (outgoing_executive_votes)
         (outgoing_officer_votes)
         );

FC_REFLECT( node::app::account_network_state,
         (producer)
         (network_officer)
         (executive_board)
         (interface)
         (supernode)
         (governance_account)
         (enterprise_proposals)
         (block_validations)
         (incoming_producer_votes)
         (incoming_network_officer_votes)
         (incoming_executive_board_votes)
         (incoming_governance_subscriptions)
         (incoming_enterprise_votes)
         (incoming_commit_violations)
         (outgoing_producer_votes)
         (outgoing_network_officer_votes)
         (outgoing_executive_board_votes)
         (outgoing_governance_subscriptions)
         (outgoing_enterprise_votes)
         (outgoing_commit_violations)
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
         (median_rating)
         (blog)
         (feed)
         );

FC_REFLECT_DERIVED( node::app::extended_account, ( node::app::account_api_obj ),
         (following)
         (connections)
         (business)
         (balances)
         (orders)
         (keychain)
         (messages)
         (transfers)
         (communities)
         (network)
         (ads)
         (products)
         (top_shared)
         (permissions)
         (tags_usage)
         (recovery)
         (owner_history)
         (operations)
         );

FC_REFLECT_DERIVED( node::app::extended_community, ( node::app::community_api_obj ),
         (subscribers)
         (members)
         (moderators)
         (administrators)
         (blacklist)
         (public_federations)
         (member_federations)
         (moderator_federations)
         (admin_federations)
         (events)
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
         (vesting_supply)
         (delegated_supply)
         (receiving_supply)
         (pending_supply)
         (confidential_supply)
         (currency)
         (stablecoin)
         (equity)
         (bond)
         (credit)
         (stimulus)
         (unique)
         (credit_pool)
         (reward_fund)
         (liquidity_pools)
         (option_pools)
         (prediction)
         (resolutions)
         (distribution)
         (distribution_balances)
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

FC_REFLECT( node::app::market_auction_orders,
         (product_auction_bids)
         (auction_asks)
         );

FC_REFLECT( node::app::market_call_orders,
         (calls)
         (settlement_price)
         );

FC_REFLECT( node::app::market_option_orders,
         (option_calls)
         (option_puts)
         );

FC_REFLECT( node::app::market_credit_loans,
         (loan_bids)
         (loan_asks)
         );

FC_REFLECT( node::app::market_state,
         (limit_orders)
         (margin_orders)
         (auction_orders)
         (call_orders)
         (option_orders)
         (liquidity_pools)
         (credit_pools)
         (option_pools)
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

FC_REFLECT( node::app::account_product_state,
         (seller_products)
         (seller_orders)
         (seller_auctions)
         (seller_bids)
         (buyer_products)
         (buyer_orders)
         (buyer_auctions)
         (buyer_bids)
         );

FC_REFLECT( node::app::graph_data_state,
         (nodes)
         (edges)
         );

FC_REFLECT( node::app::search_result_state,
         (accounts)
         (communities)
         (tags)
         (assets)
         (posts)
         );

FC_REFLECT( node::app::list_state,
         (creator)
         (list_id)
         (name)
         (accounts)
         (comments)
         (communities)
         (assets)
         (products)
         (auctions)
         (nodes)
         (edges)
         (node_types)
         (edge_types)
         );

FC_REFLECT( node::app::account_list_state,
         (lists)
         );

FC_REFLECT_DERIVED( node::app::poll_state, ( node::app::poll_api_obj ),
         (vote_count)
         (votes)
         );

FC_REFLECT( node::app::account_poll_state,
         (polls)
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