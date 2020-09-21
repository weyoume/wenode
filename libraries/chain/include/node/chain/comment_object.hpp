#pragma once
#include <fc/fixed_string.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/shared_authority.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <numeric>

namespace node { namespace chain {

   
   /**
    * Comment containing content created by an author account.
    * 
    * Referenced by a unique permlink.
    * Uses one of several content type variants.
    * 
    * COMMENT TYPES
    * 
    * TEXT_POST - A post containing a maxmium of 300 characters of text.
    * IMAGE_POST - A post containing an IPFS media file of an image, and up to 1000 characters of description text.
    * GIF_POST - A post containing an IPFS media file of a GIF, and up to 1000 characters of description text.
    * VIDEO_POST - A post containing a title, an IPFS media file or bittorrent magent link of a video, and up to 1000 characters of description text.
    * LINK_POST - A post containing a URL link, link title, and up to 1000 characters of description text.
    * ARTICLE_POST - A post containing a title, a header image, and an unlimited amount of body text with embedded images.
    * AUDIO_POST - A post containing a title, an IPFS link to an audio file, and up to 1000 characters of description text.
    * FILE_POST - A post containing a title, either an IPFS link to a file, or a magnet link to a bittorrent swarm for a file, and up to 1000 characters of description text.
    * LIVESTREAM_POST - A post containing a title, a link to a livestreaming video, and up to 1000 characters of description text.
    */
   class comment_object : public object < comment_object_type, comment_object >
   {
      comment_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         comment_object( Constructor&& c, allocator< Allocator > a ) :
            permlink(a), 
            parent_permlink(a), 
            title(a), 
            body(a), 
            body_private(a),
            url(a),
            url_private(a), 
            ipfs(a),
            ipfs_private(a),
            magnet(a),
            magnet_private(a),
            json(a),
            json_private(a),
            language(a),
            category(a) 
            {
               c( *this );
            }

         id_type                           id;

         account_name_type                 author;                              ///< Name of the account that created the post.

         shared_string                     permlink;                            ///< Unique identifing string for the post.

         account_name_type                 parent_author;                       ///< Account that created the post this post is replying to, empty if root post.

         shared_string                     parent_permlink;                     ///< permlink of the post this post is replying to, empty if root post.

         shared_string                     title;                               ///< String containing title text.

         shared_string                     body;                                ///< Public text for display when the post is opened.

         shared_string                     body_private;                        ///< Encrypted and private text for display when the post is opened and decrypted.

         shared_string                     url;                                 ///< Plaintext URL for the post to link to. For a livestream post, should link to the streaming server embed.

         shared_string                     url_private;                         ///< Encrypted and private ciphertext URL for the post to link to. For a livestream post, should link to the streaming server embed.

         shared_string                     ipfs;                                ///< Public IPFS file hash: images, videos, files. Used as thumbnail for image posts.

         shared_string                     ipfs_private;                        ///< Private and encrypted IPFS file hash: images, videos, files.

         shared_string                     magnet;                              ///< Bittorrent magnet links to torrent file swarms: videos, files.

         shared_string                     magnet_private;                      ///< Bittorrent magnet links to Private and Encrypted torrent file swarms: videos, files.

         shared_string                     json;                                ///< JSON string of additional interface specific data relating to the post.

         shared_string                     json_private;                        ///< Private and Encrypted JSON string of additional interface specific data relating to the post.

         shared_string                     language;                            ///< String containing a two letter language code that the post is broadcast in.

         public_key_type                   public_key;                          ///< The public key used to encrypt the post, holders of the private key may decrypt. 

         community_name_type               community;                           ///< The name of the community to which the post is uploaded to. Null string if no community.

         flat_set< tag_name_type >         tags;                                ///< Set of string tags for sorting the post by.

         flat_set< account_name_type >     collaborating_authors;               ///< Accounts that are able to edit the post as shared authors.

         flat_set< account_name_type >     supernodes;                          ///< Name of the Supernodes that the IPFS file is hosted with. Each should additionally hold the private key.
         
         account_name_type                 interface;                           ///< Name of the interface account that was used to broadcast the transaction and view the post.

         double                            latitude;                            ///< Latitude co-ordinates of the comment.

         double                            longitude;                           ///< Longitude co-ordinates of the comment.

         asset                             comment_price;                       ///< The price paid to create a comment.

         asset                             premium_price;                       ///< The price paid to unlock the post's premium encryption.
         
         uint16_t                          rating = 1;                          ///< User nominated rating [1-10] as to the maturity of the content, and display sensitivity.

         id_type                           root_comment;                        ///< The root post that the comment is an ancestor of.

         post_format_type                  post_type;                           ///< The type of post that is being created, image, text, article, video etc.
         
         feed_reach_type                   reach;                               ///< The reach of the post across followers, connections, friends and companions.

         connection_tier_type              reply_connection;                    ///< Replies to the comment must be connected to the author to at least this level.

         shared_string                     category;                            ///< Permlink of root post that this comment is applied to.

         flat_map< account_name_type, flat_map< asset_symbol_type, asset > >  payments_received;    ///< Map of all transfers received that referenced this comment. 

         flat_set< beneficiary_route_type > beneficiaries;                      ///< Vector of beneficiary routes that receive a content reward distribution.
         
         time_point                        last_updated;                        ///< The time the comment was last edited by the author

         time_point                        created;                             ///< Time that the comment was created.

         time_point                        active;                              ///< The last time this post was replied to.

         time_point                        last_payout;                         ///< The last time that the post received a content reward payout

         share_type                        author_reputation = 0;               ///< Used to measure author lifetime rewards, relative to other accounts.

         uint16_t                          depth = 0;                           ///< Used to track max nested depth

         uint32_t                          children = 0;                        ///< The total number of children, grandchildren, posts with this as root comment.

         int32_t                           net_votes = 0;                       ///< The amount of upvotes, minus downvotes on the post.

         uint32_t                          view_count = 0;                      ///< The amount of views on the post.

         uint32_t                          share_count = 0;                     ///< The amount of shares on the post.

         share_type                        net_reward = 0;                      ///< Net reward is the sum of all vote, view, share and comment power.

         share_type                        vote_power = 0;                      ///< Sum of weighted voting power from votes.

         share_type                        view_power = 0;                      ///< Sum of weighted voting power from viewers.

