#pragma once
#include <node/chain/account_object.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/comment_object.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/chain/witness_objects.hpp>

#include <node/tags/tags_plugin.hpp>

#include <node/witness/witness_objects.hpp>

namespace node { namespace app {

using namespace node::chain;

typedef chain::account_balance_object                  account_balance_api_obj;
typedef chain::change_recovery_account_request_object  change_recovery_account_request_api_obj;
typedef chain::block_summary_object                    block_summary_api_obj;
typedef chain::comment_vote_object                     comment_vote_api_obj;
typedef chain::comment_view_object                     comment_view_api_obj;
typedef chain::comment_share_object                    comment_share_api_obj;
typedef chain::comment_share_object                    comment_share_api_obj;
typedef chain::escrow_object                           escrow_api_obj;
typedef chain::limit_order_object                      limit_order_api_obj;
typedef chain::unstake_asset_route_object              unstake_asset_route_api_obj;
typedef chain::decline_voting_rights_request_object    decline_voting_rights_request_api_obj;
typedef chain::witness_vote_object                     witness_vote_api_obj;
typedef chain::witness_schedule_object                 witness_schedule_api_obj;
typedef chain::asset_delegation_object                 asset_delegation_api_obj;
typedef chain::asset_delegation_expiration_object      asset_delegation_expiration_api_obj;
typedef chain::reward_fund_object                      reward_fund_api_obj;
typedef witness::account_bandwidth_object              account_bandwidth_api_obj;

struct comment_api_obj
{
   comment_api_obj( const chain::comment_object& o ):
      id( o.id ),
      author( o.author ),
      permlink( to_string( o.permlink ) ),
      post_type( to_string( o.post_type ) ),
      privacy( o.privacy ),
      public_key( to_string( o.public_key ) ),
      reach( to_string( o.reach ) ),
      board( o.board ),
      for( auto tag: o.tags )
      {
         tags.push_back( tag );
      }
      body( to_string( o.body ) ),
      ipfs( to_string( o.ipfs ) ),
      magnet( to_string( o.magnet ) ),
      interface( o.interface ),
      rating( to_string( o.rating ) ),
      language( to_string( o.language ) ),
      root_comment( o.root_comment ),
      parent_author( o.parent_author ),
      parent_permlink( to_string( o.parent_permlink ) ),
      json( to_string( o.json ) ),    
      category( to_string( o.category ) ),
      comment_price( o.comment_price ),
      
      for( auto pr: o.payments_received )
      {
         payments_received.push_back( std::make_pair(pr.first, pr.second.second) );
      }
      
      for( auto& route : o.beneficiaries )
      {
         beneficiaries.push_back( route );
      }
      
      last_update( o.last_update ),
      created( o.created ),
      active( o.active ),
      last_payout( o.last_payout ),
      author_reputation( o.author_reputation ),
      depth( o.depth ),
      children( o.children ),
      net_votes( o.net_votes ),
      view_count( o.view_count ),
      share_count( o.share_count ),
      net_reward( o.net_reward ),
      vote_power( o.vote_power ),
      view_power( o.view_power ),
      share_power( o.share_power ),
      comment_power( o.comment_power ),
      cashout_time( o.cashout_time ),
      cashouts_received( o.cashouts_received ),
      max_cashout_time( o.max_cashout_time ),
      total_vote_weight( o.total_vote_weight ),
      total_view_weight( o.total_view_weight ),
      total_share_weight( o.total_share_weight ),
      total_comment_weight( o.total_comment_weight ),
      total_payout_value( o.total_payout_value ),
      curator_payout_value( o.curator_payout_value ),
      beneficiary_payout_value( o.beneficiary_payout_value ),
      author_rewards( o.author_rewards ),
      percent_liquid( o.percent_liquid ),
      reward( o.reward ),
      weight( o.weight ),
      max_weight( o.max_weight ),
      max_accepted_payout( o.max_accepted_payout ),
      author_reward_percent( o.author_reward_percent ),
      vote_reward_percent( o.vote_reward_percent ),
      share_reward_percent( o.share_reward_percent ),
      comment_reward_percent( o.comment_reward_percent ),
      storage_reward_percent( o.storage_reward_percent ),
      moderator_reward_percent( o.moderator_reward_percent ),
      allow_replies( o.allow_replies ),
      allow_votes( o.allow_votes ),
      allow_views( o.allow_views ),
      allow_shares( o.allow_shares ),
      allow_curation_rewards( o.allow_curation_rewards ),
      root( o.root ),
      deleted( o.deleted ),
      
   comment_api_obj(){}

