file(GLOB HEADERS "include/ezira/account_history/*.hpp")

add_library( ezira_account_history
             account_history_plugin.cpp
           )

target_link_libraries( ezira_account_history ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_account_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_account_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
