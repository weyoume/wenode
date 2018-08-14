file(GLOB HEADERS "include/ezira/account_statistics/*.hpp")

add_library( eznode_account_statistics
             account_statistics_plugin.cpp
             account_statistics_api.cpp
           )

target_link_libraries( eznode_account_statistics eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_account_statistics
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_account_statistics

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)