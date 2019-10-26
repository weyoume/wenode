#include <node/tags/tags_plugin.hpp>

#include <node/app/impacted.hpp>

#include <node/protocol/config.hpp>

#include <node/chain/database.hpp>
#include <node/chain/hardfork.hpp>
#include <node/chain/index.hpp>
#include <node/chain/operation_notification.hpp>
#include <node/chain/account_object.hpp>
#include <node/chain/comment_object.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/json.hpp>
#include <fc/string.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

namespace node { namespace tags { namespace detail {

using namespace node::protocol;

class tags_plugin_impl
{
   public:
      tags_plugin_impl(tags_plugin& _plugin)
         : _self( _plugin )
      { }
      virtual ~tags_plugin_impl();

      node::chain::database& database()
      {
         return _self.database();
      }

      void on_operation( const operation_notification& note );

      tags_plugin& _self;
};

tags_plugin_impl::~tags_plugin_impl()
{
   return;
}


/**
 * Contains the time preference options for a sorting type
 */
struct sort_set
{
   sort_set( const double& active = 0, const double& rapid = 0, const double& standard = 0, const double& top = 0, const double& elite = 0 ) :
      active(active),rapid(rapid),standard(standard),top(top),elite(elite){}

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
      const sort_set& conversation = sort_set(), const sort_set& discourse = sort_set() ) :
      quality(quality),votes(votes),views(views),shares(shares),comments(comments),popular(popular),
      viral(viral),discussion(discussion),prominent(prominent),conversation(conversation),discourse(discourse){}

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

struct operation_visitor
{
   operation_visitor( database& db ):_db(db){};
   typedef void result_type;

   database& _db;

   void remove_stats( const tag_object& tag, const tag_stats_object& stats )const
   {
      _db.modify( stats, [&]( tag_stats_object& s )
      {
         if( tag.parent == comment_id_type() )
         {
            s.top_posts--;
         }
         else
         {
            s.comments--;
         }
         s.total_trending -= static_cast<uint32_t>(tag.trending);
         s.net_votes   -= tag.net_votes;
      });
   }

   void add_stats( const tag_object& tag, const tag_stats_object& stats )const
   {
      _db.modify( stats, [&]( tag_stats_object& s )
      {
         if( tag.parent == comment_id_type() )
         {
            s.top_posts++;
         }
         else
         {
            s.comments++;
         }
         s.total_trending += static_cast<uint32_t>(tag.trending);
         s.net_votes   += tag.net_votes;
      });
   }

   void remove_tag( const tag_object& tag )const
   {
      _db.remove(tag);

      const auto& idx = _db.get_index<author_tag_stats_index>().indices().get<by_author_tag_posts>();
      auto itr = idx.lower_bound( boost::make_tuple(tag.author,tag.tag) );
      if( itr != idx.end() && itr->author == tag.author && itr->tag == tag.tag )
      {
         _db.modify( *itr, [&]( author_tag_stats_object& stats )
         {
            stats.total_posts--;
         });
      }
   }

   const tag_stats_object& get_stats( const string& tag )const
   {
      const auto& stats_idx = _db.get_index<tag_stats_index>().indices().get<by_tag>();
      auto itr = stats_idx.find( tag );
      if( itr != stats_idx.end() )
         return *itr;

      return _db.create<tag_stats_object>( [&]( tag_stats_object& stats )
      {
         stats.tag = tag;
      });
   }

   comment_metadata filter_tags( const comment_object& c ) const
   {
      comment_metadata meta;
      set< string > lower_tags;
      set< string > lower_boards; 
      uint8_t tag_limit = 10;
      uint8_t count = 0;

      if( c.tags.size() )
      {
         for( auto tag : c.tags )
         {
            ++count;
            if( count > tag_limit || lower_tags.size() > tag_limit )
            {
               break;
            }  
            if( to_string(tag) == "" )
            {
               continue;
            }
               
            lower_tags.insert( fc::to_lower( to_string(tag) ) );
         }
      }

      if( c.board.size() )
      {    
         if( to_string( c.board ) == "" )
         {
            continue;
         }
            
         lower_tags.insert( fc::to_lower( to_string( c.board ) ) );
      }
      
      lower_tags.insert( string() );      // add the universal null string tag
      lower_boards.insert( string() );      // add the universal null string board
      
      meta.tags = lower_tags;
      meta.boards = lower_boards;

      return meta;
   }

