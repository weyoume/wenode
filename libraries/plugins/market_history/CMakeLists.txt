file(GLOB HEADERS "include/ezira/market_history/*.hpp")

add_library( eznode_market_history
             market_history_plugin.cpp
             market_history_api.cpp
           )

target_link_libraries( eznode_market_history eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_market_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_market_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)