file(GLOB HEADERS "include/ezira/plugins/debug_node/*.hpp")

add_library( ezira_debug_node
             ${HEADERS}
             debug_node_plugin.cpp
             debug_node_api.cpp
           )

target_link_libraries( ezira_debug_node ezira_app ezira_chain ezira_protocol fc )
target_include_directories( ezira_debug_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS ezira_debug_node
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
       )
INSTALL( FILES ${HEADERS} DESTINATION "include/ezira/plugins/debug_node" )