   comment_id_type                id;
   account_name_type              author;                       // Name of the account that created the post.
   string                         permlink;                     // Unique identifing string for the post.
   string                         post_type;                    // The type of post that is being created, image, text, article, video etc. 
   bool                           privacy;                      // True if the post is encrypted. False if it is plaintext.
   string                         public_key;                   // The public key used to encrypt the post, holders of the private key may decrypt. 
   string                         reach;                        // The reach of the post across followers, connections, friends and companions
   board_name_type                board;                        // The name of the board to which the post is uploaded to. Null string if no board. 
   vector< tag_name_type >        tags;                         // Set of string tags for sorting the post by.
   string                         body;                         // String containing text for display when the post is opened.
   string                         ipfs;                         // String containing a display image or video file as an IPFS file hash.
   string                         magnet;                       // String containing a bittorrent magnet link to a file swarm.
   account_name_type              interface;                    // Name of the interface account that was used to broadcast the transaction and view the post.
   string                         rating;                       // User nominated rating as to the maturity of the content, and display sensitivity. 
   string                         language;                     // String containing a two letter language code that the post is broadcast in.
   comment_id_type                root_comment;                 // The root post that the comment is an ancestor of. 
   account_name_type              parent_author;                // Account that created the post this post is replying to, empty if root post. 
   string                         parent_permlink;              // permlink of the post this post is replying to, empty if root post. 
   string                         json;                         // IPFS link to file containing - Json metadata of the Title, Link, and additional interface specific data relating to the post.
   string                         category;
   asset                          comment_price;                // The price paid to create a comment
   vector< pair< account_name_type, asset > >  payments_received;    // Map of all transfers received that referenced this comment. 
   vector< beneficiary_route_type > beneficiaries;
   time_point                     last_update;                  // The time the comment was last edited by the author
   time_point                     created;                      // Time that the comment was created.
   time_point                     active;                       // The last time this post was replied to.
   time_point                     last_payout;                  // The last time that the post recieved a content reward payout
   share_type                     author_reputation;            // Used to measure author lifetime rewards, relative to other accounts.
   uint16_t                       depth = 0;                    // used to track max nested depth
   uint32_t                       children = 0;                 // The total number of children, grandchildren, posts with this as root comment.
   int32_t                        net_votes = 0;                // The amount of upvotes, minus downvotes on the post.
   uint32_t                       view_count = 0;               // The amount of views on the post.
   uint32_t                       share_count = 0;              // The amount of shares on the post.
   int128_t                       net_reward = 0;               // Net reward is the sum of all vote, view, share and comment power.
   int128_t                       vote_power = 0;               // Sum of weighted voting power from votes.
   int128_t                       view_power = 0;               // Sum of weighted voting power from viewers.
   int128_t                       share_power = 0;              // Sum of weighted voting power from shares.
   int128_t                       comment_power = 0;            // Sum of weighted voting power from comments.
   time_point                     cashout_time;                 // 24 hours from the weighted average of vote time
   uint32_t                       cashouts_received = 0;        // Number of times that the comment has received content rewards
   time_point                     max_cashout_time;
   uint128_t                      total_vote_weight = 0;        // the total weight of votes, used to calculate pro-rata share of curation payouts
   uint128_t                      total_view_weight = 0;        // the total weight of views, used to calculate pro-rata share of curation payouts
   uint128_t                      total_share_weight = 0;       // the total weight of shares, used to calculate pro-rata share of curation payouts
   uint128_t                      total_comment_weight = 0;     // the total weight of comments, used to calculate pro-rata share of curation payouts
   asset                          total_payout_value = asset(0, SYMBOL_USD); // The total payout this comment has received over time, measured in USD */
   asset                          curator_payout_value = asset(0, SYMBOL_USD);
   asset                          beneficiary_payout_value = asset( 0, SYMBOL_USD );
   share_type                     author_rewards = 0;
   share_type                     percent_liquid = PERCENT_100;
   uint128_t                      reward = 0;                   // The amount of reward_curve this comment is responsible for in its root post.
   uint128_t                      weight = 0;                   // Used to define the comment curation reward this comment receives.
   uint128_t                      max_weight = 0;               // Used to define relative contribution of this comment to rewards.
   asset                          max_accepted_payout = asset( BILLION * BLOCKCHAIN_PRECISION, SYMBOL_USD );       // USD value of the maximum payout this post will receive
   uint32_t                       author_reward_percent = AUTHOR_REWARD_PERCENT;
   uint32_t                       vote_reward_percent = VOTE_REWARD_PERCENT;
   uint32_t                       view_reward_percent = VIEW_REWARD_PERCENT;
   uint32_t                       share_reward_percent = SHARE_REWARD_PERCENT;
   uint32_t                       comment_reward_percent = COMMENT_REWARD_PERCENT;
   uint32_t                       storage_reward_percent = STORAGE_REWARD_PERCENT;
   uint32_t                       moderator_reward_percent = MODERATOR_REWARD_PERCENT;
   bool                           allow_replies = true;               // allows a post to recieve replies.
   bool                           allow_votes = true;                 // allows a post to receive votes.
   bool                           allow_views = true;                 // allows a post to receive views.
   bool                           allow_shares = true;                // allows a post to receive shares.
   bool                           allow_curation_rewards = true;      // Allows a post to distribute curation rewards.
   bool                           root = true;                        // True if post is a root post. 
   bool                           deleted = false;                    // True if author selects to remove from display in all interfaces, removed from API node distribution, cannot be interacted with.
};

struct tag_api_obj
{
   tag_api_obj( const tags::tag_stats_object& o ) :
      tag( o.tag ),
      total_payouts(o.total_payout),
      net_votes(o.net_votes),
      top_posts(o.top_posts),
      comments(o.comments),
      trending(o.total_trending) {}

