file(GLOB HEADERS "include/ezira/plugins/auth_util/*.hpp")

add_library( eznode_auth_util
             ${HEADERS}
             auth_util_plugin.cpp
             auth_util_api.cpp
           )

target_link_libraries( eznode_auth_util eznode_app eznode_chain eznode_protocol fc )
target_include_directories( eznode_auth_util
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
