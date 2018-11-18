file(GLOB HEADERS "include/node/plugins/block_info/*.hpp")

add_library( node_block_info
             ${HEADERS}
             block_info_plugin.cpp
             block_info_api.cpp
           )

target_link_libraries( node_block_info node_app node_chain node_protocol fc )
target_include_directories( node_block_info
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