   tag_api_obj() {}

   string               tag;
   asset                total_payouts;
   int32_t              net_votes = 0;
   uint32_t             top_posts = 0;
   uint32_t             comments = 0;
   fc::uint128          trending = 0;
};

struct account_api_obj
{
   account_api_obj( const chain::account_object& a, const chain::database& db ) :
      id( a.id ),
      name( a.name ),
      details( to_string( a.details ) ),
      json( to_string( a.json ) ),
      json_private( to_string( a.json_private ) ),
      url( to_string( a.url ) ),
      account_type( to_string( a.account_type ) ),
      membership( to_string( a.membership ) ),
      secure_public_key( to_string( a.secure_public_key ) ),
      connection_public_key( to_string( a.connection_public_key ) ),
      friend_public_key( to_string( a.friend_public_key ) ),
      companion_public_key( to_string( a.companion_public_key ) ),
      pinned_comment( a.pinned_comment ),
      proxy( a.proxy ),
      for( auto name : a.proxied )
      {
         proxied.push_back( name );
      }
      registrar( a.registrar ),
      referrer( a.referrer ),
      recovery_account( a.recovery_account ),
      reset_account( a.reset_account ),
      membership_interface( a.membership_interface ),
      reset_account_delay_days( a.reset_account_delay_days ),
      referrer_rewards_percentage( a.referrer_rewards_percentage ),
      comment_count( a.comment_count ),
      follower_count( a.follower_count ),
      following_count( a.following_count ),
      lifetime_vote_count( a.lifetime_vote_count ),
      post_count( a.post_count ),
      voting_power( a.voting_power ),
      viewing_power( a.viewing_power ),
      sharing_power( a.sharing_power ),
      commenting_power( a.commenting_power ),
      savings_withdraw_requests( a.savings_withdraw_requests ),
      withdraw_routes( a.withdraw_routes ),
      posting_rewards( a.posting_rewards ),
      curation_rewards( a.curation_rewards ),
      moderation_rewards( a.moderation_rewards ),
      total_rewards( a.total_rewards ),
      author_reputation( a.author_reputation ),
      loan_default_balance( a.loan_default_balance ),
      recent_activity_claims( a.recent_activity_claims),
      witness_vote_count( a.witness_vote_count ),
      officer_vote_count( a.officer_vote_count ),
      executive_board_vote_count( a.executive_board_vote_count ),
      governance_subscriptions( a.governance_subscriptions ),
      recurring_membership( a.recurring_membership ),
      created( a.created ),
      membership_expiration( a.membership_expiration ),
      last_account_update( a.last_account_update ),
      last_vote_time( a.last_vote_time ),
      last_view_time( a.last_view_time ),
      last_share_time( a.last_share_time ),
      last_post( a.last_post ),
      last_root_post( a.last_root_post ),
      last_transfer_time( a.last_transfer_time ),
      last_activity_reward( a.last_activity_reward ),
      last_account_recovery( a.last_account_recovery ),
      last_board_created( a.last_board_created ),
      last_asset_created( a.last_asset_created ),
      mined( a.mined ),
      revenue_share( a.revenue_share ),
      can_vote( a.can_vote ),
   {
      const auto& auth = db.get< account_authority_object, by_account >( name );
      owner = authority( auth.owner );
      active = authority( auth.active );
      posting = authority( auth.posting );
      last_owner_update = auth.last_owner_update;

      if( db.has_index< witness::account_bandwidth_index >() )
      {
         auto forum_bandwidth = db.find< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( name, witness::bandwidth_type::forum ) );

         if( forum_bandwidth != nullptr )
         {
            average_bandwidth = forum_bandwidth->average_bandwidth;
            lifetime_bandwidth = forum_bandwidth->lifetime_bandwidth;
            last_bandwidth_update = forum_bandwidth->last_bandwidth_update;
         }

         auto market_bandwidth = db.find< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( name, witness::bandwidth_type::market ) );

         if( market_bandwidth != nullptr )
         {
            average_market_bandwidth = market_bandwidth->average_bandwidth;
            lifetime_market_bandwidth = market_bandwidth->lifetime_bandwidth;
            last_market_bandwidth_update = market_bandwidth->last_bandwidth_update;
         }
      }
   }

   account_api_obj(){}

