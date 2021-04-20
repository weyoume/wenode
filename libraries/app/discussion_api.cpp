#include <node/app/api_context.hpp>
#include <node/app/application.hpp>
#include <node/app/database_api.hpp>

#include <node/protocol/get_config.hpp>

#include <node/chain/util/reward.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hex.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include <cctype>
#include <cfenv>
#include <iostream>

namespace node { namespace app {


   //========================//
   // === Discussion API === //
   //========================//


discussion database_api::get_content( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_content( author, permlink );
   });
}


discussion database_api_impl::get_content( string author, string permlink )const
{
   const auto& comment_idx = _db.get_index< comment_index >().indices().get< by_permlink >();
   auto itr = comment_idx.find( boost::make_tuple( author, permlink ) );
   if( itr != comment_idx.end() )
   {
      discussion results(*itr);

      comment_interaction_state cstate = get_comment_interactions( author, permlink );
      results.active_votes = cstate.votes;
      results.active_views = cstate.views;
      results.active_shares = cstate.shares;
      results.active_mod_tags = cstate.moderation;

      const comment_api_obj root( _db.get< comment_object, by_id >( results.root_comment ) );
      results.url = "/" + root.community + "/@" + root.author + "/" + root.permlink;
      results.root_title = root.title;
      if( root.id != results.id )
      {
         results.url += "#@" + results.author + "/" + results.permlink;
      }

      return results;
   }
   else
   {
      return discussion();
   }
}

vector< discussion > database_api::get_content_replies( string author, string permlink )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_content_replies( author, permlink );
   });
}

vector< discussion > database_api_impl::get_content_replies( string author, string permlink )const
{
   account_name_type acc_name = account_name_type( author );
   const auto& comment_idx = _db.get_index< comment_index >().indices().get< by_parent >();
   auto comment_itr = comment_idx.find( boost::make_tuple( acc_name, permlink ) );
   vector< discussion > results;

   while( comment_itr != comment_idx.end() && 
      comment_itr->parent_author == author && 
      to_string( comment_itr->parent_permlink ) == permlink )
   {
      results.push_back( discussion( *comment_itr ) );
      ++comment_itr;
   }
   return results;
}

vector< discussion > database_api::get_replies_by_last_update( account_name_type start_parent_author, string start_permlink, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_replies_by_last_update( start_parent_author, start_permlink, limit );
   });
}


vector< discussion > database_api_impl::get_replies_by_last_update( account_name_type start_parent_author, string start_permlink, uint32_t limit )const
{
   vector< discussion > results;

   limit = std::min( limit, uint32_t( 1000 ) );
   const auto& comment_idx = _db.get_index< comment_index >().indices().get< by_last_update >();
   auto comment_itr = comment_idx.begin();
   const account_name_type* parent_author = &start_parent_author;

   if( start_permlink.size() )
   {
      const comment_object& comment = _db.get_comment( start_parent_author, start_permlink );
      comment_itr = comment_idx.iterator_to( comment );
      parent_author = &comment.parent_author;
   }
   else if( start_parent_author.size() )
   {
      comment_itr = comment_idx.lower_bound( start_parent_author );
   }

   results.reserve( limit );

   while( comment_itr != comment_idx.end() && 
      results.size() < limit && 
      comment_itr->parent_author == *parent_author )
   {
      discussion d = get_discussion( comment_itr->id, false );
      results.push_back( d );
      ++comment_itr;
   }
   
   return results;
}

discussion database_api::get_discussion( comment_id_type id, uint32_t truncate_body )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussion( id, truncate_body );
   });
}

discussion database_api_impl::get_discussion( comment_id_type id, uint32_t truncate_body )const
{
   discussion d = _db.get( id );

   const comment_api_obj root( _db.get< comment_object, by_id >( d.root_comment ) );
   d.url = "/" + root.community + "/@" + root.author + "/" + root.permlink;
   d.root_title = root.title;
   if( root.id != d.id )
   {
      d.url += "#@" + d.author + "/" + d.permlink;
   }

   comment_interaction_state cstate = get_comment_interactions( d.author, d.permlink );
   
   d.active_votes = cstate.votes;
   d.active_views = cstate.views;
   d.active_shares = cstate.shares;
   d.active_mod_tags = cstate.moderation;
   d.body_length = d.body.size();
   
   if( truncate_body )
   {
      d.body = d.body.substr( 0, truncate_body );

      if( !fc::is_utf8( d.body ) )
      {
         d.body = fc::prune_invalid_utf8( d.body );
      }  
   }
   return d;
}

template< typename Index, typename StartItr >
vector< discussion > database_api::get_discussions( 
   const discussion_query& query,
   const string& community,
   const string& tag,
   comment_id_type parent,
   const Index& tidx,
   StartItr tidx_itr,
   uint32_t truncate_body,
   const std::function< bool( const comment_api_obj& ) >& filter,
   const std::function< bool( const comment_api_obj& ) >& exit,
   const std::function< bool( const tags::tag_object& ) >& tag_exit,
   bool ignore_parent
   )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions( query, community, tag, parent, tidx, tidx_itr, truncate_body, filter, exit, tag_exit, ignore_parent );
   });
}


