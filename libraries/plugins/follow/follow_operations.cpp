#include <node/follow/follow_operations.hpp>

#include <node/protocol/operation_util_impl.hpp>

namespace node { namespace follow {

void follow_operation::validate()const
{
   FC_ASSERT( follower != following, "You cannot follow yourself" );
}

void reblog_operation::validate()const
{
   FC_ASSERT( account != author, "You cannot reblog your own content" );
}

} } //node::follow

DEFINE_OPERATION_TYPE( node::follow::follow_plugin_operation )
