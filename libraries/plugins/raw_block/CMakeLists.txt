file(GLOB HEADERS "include/node/plugins/raw_block/*.hpp")

add_library( node_raw_block
             ${HEADERS}
             raw_block_plugin.cpp
             raw_block_api.cpp
           )

target_link_libraries( node_raw_block node_app node_chain node_protocol fc )
target_include_directories( node_raw_block
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
