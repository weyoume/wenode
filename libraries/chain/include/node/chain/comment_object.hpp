#pragma once

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/chain/witness_objects.hpp>

#include <boost/multi_index/composite_key.hpp>


namespace node { namespace chain {

   using protocol::beneficiary_route_type;

   class comment_object : public object < comment_object_type, comment_object >
   {
      comment_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         comment_object( Constructor&& c, allocator< Allocator > a )
            :category( a ), parent_permlink( a ), permlink( a ), body( a ), json( a ), beneficiaries( a )
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              author;                       // Name of the account that created the post.

         shared_string                  permlink;                     // Unique identifing string for the post.

         shared_string                  title;                        // String containing title text.

         shared_string                  body;                         // String containing text for display when the post is opened.

         vector< shared_string >        ipfs;                         // String containing a display image or video file as an IPFS file hash.

         vector< shared_string >        magnet;                       // String containing a bittorrent magnet link to a file swarm.

         post_format_type               post_type;                    // The type of post that is being created, image, text, article, video etc. 

         bool                           privacy;                      // True if the post is encrypted. False if it is plaintext.

         public_key_type                public_key;                   // The public key used to encrypt the post, holders of the private key may decrypt. 

         feed_reach_type                reach;                        // The reach of the post across followers, connections, friends and companions

         board_name_type                board;                        // The name of the board to which the post is uploaded to. Null string if no board. 

         vector< tag_name_type >        tags;                         // Set of string tags for sorting the post by.

         account_name_type              interface;                    // Name of the interface account that was used to broadcast the transaction and view the post.

         post_rating_type               rating;                       // User nominated rating as to the maturity of the content, and display sensitivity. 

         shared_string                  language;                     // String containing a two letter language code that the post is broadcast in.

         id_type                        root_comment;                 // The root post that the comment is an ancestor of. 

         account_name_type              parent_author;                // Account that created the post this post is replying to, empty if root post. 

         shared_string                  parent_permlink;              // permlink of the post this post is replying to, empty if root post. 

         shared_string                  json;                         // JSON metadata of the post, including Link, and additional interface specific data relating to the post.

         shared_string                  category;                     // Permlink of root post that this comment is applied to.

         asset                          comment_price;                // The price paid to create a comment

         asset                          premium_price;                // The price paid to unlock the post's premium encryption.

         flat_map< account_name_type, flat_map< asset_symbol_type, asset > >  payments_received;    // Map of all transfers received that referenced this comment. 

         bip::vector< beneficiary_route_type, allocator< beneficiary_route_type > > beneficiaries;  // Vector of beneficiary routes that receive a content reward distribution.
         
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
 
         time_point                     cashout_time;                 // Next scheduled time to receive a content reward cashout.

         uint32_t                       cashouts_received = 0;        // Number of times that the comment has received content rewards

         uint128_t                      total_vote_weight = 0;        // the total weight of votes, used to calculate pro-rata share of curation payouts

         uint128_t                      total_view_weight = 0;        // the total weight of views, used to calculate pro-rata share of curation payouts

         uint128_t                      total_share_weight = 0;       // the total weight of shares, used to calculate pro-rata share of curation payouts

         uint128_t                      total_comment_weight = 0;     // the total weight of comments, used to calculate pro-rata share of curation payouts

         asset                          total_payout_value = asset( 0, SYMBOL_USD ); // The total payout this comment has received over time, measured in USD */

         asset                          curator_payout_value = asset( 0, SYMBOL_USD );

         asset                          beneficiary_payout_value = asset( 0, SYMBOL_USD );

         asset                          content_rewards = asset( 0, SYMBOL_COIN );

         share_type                     percent_liquid = PERCENT_100;

         int128_t                       reward = 0;                   // The amount of reward_curve this comment is responsible for in its root post.

         uint128_t                      weight = 0;                   // Used to define the comment curation reward this comment receives.

         uint128_t                      max_weight = 0;               // Used to define relative contribution of this comment to rewards.

         asset                          max_accepted_payout;          // USD value of the maximum payout this post will receive

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