   account_id_type                  id;
   account_name_type                name;                                  // Username of the account, lowercase letter and numbers and hyphens only.
   string                           details;                               // User's account details.
   string                           json;                                  // Public plaintext json information.
   string                           json_private;                          // Private ciphertext json information.
   string                           url;                                   // Account's external reference URL.
   string                           account_type;                          // Type of account, persona, profile or business.
   string                           membership;                            // Level of account membership.
   string                           secure_public_key;                     // Key used for receiving incoming encrypted direct messages and key exchanges.
   string                           connection_public_key;                 // Key used for encrypting posts for connection level visibility. 
   string                           friend_public_key;                     // Key used for encrypting posts for friend level visibility.
   string                           companion_public_key;                  // Key used for encrypting posts for companion level visibility.
   authority                        owner;
   authority                        active;
   authority                        posting;
   comment_id_type                  pinned_comment;                        // Post pinned to the top of the account's profile. 
   account_name_type                proxy;                                 // Account that votes on behalf of this account
   vector< account_name_type>       proxied;                               // Accounts that have set this account to be their proxy voter.
   account_name_type                registrar;                             // The name of the account that created the account;
   account_name_type                referrer;                              // The name of the account that originally referred the account to be created;
   account_name_type                recovery_account = NULL_ACCOUNT;       // Account that can request recovery using a recent owner key if compromised.  
   account_name_type                reset_account = NULL_ACCOUNT;          // Account that has the ability to reset owner authority after specified days of inactivity.
   account_name_type                membership_interface = NULL_ACCOUNT;   // Account of the last interface to sell a membership to the account.
   uint16_t                         reset_account_delay_days = 7;
   uint16_t                         referrer_rewards_percentage = 50 * PERCENT_1; // The percentage of registrar rewards that are directed to the referrer.
   uint32_t                         comment_count = 0;
   uint32_t                         follower_count = 0;
   uint32_t                         following_count = 0;
   uint32_t                         lifetime_vote_count = 0;
   uint32_t                         post_count = 0;
   uint16_t                         voting_power = PERCENT_100;               // current voting power of this account, falls after every vote, recovers over time.
   uint16_t                         viewing_power = PERCENT_100;              // current viewing power of this account, falls after every view, recovers over time.
   uint8_t                          savings_withdraw_requests = 0;
   uint16_t                         withdraw_routes = 0;
   share_type                       posting_rewards = 0;                      // Rewards in core asset earned from author rewards.
   share_type                       curation_rewards = 0;                     // Rewards in core asset earned from voting, shares, views, and commenting
   share_type                       moderation_rewards = 0;                   // Rewards in core asset from moderation rewards. 
   share_type                       total_rewards = 0;                        // Rewards in core asset earned from all reward sources.
   share_type                       author_reputation = 0;                    // 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards
   asset                            loan_default_balance = asset(0, SYMBOL_CREDIT);
   share_type                       recent_activity_claims = 0;
   uint16_t                         witness_vote_count = 0;
   uint16_t                         officer_vote_count = 0;                         // Number of network officers that the account has voted for.
   uint16_t                         executive_board_vote_count = 0;                 // Number of Executive boards that the account has voted for.
   uint16_t                         governance_subscriptions = 0;              // Number of governance accounts that the account subscribes to.   
   uint16_t                         recurring_membership = 0;                  // Amount of months membership should be automatically renewed for on expiration
   time_point                       created;                                   // Time that the account was created.
   time_point                       membership_expiration;                     // Time that the account has its current membership subscription until.
   time_point                       last_account_update;                       // Time that the account's details were last updated.
   time_point                       last_vote_time;                            // Time that the account last voted on a comment.
   time_point                       last_view_time;                            // Time that the account last viewed a post.
   time_point                       last_share_time;                           // Time that the account last viewed a post.
   time_point                       last_post;                                 // Time that the user most recently created a comment 
   time_point                       last_root_post;                            // Time that the account last created a post.
   time_point                       last_transfer_time;                        // Time that the account last sent a transfer or created a trading txn. 
   time_point                       last_activity_reward;
   time_point                       last_account_recovery;
   time_point                       last_board_created;
   time_point                       last_asset_created;
   bool                             mined = true;
   bool                             revenue_share = false;
   bool                             can_vote = true;
};


struct account_concise_api_obj
{
   account_concise_api_obj( const chain::account_object& a, const chain::database& db ) :
      id( a.id ),
      name( a.name ),
      details( to_string( a.details ) ),
      json( to_string( a.json ) ),
      json_private( to_string( a.json_private ) ),
      url( to_string( a.url ) ),
      account_type( to_string( a.account_type ) ),
      membership( to_string( a.membership ) ),
      secure_public_key( to_string( a.secure_public_key ) ),
      connection_public_key( to_string( a.connection_public_key ) ),
      friend_public_key( to_string( a.friend_public_key ) ),
      companion_public_key( to_string( a.companion_public_key ) ),
      pinned_comment( a.pinned_comment ),
      follower_count( a.follower_count ),
      following_count( a.following_count ),
      total_rewards( a.total_rewards ),
      author_reputation( a.author_reputation ),
      created( a.created ),
      
   account_concise_api_obj(){}

