#pragma once

#include <eznode/chain/evaluator.hpp>

#include <eznode/private_message/private_message_operations.hpp>
#include <eznode/private_message/private_message_plugin.hpp>

namespace eznode { namespace private_message {

DEFINE_PLUGIN_EVALUATOR( private_message_plugin, eznode::private_message::private_message_plugin_operation, private_message )

} }
