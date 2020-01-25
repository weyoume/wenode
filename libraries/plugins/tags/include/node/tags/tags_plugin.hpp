#pragma once

#include <node/app/plugin.hpp>
#include <node/chain/database.hpp>
#include <node/chain/comment_object.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <fc/thread/future.hpp>
#include <fc/api.hpp>

namespace node { namespace tags {
using namespace node::chain;
using namespace boost::multi_index;

using node::app::application;

using chainbase::object;
using chainbase::oid;
using chainbase::allocator;

//
// Plugins should #define their SPACE_ID's so plugins with
// conflicting SPACE_ID assignments can be compiled into the
// same binary (by simply re-assigning some of the conflicting #defined
// SPACE_ID's in a build script).
//
// Assignment of SPACE_ID's cannot be done at run-time because
// various template automagic depends on them being known at compile
// time.
//
#ifndef TAG_SPACE_ID
#define TAG_SPACE_ID 5
#endif

#define TAGS_PLUGIN_NAME "tags"

typedef protocol::fixed_string_32 tag_name_type;

// Plugins need to define object type IDs such that they do not conflict
// globally. If each plugin uses the upper 8 bits as a space identifier,
// with 0 being for chain, then the lower 8 bits are free for each plugin
// to define as they see fit.
enum
{
   tag_object_type                            = ( TAG_SPACE_ID << 8 ),
   tag_stats_object_type                      = ( TAG_SPACE_ID << 8 ) + 1,
   account_curation_metrics_object_type       = ( TAG_SPACE_ID << 8 ) + 2,
   account_adjacency_object_type              = ( TAG_SPACE_ID << 8 ) + 3,
   board_adjacency_object_type                = ( TAG_SPACE_ID << 8 ) + 4,
   tag_adjacency_object_type                  = ( TAG_SPACE_ID << 8 ) + 5,
   account_recommendations_object_type        = ( TAG_SPACE_ID << 8 ) + 6,
   peer_stats_object_type                     = ( TAG_SPACE_ID << 8 ) + 7,
   author_tag_stats_object_type               = ( TAG_SPACE_ID << 8 ) + 8
};

namespace detail { 
   class tags_plugin_impl; 
   /**
    * Contains the time preference options for a sorting type
    */
   struct sort_set
   {
      sort_set( 
         const double& active, 
         const double& rapid, 
         const double& standard, 
         const double& top, 
         const double& elite ):
         active(active),
         rapid(rapid),
         standard(standard),
         top(top),
         elite(elite){}
      
      sort_set(){}

      double active = 0;    // Ranking for Active sort, highest time preference with high active time bonus

      double rapid = 0;     // Ranking for Rapid sort, high time preference

      double standard = 0;  // Ranking for Standard sort, regular time preference

      double top = 0;       // Ranking for Top sort, low time preference

      double elite = 0;     // Ranking for Elite sort, lowest time preference with high reputation and equalization bonus
   };


   /**
    * Contains a full suite of rankings for sorting a tag 
    * according to a variety of combainations
    * of votes, views, comments and shares, with
    * multiple time preference options.
    */
   struct sort_options
   {
      sort_options( 
         const sort_set& quality, 
         const sort_set& votes, 
         const sort_set& views, 
         const sort_set& shares, 
         const sort_set& comments, 
         const sort_set& popular, 
         const sort_set& viral, 
         const sort_set& discussion, 
         const sort_set& prominent,
         const sort_set& conversation, 
         const sort_set& discourse ):
         quality(quality),
         votes(votes),
         views(views),
         shares(shares),
         comments(comments),
         popular(popular),
         viral(viral),
         discussion(discussion),
         prominent(prominent),
         conversation(conversation),
         discourse(discourse){}
      
      sort_options(){}

      sort_set quality = sort_set();

      sort_set votes = sort_set();

      sort_set views = sort_set();

      sort_set shares = sort_set();

      sort_set comments = sort_set();

      sort_set popular = sort_set();

      sort_set viral = sort_set();

      sort_set discussion = sort_set();

      sort_set prominent = sort_set();

      sort_set conversation = sort_set();

      sort_set discourse = sort_set();
   };
   
}    // ::detail


/**
 *  The purpose of the tag object is to allow the generation and listing of
 *  all top level posts by a string tag.  The desired sort orders include:
 *
 *  1. created - time of creation
 *  2. maturing - about to receive a payout
 *  3. active - last reply the post or any child of the post
 *  4. netvotes - individual accounts voting for post minus accounts voting against it
 *
 *  When ever a comment is modified, all tag_objects for that comment are updated to match.
 */
class tag_object : public object< tag_object_type, tag_object >
{
   public:
      template< typename Constructor, typename Allocator >
      tag_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      tag_object(){}