         bool                           comment_paid( account_name_type name ) const    // return true if user has paid comment price
         {
            if( comment_price.amount > 0 )
            {
               if( payments_received.find( name ) != payments_received.end() )
               {
                  flat_map< asset_symbol_type, asset > payments = payments_received.at( name );

                  if( payments.find( comment_price.symbol ) != payments.end() )
                  {
                     asset comment_payment = payments.at( comment_price.symbol );

                     if( comment_payment >= comment_price )
                     {
                        return true;    // comment price payment received.
                     }
                     else
                     {
                        return false; 
                     } 
                  }
                  else
                  {
                     return false; 
                  }
               }
               else
               {
                  return false; 
               }
            }
            else
            {
               return true;     // No comment price, allow all comments.
            }
         }   
   };
   

   /**
    * Feed objects are used to hold the posts that have been posted or shared by the 
    * accounts that a user follows or is connected with, the boards that they follow, or
    * the tags that they follow. Operates like inbox.
    */
   class feed_object : public object< feed_object_type, feed_object >
   {
      feed_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         feed_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                     id;

         account_name_type                           account;               // Account that should see comment in their feed.

         comment_id_type                             comment;               // ID of comment being shared

         feed_reach_type                             feed_type;             // Type of feed, follow, connection, board, tag etc. 

         flat_map< account_name_type, time_point >   shared_by;             // Map of the times that accounts that have shared the comment.

         flat_map< board_name_type, flat_map< account_name_type, time_point > >   boards;  // Map of all boards that the comment has been shared with

         flat_map< tag_name_type, flat_map< account_name_type, time_point > >     tags;    // Map of all tags that the comment has been shared with.

         account_name_type                           first_shared_by;       // First account that shared the comment with account. 

         uint32_t                                    shares;                // Number of accounts that have shared the comment with account.

         time_point                                  feed_time;             // Time that the comment was added or last shared with account. 
   };


   /**
    * Blog objects hold posts that are shared or posted by a particular account or to a tag, or a board.
    * Operates like outbox. 
    */
   class blog_object : public object< blog_object_type, blog_object >
   {
      blog_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         blog_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                   id;

         account_name_type         account;               // Blog or sharing account for account type blogs, null for other types

         board_name_type           board;                 // Board posted or shared to for board type blogs

         tag_name_type             tag;                   // Tag posted or shared to for tag type blogs.            

         comment_id_type           comment;               // Comment ID

         flat_map< account_name_type, time_point >   shared_by;     // Map of the times that accounts that have shared the comment in the blog.

         blog_reach_type           blog_type;             // Account, Board, or Tag blog

         account_name_type         first_shared_by;       // First account that shared the comment with the account, board or tag. 

         uint32_t                  shares;                // Number of accounts that have shared the comment with account, board or tag.

         time_point                blog_time;             // Latest time that the comment was shared on the account, board or tag
   };


   /**
    * This index maintains the set of voter/comment pairs that have been used, voters cannot
    * vote on the same comment more than once per payout period.
    */
   class comment_vote_object : public object< comment_vote_object_type, comment_vote_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         comment_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       voter;

         comment_id_type         comment;            // ID of the comment

         uint128_t               weight = 0;         // Used to define the curation reward this vote receives. Decays with time and additional votes.

         uint128_t               max_weight = 0;     // Used to define relative contribution of this comment to rewards.

         int128_t                reward = 0;         // The amount of reward_curve this vote is responsible for

         int16_t                 vote_percent = 0;   //  The percent weight of the vote

         time_point              last_update;        // The time of the last update of the vote

         time_point              created;            // Time the vote was created

         int8_t                  num_changes = 0;    // Number of times the vote has been adjusted
   };


   /**
    * This index maintains the set of viewer/comment pairs that have been used one view per account
    * vote on the same comment more than once per payout period.
    */
   class comment_view_object : public object< comment_view_object_type, comment_view_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         comment_view_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       viewer;             // Name of the viewing account.

         comment_id_type         comment;            // ID of the comment.

         account_name_type       interface;          // Name of the interface account that was used to broadcast the transaction and view the post. 

         account_name_type       supernode;          // Name of the supernode account that served the IPFS file data in the post.

         int128_t                reward = 0;         // The amount of voting power this view contributed.

         uint128_t               weight = 0;         // The curation reward weight of the view. Decays with time and additional views.

         uint128_t               max_weight = 0;     // Used to define relative contribution of this view to rewards.

