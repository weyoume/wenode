file(GLOB HEADERS "include/eznode/blockchain_statistics/*.hpp")

add_library( eznode_blockchain_statistics
             blockchain_statistics_plugin.cpp
             blockchain_statistics_api.cpp
           )

target_link_libraries( eznode_blockchain_statistics eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_blockchain_statistics
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_blockchain_statistics

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)