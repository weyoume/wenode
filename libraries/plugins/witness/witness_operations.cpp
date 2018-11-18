#include <node/witness/witness_operations.hpp>

#include <node/protocol/operation_util_impl.hpp>

namespace node { namespace witness {

void enable_content_editing_operation::validate()const
{
   chain::validate_account_name( account );
}

} } // node::witness

DEFINE_OPERATION_TYPE( node::witness::witness_plugin_operation )