   void update_tag( const tag_object& current, const comment_object& comment, const sort_options& sort )const
   {
      const tag_stats_object& stats = get_stats( current.tag );
      remove_stats( current, stats );

      if( comment.cashout_time != fc::time_point::maximum() ) 
      {
         _db.modify( current, [&]( tag_object& obj ) 
         {
            obj.active            = comment.active;
            obj.rating            = comment.rating;
            obj.privacy           = comment.privacy;
            obj.author_reputation = comment.author_reputation;
            obj.cashout           = _db.calculate_discussion_payout_time( comment );
            obj.net_reward        = comment.net_reward.value;
            obj.net_votes         = comment.net_votes;
            obj.view_count        = comment.view_count;
            obj.share_count       = comment.share_count;
            obj.children          = comment.children;
            obj.vote_power        = comment.vote_power;
            obj.view_power        = comment.view_power;
            obj.share_power       = comment.share_power;
            obj.comment_power     = comment.comment_power;
            obj.sort              = sort;
      });
      add_stats( current, stats );
      } 
      else 
      {
         _db.remove( current );
      }
   }

   /**
    * Creates a new tag object for a comment, with a specified pair of board, and tag.
    */
   void create_tag( const string& board, const string& tag, const comment_object& comment, const sort_options& sort )const
   {
      comment_id_type parent;
      account_id_type author = _db.get_account( comment.author ).id;

      if( comment.parent_author.size() )
      {
         parent = _db.get_comment( comment.parent_author, comment.parent_permlink ).id;
      }
         
      const auto& tag_obj = _db.create<tag_object>( [&]( tag_object& obj )
      {
         obj.tag               = tag;
         obj.board             = board;
         obj.comment           = comment.id;
         obj.privacy           = comment.privacy;
         obj.rating            = comment.rating;
         obj.language          = comment.language;
         obj.author_reputation = comment.author_reputation;
         obj.parent            = parent;
         obj.created           = comment.created;
         obj.active            = comment.active;
         obj.cashout           = comment.cashout_time;

         obj.net_votes         = comment.net_votes;
         obj.view_count        = comment.view_count;
         obj.share_count       = comment.share_count;
         obj.children          = comment.children;

         obj.vote_power        = comment.vote_power;
         obj.view_power        = comment.view_power;
         obj.share_power       = comment.share_power;
         obj.comment_power     = comment.comment_power;

         obj.net_reward        = comment.net_reward.value;
         obj.author            = author;
         obj.sort              = sort;
      });
      add_stats( tag_obj, get_stats( tag ) );


      const auto& idx = _db.get_index<author_tag_stats_index>().indices().get<by_author_tag_posts>();
      auto itr = idx.lower_bound( boost::make_tuple(author,tag) );
      if( itr != idx.end() && itr->author == author && itr->tag == tag )
      {
         _db.modify( *itr, [&]( author_tag_stats_object& stats )
         {
            stats.total_posts++;
         });
      }
      else
      {
         _db.create<author_tag_stats_object>( [&]( author_tag_stats_object& stats )
         {
            stats.author = author;
            stats.tag    = tag;
            stats.total_posts = 1;
         });
      }
   }

/**  //=================================================//
 *   //=========== WEYOUME SORTING ALGORITHM ===========//
 *   //=================================================// 
 * 
 * Latency Factor (LF): Varies the weighting of post age against post scoring from all sources.
 * Equalization Factor (EF): Varies the distribution of weighted Power values to be more democratic (one account one vote) or meritocratic (one power one vote).
 * Reputation Factor (REPF): Determines the relative weight of posts made by users with higher reputations, measured by lifetime content rewards earned.
 * Activity Factor (AF): Varies the weight of the time used for latency between the post time, and the time of last comment. 
 * Vote Rank (VR): Varies the weight of post score by voting score.
 * View Rank (VIR): Varies the weight of the post score by view score.
 * Share Rank (SR): Varies the weight of the post score by the share score.
 * Comment Rank (CR): Varies the weight of the post score by the comment score.
 * Each sorting parameter should be between -100 and +100.
 */
   template< double LF, double EF, double REPF, double AF, double VR, double VIR, double SR, double CR >
   double calculate_total_post_score( const comment_object& c, const comment_metrics_object& m )const 
   {
      double weighted_vote_power = double(c.vote_power) * ( 1 - ( EF / 100 ) ) + double(c.vote_count) * double(m.average_vote_power) * ( EF / 100 );
      double weighted_view_power = double(c.view_power) * ( 1 - ( EF / 100 ) ) + double(c.view_count) * double(m.average_view_power) * ( EF / 100 );
      double weighted_share_power = double(c.share_power) * ( 1 - ( EF / 100 ) ) + double(c.share_count) * double(m.average_share_power) * ( EF / 100 );
      double weighted_comment_power = double(c.comment_power) * ( 1 - ( EF / 100 ) ) + double(c.comment_count) * double(m.average_comment_power) * ( EF / 100 );

      int vote_sign = 0;
      if( weighted_vote_power > 0 ) 
      {
         vote_sign = 1;
      } 
      else if( weighted_vote_power < 0 ) 
      {
         vote_sign = -1;
      }

      double vote_score = log2( std::abs( weighted_vote_power ) + 1 ) * vote_sign;
      double view_score = log2( weighted_view_power + 1 ) * m.vote_view_ratio;
      double share_score = log2( weighted_share_power + 1 ) * m.vote_share_ratio;
      double comment_score = log2( weighted_comment_power + 1 ) * m.vote_comment_ratio;

      double base_post_score = vote_score * ( VR / 100 ) + view_score * ( VIR / 100 ) + share_score * ( SR / 100 ) + comment_score * ( CR / 100 );
      double post_score = base_post_score * ( 1 + double( ( 10 * c.author_reputation ) / BLOCKCHAIN_PRECISION ) * ( REPF / 100 ) );
      double activity_weighted_time = double(c.created.sec_since_epoch()) * ( 1 - ( AF / 100 ) ) + double(c.active.sec_since_epoch()) * ( AF / 100 );
      return post_score * ( 1 - ( LF / 100 ) ) + ( activity_weighted_time / 3600 ) * ( LF / 100 );
   }

