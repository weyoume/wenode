
# The node_plugins target links with all plugins
add_library( node_plugins
   plugins.cpp )

target_link_libraries( node_plugins

   node_raw_block

   node_auth_util

   node_blockchain_statistics

   node_witness

   node_private_message

   node_account_history

   node_follow

   node_account_statistics

   node_debug_node

   node_block_info

   node_tags

   node_delayed_node

   node_account_by_key

   node_market_history

)

# The `node_mf_plugins` target has prototypes for querying plugins
# as abstract, but does not itself define the plugins.
#
# The reason for this design is that if we try to use `plugins`
# we will have this circular dependency:
#
# ```
# xxx_plugin ===> node::app ===> plugins ===> xxx_plugin
# ```
#
# To break the dependency, we create `mf_plugins` which only
# has a "weak" reference to the plugin:
#
# ```
# xxx_plugin ===> node::app ===> mf_plugins ---> xxx_plugin
# ```
#
# This weak reference is implemented by a declaring a
# `create_xxx_plugin()` global function which returns a
# `shared_ptr< abstract_plugin >`.  The weak reference isn't
# resolved until link time.  So as long as we link with both
# `node_mf_plugins` and `node_plugins` we are fine.
#

add_library( node_mf_plugins
   mf_plugins.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/include/node/manifest/plugins.hpp
)

target_include_directories( node_mf_plugins
   PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
   )
target_link_libraries( node_mf_plugins fc )

INSTALL( TARGETS
   node_mf_plugins

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)