         share_type                        share_power = 0;                     ///< Sum of weighted voting power from shares.

         share_type                        comment_power = 0;                   ///< Sum of weighted voting power from comments.
 
         time_point                        cashout_time;                        ///< Next scheduled time to receive a content reward cashout.

         uint32_t                          cashouts_received = 0;               ///< Number of times that the comment has received content rewards.

         uint128_t                         total_vote_weight = 0;               ///< The total weight of votes, used to calculate pro-rata share of curation payouts.

         uint128_t                         total_view_weight = 0;               ///< The total weight of views, used to calculate pro-rata share of curation payouts.

         uint128_t                         total_share_weight = 0;              ///< The total weight of shares, used to calculate pro-rata share of curation payouts.

         uint128_t                         total_comment_weight = 0;            ///< The total weight of comments, used to calculate pro-rata share of curation payouts.
   
         asset                             total_payout_value = asset( 0, SYMBOL_USD );        ///< The total payout this comment has received over time, measured in USD.

         asset                             curator_payout_value = asset( 0, SYMBOL_USD );      ///< The total payout this comment paid to curators, measured in USD.

         asset                             beneficiary_payout_value = asset( 0, SYMBOL_USD );  ///< The total payout this comment paid to beneficiaries, measured in USD.

         asset                             content_rewards = asset( 0, SYMBOL_COIN );          ///< The Total amount of content rewards that the post has earned, measured in reward currency. 

         uint32_t                          percent_liquid = PERCENT_100;        ///< The percentage of content rewards that should be paid in liquid reward balance. 0 to stake all rewards.

         share_type                        reward = 0;                          ///< The amount of reward_curve this comment is responsible for in its root post.

         uint128_t                         weight = 0;                          ///< Used to define the comment curation reward this comment receives.

         uint128_t                         max_weight = 0;                      ///< Used to define relative contribution of this comment to rewards.

         asset                             max_accepted_payout = MAX_ACCEPTED_PAYOUT;  ///< USD value of the maximum payout this post will receive.

         asset_symbol_type                 reward_currency = SYMBOL_COIN;       ///< The currency asset that the post can earn content rewards in.

         comment_reward_curve              reward_curve = comment_reward_curve();  ///< The components of the reward curve determined at the time of creating the post.

         bool                              allow_replies = true;                ///< allows a post to receive replies.

         bool                              allow_votes = true;                  ///< allows a post to receive votes.

         bool                              allow_views = true;                  ///< allows a post to receive views.

         bool                              allow_shares = true;                 ///< allows a post to receive shares.

         bool                              allow_curation_rewards = true;       ///< Allows a post to distribute curation rewards.

         bool                              root = true;                         ///< True if post is a root post. 

         bool                              deleted = false;                     ///< True if author selects to remove from display in all interfaces, removed from API node distribution, cannot be interacted with.

         bool                              is_encrypted() const                 ///< True if the post is encrypted. False if it is plaintext.
         {
            return public_key != public_key_type();
         };

