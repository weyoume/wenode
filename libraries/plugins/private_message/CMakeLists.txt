file(GLOB HEADERS "include/ezira/private_message/*.hpp")

add_library( ezira_private_message
             private_message_plugin.cpp
           )

target_link_libraries( ezira_private_message ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_private_message
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_private_message

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
