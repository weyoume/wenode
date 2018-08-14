file(GLOB HEADERS "include/eznode/delayed_node/*.hpp")

add_library( eznode_delayed_node
             delayed_node_plugin.cpp
           )

target_link_libraries( eznode_delayed_node eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_delayed_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( delayed_node_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   eznode_delayed_node

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
