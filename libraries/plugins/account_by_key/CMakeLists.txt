file(GLOB HEADERS "include/ezira/account_by_key/*.hpp")

add_library( ezira_account_by_key
             account_by_key_plugin.cpp
             account_by_key_api.cpp
           )

target_link_libraries( ezira_account_by_key ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_account_by_key
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_account_by_key

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
