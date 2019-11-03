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
      string           category;    /// category by which everything is filtered
      vector< string > trending;    /// trending posts over the last 24 hours
      vector< string > payout;      /// pending posts by payout
      vector< string > payout_comments; /// pending comments by payout
      vector< string > trending30;  /// pending lifetime payout
      vector< string > created;     /// creation date
      vector< string > responses;   /// creation date
      vector< string > updated;     /// creation date
      vector< string > active;      /// last update or reply
      vector< string > votes;       /// last update or reply
      vector< string > cashout;     /// last update or reply
      vector< string > maturing;    /// about to be paid out
      vector< string > best;        /// total lifetime payout
      vector< string > hot;         /// total lifetime payout
   };

   struct tag_index
   {
      vector< string > trending; /// pending payouts
   };

   struct vote_state
   {
      string         voter;
      uint64_t       weight = 0;
      int64_t        reward = 0;
      int16_t        percent = 0;
      time_point     time;
   };

   struct view_state
   {
      string         viewer;
      uint64_t       weight = 0;
      int64_t        reward = 0;
      time_point     time;
   };

   struct share_state
   {
      string         sharer;
      uint64_t       weight = 0;
      int64_t        reward = 0;
      time_point     time;
   };

   struct moderation_state
   {
      string                     moderator;
      vector< tag_name_type >    tags;             // Set of additional string tags for sorting the post by
      string                     rating;           // Moderator updated rating as to the maturity of the content, and display sensitivity. 
      string                     details;          // Explanation as to what rule the post is in contravention of and why it was tagged.
      bool                       filter;           // True if the post should be filtered by the board or governance address subscribers.
      time_point                 time;
   };

   struct account_vote
   {
      string         authorperm;
      uint64_t       weight = 0;
      int64_t        reward = 0;
      int16_t        percent = 0;
      time_point     time;
   };

   struct order_state
   {
      vector< limit_order_api_obj> limit_orders;
      vector< margin_order_api_obj> margin_orders;
      vector< call_order_api_obj> call_orders;
      vector< credit_loan_api_obj> loan_orders;
      vector< credit_collateral_api_obj> collateral;
   };

   struct discussion : public comment_api_obj 
   {
      discussion( const comment_object& o ):comment_api_obj(o){}
      discussion(){}

      string                      url;                            // /category/@rootauthor/root_permlink#author/permlink
      string                      root_title;
      vector<vote_state>          active_votes;
      vector<view_state>          active_views;
      vector<share_state>         active_shares;
      vector<moderation_state>    active_mod_tags;
      vector<string>              replies;                        // author/slug mapping
      uint32_t                    body_length = 0;
   };

   struct extended_account : public account_api_obj
   {
      extended_account(){}
      extended_account( const account_object& a, const database& db ):account_api_obj( a, db ){}

      map<uint64_t,applied_operation>         transfer_history;        // transfer to/from
      map<uint64_t,applied_operation>         market_history;          // limit order / cancel / fill
      map<uint64_t,applied_operation>         post_history;
      map<uint64_t,applied_operation>         vote_history;
      map<uint64_t,applied_operation>         other_history;

      map<string, account_balance_api_obj >   balances;
      account_following_api_obj               following; 
      account_business_api_obj                business; 
      key_state                               keychain; // todo
      message_state                           messages;

      map<account_name_type, connection_api_obj>      connections;
      map<account_name_type, connection_api_obj>      friends;
      map<account_name_type, connection_api_obj>      companions;

      map<account_name_type, uint16_t>                                    witness_votes;
      map<string, map< account_name_type, uint16_t > >                    network_officer_votes;
      map<account_name_type, uint16_t >                                   executive_board_votes;

      map<account_name_type, map< string, pair< account_name_type, uint16_t > > >   account_executive_votes;
      map<account_name_type, map< account_name_type, uint16_t > >        account_officer_votes;
      map<board_name_type, map<account_name_type, uint16_t > >           board_moderator_votes;
      map<account_name_type, map< string, uint16_t > >                   enterprise_approvals;

      vector<pair<string,uint32_t>>            tags_usage;
      vector<pair<account_name_type,uint32_t>> guest_bloggers;

      optional<map<uint32_t,extended_limit_order>> open_orders;
      optional<vector<string>>                comments; /// permlinks for this user
      optional<vector<string>>                blog; /// blog posts for this user
      optional<vector<string>>                feed; /// feed posts for this user
      optional<vector<string>>                recent_replies; /// blog posts for this user
      optional<vector<string>>                recommended; /// posts recommened for this user
   };


   struct extended_board : public board_api_obj
   {
      extended_board(){}
      extended_board( const board_object& b, const database& db ):board_api_obj( b, db ){}

      vector<account_name_type>             subscribers;                 // List of accounts that subscribe to the posts made in the board.
      vector<account_name_type>             members;                     // List of accounts that are permitted to post in the board. Can invite and accept on public boards
      vector<account_name_type>             moderators;                  // Accounts able to filter posts. Can invite and accept on private boards.
      vector<account_name_type>             administrators;              // Accounts able to add and remove moderators and update board details. Can invite and accept on Exclusive boards. 
      vector<account_name_type>             blacklist;                   // Accounts that are not able to post in this board, or request to join.
      map<account_name_type,int64_t>        mod_weight;                  // Map of all moderator voting weights for distributing rewards. 
      int64_t                               total_mod_weight = 0;        // Total of all moderator weights. 
   };


   struct extended_asset : public asset_api_obj
   {
      extended_asset(){}
      extended_asset( const asset_object& a, const database& db ):asset_api_obj( a, db ){}

      int64_t                                  total_supply;              // The total outstanding supply of the asset
      int64_t                                  liquid_supply;             // The current liquid supply of the asset
      int64_t                                  staked_supply;             // The current staked supply of the asset
      int64_t                                  reward_supply;             // The current reward supply of the asset
      int64_t                                  savings_supply;            // The current savings supply of the asset
      int64_t                                  delegated_supply;          // The current delegated supply of the asset
      int64_t                                  receiving_supply;          // The current receiving supply supply of the asset, should equal delegated
      int64_t                                  pending_supply;            // The current supply contained in reward funds and active order objects
      int64_t                                  confidential_supply;       // total confidential asset supply
      int64_t                                  accumulated_fees;          // Amount of Fees that have accumulated to be paid to the asset issuer. Denominated in this asset.
      int64_t                                  fee_pool;                  // Amount of core asset available to pay fees. Denominated in the core asset.

      optional<bitasset_data_api_obj>          bitasset;
      optional<equity_data_api_obj>            equity; 
      optional<credit_data_api_obj>            credit;
      optional<credit_pool_api_obj>            credit_pool;
      map< string, liquidity_pool_api_obj >    liquidity_pools;

   }

   struct message_state
   {
      vector< message_api_obj >                                inbox;
      vector< message_api_obj >                                outbox;
      map< account_name_type, vector< message_api_obj > >      conversations;
   };

   struct balance_state
   {
      map<string, account_balance_api_obj >                    balances;
   }


   struct key_state
   {
      map<account_name_type, string>                           board_keys;
      map<account_name_type, string>                           business_keys;
      map<account_name_type, string>                           connection_keys;
      map<account_name_type, string>                           friend_keys;
      map<account_name_type, string>                           companion_keys;
   };



   struct candle_stick 
   {
      time_point      open_time;
      uint32_t        period = 0;
      double          high = 0;
      double          low = 0;
      double          open = 0;
      double          close = 0;
      double          buy_asset_volume = 0;
      double          sell_asset_volume = 0;
   };

   struct order_history_item 
   {
      time_point     time;
      string         type; // buy or sell
      asset          buy_asset_quantity;
      asset          sell_asset_quantity;
      double         real_price = 0;
   };

   struct market 
   {
      vector<extended_limit_order> bids;
      vector<extended_limit_order> asks;
      vector<order_history_item>   history;
      vector<int>                  available_candlesticks;
      vector<int>                  available_zoom;
      int                          current_candlestick = 0;
      int                          current_zoom = 0;
      vector<candle_stick>         price_history;
   };

   /**
    *  This struct is designed
    */
   struct state 
   {
        string                            current_route;

        dynamic_global_property_api_obj   props;

        app::tag_index                    tag_idx;

        /**
         * "" is the global discussion index
         */
        map<string, discussion_index>     discussion_idx;

        map< string, tag_api_obj >        tags;

        /**
         *  map from account/slug to full nested discussion
         */
        map< string, discussion >         content;
        map< string, extended_account >   accounts;

        /**
         * The list of miners who are queued to produce work
         */
        vector< account_name_type >       pow_queue;
        map< string, witness_api_obj >    witnesses;
        witness_schedule_api_obj          witness_schedule;
        price                             feed_price;
        string                            error;
        optional< market >                market_data;
   };

} }

