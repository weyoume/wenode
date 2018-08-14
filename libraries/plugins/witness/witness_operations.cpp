#include <eznode/witness/witness_operations.hpp>

#include <eznode/protocol/operation_util_impl.hpp>

namespace ezira { namespace witness {

void enable_content_editing_operation::validate()const
{
   chain::validate_account_name( account );
}

} } // eznode::witness

DEFINE_OPERATION_TYPE( eznode::witness::witness_plugin_operation )
