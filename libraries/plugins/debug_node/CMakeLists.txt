file(GLOB HEADERS "include/eznode/plugins/debug_node/*.hpp")

add_library( eznode_debug_node
             ${HEADERS}
             debug_node_plugin.cpp
             debug_node_api.cpp
           )

target_link_libraries( eznode_debug_node eznode_app eznode_chain eznode_protocol fc )
target_include_directories( eznode_debug_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS eznode_debug_node
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
       )
INSTALL( FILES ${HEADERS} DESTINATION "include/eznode/plugins/debug_node" )
