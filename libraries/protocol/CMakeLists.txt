file(GLOB HEADERS "include/ezira/protocol/*.hpp")

## SORT .cpp by most likely to change / break compile
add_library( ezira_protocol

             types.cpp
             authority.cpp
             operations.cpp
             sign_state.cpp
             operation_util_impl.cpp
             steem_operations.cpp
             transaction.cpp
             block.cpp
             asset.cpp
             version.cpp
             get_config.cpp

             ${HEADERS}
           )

target_link_libraries( ezira_protocol fc )
target_include_directories( ezira_protocol
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include" )

INSTALL( TARGETS
   ezira_protocol

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

INSTALL( FILES ${HEADERS} DESTINATION "include/ezira/protocol" )