   /**
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * |                         | LF  |  EF | REPF| AF  | VR  | VIR | SR  | CR  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Quality (Active)        | 10  | 10  | 10  | 50  | 50  | 50  | 50  | 50  |
    * | Quality (Rapid)	        | 25  | 10  | 10  | 10  | 50  | 50  | 50  | 50  | 
    * | Quality (Standard)	     | 50  | 10  | 10  | 10  | 50  | 50  | 50  | 50  |
    * | Quality (Top)	        | 75  | 10  | 10  | 10  | 50  | 50  | 50  | 50  |
    * | Quality (Elite)	        | 90  | 50  | 50  | 10  | 50  | 50  | 50  | 50  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Votes (Active)          | 10  | 10  | 10  | 50  | 100 | 10  | 10  | 10  |
    * | Votes (Rapid)	        | 25  | 10  | 10  | 10  | 100 | 10  | 10  | 10  |
    * | Votes (Standard)        | 50  | 10  | 10  | 10  | 100 | 10  | 10  | 10  |
    * | Votes (Top)             | 75  | 10  | 10  | 10  | 100 | 10  | 10  | 10  |
    * | Votes (Elite)           | 90  | 50  | 50  | 10  | 100 | 10  | 10  | 10  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Views (Active)          | 10  | 10  | 10  | 50  | 10  | 100 | 10  | 10  | 
    * | Views (Rapid)	        | 25  | 10  | 10  | 10  | 10  | 100 | 10  | 10  | 
    * | Views (Standard)        | 50  | 10  | 10  | 10  | 10  | 100 | 10  | 10  |
    * | Views (Top)             | 75  | 10  | 10  | 10  | 10  | 100 | 10  | 10  |
    * | Views (Elite)           | 90  | 50  | 50  | 10  | 10  | 100 | 10  | 10  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Shares (Active)         | 10  | 10  | 10  | 50  | 10  | 10  | 100 | 10  | 
    * | Shares (Rapid)	        | 25  | 10  | 10  | 10  | 10  | 10  | 100 | 10  | 
    * | Shares (Standard)       | 50  | 10  | 10  | 10  | 10  | 10  | 100 | 10  | 
    * | Shares (Top)            | 75  | 10  | 10  | 10  | 10  | 10  | 100 | 10  | 
    * | Shares (Elite)          | 90  | 50  | 50  | 10  | 10  | 10  | 100 | 10  | 
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Comments (Active)       | 10  | 10  | 10  | 50  | 10  | 10  | 10  | 100 |
    * | Comments (Rapid)        | 25  | 10  | 10  | 10  | 10  | 10  | 10  | 100 |
    * | Comments (Standard)     | 50  | 10  | 10  | 10  | 10  | 10  | 10  | 100 |
    * | Comments (Top)          | 75  | 10  | 10  | 10  | 10  | 10  | 10  | 100 |
    * | Comments (Elite)        | 90  | 50  | 50  | 10  | 10  | 10  | 10  | 100 |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Popular (Active)        | 10  | 10  | 10  | 50  | 75  | 75  | 10  | 10  |  // Popular = Votes + Views
    * | Popular (Rapid)	        | 25  | 10  | 10  | 10  | 75  | 75  | 10  | 10  |
    * | Popular (Standard)      | 50  | 10  | 10  | 10  | 75  | 75  | 10  | 10  |
    * | Popular (Top)           | 75  | 10  | 10  | 10  | 75  | 75  | 10  | 10  |
    * | Popular (Elite)         | 90  | 50  | 50  | 10  | 75  | 75  | 10  | 10  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Viral (Active)          | 10  | 10  | 10  | 50  | 10  | 75  | 75  | 10  |  // Viral = Views + Shares
    * | Viral (Rapid)	        | 25  | 10  | 10  | 10  | 10  | 75  | 75  | 10  |
    * | Viral (Standard)        | 50  | 10  | 10  | 10  | 10  | 75  | 75  | 10  |
    * | Viral (Top)             | 75  | 10  | 10  | 10  | 10  | 75  | 75  | 10  |
    * | Viral (Elite)           | 90  | 50  | 50  | 10  | 10  | 75  | 75  | 10  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Discussion (Active)     | 10  | 10  | 10  | 50  | 10  | 10  | 75  | 75  |  // Discussion = Shares + Comments
    * | Discussion (Rapid)      | 25  | 10  | 10  | 10  | 10  | 10  | 75  | 75  |
    * | Discussion (Standard)   | 50  | 10  | 10  | 10  | 10  | 10  | 75  | 75  |
    * | Discussion (Top)        | 75  | 10  | 10  | 10  | 10  | 10  | 75  | 75  |
    * | Discussion (Elite)      | 90  | 50  | 50  | 10  | 10  | 10  | 75  | 75  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Prominent (Active)      | 10  | 10  | 10  | 50  | 75  | 10  | 75  | 10  |  // Prominent = Votes + Shares
    * | Prominent (Rapid)       | 25  | 10  | 10  | 10  | 75  | 10  | 75  | 10  |
    * | Prominent (Standard)    | 50  | 10  | 10  | 10  | 75  | 10  | 75  | 10  |
    * | Prominent (Top)         | 75  | 10  | 10  | 10  | 75  | 10  | 75  | 10  |
    * | Prominent (Elite)       | 90  | 50  | 50  | 10  | 75  | 10  | 75  | 10  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Conversation (Active)   | 10  | 10  | 10  | 50  | 10  | 75  | 10  | 75  |  // Conversation = Views + Comments
    * | Conversation (Rapid)    | 25  | 10  | 10  | 10  | 10  | 75  | 10  | 75  |
    * | Conversation (Standard) | 50  | 10  | 10  | 10  | 10  | 75  | 10  | 75  |
    * | Conversation (Top)      | 75  | 10  | 10  | 10  | 10  | 75  | 10  | 75  |
    * | Conversation (Elite)    | 90  | 50  | 50  | 10  | 10  | 75  | 10  | 75  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
    * | Discourse (Active)      | 10  | 10  | 10  | 50  | 75  | 10  | 10  | 75  |  // Discourse = Votes + Comments
    * | Discourse (Rapid)       | 25  | 10  | 10  | 10  | 75  | 10  | 10  | 75  |
    * | Discourse (Standard)    | 50  | 10  | 10  | 10  | 75  | 10  | 10  | 75  |
    * | Discourse (Top)         | 75  | 10  | 10  | 10  | 75  | 10  | 10  | 75  |
    * | Discourse (Elite)       | 90  | 50  | 50  | 10  | 75  | 10  | 10  | 75  |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
     
    * 
    * | Featured                | 100 | 0   | 0   | 0   | 100 | 100 | 100 | 100 |
    * +=========================+=====+=====+=====+=====+=====+=====+=====+=====+
   */



