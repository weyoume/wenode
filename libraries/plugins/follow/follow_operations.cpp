#include <ezira/follow/follow_operations.hpp>

#include <ezira/protocol/operation_util_impl.hpp>

namespace ezira { namespace follow {

void follow_operation::validate()const
{
   FC_ASSERT( follower != following, "You cannot follow yourself" );
}

void reblog_operation::validate()const
{
   FC_ASSERT( account != author, "You cannot reblog your own content" );
}

} } //ezira::follow

DEFINE_OPERATION_TYPE( ezira::follow::follow_plugin_operation )