         bool                              comment_paid( account_name_type name ) const    ///< Return true if user has paid comment price
         {
            if( comment_price.amount > 0 )
            {
               if( name == author )
               {
                  return true;
               }
               else if( payments_received.find( name ) != payments_received.end() )
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

         bool                              is_collaborating_author( account_name_type name )const
         {
            return std::find( collaborating_authors.begin(), collaborating_authors.end(), name ) != collaborating_authors.end();
         }

         bool                              is_supernode( account_name_type name )const
         {
            return std::find( supernodes.begin(), supernodes.end(), name ) != supernodes.end();
         }

         /**
          * Prints the Content of the post
          */
         string comment_string()const
         {
            string result;
            result += ( "Post: >>" + fc::to_string( id._id ) + "\n" );
            result += ( "Author: @" + author + "/" + to_string( permlink ) + "\n" );
            result += ( "Community: " + community + "\n" );
            result += ( "Title: " + to_string( title ) + "\n" );
            result += ( to_string( body ) + "\n" );
            result += ( "tags: " );
            for( auto t : tags )
            {
               result += ( "#" + t + " " );
            }
            result += ( "\n" );
            result += ( "ipfs: " + to_string( ipfs ) + "\n" );
            result += ( "magnet: " + to_string( magnet ) + "\n" );
            result += ( "url: " + to_string( url ) + "\n" );
            return result;
         }
   };
   

   /**
    * Feed holds the posts that have been posted or shared by followed or connected accounts.
    * 
    * Also holds posts from communities that they follow, or the tags that they follow. 
    * Operates like an account's inbox.
    */
   class comment_feed_object : public object< comment_feed_object_type, comment_feed_object >
   {
      comment_feed_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         comment_feed_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                     id;

         account_name_type                           account;               ///< Account that should see comment in their feed.

         comment_id_type                             comment;               ///< ID of comment being shared

         feed_reach_type                             feed_type;             ///< Type of feed, follow, connection, community, tag etc. 

         flat_map< account_name_type, time_point >   shared_by;             ///< Map of the times that accounts that have shared the comment.

         flat_map< community_name_type, flat_map< account_name_type, time_point > >   communities;  ///< Map of all communities that the comment has been shared with

         flat_map< tag_name_type, flat_map< account_name_type, time_point > >     tags;    ///< Map of all tags that the comment has been shared with.

         account_name_type                           first_shared_by;       ///< First account that shared the comment with account. 

         uint32_t                                    shares;                ///< Number of accounts that have shared the comment with account.

         time_point                                  feed_time;             ///< Time that the comment was added or last shared with account. 
   };


   /**
    * Blog objects hold posts that are shared or posted by a particular account or to a tag, or a community.
    * Operates like a posting outbox for an account.  
    */
   class comment_blog_object : public object< comment_blog_object_type, comment_blog_object >
   {
      comment_blog_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         comment_blog_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                   id;

         account_name_type         account;               ///< Blog or sharing account for account type blogs, null for other types.

         community_name_type       community;             ///< Community posted or shared to for community type blogs.

         tag_name_type             tag;                   ///< Tag posted or shared to for tag type blogs.            

         comment_id_type           comment;               ///< Comment ID.

         flat_map< account_name_type, time_point >   shared_by;     ///< Map of the times that accounts that have shared the comment in the blog.

         blog_reach_type           blog_type;             ///< Account, Community, or Tag blog.

         account_name_type         first_shared_by;       ///< First account that shared the comment with the account, community or tag. 

         uint32_t                  shares;                ///< Number of accounts that have shared the comment with account, community or tag.

         time_point                blog_time;             ///< Latest time that the comment was shared on the account, community or tag.
   };


   /**
    * Comment Votes form the content cuation reward ranking.
    * 
    * The amount of votes is a component of determining 
    * the amount of content rewards that an author earns.
    * 
    * Accounts that vote on content earn a share of the 
    * content rewards of that post.
    * 
    * Voting power is moderately rate limited, such that an account
    * recharges votes at a rate of 20 per day
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

         account_name_type       voter;              ///< Name of the account that voted for the comment.

         comment_id_type         comment;            ///< ID of the comment.

         account_name_type       interface;          ///< Name of the interface account that was used to broadcast the transaction and view the post.

         uint128_t               weight;             ///< Used to define the curation reward this vote receives. Decays with time and additional votes.

         uint128_t               max_weight;         ///< Used to define relative contribution of this comment to rewards.

         share_type              reward;             ///< The amount of reward_curve this vote is responsible for

         int16_t                 vote_percent;       ///< The percent weight of the vote

         time_point              last_updated;       ///< The time of the last update of the vote.

         time_point              created;            ///< Time the vote was created.

         int8_t                  num_changes;        ///< Number of times the vote has been adjusted.
   };


   /**
    * Comment Views form the content cuation reward ranking.
    * 
    * The amount of views is a component of determining 
    * the amount of content rewards that an author earns.
    * 
    * Accounts that view content earn a share of the 
    * content rewards of that post.
    * 
    * View Transactions additionally determine the delivery of
    * advertising transactions to the Interface that is listed, 
    * and determine the Supernode rewards for the Supernode that is listed.
    * 
    * Viewing power is slightly rate limited, such that an account
    * recharges views at a rate of 100 per day
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

         account_name_type       viewer;             ///< Name of the viewing account.

         comment_id_type         comment;            ///< ID of the comment.

         account_name_type       interface;          ///< Name of the interface account that was used to broadcast the transaction and view the post. 

         account_name_type       supernode;          ///< Name of the supernode account that served the IPFS file data in the post.

         share_type              reward;             ///< The amount of voting power this view contributed.

         uint128_t               weight;             ///< The curation reward weight of the view. Decays with time and additional views.

         uint128_t               max_weight;         ///< Used to define relative contribution of this view to rewards.

         time_point              created;            ///< Time the view was created.
   };

   /**
    * Comment Shares form the content cuation reward ranking.
    * 
    * The amount of shares is a component of determining 
    * the amount of content rewards that an author earns.
    * 
    * Accounts that share content earn a share of the 
    * content rewards of that post.
    * 
    * Share Transactions add the post to the blog of the account
    * and boost the visibility of the post, so that
    * all accounts that follow the sharing account see it in thier feeds.
    * 
    * Sharing power is highly rate limited, such that an account
    * recharges shares at a rate of 5 per day
    */
   class comment_share_object : public object< comment_share_object_type, comment_share_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         comment_share_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       sharer;             ///< Name of the sharing account.

         comment_id_type         comment;            ///< ID of the comment.

         account_name_type       interface;          ///< Name of the interface account that was used to broadcast the transaction and view the post. 

         share_type              reward;             ///< The amount of voting power this share contributed.

         uint128_t               weight;             ///< The curation reward weight of the share.

         uint128_t               max_weight;         ///< Used to define relative contribution of this share to rewards.

         time_point              created;            ///< Time the share was created.
   };


   /**
    * Used by community moderators and governance addresses to apply Moderation Tags.
    * 
    * Moderation Tags detail the type of content that they find on the network,
    * and may apply filtering policies to posts that are in opposition to the 
    * moderation policies of the community rules, 
    * or the governance addresses content standards.
    * 
    * Moderation Tags apply a rating from 1 to 10 assesing the subjective nature 
    * of the severity of the content
    * for determining whether it is NSFW (5 and above)
    * 
    * Rating values are collected on posts, and the API finds the median rating
    * from all the Governance account and moderators that the User is subscribed to.
    */
   class comment_moderation_object : public object < comment_moderation_object_type, comment_moderation_object >
   {
      comment_moderation_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         comment_moderation_object( Constructor&& c, allocator< Allocator > a ) :
            details(a)
            {
               c( *this );
            }

         id_type                             id;

         account_name_type                   moderator;        ///< Name of the moderator or goverance account that created the comment tag.

         comment_id_type                     comment;          ///< ID of the comment.

         community_name_type                 community;        ///< The name of the community to which the post is uploaded to.

         flat_set< tag_name_type >           tags;             ///< Set of string tags for sorting the post by.

         uint16_t                            rating;           ///< Moderator updated rating (1-10) as to the maturity of the content, and display sensitivity. 

         shared_string                       details;          ///< Explaination as to what rule the post is in contravention of and why it was tagged.

         account_name_type                   interface;        ///< Interface account used for the transaction.

         bool                                filter;           ///< True if the post should be filtered by the community or governance address subscribers. 

         time_point                          last_updated;     ///< Time the comment tag was last edited by the author.

         time_point                          created;          ///< Time that the comment tag was created.
   };



