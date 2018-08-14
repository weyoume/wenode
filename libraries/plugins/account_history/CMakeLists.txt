file(GLOB HEADERS "include/ezira/account_history/*.hpp")

add_library( eznode_account_history
             account_history_plugin.cpp
           )

target_link_libraries( eznode_account_history eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_account_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_account_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