      id_type                    id;

      board_name_type            board;                        // the name of the board that the post was uploaded to. 

      tag_name_type              tag;                          // Name of the tag that is used for this tag object.

      detail::sort_options       sort;                         // Sorting values for tracking post rankings across combinations of votes, views, shares and comments, in releation to time.

      account_id_type            author;

      comment_id_type            parent;

      comment_id_type            comment;

      time_point                 created;                      // Time the post was created. 

      time_point                 active;                       // Time that a new comment on the post was last created.

      time_point                 cashout;                      // Time of the next content reward cashout due for the post. 

      bool                       encrypted;                    // True if the post is encrypted.

      post_rating_type           rating;                       // Content severity rating from the author. 

      string                     language;                     // Two letter language string for the language of the content.

      share_type                 author_reputation;            // Reputation of the author, from 0 to BLOCKCHAIN_PRECISION.

      uint32_t                   children = 0;                 // The total number of children, grandchildren, posts with this as root comment.

      int32_t                    net_votes = 0;                // The amount of upvotes, minus downvotes on the post.

      int32_t                    view_count = 0;               // The amount of views on the post.

      int32_t                    share_count = 0;              // The amount of shares on the post.

      int128_t                   net_reward = 0;               // Net reward is the sum of all vote, view, share and comment power, with the reward curve formula applied. 

      int128_t                   vote_power = 0;               // Sum of weighted voting power from votes.

      int128_t                   view_power = 0;               // Sum of weighted voting power from viewers.

      int128_t                   share_power = 0;              // Sum of weighted voting power from shares.

      int128_t                   comment_power = 0;            // Sum of weighted voting power from comments.

      bool                       is_post()const { return parent == comment_id_type(); }

      double                     quality_active()const { return sort.quality.active; }

      double                     quality_rapid()const { return sort.quality.rapid; }

      double                     quality_standard()const { return sort.quality.standard; }

      double                     quality_top()const { return sort.quality.top; }

      double                     quality_elite()const { return sort.quality.elite; }

      double                     votes_active()const { return sort.votes.active; }

      double                     votes_rapid()const { return sort.votes.rapid; }

      double                     votes_standard()const { return sort.votes.standard; }

      double                     votes_top()const { return sort.votes.top; }

      double                     votes_elite()const { return sort.votes.elite; }

      double                     views_active()const { return sort.views.active; }

      double                     views_rapid()const { return sort.views.rapid; }

      double                     views_standard()const { return sort.views.standard; }

      double                     views_top()const { return sort.views.top; }

      double                     views_elite()const { return sort.views.elite; }

      double                     shares_active()const { return sort.shares.active; }

      double                     shares_rapid()const { return sort.shares.rapid; }

      double                     shares_standard()const { return sort.shares.standard; }

      double                     shares_top()const { return sort.shares.top; }

      double                     shares_elite()const { return sort.shares.elite; }

      double                     comments_active()const { return sort.comments.active; }

      double                     comments_rapid()const { return sort.comments.rapid; }

      double                     comments_standard()const { return sort.comments.standard; }

      double                     comments_top()const { return sort.comments.top; }

      double                     comments_elite()const { return sort.comments.elite; }

      double                     popular_active()const { return sort.popular.active; }

      double                     popular_rapid()const { return sort.popular.rapid; }

      double                     popular_standard()const { return sort.popular.standard; }

      double                     popular_top()const { return sort.popular.top; }

      double                     popular_elite()const { return sort.popular.elite; }

      double                     viral_active()const { return sort.viral.active; }

      double                     viral_rapid()const { return sort.viral.rapid; }

      double                     viral_standard()const { return sort.viral.standard; }

      double                     viral_top()const { return sort.viral.top; }

      double                     viral_elite()const { return sort.viral.elite; }

      double                     discussion_active()const { return sort.discussion.active; }

      double                     discussion_rapid()const { return sort.discussion.rapid; }

      double                     discussion_standard()const { return sort.discussion.standard; }

      double                     discussion_top()const { return sort.discussion.top; }

      double                     discussion_elite()const { return sort.discussion.elite; }

      double                     prominent_active()const { return sort.prominent.active; }

      double                     prominent_rapid()const { return sort.prominent.rapid; }

      double                     prominent_standard()const { return sort.prominent.standard; }

      double                     prominent_top()const { return sort.prominent.top; }

      double                     prominent_elite()const { return sort.prominent.elite; }

      double                     conversation_active()const { return sort.conversation.active; }

      double                     conversation_rapid()const { return sort.conversation.rapid; }

      double                     conversation_standard()const { return sort.conversation.standard; }

      double                     conversation_top()const { return sort.conversation.top; }

      double                     conversation_elite()const { return sort.conversation.elite; }

