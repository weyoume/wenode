add_executable( steemd main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling steemd with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if( EZIRA_STATIC_BUILD )
   target_link_libraries( steemd PRIVATE
      "-static-libstdc++ -static-libgcc"
      ezira_plugins
      ezira_mf_plugins
      ezira_app
      ezira_witness
      ezira_account_history
      ezira_chain
      ezira_protocol
      fc
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
else( EZIRA_STATIC_BUILD )
   target_link_libraries( steemd PRIVATE
      ezira_plugins
      ezira_mf_plugins
      ezira_app
      ezira_witness
      ezira_account_history
      ezira_chain
      ezira_protocol
      fc
      ${CMAKE_DL_LIBS}
      ${PLATFORM_SPECIFIC_LIBS}
   )
endif( EZIRA_STATIC_BUILD )

install( TARGETS
   steemd

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
