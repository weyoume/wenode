file(GLOB HEADERS "include/ezira/account_statistics/*.hpp")

add_library( ezira_account_statistics
             account_statistics_plugin.cpp
             account_statistics_api.cpp
           )

target_link_libraries( ezira_account_statistics ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_account_statistics
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_account_statistics

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)