      double                     discourse_active()const { return sort.discourse.active; }

      double                     discourse_rapid()const { return sort.discourse.rapid; }

      double                     discourse_standard()const { return sort.discourse.standard; }

      double                     discourse_top()const { return sort.discourse.top; }

      double                     discourse_elite()const { return sort.discourse.elite; }
};

typedef oid< tag_object > tag_id_type;

struct by_tag;
struct by_net_reward;             // all comments regardless of depth
struct by_comment;
struct by_parent_created;
struct by_parent_active;

struct by_parent_net_reward;      // all top level posts by direct pending payout

struct by_parent_net_votes;       
struct by_parent_view_count;     
struct by_parent_share_count;     
struct by_parent_children;       

struct by_parent_vote_power;   
struct by_parent_view_power;      
struct by_parent_share_power;    
struct by_parent_comment_power;    

struct by_author_parent_created;
struct by_author_net_votes;
struct by_author_view_count;
struct by_author_share_count;
struct by_author_children;
struct by_reward_fund_net_reward;

//========== Sorting Indexes ==========// 

struct by_parent_quality_active;
struct by_parent_quality_rapid;
struct by_parent_quality_standard;
struct by_parent_quality_top;
struct by_parent_quality_elite;

struct by_parent_votes_active;
struct by_parent_votes_rapid;
struct by_parent_votes_standard;
struct by_parent_votes_top;
struct by_parent_votes_elite;

struct by_parent_views_active;
struct by_parent_views_rapid;
struct by_parent_views_standard;
struct by_parent_views_top;
struct by_parent_views_elite;

struct by_parent_shares_active;
struct by_parent_shares_rapid;
struct by_parent_shares_standard;
struct by_parent_shares_top;
struct by_parent_shares_elite;

struct by_parent_comments_active;
struct by_parent_comments_rapid;
struct by_parent_comments_standard;
struct by_parent_comments_top;
struct by_parent_comments_elite;

struct by_parent_popular_active;
struct by_parent_popular_rapid;
struct by_parent_popular_standard;
struct by_parent_popular_top;
struct by_parent_popular_elite;

struct by_parent_viral_active;
struct by_parent_viral_rapid;
struct by_parent_viral_standard;
struct by_parent_viral_top;
struct by_parent_viral_elite;

struct by_parent_discussion_active;
struct by_parent_discussion_rapid;
struct by_parent_discussion_standard;
struct by_parent_discussion_top;
struct by_parent_discussion_elite;

struct by_parent_prominent_active;
struct by_parent_prominent_rapid;
struct by_parent_prominent_standard;
struct by_parent_prominent_top;
struct by_parent_prominent_elite;

struct by_parent_conversation_active;
struct by_parent_conversation_rapid;
struct by_parent_conversation_standard;
struct by_parent_conversation_top;
struct by_parent_conversation_elite;

struct by_parent_discourse_active;
struct by_parent_discourse_rapid;
struct by_parent_discourse_standard;
struct by_parent_discourse_top;
struct by_parent_discourse_elite;


typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_comment >,
         composite_key< tag_object,
            member< tag_object, comment_id_type, &tag_object::comment >,
            member< tag_object, tag_id_type, &tag_object::id >
         >,
         composite_key_compare< std::less< comment_id_type >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_created >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, time_point, &tag_object::created >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less< tag_name_type >, std::less<comment_id_type>, std::greater< time_point >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, time_point, &tag_object::active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< time_point >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_net_reward >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int128_t, &tag_object::net_reward >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< uint128_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_net_votes >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int32_t, &tag_object::net_votes >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_view_count >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int32_t, &tag_object::view_count >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_share_count >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int32_t, &tag_object::share_count >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_children >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, uint32_t, &tag_object::children >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_vote_power >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int128_t, &tag_object::vote_power >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_view_power >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int128_t, &tag_object::view_power >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_share_power >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int128_t, &tag_object::share_power >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comment_power >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, int128_t, &tag_object::comment_power >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_net_reward >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, int128_t, &tag_object::net_reward >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::greater< uint128_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_author_parent_created >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, account_id_type, &tag_object::author >,
               member< tag_object, time_point, &tag_object::created >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<account_id_type>, std::greater< time_point >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_author_net_votes >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, account_id_type, &tag_object::author >,
               member< tag_object, int32_t, &tag_object::net_votes >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<account_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_author_view_count >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, account_id_type, &tag_object::author >,
               member< tag_object, int32_t, &tag_object::view_count >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<account_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_author_share_count >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, account_id_type, &tag_object::author >,
               member< tag_object, int32_t, &tag_object::share_count >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<account_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_author_children >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, account_id_type, &tag_object::author >,
               member< tag_object, uint32_t, &tag_object::children >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<account_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_reward_fund_net_reward >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               const_mem_fun< tag_object, bool, &tag_object::is_post >,
               member< tag_object, int128_t, &tag_object::net_reward >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less< bool >,std::greater< int64_t >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_index;

// =========== Quality Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_quality_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::quality_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::quality_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::quality_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::quality_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::quality_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_quality_sort_index;

// =========== Votes Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_votes_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::votes_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::votes_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::votes_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::votes_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::votes_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_votes_sort_index;

// =========== Views Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_views_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::views_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::views_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::views_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::views_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::views_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_views_sort_index;

// =========== Shares Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_shares_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::shares_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::shares_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::shares_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::shares_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::shares_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_shares_sort_index;

// =========== Comments Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_comments_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::comments_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::comments_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::comments_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::comments_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::comments_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_comments_sort_index;

// =========== Popular Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_popular_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::popular_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::popular_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::popular_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::popular_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::popular_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_popular_sort_index;

// =========== Viral Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_viral_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::viral_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::viral_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::viral_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::viral_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::viral_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_viral_sort_index;

// =========== Discussion Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_discussion_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discussion_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discussion_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discussion_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discussion_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discussion_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_discussion_sort_index;

// =========== Prominent Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_prominent_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::prominent_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::prominent_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::prominent_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::prominent_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::prominent_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_prominent_sort_index;

// =========== Conversation Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_conversation_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::conversation_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::conversation_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::conversation_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::conversation_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::conversation_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_conversation_sort_index;

// =========== Discourse Index ========== //

typedef multi_index_container<
   tag_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_object, tag_id_type, &tag_object::id > >,
      ordered_unique< tag< by_parent_discourse_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discourse_active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discourse_rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discourse_standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discourse_top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               const_mem_fun< tag_object, double, &tag_object::discourse_elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >
   >,
   allocator< tag_object >
> tag_discourse_sort_index;


/**
 *  The purpose of this index is to quickly identify how popular various
 *  tags by maintaining various sums over
 *  all posts under a particular tag.
 */
class tag_stats_object : public object< tag_stats_object_type, tag_stats_object >
{
   public:
      template< typename Constructor, typename Allocator >
      tag_stats_object( Constructor&& c, allocator< Allocator > )
      {
         c( *this );
      }

