file(GLOB HEADERS "include/node/private_message/*.hpp")

add_library( node_private_message
             private_message_plugin.cpp
           )

target_link_libraries( node_private_message node_chain node_protocol node_app )
target_include_directories( node_private_message
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   node_private_message

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