   /**
    * Finds an evenly balanced ranking that favours posts with a combination of votes, views, shares and comments
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_quality_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 50, 50, 50, 50 >( c, m );
   }


   /**
    * Finds an evenly balanced ranking that favours posts with a combination of votes, views, shares and comments
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_quality_rapid(const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 50, 50, 50, 50 >( c, m );
   }


   /**
    * Finds an evenly balanced ranking that favours posts with a combination of votes, views, shares and comments
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_quality_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 50, 50, 50, 50 >( c, m );
   }


   /**
    * Finds an evenly balanced ranking that favours posts with a combination of votes, views, shares and comments
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_quality_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 50, 50, 50, 50 >( c, m );
   }


   /**
    * Finds an evenly balanced ranking that favours posts with a combination of votes, views, shares and comments
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_quality_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 50, 50, 50, 50 >( c, m );
   }


   // ========== Vote Rankings ========== // 


   /**
    * Finds a ranking that heavily favours highly voted posts, with slight consideration of views, shares and comments.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_votes_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 100, 10, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted posts, with slight consideration of views, shares and comments.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_votes_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 100, 10, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted posts, with slight consideration of views, shares and comments.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_votes_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 100, 10, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted posts, with slight consideration of views, shares and comments.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_votes_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 100, 10, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted posts, with slight consideration of views, shares and comments.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_votes_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 100, 10, 10, 10 >( c, m );
   }


   // ========== View Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly viewed posts, with slight consideration of votes, shares and comments.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_views_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 10, 100, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed posts, with slight consideration of votes, shares and comments.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_views_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 10, 100, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed posts, with slight consideration of votes, shares and comments.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_views_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 10, 100, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed posts, with slight consideration of votes, shares and comments.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_views_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 10, 100, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed posts, with slight consideration of votes, shares and comments.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_views_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 10, 100, 10, 10 >( c, m );
   }


   // ========== Share Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly shared posts, with slight consideration of votes, views, and comments.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_shares_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 10, 10, 100, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared posts, with slight consideration of votes, views, and comments.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_shares_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 10, 10, 100, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared posts, with slight consideration of votes, views, and comments.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_shares_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 10, 10, 100, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared posts, with slight consideration of votes, views, and comments.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_shares_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 10, 10, 100, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared posts, with slight consideration of votes, views, and comments.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_shares_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 10, 10, 100, 10 >( c, m );
   }


   // ========== Comment Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly commented posts, with slight consideration of votes, views, and shares.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_comments_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 10, 10, 10, 100 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly commented posts, with slight consideration of votes, views, and shares.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_comments_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 10, 10, 10, 100 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly commented posts, with slight consideration of votes, views, and shares.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_comments_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 10, 10, 10, 100 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly commented posts, with slight consideration of votes, views, and shares.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_comments_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 10, 10, 10, 100 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly commented posts, with slight consideration of votes, views, and shares.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_comments_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 10, 10, 10, 100 >( c, m );
   }


   // ========== Popular Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly voted and viewed posts, with slight consideration of shares, and comments.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_popular_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 75, 75, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and viewed posts, with slight consideration of shares, and comments.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_popular_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 75, 75, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and viewed posts, with slight consideration of shares, and comments.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_popular_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 75, 75, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and viewed posts, with slight consideration of shares, and comments.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_popular_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 75, 75, 10, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and viewed posts, with slight consideration of shares, and comments.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_popular_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 75, 75, 10, 10 >( c, m );
   }


   // ========== Viral Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly viewed and shared posts, with slight consideration of votes, and comments.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_viral_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 10, 75, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and shared posts, with slight consideration of votes, and comments.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_viral_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 10, 75, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and shared posts, with slight consideration of votes, and comments.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_viral_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 10, 75, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and shared posts, with slight consideration of votes, and comments.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_viral_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 10, 75, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and shared posts, with slight consideration of votes, and comments.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_viral_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 10, 75, 75, 10 >( c, m );
   }


   // ========== Discussion Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly shared and commented posts, with slight consideration of votes, and views.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discussion_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 10, 10, 75, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared and commented posts, with slight consideration of votes, and views.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discussion_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 10, 10, 75, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared and commented posts, with slight consideration of votes, and views.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discussion_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 10, 10, 75, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared and commented posts, with slight consideration of votes, and views.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discussion_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 10, 10, 75, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly shared and commented posts, with slight consideration of votes, and views.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_discussion_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 10, 10, 75, 75 >( c, m );
   }


   // ========== Prominent Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly voted and shared posts, with slight consideration of views, and comments.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_prominent_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 75, 10, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and shared posts, with slight consideration of views, and comments.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_prominent_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 75, 10, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and shared posts, with slight consideration of views, and comments.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_prominent_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 75, 10, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and shared posts, with slight consideration of views, and comments.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_prominent_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 75, 10, 75, 10 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and shared posts, with slight consideration of views, and comments.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_prominent_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 75, 10, 75, 10 >( c, m );
   }


   // ========== Conversation Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly viewed and commented posts, with slight consideration of votes and shares.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_conversation_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 10, 75, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and commented posts, with slight consideration of votes and shares.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_conversation_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 10, 75, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and commented posts, with slight consideration of votes and shares.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_conversation_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 10, 75, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and commented posts, with slight consideration of votes and shares.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_conversation_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 10, 75, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly viewed and commented posts, with slight consideration of votes and shares.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_conversation_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 10, 75, 10, 75 >( c, m );
   }


   // ========== Discourse Rankings ========== //


   /**
    * Finds a ranking that heavily favours highly voted and commented posts, with slight consideration of views and shares.
    * Weights time with a very low latency peference, favouring newer posts, with a large activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discourse_active( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 10, 10, 10, 50, 75, 10, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and commented posts, with slight consideration of views and shares.
    * Weights time with a low latency peference, favouring newer posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discourse_rapid( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 25, 10, 10, 10, 75, 10, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and commented posts, with slight consideration of views and shares.
    * Weights time normally, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discourse_standard( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 50, 10, 10, 10, 75, 10, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and commented posts, with slight consideration of views and shares.
    * Weights time with a high latency peference, favouring older posts, with slight activity boost.
    * Slight Equalization and reputation boost. 
    */
   inline double calculate_discourse_top( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 75, 10, 10, 10, 75, 10, 10, 75 >( c, m );
   }


