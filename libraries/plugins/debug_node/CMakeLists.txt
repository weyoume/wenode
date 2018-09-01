file(GLOB HEADERS "include/node/plugins/debug_node/*.hpp")

add_library( node_debug_node
             ${HEADERS}
             debug_node_plugin.cpp
             debug_node_api.cpp
           )

target_link_libraries( node_debug_node node_app node_chain node_protocol fc )
target_include_directories( node_debug_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS node_debug_node
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
       )
INSTALL( FILES ${HEADERS} DESTINATION "include/node/plugins/debug_node" )
