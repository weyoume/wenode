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

typedef chain::change_recovery_account_request_object  change_recovery_account_request_api_obj;
typedef chain::block_summary_object                    block_summary_api_obj;
typedef chain::comment_vote_object                     comment_vote_api_obj;
typedef chain::comment_view_object                     comment_view_api_obj;
typedef chain::comment_share_object                    comment_share_api_obj;
typedef chain::comment_share_object                    comment_share_api_obj;
typedef chain::escrow_object                           escrow_api_obj;
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
      {
         for( auto tag: o.tags )
         {
            tags.push_back( tag );
         }
         for( auto pr: o.payments_received )
         {
            payments_received.push_back( std::make_pair(pr.first, pr.second.second) );
         }
         for( auto& route : o.beneficiaries )
         {
            beneficiaries.push_back( route );
         }
      },
      
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
   int64_t                        author_reputation;            // Used to measure author lifetime rewards, relative to other accounts.
   uint16_t                       depth;                    // used to track max nested depth
   uint32_t                       children;                 // The total number of children, grandchildren, posts with this as root comment.
   int32_t                        net_votes;                // The amount of upvotes, minus downvotes on the post.
   uint32_t                       view_count;               // The amount of views on the post.
   uint32_t                       share_count;              // The amount of shares on the post.
   int128_t                       net_reward;               // Net reward is the sum of all vote, view, share and comment power.
   int128_t                       vote_power;               // Sum of weighted voting power from votes.
   int128_t                       view_power;               // Sum of weighted voting power from viewers.
   int128_t                       share_power;              // Sum of weighted voting power from shares.
   int128_t                       comment_power;            // Sum of weighted voting power from comments.
   time_point                     cashout_time;                 // 24 hours from the weighted average of vote time
   uint32_t                       cashouts_received;        // Number of times that the comment has received content rewards
   time_point                     max_cashout_time;
   uint128_t                      total_vote_weight;        // the total weight of votes, used to calculate pro-rata share of curation payouts
   uint128_t                      total_view_weight;        // the total weight of views, used to calculate pro-rata share of curation payouts
   uint128_t                      total_share_weight;       // the total weight of shares, used to calculate pro-rata share of curation payouts
   uint128_t                      total_comment_weight;     // the total weight of comments, used to calculate pro-rata share of curation payouts
   asset                          total_payout_value; // The total payout this comment has received over time, measured in USD */
   asset                          curator_payout_value;
   asset                          beneficiary_payout_value;
   int64_t                        author_rewards;
   int64_t                        percent_liquid;
   uint128_t                      reward;                   // The amount of reward_curve this comment is responsible for in its root post.
   uint128_t                      weight;                   // Used to define the comment curation reward this comment receives.
   uint128_t                      max_weight;               // Used to define relative contribution of this comment to rewards.
   asset                          max_accepted_payout;       // USD value of the maximum payout this post will receive
   uint32_t                       author_reward_percent;
   uint32_t                       vote_reward_percent;
   uint32_t                       view_reward_percent;
   uint32_t                       share_reward_percent;
   uint32_t                       comment_reward_percent;
   uint32_t                       storage_reward_percent;
   uint32_t                       moderator_reward_percent;
   bool                           allow_replies;               // allows a post to recieve replies.
   bool                           allow_votes;                 // allows a post to receive votes.
   bool                           allow_views;                 // allows a post to receive views.
   bool                           allow_shares;                // allows a post to receive shares.
   bool                           allow_curation_rewards;      // Allows a post to distribute curation rewards.
   bool                           root;                        // True if post is a root post. 
   bool                           deleted;                    // True if author selects to remove from display in all interfaces, removed from API node distribution, cannot be interacted with.
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
   int32_t              net_votes;
   uint32_t             top_posts;
   uint32_t             comments;
   fc::uint128          trending;
};

