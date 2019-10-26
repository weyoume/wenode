#pragma once

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/chain/witness_objects.hpp>

#include <boost/multi_index/composite_key.hpp>


namespace node { namespace chain {

   using protocol::beneficiary_route_type;

   struct strcmp_less
   {
      bool operator()( const shared_string& a, const shared_string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      bool operator()( const shared_string& a, const string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      bool operator()( const string& a, const shared_string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      private:
         inline bool less( const char* a, const char* b )const
         {
            return std::strcmp( a, b ) < 0;
         }
   };

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

         post_types                     post_type;                    // The type of post that is being created, image, text, article, video etc. 

         board_name_type                board;                        // The name of the board to which the post is uploaded to.

         flat_set< shared_string >      tags;                         // Set of string tags for sorting the post by

         shared_string                  body;                         // String containing text for display when the post is opened.

         shared_string                  ipfs;                         // String containing a display image or video file as an IPFS file hash.

         shared_string                  magnet;                       // String containing a bittorrent magnet link to a file swarm.

         connection_types               privacy;                      // Type of privacy setting for encryption levels to connections.

         account_name_type              interface;                    // Name of the interface account that was used to broadcast the transaction and view the post.

         rating_types                   rating;                       // User nominated rating as to the maturity of the content, and display sensitivity. 

         shared_string                  language;                     // String containing a two letter language code that the post is broadcast in.

         id_type                        root_comment;                 // The root post that the comment is an ancestor of. 

         account_name_type              parent_author;                // Account that created the post this post is replying to, empty if root post. 

         shared_string                  parent_permlink;              // permlink of the post this post is replying to, empty if root post. 

         shared_string                  json;                         // IPFS link to file containing - Json metadata of the Title, Link, and additional interface specific data relating to the post.

         shared_string                  category;

         asset                          comment_price;                // The price paid to create a comment

         flat_map<account_name_type, flat_map< asset_symbol_type, asset > >  payments_received;    // Map of all transfers received that referenced this comment. 

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

         uint128_t                      reward = 0;         // The amount of reward_curve this comment is responsible for in its root post.

         uint128_t                      weight = 0;         // Used to define the comment curation reward this comment receives.

         uint128_t                      max_weight = 0;     // Used to define relative contribution of this comment to rewards.

         asset                          max_accepted_payout = asset( BILLION * BLOCKCHAIN_PRECISON, SYMBOL_USD );       // USD value of the maximum payout this post will receive

         uint32_t                       author_reward_percent = AUTHOR_REWARD_PERCENT;

         uint32_t                       vote_reward_percent = VOTE_REWARD_PERCENT;

         uint32_t                       view_reward_percent = VIEW_REWARD_PERCENT;

         uint32_t                       share_reward_percent = SHARE_REWARD_PERCENT;

         uint32_t                       comment_reward_percent = COMMENT_REWARD_PERCENT;

         uint32_t                       storage_reward_percent = STORAGE_REWARD_PERCENT;

         uint32_t                       moderator_reward_percent = MODERATOR_REWARD_PERCENT;

         bool                           allow_replies = true;      // allows a post to recieve replies

         bool                           allow_votes = true;      // allows a post to receive votes

         bool                           allow_views = true;      // allows a post to receive views

         bool                           allow_shares = true;      // allows a post to receive shares

         bool                           allow_curation_rewards = true;      // Allows a post to distribute curation rewards

         bool                           root = true;             // True if post is a root post. 

         bip::vector< beneficiary_route_type, allocator< beneficiary_route_type > > beneficiaries;

         bool                           comment_paid( account_name_type name )    // return true if user has paid comment price
         {
            if( comment_price.amount > 0 )
            {
               if( payments_received[ name ].size() )
               {
                  if( payments_received[ name ][ comment_price.symbol ].amount > 0 )
                  {
                     if( payments_received[ name ][ comment_price.symbol ] >= comment_price )
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
    * accounts that a user follows or is connected with.
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

         id_type                          id;

         account_name_type                account;

         vector< account_name_type >      shared_by;

         feed_types                       feed_type;

         account_name_type                first_shared_by;

         time_point                       first_shared_on;

         comment_id_type                  comment;

         uint32_t                         shares;

         uint32_t                         account_feed_id = 0;
   };

   /**
    * Blog objects hold posts that are shared or posted by a particular account
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

         account_name_type         account;

         comment_id_type           comment;

         time_point                shared_on;

         uint32_t                  blog_feed_id = 0;
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

         uint128_t               reward = 0;         // The amount of reward_curve this vote is responsible for

         int16_t                 vote_percent = 0;   //  The percent weight of the vote

         time_point              last_update;        // The time of the last update of the vote

         time_point              created;            // Time the vote was created

         int8_t                  num_changes = 0;    
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

         uint128_t               reward = 0;         // The amount of voting power this view contributed.

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

         uint128_t               reward = 0;         // The amount of voting power this share contributed.

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

         flat_set< shared_string >      tags;             // Set of string tags for sorting the post by

         rating_types                   rating;           // Moderator updated rating as to the maturity of the content, and display sensitivity. 

         shared_string                  details;          // Explaination as to what rule the post is in contravention of and why it was tagged.

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

         time_point              created;                  // Time the message was sent.

         time_point              last_updated;             // Time the message was last changed, used to reload encrypted message storage.
   };


   /**
    *  This object gets updated once per hour, on the hour.
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

         int128_t                recent_vote_power = 0;

         int128_t                recent_view_power = 0;

         int128_t                recent_share_power = 0;

         int128_t                recent_comment_power = 0;

         int128_t                average_vote_power = 0;

         int128_t                average_view_power = 0;

         int128_t                average_share_power = 0;

         int128_t                average_comment_power = 0;

         int128_t                median_vote_power = 0;

         int128_t                median_view_power = 0;

         int128_t                median_share_power = 0;

         int128_t                median_comment_power = 0;

         uint32_t                recent_vote_count = 0;

         uint32_t                recent_view_count = 0;

         uint32_t                recent_share_count = 0;

         uint32_t                recent_comment_count = 0;

         uint32_t                average_vote_count = 0;

         uint32_t                average_view_count = 0;

         uint32_t                average_share_count = 0;

         uint32_t                average_comment_count = 0;

         uint32_t                median_vote_count = 0;

         uint32_t                median_view_count = 0;

         uint32_t                median_share_count = 0;

         uint32_t                median_comment_count = 0;

         double                  vote_view_ratio = 0;

         double                  vote_share_ratio = 0;

         double                  vote_comment_ratio = 0;

         time_point              last_update;
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

   struct by_feed;
   struct by_old_feed;
   struct by_account;
   struct by_comment;

   typedef multi_index_container<
      feed_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< feed_object, feed_id_type, &feed_object::id > >,
         ordered_unique< tag< by_feed >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, uint32_t, &feed_object::account_feed_id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< uint32_t > >
         >,
         ordered_unique< tag< by_old_feed >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, uint32_t, &feed_object::account_feed_id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< uint32_t > >
         >,
         ordered_unique< tag< by_account >,
            composite_key< feed_object,
               member< feed_object, account_name_type, &feed_object::account >,
               member< feed_object, feed_id_type, &feed_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< feed_id_type > >
         >,
         ordered_unique< tag< by_comment >,
            composite_key< feed_object,
               member< feed_object, comment_id_type, &feed_object::comment >,
               member< feed_object, account_name_type, &feed_object::account >
            >,
            composite_key_compare< std::less< comment_id_type >, std::less< account_name_type > >
         >
      >,
      allocator< feed_object >
   > feed_index;

   struct by_blog;
   struct by_old_blog;

   typedef multi_index_container<
      blog_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< blog_object, blog_id_type, &blog_object::id > >,
         ordered_unique< tag< by_blog >,
            composite_key< blog_object,
               member< blog_object, account_name_type, &blog_object::account >,
               member< blog_object, uint32_t, &blog_object::blog_feed_id >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< uint32_t > >
         >,
         ordered_unique< tag< by_old_blog >,
            composite_key< blog_object,
               member< blog_object, account_name_type, &blog_object::account >,
               member< blog_object, uint32_t, &blog_object::blog_feed_id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< uint32_t > >
         >,
         ordered_unique< tag< by_comment >,
            composite_key< blog_object,
               member< blog_object, comment_id_type, &blog_object::comment >,
               member< blog_object, account_name_type, &blog_object::account >
            >,
            composite_key_compare< std::less< comment_id_type >, std::less< account_name_type > >
         >
      >,
      allocator< blog_object >
   > blog_index;

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

   typedef multi_index_container<
      comment_object,
      indexed_by<
         // CONSENSUS INDICIES - used by evaluators
         ordered_unique< tag< by_id >, member< comment_object, comment_id_type, &comment_object::id > >,
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
         (beneficiary_payout_value)
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

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_object, node::chain::comment_index );

FC_REFLECT( node::chain::comment_vote_object,
         (id)
         (voter)
         (comment)
         (weight)
         (reward)
         (vote_percent)
         (last_update)
         (num_changes)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_vote_object, node::chain::comment_vote_index );


FC_REFLECT( node::chain::feed_object, 
         (id)
         (account)
         (first_reblogged_by)
         (first_reblogged_on)
         (reblogged_by)
         (comment)
         (reblogs)
         (account_feed_id) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::feed_object, node::chain::feed_index );

FC_REFLECT( node::chain::blog_object, 
         (id)
         (account)
         (comment)
         (reblogged_on)
         (blog_feed_id) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::blog_object, node::chain::blog_index );