   /**
    * Tracks the Comment Metrics for post sorting and activity reward validation.
    * 
    * Gets updated once per hour, with current average and median statistics of all posts on the network.
    * Used for tag object sorting equalization formulae.
    * 
    * Provides consensus level statistics, and determines the minimum interaction for a comment
    * to be acceptable for an activity reward.
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

         uint128_t               recent_post_count;        ///< Number of root posts in the last 30 days, rolling average.

         uint128_t               recent_vote_power;        ///< Sum of vote power for all posts in last 30 days

         uint128_t               recent_view_power;        ///< Sum of view power for all posts in last 30 days

         uint128_t               recent_share_power;       ///< Sum of share power for all posts in last 30 days

         uint128_t               recent_comment_power;     ///< Sum of comment power for all posts in last 30 days

         uint128_t               average_vote_power;       ///< Recent vote power / recent post count

         uint128_t               average_view_power;       ///< Recent view power / recent post count

         uint128_t               average_share_power;      ///< Recent share power / recent post count

         uint128_t               average_comment_power;    ///< Recent comment power / recent post count

         uint128_t               median_vote_power;        ///< Vote power of the post ranked for vote power at position recent_post_count / 2

         uint128_t               median_view_power;        ///< View power of the post ranked for view power at position recent_post_count / 2

         uint128_t               median_share_power;       ///< Share power of the post ranked for share power at position recent_post_count / 2

         uint128_t               median_comment_power;     ///< Comment power of the post ranked for comment power at position recent_post_count / 2

         uint128_t               recent_vote_count;        ///< Sum of net_votes for all posts in last 30 days

         uint128_t               recent_view_count;        ///< Sum of view_count for all posts in last 30 days

         uint128_t               recent_share_count;       ///< Sum of share_count for all posts in last 30 days

         uint128_t               recent_comment_count;     ///< Sum of children for all posts in last 30 days

         uint128_t               average_vote_count;       ///< Recent vote count / recent post count

         uint128_t               average_view_count;       ///< Recent view count / recent post count

         uint128_t               average_share_count;      ///< Recent share count / recent post count

         uint128_t               average_comment_count;    ///< Recent comment count / recent post count

         uint128_t               median_vote_count;        ///< Vote count of the post ranked for vote count at position recent_post_count / 2

         uint128_t               median_view_count;        ///< View count of the post ranked for view count at position recent_post_count / 2

         uint128_t               median_share_count;       ///< Share count of the post ranked for share count at position recent_post_count / 2

         uint128_t               median_comment_count;     ///< Comment count of the post ranked for comment count at position recent_post_count / 2

         double                  vote_view_ratio;          ///< recent view power / recent vote power ratio

         double                  vote_share_ratio;         ///< recent share power / recent vote power ratio

         double                  vote_comment_ratio;       ///< recent comment power / recent vote power ration

         time_point              last_updated;                 ///< Time of last metrics update
   };


   /** 
    * Encrypted Private Direct messages are sent between users
    * 
    * Process for creating encrypted messages:
    * 
    * 1 - Plaintext is encrypted with the private key of the sender.
    * 2 - This is then encrypted with the public key of the recipient.
    * 3 - The ciphertext is included in the message field.
    * 4 - Each message generates a UUIDv4 for local storage reference.
    */
   class message_object : public object< message_object_type, message_object>
   {
      public:
         template< typename Constructor, typename Allocator >
         message_object( Constructor&& c, allocator< Allocator > a ) :
            message(a), 
            json(a), 
            uuid(a)
            {
               c( *this );
            }

         id_type                 id;

         account_name_type       sender;                   ///< Name of the message sender.

         account_name_type       recipient;                ///< Name of the intended message recipient.

         public_key_type         sender_public_key;        ///< Public secure key of the sender.

         public_key_type         recipient_public_key;     ///< Public secure key of the recipient.

         shared_string           message;                  ///< Encrypted private message ciphertext.

         shared_string           json;                     ///< Encrypted Message metadata.

         shared_string           uuid;                     ///< uuidv4 uniquely identifying the message for local storage.

         time_point              last_updated;             ///< Time the message was last changed, used to reload encrypted message storage.

         time_point              created;                  ///< Time the message was sent.
   };



   /**
    * Lists contain a curated group of accounts, comments, communities and other objects.
    * 
    * Used to collect a group of objects for reference and browsing.
    * 
    * By default, applications should create:
    * 
    * - A list of posts called "Bookmarks"
    * - A list of products and auctions called "Wishlist"
    * - A list of accounts called "Favourites"
    * - A list of communities called "Starred"
    * - A list of assets called "Watchlist"
    * 
    * And use the application to add new objects to these lists as the user interacts.
    */
   class list_object : public object < list_object_type, list_object >
   {
      list_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         list_object( Constructor&& c, allocator< Allocator > a ) :
            list_id(a),
            name(a)
            {
               c( *this );
            }

         id_type                                     id;

         account_name_type                           creator;          ///< Name of the account that created the list.

         shared_string                               list_id;          ///< uuidv4 referring to the list.
         
         shared_string                               name;             ///< Name of the list, unique for each account.

         flat_set< account_id_type >                 accounts;         ///< Account IDs within the list.

         flat_set< comment_id_type >                 comments;         ///< Comment IDs within the list.

         flat_set< community_id_type >               communities;      ///< Community IDs within the list.

         flat_set< asset_id_type >                   assets;           ///< Asset IDs within the list.

         flat_set< product_sale_id_type >            products;         ///< Product IDs within the list.

         flat_set< product_auction_sale_id_type >    auctions;         ///< Auction IDs within the list.

         flat_set< graph_node_id_type >              nodes;            ///< Graph node IDs within the list.

         flat_set< graph_edge_id_type >              edges;            ///< Graph edge IDs within the list.

         flat_set< graph_node_property_id_type >     node_types;       ///< Graph node property IDs within the list.

         flat_set< graph_edge_property_id_type >     edge_types;       ///< Graph edge property IDs within the list.

         time_point                                  last_updated;     ///< Time the list was last edited by the creator.

         time_point                                  created;          ///< Time that the list was created.
   };


   /**
    * Polls enable accounts to vote on a series of options.
    * 
    * Polls have a fixed duration, and determine the winning option.
    */
   class poll_object : public object < poll_object_type, poll_object >
   {
      poll_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         poll_object( Constructor&& c, allocator< Allocator > a ) :
            poll_id(a),
            details(a),
            poll_options( a.get_segment_manager() )
            {
               c( *this );
            }

         id_type                                     id;

         account_name_type                           creator;             ///< Name of the account that created the poll.

         shared_string                               poll_id;             ///< uuidv4 referring to the poll.

         shared_string                               details;             ///< Text describing the question being asked.

         shared_vector< fixed_string_32 >            poll_options;        ///< Available poll voting options.

         time_point                                  completion_time;     ///< Time the poll voting completes.

         time_point                                  last_updated;        ///< Time the poll was last edited by the creator.

         time_point                                  created;             ///< Time that the poll was created.
   };


   /**
    * Poll Vote for a specified option.
    * 
    * Polls have a fixed duration, and determine the winning option.
    */
   class poll_vote_object : public object < poll_vote_object_type, poll_vote_object >
   {
      poll_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         poll_vote_object( Constructor&& c, allocator< Allocator > a ) :
            poll_id(a)
            {
               c( *this );
            }

