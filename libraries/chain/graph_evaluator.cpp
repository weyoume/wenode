
#include <node/chain/node_evaluator.hpp>
#include <node/chain/database.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <cmath>

#include <node/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
//#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace node { namespace chain {


//===============================//
// === Graph Data Evaluators === //
//===============================//


void graph_node_evaluator::do_apply( const graph_node_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   flat_set< string > attribute_set;

   connection_tier_type connection_tier = connection_tier_type::PUBLIC;

   // Use the highest level of node encryption required by node properties

   for( graph_node_name_type property : o.node_types )
   {
      const graph_node_property_object& node_property = _db.get_graph_node_property( property );

      for( auto att : node_property.attributes )
      {
         attribute_set.insert( to_string( att ) );
      }

      if( node_property.graph_privacy > connection_tier )
      {
         connection_tier = node_property.graph_privacy;
      }
   }

   for( auto att : attribute_set )
   {
      FC_ASSERT( std::find( o.attributes.begin(), o.attributes.end(), att ) != o.attributes.end(),
         "Nodes must have an attribute value for ${a}.", ("a", att ) );
   }

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   const account_object& account = _db.get_account( o.account );
   public_key_type public_key = public_key_type();

   switch( connection_tier )
   {
      case connection_tier_type::PUBLIC:
      {
         public_key = public_key_type();
      }
      break;
      case connection_tier_type::CONNECTION:
      {
         public_key = account.connection_public_key;
      }
      break;
      case connection_tier_type::FRIEND:
      {
         public_key = account.friend_public_key;
      }
      break;
      case connection_tier_type::COMPANION:
      {
         public_key = account.companion_public_key;
      }
      break;
      case connection_tier_type::SECURE:
      {
         public_key = account.secure_public_key;
      }
      break;
      default:
      {
         FC_ASSERT( false, 
            "Invalid connection type." );
      }
   }

   FC_ASSERT( public_key == public_key_type( o.node_public_key ),
      "Node must use the required public key of the highest graph privacy level of all node types." );
   
   time_point now = _db.head_block_time();

   const auto& node_idx = _db.get_index< graph_node_index >().indices().get< by_account_id >();
   auto node_itr = node_idx.find( boost::make_tuple( o.account, o.node_id ) );

   if( node_itr == node_idx.end() )
   {
      _db.create< graph_node_object >( [&]( graph_node_object& gno )
      {
         gno.account = o.account;
         for( auto nt : o.node_types )
         {
            gno.node_types.push_back( nt );
         }
         from_string( gno.node_id, o.node_id );
         from_string( gno.name, o.name );
         from_string( gno.details, o.details );

         gno.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( gno.attributes[ i ], o.attributes[ i ] );
         }

         gno.attribute_values.reserve( o.attribute_values.size() );
         for( size_t i = 0; i < o.attribute_values.size(); i++ )
         {
            from_string( gno.attribute_values[ i ], o.attribute_values[ i ] );
         }

         from_string( gno.json, o.json );
         from_string( gno.json_private, o.json_private );
         gno.node_public_key = public_key_type( o.node_public_key );

         if( o.interface.size() )
         {
            gno.interface = o.interface;
         }
         
         gno.last_updated = now;
         gno.created = now;
      });
   }
   else
   {
      const graph_node_object& node = *node_itr;

      _db.modify( node, [&]( graph_node_object& gno )
      {
         for( auto nt : o.node_types )
         {
            gno.node_types.push_back( nt );
         }
         from_string( gno.name, o.name );
         from_string( gno.details, o.details );

         gno.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( gno.attributes[ i ], o.attributes[ i ] );
         }

         gno.attribute_values.reserve( o.attribute_values.size() );
         for( size_t i = 0; i < o.attribute_values.size(); i++ )
         {
            from_string( gno.attribute_values[ i ], o.attribute_values[ i ] );
         }

         from_string( gno.json, o.json );
         from_string( gno.json_private, o.json_private );
         gno.node_public_key = public_key_type( o.node_public_key );
         gno.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void graph_edge_evaluator::do_apply( const graph_edge_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const graph_node_object& from_node = _db.get_graph_node( o.from_node_account, o.from_node_id );
   const graph_node_object& to_node = _db.get_graph_node( o.to_node_account, o.to_node_id );
   const account_object& from_account = _db.get_account( o.from_node_account );
   const account_object& to_account = _db.get_account( o.to_node_account );

   flat_set< string > attribute_set;

   connection_tier_type graph_privacy = connection_tier_type::PUBLIC;
   connection_tier_type edge_permission = connection_tier_type::PUBLIC;

   for( graph_edge_name_type property : o.edge_types )
   {
      const graph_edge_property_object& edge_property = _db.get_graph_edge_property( property );
      for( auto att : edge_property.attributes )
      {
         attribute_set.insert( to_string( att ) );
      }

      for( graph_node_name_type node_type : from_node.node_types )
      {
         FC_ASSERT( std::find( edge_property.from_node_types.begin(), edge_property.from_node_types.end(), node_type ) != edge_property.from_node_types.end(),
            "From Node type not found in available node types in edge property ${a}.", ("a", node_type ) );
         const graph_node_property_object& node_property = _db.get_graph_node_property( node_type );
         if( node_property.edge_permission > edge_permission )
         {
            edge_permission = node_property.edge_permission;
         }
      }

      for( graph_node_name_type node_type : to_node.node_types )
      {
         FC_ASSERT( std::find( edge_property.to_node_types.begin(), edge_property.to_node_types.end(), node_type ) != edge_property.to_node_types.end(),
            "To Node type not found in available node types in edge property ${a}.", ("a", node_type ) );
         const graph_node_property_object& node_property = _db.get_graph_node_property( node_type );
         if( node_property.edge_permission > edge_permission )
         {
            edge_permission = node_property.edge_permission;
         }
      }

      if( edge_property.graph_privacy > graph_privacy )
      {
         graph_privacy = edge_property.graph_privacy;
      }
      
      if( edge_property.edge_permission > edge_permission )
      {
         edge_permission = edge_property.edge_permission;
      }
   }
   
   const account_object& account = _db.get_account( o.account );
   public_key_type public_key = public_key_type();

   switch( graph_privacy )
   {
      case connection_tier_type::PUBLIC:
      {
         public_key = public_key_type();
      }
      break;
      case connection_tier_type::CONNECTION:
      {
         public_key = account.connection_public_key;
      }
      break;
      case connection_tier_type::FRIEND:
      {
         public_key = account.friend_public_key;
      }
      break;
      case connection_tier_type::COMPANION:
      {
         public_key = account.companion_public_key;
      }
      break;
      case connection_tier_type::SECURE:
      {
         public_key = account.secure_public_key;
      }
      break;
      default:
      {
         FC_ASSERT( false, 
            "Invalid connection type." );
      }
   }

   account_name_type account_a_name;
   account_name_type account_b_name;

   if( from_account.id < to_account.id )
   {
      account_a_name = from_account.name;
      account_b_name = to_account.name;
   }
   else
   {
      account_b_name = from_account.name;
      account_a_name = to_account.name;
   }

   const auto& con_idx = _db.get_index< connection_index >().indices().get< by_accounts >();
   auto con_itr = con_idx.find( boost::make_tuple( account_a_name, account_b_name, edge_permission ) );

   FC_ASSERT( con_itr != con_idx.end(),
      "Edge creation requires connection between from account and to account." );
   FC_ASSERT( public_key == public_key_type( o.edge_public_key ),
      "Edge must use the required public key of the highest graph privacy level of all edge types." );

   for( auto att : attribute_set )
   {
      FC_ASSERT( std::find( o.attributes.begin(), o.attributes.end(), att ) != o.attributes.end(),
         "Edges must have an attribute value for ${a}.", ("a", att ) );
   }

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }
   
   time_point now = _db.head_block_time();

   const auto& edge_idx = _db.get_index< graph_edge_index >().indices().get< by_account_id >();
   auto edge_itr = edge_idx.find( boost::make_tuple( o.account, o.edge_id ) );

   if( edge_itr == edge_idx.end() )
   {
      _db.create< graph_edge_object >( [&]( graph_edge_object& geo )
      {
         geo.account = o.account;
         for( auto et : o.edge_types )
         {
            geo.edge_types.push_back( et );
         }
         from_string( geo.edge_id, o.edge_id );
         geo.from_node = from_node.id;
         geo.to_node = to_node.id;

         from_string( geo.name, o.name );
         from_string( geo.details, o.details );

         geo.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( geo.attributes[ i ], o.attributes[ i ] );
         }

         geo.attribute_values.reserve( o.attribute_values.size() );
         for( size_t i = 0; i < o.attribute_values.size(); i++ )
         {
            from_string( geo.attribute_values[ i ], o.attribute_values[ i ] );
         }

         from_string( geo.json, o.json );
         from_string( geo.json_private, o.json_private );
         geo.edge_public_key = public_key_type( o.edge_public_key );

         if( o.interface.size() )
         {
            geo.interface = o.interface;
         }
         
         geo.last_updated = now;
         geo.created = now;
      });
   }
   else
   {
      const graph_edge_object& edge = *edge_itr;

      _db.modify( edge, [&]( graph_edge_object& geo )
      {
         for( auto et : o.edge_types )
         {
            geo.edge_types.push_back( et );
         }
         from_string( geo.edge_id, o.edge_id );
         geo.from_node = from_node.id;
         geo.to_node = to_node.id;

         from_string( geo.name, o.name );
         from_string( geo.details, o.details );

         geo.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( geo.attributes[ i ], o.attributes[ i ] );
         }

         geo.attribute_values.reserve( o.attribute_values.size() );
         for( size_t i = 0; i < o.attribute_values.size(); i++ )
         {
            from_string( geo.attribute_values[ i ], o.attribute_values[ i ] );
         }

         from_string( geo.json, o.json );
         from_string( geo.json_private, o.json_private );
         geo.edge_public_key = public_key_type( o.edge_public_key );

         if( o.interface.size() )
         {
            geo.interface = o.interface;
         }
         
         geo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void graph_node_property_evaluator::do_apply( const graph_node_property_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   connection_tier_type graph_privacy = connection_tier_type::PUBLIC;

   for( size_t i = 0; i < connection_tier_values.size(); i++ )
   {
      if( o.graph_privacy == connection_tier_values[ i ] )
      {
         graph_privacy = connection_tier_type( i );
         break;
      }
   }

   connection_tier_type edge_permission = connection_tier_type::PUBLIC;

   for( size_t i = 0; i < connection_tier_values.size(); i++ )
   {
      if( o.edge_permission == connection_tier_values[ i ] )
      {
         edge_permission = connection_tier_type( i );
         break;
      }
   }
   
   time_point now = _db.head_block_time();

   const auto& graph_node_property_idx = _db.get_index< graph_node_property_index >().indices().get< by_node_type >();
   auto graph_node_property_itr = graph_node_property_idx.find( o.node_type );

   if( graph_node_property_itr == graph_node_property_idx.end() )
   {
      _db.create< graph_node_property_object >( [&]( graph_node_property_object& gnpo )
      {
         gnpo.account = o.account;
         gnpo.node_type = o.node_type;
         gnpo.graph_privacy = graph_privacy;
         gnpo.edge_permission = edge_permission;
         
         from_string( gnpo.details, o.details );
         from_string( gnpo.url, o.url );
         from_string( gnpo.json, o.json );

         gnpo.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( gnpo.attributes[ i ], o.attributes[ i ] );
         }
         
         if( o.interface.size() )
         {
            gnpo.interface = o.interface;
         }
         
         gnpo.last_updated = now;
         gnpo.created = now;
      });
   }
   else
   {
      const graph_node_property_object& node = *graph_node_property_itr;

      _db.modify( node, [&]( graph_node_property_object& gnpo )
      {
         gnpo.node_type = o.node_type;
         gnpo.graph_privacy = graph_privacy;
         gnpo.edge_permission = edge_permission;
         
         from_string( gnpo.details, o.details );
         from_string( gnpo.url, o.url );
         from_string( gnpo.json, o.json );

         gnpo.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( gnpo.attributes[ i ], o.attributes[ i ] );
         }
         
         if( o.interface.size() )
         {
            gnpo.interface = o.interface;
         }
         
         gnpo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void graph_edge_property_evaluator::do_apply( const graph_edge_property_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   for( graph_node_name_type property : o.from_node_types )
   {
      _db.get_graph_node_property( property );
   }

   for( graph_node_name_type property : o.to_node_types )
   {
      _db.get_graph_node_property( property );
   }

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   connection_tier_type graph_privacy = connection_tier_type::PUBLIC;

   for( size_t i = 0; i < connection_tier_values.size(); i++ )
   {
      if( o.graph_privacy == connection_tier_values[ i ] )
      {
         graph_privacy = connection_tier_type( i );
         break;
      }
   }
   
   time_point now = _db.head_block_time();

   const auto& graph_edge_property_idx = _db.get_index< graph_edge_property_index >().indices().get< by_edge_type >();
   auto graph_edge_property_itr = graph_edge_property_idx.find( o.edge_type );

   if( graph_edge_property_itr == graph_edge_property_idx.end() )
   {
      _db.create< graph_edge_property_object >( [&]( graph_edge_property_object& gepo )
      {
         gepo.account = o.account;
         gepo.edge_type = o.edge_type;
         gepo.graph_privacy = graph_privacy;
         
         from_string( gepo.details, o.details );
         from_string( gepo.url, o.url );
         from_string( gepo.json, o.json );

         gepo.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( gepo.attributes[ i ], o.attributes[ i ] );
         }
         
         if( o.interface.size() )
         {
            gepo.interface = o.interface;
         }
         
         gepo.last_updated = now;
         gepo.created = now;
      });
   }
   else
   {
      const graph_edge_property_object& edge = *graph_edge_property_itr;

      _db.modify( edge, [&]( graph_edge_property_object& gepo )
      {
         gepo.edge_type = o.edge_type;
         gepo.graph_privacy = graph_privacy;
         
         from_string( gepo.details, o.details );
         from_string( gepo.url, o.url );
         from_string( gepo.json, o.json );

         gepo.attributes.reserve( o.attributes.size() );
         for( size_t i = 0; i < o.attributes.size(); i++ )
         {
            from_string( gepo.attributes[ i ], o.attributes[ i ] );
         }
         
         if( o.interface.size() )
         {
            gepo.interface = o.interface;
         }
         
         gepo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain