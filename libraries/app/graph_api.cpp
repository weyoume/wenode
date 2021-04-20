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


   //===================//
   // === Graph API === //
   //===================//


graph_data_state database_api::get_graph_query( const graph_query& query )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_graph_query( query );
   });
}


/**
 * Retreives a series of graph nodes and edges based on the query.
 */
graph_data_state database_api_impl::get_graph_query( const graph_query& query )const
{
   graph_data_state results;

   const auto& node_idx = _db.get_index< graph_node_index >().indices().get< by_account_id >();
   const auto& edge_idx = _db.get_index< graph_edge_index >().indices().get< by_account_id >();

   auto node_itr = node_idx.begin();
   auto edge_itr = edge_idx.begin();
   
   vector< graph_node_api_obj > nodes;
   vector< graph_edge_api_obj > edges;
   
   nodes.reserve( query.limit );
   edges.reserve( query.limit );

   bool found = false;
   bool not_found = false;

   while( node_itr != node_idx.end() )
   {
      if( !query.include_private && node_itr->is_encrypted() )
      {
         ++node_itr;
         continue;
      }

      if( query.select_accounts.find( node_itr->account ) == query.select_accounts.end() )
      {
         ++node_itr;
         continue;
      }

      if( query.filter_accounts.find( node_itr->account ) != query.filter_accounts.end() )
      {
         ++node_itr;
         continue;
      }

      // Must contain all intersect select node types
      for( auto node_type : node_itr->node_types )
      {
         not_found = false;

         if( query.intersect_select_node_types.find( node_type ) == query.intersect_select_node_types.end() )
         {
            not_found = true;
            break;
         }
      }

      if( not_found )
      {
         ++node_itr;
         continue;
      }

      // Must not contain all intersect filter node types
      for( auto node_type : node_itr->node_types )
      {
         not_found = false;

         if( query.intersect_filter_node_types.find( node_type ) == query.intersect_filter_node_types.end() )
         {
            not_found = true;
            break;
         }
      }

      if( !not_found )
      {
         ++node_itr;
         continue;
      }

      // Must contain any union node types
      for( auto node_type : node_itr->node_types )
      {
         found = false;

         if( query.union_select_node_types.find( node_type ) != query.union_select_node_types.end() )
         {
            found = true;
            break;
         }
      }

      if( !found )
      {
         ++node_itr;
         continue;
      }

      // Must not contain any union node types
      for( auto node_type : node_itr->node_types )
      {
         found = false;

         if( query.union_filter_node_types.find( node_type ) != query.union_filter_node_types.end() )
         {
            found = true;
            break;
         }
      }

      if( found )
      {
         ++node_itr;
         continue;
      }

      flat_map< fixed_string_32, fixed_string_32 > attribute_map = node_itr->attribute_map();

      // Must match all intersect select attribute values
      for( size_t i = 0; i < query.node_intersect_select_attributes.size(); i++ )
      {
         not_found = false;

         if( attribute_map[ query.node_intersect_select_attributes[ i ] ] != query.node_intersect_select_values[ i ] )
         {
            not_found = true;
            break;
         }
      }

      if( not_found )
      {
         ++node_itr;
         continue;
      }

      // Must not match all intersect select attribute values
      for( size_t i = 0; i < query.node_intersect_filter_attributes.size(); i++ )
      {
         not_found = false;

         if( attribute_map[ query.node_intersect_filter_attributes[ i ] ] != query.node_intersect_filter_values[ i ] )
         {
            not_found = true;
            break;
         }
      }

      if( !not_found )
      {
         ++node_itr;
         continue;
      }

      // Must contain any union select attribute values
      for( size_t i = 0; i < query.node_union_select_attributes.size(); i++ )
      {
         found = false;

         if( attribute_map[ query.node_union_select_attributes[ i ] ] == query.node_union_select_values[ i ] )
         {
            found = true;
            break;
         }
      }

      if( !found )
      {
         ++node_itr;
         continue;
      }

      // Must not contain any union filter attribute values
      for( size_t i = 0; i < query.node_union_filter_attributes.size(); i++ )
      {
         found = false;

         if( attribute_map[ query.node_union_filter_attributes[ i ] ] == query.node_union_filter_values[ i ] )
         {
            found = true;
            break;
         }
      }

      if( found )
      {
         ++node_itr;
         continue;
      }

      nodes.push_back( *node_itr );
      ++node_itr;
   }

   results.nodes = nodes;

   while( edge_itr != edge_idx.end() )
   {
      if( !query.include_private && edge_itr->is_encrypted() )
      {
         ++edge_itr;
         continue;
      }

      if( query.select_accounts.find( edge_itr->account ) == query.select_accounts.end() )
      {
         ++edge_itr;
         continue;
      }

      if( query.filter_accounts.find( edge_itr->account ) != query.filter_accounts.end() )
      {
         ++edge_itr;
         continue;
      }

      // Must contain all intersect select edge types
      for( auto edge_type : edge_itr->edge_types )
      {
         not_found = false;

         if( query.intersect_select_edge_types.find( edge_type ) == query.intersect_select_edge_types.end() )
         {
            not_found = true;
            break;
         }
      }

      if( not_found )
      {
         ++edge_itr;
         continue;
      }

      // Must not contain all intersect filter edge types
      for( auto edge_type : edge_itr->edge_types )
      {
         not_found = false;

         if( query.intersect_filter_edge_types.find( edge_type ) == query.intersect_filter_edge_types.end() )
         {
            not_found = true;
            break;
         }
      }

      if( !not_found )
      {
         ++edge_itr;
         continue;
      }

      // Must contain any union edge types
      for( auto edge_type : edge_itr->edge_types )
      {
         found = false;

         if( query.union_select_edge_types.find( edge_type ) != query.union_select_edge_types.end() )
         {
            found = true;
            break;
         }
      }

      if( !found )
      {
         ++edge_itr;
         continue;
      }

      // Must not contain any union edge types
      for( auto edge_type : edge_itr->edge_types )
      {
         found = false;

         if( query.union_filter_edge_types.find( edge_type ) != query.union_filter_edge_types.end() )
         {
            found = true;
            break;
         }
      }

      if( found )
      {
         ++edge_itr;
         continue;
      }

      flat_map< fixed_string_32, fixed_string_32 > attribute_map = edge_itr->attribute_map();

      // Must match all intersect select attribute values
      for( size_t i = 0; i < query.edge_intersect_select_attributes.size(); i++ )
      {
         not_found = false;

         if( attribute_map[ query.edge_intersect_select_attributes[ i ] ] != query.edge_intersect_select_values[ i ] )
         {
            not_found = true;
            break;
         }
      }

      if( not_found )
      {
         ++edge_itr;
         continue;
      }

      // Must not match all intersect select attribute values
      for( size_t i = 0; i < query.edge_intersect_filter_attributes.size(); i++ )
      {
         not_found = false;

         if( attribute_map[ query.edge_intersect_filter_attributes[ i ] ] != query.edge_intersect_filter_values[ i ] )
         {
            not_found = true;
            break;
         }
      }

      if( !not_found )
      {
         ++edge_itr;
         continue;
      }

      // Must contain any union select attribute values
      for( size_t i = 0; i < query.edge_union_select_attributes.size(); i++ )
      {
         found = false;

         if( attribute_map[ query.edge_union_select_attributes[ i ] ] == query.edge_union_select_values[ i ] )
         {
            found = true;
            break;
         }
      }

      if( !found )
      {
         ++edge_itr;
         continue;
      }

      // Must not contain any union filter attribute values
      for( size_t i = 0; i < query.edge_union_filter_attributes.size(); i++ )
      {
         found = false;

         if( attribute_map[ query.edge_union_filter_attributes[ i ] ] == query.edge_union_filter_values[ i ] )
         {
            found = true;
            break;
         }
      }

      if( found )
      {
         ++edge_itr;
         continue;
      }

      edges.push_back( *edge_itr );
      ++edge_itr;
   }

   results.edges = edges;
   
   return results;
}


vector< graph_node_property_api_obj > database_api::get_graph_node_properties( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_graph_node_properties( names );
   });
}


vector< graph_node_property_api_obj > database_api_impl::get_graph_node_properties( vector< string > names )const
{
   vector< graph_node_property_api_obj > results;

   const auto& node_idx = _db.get_index< graph_node_property_index >().indices().get< by_node_type >();

   for( auto node_type : names )
   {
      auto node_itr = node_idx.find( node_type );
      if( node_itr != node_idx.end() )
      {
         results.push_back( *node_itr );
      }
   }

   return results;
}


vector< graph_edge_property_api_obj > database_api::get_graph_edge_properties( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_graph_edge_properties( names );
   });
}


vector< graph_edge_property_api_obj > database_api_impl::get_graph_edge_properties( vector< string > names )const
{
   vector< graph_edge_property_api_obj > results;

   const auto& edge_idx = _db.get_index< graph_edge_property_index >().indices().get< by_edge_type >();

   for( auto edge_type : names )
   {
      auto edge_itr = edge_idx.find( edge_type );
      if( edge_itr != edge_idx.end() )
      {
         results.push_back( *edge_itr );
      }
   }

   return results;
}


} } // node::app