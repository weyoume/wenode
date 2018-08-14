file(GLOB HEADERS "include/eznode/app/*.hpp")

add_library( eznode_app
             database_api.cpp
             api.cpp
             application.cpp
             impacted.cpp
             plugin.cpp
             ${HEADERS}
           )

target_link_libraries( eznode_app eznode_chain eznode_protocol eznode_tags eznode_follow eznode_witness eznode_mf_plugins fc graphene_net graphene_utilities )
target_include_directories( eznode_app
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( application.cpp api.cpp database_api.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   eznode_app

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/eznode/app" )
