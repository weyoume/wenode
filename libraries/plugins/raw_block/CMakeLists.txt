file(GLOB HEADERS "include/ezira/plugins/raw_block/*.hpp")

add_library( ezira_raw_block
             ${HEADERS}
             raw_block_plugin.cpp
             raw_block_api.cpp
           )

target_link_libraries( ezira_raw_block ezira_app ezira_chain ezira_protocol fc )
target_include_directories( ezira_raw_block
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