         id_type                                     id;

         account_name_type                           voter;               ///< Name of the account that created the vote.

         account_name_type                           creator;             ///< Name of the account that created the poll.

         shared_string                               poll_id;             ///< uuidv4 referring to the poll.

         fixed_string_32                             poll_option;         ///< Poll option chosen.

         time_point                                  last_updated;        ///< Time the vote was last edited by the voter.

         time_point                                  created;             ///< Time that the vote was created.
   };


   /**
    * Initiates a purchase of a premium post.
    * 
    * Holds funds for the purchase to be completed 
    * after a view is signed and broadcasted.
    */
   class premium_purchase_object : public object < premium_purchase_object_type, premium_purchase_object >
   {
      premium_purchase_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         premium_purchase_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                     id;

         account_name_type                           account;             ///< Name of the account that created the vote.

         comment_id_type                             comment;             ///< ID of the premium post being purchased.

         asset                                       premium_price;       ///< Amount of asset paid to purchase the post.

         account_name_type                           interface;           ///< Interface account used for the transaction.

         time_point                                  expiration;          ///< Time that the purchase must be completed before expiring.

         time_point                                  last_updated;        ///< Time the vote was last edited by the voter.

         time_point                                  created;             ///< Time that the vote was created.

         bool                                        released = false;    ///< True when the purchase has been matched by at least one premium purchase key.
   };


   /**
    * Initiates a purchase of a premium post.
    * 
    * Holds funds for the purchase to be completed 
    * after a view is signed and broadcasted.
    */
   class premium_purchase_key_object : public object < premium_purchase_key_object_type, premium_purchase_key_object >
   {
      premium_purchase_key_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         premium_purchase_key_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                     id;

         account_name_type                           provider;            ///< Name of the account releasing the premium content. Post author or designated Supernode. 

         account_name_type                           account;             ///< Name of the account purchasing the premium content.

         comment_id_type                             comment;             ///< ID of the Premium Post being purchased.

         encrypted_keypair_type                      encrypted_key;       ///< The private key used to encrypt the premium post, encrypted with the public secure key of the purchaser.

         time_point                                  last_updated;        ///< Time the vote was last edited by the voter.

         time_point                                  created;             ///< Time that the vote was created.
   };



   struct by_currency_cashout_time; // cashout_time
   struct by_permlink; // author, perm
   struct by_root;
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
         ordered_unique< tag< by_permlink >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::author >,
               member< comment_object, shared_string, &comment_object::permlink >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >,
         ordered_unique< tag< by_currency_cashout_time >,
            composite_key< comment_object,
               member< comment_object, asset_symbol_type, &comment_object::reward_currency >,
               member< comment_object, time_point, &comment_object::cashout_time >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< time_point >,
               std::less< comment_id_type >
            >
         >,
         ordered_unique< tag< by_root >,
            composite_key< comment_object,
               member< comment_object, comment_id_type, &comment_object::root_comment >,
               member< comment_object, comment_id_type, &comment_object::id >
            >
         >,
         ordered_unique< tag< by_time >,
            composite_key< comment_object,
               member< comment_object, bool, &comment_object::root >,
               member< comment_object, time_point, &comment_object::created >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< 
               std::less< bool >, 
               std::greater< time_point >, 
               std::less< comment_id_type > 
            >
         >
         // NON_CONSENSUS INDICIES - used by APIs
#ifndef IS_LOW_MEM
         ,
         ordered_unique< tag< by_parent >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::parent_author >,
               member< comment_object, shared_string, &comment_object::parent_permlink >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less, 
               std::less< comment_id_type > 
            >
         >,
         ordered_unique< tag< by_title >,
            composite_key< comment_object,
               member< comment_object, shared_string, &comment_object::title >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< 
               strcmp_less, 
               std::less< comment_id_type > 
            >
         >,
         ordered_unique< tag< by_last_update >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::parent_author >,
               member< comment_object, time_point, &comment_object::last_updated >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< time_point >, 
               std::less< comment_id_type > 
            >
         >,
         ordered_unique< tag< by_author_last_update >,
            composite_key< comment_object,
               member< comment_object, account_name_type, &comment_object::author >,
               member< comment_object, time_point, &comment_object::last_updated >,
               member< comment_object, comment_id_type, &comment_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< time_point >,
               std::less< comment_id_type >
            >
         >
#endif
      >,
      allocator< comment_object >
   > comment_index;


   struct by_new_account_type;
   struct by_old_account_type;
   struct by_new_account;
   struct by_old_account;
   struct by_account_comment_type;
   struct by_comment;


   typedef multi_index_container<
      comment_feed_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_feed_object, comment_feed_id_type, &comment_feed_object::id > >,
         ordered_unique< tag< by_new_account_type >,
            composite_key< comment_feed_object,
               member< comment_feed_object, account_name_type, &comment_feed_object::account >,
               member< comment_feed_object, feed_reach_type, &comment_feed_object::feed_type >,
               member< comment_feed_object, time_point, &comment_feed_object::feed_time >,
               member< comment_feed_object, comment_feed_id_type, &comment_feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< feed_reach_type >, 
               std::greater< time_point >,   // Newest feeds first
               std::less< comment_feed_id_type > 
            >
         >,
         ordered_unique< tag< by_old_account_type >,
            composite_key< comment_feed_object,
               member< comment_feed_object, account_name_type, &comment_feed_object::account >,
               member< comment_feed_object, feed_reach_type, &comment_feed_object::feed_type >,
               member< comment_feed_object, time_point, &comment_feed_object::feed_time >,
               member< comment_feed_object, comment_feed_id_type, &comment_feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< feed_reach_type >, 
               std::less< time_point >,     // Oldest feeds first
               std::less< comment_feed_id_type > 
            >
         >,
         ordered_unique< tag< by_new_account >,
            composite_key< comment_feed_object,
               member< comment_feed_object, account_name_type, &comment_feed_object::account >,
               member< comment_feed_object, time_point, &comment_feed_object::feed_time >,
               member< comment_feed_object, comment_feed_id_type, &comment_feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< time_point >,     // Newest feeds first
               std::less< comment_feed_id_type > 
            >
         >,
         ordered_unique< tag< by_old_account >,
            composite_key< comment_feed_object,
               member< comment_feed_object, account_name_type, &comment_feed_object::account >,
               member< comment_feed_object, time_point, &comment_feed_object::feed_time >,
               member< comment_feed_object, comment_feed_id_type, &comment_feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< time_point >,     // Oldest feeds first
               std::less< comment_feed_id_type > 
            >
         >,
         ordered_unique< tag< by_account_comment_type >,
            composite_key< comment_feed_object,
               member< comment_feed_object, account_name_type, &comment_feed_object::account >,
               member< comment_feed_object, comment_id_type, &comment_feed_object::comment >,
               member< comment_feed_object, feed_reach_type, &comment_feed_object::feed_type >,
               member< comment_feed_object, comment_feed_id_type, &comment_feed_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< comment_id_type >,
               std::less< feed_reach_type >, 
               std::less< comment_feed_id_type > 
            >
         >,
         ordered_unique< tag< by_comment >,
            composite_key< comment_feed_object,
               member< comment_feed_object, comment_id_type, &comment_feed_object::comment >,
               member< comment_feed_object, comment_feed_id_type, &comment_feed_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< comment_feed_id_type > 
            >
         >
      >,
      allocator< comment_feed_object >
   > comment_feed_index;


