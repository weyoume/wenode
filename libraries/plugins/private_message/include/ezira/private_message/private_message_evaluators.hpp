#pragma once

#include <ezira/chain/evaluator.hpp>

#include <ezira/private_message/private_message_operations.hpp>
#include <ezira/private_message/private_message_plugin.hpp>

namespace ezira { namespace private_message {

DEFINE_PLUGIN_EVALUATOR( private_message_plugin, ezira::private_message::private_message_plugin_operation, private_message )

} }
