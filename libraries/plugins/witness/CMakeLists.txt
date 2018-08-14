file(GLOB HEADERS "include/eznode/witness/*.hpp")

add_library( eznode_witness
             witness_plugin.cpp
             witness_evaluators.cpp
             witness_operations.cpp
           )

target_link_libraries( eznode_witness eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_witness
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_witness

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