   struct by_new_account_blog;
   struct by_old_account_blog;
   struct by_new_community_blog;
   struct by_old_community_blog;
   struct by_new_tag_blog;
   struct by_old_tag_blog;
   struct by_comment_account;
   struct by_comment_community;
   struct by_comment_tag;


   typedef multi_index_container<
      comment_blog_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id > >,
         ordered_unique< tag< by_new_account_blog >,
            composite_key< comment_blog_object,
               member< comment_blog_object, account_name_type, &comment_blog_object::account >,
               member< comment_blog_object, time_point, &comment_blog_object::blog_time >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< time_point >, 
               std::less< comment_blog_id_type >
            >
         >,
         ordered_unique< tag< by_old_account_blog >,
            composite_key< comment_blog_object,
               member< comment_blog_object, account_name_type, &comment_blog_object::account >,
               member< comment_blog_object, time_point, &comment_blog_object::blog_time >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< time_point >,
               std::less< comment_blog_id_type >
            >
         >,
         ordered_unique< tag< by_new_community_blog >,
            composite_key< comment_blog_object,
               member< comment_blog_object, community_name_type, &comment_blog_object::community >,
               member< comment_blog_object, time_point, &comment_blog_object::blog_time >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >, 
               std::greater< time_point >, 
               std::less< comment_blog_id_type > 
            >
         >,
         ordered_unique< tag< by_old_community_blog >,
            composite_key< comment_blog_object,
               member< comment_blog_object, community_name_type, &comment_blog_object::community >,
               member< comment_blog_object, time_point, &comment_blog_object::blog_time >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >, 
               std::less< time_point >, 
               std::less< comment_blog_id_type > 
            >
         >,
         ordered_unique< tag< by_new_tag_blog >,
            composite_key< comment_blog_object,
               member< comment_blog_object, tag_name_type, &comment_blog_object::tag >,
               member< comment_blog_object, time_point, &comment_blog_object::blog_time >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< tag_name_type >, 
               std::greater< time_point >, 
               std::less< comment_blog_id_type > 
            >
         >,
         ordered_unique< tag< by_old_tag_blog >,
            composite_key< comment_blog_object,
               member< comment_blog_object, tag_name_type, &comment_blog_object::tag >,
               member< comment_blog_object, time_point, &comment_blog_object::blog_time >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< tag_name_type >, 
               std::less< time_point >, 
               std::less< comment_blog_id_type > 
            >
         >,
         ordered_unique< tag< by_comment_account >,
            composite_key< comment_blog_object,
               member< comment_blog_object, comment_id_type, &comment_blog_object::comment >,
               member< comment_blog_object, account_name_type, &comment_blog_object::account >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< account_name_type >,
               std::less< comment_blog_id_type > 
            >
         >,
         ordered_unique< tag< by_comment_community >,
            composite_key< comment_blog_object,
               member< comment_blog_object, comment_id_type, &comment_blog_object::comment >,
               member< comment_blog_object, community_name_type, &comment_blog_object::community >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< community_name_type >,
               std::less< comment_blog_id_type >  
            >
         >,
         ordered_unique< tag< by_comment_tag >,
            composite_key< comment_blog_object,
               member< comment_blog_object, comment_id_type, &comment_blog_object::comment >,
               member< comment_blog_object, tag_name_type, &comment_blog_object::tag >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::less< tag_name_type >,
               std::less< comment_blog_id_type >  
            >
         >,
         ordered_unique< tag< by_comment >,
            composite_key< comment_blog_object,
               member< comment_blog_object, comment_id_type, &comment_blog_object::comment >,
               member< comment_blog_object, comment_blog_id_type, &comment_blog_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >,
               std::less< comment_blog_id_type >  
            >
         >
      >,
      allocator< comment_blog_object >
   > comment_blog_index;



   struct by_comment_voter;
   struct by_voter_comment;
   struct by_comment_weight_voter;
   struct by_voter_last_update;
   struct by_voter_recent;
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
               member< comment_vote_object, time_point, &comment_vote_object::last_updated>,
               member< comment_vote_object, comment_id_type, &comment_vote_object::comment>
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< time_point >,
               std::less< comment_id_type >
            >
         >,
         ordered_unique< tag< by_voter_recent >,
            composite_key< comment_vote_object,
               member< comment_vote_object, account_name_type, &comment_vote_object::voter>,
               member< comment_vote_object, comment_vote_id_type, &comment_vote_object::id>
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< comment_vote_id_type > 
            >
         >,
         ordered_unique< tag< by_comment_weight_voter >,
            composite_key< comment_vote_object,
               member< comment_vote_object, comment_id_type, &comment_vote_object::comment>,
               member< comment_vote_object, uint128_t, &comment_vote_object::weight>,
               member< comment_vote_object, account_name_type, &comment_vote_object::voter>
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::greater< uint128_t >, 
               std::less< account_name_type > 
            >
         >
      >,
      allocator< comment_vote_object >
   > comment_vote_index;

