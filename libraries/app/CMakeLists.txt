file(GLOB HEADERS "include/ezira/app/*.hpp")

add_library( ezira_app
             database_api.cpp
             api.cpp
             application.cpp
             impacted.cpp
             plugin.cpp
             ${HEADERS}
           )

target_link_libraries( ezira_app ezira_chain ezira_protocol ezira_tags ezira_follow ezira_witness ezira_mf_plugins fc graphene_net graphene_utilities )
target_include_directories( ezira_app
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( application.cpp api.cpp database_api.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   ezira_app

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/ezira/app" )
