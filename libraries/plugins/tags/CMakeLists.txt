file(GLOB HEADERS "include/node/tags/*.hpp")

add_library( node_tags
             tags_plugin.cpp )

target_link_libraries( node_tags node_chain node_protocol node_app )
target_include_directories( node_tags
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   node_tags

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
