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
   tag_object_type              = ( TAG_SPACE_ID << 8 ),
   tag_stats_object_type        = ( TAG_SPACE_ID << 8 ) + 1,
   peer_stats_object_type       = ( TAG_SPACE_ID << 8 ) + 2,
   author_tag_stats_object_type = ( TAG_SPACE_ID << 8 ) + 3
};

namespace detail { 
   class tags_plugin_impl; 
   /**
    * Contains the time preference options for a sorting type
    */
   struct sort_set
   {
      sort_set( const double& active = 0, const double& rapid = 0, const double& standard = 0, const double& top = 0, const double& elite = 0 ):
         active(active),rapid(rapid),standard(standard),top(top),elite(elite), 
      
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
      sort_options( const sort_set& quality = sort_set(), const sort_set& votes = sort_set(), const sort_set& views = sort_set(), 
         const sort_set& shares = sort_set(), const sort_set& comments = sort_set(), const sort_set& popular = sort_set(), 
         const sort_set& viral = sort_set(), const sort_set& discussion = sort_set(), const sort_set& prominent = sort_set(),
         const sort_set& conversation = sort_set(), const sort_set& discourse = sort_set() ):
         quality(quality),votes(votes),views(views),shares(shares),comments(comments),popular(popular),
         viral(viral),discussion(discussion),prominent(prominent),conversation(conversation),discourse(discourse),
      
      sort_options(){}

      sort_set quality;

      sort_set votes;

      sort_set views;

      sort_set shares;

      sort_set comments;

      sort_set popular;

      sort_set viral;

      sort_set discussion;

      sort_set prominent;

      sort_set conversation;

      sort_set discourse;
   };
   
}


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

      tag_object() {}

      id_type           id;

      board_name_type   board;                        // the name of the board that the post was uploaded to. 

      tag_name_type     tag;                          // Name of the tag that is used for this tag object.

      sort_options      sort;                         // Sorting values for tracking post rankings across combinations of votes, views, shares and comments, in releation to time.

      account_id_type   author;

      comment_id_type   parent;

      comment_id_type   comment;

      time_point        created;                      // Time the post was created. 

      time_point        active;                       // Time that a new comment on the post was last created.

      time_point        cashout;                      // Time of the next content reward cashout due for the post. 

      bool              privacy;                      // Privacy type of the post.

      rating_types      rating;                       // Content severity rating from the author. 

      string            language;                     // Two letter language string for the language of the content.

      share_type        author_reputation;            // Reputation of the author, from 0 to BLOCKCHAIN_PRECISION.

      uint32_t          children = 0;                 // The total number of children, grandchildren, posts with this as root comment.

      int32_t           net_votes = 0;                // The amount of upvotes, minus downvotes on the post.

      int32_t           view_count = 0;               // The amount of views on the post.

      int32_t           share_count = 0;              // The amount of shares on the post.

      int128_t          net_reward = 0;               // Net reward is the sum of all vote, view, share and comment power, with the reward curve formula applied. 

      int128_t          vote_power = 0;               // Sum of weighted voting power from votes.

      int128_t          view_power = 0;               // Sum of weighted voting power from viewers.

      int128_t          share_power = 0;              // Sum of weighted voting power from shares.

      int128_t          comment_power = 0;            // Sum of weighted voting power from comments.

      bool is_post()const { return parent == comment_id_type(); }
};

typedef oid< tag_object > tag_id_type;