      tag_stats_object() {}

      id_type           id;

      tag_name_type     tag;                 // Name of the tag being measured.

      asset             total_payout = asset( 0, SYMBOL_USD );    // USD valud of all earned content rewards for all posts using the tag.

      uint32_t          post_count = 0;      // Number of posts using the tag.

      uint32_t          children = 0;        // The amount of comments on root posts for all posts using the tag.

      int32_t           net_votes = 0;       // The amount of upvotes, minus downvotes for all posts using the tag.

      int32_t           view_count = 0;      // The amount of views for all posts using the tag.

      int32_t           share_count = 0;     // The amount of shares for all posts using the tag.

      int128_t          net_reward = 0;      // Net reward is the sum of all vote, view, share and comment power, with the reward curve formula applied. 

      int128_t          vote_power = 0;      // Sum of weighted voting power for all posts using the tag.

      int128_t          view_power = 0;      // Sum of weighted view power for all posts using the tag.

      int128_t          share_power = 0;     // Sum of weighted share power for all posts using the tag.

      int128_t          comment_power = 0;   // Sum of weighted comment power for all posts using the tag.
};

typedef oid< tag_stats_object > tag_stats_id_type;

struct by_net_reward;
struct by_vote_power;
struct by_view_power;
struct by_share_power;
struct by_comment_power;

typedef multi_index_container<
   tag_stats_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_stats_object, tag_stats_id_type, &tag_stats_object::id > >,
      ordered_unique< tag< by_tag >, member< tag_stats_object, tag_name_type, &tag_stats_object::tag > >,
      ordered_non_unique< tag< by_net_reward >,
         composite_key< tag_stats_object,
            member< tag_stats_object, int128_t, &tag_stats_object::net_reward >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag >
         >,
         composite_key_compare< std::greater< int128_t >, std::less< tag_name_type > >
      >,
      ordered_unique< tag< by_vote_power >,
         composite_key< tag_stats_object,
            member< tag_stats_object, int128_t, &tag_stats_object::vote_power >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag >
         >,
         composite_key_compare< std::greater< int128_t >, std::less< tag_name_type > >
      >,
      ordered_unique< tag< by_view_power >,
         composite_key< tag_stats_object,
            member< tag_stats_object, int128_t, &tag_stats_object::view_power >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag > 
         >,
         composite_key_compare< std::greater< int128_t >, std::less< tag_name_type > >
      >,
      ordered_unique< tag< by_share_power >,
         composite_key< tag_stats_object,
            member< tag_stats_object, int128_t, &tag_stats_object::share_power >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag >
         >,
         composite_key_compare< std::greater< int128_t >, std::less< tag_name_type > >
      >,
      ordered_unique< tag< by_comment_power >,
         composite_key< tag_stats_object,
            member< tag_stats_object, int128_t, &tag_stats_object::comment_power >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag >
         >,
         composite_key_compare< std::greater< int128_t >, std::less< tag_name_type > >
      >
  >,
  allocator< tag_stats_object >
> tag_stats_index;


/**
 * Measures an account's accumulated votes, views and 
 * shares according to specified authors, boards, and tags.
 * Used for generating and sorting post recommendations.
 */
class account_curation_metrics_object : public object< account_curation_metrics_object_type, account_curation_metrics_object >
{
   public:
      template< typename Constructor, typename Allocator >
      account_curation_metrics_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      account_curation_metrics_object() {}

