file(GLOB HEADERS "include/eznode/plugins/raw_block/*.hpp")

add_library( eznode_raw_block
             ${HEADERS}
             raw_block_plugin.cpp
             raw_block_api.cpp
           )

target_link_libraries( eznode_raw_block eznode_app eznode_chain eznode_protocol fc )
target_include_directories( eznode_raw_block
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
