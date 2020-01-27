#include <node/producer/producer_operations.hpp>

#include <node/protocol/operation_util_impl.hpp>

namespace node { namespace producer {

void enable_content_editing_operation::validate()const
{
   chain::validate_account_name( account );
}

} } // node::producer

DEFINE_OPERATION_TYPE( node::producer::producer_plugin_operation )
