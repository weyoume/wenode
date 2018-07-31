add_executable( js_operation_serializer main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

target_link_libraries( js_operation_serializer
                       PRIVATE ezira_app graphene_net ezira_chain ezira_protocol graphene_utilities ezira_wallet fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )

install( TARGETS
   js_operation_serializer

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
