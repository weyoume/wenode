file(GLOB HEADERS "include/node/account_history/*.hpp")

add_library( node_account_history
             account_history_plugin.cpp
           )

target_link_libraries( node_account_history node_chain node_protocol node_app )
target_include_directories( node_account_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   node_account_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
