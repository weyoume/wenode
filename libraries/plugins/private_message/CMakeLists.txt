file(GLOB HEADERS "include/eznode/private_message/*.hpp")

add_library( eznode_private_message
             private_message_plugin.cpp
           )

target_link_libraries( eznode_private_message eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_private_message
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_private_message

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
