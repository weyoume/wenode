file(GLOB HEADERS "include/ezira/plugins/block_info/*.hpp")

add_library( ezira_block_info
             ${HEADERS}
             block_info_plugin.cpp
             block_info_api.cpp
           )

target_link_libraries( ezira_block_info ezira_app ezira_chain ezira_protocol fc )
target_include_directories( ezira_block_info
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