struct by_tag;
struct by_cashout;                // all posts regardless of depth
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
struct by_author_comment;
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
      ordered_unique< tag< by_author_comment >,
            composite_key< tag_object,
               member< tag_object, account_id_type, &tag_object::author >,
               member< tag_object, comment_id_type, &tag_object::comment >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< account_id_type >, std::less< comment_id_type >, std::less< tag_id_type > >
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
               member< tag_object, uint128_t, &tag_object::net_reward >,
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
               member< tag_object, int32_t, &tag_object::children >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< int32_t >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_cashout >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, time_point, &tag_object::cashout >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less< time_point >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_net_reward >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, uint128_t, &tag_object::net_reward >,
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
      ordered_unique< tag< by_reward_fund_net_reward >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               const_mem_fun< tag_object, bool, &tag_object::is_post >,
               member< tag_object, int64_t, &tag_object::net_reward >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less< bool >,std::greater< int64_t >, std::less< tag_id_type > >
      >,

      // =========== Quality Indexes ========== //

      ordered_unique< tag< by_parent_quality_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.quality.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.quality.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.quality.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.quality.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_quality_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.quality.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Votes Indexes ========== //

      ordered_unique< tag< by_parent_votes_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.votes.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.votes.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.votes.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.votes.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_votes_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.votes.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Views Indexes ========== //

      ordered_unique< tag< by_parent_views_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.views.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.views.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.views.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.views.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_views_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.views.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Shares Indexes ========== //

      ordered_unique< tag< by_parent_shares_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.shares.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.shares.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.shares.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.shares.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_shares_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.shares.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Comments Indexes ========== //

      ordered_unique< tag< by_parent_comments_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.comments.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.comments.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.comments.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.comments.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_comments_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.comments.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Popular Indexes ========== //

      ordered_unique< tag< by_parent_popular_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.popular.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.popular.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.popular.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.popular.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_popular_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.popular.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Viral Indexes ========== //

      ordered_unique< tag< by_parent_viral_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.viral.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.viral.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.viral.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.viral.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_viral_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.viral.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Discussion Indexes ========== //

      ordered_unique< tag< by_parent_discussion_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discussion.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discussion.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discussion.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discussion.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discussion_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discussion.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Prominent Indexes ========== //

      ordered_unique< tag< by_parent_prominent_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.prominent.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.prominent.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.prominent.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.prominent.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_prominent_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.prominent.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Conversation Indexes ========== //

      ordered_unique< tag< by_parent_conversation_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.conversation.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.conversation.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.conversation.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.conversation.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_conversation_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.conversation.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,

      // =========== Discourse Indexes ========== //

      ordered_unique< tag< by_parent_discourse_active >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discourse.active >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_rapid >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discourse.rapid >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_standard >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discourse.standard >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_top >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discourse.top >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      ordered_unique< tag< by_parent_discourse_elite >,
            composite_key< tag_object,
               member< tag_object, board_name_type, &tag_object::board >,
               member< tag_object, tag_name_type, &tag_object::tag >,
               member< tag_object, comment_id_type, &tag_object::parent >,
               member< tag_object, double, &tag_object::sort.discourse.elite >,
               member< tag_object, tag_id_type, &tag_object::id >
            >,
            composite_key_compare< std::less< board_name_type >, std::less<tag_name_type>, std::less<comment_id_type>, std::greater< double >, std::less< tag_id_type > >
      >,
      
   >,
   allocator< tag_object >
> tag_index;

/**
 *  The purpose of this index is to quickly identify how popular various tags by maintaining variou sums over
 *  all posts under a particular tag
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

      tag_name_type     tag;
      asset             total_payout = asset( 0, SYMBOL_USD );
      int32_t           net_votes = 0;
      uint32_t          top_posts = 0;
      uint32_t          comments  = 0;
      fc::uint128       total_trending = 0;
};

typedef oid< tag_stats_object > tag_stats_id_type;

struct by_comments;
struct by_top_posts;
struct by_trending;

typedef multi_index_container<
   tag_stats_object,
   indexed_by<
      ordered_unique< tag< by_id >, member< tag_stats_object, tag_stats_id_type, &tag_stats_object::id > >,
      ordered_unique< tag< by_tag >, member< tag_stats_object, tag_name_type, &tag_stats_object::tag > >,
      /*
      ordered_non_unique< tag< by_comments >,
         composite_key< tag_stats_object,
            member< tag_stats_object, uint32_t, &tag_stats_object::comments >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag >
         >,
         composite_key_compare< std::less< tag_name_type >, std::greater< uint32_t > >
      >,
      ordered_non_unique< tag< by_top_posts >,
         composite_key< tag_stats_object,
            member< tag_stats_object, uint32_t, &tag_stats_object::top_posts >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag >
         >,
         composite_key_compare< std::less< tag_name_type >, std::greater< uint32_t > >
      >,
      */
      ordered_non_unique< tag< by_trending >,
         composite_key< tag_stats_object,
            member< tag_stats_object, fc::uint128 , &tag_stats_object::total_trending >,
            member< tag_stats_object, tag_name_type, &tag_stats_object::tag >
         >,
         composite_key_compare<  std::greater< fc::uint128  >, std::less< tag_name_type > >
      >
  >,
  allocator< tag_stats_object >
> tag_stats_index;


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

      id_type         id;
      account_id_type author;
      tag_name_type   tag;
      asset           total_rewards = asset( 0, SYMBOL_USD );
      uint32_t        total_posts = 0;
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
   (id)(tag)(created)(active)(cashout)(net_reward)(net_votes)(hot)(trending)(promoted_balance)(children)(author)(parent)(comment) )
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_object, node::tags::tag_index )

FC_REFLECT( node::tags::tag_stats_object,
   (id)(tag)(total_payout)(net_votes)(top_posts)(comments)(total_trending) );
CHAINBASE_SET_INDEX_TYPE( node::tags::tag_stats_object, node::tags::tag_stats_index )

FC_REFLECT( node::tags::peer_stats_object,
   (id)(voter)(peer)(direct_positive_votes)(direct_votes)(indirect_positive_votes)(indirect_votes)(rank) );
CHAINBASE_SET_INDEX_TYPE( node::tags::peer_stats_object, node::tags::peer_stats_index )

FC_REFLECT( node::tags::comment_metadata, (tags) );

FC_REFLECT( node::tags::author_tag_stats_object, (id)(author)(tag)(total_posts)(total_rewards) )
CHAINBASE_SET_INDEX_TYPE( node::tags::author_tag_stats_object, node::tags::author_tag_stats_index )
