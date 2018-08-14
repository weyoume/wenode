file(GLOB HEADERS "include/eznode/tags/*.hpp")

add_library( eznode_tags
             tags_plugin.cpp )

target_link_libraries( eznode_tags eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_tags
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_tags

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