         time_point              created;            // Time the view was created
   };


   class comment_share_object : public object< comment_share_object_type, comment_share_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         comment_share_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       sharer;             // Name of the sharing account.

         comment_id_type         comment;            // ID of the comment.

         account_name_type       interface;          // Name of the interface account that was used to broadcast the transaction and view the post. 

         int128_t                reward = 0;         // The amount of voting power this share contributed.

         uint128_t               weight = 0;         // The curation reward weight of the share.

         uint128_t               max_weight = 0;     // Used to define relative contribution of this share to rewards.

         time_point              created;            // Time the share was created
   };


   /**
    * Moderation Tag objects are used by board moderators and governance addresses to apply
    * tags detailing the type of content that they find on the network that is 
    * in opposition to the moderation policies of that moderator's board rules, 
    * or the governance addresses content standards.
    */
   class moderation_tag_object : public object < moderation_tag_object_type, moderation_tag_object >
   {
      moderation_tag_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         moderation_tag_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              moderator;        // Name of the moderator or goverance account that created the comment tag.

         comment_id_type                comment;          // ID of the comment.

         board_name_type                board;            // The name of the board to which the post is uploaded to.

         vector< tag_name_type >        tags;             // Set of string tags for sorting the post by

         post_rating_type               rating;           // Moderator updated rating as to the maturity of the content, and display sensitivity. 

         shared_string                  details;          // Explaination as to what rule the post is in contravention of and why it was tagged.

         account_name_type              interface;        // Interface account used for the transaction

         bool                           filter;           // True if the post should be filtered by the board or governance address subscribers. 

         time_point                     last_update;      // Time the comment tag was last edited by the author.

         time_point                     created;          // Time that the comment tag was created.
   };


   /** 
    * Encrypted Private messages are sent between users following this specification:
    * 1 - Plaintext is encrypted with the private key of the sender
    * 2 - This is then encrypted with the public key of the recipient.
    * 3 - The ciphertext is included in the message field
    * 4 - Each message generates a UUIDV4 for local storage reference.
    */
   class message_object : public object< message_object_type, message_object>
   {
      public:
         template< typename Constructor, typename Allocator >
         message_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       sender;                   // Name of the message sender.

         account_name_type       recipient;                // Name of the intended message recipient.

         public_key_type         sender_public_key;        // Public secure key of the sender.

         public_key_type         recipient_public_key;     // Public secure key of the recipient.

         shared_string           message;                  // Encrypted private message ciphertext.

         shared_string           json;                     // Encrypted Message metadata.

         shared_string           uuid;                     // uuidv4 uniquely identifying the message for local storage.

         time_point              last_updated;             // Time the message was last changed, used to reload encrypted message storage.

         time_point              created;                  // Time the message was sent.
   };


   /**
    * This object gets updated once per hour, with current average and median statistics of all posts on the network.
    * Used for tag object sorting equalization formulae and network monitoring.
    */
   class comment_metrics_object  : public object< comment_metrics_object_type, comment_metrics_object >
   {
      comment_metrics_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         comment_metrics_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         uint32_t                recent_post_count = 0;        // Number of root posts in the last 30 days, rolling average.

         int128_t                recent_vote_power = 0;        // Sum of vote power for all posts in last 30 days

         int128_t                recent_view_power = 0;        // Sum of view power for all posts in last 30 days

         int128_t                recent_share_power = 0;       // Sum of share power for all posts in last 30 days

         int128_t                recent_comment_power = 0;     // Sum of comment power for all posts in last 30 days

         int128_t                average_vote_power = 0;       // Recent vote power / recent post count

         int128_t                average_view_power = 0;       // Recent view power / recent post count

         int128_t                average_share_power = 0;      // Recent share power / recent post count

         int128_t                average_comment_power = 0;    // Recent comment power / recent post count

         int128_t                median_vote_power = 0;        // Vote power of the post ranked for vote power at position recent_post_count / 2

         int128_t                median_view_power = 0;        // View power of the post ranked for view power at position recent_post_count / 2

         int128_t                median_share_power = 0;       // Share power of the post ranked for share power at position recent_post_count / 2

         int128_t                median_comment_power = 0;     // Comment power of the post ranked for comment power at position recent_post_count / 2

         uint32_t                recent_vote_count = 0;        // Sum of net_votes for all posts in last 30 days

         uint32_t                recent_view_count = 0;        // Sum of view_count for all posts in last 30 days

         uint32_t                recent_share_count = 0;       // Sum of share_count for all posts in last 30 days  

         uint32_t                recent_comment_count = 0;     // Sum of children for all posts in last 30 days

         uint32_t                average_vote_count = 0;       // Recent vote count / recent post count

         uint32_t                average_view_count = 0;       // Recent view count / recent post count

         uint32_t                average_share_count = 0;      // Recent share count / recent post count

         uint32_t                average_comment_count = 0;    // Recent comment count / recent post count

         uint32_t                median_vote_count = 0;        // Vote count of the post ranked for vote count at position recent_post_count / 2

         uint32_t                median_view_count = 0;        // View count of the post ranked for view count at position recent_post_count / 2

         uint32_t                median_share_count = 0;       // Share count of the post ranked for share count at position recent_post_count / 2

         uint32_t                median_comment_count = 0;     // Comment count of the post ranked for comment count at position recent_post_count / 2

         double                  vote_view_ratio = 0;          // recent view power / recent vote power ratio

         double                  vote_share_ratio = 0;         // recent share power / recent vote power ratio

         double                  vote_comment_ratio = 0;       // recent comment power / recent vote power ration

         time_point              last_update;                  // Time of last metrics update
   };


   struct by_comment_voter;
   struct by_voter_comment;
   struct by_comment_weight_voter;
   struct by_voter_last_update;
   struct by_permlink;

   typedef multi_index_container<
      comment_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_vote_object, comment_vote_id_type, &comment_vote_object::id > >,
         ordered_unique< tag< by_comment_voter >,
            composite_key< comment_vote_object,
               member< comment_vote_object, comment_id_type, &comment_vote_object::comment>,
               member< comment_vote_object, account_name_type, &comment_vote_object::voter>
            >
         >,
         ordered_unique< tag< by_voter_comment >,
            composite_key< comment_vote_object,
               member< comment_vote_object, account_name_type, &comment_vote_object::voter>,
               member< comment_vote_object, comment_id_type, &comment_vote_object::comment>
            >
         >,
         ordered_unique< tag< by_voter_last_update >,
            composite_key< comment_vote_object,
               member< comment_vote_object, account_name_type, &comment_vote_object::voter>,
               member< comment_vote_object, time_point, &comment_vote_object::last_update>,
               member< comment_vote_object, comment_id_type, &comment_vote_object::comment>
            >,
            composite_key_compare< std::less< account_id_type >, std::greater< time_point >, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_comment_weight_voter >,
            composite_key< comment_vote_object,
               member< comment_vote_object, comment_id_type, &comment_vote_object::comment>,
               member< comment_vote_object, uint128_t, &comment_vote_object::weight>,
               member< comment_vote_object, account_name_type, &comment_vote_object::voter>
            >,
            composite_key_compare< std::less< comment_id_type >, std::greater< uint128_t >, std::less< account_id_type > >
         >
      >,
      allocator< comment_vote_object >
   > comment_vote_index;

   struct by_comment_viewer;
   struct by_viewer_comment;

   struct by_interface_viewer;
   struct by_supernode_viewer;
   struct by_comment_weight_viewer;

   typedef multi_index_container<
      comment_view_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_view_object, comment_view_id_type, &comment_view_object::id > >,
         ordered_unique< tag< by_comment_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, comment_id_type, &comment_view_object::comment>,
               member< comment_view_object, account_name_type, &comment_view_object::viewer>
            >
         >,
         ordered_unique< tag< by_viewer_comment >,
            composite_key< comment_view_object,
               member< comment_view_object, account_name_type, &comment_view_object::viewer>,
               member< comment_view_object, comment_id_type, &comment_view_object::comment>
            >
         >,
         ordered_unique< tag< by_interface_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, account_name_type, &comment_view_object::interface>,
               member< comment_view_object, account_name_type, &comment_view_object::viewer>,
               member< comment_view_object, time_point, &comment_view_object::created>,
               member< comment_view_object, comment_id_type, &comment_view_object::comment>
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_name_type >, std::greater< time_point >, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_supernode_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, account_name_type, &comment_view_object::supernode>,
               member< comment_view_object, account_name_type, &comment_view_object::viewer>,
               member< comment_view_object, time_point, &comment_view_object::created>,
               member< comment_view_object, comment_id_type, &comment_view_object::comment>
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_name_type >, std::greater< time_point >, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_comment_weight_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, comment_id_type, &comment_view_object::comment>,
               member< comment_view_object, uint128_t, &comment_view_object::weight>,
               member< comment_view_object, account_name_type, &comment_view_object::viewer>
            >,
            composite_key_compare< std::less< comment_id_type >, std::greater< uint128_t >, std::less< account_id_type > >
         >
      >,
      allocator< comment_view_object >
   > comment_view_index;

   struct by_comment_sharer;
   struct by_sharer_comment;
   struct by_sharer_last_update;
   struct by_comment_weight_sharer;

   typedef multi_index_container<
      comment_share_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_share_object, comment_share_id_type, &comment_share_object::id > >,
         ordered_unique< tag< by_comment_sharer >,
            composite_key< comment_share_object,
               member< comment_share_object, comment_id_type, &comment_share_object::comment>,
               member< comment_share_object, account_name_type, &comment_share_object::sharer>
            >
         >,
         ordered_unique< tag< by_sharer_comment >,
            composite_key< comment_share_object,
               member< comment_share_object, account_name_type, &comment_share_object::sharer>,
               member< comment_share_object, comment_id_type, &comment_share_object::comment>
            >
         >,
         ordered_unique< tag< by_comment_weight_sharer >,
            composite_key< comment_share_object,
               member< comment_share_object, comment_id_type, &comment_share_object::comment>,
               member< comment_share_object, uint128_t, &comment_share_object::weight>,
               member< comment_share_object, account_name_type, &comment_share_object::sharer>
            >,
            composite_key_compare< std::less< comment_id_type >, std::greater< uint128_t >, std::less< account_id_type > >
         >
      >,
      allocator< comment_share_object >
   > comment_share_index;

   struct by_new_account_type;
   struct by_old_account_type;
   struct by_new_account;
   struct by_old_account;
   struct by_account_comment_type;
   struct by_comment;

   typedef multi_index_container<
      feed_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< feed_object, feed_id_type, &feed_object::id > >,
         ordered_unique< tag< by_new_account_type >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, feed_reach_type, &feed_object::feed_type >,
               member< feed_object, time_point, &feed_object::feed_time >,
               member< feed_object, feed_id_type, &feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< feed_reach_type >, 
               std::greater< time_point >,   // Newest feeds first
               std::less< feed_id_type > 
            >
         >,
         ordered_unique< tag< by_old_account_type >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, feed_reach_type, &feed_object::feed_type >,
               member< feed_object, time_point, &feed_object::feed_time >,
               member< feed_object, feed_id_type, &feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< feed_reach_type >, 
               std::less< time_point >,     // Oldest feeds first
               std::less< feed_id_type > 
            >
         >,
         ordered_unique< tag< by_new_account >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, time_point, &feed_object::feed_time >,
               member< feed_object, feed_id_type, &feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< time_point >,     // Newest feeds first
               std::less< feed_id_type > 
            >
         >,
         ordered_unique< tag< by_new_account >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, time_point, &feed_object::feed_time >,
               member< feed_object, feed_id_type, &feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< time_point >,     // Oldest feeds first
               std::less< feed_id_type > 
            >
         >,
         ordered_unique< tag< by_account_comment_type >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, comment_id_type, &feed_object::comment >,
               member< feed_object, feed_reach_type, &feed_object::feed_type >,
               member< feed_object, feed_id_type, &feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< feed_reach_type >, 
               std::less< comment_id_type >, 
               std::less< feed_id_type > 
            >
         >,
         ordered_unique< tag< by_comment >,
            composite_key< feed_object,
               member< feed_object, comment_id_type, &feed_object::comment >,
               member< feed_object, feed_id_type, &feed_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< feed_id_type > 
            >
         >
      >,
      allocator< feed_object >
   > feed_index;

   struct by_new_account_blog;
   struct by_old_account_blog;
   struct by_new_board_blog;
   struct by_old_board_blog;
   struct by_new_tag_blog;
   struct by_old_tag_blog;
   struct by_comment_account;
   struct by_comment_board;
   struct by_comment_tag;

   typedef multi_index_container<
      blog_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< blog_object, blog_id_type, &blog_object::id > >,
         ordered_unique< tag< by_new_account_blog >,
            composite_key< blog_object,
               member< blog_object, account_name_type, &blog_object::account >,
               member< blog_object, time_point, &blog_object::blog_time >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< time_point >, 
               std::less< blog_id_type >
            >
         >,
         ordered_unique< tag< by_old_account_blog >,
            composite_key< blog_object,
               member< blog_object, account_name_type, &blog_object::account >,
               member< blog_object, time_point, &blog_object::blog_time >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< time_point >,
               std::less< blog_id_type >
            >
         >,
         ordered_unique< tag< by_new_board_blog >,
            composite_key< blog_object,
               member< blog_object, board_name_type, &blog_object::board >,
               member< blog_object, time_point, &blog_object::blog_time >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< board_name_type >, 
               std::greater< time_point >, 
               std::less< blog_id_type > 
            >
         >,
         ordered_unique< tag< by_old_board_blog >,
            composite_key< blog_object,
               member< blog_object, board_name_type, &blog_object::board >,
               member< blog_object, time_point, &blog_object::blog_time >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< board_name_type >, 
               std::less< time_point >, 
               std::less< blog_id_type > 
            >
         >,
         ordered_unique< tag< by_new_tag_blog >,
            composite_key< blog_object,
               member< blog_object, tag_name_type, &blog_object::tag >,
               member< blog_object, time_point, &blog_object::blog_time >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< tag_name_type >, 
               std::greater< time_point >, 
               std::less< blog_id_type > 
            >
         >,
         ordered_unique< tag< by_old_tag_blog >,
            composite_key< blog_object,
               member< blog_object, tag_name_type, &blog_object::tag >,
               member< blog_object, time_point, &blog_object::blog_time >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< tag_name_type >, 
               std::less< time_point >, 
               std::less< blog_id_type > 
            >
         >,
         ordered_unique< tag< by_comment_account >,
            composite_key< blog_object,
               member< blog_object, comment_id_type, &blog_object::comment >,
               member< blog_object, account_name_type, &blog_object::account >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< account_name_type >,
               std::less< blog_id_type > 
            >
         >,
         ordered_unique< tag< by_comment_board >,
            composite_key< blog_object,
               member< blog_object, comment_id_type, &blog_object::comment >,
               member< blog_object, board_name_type, &blog_object::board >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< board_name_type >,
               std::less< blog_id_type >  
            >
         >,
         ordered_unique< tag< by_comment_tag >,
            composite_key< blog_object,
               member< blog_object, comment_id_type, &blog_object::comment >,
               member< blog_object, tag_name_type, &blog_object::tag >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< tag_name_type >,
               std::less< blog_id_type >  
            >
         >,
         ordered_unique< tag< by_comment >,
            composite_key< blog_object,
               member< blog_object, comment_id_type, &blog_object::comment >,
               member< blog_object, blog_id_type, &blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >,
               std::less< blog_id_type >  
            >
         >
      >,
      allocator< blog_object >
   > blog_index;

   struct by_root_author;
   struct by_cashout_time; // cashout_time
   struct by_permlink; // author, perm
   struct by_root;
   struct by_root_weight;
   struct by_parent;
   struct by_active; // parent_auth, active
   struct by_pending_payout;
   struct by_total_pending_payout;
   struct by_last_update; // parent_auth, last_update
   struct by_created; // parent_auth, last_update
   struct by_payout; // parent_auth, last_update
   struct by_blog;
   struct by_votes;
   struct by_responses;
   struct by_author_last_update;
   struct by_time;
   struct by_title;

   typedef multi_index_container<
      comment_object,
      indexed_by<
         // CONSENSUS INDICIES - used by evaluators
         ordered_unique< tag< by_id >, member< comment_object, comment_id_type, &comment_object::id > >,
         ordered_unique< tag< by_root_author >,
            composite_key< comment_object,
               member< comment_object, bool, &comment_object::root >,
               member< comment_object, account_name_type, &comment_object::author >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< std::less< bool >, std::less< account_name_type >, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_cashout_time >,
            composite_key< comment_object,
               member< comment_object, time_point, &comment_object::cashout_time>,
               member< comment_object, comment_id_type, &comment_object::id >
            >
         >,
         ordered_unique< tag< by_permlink >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::author >,
               member< comment_object, shared_string, &comment_object::permlink >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         >,
         ordered_unique< tag< by_root >,
            composite_key< comment_object,
               member< comment_object, comment_id_type, &comment_object::root_comment >,
               member< comment_object, comment_id_type, &comment_object::id >
            >
         >,
         ordered_unique< tag< by_root_weight >,
            composite_key< comment_object,
               member< comment_object, comment_id_type, &comment_object::root_comment >,
               member< comment_object, uint128_t, &comment_object::weight>,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< std::less< comment_id_type >, std::greater< uint128_t >, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_time >,
            composite_key< comment_object,
               member< comment_object, bool, &comment_object::root >,
               member< comment_object, time_point, &comment_object::created >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< std::less< bool >, std::greater< time_point >, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_parent >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::parent_author >,
               member< comment_object, shared_string, &comment_object::parent_permlink >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less, std::less< comment_id_type > >
         >
         // NON_CONSENSUS INDICIES - used by APIs
