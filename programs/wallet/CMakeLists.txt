add_executable( wallet main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling wallet with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if( STATIC_BUILD )
   # I don't know why node_app is required twice in the following line, I just know the linker breaks if it isn't.
   target_link_libraries( wallet PRIVATE
                          "-static-libstdc++ -static-libgcc -lreadline"
                          node_app graphene_net node_chain node_protocol graphene_utilities node_wallet node_private_message node_app node_follow fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
else( STATIC_BUILD )
   target_link_libraries( wallet PRIVATE
                          node_app graphene_net node_chain node_protocol graphene_utilities node_wallet node_private_message node_app node_follow fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
endif( STATIC_BUILD )

if(MSVC)
  set_source_files_properties( main.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   wallet

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