   /**
    * Finds a ranking that heavily favours highly voted and commented posts, with slight consideration of views and shares.
    * Weights time with a very high latency peference, favouring older posts, with slight activity boost.
    * Large Equalization and reputation boost. 
    */
   inline double calculate_discourse_elite( const comment_object& c, const comment_metrics_object& m )const 
   {
      return calculate_total_post_score< 90, 50, 50, 10, 75, 10, 10, 75 >( c, m );
   }
   
   
   /**
    * https://medium.com/hacking-and-gonzo/how-reddit-ranking-algorithms-work-ef111e33d0d9#.lcbj6auuw
    * SORTING ALGORITHM
    *
   template< int64_t S, int32_t T >
   double calculate_sortrank( const share_type& voting_power, const time_point& created_time ) const
   {
      /// new algorithm
      auto mod_sortrank = voting_power.value / S;

      /// reddit algorithm
      double order = log10( std::max<int64_t>( std::abs( mod_sortrank ), 1) );
      int sign = 0;
      if( mod_sortrank > 0 ) sign = 1;
      else if( mod_sortrank < 0 ) sign = -1;

      return sign * order + double( created_time.sec_since_epoch() ) / double( T );
   }

   inline double calculate_hot( const share_type& voting_power, const time_point& created_time )const
   {
      return calculate_sortrank< 10000000, 10000 >( voting_power, created_time );
   }

   inline double calculate_trending( const share_type& voting_power, const time_point& created_time )const
   {
      return calculate_sortrank< 10000000, 480000 >( voting_power, created_time );
   }
   */


