file(GLOB HEADERS "include/node/producer/*.hpp")

add_library( node_producer
             producer_plugin.cpp
             producer_evaluators.cpp
             producer_operations.cpp
           )

target_link_libraries( node_producer node_chain node_protocol node_app )
target_include_directories( node_producer
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   node_producer

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