struct account_api_obj
{
   account_api_obj( const chain::account_object& a, const chain::database& db ):
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
      for( auto name : a.proxied )
      {
         proxied.push_back( name );
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
   account_name_type                recovery_account;       // Account that can request recovery using a recent owner key if compromised.  
   account_name_type                reset_account;          // Account that has the ability to reset owner authority after specified days of inactivity.
   account_name_type                membership_interface;   // Account of the last interface to sell a membership to the account.
   uint16_t                         reset_account_delay_days;
   uint16_t                         referrer_rewards_percentage; // The percentage of registrar rewards that are directed to the referrer.
   uint32_t                         comment_count;
   uint32_t                         follower_count;
   uint32_t                         following_count;
   uint32_t                         lifetime_vote_count;
   uint32_t                         post_count;
   uint16_t                         voting_power;               // current voting power of this account, falls after every vote, recovers over time.
   uint16_t                         viewing_power;              // current viewing power of this account, falls after every view, recovers over time.
   uint16_t                         sharing_power;              // current sharing power of this account, falls after every share, recovers over time.
   uint16_t                         commenting_power;           // current commenting power of this account, falls after every comment, recovers over time.
   uint8_t                          savings_withdraw_requests;
   uint16_t                         withdraw_routes;
   int64_t                          posting_rewards;                      // Rewards in core asset earned from author rewards.
   int64_t                          curation_rewards;                     // Rewards in core asset earned from voting, shares, views, and commenting
   int64_t                          moderation_rewards;                   // Rewards in core asset from moderation rewards. 
   int64_t                          total_rewards;                        // Rewards in core asset earned from all reward sources.
   int64_t                          author_reputation;                    // 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards
   asset                            loan_default_balance;
   int64_t                          recent_activity_claims;
   uint16_t                         witness_vote_count;
   uint16_t                         officer_vote_count;                         // Number of network officers that the account has voted for.
   uint16_t                         executive_board_vote_count;                 // Number of Executive boards that the account has voted for.
   uint16_t                         governance_subscriptions;              // Number of governance accounts that the account subscribes to.   
   uint16_t                         recurring_membership;                  // Amount of months membership should be automatically renewed for on expiration
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
   bool                             mined;
   bool                             revenue_share;
   bool                             can_vote;
};


struct account_concise_api_obj
{
   account_concise_api_obj( const chain::account_object& a, const chain::database& db ):
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
   uint32_t                         follower_count;                        // Number of account followers.
   uint32_t                         following_count;                       // Number of accounts that the account follows. 
   int64_t                          total_rewards;                         // Rewards in core asset earned from all reward sources.
   int64_t                          author_reputation;                     // 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards
   time_point                       created;                               // Time that the account was created.
};


struct account_balance_api_obj
{
   account_balance_api_obj( const chain::account_balance_object& b, const chain::database& db ):
      id( b.id ),
      owner( b.owner ),
      symbol( b.symbol ),
      liquid_balance( b.liquid_balance ),
      staked_balance( b.staked_balance ),
      reward_balance( b.reward_balance ),
      savings_balance( b.savings_balance ),
      delegated_balance( b.delegated_balance ),
      receiving_balance( b.receiving_balance ),
      total_balance( b.total_balance ),
      stake_rate( b.stake_rate ),
      next_stake_time( b.next_stake_time ),
      to_stake( b.to_stake ),
      total_staked( b.total_staked ),
      unstake_rate( b.unstake_rate ),
      next_unstake_time( b.next_unstake_time ),
      to_unstake( b.to_unstake ),
      total_unstaked( b.total_unstaked ),
      last_interest_time( b.last_interest_time ),
      maintenance_flag( b.maintenance_flag ),

   account_balance_api_obj(){}

   account_balance_id_type        id;
   account_name_type              owner;
   asset_symbol_type              symbol;
   int64_t                        liquid_balance;             // Balance that can be freely transferred.
   int64_t                        staked_balance;             // Balance that cannot be transferred, and is vested in the account for a period of time.
   int64_t                        reward_balance;             // Balance that is newly issued from the network.
   int64_t                        savings_balance;            // Balance that cannot be transferred, and must be withdrawn after a delay period. 
   int64_t                        delegated_balance;          // Balance that is delegated to other accounts for voting power.
   int64_t                        receiving_balance;          // Balance that has been delegated to the account by other delegators. 
   int64_t                        total_balance;              // The total of all balances
   int64_t                        stake_rate;                 // Amount of liquid balance that is being staked from the liquid balance to the staked balance.  
   time_point                     next_stake_time;            // time at which the stake rate will be transferred from liquid to staked. 
   int64_t                        to_stake;                   // total amount to stake over the staking period. 
   int64_t                        total_staked;               // total amount that has been staked so far. 
   int64_t                        unstake_rate;               // Amount of staked balance that is being unstaked from the staked balance to the liquid balance.  
   time_point                     next_unstake_time;          // time at which the unstake rate will be transferred from staked to liquid. 
   int64_t                        to_unstake;                 // total amount to unstake over the withdrawal period. 
   int64_t                        total_unstaked;             // total amount that has been unstaked so far. 
   time_point                     last_interest_time;         // Last time that interest was compounded.
   bool                           maintenance_flag;           // Whether need to process this balance object in maintenance interval
}


struct account_business_api_obj
{
   account_business_api_obj( const chain::account_business_object& a, const chain::database& db ):
      id( a.id ),
      account( a.account ),
      business_type( to_string( a.business_type ) ),
      executive_board( a.executive_board ),
      officer_vote_threshold( a.officer_vote_threshold ),
      {
         for( auto name : a.executives )
         {
            executives.push_back( std::make_pair( name.first, std::make_pair( to_string( name.second.first ), name.second.second ) ) );
         }
         for( auto name : a.officers )
         {
            officers.push_back( std::make_pair( name.first, name.second.first ) );
         }
         for( auto name : a.members )
         {
            members.push_back( name );
         }
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
            equity_revenue_shares.push_back( std::make_pair( symbol.first, symbol.second ) );
         }
         for( auto symbol : a.credit_revenue_shares )
         {
            credit_revenue_shares.push_back( std::make_pair( symbol.first, symbol.second ) );
         }
      }

