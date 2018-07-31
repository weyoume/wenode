file(GLOB HEADERS "include/ezira/market_history/*.hpp")

add_library( ezira_market_history
             market_history_plugin.cpp
             market_history_api.cpp
           )

target_link_libraries( ezira_market_history ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_market_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_market_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)