#pragma once

#include <node/protocol/base.hpp>
#include <node/protocol/operation_util.hpp>

#include <node/app/plugin.hpp>

namespace node { namespace producer {

using namespace std;
using node::protocol::base_operation;
using node::chain::database;

class producer_plugin;

struct enable_content_editing_operation : base_operation
{
   protocol::account_name_type   account;
   fc::time_point                relock_time;

   void validate()const;
   void get_creator_name( flat_set< protocol::account_name_type>& a )const { a.insert( account ); }
   void get_required_active_authorities( flat_set< protocol::account_name_type>& a )const { a.insert( account ); }
};

typedef fc::static_variant<
         enable_content_editing_operation
      > producer_plugin_operation;

DEFINE_PLUGIN_EVALUATOR( producer_plugin, producer_plugin_operation, enable_content_editing );

} } // node::producer

FC_REFLECT( node::producer::enable_content_editing_operation, (account)(relock_time) )

FC_REFLECT_TYPENAME( node::producer::producer_plugin_operation )

DECLARE_OPERATION_TYPE( node::producer::producer_plugin_operation )