   account_business_api_obj(){}

   account_business_id_type                                        id;
   account_name_type                                               account;                    // Username of the business account, lowercase letters only.
   string                                                          business_type;              // Type of business account, controls authorizations for transactions of different types.
   executive_officer_set                                           executive_board;            // Set of highest voted executive accounts for each role.
   vector< pair< account_name_type, pair< string, int64_t > > >    executives;                 // Set of all executive names.    
   vector< pair< account_name_type, int64_t>>                      officers;                   // Set of all officers in the business, and their supporting voting power.
   vector< account_name_type >                                     members;                    // Set of all members of the business.
   int64_t                                                         officer_vote_threshold;     // Amount of voting power required for an officer to be active. 
   vector<asset_symbol_type >                                      equity_assets;              // Set of equity assets that offer dividends and voting power over the business account's structure
   vector<asset_symbol_type >                                      credit_assets;              // Set of credit assets that offer interest and buybacks from the business account
   vector< pair < asset_symbol_type,uint16_t > >                   equity_revenue_shares;      // Holds a map of all equity assets that the account shares incoming revenue with, and percentages.
   vector< pair < asset_symbol_type,uint16_t > >                   credit_revenue_shares;      // Holds a map of all equity assets that the account shares incoming revenue with, and percentages.
};


struct account_following_api_obj
{
   account_following_api_obj( const chain::account_following_object& a, const chain::database& db ):
      id( a.id ),
      account( a.account ),
      last_update( a.last_update ),
      { 
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
      }

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
};

struct message_api_obj
{
   message_api_obj( const chain::message_object& m, const chain::database& db ):
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
};


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

   connection_id_type           id;                 
   account_name_type            account_a;                // Account with the lower ID.
   encrypted_keypair_type       encrypted_key_a;          // A's private connection key, encrypted with the public secure key of account B.
   account_name_type            account_b;                // Account with the greater ID.
   encrypted_keypair_type       encrypted_key_b;          // B's private connection key, encrypted with the public secure key of account A.
   string                       connection_type;          // The connection level shared in this object
   string                       connection_id;            // Unique uuidv4 for the connection, for local storage of decryption key.
   uint32_t                     connection_strength;  // Number of total messages sent between connections
   uint32_t                     consecutive_days;     // Number of consecutive days that the connected accounts have both sent a message.
   time_point                   last_message_time_a;      // Time since the account A last sent a message
   time_point                   last_message_time_b;      // Time since the account B last sent a message
   time_point                   created;                  // Time the connection was created. 
};


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

   board_id_type                      id;
   board_name_type                    name;                               // Name of the board, lowercase letters, numbers and hyphens only.
   account_name_type                  founder;                            // The account that created the board, able to add and remove administrators.
   string                             board_type;                         // Type of board, persona, profile or business.
   string                             board_privacy;                      // Board privacy level, open, public, private, or exclusive
   string                             board_public_key;                   // Key used for encrypting and decrypting posts. Private key shared with accepted members.
   string                             json;                               // Public plaintext json information about the board, its topic and rules.
   string                             json_private;                       // Private ciphertext json information about the board.
   comment_id_type                    pinned_comment;                     // Post pinned to the top of the board's page. 
   uint32_t                           subscriber_count;               // number of accounts that are subscribed to the board
   uint32_t                           post_count;                     // number of posts created in the board
   uint32_t                           comment_count;                  // number of comments on posts in the board
   uint32_t                           vote_count;                     // accumulated number of votes received by all posts in the board
   uint32_t                           view_count;                     // accumulated number of views on posts in the board 
   uint32_t                           share_count;                    // accumulated number of shares on posts in the board 
   asset                              total_content_rewards = asset(0, SYMBOL_COIN);   // total amount of rewards earned by posts in the board
   time_point                         created;                            // Time that the board was created.
   time_point                         last_board_update;                  // Time that the board's details were last updated.
   time_point                         last_post;                          // Time that the user most recently created a comment.
   time_point                         last_root_post;                     // Time that the board last created a post. 
};