      id_type                                      id;

      account_name_type                            account;

      flat_map< account_name_type, uint32_t >      author_votes;

      flat_map< board_name_type, uint32_t >        board_votes;

      flat_map< tag_name_type, uint32_t >          tag_votes;

      flat_map< account_name_type, uint32_t >      author_views;

      flat_map< board_name_type, uint32_t >        board_views;

      flat_map< tag_name_type, uint32_t >          tag_views;

      flat_map< account_name_type, uint32_t >      author_shares;

      flat_map< board_name_type, uint32_t >        board_shares;

      flat_map< tag_name_type, uint32_t >          tag_shares;
};

typedef oid< account_curation_metrics_object > account_curation_metrics_id_type;

struct by_account;

typedef multi_index_container<
   account_curation_metrics_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< account_curation_metrics_object, account_curation_metrics_id_type, &account_curation_metrics_object::id > >,
      ordered_unique< tag< by_account >, member< account_curation_metrics_object, account_name_type, &account_curation_metrics_object::account > >
   >,
   allocator< account_curation_metrics_object >
> account_curation_metrics_index;


/**
 * Measures an account's common followers, followed account, 
 * board, and tags and connections with all other accounts.
 */
class account_adjacency_object : public object< account_adjacency_object_type, account_adjacency_object >
{
   public:
      template< typename Constructor, typename Allocator >
      account_adjacency_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      account_adjacency_object() {}

      id_type                                      id;

      account_name_type                            account_a;

      account_name_type                            account_b;

      share_type                                   adjacency;

      time_point                                   last_updated;
};

typedef oid< account_adjacency_object > account_adjacency_id_type;

struct by_account_a_adjacent;
struct by_account_b_adjacent;
struct by_account_pair;

typedef multi_index_container<
   account_adjacency_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< account_adjacency_object, account_adjacency_id_type, &account_adjacency_object::id > >,
      ordered_unique< tag< by_account_pair >,
         composite_key< account_adjacency_object,
            member< account_adjacency_object, account_name_type, &account_adjacency_object::account_a >,
            member< account_adjacency_object, account_name_type, &account_adjacency_object::account_b >
         >
      >,
      ordered_unique< tag< by_account_a_adjacent >,
         composite_key< account_adjacency_object,
            member< account_adjacency_object, account_name_type, &account_adjacency_object::account_a >,
            member< account_adjacency_object, share_type, &account_adjacency_object::adjacency >,
            member< account_adjacency_object, account_adjacency_id_type, &account_adjacency_object::id >
         >,
         composite_key_compare< 
            std::less< account_name_type >, 
            std::greater< share_type >, 
            std::less< account_adjacency_id_type > 
         >
      >,
      ordered_unique< tag< by_account_b_adjacent >,
         composite_key< account_adjacency_object,
            member< account_adjacency_object, account_name_type, &account_adjacency_object::account_b >,
            member< account_adjacency_object, share_type, &account_adjacency_object::adjacency >,
            member< account_adjacency_object, account_adjacency_id_type, &account_adjacency_object::id >
         >,
         composite_key_compare< 
            std::less< account_name_type >, 
            std::greater< share_type >, 
            std::less< account_adjacency_id_type > 
         >
      >
   >,
   allocator< account_adjacency_object >
> account_adjacency_index;


/**
 * Measures a board's common subscribers and members with all other boards.
 */
class board_adjacency_object : public object< board_adjacency_object_type, board_adjacency_object >
{
   public:
      template< typename Constructor, typename Allocator >
      board_adjacency_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      board_adjacency_object() {}

      id_type                                      id;

      board_name_type                              board_a;

      board_name_type                              board_b;

      share_type                                   adjacency;

