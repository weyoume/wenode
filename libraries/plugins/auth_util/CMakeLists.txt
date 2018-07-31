file(GLOB HEADERS "include/ezira/plugins/auth_util/*.hpp")

add_library( ezira_auth_util
             ${HEADERS}
             auth_util_plugin.cpp
             auth_util_api.cpp
           )

target_link_libraries( ezira_auth_util ezira_app ezira_chain ezira_protocol fc )
target_include_directories( ezira_auth_util
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