struct moderation_tag_api_obj
{
   moderation_tag_api_obj( const chain::moderation_tag_object& m, const chain::database& db ) :
      id( m.id ),
      moderator( m.moderator ),
      comment( m.comment ),
      board( m.board ),
      rating( to_string( m.rating ) ),
      details( to_string( m.details ) ),
      filter( m.filter ),
      last_update( m.last_update ),
      created( m.created ),
      {
         for( auto tag : m.tags )
         {
            tags.push_back( tag );
         }
      }

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
};

struct asset_api_obj
{
   asset_api_obj( const chain::asset_object& a, const chain::database& db ):
      id( a.id ),
      symbol( a.symbol ),
      asset_type( to_string( a.asset_type ) ),
      issuer( a.issuer ),
      created( a.created ),
      display_symbol( to_string( a.options.display_symbol) ),
      description( to_string( a.options.description) ),
      json( to_string( a.options.json ) ),
      url( to_string( a.options.url ) ),
      max_supply( a.options.max_supply ),
      stake_intervals( a.options.stake_intervals ),
      unstake_intervals( a.options.unstake_intervals ),
      market_fee_percent( a.options.market_fee_percent ),
      market_fee_share_percent( a.options.market_fee_share_percent ),
      issuer_permissions( a.options.issuer_permissions ),
      flags( a.options.flags ),
      core_exchange_rate( a.options.core_exchange_rate ),
      {
         for( auto auth : a.options.whitelist_authorities )
         {
            whitelist_authorities.push_back( auth );
         }
         for( auto auth : a.options.blacklist_authorities )
         {
            blacklist_authorities.push_back( auth );
         }
         for( auto market : a.options.whitelist_markets )
         {
            whitelist_markets.push_back( auth );
         }
         for( auto market : a.options.blacklist_markets )
         {
            blacklist_markets.push_back( auth );
         }
      }

   asset_api_obj(){}

   asset_id_type                   id; 
   asset_symbol_type               symbol;                                // Consensus enforced unique Ticker symbol string for this asset. 
   string                          asset_type;                            // The type of the asset.
   account_name_type               issuer;                                // name of the account which issued this asset.
   time_point                      created;                               // Time that the asset was created. 
   string                          display_symbol;                        // Non-consensus display name for interface reference.
   string                          description;                           // Data that describes the purpose of this asset.
   string                          json;                                  // Additional JSON metadata of this asset.
   string                          url;                                   // Reference URL for the asset. 
   int64_t                         max_supply;                            // The maximum supply of this asset which may exist at any given time. 
   uint8_t                         stake_intervals;                       // Weeks required to stake the asset.
   uint8_t                         unstake_intervals;                     // Weeks require to unstake the asset.
   uint16_t                        market_fee_percent;                    // Percentage of the total traded will be paid to the issuer of the asset.
   uint16_t                        market_fee_share_percent;              // Percentage of the market fee that will be shared with the account's referrers.
   int64_t                         max_market_fee;                        // Market fee charged on a trade is capped to this value.
   uint32_t                        issuer_permissions;                    // The flags which the issuer has permission to update.
   uint32_t                        flags;                                 // The currently active flags on this permission.
   price                           core_exchange_rate;                    // Exchange rate between asset and core for fee pool exchange
   vector<account_name_type>       whitelist_authorities;                 // Accounts able to transfer this asset if the flag is set and whitelist is non-empty.
   vector<account_name_type>       blacklist_authorities;                 // Accounts which cannot transfer or recieve this asset.
   vector<asset_symbol_type>       whitelist_markets;                     // The assets that this asset may be traded against in the market
   vector<asset_symbol_type>       blacklist_markets;                     // The assets that this asset may not be traded against in the market, must not overlap whitelist
};