   account_id_type                  id;
   account_name_type                name;                                  // Username of the account, lowercase letter and numbers and hyphens only.
   string                           details;                               // User's account details.
   string                           json;                                  // Public plaintext json information.
   string                           json_private;                          // Private ciphertext json information.
   string                           url;                                   // Account's external reference URL.
   string                           account_type;                          // Type of account, persona, profile or business.
   string                           membership;                            // Level of account membership.
   string                           secure_public_key;                     // Key used for receiving incoming encrypted direct messages and key exchanges.
   string                           connection_public_key;                 // Key used for encrypting posts for connection level visibility. 
   string                           friend_public_key;                     // Key used for encrypting posts for friend level visibility.
   string                           companion_public_key;                  // Key used for encrypting posts for companion level visibility.
   comment_id_type                  pinned_comment;                        // Post pinned to the top of the account's profile. 
   uint32_t                         follower_count = 0;
   uint32_t                         following_count = 0;
   share_type                       total_rewards = 0;                        // Rewards in core asset earned from all reward sources.
   share_type                       author_reputation = 0;                    // 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards
   time_point                       created;                                   // Time that the account was created.
};


struct account_business_api_obj
{
   account_business_api_obj( const chain::account_business_object& a, const chain::database& db ) :
      id( a.id ),
      account( a.account ),
      business_type( to_string( a.business_type ) ),
      executive_board( a.executive_board ),
      for( auto name : a.executives )
      {
         executives.push_back( std::make_pair( name.first, std::make_pair( name.second.first, name.second.second ) ) );
      }
      for( auto name : a.officers )
      {
         officers.push_back( std::make_pair(name.first, name.second.first ) );
      }
      for( auto name : a.members )
      {
         members.push_back( name );
      }
      officer_vote_threshold( a.officer_vote_threshold ),
      for( auto symbol : a.equity_assets )
      {
         equity_assets.push_back( symbol );
      }
      for( auto symbol : a.credit_assets )
      {
         credit_assets.push_back( symbol );
      }
      for( auto symbol : a.equity_revenue_shares )
      {
         equity_revenue_shares.push_back( std::make_pair( symbol.first, symbol.second );
      }
      for( auto symbol : a.credit_revenue_shares )
      {
         credit_revenue_shares.push_back( std::make_pair( symbol.first, symbol.second ) );
      }

   account_business_api_obj(){}

   account_business_id_type                   id;
   account_name_type                          account;                    // Username of the business account, lowercase letters only.
   business_types                             business_type;              // Type of business account, controls authorizations for transactions of different types.
   executive_officer_set                      executive_board;            // Set of highest voted executive accounts for each role.
   vector<pair<account_name_type,pair<executive_types,share_type>>>   executives;   // Set of all executive names.    
   vector<pair<account_name_type,share_type>> officers;                   // Set of all officers in the business, and their supporting voting power.
   vector<account_name_type>                  members;                    // Set of all members of the business.
   share_type                                 officer_vote_threshold;     // Amount of voting power required for an officer to be active. 
   vector<asset_symbol_type>                  equity_assets;              // Set of equity assets that offer dividends and voting power over the business account's structure
   vector<asset_symbol_type>                  credit_assets;              // Set of credit assets that offer interest and buybacks from the business account
   vector<pair<asset_symbol_type,uint16_t>>   equity_revenue_shares;      // Holds a map of all equity assets that the account shares incoming revenue with, and percentages.
   vector<pair<asset_symbol_type,uint16_t>>   credit_revenue_shares;      // Holds a map of all equity assets that the account shares incoming revenue with, and percentages.
}


struct account_following_api_obj
{
   account_following_api_obj( const chain::account_following_object& a, const chain::database& db ) :
      id( a.id ),
      account( a.account ),
      for( auto name : a.followers )
      {
         followers.push_back( name );
      }
      for( auto name : a.following )
      {
         following.push_back( name );
      }
      for( auto name : a.mutual_followers )
      {
         mutual_followers.push_back( name );
      }
      for( auto name : a.connections )
      {
         connections.push_back( name );
      }
      for( auto name : a.friends )
      {
         friends.push_back( name );
      }
      for( auto name : a.companions )
      {
         companions.push_back( name );
      }
      for( auto board : a.followed_boards )
      {
         followed_boards.push_back( board );
      }
      for( auto tag : a.followed_tags )
      {
         followed_tags.push_back( tags );
      }
      for( auto name : a.filtered )
      {
         filtered.push_back( name );
      }
      for( auto name : a.filtered_boards )
      {
         filtered_boards.push_back( name );
      }
      for( auto name : a.filtered_tags )
      {
         filtered_tags.push_back( name );
      }
      last_update( a.last_update ),

   account_following_api_obj(){}

