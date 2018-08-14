add_executable( eznode main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling eznode with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if( STATIC_BUILD )
   target_link_libraries( eznode PRIVATE
      "-static-libstdc++ -static-libgcc"
      eznode_plugins
      eznode_mf_plugins
      eznode_app
      eznode_witness
      eznode_account_history
      eznode_chain
      eznode_protocol
      fc
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
else( STATIC_BUILD )
   target_link_libraries( eznode PRIVATE
      eznode_plugins
      eznode_mf_plugins
      eznode_app
      eznode_witness
      eznode_account_history
      eznode_chain
      eznode_protocol
      fc
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
endif( STATIC_BUILD )

install( TARGETS
   eznode

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