struct bitasset_data_api_obj
{
   bitasset_data_api_obj( const chain::asset_bitasset_data_object& b, const chain::database& db ):
      id( b.id ),
      symbol( b.symbol ),
      issuer( b.issuer ),
      backing_asset( b.backing_asset ),
      current_feed( b.current_feed ),
      current_feed_publication_time( current_feed_publication_time ),
      force_settled_volume( b.force_settled_volume ),
      settlement_price( b.settlement_price ),
      settlement_fund( b.settlement_fund ),
      asset_cer_updated( b.asset_cer_updated ),
      feed_cer_updated( b.asset_cer_updated ),
      feed_lifetime( b.options.feed_lifetime ),
      minimum_feeds( b.options.minimum_feeds ),
      force_settlement_delay( b.options.force_settlement_delay ),
      force_settlement_offset_percent( b.options.force_settlement_offset_percent ),
      maximum_force_settlement_volume( b.options.maximum_force_settlement_volume ),
      {
         for( auto feed: b.feeds )
         {
            feeds[ feed.first ] = feed.second;
         }
      }
   
   bitasset_data_api_obj(){}

   asset_bitasset_data_id_type                             id;
   asset_symbol_type                                       symbol;                                  // The symbol of the bitasset that this object belongs to
   account_name_type                                       issuer;                                  // The account name of the issuer 
   asset_symbol_type                                       backing_asset;             // The collateral backing asset of the bitasset
   map<account_name_type, pair<time_point,price_feed>>     feeds;                       // Feeds published for this asset. 
   price_feed                                              current_feed;                            // Currently active price feed, median of values from the currently active feeds.
   time_point                                              current_feed_publication_time;           // Publication time of the oldest feed which was factored into current_feed.
   price                                                   current_maintenance_collateralization;   // Call orders with collateralization (aka collateral/debt) not greater than this value are in margin call territory.
   int64_t                                                 force_settled_volume;                    // This is the volume of this asset which has been force-settled this maintanence interval
   price                                                   settlement_price;      // Price at which force settlements of a black swanned asset will occur
   int64_t                                                 settlement_fund;       // Amount of collateral which is available for force settlement
   bool                                                    asset_cer_updated;       // Track whether core_exchange_rate in corresponding asset_object has updated
   bool                                                    feed_cer_updated;// Track whether core exchange rate in current feed has updated
   fc::microseconds                                        feed_lifetime;                            // Time before a price feed expires
   uint8_t                                                 minimum_feeds;                                              // Minimum number of unexpired feeds required to extract a median feed from
   fc::microseconds                                        force_settlement_delay;                // This is the delay between the time a long requests settlement and the chain evaluates the settlement
   uint16_t                                                force_settlement_offset_percent;      // The percentage to adjust the feed price in the short's favor in the event of a forced settlement
   uint16_t                                                maximum_force_settlement_volume;  // the percentage of current supply which may be force settled within each maintenance interval.
};


struct equity_data_api_obj
{
   equity_data_api_obj( const chain::asset_equity_data_object& e, const chain::database& db ):
      id( e.id ),
      dividend_asset( e.dividend_asset ),
      dividend_pool( e.dividend_pool ),
      last_dividend( e.last_dividend ),
      dividend_share_percent( e.options.dividend_share_percent ),
      liquid_dividend_percent( e.options.liquid_dividend_percent ),
      staked_dividend_percent( e.options.staked_dividend_percent ),
      savings_dividend_percent( e.options.savings_dividend_percent ),
      liquid_voting_rights( e.options.liquid_voting_rights ),
      staked_voting_rights( e.options.staked_voting_rights ),
      savings_voting_rights( e.options.savings_voting_rights ),
      min_active_time( e.options.min_active_time ),
      min_balance( e.options.min_balance ),
      min_witnesses( e.options.min_witnesses ),
      boost_balance( e.options.boost_balance ),
      boost_activity( e.options.boost_activity ),
      boost_witnesses( e.options.boost_witnesses ),
      boost_top( e.options.boost_top ),

   equity_data_api_obj(){}

   asset_equity_data_id_type                    id;
   asset_symbol_type                            dividend_asset;                // The asset used to distribute dividends to asset holders
   asset                                        dividend_pool;                 // Amount of assets pooled for distribution at the next interval
   time_point                                   last_dividend;                 // Time that the asset last distributed a dividend.
   uint16_t                                     dividend_share_percent;        // Percentage of incoming assets added to the dividends pool
   uint16_t                                     liquid_dividend_percent;       // percentage of equity dividends distributed to liquid balances
   uint16_t                                     staked_dividend_percent;       // percentage of equity dividends distributed to staked balances
   uint16_t                                     savings_dividend_percent;      // percentage of equity dividends distributed to savings balances
   uint16_t                                     liquid_voting_rights;          // Amount of votes per asset conveyed to liquid holders of the asset
   uint16_t                                     staked_voting_rights;          // Amount of votes per asset conveyed to staked holders of the asset
   uint16_t                                     savings_voting_rights;         // Amount of votes per asset conveyed to savings holders of the asset
   fc::microseconds                             min_active_time;
   int64_t                                      min_balance;
   uint16_t                                     min_witnesses;
   uint16_t                                     boost_balance;
   uint16_t                                     boost_activity;
   uint16_t                                     boost_witnesses;
   uint16_t                                     boost_top;
};


