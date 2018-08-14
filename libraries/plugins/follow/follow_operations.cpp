#include <eznode/follow/follow_operations.hpp>

#include <eznode/protocol/operation_util_impl.hpp>

namespace eznode { namespace follow {

void follow_operation::validate()const
{
   FC_ASSERT( follower != following, "You cannot follow yourself" );
}

void reblog_operation::validate()const
{
   FC_ASSERT( account != author, "You cannot reblog your own content" );
}

} } //eznode::follow

DEFINE_OPERATION_TYPE( eznode::follow::follow_plugin_operation )
