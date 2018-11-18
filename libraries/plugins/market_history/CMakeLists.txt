file(GLOB HEADERS "include/node/market_history/*.hpp")

add_library( node_market_history
             market_history_plugin.cpp
             market_history_api.cpp
           )

target_link_libraries( node_market_history node_chain node_protocol node_app )
target_include_directories( node_market_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   node_market_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)