      time_point                                   last_updated;
};

typedef oid< board_adjacency_object > board_adjacency_id_type;

struct by_board_a_adjacent;
struct by_board_b_adjacent;
struct by_board_pair;

typedef multi_index_container<
   board_adjacency_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< board_adjacency_object, board_adjacency_id_type, &board_adjacency_object::id > >,
      ordered_unique< tag< by_board_pair >,
         composite_key< board_adjacency_object,
            member< board_adjacency_object, board_name_type, &board_adjacency_object::board_a >,
            member< board_adjacency_object, board_name_type, &board_adjacency_object::board_b >
         >
      >,
      ordered_unique< tag< by_board_a_adjacent >,
         composite_key< board_adjacency_object,
            member< board_adjacency_object, board_name_type, &board_adjacency_object::board_a >,
            member< board_adjacency_object, share_type, &board_adjacency_object::adjacency >,
            member< board_adjacency_object, board_adjacency_id_type, &board_adjacency_object::id >
         >,
         composite_key_compare< 
            std::less< board_name_type >, 
            std::greater< share_type >, 
            std::less< board_adjacency_id_type > 
         >
      >,
      ordered_unique< tag< by_board_b_adjacent >,
         composite_key< board_adjacency_object,
            member< board_adjacency_object, board_name_type, &board_adjacency_object::board_b >,
            member< board_adjacency_object, share_type, &board_adjacency_object::adjacency >,
            member< board_adjacency_object, board_adjacency_id_type, &board_adjacency_object::id >
         >,
         composite_key_compare< 
            std::less< board_name_type >, 
            std::greater< share_type >, 
            std::less< board_adjacency_id_type > 
         >
      >
   >,
   allocator< board_adjacency_object >
> board_adjacency_index;


/**
 * Measures a tag's common followers with all other tags.
 */
class tag_adjacency_object : public object< tag_adjacency_object_type, tag_adjacency_object >
{
   public:
      template< typename Constructor, typename Allocator >
      tag_adjacency_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      tag_adjacency_object() {}

      id_type                                      id;

      tag_name_type                                tag_a;

      tag_name_type                                tag_b;

      share_type                                   adjacency;

      time_point                                   last_updated;
};

typedef oid< tag_adjacency_object > tag_adjacency_id_type;

struct by_tag_a_adjacent;
struct by_tag_b_adjacent;
struct by_tag_pair;

typedef multi_index_container<
   tag_adjacency_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_adjacency_object, tag_adjacency_id_type, &tag_adjacency_object::id > >,
      ordered_unique< tag< by_tag_pair >,
         composite_key< tag_adjacency_object,
            member< tag_adjacency_object, tag_name_type, &tag_adjacency_object::tag_a >,
            member< tag_adjacency_object, tag_name_type, &tag_adjacency_object::tag_b >
         >
      >,
      ordered_unique< tag< by_tag_a_adjacent >,
         composite_key< tag_adjacency_object,
            member< tag_adjacency_object, tag_name_type, &tag_adjacency_object::tag_a >,
            member< tag_adjacency_object, share_type, &tag_adjacency_object::adjacency >,
            member< tag_adjacency_object, tag_adjacency_id_type, &tag_adjacency_object::id >
         >,
         composite_key_compare< 
            std::less< tag_name_type >, 
            std::greater< share_type >, 
            std::less< tag_adjacency_id_type > 
         >
      >,
      ordered_unique< tag< by_tag_b_adjacent >,
         composite_key< tag_adjacency_object,
            member< tag_adjacency_object, tag_name_type, &tag_adjacency_object::tag_b >,
            member< tag_adjacency_object, share_type, &tag_adjacency_object::adjacency >,
            member< tag_adjacency_object, tag_adjacency_id_type, &tag_adjacency_object::id >
         >,
         composite_key_compare< 
            std::less< tag_name_type >, 
            std::greater< share_type >, 
            std::less< tag_adjacency_id_type > 
         >
      >
   >,
   allocator< tag_adjacency_object >
> tag_adjacency_index;


/**
 * Records an account's recommended posts list, according to the authors, 
 * boards, tags that the account has interacted with.
 */
class account_recommendations_object : public object< account_recommendations_object_type, account_recommendations_object >
{
   public:
      template< typename Constructor, typename Allocator >
      account_recommendations_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      account_recommendations_object() {}

      id_type                                      id;

      account_name_type                            account;

      flat_set< comment_id_type >                  recommended_posts;

      time_point                                   last_updated;
};

typedef oid< account_recommendations_object > account_recommendations_id_type;

struct by_account;

typedef multi_index_container<
   account_recommendations_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< account_recommendations_object, account_recommendations_id_type, &account_recommendations_object::id > >,
      ordered_unique< tag< by_account >, member< account_recommendations_object, account_name_type, &account_recommendations_object::account > >
   >,
   allocator< account_recommendations_object >
> account_recommendations_index;


/**
 *  The purpose of this object is to track the relationship between accounts based upon how a user votes. Every time
 *  a user votes on a post, the relationship between voter and author increases direct reward.
 */
class peer_stats_object : public object< peer_stats_object_type, peer_stats_object >
{
   public:
      template< typename Constructor, typename Allocator >
      peer_stats_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      peer_stats_object() {}