   inline sort_options build_sort_options( const comment_object& c, const comment_metrics_object& m )
   {
      sort_options sort;

      sort.quality.active        = calculate_quality_active( c, m );
      sort.quality.rapid         = calculate_quality_rapid( c, m );
      sort.quality.standard      = calculate_quality_standard( c, m );
      sort.quality.top           = calculate_quality_top( c, m );
      sort.quality.elite         = calculate_quality_elite( c, m );

      sort.votes.active          = calculate_votes_active( c, m );
      sort.votes.rapid           = calculate_votes_rapid( c, m );
      sort.votes.standard        = calculate_votes_standard( c, m );
      sort.votes.top             = calculate_votes_top( c, m );
      sort.votes.elite           = calculate_votes_elite( c, m );

      sort.views.active          = calculate_views_active( c, m );
      sort.views.rapid           = calculate_views_rapid( c, m );
      sort.views.standard        = calculate_views_standard( c, m );
      sort.views.top             = calculate_views_top( c, m );
      sort.views.elite           = calculate_views_elite( c, m );

      sort.shares.active         = calculate_shares_active( c, m );
      sort.shares.rapid          = calculate_shares_rapid( c, m );
      sort.shares.standard       = calculate_shares_standard( c, m );
      sort.shares.top            = calculate_shares_top( c, m );
      sort.shares.elite          = calculate_shares_elite( c, m );

      sort.comments.active       = calculate_comments_active( c, m );
      sort.comments.rapid        = calculate_comments_rapid( c, m );
      sort.comments.standard     = calculate_comments_standard( c, m );
      sort.comments.top          = calculate_comments_top( c, m );
      sort.comments.elite        = calculate_comments_elite( c, m );

      sort.popular.active        = calculate_popular_active( c, m );
      sort.popular.rapid         = calculate_popular_rapid( c, m );
      sort.popular.standard      = calculate_popular_standard( c, m );
      sort.popular.top           = calculate_popular_top( c, m );
      sort.popular.elite         = calculate_popular_elite( c, m );

      sort.viral.active          = calculate_viral_active( c, m );
      sort.viral.rapid           = calculate_viral_rapid( c, m );
      sort.viral.standard        = calculate_viral_standard( c, m );
      sort.viral.top             = calculate_viral_top( c, m );
      sort.viral.elite           = calculate_viral_elite( c, m );

      sort.discussion.active     = calculate_discussion_active( c, m );
      sort.discussion.rapid      = calculate_discussion_rapid( c, m );
      sort.discussion.standard   = calculate_discussion_standard( c, m );
      sort.discussion.top        = calculate_discussion_top( c, m );
      sort.discussion.elite      = calculate_discussion_elite( c, m );

      sort.prominent.active      = calculate_prominent_active( c, m );
      sort.prominent.rapid       = calculate_prominent_rapid( c, m );
      sort.prominent.standard    = calculate_prominent_standard( c, m );
      sort.prominent.top         = calculate_prominent_top( c, m );
      sort.prominent.elite       = calculate_prominent_elite( c, m );

      sort.conversation.active   = calculate_conversation_active( c, m );
      sort.conversation.rapid    = calculate_conversation_rapid( c, m );
      sort.conversation.standard = calculate_conversation_standard( c, m );
      sort.conversation.top      = calculate_conversation_top( c, m );
      sort.conversation.elite    = calculate_conversation_elite( c, m );

      sort.discourse.active      = calculate_discourse_active( c, m );
      sort.discourse.rapid       = calculate_discourse_rapid( c, m );
      sort.discourse.standard    = calculate_discourse_standard( c, m );
      sort.discourse.top         = calculate_discourse_top( c, m );
      sort.discourse.elite       = calculate_discourse_elite( c, m );

      return sort;
   }

