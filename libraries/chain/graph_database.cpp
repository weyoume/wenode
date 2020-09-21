#include <node/protocol/types.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/transaction.hpp>
#include <node/chain/database.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>

#include <node/chain/database_exceptions.hpp>
#include <node/chain/db_with.hpp>
#include <node/chain/evaluator_registry.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/index.hpp>
#include <node/chain/node_evaluator.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/chain/shared_db_merkle.hpp>
#include <node/chain/operation_notification.hpp>
#include <node/chain/producer_schedule.hpp>

#include <node/chain/util/asset.hpp>
#include <node/chain/util/reward.hpp>
#include <node/chain/util/uint256.hpp>
#include <node/chain/util/reward.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/uint128.hpp>

#include <fc/container/deque.hpp>

#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>

namespace node { namespace chain {

const graph_node_object& database::get_graph_node( const account_name_type& account, const shared_string& node_id )const
{ try {
   return get< graph_node_object, by_account_id >( boost::make_tuple( account, node_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(node_id) ) }

const graph_node_object* database::find_graph_node( const account_name_type& account, const shared_string& node_id )const
{
   return find< graph_node_object, by_account_id >( boost::make_tuple( account, node_id ) );
}

const graph_node_object& database::get_graph_node( const account_name_type& account, const string& node_id )const
{ try {
   return get< graph_node_object, by_account_id >( boost::make_tuple( account, node_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(node_id) ) }

const graph_node_object* database::find_graph_node( const account_name_type& account, const string& node_id )const
{
   return find< graph_node_object, by_account_id >( boost::make_tuple( account, node_id ) );
}

const graph_edge_object& database::get_graph_edge( const account_name_type& account, const shared_string& edge_id )const
{ try {
   return get< graph_edge_object, by_account_id >( boost::make_tuple( account, edge_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(edge_id) ) }

const graph_edge_object* database::find_graph_edge( const account_name_type& account, const shared_string& edge_id )const
{
   return find< graph_edge_object, by_account_id >( boost::make_tuple( account, edge_id ) );
}

const graph_edge_object& database::get_graph_edge( const account_name_type& account, const string& edge_id )const
{ try {
   return get< graph_edge_object, by_account_id >( boost::make_tuple( account, edge_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(edge_id) ) }

const graph_edge_object* database::find_graph_edge( const account_name_type& account, const string& edge_id )const
{
   return find< graph_edge_object, by_account_id >( boost::make_tuple( account, edge_id ) );
}

const graph_node_property_object& database::get_graph_node_property( const graph_node_name_type& node_type )const
{ try {
   return get< graph_node_property_object, by_node_type >( node_type );
} FC_CAPTURE_AND_RETHROW( (node_type) ) }

const graph_node_property_object* database::find_graph_node_property( const graph_node_name_type& node_type )const
{
   return find< graph_node_property_object, by_node_type >( node_type );
}

const graph_edge_property_object& database::get_graph_edge_property( const graph_edge_name_type& edge_type )const
{ try {
   return get< graph_edge_property_object, by_edge_type >( edge_type );
} FC_CAPTURE_AND_RETHROW( (edge_type) ) }

const graph_edge_property_object* database::find_graph_edge_property( const graph_edge_name_type& edge_type )const
{
   return find< graph_edge_property_object, by_edge_type >( edge_type );
}


} } // node::chain