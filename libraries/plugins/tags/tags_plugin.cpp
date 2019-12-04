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
      tags_plugin_impl(tags_plugin& _plugin): _self( _plugin ){}
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
            s.post_count--;
         }
         s.net_votes -= tag.net_votes;
         s.view_count -= tag.view_count;
         s.share_count -= tag.share_count;
         s.children -= tag.children;
         s.vote_power -= tag.vote_power;
         s.view_power -= tag.view_power;
         s.share_power -= tag.share_power;
         s.comment_power -= tag.comment_power;
      });
   }

   void add_stats( const tag_object& tag, const tag_stats_object& stats )const
   {
      _db.modify( stats, [&]( tag_stats_object& s )
      {
         if( tag.parent == comment_id_type() )
         {
            s.post_count++;
         }
         s.net_votes += tag.net_votes;
         s.view_count += tag.view_count;
         s.share_count += tag.share_count;
         s.children += tag.children;
         s.vote_power += tag.vote_power;
         s.view_power += tag.view_power;
         s.share_power += tag.share_power;
         s.comment_power += tag.comment_power;
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

      if( comment.cashout_time != fc::time_point::maximum() || comment.deleted )
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
            stats.author        = author;
            stats.tag           = tag;
            stats.total_posts   = 1;
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


   void update_account_votes( const comment_object& c, const vote_operation& op )const
   { try {
      const auto& metrics_idx = _db.get_index< account_curation_metrics_index >().indices().get< by_account >();
      comment_metadata meta = filter_tags( c );
      auto metrics_itr = metrics_idx.find( op.voter );
      if( metrics_itr != metrics_idx.end() )
      {
         const account_curation_metrics_object& metrics = *metrics_itr;

         metrics.author_votes[ c.author ]++;

         for( const auto& board : meta.boards )
         {
            if( board != board_name_type() )
            {
               metrics.board_votes[ board ]++;
            }
         }

         for( const auto& tag : meta.tags )  
         {
            if( tag != tag_name_type() )
            {
               metrics.tag_votes[ tag ]++;
            }
         }
      }
      else
      {
         _db.create< account_curation_metrics_object >( [&]( account_curation_metrics_object& obj )
         {
            obj.account = op.voter;
            obj.author_votes[ c.author ]++;

            for( const auto& board : meta.boards )
            {
               if( board != board_name_type() )
               {
                  obj.board_votes[ board ]++;
               }
            }

            for( const auto& tag : meta.tags )  
            {
               if( tag != tag_name_type() )
               {
                  obj.tag_votes[ tag ]++;
               }
            }
         });
      }
   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }


   void update_account_views( const comment_object& c, const view_operation& op )const
   { try {
      const auto& metrics_idx = _db.get_index< account_curation_metrics_index >().indices().get< by_account >();
      comment_metadata meta = filter_tags( c );
      auto metrics_itr = metrics_idx.find( op.viewer );
      if( metrics_itr != metrics_idx.end() )
      {
         const account_curation_metrics_object& metrics = *metrics_itr;

         metrics.author_views[ c.author ]++;

         for( const auto& board : meta.boards )
         {
            if( board != board_name_type() )
            {
               metrics.board_views[ board ]++;
            }
         }

         for( const auto& tag : meta.tags )  
         {
            if( tag != tag_name_type() )
            {
               metrics.tag_views[ tag ]++;
            }
         }
      }
      else
      {
         _db.create< account_curation_metrics_object >( [&]( account_curation_metrics_object& obj )
         {
            obj.account = op.viewer;
            obj.author_views[ c.author ]++;

            for( const auto& board : meta.boards )
            {
               if( board != board_name_type() )
               {
                  obj.board_views[ board ]++;
               }
            }

            for( const auto& tag : meta.tags )  
            {
               if( tag != tag_name_type() )
               {
                  obj.tag_views[ tag ]++;
               }
            }
         });
      }
   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }


   void update_account_shares( const comment_object& c, const share_operation& op )const
   { try {
      const auto& metrics_idx = _db.get_index< account_curation_metrics_index >().indices().get< by_account >();
      comment_metadata meta = filter_tags( c );
      auto metrics_itr = metrics_idx.find( op.sharer );
      if( metrics_itr != metrics_idx.end() )
      {
         const account_curation_metrics_object& metrics = *metrics_itr;

         metrics.author_shares[ c.author ]++;

         for( const auto& board : meta.boards )
         {
            if( board != board_name_type() )
            {
               metrics.board_shares[ board ]++;
            }
         }

         for( const auto& tag : meta.tags )  
         {
            if( tag != tag_name_type() )
            {
               metrics.tag_shares[ tag ]++;
            }
         }
      }
      else
      {
         _db.create< account_curation_metrics_object >( [&]( account_curation_metrics_object& obj )
         {
            obj.account = op.sharer;
            obj.author_shares[ c.author ]++;

            for( const auto& board : meta.boards )
            {
               if( board != board_name_type() )
               {
                  obj.board_shares[ board ]++;
               }
            }

            for( const auto& tag : meta.tags )  
            {
               if( tag != tag_name_type() )
               {
                  obj.tag_shares[ tag ]++;
               }
            }
         });
      }
   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }


   void update_account_adjacency( const account_following_object& f )const
   { try {
      time_point now = _db.head_block_time();

      const auto& following_idx = _db.get_index< account_following_index >().indices().get< by_account >();
      const auto& adjacency_idx = _db.get_index< account_adjacency_index >().indices().get< by_account_pair >();

      for( auto name : f.followers )
      {
         auto follow_itr = following_idx.find( name );
         const account_following_object& acc_following = *follow_itr;

         for( auto sec_name : acc_following.followers )
         {
            account_name_type account_a;
            account_name_type account_b;
            if( acc_following.id < f.id )
            {
               account_a = acc_following.account;
               account_b = f.account;
            }
            else
            {
               account_b = acc_following.account;
               account_a = f.account;
            }
            
            auto adjacency_itr = adjacency_idx.find( boost::make_tuple( account_a, account_b ) );
            if( adjacency_itr != adjacency_idx.end() )
            {
               if( ( now - adjacency_itr->last_updated ) >= fc::days(1) )
               {
                  _db.modify( *adjacency_itr, [&]( account_adjacency_object& o )
                  {
                     o.adjacency = f.adjacency_value( acc_following );
                     o.last_updated = now;
                  });
               }
            }
            else
            {
               _db.create< account_adjacency_object >( [&]( account_adjacency_object& o )
               {
                  o.account_a = account_a;
                  o.account_b = account_b;
                  o.adjacency = f.adjacency_value( acc_following );
                  o.last_updated = now;
               });
            }
         }

         for( auto sec_name : acc_following.following )
         {
            account_name_type account_a;
            account_name_type account_b;
            if( acc_following.id < f.id )
            {
               account_a = acc_following.account;
               account_b = f.account;
            }
            else
            {
               account_b = acc_following.account;
               account_a = f.account;
            }
            
            auto adjacency_itr = adjacency_idx.find( boost::make_tuple( account_a, account_b ) );
            if( adjacency_itr != adjacency_idx.end() )
            {
               if( ( now - adjacency_itr->last_updated ) >= fc::days(1) )
               {
                  _db.modify( *adjacency_itr, [&]( account_adjacency_object& o )
                  {
                     o.adjacency = f.adjacency_value( acc_following );
                     o.last_updated = now;
                  });
               }
            }
            else
            {
               _db.create< account_adjacency_object >( [&]( account_adjacency_object& o ) 
               {
                  o.account_a = account_a;
                  o.account_b = account_b;
                  o.adjacency = f.adjacency_value( acc_following );
                  o.last_updated = now;
               });
            }
         }
      }

      for( auto name : f.following )
      {
         auto follow_itr = following_idx.find( name );
         const account_following_object& acc_following = *follow_itr;

         for( auto sec_name : acc_following.followers )
         {
            account_name_type account_a;
            account_name_type account_b;
            if( acc_following.id < f.id )
            {
               account_a = acc_following.account;
               account_b = f.account;
            }
            else
            {
               account_b = acc_following.account;
               account_a = f.account;
            }
            
            auto adjacency_itr = adjacency_idx.find( boost::make_tuple( account_a, account_b ) );
            if( adjacency_itr != adjacency_idx.end() )
            {
               if( ( now - adjacency_itr->last_updated ) >= fc::days(1) )
               {
                  _db.modify( *adjacency_itr, [&]( account_adjacency_object& o )
                  {
                     o.adjacency = f.adjacency_value( acc_following );
                     o.last_updated = now;
                  });
               }
            }
            else
            {
               _db.create< account_adjacency_object >( [&]( account_adjacency_object& o ) 
               {
                  o.account_a = account_a;
                  o.account_b = account_b;
                  o.adjacency = f.adjacency_value( acc_following );
                  o.last_updated = now;
               });
            }
         }

         for( auto sec_name : acc_following.following )
         {
            account_name_type account_a;
            account_name_type account_b;
            if( acc_following.id < f.id )
            {
               account_a = acc_following.account;
               account_b = f.account;
            }
            else
            {
               account_b = acc_following.account;
               account_a = f.account;
            }
            
            auto adjacency_itr = adjacency_idx.find( boost::make_tuple( account_a, account_b ) );
            if( adjacency_itr != adjacency_idx.end() )
            {
               if( ( now - adjacency_itr->last_updated ) >= fc::days(1) )
               {
                  _db.modify( *adjacency_itr, [&]( account_adjacency_object& o )
                  {
                     o.adjacency = f.adjacency_value( acc_following );
                     o.last_updated = now;
                  });
               }
            }
            else
            {
               _db.create< account_adjacency_object >( [&]( account_adjacency_object& o ) 
               {
                  o.account_a = account_a;
                  o.account_b = account_b;
                  o.adjacency = f.adjacency_value( acc_following );
                  o.last_updated = now;
               });
            }
         }
      }
   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }


   void update_board_adjacency( const board_member_object& m )const
   { try {
      time_point now = _db.head_block_time();
      const auto& following_idx = _db.get_index< account_following_index >().indices().get< by_account >();
      const auto& member_idx = _db.get_index< board_member_index >().indices().get< by_name >();
      const auto& adjacency_idx = _db.get_index< board_adjacency_index >().indices().get< by_board_pair >();

      for( auto name : m.subscribers )
      {
         auto follow_itr = following_idx.find( name );
         const account_following_object& acc_following = *follow_itr;

         for( auto board_name : acc_following.followed_boards )
         {
            board_name_type board_a;
            board_name_type board_b;
            auto member_itr = member_idx.find( board_name );

            const board_member_object& board_member = *member_itr;

            if( board_member.id < m.id )
            {
               board_a = board_member.name;
               board_b = m.name;
            }
            else
            {
               board_b = board_member.name;
               board_a = m.name;
            }
            
            auto adjacency_itr = adjacency_idx.find( boost::make_tuple( board_a, board_b ) );
            if( adjacency_itr != adjacency_idx.end() )
            {
               if( ( now - adjacency_itr->last_updated ) >= fc::days(1) )
               {
                  _db.modify( *adjacency_itr, [&]( board_adjacency_object& o )
                  {
                     o.adjacency = m.adjacency_value( board_member );
                     o.last_updated = now;
                  });
               }
            }
            else
            {
               _db.create< board_adjacency_object >( [&]( board_adjacency_object& o )
               {
                  o.board_a = board_a;
                  o.board_b = board_b;
                  o.adjacency = m.adjacency_value( board_member );
                  o.last_updated = now;
               });
            }
         }
      } 
   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }


   void update_tag_adjacency( const tag_following_object& t )const
   { try {
      time_point now = _db.head_block_time();
      const auto& following_idx = _db.get_index< account_following_index >().indices().get< by_account >();
      const auto& tag_idx = _db.get_index< tag_following_index >().indices().get< by_tag >();
      const auto& adjacency_idx = _db.get_index< tag_adjacency_index >().indices().get< by_tag_pair >();

      for( auto name : t.followers )
      {
         auto follow_itr = following_idx.find( name );
         const account_following_object& acc_following = *follow_itr;

         for( auto tag : acc_following.followed_tags )
         {
            tag_name_type tag_a;
            tag_name_type tag_b;
            auto tag_itr = tag_idx.find( tag );

            const tag_following_object& tag_following = *tag_itr;

            if( tag_following.id < t.id )
            {
               tag_a = tag_following.tag;
               tag_b = t.tag;
            }
            else
            {
               tag_b = tag_following.tag;
               tag_a = t.tag;
            }
            
            auto adjacency_itr = adjacency_idx.find( boost::make_tuple( tag_a, tag_b ) );
            if( adjacency_itr != adjacency_idx.end() )
            {
               if( ( now - adjacency_itr->last_updated ) >= fc::days(1) )
               {
                  _db.modify( *adjacency_itr, [&]( tag_adjacency_object& o )
                  {
                     o.adjacency = t.adjacency_value( tag_following );
                     o.last_updated = now;
                  });
               }
            }
            else
            {
               _db.create< tag_adjacency_object >( [&]( tag_adjacency_object& o )
               {
                  o.tag_a = tag_a;
                  o.tag_b = tag_b;
                  o.adjacency = t.adjacency_value( tag_following );
                  o.last_updated = now;
               });
            }
         }
      } 
   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }


   const peer_stats_object& get_or_create_peer_stats( account_id_type voter, account_id_type peer )const
   {
      const auto& peeridx = _db.get_index<peer_stats_index>().indices().get<by_voter_peer>();
      auto itr = peeridx.find( boost::make_tuple( voter, peer ) );
      if( itr == peeridx.end() )
      {
         return _db.create<peer_stats_object>( [&]( peer_stats_object& obj ) 
         {
            obj.voter = voter;
            obj.peer  = peer;
         });
      }
      return *itr;
   }

   const peer_stats_object& get_or_create_peer_stats( account_id_type voter, account_id_type peer )const
   {
      const auto& peeridx = _db.get_index<peer_stats_index>().indices().get<by_voter_peer>();
      auto itr = peeridx.find( boost::make_tuple( voter, peer ) );
      if( itr == peeridx.end() )
      {
         return _db.create<peer_stats_object>( [&]( peer_stats_object& obj ) 
         {
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

   flat_set<comment_id_type> get_recommendations( const account_name_type& account )const
   { try {
      const auto& acc_obj = _db.get_account( account );
      const auto& following_obj = _db.get_account_following( account );
      flat_set< comment_id_type > selected;    

      const auto& curation_idx = _db.get_index< account_curation_metrics_index >().indices().get< by_account >();
      auto curation_itr = curation_idx.find( account );

      if( curation_itr != curation_idx.end() )    // finds the top 10 authors, boards, and tags that the user has interacted with
      {
         const account_curation_metrics_object& metrics = *curation_itr;
         flat_map< account_name_type, share_type > authors;
         flat_map< board_name_type, share_type > boards;
         flat_map< tag_name_type, share_type > tags;

         for( auto author : metrics.author_votes )
         {
            authors[ author.first ] += author.second * 5;
         }
         for( auto author : metrics.author_views )
         {
            authors[ author.first ] += author.second;
         }
         for( auto author : metrics.author_shares )
         {
            authors[ author.first ] += author.second * 10;
         }
         for( auto board : metrics.board_votes )
         {
            boards[ board.first ] += board.second * 5;
         }
         for( auto board : metrics.board_views )
         {
            boards[ board.first ] += board.second;
         }
         for( auto board : metrics.board_shares )
         {
            boards[ board.first ] += board.second * 10;
         }
         for( auto tag : metrics.tag_votes )
         {
            tags[ tag.first ] += tag.second * 5;
         }
         for( auto tag : metrics.tag_views )
         {
            tags[ tag.first ] += tag.second;
         }
         for( auto tag : metrics.tag_shares )
         {
            tags[ tag.first ] += tag.second * 10;
         }

         vector< pair < account_name_type, share_type > > top_authors;
         vector< pair < board_name_type, share_type > > top_boards;
         vector< pair < tag_name_type, share_type > > top_tags;

         top_authors.reserve( authors.size() );
         top_board.reserve( boards.size() );
         top_tags.reserve( tags.size() );

         for( auto author : authors )
         {
            top_authors.push_back( std::make_pair( author.first, author.second ) );
         }
         for( auto board : boards )
         {
            top_boards.push_back( std::make_pair( board.first, board.second ) );
         }
         for( auto tag : tags )
         {
            top_tags.push_back( std::make_pair( tag.first, tag.second ) );
         }

         std::sort( top_boards.begin(), top_boards.end(), [&]( auto a, auto b )
         {
            return a.second < b.second;
         });

         std::sort( top_tags.begin(), top_tags.end(), [&]( auto a, auto b )
         {
            return a.second < b.second;
         });

         std::sort( top_authors.begin(), top_authors.end(), [&]( auto a, auto b )
         {
            return a.second < b.second;
         });

         if( top_boards.size() > 10 )
         {
            top_boards = std::vector( top_boards.begin(), top_boards.begin() + 10 );
         }

         if( top_tags.size() > 10 )
         {
            top_tags = std::vector( top_tags.begin(), top_tags.begin() + 10 );
         }

         if( top_authors.size() > 10 )
         {
            top_authors = std::vector( top_authors.begin(), top_authors.begin() + 10 );
         }

         // Retrieves all hybrid tag indexes for getting posts from each tag and board.

         const auto& view_idx = _db.get_index<comment_view_index>().indices().get<by_viewer_comment>();
         const auto& blog_idx = _db.get_index<blog_index>().indices().get<by_new_account_blog>();
         const auto& author_vote_idx = _db.get_index<tag_index>().indices().get<by_author_net_votes>();
         const auto& author_view_idx = _db.get_index<tag_index>().indices().get<by_author_view_count>();
         const auto& author_share_idx = _db.get_index<tag_index>().indices().get<by_author_share_count>();
         const auto& author_comment_idx = _db.get_index<tag_index>().indices().get<by_author_children>();

         const auto& popular_rapid_idx = _db.get_index<tag_index>().indices().get<by_parent_popular_rapid>();
         const auto& popular_top_idx = _db.get_index<tag_index>().indices().get<by_parent_popular_top>();
         const auto& viral_rapid_idx = _db.get_index<tag_index>().indices().get<by_parent_viral_rapid>();
         const auto& viral_top_idx = _db.get_index<tag_index>().indices().get<by_parent_viral_top>();
         const auto& discussion_rapid_idx = _db.get_index<tag_index>().indices().get<by_parent_discussion_rapid>();
         const auto& discussion_top_idx = _db.get_index<tag_index>().indices().get<by_parent_discussion_top>();
         const auto& prominent_rapid_idx = _db.get_index<tag_index>().indices().get<by_parent_prominent_rapid>();
         const auto& prominent_top_idx = _db.get_index<tag_index>().indices().get<by_parent_prominent_top>();
         const auto& conversation_rapid_idx = _db.get_index<tag_index>().indices().get<by_parent_conversation_rapid>();
         const auto& conversation_top_idx = _db.get_index<tag_index>().indices().get<by_parent_conversation_top>();
         const auto& discourse_rapid_idx = _db.get_index<tag_index>().indices().get<by_parent_discourse_rapid>();
         const auto& discourse_top_idx = _db.get_index<tag_index>().indices().get<by_parent_discourse_top>();

         const auto& account_a_adjacency_idx = _db.get_index<account_adjacency_index>().indices().get<by_account_a_adjacent>();
         const auto& account_b_adjacency_idx = _db.get_index<account_adjacency_index>().indices().get<by_account_b_adjacent>();
         const auto& board_a_adjacency_idx = _db.get_index<board_adjacency_index>().indices().get<by_board_a_adjacent>();
         const auto& board_b_adjacency_idx = _db.get_index<board_adjacency_index>().indices().get<by_board_b_adjacent>();
         const auto& tag_a_adjacency_idx = _db.get_index<tag_adjacency_index>().indices().get<by_tag_a_adjacent>();
         const auto& tag_b_adjacency_idx = _db.get_index<tag_adjacency_index>().indices().get<by_tag_b_adjacent>();

         vector<account_name_type> related_authors;
         vector<board_name_type> related_boards;
         vector<tag_name_type> related_tags;

         related_authors.reserve( top_authors.size() );
         related_board.reserve( top_boards.size() );
         related_tags.reserve( top_tags.size() );

         // Add related unfollowed authors for each of the top authors, according to how many followers and connections in common they have.
         for( auto author : top_authors )
         {
            auto account_a_adjacency_itr = account_a_adjacency_idx.lower_bound( author.first );
            auto account_b_adjacency_itr = account_b_adjacency_idx.lower_bound( author.first );
            while( ( account_a_adjacency_itr->account_a == author.first && 
               account_a_adjacency_itr != account_a_adjacency_idx.end() ) || 
               ( account_b_adjacency_itr->account_b == author.first &&
               account_b_adjacency_itr != account_b_adjacency_idx.end() ) )
            {
               share_type adj_a = 0;
               account_name_type related_acc_a;
               share_type adj_b = 0;
               account_name_type related_acc_b;
               while( account_a_adjacency_itr->account_a == author.first && 
                  account_a_adjacency_itr != account_a_adjacency_idx.end() && 
                  related_acc_a == account_name_type() )
               {
                  if( following_obj.is_following( account_a_adjacency_itr->account_b ) )  // skip if already following
                  {
                     ++account_a_adjacency_itr;
                  }
                  else
                  {
                     adj_a = account_a_adjacency_itr->adjacency;
                     related_acc_a = account_a_adjacency_itr->account_b;
                  }
               }
               while( account_b_adjacency_itr->account_b == author.first && 
                  account_b_adjacency_itr != account_b_adjacency_idx.end() && 
                  related_acc_b == account_name_type() )
               {
                  if( following_obj.is_following( account_b_adjacency_itr->account_a ) )    // skip if already following
                  {
                     ++account_b_adjacency_itr;
                  }
                  else
                  {
                     adj_b = account_b_adjacency_itr->adjacency;
                     related_acc_b = account_b_adjacency_itr->account_a;
                  }
               }
               if( adj_a > adj_b )    // Add the most highly adjacent author to the related authors vector
               {
                  related_authors.push_back( related_acc_a );
                  ++account_a_adjacency_itr;
               }
               else
               {
                  related_authors.push_back( related_acc_b );
                  ++account_b_adjacency_itr;
               }
            }
         }

         // Add related unfollowed boards for each of the top boards, according to how many followers and connections in common they have.
         for( auto board : top_boards )
         {
            auto board_a_adjacency_itr = board_a_adjacency_idx.lower_bound( board.first );
            auto board_b_adjacency_itr = board_b_adjacency_idx.lower_bound( board.first );

            while( ( board_a_adjacency_itr->board_a == board.first && 
               board_a_adjacency_itr != board_a_adjacency_idx.end() ) || 
               ( board_b_adjacency_itr->board_b == board.first &&
               board_b_adjacency_itr != board_b_adjacency_idx.end() ) )
            {
               share_type adj_a = 0;
               board_name_type related_board_a;
               share_type adj_b = 0;
               board_name_type related_board_b;
               while( board_a_adjacency_itr->board_a == board.first && 
                  board_a_adjacency_itr != board_a_adjacency_idx.end() && 
                  related_board_a == board_name_type() )
               {
                  if( following_obj.is_following( board_a_adjacency_itr->board_b ) )  // skip if already following
                  {
                     ++board_a_adjacency_itr;
                  }
                  else
                  {
                     adj_a = board_a_adjacency_itr->adjacency;
                     related_board_a = board_a_adjacency_itr->board_b;
                  }
               }
               while( board_b_adjacency_itr->board_b == board.first && 
                  board_b_adjacency_itr != board_b_adjacency_idx.end() && 
                  related_board_b == board_name_type() )
               {
                  if( following_obj.is_following( board_b_adjacency_itr->board_a ) )    // skip if already following
                  {
                     ++board_b_adjacency_itr;
                  }
                  else
                  {
                     adj_b = board_b_adjacency_itr->adjacency;
                     related_board_b = board_b_adjacency_itr->board_a;
                  }
               }
               if( adj_a > adj_b )    // Add the most highly adjacent board to the related boards vector
               {
                  related_boards.push_back( related_board_a );
                  ++board_a_adjacency_itr;
               }
               else
               {
                  related_boards.push_back( related_board_b );
                  ++board_b_adjacency_itr;
               }
            }
         }

         // Add related unfollowed tags for each of the top tags, according to how many followers and connections in common they have.
         for( auto tag : top_tags )
         {
            auto tag_a_adjacency_itr = tag_a_adjacency_idx.lower_bound( tag.first );
            auto tag_b_adjacency_itr = tag_b_adjacency_idx.lower_bound( tag.first );

            while( ( tag_a_adjacency_itr->tag_a == tag.first && 
               tag_a_adjacency_itr != tag_a_adjacency_idx.end() ) || 
               ( tag_b_adjacency_itr->tag_b == tag.first &&
               tag_b_adjacency_itr != tag_b_adjacency_idx.end() ) )
            {
               share_type adj_a = 0;
               tag_name_type related_tag_a;
               share_type adj_b = 0;
               tag_name_type related_tag_b;

               while( tag_a_adjacency_itr->tag_a == tag.first && 
                  tag_a_adjacency_itr != tag_a_adjacency_idx.end() && 
                  related_tag_a == tag_name_type() )
               {
                  if( following_obj.is_following( tag_a_adjacency_itr->tag_b ) )  // skip if already following
                  {
                     ++tag_a_adjacency_itr;
                  }
                  else
                  {
                     adj_a = tag_a_adjacency_itr->adjacency;
                     related_tag_a = tag_a_adjacency_itr->tag_b;
                  }
               }

               while( tag_b_adjacency_itr->tag_b == tag.first && 
                  tag_b_adjacency_itr != tag_b_adjacency_idx.end() && 
                  related_tag_b == tag_name_type() )
               {
                  if( following_obj.is_following( tag_b_adjacency_itr->tag_a ) )    // skip if already following
                  {
                     ++tag_b_adjacency_itr;
                  }
                  else
                  {
                     adj_b = tag_b_adjacency_itr->adjacency;
                     related_tag_b = tag_b_adjacency_itr->tag_a;
                  }
               }

               if( adj_a > adj_b )    // Add the most highly adjacent tag to the related tags vector
               {
                  related_tags.push_back( related_tag_a );
                  ++tag_a_adjacency_itr;
               }
               else
               {
                  related_tags.push_back( related_tag_b );
                  ++tag_b_adjacency_itr;
               }
            }
         }

         vector<account_name_type> total_authors;
         vector<board_name_type> total_boards;
         vector<tag_name_type> total_tags;

         total_authors.reserve( top_authors.size() + related_authors.size());
         total_board.reserve( top_boards.size() + related_boards.size() );
         total_tags.reserve( top_tags.size() + related_tags.size() );

         for( auto author : top_authors )
         {
            total_authors.push_back( author.first );
         }
         for( auto board : top_boards )
         {
            total_boards.push_back( board.first );
         }
         for( auto tag : top_tags )
         {
            total_tags.push_back( tag.first );
         }
         for( auto author : related_authors )
         {
            total_authors.push_back( author );
         }
         for( auto board : related_boards )
         {
            total_boards.push_back( board );
         }
         for( auto tag : related_tags )
         {
            total_tags.push_back( tag );
         }

         for( auto author : total_authors )   // Get the top 6 unviewed most recent, most voted, most viewed, most shared and most commented posts per author
         {
            auto blog_itr = blog_idx.lower_bound( boost::make_tuple( board_name_type(), tag_name_type(), author ) );
            uint32_t count = 0;
            while( blog_itr != blog_idx.end() && blog_itr->account == author && count < 4 )
            {
               if( view_idx.find( boost::make_tuple( account, blog_itr->comment ) ) == view_idx.end() )
               {
                  selected.insert( blog_itr->comment );
                  count++;
               }
               blog_itr++;
            }

            auto author_vote_itr = author_vote_idx.lower_bound( boost::make_tuple( board_name_type(), tag_name_type(), author ) );
            uint32_t count = 0;
            while( author_vote_itr != author_vote_idx.end() && author_vote_itr->account == author && count < 2 )
            {
               if( view_idx.find( boost::make_tuple( account, author_vote_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( author_vote_itr->comment );
                  count++;
               }
               author_vote_itr++;
            }

            auto author_view_itr = author_view_idx.lower_bound( boost::make_tuple( board_name_type(), tag_name_type(), author ) );
            uint32_t count = 0;
            while( author_view_itr != author_view_idx.end() && author_view_itr->account == author && count < 2 )
            {
               if( view_idx.find( boost::make_tuple( account, author_view_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( author_view_itr->comment );
                  count++;
               }
               author_view_itr++;
            }

            auto author_share_itr = author_share_idx.lower_bound( boost::make_tuple( board_name_type(), tag_name_type(), author ) );
            uint32_t count = 0;
            while( author_share_itr != author_share_idx.end() && author_share_itr->account == author && count < 2 )
            {
               if( view_idx.find( boost::make_tuple( account, author_share_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( author_share_itr->comment );
                  count++;
               }
               author_share_itr++;
            }

            auto author_comment_itr = author_comment_idx.lower_bound( boost::make_tuple( board_name_type(), tag_name_type(), author ) );
            uint32_t count = 0;
            while( author_comment_itr != author_comment_idx.end() && author_comment_itr->account == author && count < 2 )
            {
               if( view_idx.find( boost::make_tuple( account, author_comment_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( author_comment_itr->comment );
                  count++;
               }
               author_comment_itr++;
            }
         }

         for( auto board : total_boards )
         {
            auto popular_rapid_itr = popular_rapid_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( popular_rapid_itr != popular_rapid_idx.end() && popular_rapid_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, popular_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( popular_rapid_itr->comment );
                  break;
               }
               popular_rapid_itr++;
            }
            auto popular_top_itr = popular_top_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( popular_top_itr != popular_top_idx.end() && popular_top_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, popular_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( popular_top_itr->comment );
                  break;
               }
               popular_top_itr++;
            }
            auto viral_rapid_itr = viral_rapid_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( viral_rapid_itr != viral_rapid_idx.end() && viral_rapid_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, viral_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( viral_rapid_itr->comment );
                  break;
               }
               viral_rapid_itr++;
            }
            auto viral_top_itr = viral_top_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( viral_top_itr != viral_top_idx.end() && viral_top_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, viral_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( viral_top_itr->comment );
                  break;
               }
               viral_top_itr++;
            }
            auto discussion_rapid_itr = discussion_rapid_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( discussion_rapid_itr != discussion_rapid_idx.end() && discussion_rapid_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, discussion_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discussion_rapid_itr->comment );
                  break;
               }
               discussion_rapid_itr++;
            }
            auto discussion_top_itr = discussion_top_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( discussion_top_itr != discussion_top_idx.end() && discussion_top_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, discussion_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discussion_top_itr->comment );
                  break;
               }
               discussion_top_itr++;
            }
            auto prominent_rapid_itr = prominent_rapid_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( prominent_rapid_itr != prominent_rapid_idx.end() && prominent_rapid_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, prominent_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( prominent_rapid_itr->comment );
                  break;
               }
               prominent_rapid_itr++;
            }
            auto prominent_top_itr = prominent_top_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( prominent_top_itr != prominent_top_idx.end() && prominent_top_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, prominent_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( prominent_top_itr->comment );
                  break;
               }
               prominent_top_itr++;
            }
            auto conversation_rapid_itr = conversation_rapid_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( conversation_rapid_itr != conversation_rapid_idx.end() && conversation_rapid_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, conversation_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( conversation_rapid_itr->comment );
                  break;
               }
               conversation_rapid_itr++;
            }
            auto conversation_top_itr = conversation_top_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( conversation_top_itr != conversation_top_idx.end() && conversation_top_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, conversation_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( conversation_top_itr->comment );
                  break;
               }
               conversation_top_itr++;
            }
            auto discourse_rapid_itr = discourse_rapid_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( discourse_rapid_itr != discourse_rapid_idx.end() && discourse_rapid_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, discourse_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discourse_rapid_itr->comment );
                  break;
               }
               discourse_rapid_itr++;
            }
            auto discourse_top_itr = discourse_top_idx.lower_bound( boost::make_tuple( board, tag_name_type() ) );
            while( discourse_top_itr != discourse_top_idx.end() && discourse_top_itr->board == board )
            {
               if( view_idx.find( boost::make_tuple( account, discourse_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discourse_top_itr->comment );
                  break;
               }
               discourse_top_itr++;
            }
         }

         for( auto tag : total_tags )
         {
            auto popular_rapid_itr = popular_rapid_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( popular_rapid_itr != popular_rapid_idx.end() && popular_rapid_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, popular_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( popular_rapid_itr->comment );
                  break;
               }
               popular_rapid_itr++;
            }
            auto popular_top_itr = popular_top_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( popular_top_itr != popular_top_idx.end() && popular_top_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, popular_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( popular_top_itr->comment );
                  break;
               }
               popular_top_itr++;
            }
            auto viral_rapid_itr = viral_rapid_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( viral_rapid_itr != viral_rapid_idx.end() && viral_rapid_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, viral_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( viral_rapid_itr->comment );
                  break;
               }
               viral_rapid_itr++;
            }
            auto viral_top_itr = viral_top_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( viral_top_itr != viral_top_idx.end() && viral_top_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, viral_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( viral_top_itr->comment );
                  break;
               }
               viral_top_itr++;
            }
            auto discussion_rapid_itr = discussion_rapid_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( discussion_rapid_itr != discussion_rapid_idx.end() && discussion_rapid_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, discussion_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discussion_rapid_itr->comment );
                  break;
               }
               discussion_rapid_itr++;
            }
            auto discussion_top_itr = discussion_top_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( discussion_top_itr != discussion_top_idx.end() && discussion_top_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, discussion_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discussion_top_itr->comment );
                  break;
               }
               discussion_top_itr++;
            }
            auto prominent_rapid_itr = prominent_rapid_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( prominent_rapid_itr != prominent_rapid_idx.end() && prominent_rapid_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, prominent_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( prominent_rapid_itr->comment );
                  break;
               }
               prominent_rapid_itr++;
            }
            auto prominent_top_itr = prominent_top_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( prominent_top_itr != prominent_top_idx.end() && prominent_top_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, prominent_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( prominent_top_itr->comment );
                  break;
               }
               prominent_top_itr++;
            }
            auto conversation_rapid_itr = conversation_rapid_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( conversation_rapid_itr != conversation_rapid_idx.end() && conversation_rapid_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, conversation_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( conversation_rapid_itr->comment );
                  break;
               }
               conversation_rapid_itr++;
            }
            auto conversation_top_itr = conversation_top_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( conversation_top_itr != conversation_top_idx.end() && conversation_top_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, conversation_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( conversation_top_itr->comment );
                  break;
               }
               conversation_top_itr++;
            }
            auto discourse_rapid_itr = discourse_rapid_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( discourse_rapid_itr != discourse_rapid_idx.end() && discourse_rapid_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, discourse_rapid_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discourse_rapid_itr->comment );
                  break;
               }
               discourse_rapid_itr++;
            }
            auto discourse_top_itr = discourse_top_idx.lower_bound( boost::make_tuple( board_name_type(), tag ) );
            while( discourse_top_itr != discourse_top_idx.end() && discourse_top_itr->tag == tag )
            {
               if( view_idx.find( boost::make_tuple( account, discourse_top_itr->comment ) ) == view_idx.end() )     
               {
                  selected.insert( discourse_top_itr->comment );
                  break;
               }
               discourse_top_itr++;
            }
         }
      }
      return selected;
   } FC_CAPTURE_LOG_AND_RETHROW( (c) ) }


   /**
    * Updates the recommended posts for an account
    * after voting, viewing, or sharing a post.
    * Reloads all recommendations up to once per hour.
    * Optionally, removes the comment from the recommendations set.
    */
   void update_recommendations( const comment_object& c, const account_name_type& account, bool remove_comment )const
   {
      const auto& recommendation_idx = _db.get_index< account_recommendations_index >().indices().get< by_account >();
      auto recommendation_itr = recommendation_idx.find( account );
      time_point now = _db.head_block_time();

      if( recommendation_itr != recommendation_idx.end() )
      {
         if( ( now - recommendation_itr->last_updated ) >= fc::hours(1) )   // Fully reload recommendations up to once per hour
         {
            _db.modify( *recommendation_itr, [&]( account_recommendations_object& o )
            {
               o.recommended_posts = get_recommendations( account );
               o.last_updated = now;
            });
         }
         else if( remove_comment && recommendation_itr->recommended_posts.find( c.id ) )    // Remove post from recommendations when viewed
         {
            _db.modify( *recommendation_itr, [&]( account_recommendations_object& o )
            {
               o.recommended_posts.erase( c.id );
            });
         }
      }
      else
      {
         _db.create< account_recommendations_object >( [&]( account_recommendations_object& o ) 
         {
            o.account = account;
            o.recommended_posts = get_recommendations( account );
            o.last_updated = now;
         });
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
      update_account_votes( _db.get_comment( op.author, op.permlink ), op );
      update_recommendations( _db.get_comment( op.author, op.permlink ), op.voter, false );
   }

   void operator()( const view_operation& op )const
   {
      update_tags( _db.get_comment( op.author, op.permlink ), _db.get_comment_metrics(), false );
      update_account_views( _db.get_comment( op.author, op.permlink ), op );
      update_recommendations( _db.get_comment( op.author, op.permlink ), op.viewer, true );
   }

   void operator()( const share_operation& op )const
   {
      update_tags( _db.get_comment( op.author, op.permlink ), _db.get_comment_metrics(), false );
      update_account_shares( _db.get_comment( op.author, op.permlink ), op );
      update_recommendations( _db.get_comment( op.author, op.permlink ), op.sharer, false );
   }

   void operator()( const account_follow_operation& op )const
   {
      update_account_adjacency( _db.get_account_following( op.follower ) );
   }

   void operator()( const board_subscribe_operation& op )const
   {
      update_board_adjacency( _db.get_board_member( op.board ) );
   }

   void operator()( const tag_follow_operation& op )const
   {
      update_tag_adjacency( _db.get_tag_following( op.tag ) );
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



void tags_plugin_impl::on_operation( const operation_notification& note ) 
{
   try    /// plugins shouldn't ever throw
   {
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
   add_plugin_index< tag_index                        >(db);
   add_plugin_index< tag_stats_index                  >(db);
   add_plugin_index< peer_stats_index                 >(db);
   add_plugin_index< account_curation_metrics_index   >(db);
   add_plugin_index< account_adjacency_index          >(db);
   add_plugin_index< board_adjacency_index            >(db);
   add_plugin_index< tag_adjacency_index              >(db);
   add_plugin_index< account_recommendations_index    >(db);
   add_plugin_index< author_tag_stats_index           >(db);
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
