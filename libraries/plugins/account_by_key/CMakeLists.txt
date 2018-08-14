file(GLOB HEADERS "include/eznode/account_by_key/*.hpp")

add_library( eznode_account_by_key
             account_by_key_plugin.cpp
             account_by_key_api.cpp
           )

target_link_libraries( eznode_account_by_key eznode_chain eznode_protocol eznode_app )
target_include_directories( eznode_account_by_key
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   eznode_account_by_key

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