#ifndef IS_LOW_MEM
         ,
         ordered_unique< tag< by_title >,
            composite_key< comment_object,
               member< comment_object, shared_string, &comment_object::title >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< strcmp_less, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_last_update >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::parent_author >,
               member< comment_object, time_point, &comment_object::last_update >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< time_point >, std::less< comment_id_type > >
         >,
         ordered_unique< tag< by_author_last_update >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::author >,
               member< comment_object, time_point, &comment_object::last_update >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< time_point >, std::less< comment_id_type > >
         >
#endif
      >,
      allocator< comment_object >
   > comment_index;

   struct by_sender_recipient;
   struct by_account_inbox;
   struct by_account_outbox;
   struct by_sender_uuid;

   typedef multi_index_container<
      message_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< message_object, message_id_type, &message_object::id > >,
         ordered_unique< tag< by_sender_recipient >,
            composite_key< message_object,
               member< message_object, account_name_type, &message_object::sender >,
               member< message_object, account_name_type, &message_object::recipient >,
               member< message_object, message_id_type, &message_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_name_type >, std::less< message_id_type > >
         >,
         ordered_unique< tag< by_sender_uuid >,
            composite_key< message_object,
               member< message_object, account_name_type, &message_object::sender >,
               member< message_object, shared_string, &message_object::uuid >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         >,
         ordered_unique< tag< by_account_inbox >,
            composite_key< message_object,
               member< message_object, account_name_type, &message_object::recipient >,
               member< message_object, time_point, &message_object::created >,
               member< message_object, message_id_type, &message_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< time_point >, std::less< message_id_type > >
         >,
         ordered_unique< tag< by_account_outbox >,
            composite_key< message_object,
               member< message_object, account_name_type, &message_object::sender >,
               member< message_object, time_point, &message_object::created >,
               member< message_object, message_id_type, &message_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< time_point >, std::less< message_id_type > >
         >
      >,
      allocator< message_object >
   > message_index;

   struct by_moderator_comment;
   struct by_comment_moderator;
   struct by_comment_updated;

   typedef multi_index_container<
      moderation_tag_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< moderation_tag_object, moderation_tag_id_type, &moderation_tag_object::id > >,
         ordered_unique< tag< by_comment_moderator >,
            composite_key< moderation_tag_object,
               member< moderation_tag_object, comment_id_type, &moderation_tag_object::comment>,
               member< moderation_tag_object, account_name_type, &moderation_tag_object::moderator>
            >
         >,
         ordered_unique< tag< by_comment_updated >,
            composite_key< moderation_tag_object,
               member< moderation_tag_object, comment_id_type, &moderation_tag_object::comment>,
               member< moderation_tag_object, time_point, &moderation_tag_object::last_update>,
               member< moderation_tag_object, moderation_tag_id_type , &moderation_tag_object::id>
            >,
            composite_key_compare< std::less< comment_id_type >, std::greater< time_point >, std::less< moderation_tag_id_type > >
         >,
         ordered_unique< tag< by_moderator_comment >,
            composite_key< moderation_tag_object,
               member< moderation_tag_object, account_name_type, &moderation_tag_object::moderator>,
               member< moderation_tag_object, comment_id_type, &moderation_tag_object::comment>
            >
         >
      >,
      allocator< moderation_tag_object >
   > moderation_tag_index;

   typedef multi_index_container<
      comment_metrics_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_metrics_object, comment_metrics_id_type, &comment_metrics_object::id > >
      >,
      allocator< comment_metrics_object >
   > comment_metrics_index;

} } // node::chain

