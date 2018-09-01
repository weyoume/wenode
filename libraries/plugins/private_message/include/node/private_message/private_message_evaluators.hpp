#pragma once

#include <node/chain/evaluator.hpp>

#include <node/private_message/private_message_operations.hpp>
#include <node/private_message/private_message_plugin.hpp>

namespace node { namespace private_message {

DEFINE_PLUGIN_EVALUATOR( private_message_plugin, node::private_message::private_message_plugin_operation, private_message )

} }
