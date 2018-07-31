#include <ezira/witness/witness_operations.hpp>

#include <ezira/protocol/operation_util_impl.hpp>

namespace ezira { namespace witness {

void enable_content_editing_operation::validate()const
{
   chain::validate_account_name( account );
}

} } // ezira::witness

DEFINE_OPERATION_TYPE( ezira::witness::witness_plugin_operation )