FC_REFLECT( node::chain::comment_object,
         (id)
         (author)
         (permlink)
         (title)
         (body)
         (ipfs)
         (magnet)
         (post_type)
         (privacy)
         (public_key)
         (reach)
         (board)
         (tags)
         (interface)
         (rating)
         (language)
         (root_comment)
         (parent_author)
         (parent_permlink)
         (json)
         (category)
         (comment_price)
         (payments_received)
         (beneficiaries)
         (last_update)
         (created)
         (active)
         (last_payout)
         (author_reputation)
         (depth)
         (children)
         (net_votes)
         (view_count)
         (share_count)
         (net_reward)
         (vote_power)
         (view_power)
         (share_power)
         (comment_power)
         (cashout_time)
         (cashouts_received)
         (total_vote_weight)
         (total_view_weight)
         (total_share_weight)
         (total_comment_weight)
         (total_payout_value)
         (curator_payout_value)
         (beneficiary_payout_value)
         (content_rewards)
         (percent_liquid)
         (reward)
         (weight)
         (max_weight)
         (max_accepted_payout)
         (author_reward_percent)
         (vote_reward_percent)
         (view_reward_percent)
         (share_reward_percent)
         (comment_reward_percent)
         (storage_reward_percent)
         (moderation_reward_percent)
         (allow_replies)
         (allow_votes)
         (allow_views)
         (allows_shares)
         (allow_curation_rewards)
         (root)
         (deleted)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_object, node::chain::comment_index );