   account_following_id_type       id;
   account_name_type               account;              // Name of the account.
   vector< account_name_type >     followers;            // Accounts that follow this account.
   vector< account_name_type >     following;            // Accounts that this account follows.
   vector< account_name_type >     mutual_followers;     // Accounts that are both following and followers of this account.
   vector< account_name_type >     connections;          // Accounts that are connections of this account.
   vector< account_name_type >     friends;              // Accounts that are friends of this account.
   vector< account_name_type >     companions;           // Accounts that are companions of this account. 
   vector< board_name_type >       followed_boards;      // Boards that the account subscribes to. 
   vector< tag_name_type >         followed_tags;        // Tags that the account follows. 
   vector< account_name_type >     filtered;             // Accounts that this account has filtered. Interfaces should not show posts by these users.
   vector< board_name_type >       filtered_boards;      // Boards that this account has filtered. Posts will not display if they are in these boards.
   vector< tag_name_type >         filtered_tags;        // Tags that this account has filtered. Posts will not display if they have any of these tags. 
   time_point                      last_update;          // Last time that the account changed its following sets.
}

struct message_api_obj
{
   message_api_obj( const chain::message_api_obj& m, const chain::database& db ) :
      id( m.id ),
      sender( m.sender ),
      recipient( m.recipient ),
      sender_public_key( to_string( m.sender_public_key ) ),
      recipient_public_key( to_string ( m.recipient_public_key ) ),
      message( to_string( m.message ) ),
      json( to_string( m.json ) ),
      uuid( to_string( m.uuid ) ),
      created( m.created ),
      last_updated( m.last_updated ),

   message_api_obj(){}

   message_id_type         id;
   account_name_type       sender;                   // Name of the message sender.
   account_name_type       recipient;                // Name of the intended message recipient.
   string                  sender_public_key;        // Public secure key of the sender.
   string                  recipient_public_key;     // Public secure key of the recipient.
   string                  message;                  // Encrypted private message ciphertext.
   string                  json;                     // Encrypted Message metadata.
   string                  uuid;                     // uuidv4 uniquely identifying the message for local storage.
   time_point              created;                  // Time the message was sent.
   time_point              last_updated;             // Time the message was last changed, used to reload encrypted message storage.
}


struct connection_api_obj
{
   connection_api_obj( const chain::connection_object& c, const chain::database& db ):
      id( c.id ),
      account_a( c.account_a ),
      encrypted_key_a( c.encrypted_key_a ),
      account_b( c.account_b ),
      encrypted_key_b( c.encrypted_key_b ),
      connection_type( to_string( c.connection_type ) ),
      connection_id( to_string( c.connection_id ) ),
      connection_strength( c.connection_strength ),
      consecutive_days( c.consecutive_days ),
      last_message_time_a( c.last_message_time_a ),
      last_message_time_b( c.last_message_time_b ),
      created( c.created ),

   connection_api_obj(){}

   id_type                id;                 
   account_name_type      account_a;                // Account with the lower ID.
   string                 encrypted_key_a;          // A's private connection key, encrypted with the public secure key of account B.
   account_name_type      account_b;                // Account with the greater ID.
   string                 encrypted_key_b;          // B's private connection key, encrypted with the public secure key of account A.
   string                 connection_type;          // The connection level shared in this object
   string                 connection_id;            // Unique uuidv4 for the connection, for local storage of decryption key.
   uint32_t               connection_strength = 0;  // Number of total messages sent between connections
   uint32_t               consecutive_days = 0;     // Number of consecutive days that the connected accounts have both sent a message.
   time_point             last_message_time_a;      // Time since the account A last sent a message
   time_point             last_message_time_b;      // Time since the account B last sent a message
   time_point             created;                  // Time the connection was created. 
}


struct board_api_obj
{
   board_api_obj( const chain::board_object& b, const chain::database& db ):
      id( b.id ),
      name( b.name ),
      founder( b.founder ),
      board_type( to_string( b.board_type ) ),
      board_privacy( to_string( b.board_privacy ) ),
      board_public_key( to_string( b.board_public_key ) ),
      json( to_string( b.json ) ),
      json_private( to_string( b.json_private ) ),
      pinned_comment( b.pinned_comment ),
      subscriber_count( b.subscriber_count ),
      post_count( b.post_count ),
      comment_count( b.comment_count ),
      vote_count( b.vote_count ),
      view_count( b.view_count ),
      share_count( b.share_count ),
      total_content_rewards( b.total_content_rewards ),
      created( b.created ),
      last_board_update( b.last_board_update ),
      last_post( b.last_post ),
      last_root_post( b.last_root_post ),

      board_api_obj(){}

   board_id_type                            id;
   board_name_type                    name;                               // Name of the board, lowercase letters, numbers and hyphens only.
   account_name_type                  founder;                            // The account that created the board, able to add and remove administrators.
   board_types                        board_type;                         // Type of board, persona, profile or business.
   board_privacy_types                board_privacy;                      // Board privacy level, open, public, private, or exclusive
   public_key_type                    board_public_key;                   // Key used for encrypting and decrypting posts. Private key shared with accepted members.
   shared_string                      json;                               // Public plaintext json information about the board, its topic and rules.
   shared_string                      json_private;                       // Private ciphertext json information about the board.
   comment_id_type                    pinned_comment;                     // Post pinned to the top of the board's page. 
   uint32_t                           subscriber_count = 0;               // number of accounts that are subscribed to the board
   uint32_t                           post_count = 0;                     // number of posts created in the board
   uint32_t                           comment_count = 0;                  // number of comments on posts in the board
   uint32_t                           vote_count = 0;                     // accumulated number of votes received by all posts in the board
   uint32_t                           view_count = 0;                     // accumulated number of views on posts in the board 
   uint32_t                           share_count = 0;                    // accumulated number of shares on posts in the board 
   asset                              total_content_rewards = asset(0, SYMBOL_COIN);   // total amount of rewards earned by posts in the board
   time_point                         created;                            // Time that the board was created.
   time_point                         last_board_update;                  // Time that the board's details were last updated.
   time_point                         last_post;                          // Time that the user most recently created a comment.
   time_point                         last_root_post;                     // Time that the board last created a post. 
}


struct moderation_tag_api_obj
{
   moderation_tag_api_obj( const chain::moderation_tag_object& m, const chain::database& db ) :
      id( m.id ),
      moderator( m.moderator ),
      comment( m.comment ),
      board( m.board ),
      for( auto tag : m.tags )
      {
         tags.push_back( tag );
      }
      rating( to_string( m.rating ) ),
      details( to_string( m.details ) ),
      filter( m.filter ),
      last_update( m.last_update ),
      created( m.created ),

