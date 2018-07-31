file(GLOB HEADERS "include/ezira/blockchain_statistics/*.hpp")

add_library( ezira_blockchain_statistics
             blockchain_statistics_plugin.cpp
             blockchain_statistics_api.cpp
           )

target_link_libraries( ezira_blockchain_statistics ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_blockchain_statistics
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_blockchain_statistics

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)