struct credit_data_api_obj
{
   credit_data_api_obj( const asset_credit_data_object& c, const database& db ):
      id( c.id ),
      buyback_asset( c.buyback_asset ),
      buyback_pool( c.buyback_pool ),
      buyback_price( c.buyback_price ),
      symbol_a( c.symbol_a ),
      symbol_b( c.symbol_b ),
      last_buyback( c.last_buyback ),
      buyback_share_percent( c.options.buyback_share_percent ),
      liquid_fixed_interest_rate( c.options.liquid_fixed_interest_rate ),
      liquid_variable_interest_rate( c.options.liquid_variable_interest_rate ),
      staked_fixed_interest_rate( c.options.staked_fixed_interest_rate ),
      staked_variable_interest_rate( c.options.staked_variable_interest_rate ),
      savings_fixed_interest_rate( c.options.savings_fixed_interest_rate ),
      savings_variable_interest_rate( c.options.savings_variable_interest_rate ),
      var_interest_range( c.options.var_interest_range ),

   credit_data_api_obj(){}

   asset_credit_data_id_type  id;
   asset_symbol_type          buyback_asset;                             // Symbol used to buyback credit assets
   asset                      buyback_pool;                              // Amount of assets pooled to buyback the asset at next interval
   price                      buyback_price;                             // Price at which the credit asset is bought back
   asset_symbol_type          symbol_a;                                  // the asset with the lower id in the buyback price pair
   asset_symbol_type          symbol_b;                                  // the asset with the greater id in the buyback price pair
   time_point                 last_buyback;                              // Time that the asset was last updated
   uint32_t                   buyback_share_percent;                     // Percentage of incoming assets added to the buyback pool
   uint32_t                   liquid_fixed_interest_rate;                // Fixed component of Interest rate of the asset for liquid balances.
   uint32_t                   liquid_variable_interest_rate;             // Variable component of Interest rate of the asset for liquid balances.
   uint32_t                   staked_fixed_interest_rate;                // Fixed component of Interest rate of the asset for staked balances.
   uint32_t                   staked_variable_interest_rate;             // Variable component of Interest rate of the asset for staked balances.
   uint32_t                   savings_fixed_interest_rate;               // Fixed component of Interest rate of the asset for savings balances.
   uint32_t                   savings_variable_interest_rate;            // Variable component of Interest rate of the asset for savings balances.
   uint32_t                   var_interest_range;                        // The percentage range from the buyback price over which to apply the variable interest rate.
};

struct liquidity_pool_api_obj
{
   liquidity_pool_api_obj( const chain::asset_liquidity_pool_object& p, const database& db ):
      id( p.id ),
      issuer( p.issuer ),
      symbol_a( p.symbol_a ),
      symbol_b( p.symbol_b ),
      symbol_liquid( p.symbol_liquid ),
      balance_a( p.balance_a ),
      balance_b( p.balance_b ),
      balance_liquid( p.balance_liquid ),
      hour_median_price( p.hour_median_price ),
      day_median_price( p.day_median_price ),
      {
         bip::deque< price, allocator< price > > feeds = p.price_history;
         for( auto feed : feeds )
         {
            price_history.push_back( feed );
         }
      }

   liquidity_pool_api_obj(){}

   asset_liquidity_pool_id_type           id; 
   account_name_type                      issuer;                        // Name of the account which created the liquidity pool.
   asset_symbol_type                      symbol_a;                      // Ticker symbol string of the asset with the lower ID. Must be core asset if one asset is core.
   asset_symbol_type                      symbol_b;                      // Ticker symbol string of the asset with the higher ID.
   asset_symbol_type                      symbol_liquid;                 // Ticker symbol of the pool's liquidity pool asset. 
   asset                                  balance_a;                     // Balance of Asset A. Must be core asset if one asset is core.
   asset                                  balance_b;                     // Balance of Asset B.
   asset                                  balance_liquid;                // Outstanding supply of the liquidity asset for the asset pair.
   price                                  hour_median_price;             // The median price over the past hour, at 10 minute intervals. Used for collateral calculations. 
   price                                  day_median_price;              // The median price over the last day, at 10 minute intervals.
   vector< price >                        price_history;                 // Tracks the last 24 hours of median price, one per 10 minutes.
};



