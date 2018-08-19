add_executable( ezwallet main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling ezwallet with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if( STATIC_BUILD )
   # I don't know why eznode_app is required twice in the following line, I just know the linker breaks if it isn't.
   target_link_libraries( ezwallet PRIVATE
                          "-static-libstdc++ -static-libgcc -lreadline"
                          eznode_app graphene_net eznode_chain eznode_protocol graphene_utilities eznode_wallet eznode_private_message eznode_app eznode_follow fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
else( STATIC_BUILD )
   target_link_libraries( ezwallet PRIVATE
                          eznode_app graphene_net eznode_chain eznode_protocol graphene_utilities eznode_wallet eznode_private_message eznode_app eznode_follow fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
endif( STATIC_BUILD )

if(MSVC)
  set_source_files_properties( main.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   ezwallet

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
