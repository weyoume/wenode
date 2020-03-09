#pragma once
#include <fc/fixed_string.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace node { namespace chain {

   /**
    * An instance of a specified type of node
    * containing graph node data that is either public or private.
    */
   class graph_node_object : public object< graph_node_object_type, graph_node_object >
   {
      graph_node_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         graph_node_object( Constructor&& c, allocator< Allocator > a ) :
         node_id(a), name(a), details(a), json(a), json_private(a)
         {
            c(*this);
         };

         id_type                            id;           

         account_name_type                  account;                     ///< Name of the account that created the node.

         vector< graph_node_name_type >     node_types;                  ///< Set of Types of node being created, determines the required attributes.

         shared_string                      node_id;                     ///< uuidv4 identifying the node. Unique for each account.

         shared_string                      name;                        ///< Name of the node.

         shared_string                      details;                     ///< Describes the additional details of the node.

         shared_string                      json;                        ///< Public plaintext JSON node attribute information.

         shared_string                      json_private;                ///< Private encrypted ciphertext JSON node attribute information.

         public_key_type                    node_public_key;             ///< Key used for encrypting and decrypting private node JSON data.

         account_name_type                  interface;                   ///< Name of the application that facilitated the creation of the node.

         time_point                         created;                     ///< Time the node was created.

         time_point                         last_updated;                ///< Time that the node was last updated by its creator.

         graph_node_name_type               primary_node_type()const     ///< Primary node type of this node. 
         {
            return node_types[0];
         };
   };


   /**
    * An instance of a specified type of graph edge connecting two nodes
    * containing edge data that is either public or private.
    */
   class graph_edge_object : public object< graph_edge_object_type, graph_edge_object >
   {
      graph_edge_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         graph_edge_object( Constructor&& c, allocator< Allocator > a ) :
         edge_id(a), name(a), details(a), json(a), json_private(a)
         {
            c(*this);
         };

         id_type                            id;           

         account_name_type                  account;                     ///< Name of the account that created the edge.

         vector< graph_edge_name_type >     edge_types;                  ///< Types of the edge being created.

         shared_string                      edge_id;                     ///< uuidv4 identifying the edge.

         graph_node_id_type                 from_node;                   ///< The Base connecting node.

         graph_node_id_type                 to_node;                     ///< The Node being connected to.

         shared_string                      name;                        ///< Name of the edge.

         shared_string                      details;                     ///< Describes the edge.

         shared_string                      json;                        ///< Public plaintext JSON edge attribute information.

         shared_string                      json_private;                ///< Private encrypted ciphertext JSON edge attribute information.

         public_key_type                    edge_public_key;             ///< Key used for encrypting and decrypting private edge JSON data.

         account_name_type                  interface;                   ///< Name of the application that facilitated the creation of the edge.

         time_point                         created;                     ///< Time the edge was created.

         time_point                         last_updated;                ///< Time that the edge was last updated by its creator.

         graph_edge_name_type               primary_edge_type()const     ///< Primary edge type of this edge. 
         {
            return edge_types[0];
         };
   };


   /**
    * A Specification of the properties of a type of node that can be instantiated within the graph.
    */
   class graph_node_property_object : public object< graph_node_property_object_type, graph_node_property_object >
   {
      graph_node_property_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         graph_node_property_object( Constructor&& c, allocator< Allocator > a ) :
         details(a), url(a), json(a), attributes(a)
         {
            c(*this);
         };

         id_type                            id;           

         account_name_type                  account;                     ///< Name of the account that created the node type.

         graph_node_name_type               node_type;                   ///< Name of the type of node being specified.

         connection_tier_type               graph_privacy;               ///< Encryption level of the node attribute data. 

         connection_tier_type               edge_permission;             ///< The Level of connection required to create an edge to or from this node type. 

         shared_string                      details;                     ///< Describes the additional details of the node.

         shared_string                      url;                         ///< Reference URL link for more details.

         shared_string                      json;                        ///< Public plaintext JSON metadata information.

         vector< shared_string >            attributes;                  ///< List of attributes that each node is required to have.

         account_name_type                  interface;                   ///< Name of the application that facilitated the creation of the node type.

         time_point                         created;                     ///< Time the node type was created.

         time_point                         last_updated;                ///< Time that the node type was last updated by its creator.
   };


   /**
    * A Specification of the properties of a type of edge that can be instantiated within the graph.
    */
   class graph_edge_property_object : public object< graph_edge_property_object_type, graph_edge_property_object >
   {
      graph_edge_property_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         graph_edge_property_object( Constructor&& c, allocator< Allocator > a ) :
         details(a), url(a), json(a), attributes(a)
         {
            c(*this);
         };

         id_type                            id;           

         account_name_type                  account;                     ///< Name of the account that created the edge type.

         graph_edge_name_type               edge_type;                   ///< Name of the type of edge being specified.

         connection_tier_type               graph_privacy;               ///< Encryption level of the edge attribute data.

         vector< graph_node_name_type >     from_node_types;             ///< Types of node that the edge can connect from. Empty for all types. 

         vector< graph_node_name_type >     to_node_types;               ///< Types of node that the edge can connect to. Empty for all types.

         shared_string                      details;                     ///< Describes the additional details of the node.

         shared_string                      url;                         ///< Reference URL link for more details.

         shared_string                      json;                        ///< Public plaintext JSON metadata information.

         vector< shared_string >            attributes;                  ///< List of attributes that each edge is required to have.

         account_name_type                  interface;                   ///< Name of the application that facilitated the creation of the edge type.

         time_point                         created;                     ///< Time the edge type was created.

         time_point                         last_updated;                ///< Time that the edge type was last updated by its creator.
   };

   struct by_account_id;
   struct by_node_type;
   struct by_node_id;
   struct by_account;
   struct by_last_updated;

   typedef multi_index_container<
      graph_node_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< graph_node_object, graph_node_id_type, &graph_node_object::id > >,
         ordered_unique< tag< by_account_id >,
            composite_key< graph_node_object,
               member< graph_node_object, account_name_type, &graph_node_object::account >,
               member< graph_node_object, shared_string, &graph_node_object::node_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               strcmp_less 
            >
         >,
         ordered_unique< tag< by_node_type >,
            composite_key< graph_node_object,
               const_mem_fun< graph_node_object, graph_node_name_type, &graph_node_object::primary_node_type >,
               member< graph_node_object, graph_node_id_type, &graph_node_object::id >
            >,
            composite_key_compare< 
               std::less< graph_node_name_type >, 
               std::less< graph_node_id_type > 
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< graph_node_object,
               member< graph_node_object, account_name_type, &graph_node_object::account >,
               member< graph_node_object, graph_node_id_type, &graph_node_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< graph_node_id_type > 
            >
         >,
         ordered_unique< tag< by_node_id >,
            composite_key< graph_node_object,
               member< graph_node_object, shared_string, &graph_node_object::node_id >,
               member< graph_node_object, graph_node_id_type, &graph_node_object::id >
            >,
            composite_key_compare< 
               strcmp_less, 
               std::less< graph_node_id_type > 
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< graph_node_object,
               member< graph_node_object, time_point, &graph_node_object::last_updated >,
               member< graph_node_object, graph_node_id_type, &graph_node_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< graph_node_id_type > 
            >
         >
      >,
      allocator< graph_node_object >
   > graph_node_index;

   struct by_edge_type;
   struct by_edge_id;
   struct by_from_node;
   struct by_to_node;

   typedef multi_index_container<
      graph_edge_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< graph_edge_object, graph_edge_id_type, &graph_edge_object::id > >,
         ordered_unique< tag< by_account_id >,
            composite_key< graph_edge_object,
               member< graph_edge_object, account_name_type, &graph_edge_object::account >,
               member< graph_edge_object, shared_string, &graph_edge_object::edge_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               strcmp_less 
            >
         >,
         ordered_unique< tag< by_edge_type >,
            composite_key< graph_edge_object,
               const_mem_fun< graph_edge_object, graph_edge_name_type, &graph_edge_object::primary_edge_type >,
               member< graph_edge_object, graph_edge_id_type, &graph_edge_object::id >
            >,
            composite_key_compare< 
               std::less< graph_edge_name_type >, 
               std::less< graph_edge_id_type > 
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< graph_edge_object,
               member< graph_edge_object, account_name_type, &graph_edge_object::account >,
               member< graph_edge_object, graph_edge_id_type, &graph_edge_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< graph_edge_id_type > 
            >
         >,
         ordered_unique< tag< by_edge_id >,
            composite_key< graph_edge_object,
               member< graph_edge_object, shared_string, &graph_edge_object::edge_id >,
               member< graph_edge_object, graph_edge_id_type, &graph_edge_object::id >
            >,
            composite_key_compare< 
               strcmp_less, 
               std::less< graph_edge_id_type > 
            >
         >,
         ordered_unique< tag< by_from_node >,
            composite_key< graph_edge_object,
               member< graph_edge_object, graph_node_id_type, &graph_edge_object::from_node >,
               member< graph_edge_object, graph_edge_id_type, &graph_edge_object::id >
            >,
            composite_key_compare< 
               std::less< graph_node_id_type >, 
               std::less< graph_edge_id_type > 
            >
         >,
         ordered_unique< tag< by_to_node >,
            composite_key< graph_edge_object,
               member< graph_edge_object, graph_node_id_type, &graph_edge_object::to_node >,
               member< graph_edge_object, graph_edge_id_type, &graph_edge_object::id >
            >,
            composite_key_compare< 
               std::less< graph_node_id_type >, 
               std::less< graph_edge_id_type > 
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< graph_edge_object,
               member< graph_edge_object, time_point, &graph_edge_object::last_updated >,
               member< graph_edge_object, graph_edge_id_type, &graph_edge_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< graph_edge_id_type > 
            >
         >
      >,
      allocator< graph_edge_object >
   > graph_edge_index;


   typedef multi_index_container<
      graph_node_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< graph_node_property_object, graph_node_property_id_type, &graph_node_property_object::id > >,
         ordered_unique< tag< by_node_type >,
            member< graph_node_property_object, graph_node_name_type, &graph_node_property_object::node_type > >,
         ordered_unique< tag< by_account >,
            composite_key< graph_node_property_object,
               member< graph_node_property_object, account_name_type, &graph_node_property_object::account >,
               member< graph_node_property_object, graph_node_property_id_type, &graph_node_property_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< graph_node_property_id_type > 
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< graph_node_property_object,
               member< graph_node_property_object, time_point, &graph_node_property_object::last_updated >,
               member< graph_node_property_object, graph_node_property_id_type, &graph_node_property_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< graph_node_property_id_type > 
            >
         >
      >,
      allocator< graph_node_property_object >
   > graph_node_property_index;


   typedef multi_index_container<
      graph_edge_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< graph_edge_property_object, graph_edge_property_id_type, &graph_edge_property_object::id > >,
         ordered_unique< tag< by_edge_type >,
            member< graph_edge_property_object, graph_edge_name_type, &graph_edge_property_object::edge_type > >,
         ordered_unique< tag< by_account >,
            composite_key< graph_edge_property_object,
               member< graph_edge_property_object, account_name_type, &graph_edge_property_object::account >,
               member< graph_edge_property_object, graph_edge_property_id_type, &graph_edge_property_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< graph_edge_property_id_type > 
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< graph_edge_property_object,
               member< graph_edge_property_object, time_point, &graph_edge_property_object::last_updated >,
               member< graph_edge_property_object, graph_edge_property_id_type, &graph_edge_property_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< graph_edge_property_id_type > 
            >
         >
      >,
      allocator< graph_edge_property_object >
   > graph_edge_property_index;


} } // node::chain

FC_REFLECT( node::chain::graph_node_object,
         (id)
         (account)
         (node_types)
         (node_id)
         (name)
         (details)
         (json)
         (json_private)
         (node_public_key)
         (interface)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::graph_node_object, node::chain::graph_node_index );

FC_REFLECT( node::chain::graph_edge_object,
         (id)
         (account)
         (edge_types)
         (edge_id)
         (from_node)
         (to_node)
         (name)
         (details)
         (json)
         (json_private)
         (edge_public_key)
         (interface)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::graph_edge_object, node::chain::graph_edge_index );

FC_REFLECT( node::chain::graph_node_property_object,
         (id)
         (account)
         (node_type)
         (graph_privacy)
         (edge_permission)
         (details)
         (url)
         (json)
         (attributes)
         (interface)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::graph_node_property_object, node::chain::graph_node_property_index );

FC_REFLECT( node::chain::graph_edge_property_object,
         (id)
         (account)
         (edge_type)
         (graph_privacy)
         (from_node_types)
         (to_node_types)
         (details)
         (url)
         (json)
         (attributes)
         (interface)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::graph_edge_property_object, node::chain::graph_edge_property_index );