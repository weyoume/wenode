file(GLOB HEADERS "include/ezira/delayed_node/*.hpp")

add_library( ezira_delayed_node
             delayed_node_plugin.cpp
           )

target_link_libraries( ezira_delayed_node ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_delayed_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( delayed_node_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   ezira_delayed_node

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
