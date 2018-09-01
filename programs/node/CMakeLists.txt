add_executable( node main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling node with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if( STATIC_BUILD )
   target_link_libraries( node PRIVATE
      "-static-libstdc++ -static-libgcc"
      node_plugins
      node_mf_plugins
      node_app
      node_witness
      node_account_history
      node_chain
      node_protocol
      fc
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
else( STATIC_BUILD )
   target_link_libraries( node PRIVATE
      node_plugins
      node_mf_plugins
      node_app
      node_witness
      node_account_history
      node_chain
      node_protocol
      fc
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
endif( STATIC_BUILD )

install( TARGETS
   node

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