      id_type           id;

      account_id_type   voter;

      account_id_type   peer;

      int32_t           direct_positive_votes = 0;

      int32_t           direct_votes = 1;

      int32_t           indirect_positive_votes = 0;

      int32_t           indirect_votes = 1;

      float             rank = 0;

      void update_rank()
      {
         auto direct         = float( direct_positive_votes ) / direct_votes;
         auto indirect       = float( indirect_positive_votes ) / indirect_votes;
         auto direct_order   = log( direct_votes );
         auto indirect_order = log( indirect_votes );

         if( !(direct_positive_votes+indirect_positive_votes) ){
         direct_order *= -1;
         indirect_order *= -1;
         }

         direct *= direct;
         indirect *= indirect;

         direct *= direct_order * 10;
         indirect *= indirect_order;

         rank = direct + indirect;
      }
};

typedef oid< peer_stats_object > peer_stats_id_type;

struct by_rank;
struct by_voter_peer;
typedef multi_index_container<
   peer_stats_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< peer_stats_object, peer_stats_id_type, &peer_stats_object::id > >,
      ordered_unique< tag< by_rank >,
         composite_key< peer_stats_object,
            member< peer_stats_object, account_id_type, &peer_stats_object::voter >,
            member< peer_stats_object, float, &peer_stats_object::rank >,
            member< peer_stats_object, account_id_type, &peer_stats_object::peer >
         >,
         composite_key_compare< std::less< account_id_type >, std::greater< float >, std::less< account_id_type > >
      >,
      ordered_unique< tag< by_voter_peer >,
         composite_key< peer_stats_object,
            member< peer_stats_object, account_id_type, &peer_stats_object::voter >,
            member< peer_stats_object, account_id_type, &peer_stats_object::peer >
         >,
         composite_key_compare< std::less< account_id_type >,  std::less< account_id_type > >
      >
   >,
   allocator< peer_stats_object >
> peer_stats_index;

/**
 *  This purpose of this object is to maintain stats about which tags an author uses, how frequently, and
 *  how many total earnings of all posts by author in tag.  It also allows us to answer the question of which
 *  authors earn the most in each tag category.  This helps users to discover the best bloggers to follow for
 *  particular tags.
 */
class author_tag_stats_object : public object< author_tag_stats_object_type, author_tag_stats_object >
{
  public:
      template< typename Constructor, typename Allocator >
      author_tag_stats_object( Constructor&& c, allocator< Allocator > )
      {
         c( *this );
      }

      id_type             id;

      account_id_type     author;

      tag_name_type       tag;

      asset               total_rewards = asset( 0, SYMBOL_USD );

      uint32_t            total_posts = 0;
};

typedef oid< author_tag_stats_object > author_tag_stats_id_type;

struct by_author_tag_posts;
struct by_author_posts_tag;
struct by_author_tag_rewards;
struct by_tag_rewards_author;
using std::less;
using std::greater;

typedef chainbase::shared_multi_index_container<
  author_tag_stats_object,
  indexed_by<
      ordered_unique< tag< by_id >,
        member< author_tag_stats_object, author_tag_stats_id_type, &author_tag_stats_object::id >
      >,
      ordered_unique< tag< by_author_posts_tag >,
         composite_key< author_tag_stats_object,
            member< author_tag_stats_object, account_id_type, &author_tag_stats_object::author >,
            member< author_tag_stats_object, uint32_t, &author_tag_stats_object::total_posts >,
            member< author_tag_stats_object, tag_name_type, &author_tag_stats_object::tag >
         >,
         composite_key_compare< less< account_id_type >, greater< uint32_t >, less< tag_name_type > >
      >,
      ordered_unique< tag< by_author_tag_posts >,
         composite_key< author_tag_stats_object,
            member< author_tag_stats_object, account_id_type, &author_tag_stats_object::author >,
            member< author_tag_stats_object, tag_name_type, &author_tag_stats_object::tag >,
            member< author_tag_stats_object, uint32_t, &author_tag_stats_object::total_posts >
         >,
         composite_key_compare< less< account_id_type >, less< tag_name_type >, greater< uint32_t > >
      >,
      ordered_unique< tag< by_author_tag_rewards >,
         composite_key< author_tag_stats_object,
            member< author_tag_stats_object, account_id_type, &author_tag_stats_object::author >,
            member< author_tag_stats_object, tag_name_type, &author_tag_stats_object::tag >,
            member< author_tag_stats_object, asset, &author_tag_stats_object::total_rewards >
         >,
         composite_key_compare< less< account_id_type >, less< tag_name_type >, greater< asset > >
      >,
      ordered_unique< tag< by_tag_rewards_author >,
         composite_key< author_tag_stats_object,
            member< author_tag_stats_object, tag_name_type, &author_tag_stats_object::tag >,
            member< author_tag_stats_object, asset, &author_tag_stats_object::total_rewards >,
            member< author_tag_stats_object, account_id_type, &author_tag_stats_object::author >
         >,
         composite_key_compare< less< tag_name_type >, greater< asset >, less< account_id_type > >
      >
  >
> author_tag_stats_index;

/**
 * Used to parse the metadata from the comment json field.
 */
struct comment_metadata 
{ 
   set<string> tags;
   set<string> boards; 
};

/**
 *  This plugin will scan all changes to posts and/or their meta data and
 *
 */
class tags_plugin : public node::app::plugin
{
   public:
      tags_plugin( application* app );
      virtual ~tags_plugin();