struct credit_pool_api_obj
{
   credit_pool_api_obj( const chain::asset_credit_pool_object& p, const database& db ):
      id( p.id ),
      issuer( p.issuer ),
      base_symbol( p.base_symbol ),
      credit_symbol( p.credit_symbol ),
      base_balance( p.base_balance ),
      borrowed_balance( p.borrowed_balance ),
      credit_balance( p.credit_balance ),
      last_interest_rate( p.last_interest_rate ),
      last_price( p.last_price ),

   credit_pool_api_obj(){}

   asset_credit_pool_id_type         id; 
   account_name_type                 issuer;                 // Name of the account which created the credit pool.
   asset_symbol_type                 base_symbol;            // Ticker symbol string of the base asset being lent and borrowed.
   asset_symbol_type                 credit_symbol;          // Ticker symbol string of the credit asset for use as collateral to borrow the base asset.
   asset                             base_balance;           // Balance of the base asset that is available for loans and redemptions. 
   asset                             borrowed_balance;       // Total amount of base asset currently lent to borrowers, accumulates compounding interest payments. 
   asset                             credit_balance;         // Balance of the credit asset redeemable for an increasing amount of base asset.
   int64_t                           last_interest_rate;     // The most recently calculated interest rate when last compounded. 
   price                             last_price;             // The last price that assets were lent or withdrawn at. 
}

struct limit_order_api_obj
{
   limit_order_api_obj( chain::limit_order_object& o, database& db ):
      id( o.id ),
      created( o.created ),
      expiration( o.expiration ),
      seller( o.seller ),
      order_id( to_string( o.order_id ) ),
      for_sale( o.for_sale ),
      sell_price( o.sell_price ),
      interface( o.interface ),
      real_price( o.real_price() ),

   limit_order_api_obj(){}

   limit_order_id_type    id;
   time_point             created;           // Time that the order was created.
   time_point             expiration;        // Expiration time of the order.
   account_name_type      seller;            // Selling account name of the trading order.
   shared_string          order_id;          // UUIDv4 of the order for each account.
   int64_t                for_sale;          // asset symbol is sell_price.base.symbol
   price                  sell_price;        // Base price is the asset being sold.
   account_name_type      interface;         // The interface account that created the order
   double                 real_price;
};


struct margin_order_api_obj
{
   margin_order_api_obj( chain::margin_order_object& o, database& db ):
      id( o.id ),
      owner( o.owner ),
      order_id( to_string( o.order_id ) ),
      sell_price( o.sell_price ),
      collateral( o.collateral ),
      debt( o.debt ),
      debt_balance( o.debt_balance ),
      interest( o.interest ),
      position( o.position ),
      position_filled( o.position_filled ),
      collateralization( o.collateralization ),
      interface( o.interface ),
      created( o.created ),
      expiration( o.expiration ),
      unrealized_value( o.unrealized_value ),
      last_interest_rate( o.last_interest_rate ),
      liquidating( o.liquidating ),
      stop_loss_price( o.stop_loss_price ),
      take_profit_price( o.take_profit_price ),
      limit_stop_loss_price( o.limit_stop_loss_price ),
      limit_take_profit_price( o.limit_take_profit_price ),
      real_price( o.real_price() ),
      
   margin_order_api_obj(){}

