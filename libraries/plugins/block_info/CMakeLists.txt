file(GLOB HEADERS "include/eznode/plugins/block_info/*.hpp")

add_library( eznode_block_info
             ${HEADERS}
             block_info_plugin.cpp
             block_info_api.cpp
           )

target_link_libraries( eznode_block_info eznode_app eznode_chain eznode_protocol fc )
target_include_directories( eznode_block_info
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