FC_REFLECT_DERIVED( node::app::extended_account,
                   (node::app::account_api_obj),
                   (reputation)
                   (transfer_history)
                   (market_history)
                   (post_history)
                   (vote_history)
                   (other_history)
                   (witness_votes)
                   (tags_usage)
                   (guest_bloggers)
                   (open_orders)
                   (comments)
                   (feed)
                   (blog)
                   (recent_replies)
                   (recommended) 
                   )

FC_REFLECT( node::app::vote_state, 
            (voter)
            (weight)
            (reward)
            (percent)
            (reputation)
            (time) 
            );

FC_REFLECT( node::app::account_vote, 
            (authorperm)
            (weight)
            (reward)
            (percent)
            (time) 
            );

FC_REFLECT( node::app::discussion_index, 
         (category)
         (trending)
         (payout)
         (payout_comments)
         (trending30)
         (updated)
         (created)
         (responses)
         (active)
         (votes)
         (maturing)
         (best)
         (hot)
         (promoted)
         (cashout) 
         );

FC_REFLECT( node::app::tag_index, 
         (trending) 
         );

FC_REFLECT_DERIVED( node::app::discussion, (node::app::comment_api_obj), 
         (url)
         (root_title)
         (pending_payout_value)
         (total_pending_payout_value)
         (active_votes)
         (replies)
         (author_reputation)
         (promoted)
         (body_length)
         (reblogged_by)
         (first_reblogged_by)
         (first_reblogged_on) 
         )

FC_REFLECT( node::app::state, 
         (current_route)
         (props)
         (tag_idx)
         (tags)
         (content)
         (accounts)
         (pow_queue)
         (witnesses)
         (discussion_idx)
         (witness_schedule)
         (feed_price)
         (error)
         (market_data) 
         )

FC_REFLECT_DERIVED( node::app::extended_limit_order, (node::app::limit_order_api_obj), 
         (real_price)
         (rewarded) 
         )

FC_REFLECT( node::app::order_history_item, 
         (time)
         (type)
         (buy_asset_quantity)
         (sell_asset_quantity)
         (real_price) 
         );

FC_REFLECT( node::app::market, 
         (bids)
         (asks)
         (history)
         (price_history)
         (available_candlesticks)
         (available_zoom)
         (current_candlestick)
         (current_zoom) 
         );

FC_REFLECT( node::app::candle_stick, 
         (open_time)
         (period)
         (high)
         (low)
         (open)
         (close)
         (buy_asset_volume)
         (sell_asset_volume) 
         );