   margin_order_id_type       id;
   account_name_type          owner;                       // Margin order owners account name
   string                     order_id;                    // UUIDv4 Unique Identifier of the order for each account.
   price                      sell_price;                  // limit exchange price of the borrowed asset being sold for the position asset.
   asset                      collateral;                  // Collateral asset used to back the loan value; Returned to credit collateral object when position is closed. 
   asset                      debt;                        // Amount of asset borrowed to purchase the position asset. Repaid when the margin order is closed. 
   asset                      debt_balance;                // Debt asset that is held by the order when selling debt, or liquidating position.
   asset                      interest;                    // Amount of interest accrued on the borrowed asset into the debt value.
   asset                      position;                    // Minimum amount of asset to receive as margin position.
   asset                      position_filled;             // Amount of asset currently held within the order that has filled.                     
   int64_t                    collateralization;           // Percentage ratio of ( Collateral + position_filled + debt_balance - debt ) / debt. Position is liquidated when ratio falls below liquidation requirement 
   account_name_type          interface;                   // The interface account that created the order.
   time_point                 created;                     // Time that the order was created.
   time_point                 last_updated;                // Time that interest was last compounded on the margin order, and collateralization was last updated. 
   time_point                 expiration;                  // Expiration time of the order.
   asset                      unrealized_value;            // Current profit or loss that the position is holding.
   int64_t                    last_interest_rate;          // The interest rate that was last applied to the order.
   bool                       liquidating;                 // Set to true to place the margin order back into the orderbook and liquidate the position at sell price.
   price                      stop_loss_price;             // Price at which the position will be force liquidated if it falls into a net loss.
   price                      take_profit_price;           // Price at which the position will be force liquidated if it rises into a net profit.
   price                      limit_stop_loss_price;       // Price at which the position will be limit liquidated if it falls into a net loss.
   price                      limit_take_profit_price;     // Price at which the position will be limit liquidated if it rises into a net profit.
   double                     real_price;
};

struct call_order_api_obj
{
   call_order_api_obj( chain::call_order_object& o, database& db ):
      id( o.id ),
      borrower( o.borrower ),
      collateral( o.collateral ),
      debt( o.debt ),
      call_price( o.call_price ),
      target_collateral_ratio( o.target_collateral_ratio ),
      interface( o.interface ),
      real_price( o.real_price() ),

   call_order_api_obj(){}

   call_order_id_type      id;
   account_name_type       borrower;
   asset                   collateral;                  // call_price.base.symbol, access via get_collateral
   asset                   debt;                        // call_price.quote.symbol, access via get_debt
   price                   call_price;                  // Collateral / Debt
   uint16_t                target_collateral_ratio;     // maximum CR to maintain when selling collateral on margin call
   account_name_type       interface;                   // The interface account that created the order
   double                  real_price;
};

struct credit_loan_api_obj
{
   credit_loan_api_obj( chain::credit_loan_object& o, database& db ):
      id( o.id ),
      owner( o.owner ),
      loan_id( to_string( o.loan_id ) ),
      debt( o.debt ),
      interest( o.interest ),
      collateral( o.collateral ),
      loan_price( o.loan_price ),
      liquidation_price( o.liquidation_price ),
      symbol_a( o.symbol_a ),
      symbol_b( o.symbol_b ),
      last_interest_rate( o.last_interest_rate ),
      created( o.created ),
      last_updated( o.last_updated ),
   
   credit_loan_api_obj(){}

   credit_loan_id_type        id;
   account_name_type          owner;                   // Collateral owner's account name
   string                     loan_id;                 // UUIDV4 for the loan to uniquely identify it for reference. 
   asset                      debt;                    // Amount of an asset borrowed. Limit of 75% of collateral value. Increases with interest charged.
   asset                      interest;                // Total Amount of interest accrued on the loan. 
   asset                      collateral;              // Amount of an asset to use as collateral for the loan. 
   price                      loan_price;              // Collateral / Debt. Must be higher than liquidation price to remain solvent. 
   price                      liquidation_price;       // Collateral / max_debt value. Rises when collateral/debt market price falls.
   asset_symbol_type          symbol_a;                // The symbol of asset A in the debt / collateral exchange pair.
   asset_symbol_type          symbol_b;                // The symbol of asset B in the debt / collateral exchange pair.
   int64_t                    last_interest_rate;      // Updates the interest rate of the loan hourly. 
   time_point                 created;                 // Time that the loan was taken out.
   time_point                 last_updated;            // Time that the loan was last updated, and interest was accrued.
};

struct credit_collateral_api_obj
{
   credit_collateral_api_obj( chain::credit_loan_object& o, database& db ):
      id( o.id ),
      owner( o.owner ),
      symbol( o.symbol ),
      collateral( o.collateral ),

   credit_collateral_api_obj(){}

   credit_collateral_id_type                    id;
   account_name_type                            owner;         // Collateral owners account name.
   asset_symbol_type                            symbol;        // Asset symbol being collateralized. 
   asset                                        collateral;    // Asset balance that is being locked in for loan backing for loan or margin orders.  
};

struct owner_authority_history_api_obj
{
   owner_authority_history_api_obj( const chain::owner_authority_history_object& o ) :
      id( o.id ),
      account( o.account ),
      previous_owner_authority( authority( o.previous_owner_authority ) ),
      last_valid_time( o.last_valid_time ),
   

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
   int64_t              votes;
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