   /** finds tags that have been added or removed or updated */
   void update_tags( const comment_object& c, const comment_metrics_object& m, bool parse_tags = false )const
   { try {
      sort_options sort = build_sort_options( c, m );

      const auto& comment_idx = _db.get_index< tag_index >().indices().get< by_comment >();

      if( parse_tags )
      {
         comment_metadata meta = filter_tags( c );
         auto citr = comment_idx.lower_bound( c.id );

         map< string, const tag_object* > existing_tags;
         map< string, const tag_object* > existing_boards;
         vector< const tag_object* > remove_queue;

         while( citr != comment_idx.end() && citr->comment == c.id )
         {
            const tag_object* tag = &*citr;
            ++citr;

            if( meta.tags.find( tag->tag ) == meta.tags.end()  )
            {
               remove_queue.push_back(tag);
            }
            else
            {
               existing_tags[tag->tag] = tag;
            }

            if( meta.boards.find( tag->board ) == meta.boards.end()  )
            {
               remove_queue.push_back(tag);
            }
            else
            {
               existing_boards[tag->board] = tag;
            }
         }

         for( const auto& board : meta.boards )
         {
            for( const auto& tag : meta.tags )   // Add tag for each combination of board and tag, including universal tag and universal board.
            {
               auto existing_tag = existing_tags.find(tag);
               auto existing_board = existing_boards.find(board);

               if( existing_tag == existing_tags.end() || existing_board == existing_boards.end() )
               {
                  create_tag( board, tag, c, sort );
               }
               else
               {
                  update_tag( *existing->second, c, sort );
               }
            }
         }

         for( const auto& item : remove_queue )
         {
            remove_tag(*item);
         }  
      }
      else
      {
         auto citr = comment_idx.lower_bound( c.id );

         while( citr != comment_idx.end() && citr->comment == c.id )
         {
            update_tag( *citr, c, sort );
            ++citr;
         }
      }

      if( c.parent_author.size() )    // Recursively update the tags up the comment parent tree.
      {
         update_tags( _db.get_comment( c.parent_author, c.parent_permlink ), _db.get_comment_metrics(), false );
      }

   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }

   const peer_stats_object& get_or_create_peer_stats( account_id_type voter, account_id_type peer )const
   {
      const auto& peeridx = _db.get_index<peer_stats_index>().indices().get<by_voter_peer>();
      auto itr = peeridx.find( boost::make_tuple( voter, peer ) );
      if( itr == peeridx.end() )
      {
         return _db.create<peer_stats_object>( [&]( peer_stats_object& obj ) {
               obj.voter = voter;
               obj.peer  = peer;
         });
      }
      return *itr;
   }

   void update_indirect_vote( account_id_type a, account_id_type b, int positive )const
   {
      if( a == b )
         return;
      const auto& ab = get_or_create_peer_stats( a, b );
      const auto& ba = get_or_create_peer_stats( b, a );
      _db.modify( ab, [&]( peer_stats_object& o )
      {
         o.indirect_positive_votes += positive;
         o.indirect_votes++;
         o.update_rank();
      });
      _db.modify( ba, [&]( peer_stats_object& o )
      {
         o.indirect_positive_votes += positive;
         o.indirect_votes++;
         o.update_rank();
      });
   }

   void update_peer_stats( const account_object& voter, const account_object& author, const comment_object& c, int vote )const
   {
      if( voter.id == author.id ) return; /// ignore votes for yourself
      if( c.parent_author.size() ) return; /// only count top level posts

      const auto& stat = get_or_create_peer_stats( voter.id, author.id );
      _db.modify( stat, [&]( peer_stats_object& obj )
      {
         obj.direct_votes++;
         obj.direct_positive_votes += vote > 0;
         obj.update_rank();
      });

      const auto& voteidx = _db.get_index<comment_vote_index>().indices().get<by_comment_voter>();
      auto itr = voteidx.lower_bound( boost::make_tuple( comment_id_type(c.id), account_id_type() ) );
      while( itr != voteidx.end() && itr->comment == c.id )
      {
         update_indirect_vote( voter.id, itr->voter, (itr->vote_percent > 0)  == (vote > 0) );
         ++itr;
      }
   }

   void operator()( const comment_operation& op )const
   {
      update_tags( _db.get_comment( op.author, op.permlink ), _db.get_comment_metrics(), true );
   }


/**
   void operator()( const transfer_operation& op )const
   {
      if( op.to == NULL_ACCOUNT && op.amount.symbol == SYMBOL_USD )
      {
         vector<string> part; part.reserve(4);
         auto path = op.memo;
         boost::split( part, path, boost::is_any_of("/") );
         if( part.size() > 1 && part[0].size() && part[0][0] == '@' )
         {
            auto acnt = part[0].substr(1);
            auto perm = part[1];

            auto c = _db.find_comment( acnt, perm );
            if( c && c->parent_author.size() == 0 )
            {
               const auto& comment_idx = _db.get_index<tag_index>().indices().get<by_comment>();
               auto citr = comment_idx.lower_bound( c->id );
               while( citr != comment_idx.end() && citr->comment == c->id )
               {
                  _db.modify( *citr, [&]( tag_object& t )
                  {
                      if( t.cashout != fc::time_point::maximum() )
                          t.promoted_balance += op.amount.amount;
                  });
                  ++citr;
               }
            }
            else
            {
               ilog( "unable to find body" );
            }
         }
      }
   }
   */

   void operator()( const vote_operation& op )const
   {
      update_tags( _db.get_comment( op.author, op.permlink ), _db.get_comment_metrics(), false );
   }

   void operator()( const view_operation& op )const
   {
      update_tags( _db.get_comment( op.author, op.permlink ), _db.get_comment_metrics(), false );
   }

   void operator()( const share_operation& op )const
   {
      update_tags( _db.get_comment( op.author, op.permlink ), _db.get_comment_metrics(), false );
   }

   void operator()( const comment_reward_operation& op )const
   {
      const auto& c = _db.get_comment( op.author, op.permlink );
      update_tags( c, _db.get_comment_metrics(), false );

      comment_metadata meta = filter_tags( c );

      for( const string& tag : meta.tags )
      {
         _db.modify( get_stats( tag ), [&]( tag_stats_object& ts )
         {
            ts.total_payout += op.payout;
         });
      }
   }

   void operator()( const comment_payout_update_operation& op )const
   {
      const auto& c = _db.get_comment( op.author, op.permlink );
      update_tags( c, _db.get_comment_metrics(), false );
   }

   template<typename Op>
   void operator()( Op&& )const{} /// ignore all other ops
};



void tags_plugin_impl::on_operation( const operation_notification& note ) {
   try
   {
      /// plugins shouldn't ever throw
      note.op.visit( operation_visitor( database() ) );
   }
   catch ( const fc::exception& e )
   {
      edump( (e.to_detail_string()) );
   }
   catch ( ... )
   {
      elog( "unhandled exception" );
   }
}

} /// end detail namespace

tags_plugin::tags_plugin( application* app )
   : plugin( app ), my( new detail::tags_plugin_impl(*this) )
{
   chain::database& db = database();
   add_plugin_index< tag_index        >(db);
   add_plugin_index< tag_stats_index  >(db);
   add_plugin_index< peer_stats_index >(db);
   add_plugin_index< author_tag_stats_index >(db);
}

tags_plugin::~tags_plugin()
{
}

void tags_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
}

void tags_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
   ilog("Intializing tags plugin" );
   database().post_apply_operation.connect( [&]( const operation_notification& note){ my->on_operation(note); } );

   app().register_api_factory<tag_api>("tag_api");
}


void tags_plugin::plugin_startup()
{
}

} } /// node::tags

DEFINE_PLUGIN( tags, node::tags::tags_plugin )