   struct by_comment_viewer;
   struct by_viewer_comment;
   struct by_viewer_recent;
   struct by_interface_viewer;
   struct by_supernode_viewer;
   struct by_comment_weight_viewer;

   typedef multi_index_container<
      comment_view_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_view_object, comment_view_id_type, &comment_view_object::id > >,
         ordered_unique< tag< by_comment_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, comment_id_type, &comment_view_object::comment >,
               member< comment_view_object, account_name_type, &comment_view_object::viewer >
            >
         >,
         ordered_unique< tag< by_viewer_comment >,
            composite_key< comment_view_object,
               member< comment_view_object, account_name_type, &comment_view_object::viewer >,
               member< comment_view_object, comment_id_type, &comment_view_object::comment >
            >
         >,
         ordered_unique< tag< by_viewer_recent >,
            composite_key< comment_view_object,
               member< comment_view_object, account_name_type, &comment_view_object::viewer >,
               member< comment_view_object, comment_view_id_type, &comment_view_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< comment_view_id_type >
            >
         >,
         ordered_unique< tag< by_interface_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, account_name_type, &comment_view_object::interface >,
               member< comment_view_object, account_name_type, &comment_view_object::viewer >,
               member< comment_view_object, time_point, &comment_view_object::created >,
               member< comment_view_object, comment_id_type, &comment_view_object::comment >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               std::greater< time_point >, 
               std::less< comment_id_type >
            >
         >,
         ordered_unique< tag< by_supernode_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, account_name_type, &comment_view_object::supernode >,
               member< comment_view_object, account_name_type, &comment_view_object::viewer >,
               member< comment_view_object, time_point, &comment_view_object::created >,
               member< comment_view_object, comment_id_type, &comment_view_object::comment >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               std::greater< time_point >, 
               std::less< comment_id_type > 
            >
         >,
         ordered_unique< tag< by_comment_weight_viewer >,
            composite_key< comment_view_object,
               member< comment_view_object, comment_id_type, &comment_view_object::comment >,
               member< comment_view_object, uint128_t, &comment_view_object::weight >,
               member< comment_view_object, account_name_type, &comment_view_object::viewer >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::greater< uint128_t >, 
               std::less< account_name_type > 
            >
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
               member< comment_share_object, comment_id_type, &comment_share_object::comment >,
               member< comment_share_object, account_name_type, &comment_share_object::sharer >
            >
         >,
         ordered_unique< tag< by_sharer_comment >,
            composite_key< comment_share_object,
               member< comment_share_object, account_name_type, &comment_share_object::sharer >,
               member< comment_share_object, comment_id_type, &comment_share_object::comment >
            >
         >,
         ordered_unique< tag< by_comment_weight_sharer >,
            composite_key< comment_share_object,
               member< comment_share_object, comment_id_type, &comment_share_object::comment >,
               member< comment_share_object, uint128_t, &comment_share_object::weight >,
               member< comment_share_object, account_name_type, &comment_share_object::sharer >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::greater< uint128_t >, 
               std::less< account_name_type > 
            >
         >
      >,
      allocator< comment_share_object >
   > comment_share_index;



   struct by_moderator_comment;
   struct by_comment_moderator;
   struct by_comment_updated;

   typedef multi_index_container<
      comment_moderation_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_moderation_object, comment_moderation_id_type, &comment_moderation_object::id > >,
         ordered_unique< tag< by_comment_moderator >,
            composite_key< comment_moderation_object,
               member< comment_moderation_object, comment_id_type, &comment_moderation_object::comment >,
               member< comment_moderation_object, account_name_type, &comment_moderation_object::moderator >
            >
         >,
         ordered_unique< tag< by_comment_updated >,
            composite_key< comment_moderation_object,
               member< comment_moderation_object, comment_id_type, &comment_moderation_object::comment >,
               member< comment_moderation_object, time_point, &comment_moderation_object::last_updated >,
               member< comment_moderation_object, comment_moderation_id_type , &comment_moderation_object::id >
            >,
            composite_key_compare< 
               std::less< comment_id_type >, 
               std::greater< time_point >, 
               std::less< comment_moderation_id_type > 
            >
         >,
         ordered_unique< tag< by_moderator_comment >,
            composite_key< comment_moderation_object,
               member< comment_moderation_object, account_name_type, &comment_moderation_object::moderator >,
               member< comment_moderation_object, comment_id_type, &comment_moderation_object::comment >
            >
         >
      >,
      allocator< comment_moderation_object >
   > comment_moderation_index;


   typedef multi_index_container<
      comment_metrics_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< comment_metrics_object, comment_metrics_id_type, &comment_metrics_object::id > >
      >,
      allocator< comment_metrics_object >
   > comment_metrics_index;


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
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               std::less< message_id_type > 
            >
         >,
         ordered_unique< tag< by_sender_uuid >,
            composite_key< message_object,
               member< message_object, account_name_type, &message_object::sender >,
               member< message_object, shared_string, &message_object::uuid >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >,
         ordered_unique< tag< by_account_inbox >,
            composite_key< message_object,
               member< message_object, account_name_type, &message_object::recipient >,
               member< message_object, time_point, &message_object::created >,
               member< message_object, message_id_type, &message_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< time_point >, 
               std::less< message_id_type > 
            >
         >,
         ordered_unique< tag< by_account_outbox >,
            composite_key< message_object,
               member< message_object, account_name_type, &message_object::sender >,
               member< message_object, time_point, &message_object::created >,
               member< message_object, message_id_type, &message_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< time_point >, 
               std::less< message_id_type > 
            >
         >
      >,
      allocator< message_object >
   > message_index;

   struct by_list_id;

   typedef multi_index_container<
      list_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< list_object, list_id_type, &list_object::id > >,
         ordered_unique< tag< by_list_id >,
            composite_key< list_object,
               member< list_object, account_name_type, &list_object::creator >,
               member< list_object, shared_string, &list_object::list_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >
      >,
      allocator< list_object >
   > list_index;


   struct by_poll_id;


   typedef multi_index_container<
      poll_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< poll_object, poll_id_type, &poll_object::id > >,
         ordered_unique< tag< by_poll_id >,
            composite_key< poll_object,
               member< poll_object, account_name_type, &poll_object::creator >,
               member< poll_object, shared_string, &poll_object::poll_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >
      >,
      allocator< poll_object >
   > poll_index;


