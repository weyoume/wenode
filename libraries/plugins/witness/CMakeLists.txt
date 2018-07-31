file(GLOB HEADERS "include/ezira/witness/*.hpp")

add_library( ezira_witness
             witness_plugin.cpp
             witness_evaluators.cpp
             witness_operations.cpp
           )

target_link_libraries( ezira_witness ezira_chain ezira_protocol ezira_app )
target_include_directories( ezira_witness
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   ezira_witness

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