FC_REFLECT( node::chain::feed_object, 
         (id)
         (account)
         (comment)
         (feed_type)
         (shared_by)
         (boards)
         (tags)
         (first_shared_by)
         (shares)
         (feed_time) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::feed_object, node::chain::feed_index );

FC_REFLECT( node::chain::blog_object, 
         (id)
         (account)
         (board)
         (tag)
         (comment)
         (shared_by)
         (blog_type)
         (first_shared_by)
         (shares)
         (blog_time) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::blog_object, node::chain::blog_index );

FC_REFLECT( node::chain::comment_vote_object,
         (id)
         (voter)
         (comment)
         (weight)
         (max_weight)
         (reward)
         (vote_percent)
         (last_update)
         (created)
         (num_changes)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_vote_object, node::chain::comment_vote_index );

FC_REFLECT( node::chain::comment_view_object,
         (id)
         (viewer)
         (comment)
         (interface)
         (supernode)
         (reward)
         (weight)
         (max_weight)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_view_object, node::chain::comment_view_index );

FC_REFLECT( node::chain::comment_share_object,
         (id)
         (sharer)
         (comment)
         (interface)
         (reward)
         (weight)
         (max_weight)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_share_object, node::chain::comment_share_index );

FC_REFLECT( node::chain::moderation_tag_object,
         (id)
         (moderator)
         (comment)
         (board)
         (tags)
         (rating)
         (details)
         (interface)
         (filter)
         (last_update)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::moderation_tag_object, node::chain::moderation_tag_index );

FC_REFLECT( node::chain::message_object,
         (id)
         (sender)
         (recipient)
         (sender_public_key)
         (recipient_public_key)
         (message)
         (json)
         (uuid)
         (last_update)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::message_object, node::chain::message_index );

FC_REFLECT( node::chain::comment_metrics_object,
         (id)
         (recent_post_count)
         (recent_vote_power)
         (recent_view_power)
         (recent_share_power)
         (recent_comment_power)
         (average_vote_power)
         (average_view_power)
         (average_share_power)
         (average_comment_power)
         (median_vote_power)
         (median_view_power)
         (median_share_power)
         (median_comment_power)
         (recent_vote_count)
         (recent_view_count)
         (recent_share_count)
         (recent_comment_count)
         (average_vote_count)
         (average_view_count)
         (average_share_count)
         (average_comment_count)
         (median_vote_count)
         (median_view_count)
         (median_share_count)
         (median_comment_count)
         (vote_view_ratio)
         (vote_share_ratio)
         (vote_comment_ratio)
         (last_update)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_metrics_object, node::chain::comment_metrics_index );