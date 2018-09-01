#pragma once

#include <node/protocol/base.hpp>
#include <node/protocol/operation_util.hpp>

#include <node/app/plugin.hpp>

namespace node { namespace witness {

using namespace std;
using node::protocol::base_operation;
using node::chain::database;

class witness_plugin;

struct enable_content_editing_operation : base_operation
{
   protocol::account_name_type   account;
   fc::time_point_sec            relock_time;

   void validate()const;

   void get_required_active_authorities( flat_set< protocol::account_name_type>& a )const { a.insert( account ); }
};

typedef fc::static_variant<
         enable_content_editing_operation
      > witness_plugin_operation;

DEFINE_PLUGIN_EVALUATOR( witness_plugin, witness_plugin_operation, enable_content_editing );

} } // node::witness

FC_REFLECT( node::witness::enable_content_editing_operation, (account)(relock_time) )

FC_REFLECT_TYPENAME( node::witness::witness_plugin_operation )

DECLARE_OPERATION_TYPE( node::witness::witness_plugin_operation )
