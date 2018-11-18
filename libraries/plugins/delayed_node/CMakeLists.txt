file(GLOB HEADERS "include/node/delayed_node/*.hpp")

add_library( node_delayed_node
             delayed_node_plugin.cpp
           )

target_link_libraries( node_delayed_node node_chain node_protocol node_app )
target_include_directories( node_delayed_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( delayed_node_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   node_delayed_node

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