   moderation_tag_api_obj(){}

   moderation_tag_id_type         id;
   account_name_type              moderator;        // Name of the moderator or goverance account that created the comment tag.
   comment_id_type                comment;          // ID of the comment.
   board_name_type                board;            // The name of the board to which the post is uploaded to.
   vector< tag_name_type >        tags;             // Set of string tags for sorting the post by
   string                         rating;           // Moderator updated rating as to the maturity of the content, and display sensitivity. 
   string                         details;          // Explaination as to what rule the post is in contravention of and why it was tagged.
   bool                           filter;           // True if the post should be filtered by the board or governance address subscribers. 
   time_point                     last_update;      // Time the comment tag was last edited by the author.
   time_point                     created;          // Time that the comment tag was created.
}

struct limit_order
{
   limit_order( chain::limit_order_object& o ):
      id( o.id ),
      created( o.created ),
      expiration( o.expiration ),
      seller( o.seller ),
      order_id( to_string( o.order_id ) ),
      for_sale( o.for_sale ),
      sell_price( o.sell_price )
   {}

   limit_order(){}

   limit_order_id_type            id;
   time_point                     created;
   time_point                     expiration;
   account_name_type              seller;
   string                         order_id;
   share_type                     for_sale;
   price                          sell_price;
};


struct owner_authority_history_api_obj
{
   owner_authority_history_api_obj( const chain::owner_authority_history_object& o ) :
      id( o.id ),
      account( o.account ),
      previous_owner_authority( authority( o.previous_owner_authority ) ),
      last_valid_time( o.last_valid_time )
   {}

   owner_authority_history_api_obj() {}

   owner_authority_history_id_type      id;
   account_name_type                    account;
   authority                            previous_owner_authority;
   time_point                           last_valid_time;
};


struct account_recovery_request_api_obj
{
   account_recovery_request_api_obj( const chain::account_recovery_request_object& o ) :
      id( o.id ),
      account_to_recover( o.account_to_recover ),
      new_owner_authority( authority( o.new_owner_authority ) ),
      expires( o.expires )
   {}

   account_recovery_request_api_obj() {}

   account_recovery_request_id_type     id;
   account_name_type                    account_to_recover;
   authority                            new_owner_authority;
   time_point                           expires;
};


struct savings_withdraw_api_obj
{
   savings_withdraw_api_obj( const chain::savings_withdraw_object& o ) :
      id( o.id ),
      from( o.from ),
      to( o.to ),
      memo( to_string( o.memo ) ),
      request_id( o.request_id ),
      amount( o.amount ),
      complete( o.complete )
   {}

   savings_withdraw_api_obj() {}

   savings_withdraw_id_type   id;
   account_name_type          from;
   account_name_type          to;
   string                     memo;
   uint32_t                   request_id = 0;
   asset                      amount;
   time_point                 complete;
};

struct witness_api_obj
{
   witness_api_obj( const chain::witness_object& w ) :
      id( w.id ),
      owner( w.owner ),
      created( w.created ),
      url( to_string( w.url ) ),
      total_missed( w.total_missed ),
      last_aslot( w.last_aslot ),
      last_confirmed_block_num( w.last_confirmed_block_num ),
      pow_worker( w.pow_worker ),
      signing_key( w.signing_key ),
      props( w.props ),
      USD_exchange_rate( w.USD_exchange_rate ),
      last_USD_exchange_update( w.last_USD_exchange_update ),
      votes( w.votes ),
      virtual_last_update( w.virtual_last_update ),
      virtual_position( w.virtual_position ),
      virtual_scheduled_time( w.virtual_scheduled_time ),
      last_work( w.last_work ),
      running_version( w.running_version ),
      hardfork_version_vote( w.hardfork_version_vote ),
      hardfork_time_vote( w.hardfork_time_vote ),

   witness_api_obj() {}

