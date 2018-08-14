file(GLOB COMMON_SOURCES "common/*.cpp")

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling tests with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

file(GLOB UNIT_TESTS "tests/*.cpp")
add_executable( chain_test ${UNIT_TESTS} ${COMMON_SOURCES} )
target_link_libraries( chain_test chainbase eznode_chain eznode_protocol eznode_app eznode_account_history eznode_market_history eznode_witness eznode_debug_node fc ${PLATFORM_SPECIFIC_LIBS} )

file(GLOB PLUGIN_TESTS "plugin_tests/*.cpp")
add_executable( plugin_test ${PLUGIN_TESTS} ${COMMON_SOURCES} )
target_link_libraries( plugin_test eznode_chain eznode_protocol eznode_app eznode_account_history eznode_market_history eznode_witness eznode_debug_node fc ${PLATFORM_SPECIFIC_LIBS} )

if(MSVC)
  set_source_files_properties( tests/serialization_tests.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

#add_subdirectory( generate_empty_blocks )
