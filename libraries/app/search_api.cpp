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


   //====================//
   // === Search API === //
   //====================//


search_result_state database_api::get_search_query( const search_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_search_query( query );
   });
}

/**
 * Retreives a series of accounts, communities, tags, assets and posts according to the 
 * lowest edit distance between the search query and the names of the objects.
 */
search_result_state database_api_impl::get_search_query( const search_query& query )const
{
   search_result_state results;
   string q = query.query;
   size_t margin = ( ( q.size() * query.margin_percent ) / PERCENT_100 ) + 1;

   const auto& account_idx = _db.get_index< account_index >().indices().get< by_name >();
   const auto& community_idx = _db.get_index< community_index >().indices().get< by_name >();
   const auto& tag_idx = _db.get_index< account_tag_following_index >().indices().get< by_tag >();
   const auto& asset_idx = _db.get_index< asset_index >().indices().get< by_symbol >();
   const auto& post_idx = _db.get_index< comment_index >().indices().get< by_title >();

   auto account_itr = account_idx.begin();
   auto community_itr = community_idx.begin();
   auto tag_itr = tag_idx.begin();
   auto asset_itr = asset_idx.begin();
   auto post_itr = post_idx.upper_bound( string() );    // Skip index posts with no title

   vector< pair< account_name_type, size_t > > accounts;
   vector< pair< community_name_type, size_t > > communities;
   vector< pair< tag_name_type, size_t > > tags;
   vector< pair< asset_symbol_type, size_t > > assets;
   vector< pair< string, size_t > > posts;

   accounts.reserve( account_idx.size() );
   communities.reserve( community_idx.size() );
   tags.reserve( tag_idx.size() );
   assets.reserve( asset_idx.size() );
   posts.reserve( post_idx.size() );

   results.accounts.reserve( query.limit );
   results.communities.reserve( query.limit );
   results.tags.reserve( query.limit );
   results.assets.reserve( query.limit );
   results.posts.reserve( query.limit );

   // Finds items that are within the specified margin of error by edit distance from the search term.
   // Sort items in Ascending order, lowest edit distance first.

   if( query.include_accounts )
   {
      while( account_itr != account_idx.end() )
      {
         size_t edit_dist = protocol::edit_distance( string( account_itr->name ), q );
         if( edit_dist <= margin )
         {
            accounts.push_back( std::make_pair( account_itr->name, edit_dist  ) );
         }
         ++account_itr;
      }

      std::sort( accounts.begin(), accounts.end(), [&]( pair< account_name_type, size_t > a, pair< account_name_type, size_t > b )
      {
         return a.second > b.second;
      });

      for( auto item : accounts )
      {
         if( results.accounts.size() < query.limit )
         {
            account_itr = account_idx.find( item.first );
            results.accounts.push_back( account_api_obj( *account_itr, _db ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_communities )
   {
      while( community_itr != community_idx.end() )
      {
         size_t edit_dist = protocol::edit_distance( string( community_itr->name ), q );
         if( edit_dist <= margin )
         {
            communities.push_back( std::make_pair( community_itr->name, edit_dist ) );
         }
         ++community_itr;
      }

      std::sort( communities.begin(), communities.end(), [&]( pair< community_name_type, size_t > a, pair< community_name_type, size_t > b )
      {
         return a.second > b.second;
      });

      for( auto item : communities )
      {
         if( results.communities.size() < query.limit )
         {
            community_itr = community_idx.find( item.first );
            results.communities.push_back( community_api_obj( *community_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_tags )
   {
      while( tag_itr != tag_idx.end() )
      {
         size_t edit_dist = protocol::edit_distance( string( tag_itr->tag ), q );
         if( edit_dist <= margin )
         {
            tags.push_back( std::make_pair( tag_itr->tag, edit_dist ) );
         }
         ++tag_itr;
      }

      std::sort( tags.begin(), tags.end(), [&]( pair< tag_name_type, size_t > a, pair< tag_name_type, size_t > b )
      {
         return a.second > b.second;
      });

      for( auto item : tags )
      {
         if( results.tags.size() < query.limit )
         {
            tag_itr = tag_idx.find( item.first );
            results.tags.push_back( account_tag_following_api_obj( *tag_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_assets )
   {
      while( asset_itr != asset_idx.end() )
      {
         size_t edit_dist = protocol::edit_distance( string( asset_itr->symbol ), q );
         if( edit_dist <= margin )
         {
            assets.push_back( std::make_pair( asset_itr->symbol, edit_dist ) );
         }
         ++asset_itr;
      }

      std::sort( assets.begin(), assets.end(), [&]( pair< asset_symbol_type, size_t > a, pair< asset_symbol_type, size_t > b )
      {
         return a.second > b.second;
      });

      for( auto item : assets )
      {
         if( results.assets.size() < query.limit )
         {
            asset_itr = asset_idx.find( item.first );
            results.assets.push_back( asset_api_obj( *asset_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   if( query.include_posts )
   {
      while( post_itr != post_idx.end() )
      {
         string title = to_string( post_itr->title );
         size_t edit_dist = protocol::edit_distance( title, q );
         if( edit_dist <= margin )
         {
            posts.push_back( std::make_pair( title, edit_dist ) );
         }
         ++post_itr;
      }

      std::sort( posts.begin(), posts.end(), [&]( pair< string, size_t > a, pair< string, size_t > b )
      {
         return a.second > b.second;
      });

      for( auto item : posts )
      {
         if( results.posts.size() < query.limit )
         {
            post_itr = post_idx.find( item.first );
            results.posts.push_back( discussion( *post_itr ) );
         }
         else
         {
            break;
         }
      }
   }

   return results;
}


} } // node::app