   witness_id_type      id;
   account_name_type    owner;
   time_point           created;
   string               url;
   uint32_t             total_missed = 0;
   uint64_t             last_aslot = 0;
   uint64_t             last_confirmed_block_num = 0;
   uint64_t             pow_worker = 0;
   public_key_type      signing_key;
   chain_properties     props;
   price                USD_exchange_rate;
   time_point           last_USD_exchange_update;
   share_type           votes;
   fc::uint128          virtual_last_update;
   fc::uint128          virtual_position;
   fc::uint128          virtual_scheduled_time;
   digest_type          last_work;
   version              running_version;
   hardfork_version     hardfork_version_vote;
   time_point           hardfork_time_vote;
};

struct signed_block_api_obj : public signed_block
{
   signed_block_api_obj( const signed_block& block ) : signed_block( block )
   {
      block_id = id();
      signing_key = signee();
      transaction_ids.reserve( transactions.size() );
      for( const signed_transaction& tx : transactions )
         transaction_ids.push_back( tx.id() );
   }
   signed_block_api_obj() {}

   block_id_type                 block_id;
   public_key_type               signing_key;
   vector< transaction_id_type > transaction_ids;
};

struct dynamic_global_property_api_obj : public dynamic_global_property_object
{
   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo, const chain::database& db ) :
      dynamic_global_property_object( gpo )
   {
      if( db.has_index< witness::reserve_ratio_index >() )
      {
         const auto& r = db.find( witness::reserve_ratio_id_type() );

         if( BOOST_LIKELY( r != nullptr ) )
         {
            current_reserve_ratio = r->current_reserve_ratio;
            average_block_size = r->average_block_size;
            max_virtual_bandwidth = r->max_virtual_bandwidth;
         }
      }
   }

   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo ) :
      dynamic_global_property_object( gpo ) {}

   dynamic_global_property_api_obj() {}

   uint32_t    current_reserve_ratio = 0;
   uint64_t    average_block_size = 0;
   uint128_t   max_virtual_bandwidth = 0;
};

} } // node::app

FC_REFLECT( node::app::comment_api_obj,
         (id)
         (author)
         (permlink)
         (category)
         (parent_author)
         (parent_permlink)
         (title)
         (body)
         (json)
         (last_update)
         (created)
         (active)
         (last_payout)
         (depth)
         (children)
         (net_reward)
         (cashout_time)
         (max_cashout_time)
         (total_vote_weight)
         (total_view_weight)
         (total_share_weight)
         (total_comment_weight)
         (total_payout_value)
         (curator_payout_value)
         (author_rewards)
         (net_votes)
         (root_comment)
         (max_accepted_payout)
         (percent_liquid)
         (allow_replies)
         (allow_votes)
         (allow_curation_rewards)
         (beneficiaries)
         );

FC_REFLECT( node::app::account_api_obj,
         (id)
         (name)
         (owner)
         (active)
         (posting)
         (secure_public_key)
         (json)
         (proxy)
         (last_owner_update)
         (last_account_update)
         (created)
         (mined)
         (recovery_account)
         (last_account_recovery)
         (reset_account)
         (comment_count)
         (lifetime_vote_count)
         (post_count)
         (can_vote)
         (voting_power)
         (last_vote_time)
         (savings_withdraw_requests)
         (withdraw_routes)
         (curation_rewards)
         (posting_rewards)
         (proxied_voting_power)
         (witness_vote_count)
         (average_bandwidth)
         (lifetime_bandwidth)
         (last_bandwidth_update)
         (average_market_bandwidth)
         (lifetime_market_bandwidth)
         (last_market_bandwidth_update)
         (last_post)
         (last_root_post)
         );

FC_REFLECT( node::app::owner_authority_history_api_obj,
         (id)
         (account)
         (previous_owner_authority)
         (last_valid_time)
         );

FC_REFLECT( node::app::account_recovery_request_api_obj,
         (id)
         (account_to_recover)
         (new_owner_authority)
         (expires)
         );

FC_REFLECT( node::app::savings_withdraw_api_obj,
         (id)
         (from)
         (to)
         (memo)
         (request_id)
         (amount)
         (complete)
         );

FC_REFLECT( node::app::feed_history_api_obj,
         (id)
         (current_median_history)
         (price_history)
         );

FC_REFLECT( node::app::tag_api_obj,
         (name)
         (total_payouts)
         (net_votes)
         (top_posts)
         (comments)
         (trending)
         );

FC_REFLECT( node::app::witness_api_obj,
         (id)
         (owner)
         (created)
         (url)(votes)
         (virtual_last_update)
         (virtual_position)
         (virtual_scheduled_time)
         (total_missed)
         (last_aslot)
         (last_confirmed_block_num)
         (pow_worker)
         (signing_key)
         (props)
         (USD_exchange_rate)
         (last_USD_exchange_update)
         (last_work)
         (running_version)
         (hardfork_version_vote)
         (hardfork_time_vote)
         );

FC_REFLECT_DERIVED( node::app::signed_block_api_obj, (node::protocol::signed_block),
         (block_id)
         (signing_key)
         (transaction_ids)
         );

FC_REFLECT_DERIVED( node::app::dynamic_global_property_api_obj, (node::chain::dynamic_global_property_object),
         (current_reserve_ratio)
         (average_block_size)
         (max_virtual_bandwidth)
         );