      std::string plugin_name()const override { return TAGS_PLUGIN_NAME; }
      virtual void plugin_set_program_options(
         boost::program_options::options_description& cli,
         boost::program_options::options_description& cfg) override;
      virtual void plugin_initialize(const boost::program_options::variables_map& options) override;
      virtual void plugin_startup() override;

      friend class detail::tags_plugin_impl;
      std::unique_ptr<detail::tags_plugin_impl> my;
};

/**
 *  This API is used to query data maintained by the tags_plugin
 */
class tag_api : public std::enable_shared_from_this<tag_api> {
   public:
      tag_api(){};
      tag_api(const app::api_context& ctx){}//:_app(&ctx.app){}

      void on_api_startup(){
      }

      vector<tag_stats_object> get_tags()const { return vector<tag_stats_object>(); }

   private:
      //app::application* _app = nullptr;
};

} } //node::tag

FC_API( node::tags::tag_api, (get_tags) );

FC_REFLECT( node::tags::tag_object,
         (id)
         (board)
         (tag)
         (sort)
         (author)
         (parent)
         (comment)
         (created)
         (active)
         (cashout)
         (encrypted)
         (rating)
         (language)
         (author_reputation)
         (children)
         (net_votes)
         (view_count)
         (share_count)
         (net_reward)
         (vote_power)
         (view_power)
         (share_power)
         (comment_power)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_quality_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_votes_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_views_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_shares_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_comments_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_popular_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_viral_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_discussion_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_prominent_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_conversation_sort_index );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_discourse_sort_index );

FC_REFLECT( node::tags::tag_stats_object,
         (id)
         (tag)
         (total_payout)
         (post_count)
         (children)
         (net_votes)
         (view_count)
         (share_count)
         (net_reward)
         (vote_power)
         (view_power)
         (share_power)
         (comment_power)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::tag_stats_object, node::tags::tag_stats_index );

FC_REFLECT( node::tags::account_curation_metrics_object,
         (id)
         (account)
         (author_votes)
         (board_votes)
         (tag_votes)
         (author_views)
         (board_views)
         (tag_views)
         (author_shares)
         (board_shares)
         (tag_shares)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::account_curation_metrics_object, node::tags::account_curation_metrics_index );

FC_REFLECT( node::tags::account_adjacency_object,
         (id)
         (account_a)
         (account_b)
         (adjacency)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::account_adjacency_object, node::tags::account_adjacency_index );

FC_REFLECT( node::tags::board_adjacency_object,
         (id)
         (board_a)
         (board_b)
         (adjacency)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::board_adjacency_object, node::tags::board_adjacency_index );

FC_REFLECT( node::tags::tag_adjacency_object,
         (id)
         (tag_a)
         (tag_b)
         (adjacency)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::tag_adjacency_object, node::tags::tag_adjacency_index );

FC_REFLECT( node::tags::account_recommendations_object,
         (id)
         (account)
         (recommended_posts)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::account_recommendations_object, node::tags::account_recommendations_index );

FC_REFLECT( node::tags::peer_stats_object,
         (id)
         (voter)
         (peer)
         (direct_positive_votes)
         (direct_votes)
         (indirect_positive_votes)
         (indirect_votes)
         (rank)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::peer_stats_object, node::tags::peer_stats_index );

FC_REFLECT( node::tags::author_tag_stats_object,
         (id)
         (author)
         (tag)
         (total_posts)
         (total_rewards)
         );

CHAINBASE_SET_INDEX_TYPE( node::tags::author_tag_stats_object, node::tags::author_tag_stats_index );

FC_REFLECT( node::tags::comment_metadata,
         (tags)
         (board)
         );