template< typename Index, typename StartItr >
vector< discussion > database_api_impl::get_discussions( 
   const discussion_query& query,
   const string& community,
   const string& tag,
   comment_id_type parent,
   const Index& tidx, 
   StartItr tidx_itr,
   uint32_t truncate_body,
   const std::function< bool( const comment_api_obj& ) >& filter,
   const std::function< bool( const comment_api_obj& ) >& exit,
   const std::function< bool( const tags::tag_object& ) >& tag_exit,
   bool ignore_parent
   )const
{
   vector< discussion > results;

   if( !_db.has_index<tags::tag_index>() )
   {
      return results;
   }
      
   const auto& comment_tag_idx = _db.get_index< tags::tag_index >().indices().get< tags::by_comment >();
   const auto& governance_idx = _db.get_index< governance_member_index >().indices().get< by_account >();
   auto governance_itr = governance_idx.begin();
   comment_id_type start;

   if( query.start_author && query.start_permlink ) 
   {
      const comment_object& start_comment = _db.get_comment( *query.start_author, *query.start_permlink );
      start = start_comment.id;
      auto comment_tag_itr = comment_tag_idx.find( start );
      
      while( comment_tag_itr != comment_tag_idx.end() && 
         comment_tag_itr->comment == start )
      {
         if( comment_tag_itr->tag == tag && 
            comment_tag_itr->community == community )
         {
            tidx_itr = tidx.iterator_to( *comment_tag_itr );
            break;
         }
         ++comment_tag_itr;
      }
   }

   uint32_t count = query.limit;
   uint64_t itr_count = 0;
   uint64_t filter_count = 0;
   uint64_t exc_count = 0;
   uint64_t max_itr_count = 10 * query.limit;

   while( count > 0 && tidx_itr != tidx.end() )
   {
      ++itr_count;
      if( itr_count > max_itr_count )
      {
         wlog( "Maximum iteration count exceeded serving query: ${q}", ("q", query) );
         wlog( "count=${count}   itr_count=${itr_count}   filter_count=${filter_count}   exc_count=${exc_count}",
               ("count", count)("itr_count", itr_count)("filter_count", filter_count)("exc_count", exc_count) );
         break;
      }
      if( tidx_itr->tag != tag || tidx_itr->community != community || ( !ignore_parent && tidx_itr->parent != parent ) )
      {
         break;
      } 
      try
      {
         if( !query.include_private )
         {
            if( tidx_itr->encrypted )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.post_include_time.size() )
         {
            time_point now = _db.head_block_time();
            time_point created = tidx_itr->created;
            bool old_post = false;

            if( query.post_include_time == post_time_values[ int( post_time_type::ALL_TIME ) ] )
            {
               old_post = false;
            }
            else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_HOUR ) ] )
            {
               if( created + fc::hours(1) > now )
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_DAY ) ] )
            {
               if( created + fc::days(1) > now ) 
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_WEEK ) ] )
            {
               if( created + fc::days(7) > now ) 
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_MONTH ) ] )
            {
               if( created + fc::days(30) > now ) 
               {
                  old_post = true;
               }
            }
            else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_YEAR ) ] )
            {
               if( created + fc::days(365) > now ) 
               {
                  old_post = true;
               }
            }

            if( old_post )
            {
               ++tidx_itr;
               continue;
            }
         }
      
         if( tidx_itr->rating > query.max_rating  )
         {
            ++tidx_itr;
            continue;
         }
         
         if( query.select_authors.size() && query.select_authors.find( tidx_itr->author ) == query.select_authors.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.select_languages.size() && query.select_languages.find( tidx_itr->language ) == query.select_languages.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.select_communities.size() )
         {
            auto tag_itr = tidx.begin();
            auto comment_tag_itr = comment_tag_idx.find( tidx_itr->comment );
            
            if( comment_tag_itr != comment_tag_idx.end() && comment_tag_itr->comment == tidx_itr->comment )
            {
               tag_itr = tidx.iterator_to( *comment_tag_itr );
            }

            bool found = false;
            while( tag_itr != tidx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.select_communities.find( tag_itr->community ) != query.select_communities.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( !found )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.select_tags.size() )
         {
            auto tag_itr = tidx.begin();
            auto comment_tag_itr = comment_tag_idx.find( tidx_itr->comment );
            
            if( comment_tag_itr != comment_tag_idx.end() && comment_tag_itr->comment == tidx_itr->comment )
            {
               tag_itr = tidx.iterator_to( *comment_tag_itr );
            }

            bool found = false;
            while( tag_itr != tidx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.select_tags.find( tag_itr->tag ) != query.select_tags.end() )
               {
                  found = true;
                  break;
               }
               ++tag_itr;
            }
            if( !found )
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.filter_authors.size() && query.filter_authors.find( tidx_itr->author ) != query.filter_authors.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.filter_languages.size() && query.filter_languages.find( tidx_itr->language ) != query.filter_languages.end() )
         {
            ++tidx_itr;
            continue;
         }

         if( query.filter_communities.size() )
         {
            auto tag_itr = tidx.begin();
            auto comment_tag_itr = comment_tag_idx.find( tidx_itr->comment );
            
            if( comment_tag_itr != comment_tag_idx.end() && comment_tag_itr->comment == tidx_itr->comment )
            {
               tag_itr = tidx.iterator_to( *comment_tag_itr );
            }

            bool found = false;
            while( tag_itr != tidx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.filter_communities.find( tag_itr->community ) != query.filter_communities.end() )
               {
                  found = true; 
                  break;
               }
               ++tag_itr;
            }
            if( found ) 
            {
               ++tidx_itr;
               continue;
            }
         }

         if( query.filter_tags.size() )
         {
            auto tag_itr = tidx.begin();
            auto comment_tag_itr = comment_tag_idx.find( tidx_itr->comment );
            
            if( comment_tag_itr != comment_tag_idx.end() && comment_tag_itr->comment == tidx_itr->comment )
            {
               tag_itr = tidx.iterator_to( *comment_tag_itr );
            }

            bool found = false;
            while( tag_itr != tidx.end() && tag_itr->comment == tidx_itr->comment )
            {
               if( query.filter_tags.find( tag_itr->tag ) != query.filter_tags.end() )
               {
                  found = true;
                  break;
               }
               ++tag_itr;
            }
            if( found ) 
            {
               ++tidx_itr;
               continue;
            }
         }

         discussion d = get_discussion( tidx_itr->comment, truncate_body );
         vector< moderation_state > active_mod_tags;
         vector< account_name_type > accepted_moderators;

         if( query.account != account_name_type() )
         {
            governance_itr = governance_idx.find( query.account );
            if( governance_itr != governance_idx.end() && 
               governance_itr->account == query.account )
            {
               accepted_moderators.push_back( governance_itr->governance );
               ++governance_itr;
            }
         }
         
         if( d.community != community_name_type() )
         {
            const community_permission_object& community = _db.get_community_permission( d.community );
            for( account_name_type mod : community.moderators )
            {
               accepted_moderators.push_back( mod );
            }
         }

         bool filtered = false;
         
         for( moderation_state m : d.active_mod_tags )
         {
            if( std::find( accepted_moderators.begin(), accepted_moderators.end(), m.moderator ) != accepted_moderators.end() )
            {
               active_mod_tags.push_back( m );
               if( m.filter )
               {
                  filtered = true;
                  break;
               }
            }
         }

         if( filtered )
         {
            ++tidx_itr;
            continue;
         }

         moderation_state init_state;
         init_state.rating = d.rating;
         active_mod_tags.push_back( init_state );        // Inject author's own rating.

         std::sort( active_mod_tags.begin(), active_mod_tags.end(), [&]( moderation_state a, moderation_state b )
         {
            return a.rating < b.rating;
         });

         d.median_rating = active_mod_tags[ active_mod_tags.size() / 2 ].rating;

         if( d.median_rating > query.max_rating )        // Exclude if median rating is greater than maximum.
         {
            ++tidx_itr;
            continue;
         }

         results.push_back( d );

         if( filter( results.back() ) )
         {
            results.pop_back();
            ++filter_count;
         }
         else if( exit( results.back() ) || tag_exit( *tidx_itr ) )
         {
            results.pop_back();
            break;
         }
         else
         {
            --count;
         }  
      }
      catch ( const fc::exception& e )
      {
         ++exc_count;
         edump((e.to_detail_string()));
      }
      ++tidx_itr;
   }
   return results;
}