   struct by_voter_creator_poll_id;
   struct by_expiration;


   typedef multi_index_container<
      poll_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< poll_vote_object, poll_vote_id_type, &poll_vote_object::id > >,
         ordered_unique< tag< by_voter_creator_poll_id >,
            composite_key< poll_vote_object,
               member< poll_vote_object, account_name_type, &poll_vote_object::voter >,
               member< poll_vote_object, account_name_type, &poll_vote_object::creator >,
               member< poll_vote_object, shared_string, &poll_vote_object::poll_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_poll_id >,
            composite_key< poll_vote_object,
               member< poll_vote_object, account_name_type, &poll_vote_object::creator >,
               member< poll_vote_object, shared_string, &poll_vote_object::poll_id >,
               member< poll_vote_object, poll_vote_id_type, &poll_vote_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less,
               std::less< poll_vote_id_type >
            >
         >
      >,
      allocator< poll_vote_object >
   > poll_vote_index;


   struct by_account_comment;


   typedef multi_index_container<
      premium_purchase_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< premium_purchase_object, premium_purchase_id_type, &premium_purchase_object::id > >,
         ordered_non_unique< tag< by_expiration >, member< premium_purchase_object, time_point, &premium_purchase_object::expiration > >,
         ordered_unique< tag< by_account_comment >,
            composite_key< premium_purchase_object,
               member< premium_purchase_object, account_name_type, &premium_purchase_object::account >,
               member< premium_purchase_object, comment_id_type, &premium_purchase_object::comment >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< comment_id_type >
            >
         >
      >,
      allocator< premium_purchase_object >
   > premium_purchase_index;


   struct by_provider_account_comment;
   struct by_comment;
   struct by_account;


   typedef multi_index_container<
      premium_purchase_key_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< premium_purchase_key_object, premium_purchase_key_id_type, &premium_purchase_key_object::id > >,
         ordered_unique< tag< by_provider_account_comment >,
            composite_key< premium_purchase_key_object,
               member< premium_purchase_key_object, account_name_type, &premium_purchase_key_object::provider >,
               member< premium_purchase_key_object, account_name_type, &premium_purchase_key_object::account >,
               member< premium_purchase_key_object, comment_id_type, &premium_purchase_key_object::comment >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >,
               std::less< comment_id_type >
            >
         >,
         ordered_unique< tag< by_comment >,
            composite_key< premium_purchase_key_object,
               member< premium_purchase_key_object, comment_id_type, &premium_purchase_key_object::comment >,
               member< premium_purchase_key_object, premium_purchase_key_id_type, &premium_purchase_key_object::id >
            >,
            composite_key_compare<
               std::less< comment_id_type >,
               std::less< premium_purchase_key_id_type >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< premium_purchase_key_object,
               member< premium_purchase_key_object, account_name_type, &premium_purchase_key_object::account >,
               member< premium_purchase_key_object, premium_purchase_key_id_type, &premium_purchase_key_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< premium_purchase_key_id_type >
            >
         >
      >,
      allocator< premium_purchase_key_object >
   > premium_purchase_key_index;


} } // node::chain

FC_REFLECT( node::chain::comment_object,
         (id)
         (author)
         (permlink)
         (parent_author)
         (parent_permlink)
         (title)
         (body)
         (body_private)
         (url)
         (url_private)
         (ipfs)
         (ipfs_private)
         (magnet)
         (magnet_private)
         (json)
         (json_private)
         (language)
         (public_key)
         (community)
         (tags)
         (collaborating_authors)
         (supernodes)
         (interface)
         (latitude)
         (longitude)
         (comment_price)
         (premium_price)
         (rating)
         (root_comment)
         (post_type)
         (reach)
         (reply_connection)
         (category)
         (payments_received)
         (beneficiaries)
         (last_updated)
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
         (reward_currency)
         (reward_curve)
         (allow_replies)
         (allow_votes)
         (allow_views)
         (allow_shares)
         (allow_curation_rewards)
         (root)
         (deleted)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_object, node::chain::comment_index );

FC_REFLECT( node::chain::comment_feed_object, 
         (id)
         (account)
         (comment)
         (feed_type)
         (shared_by)
         (communities)
         (tags)
         (first_shared_by)
         (shares)
         (feed_time) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_feed_object, node::chain::comment_feed_index );

FC_REFLECT( node::chain::comment_blog_object, 
         (id)
         (account)
         (community)
         (tag)
         (comment)
         (shared_by)
         (blog_type)
         (first_shared_by)
         (shares)
         (blog_time) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_blog_object, node::chain::comment_blog_index );

FC_REFLECT( node::chain::comment_vote_object,
         (id)
         (voter)
         (comment)
         (interface)
         (weight)
         (max_weight)
         (reward)
         (vote_percent)
         (last_updated)
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

FC_REFLECT( node::chain::comment_moderation_object,
         (id)
         (moderator)
         (comment)
         (community)
         (tags)
         (rating)
         (details)
         (interface)
         (filter)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_moderation_object, node::chain::comment_moderation_index );

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
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::comment_metrics_object, node::chain::comment_metrics_index );

FC_REFLECT( node::chain::message_object,
         (id)
         (sender)
         (recipient)
         (sender_public_key)
         (recipient_public_key)
         (message)
         (json)
         (uuid)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::message_object, node::chain::message_index );

FC_REFLECT( node::chain::list_object,
         (id)
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
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::list_object, node::chain::list_index );

FC_REFLECT( node::chain::poll_object,
         (id)
         (creator)
         (poll_id)
         (details)
         (poll_options)
         (completion_time)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::poll_object, node::chain::poll_index );

FC_REFLECT( node::chain::poll_vote_object,
         (id)
         (voter)
         (creator)
         (poll_id)
         (poll_option)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::poll_vote_object, node::chain::poll_vote_index );

FC_REFLECT( node::chain::premium_purchase_object,
         (id)
         (account)
         (comment)
         (premium_price)
         (interface)
         (expiration)
         (last_updated)
         (created)
         (released)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::premium_purchase_object, node::chain::premium_purchase_index );

FC_REFLECT( node::chain::premium_purchase_key_object,
         (id)
         (provider)
         (account)
         (comment)
         (encrypted_key)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::premium_purchase_key_object, node::chain::premium_purchase_key_index );