file(GLOB HEADERS "include/ezira/tags/*.hpp")

add_library( ezira_tags
             tags_plugin.cpp )

target_link_libraries( ezira_tags ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_tags
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_tags

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