comment_id_type database_api::get_parent( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_parent( query );
   });
}


comment_id_type database_api_impl::get_parent( const discussion_query& query )const
{
   comment_id_type parent;
   if( query.parent_author && query.parent_permlink ) 
   {
      parent = _db.get_comment( *query.parent_author, *query.parent_permlink ).id;
   }
   return parent;
}

vector< discussion > database_api::get_discussions_by_sort_rank( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_sort_rank( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_sort_rank( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   string sort_option;
   string sort_time;

   if( query.sort_option.size() && query.sort_time.size() )
   {
      sort_option = query.sort_option;
      sort_time = query.sort_time;
   }

   sort_option_type option_type = sort_option_type::QUALITY_SORT;

   for( size_t i = 0; i < sort_option_values.size(); i++ )
   {
      if( sort_option == sort_option_values[ i ] )
      {
         option_type = sort_option_type( i );
         break;
      }
   }

   sort_time_type time_type = sort_time_type::STANDARD_TIME;

   for( size_t i = 0; i < sort_time_values.size(); i++ )
   {
      if( sort_time == sort_time_values[ i ] )
      {
         time_type = sort_time_type( i );
         break;
      }
   }

   switch( option_type )
   {
      case sort_option_type::QUALITY_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_quality_sort_index >().indices().get< tags::by_parent_quality_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::VOTES_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_votes_sort_index >().indices().get< tags::by_parent_votes_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::VIEWS_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_views_sort_index >().indices().get< tags::by_parent_views_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::SHARES_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_shares_sort_index >().indices().get< tags::by_parent_shares_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::COMMENTS_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_comments_sort_index >().indices().get< tags::by_parent_comments_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::POPULAR_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_popular_sort_index >().indices().get< tags::by_parent_popular_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::VIRAL_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_viral_sort_index >().indices().get< tags::by_parent_viral_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::DISCUSSION_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discussion_sort_index >().indices().get< tags::by_parent_discussion_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::PROMINENT_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_prominent_sort_index >().indices().get< tags::by_parent_prominent_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::CONVERSATION_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_conversation_sort_index >().indices().get< tags::by_parent_conversation_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      case sort_option_type::DISCOURSE_SORT:
      {
         switch( time_type )
         {
            case sort_time_type::ACTIVE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_active >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::RAPID_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_rapid >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::STANDARD_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_standard >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::TOP_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_top >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            case sort_time_type::ELITE_TIME:
            {
               const auto& tag_sort_index = _db.get_index< tags::tag_discourse_sort_index >().indices().get< tags::by_parent_discourse_elite >();
               auto tag_sort_itr = tag_sort_index.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<double>::max() ) );
               return get_discussions( query, community, tag, parent, tag_sort_index, tag_sort_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
            }
            break;
            default:
            {
               return vector< discussion >();
            }
            break;
         }
      }
      break;
      default:
      {
         return vector< discussion >();
      }
      break;
   }
}


vector< discussion > database_api::get_discussions_by_feed( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_feed( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_feed( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   auto start_author = query.start_author ? *( query.start_author ) : "";
   auto start_permlink = query.start_permlink ? *( query.start_permlink ) : "";
   string account;
   if( query.account.size() )
   {
      account = query.account;
   }
   else
   {
      return vector< discussion >();
   }

   const auto& tag_idx = _db.get_index< tags::tag_index >().indices().get<tags::by_comment>();
   const auto& f_idx = _db.get_index< comment_feed_index >().indices().get< by_new_account_type >();
   auto comment_feed_itr = f_idx.lower_bound( account );
   feed_reach_type reach = feed_reach_type::FOLLOW_FEED;

   if( query.feed_type.size() )
   {
      for( size_t i = 0; i < feed_reach_values.size(); i++ )
      {
         if( query.feed_type == feed_reach_values[ i ] )
         {
            reach = feed_reach_type( i );
            break;
         }
      }

      comment_feed_itr = f_idx.lower_bound( boost::make_tuple( account, reach ) );
   }

   const auto& comment_feed_idx = _db.get_index< comment_feed_index >().indices().get< by_comment >();
   if( start_author.size() || start_permlink.size() )
   {
      const comment_object& com = _db.get_comment( start_author, start_permlink );
      auto start_c = comment_feed_idx.find( com.id );
      FC_ASSERT( start_c != comment_feed_idx.end(),
         "Comment is not in account's feed" );
      comment_feed_itr = f_idx.iterator_to( *start_c );
   }

   vector< discussion > results;
   results.reserve( query.limit );

   while( results.size() < query.limit && comment_feed_itr != f_idx.end() )
   {
      if( comment_feed_itr->account != account )
      {
         break;
      }

      if( query.post_include_time.size() )
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         time_point now = _db.head_block_time();
         time_point created = tag_itr->created;
         bool old_post = false;

         if( query.post_include_time == post_time_values[ int( post_time_type::ALL_TIME ) ] )
         {
            old_post = false;
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_HOUR ) ] )
         {
            if( created + fc::hours(1) > now )
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_DAY ) ] )
         {
            if( created + fc::days(1) > now ) 
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_WEEK ) ] )
         {
            if( created + fc::days(7) > now ) 
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_MONTH ) ] )
         {
            if( created + fc::days(30) > now ) 
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_YEAR ) ] )
         {
            if( created + fc::days(365) > now ) 
            {
               old_post = true;
            }
         }

         if( old_post )
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( !query.include_private )
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );
         if( tag_itr->encrypted )
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.max_rating <= 9 )
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         if( tag_itr->rating > query.max_rating )
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.select_authors.size() )
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         if( query.select_authors.find( tag_itr->author ) == query.select_authors.end() )
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.select_languages.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_feed_itr->comment )
         {
            if( query.select_languages.find( tag_itr->language ) != query.select_languages.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( !found ) 
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.select_communities.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_feed_itr->comment )
         {
            if( query.select_communities.find( tag_itr->community ) != query.select_communities.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( !found ) 
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.select_tags.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_feed_itr->comment )
         {
            if( query.select_tags.find( tag_itr->tag ) != query.select_tags.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( !found ) 
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.filter_authors.size()  )
      {
         if( query.filter_authors.find( comment_feed_itr->account ) != query.filter_authors.end() )
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.filter_languages.size() )
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_feed_itr->comment )
         {
            if( query.filter_languages.find( tag_itr->language ) != query.filter_languages.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( found ) 
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.filter_communities.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_feed_itr->comment )
         {
            if( query.filter_communities.find( tag_itr->community ) != query.filter_communities.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( found ) 
         {
            ++comment_feed_itr;
            continue;
         }
      }

      if( query.filter_tags.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_feed_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_feed_itr->comment )
         {
            if( query.filter_tags.find( tag_itr->tag ) != query.filter_tags.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( found ) 
         {
            ++comment_feed_itr;
            continue;
         }
      }

      try
      {
         results.push_back( get_discussion( comment_feed_itr->comment, query.truncate_body ) );
         results.back().feed = comment_feed_api_obj( *comment_feed_itr );
      }
      catch ( const fc::exception& e )
      {
         edump((e.to_detail_string()));
      }

      ++comment_feed_itr;
   }

   return results;
}

vector< discussion > database_api::get_discussions_by_blog( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_blog( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_blog( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();

   string start_author = query.start_author ? *( query.start_author ) : "";
   string start_permlink = query.start_permlink ? *( query.start_permlink ) : "";

   string account;
   string community;
   string tag;

   if( query.account.size() )
   {
      account = query.account;
      const account_object& acc_obj = _db.get_account( account );
      if( !acc_obj.active )
      {
         return vector< discussion >();
      }
   }

   if( query.community.size() )
   {
      community = query.community;
      const community_object& community_obj = _db.get_community( community );
      if( !community_obj.active )
      {
         return vector< discussion >();
      }
   }

   if( query.tag.size() )
   {
      tag = query.tag;
   }

   blog_reach_type reach_type = blog_reach_type::ACCOUNT_BLOG;

   for( size_t i = 0; i < blog_reach_values.size(); i++ )
   {
      if( query.blog_type == blog_reach_values[ i ] )
      {
         reach_type = blog_reach_type( i );
         break;
      }
   }

   const auto& tag_idx = _db.get_index< tags::tag_index >().indices().get<tags::by_comment>();
   const auto& comment_blog_idx = _db.get_index< comment_blog_index >().indices().get< by_comment >();
   auto comment_blog_itr = comment_blog_idx.begin();

   switch( reach_type )
   {
      case blog_reach_type::ACCOUNT_BLOG:
      {
         const auto& account_comment_blog_idx = _db.get_index< comment_blog_index >().indices().get< by_new_account_blog >();
         auto comment_blog_itr = account_comment_blog_idx.lower_bound( account );
         if( start_author.size() || start_permlink.size() )
         {
            const comment_object& com = _db.get_comment( start_author, start_permlink );
            auto start_c = comment_blog_idx.find( com.id );
            FC_ASSERT( start_c != comment_blog_idx.end(),
               "Comment is not in account's blog" );
            comment_blog_itr = account_comment_blog_idx.iterator_to( *start_c );
         }
      }
      break;
      case blog_reach_type::COMMUNITY_BLOG:
      {
         const auto& community_comment_blog_idx = _db.get_index< comment_blog_index >().indices().get< by_new_community_blog >();
         auto comment_blog_itr = community_comment_blog_idx.lower_bound( community );
         if( start_author.size() || start_permlink.size() )
         {
            const comment_object& com = _db.get_comment( start_author, start_permlink );
            auto start_c = comment_blog_idx.find( com.id );
            FC_ASSERT( start_c != comment_blog_idx.end(),
               "Comment is not in community's blog" );
            comment_blog_itr = community_comment_blog_idx.iterator_to( *start_c );
         }
      }
      break;
      case blog_reach_type::TAG_BLOG:
      {
         const auto& tag_comment_blog_idx = _db.get_index< comment_blog_index >().indices().get< by_new_tag_blog >();
         auto comment_blog_itr = tag_comment_blog_idx.lower_bound( tag );
         if( start_author.size() || start_permlink.size() )
         {
            const comment_object& com = _db.get_comment( start_author, start_permlink );
            auto start_c = comment_blog_idx.find( com.id );
            FC_ASSERT( start_c != comment_blog_idx.end(),
               "Comment is not in tag's blog" );
            comment_blog_itr = tag_comment_blog_idx.iterator_to( *start_c );
         }
      }
      break;
      default:
      {
         return vector< discussion >();
      }
   }

   vector< discussion > results;
   results.reserve( query.limit );

   while( results.size() < query.limit && comment_blog_itr != comment_blog_idx.end() )
   { 
      if( account.size() && comment_blog_itr->account != account && query.blog_type == blog_reach_values[ int( blog_reach_type::ACCOUNT_BLOG ) ] )
      {
         break;
      }
      if( community.size() && comment_blog_itr->community != community && query.blog_type == blog_reach_values[ int( blog_reach_type::COMMUNITY_BLOG ) ] )
      {
         break;
      }
      if( tag.size() && comment_blog_itr->tag != tag && query.blog_type == blog_reach_values[ int( blog_reach_type::TAG_BLOG ) ] )
      {
         break;
      }

      if( query.post_include_time.size() )
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         time_point now = _db.head_block_time();
         time_point created = tag_itr->created;
         bool old_post = false;

         if( query.post_include_time == post_time_values[ int( post_time_type::ALL_TIME ) ] )
         {
            old_post = false;
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_HOUR ) ] )
         {
            if( created + fc::hours(1) > now )
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_DAY ) ] )
         {
            if( created + fc::days(1) > now ) 
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_WEEK ) ] )
         {
            if( created + fc::days(7) > now ) 
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_MONTH ) ] )
         {
            if( created + fc::days(30) > now ) 
            {
               old_post = true;
            }
         }
         else if( query.post_include_time == post_time_values[ int( post_time_type::LAST_YEAR ) ] )
         {
            if( created + fc::days(365) > now ) 
            {
               old_post = true;
            }
         }

         if( old_post )
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( !query.include_private )
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );
         if( tag_itr->encrypted )
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( query.max_rating <= 9 )
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         if( tag_itr->rating > query.max_rating )
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( query.select_authors.size() && query.select_authors.find( comment_blog_itr->account ) == query.select_authors.end() )
      {
         ++comment_blog_itr;
         continue;
      }

      if( query.select_languages.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_blog_itr->comment )
         {
            if( query.select_languages.find( tag_itr->language ) != query.select_languages.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( !found ) 
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( query.select_communities.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_blog_itr->comment )
         {
            if( query.select_communities.find( tag_itr->community ) != query.select_communities.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( !found ) 
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( query.select_tags.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_blog_itr->comment )
         {
            if( query.select_tags.find( tag_itr->tag ) != query.select_tags.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( !found ) 
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( query.filter_authors.size() && query.filter_authors.find( comment_blog_itr->account ) != query.filter_authors.end() )
      {
         ++comment_blog_itr;
         continue;
      }

      if( query.filter_languages.size() )
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_blog_itr->comment )
         {
            if( query.filter_languages.find( tag_itr->language ) != query.filter_languages.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( found ) 
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( query.filter_communities.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_blog_itr->comment )
         {
            if( query.filter_communities.find( tag_itr->community ) != query.filter_communities.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( found ) 
         {
            ++comment_blog_itr;
            continue;
         }
      }

      if( query.filter_tags.size() ) 
      {
         auto tag_itr = tag_idx.lower_bound( comment_blog_itr->comment );

         bool found = false;
         while( tag_itr != tag_idx.end() && tag_itr->comment == comment_blog_itr->comment )
         {
            if( query.filter_tags.find( tag_itr->tag ) != query.filter_tags.end() )
            {
               found = true; 
               break;
            }
            ++tag_itr;
         }
         if( found ) 
         {
            ++comment_blog_itr;
            continue;
         }
      }

      try
      {
         results.push_back( get_discussion( comment_blog_itr->comment, query.truncate_body ) );
         results.back().blog = comment_blog_api_obj( *comment_blog_itr );
      }
      catch ( const fc::exception& e )
      {
         edump((e.to_detail_string()));
      }

      ++comment_blog_itr;
   }

   return results;
}


vector< discussion > database_api::get_discussions_by_featured( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_featured( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_featured( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_featured>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, fc::time_point::maximum() ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}


vector< discussion > database_api::get_discussions_by_recommended( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_recommended( query );
   });
}

/**
 * Recommended Feed is generated with a psudeorandom 
 * ordering of posts from the authors, 
 * communities, and tags that the account has previously 
 * interacted with.
 * Pulls the top posts from each sorting index
 * of each author blog, community, and tag that the
 * account has previously interacted with.
 * Adds the top posts by each index from the highest adjacency 
 * authors, communities and tags that are currently not followed by the account.
 * Randomly pulls the limit amount of posts from
 * this set of posts.
 */
vector< discussion > database_api_impl::get_discussions_by_recommended( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   vector< discussion > results;
   results.reserve( query.limit );

   query.validate();
   if( query.account.size() )
   {
      account_name_type account = query.account;
      const auto& recommend_idx = _db.get_index< tags::account_recommendations_index >().indices().get< tags::by_account >();
      auto recommend_itr = recommend_idx.find( account );

      if( recommend_itr != recommend_idx.end() )
      {
         const tags::account_recommendations_object& aro = *recommend_itr;
         vector< comment_id_type > recommended_posts;
         recommended_posts.reserve( aro.recommended_posts.size() );

         for( auto post : aro.recommended_posts )
         {
            recommended_posts.push_back( post );
         }

         auto now_hi = uint64_t( _db.head_block_time().time_since_epoch().count() ) << 32;
         for( uint32_t i = 0; i < query.limit; ++i )
         {
            uint64_t k = now_hi +      uint64_t(i)*2685757105773633871ULL;
            uint64_t l = ( now_hi >> 1 ) + uint64_t(i)*9519819187187829351ULL;
            uint64_t m = ( now_hi >> 2 ) + uint64_t(i)*5891972902484196198ULL;
            uint64_t n = ( now_hi >> 3 ) + uint64_t(i)*2713716410970705441ULL;
         
            k ^= (l >> 7);
            l ^= (m << 9);
            m ^= (n >> 5);
            n ^= (k << 3);

            k*= 1422657256589674161ULL;
            l*= 9198587865873687103ULL;
            m*= 3060558831167252908ULL;
            n*= 4306921374257631524ULL;

            k ^= (l >> 2);
            l ^= (m << 4);
            m ^= (n >> 1);
            n ^= (k << 9);

            k*= 7947775653275249570ULL;
            l*= 9490802558828203479ULL;
            m*= 2694198061645862341ULL;
            n*= 3190223686201138213ULL;

            uint64_t rand = (k ^ l) ^ (m ^ n) ; 
            uint32_t max = recommended_posts.size() - i;

            uint32_t j = i + rand % max;
            std::swap( recommended_posts[i], recommended_posts[j] );
            results.push_back( get_discussion( recommended_posts[i], query.truncate_body ) );    // Returns randomly selected comments from the recommended posts collection
         }
      }
      else
      {
         return vector< discussion >();
      }
   }
   else
   {
      return vector< discussion >();
   }

   return results;
}

vector< discussion > database_api::get_discussions_by_comments( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_comments( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_comments( const discussion_query& query )const
{
   vector< discussion > results;
   query.validate();
   FC_ASSERT( query.start_author,
      "Must get comments for a specific author" );
   string start_author = *( query.start_author );
   string start_permlink = query.start_permlink ? *( query.start_permlink ) : "";

   const auto& comment_idx = _db.get_index< comment_index >().indices().get< by_permlink >();
   const auto& t_idx = _db.get_index< comment_index >().indices().get< by_author_last_update >();
   auto comment_itr = t_idx.lower_bound( start_author );

   if( start_permlink.size() )
   {
      auto start_c = comment_idx.find( boost::make_tuple( start_author, start_permlink ) );
      FC_ASSERT( start_c != comment_idx.end(),
         "Comment is not in account's comments" );
      comment_itr = t_idx.iterator_to( *start_c );
   }

   results.reserve( query.limit );

   while( results.size() < query.limit && comment_itr != t_idx.end() )
   {
      if( comment_itr->author != start_author )
      {
         break;
      } 
      if( comment_itr->parent_author.size() > 0 )
      {
         try
         {
            results.push_back( get_discussion( comment_itr->id, query.truncate_body ) );
         }
         catch( const fc::exception& e )
         {
            edump( (e.to_detail_string() ) );
         }
      }

      ++comment_itr;
   }
   return results;
}


vector< discussion > database_api::get_discussions_by_payout( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_payout( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_payout( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index< tags::tag_index >().indices().get< tags::by_net_reward >();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
}

vector< discussion > database_api::get_post_discussions_by_payout( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_post_discussions_by_payout( query );
   });
}

vector< discussion > database_api_impl::get_post_discussions_by_payout( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = comment_id_type();    // Root posts

   const auto& tidx = _db.get_index< tags::tag_index >().indices().get< tags::by_reward_fund_net_reward >();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, true ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
}

vector< discussion > database_api::get_comment_discussions_by_payout( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_comment_discussions_by_payout( query );
   });
}

vector< discussion > database_api_impl::get_comment_discussions_by_payout( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = comment_id_type(1);

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_reward_fund_net_reward>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, false ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, []( const comment_api_obj& c ){ return c.net_reward <= 0; }, exit_default, tag_exit_default, true );
}



vector< discussion > database_api::get_discussions_by_created( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_created( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_created( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_created>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, fc::time_point::maximum() )  );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_active( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_active( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_active( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_active>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, fc::time_point::maximum() )  );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_votes( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_votes( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_votes( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_net_votes>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_views( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_views( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_views( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_view_count>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() )  );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_shares( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_shares( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_shares( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_share_count>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() )  );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_children( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_children( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_children( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_children>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() )  );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_vote_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_vote_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_vote_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_vote_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_view_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_view_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_view_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_view_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}


vector< discussion > database_api::get_discussions_by_share_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_share_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_share_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_share_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

vector< discussion > database_api::get_discussions_by_comment_power( const discussion_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_discussions_by_comment_power( query );
   });
}

vector< discussion > database_api_impl::get_discussions_by_comment_power( const discussion_query& query )const
{
   if( !_db.has_index< tags::tag_index >() )
   {
      return vector< discussion >();
   }

   query.validate();
   string community = fc::to_lower( query.community );
   string tag = fc::to_lower( query.tag );
   comment_id_type parent = get_parent( query );

   const auto& tidx = _db.get_index<tags::tag_index>().indices().get<tags::by_parent_comment_power>();
   auto tidx_itr = tidx.lower_bound( boost::make_tuple( community, tag, parent, std::numeric_limits<int32_t>::max() ) );

   return get_discussions( query, community, tag, parent, tidx, tidx_itr, query.truncate_body, filter_default, exit_default, tag_exit_default, false );
}

/**
 * This call assumes root already stored as part of state, it will
 * modify root. Replies to contain links to the reply posts and then
 * add the reply discussions to the state. This method also fetches
 * any accounts referenced by authors.
 */
void database_api::recursively_fetch_content( state& _state, discussion& root, set< string >& referenced_accounts )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->recursively_fetch_content( _state, root, referenced_accounts );
   });
}

void database_api_impl::recursively_fetch_content( state& _state, discussion& root, set< string >& referenced_accounts )const
{
   try
   {
      if( root.author.size() )
      {
         referenced_accounts.insert( root.author );
      }
      
      vector< discussion > replies = get_content_replies( root.author, root.permlink );

      for( auto& r : replies )
      {
         recursively_fetch_content( _state, r, referenced_accounts );
         root.replies.push_back( r.author + "/" + r.permlink );
         _state.content[ r.author + "/" + r.permlink ] = std::move(r);

         if( r.author.size() )
         {
            referenced_accounts.insert( r.author );
         }
      }
   }
   FC_CAPTURE_AND_RETHROW( (root.author)(root.permlink) )
}

state database_api::get_state( string path )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_state( path );
   });
}

state database_api_impl::get_state( string path )const
{
   state _state;
   _state.props = _db.get_dynamic_global_properties();
   _state.current_route = path;

   try {
      if( path.size() && path[0] == '/' )
      {
         path = path.substr(1);   // remove '/' from front
      }
      
      vector< tag_api_obj > trending_tags = get_top_tags( std::string(), 50 );

      for( const auto& t : trending_tags )
      {
         _state.tag_idx.trending.push_back( string( t.tag ) );    // Trending tags record of highest voted tags
      }

      set< string > accounts;
      vector< string > part;
      part.reserve(4);
      boost::split( part, path, boost::is_any_of("/") );
      part.resize( std::max( part.size(), size_t(4) ) );
      string account;
      string community;
      string tag;
      string section;

      for( auto item : part )
      {
         if( item[0] == '@' )
         {
            string account = fc::to_lower( item.substr(1) );
            vector< string > accvec;
            accvec.push_back( account );
            _state.accounts[ account ] = get_full_accounts( accvec )[0];
         }
         else if( item[0] == '&' )
         {
            string community = fc::to_lower( item.substr(1) );
            vector< string > communityvec;
            communityvec.push_back( community );
            _state.communities[ community ] = get_communities( communityvec )[0];
         }
         else if( item[0] == '#' )
         {
            string tag = fc::to_lower( item.substr(1) );
            vector< string > tagvec;
            tagvec.push_back( tag );
            _state.tags[ tag ] = get_account_tag_followings( tagvec )[0];
         }
         else
         {
            string section = fc::to_lower( item.substr(1) );
         }
      }

      if( section == "recent-replies" )
      {
         vector< discussion > replies = get_replies_by_last_update( account, "", 50 );
         _state.recent_replies[ account ] = vector< string >();

         for( const auto& reply : replies )
         {
            string reply_ref = reply.author + "/" + reply.permlink;
            _state.content[ reply_ref ] = reply;
            _state.recent_replies[ account ].push_back( reply_ref );
         }
      }
      else if( section == "posts" || section == "comments" )
      {
         int count = 0;
         const auto& comment_idx = _db.get_index< comment_index >().indices().get< by_author_last_update >();
         auto comment_itr = comment_idx.lower_bound( account );
         _state.comments[ account ] = vector< string >();

         while( comment_itr != comment_idx.end() && 
            comment_itr->author == account && count < 20 )
         {
            if( comment_itr->parent_author.size() )
            {
               string link = account + "/" + to_string( comment_itr->permlink );
               _state.recent_replies[ account ].push_back( link );
               _state.content[ link ] = *comment_itr;
               ++count;
            }
            ++comment_itr;
         }
      }
      else if( section == "blog" )
      {
         discussion_query q;
         q.account = account;
         q.blog_type = blog_reach_values[ int( blog_reach_type::ACCOUNT_BLOG ) ];
         vector< discussion > blog_posts = get_discussions_by_blog( q );
         _state.blogs[ account ] = vector< string >();

         for( auto b : blog_posts )
         {
            string link = b.author + "/" + b.permlink;
            _state.blogs[ account ].push_back( link );
            _state.content[ link ] = b;
         }
      }
      else if( section == "feed" )
      {
         discussion_query q;
         q.account = account;
         q.feed_type = feed_reach_values[ int( feed_reach_type::FOLLOW_FEED ) ];
         vector< discussion > feed_posts = get_discussions_by_feed( q );
         _state.blogs[ account ] = vector< string >();

         for( auto f: feed_posts )
         {
            string link = f.author + "/" + f.permlink;
            _state.feeds[ account ].push_back( link );
            _state.content[ link ] = f;
         }
      }
      else if( section == "voting_producers" || section == "~voting_producers")
      {
         vector< producer_api_obj > producers = get_producers_by_voting_power( "", 50 );
         for( const auto& p : producers )
         {
            _state.voting_producers[ p.owner ] = p;
         }
      }
      else if( section == "mining_producers" || section == "~mining_producers")
      {
         vector< producer_api_obj > producers = get_producers_by_mining_power( "", 50 );
         for( const auto& p : producers )
         {
            _state.mining_producers[ p.owner ] = p;
         }
      }
      else if( section == "communities" || section == "~communities")
      {
         vector< extended_community > communities = get_communities_by_subscribers( "", 50 );
         for( const auto& b : communities )
         {
            _state.communities[ b.name ] = b;
         }
      }
      else if( section == "payout" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         vector< discussion > trending_disc = get_post_discussions_by_payout( q );

         for( const auto& d : trending_disc )
         {
            string key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].payout.push_back( key );

            if( d.author.size() )
            {
               accounts.insert( d.author );
            } 
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "payout_comments" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         vector< discussion > trending_disc = get_comment_discussions_by_payout( q );

         for( const auto& d : trending_disc )
         {
            string key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].payout_comments.push_back( key );

            if( d.author.size() )
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "responses" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         vector< discussion > trending_disc = get_discussions_by_children( q );

         for( const auto& d : trending_disc )
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].responses.push_back( key );

            if( d.author.size() )
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "net_votes" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_votes( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].net_votes.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert (d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "view_count" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_views( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].view_count.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "share_count" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_shares( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].share_count.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "comment_count" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_children( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].comment_count.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "vote_power" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_vote_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].vote_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "view_power" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_view_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].view_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "share_power" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_share_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].share_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "comment_power" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_comment_power( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].comment_power.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "active" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_active( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].active.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "created" )
      {
         discussion_query q;
         q.community = community;
         q.tag = tag;
         q.limit = 20;
         q.truncate_body = 1024;
         auto trending_disc = get_discussions_by_created( q );

         for( const auto& d : trending_disc ) 
         {
            auto key = d.author + "/" + d.permlink;
            _state.discussion_idx[ tag ].created.push_back( key );

            if( d.author.size() ) 
            {
               accounts.insert( d.author );
            }
            _state.content[ key ] = std::move( d );
         }
      }
      else if( section == "tags" )
      {
         _state.tag_idx.trending.clear();
         auto trending_tags = get_top_tags( std::string(), 250 );
         for( const auto& t : trending_tags )
         {
            string name = t.tag;
            _state.tag_idx.trending.push_back( name );
            _state.tags[ name ] = account_tag_following_api_obj( _db.get_account_tag_following( name ) );
         }
      }
      else if( account.size() && section.size() )
      {
         string permlink = section;
         string key = account + "/" + permlink;
         discussion dis = get_content( account, permlink );
         recursively_fetch_content( _state, dis, accounts );
         _state.content[ key ] = std::move( dis );
      }

      for( const auto& a : accounts )
      {
         _state.accounts.erase("");
         _state.accounts[ a ] = extended_account( _db.get_account( a ), _db );
      }
      for( auto& d : _state.content ) 
      {
         comment_interaction_state cstate = get_comment_interactions( d.second.author, d.second.permlink );

         d.second.active_votes = cstate.votes;
         d.second.active_views = cstate.views;
         d.second.active_shares = cstate.shares;
         d.second.active_mod_tags = cstate.moderation;
         d.second.body_length = d.second.body.size();
      }

      _state.producer_schedule = _db.get_producer_schedule();
   }
   catch ( const fc::exception& e ) 
   {
      _state.error = e.to_detail_string();
   }
   return _state;
}


} } // node::app