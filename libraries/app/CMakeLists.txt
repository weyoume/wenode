file(GLOB HEADERS "include/node/app/*.hpp")

add_library( 
              node_app
              database_api.cpp
              account_api.cpp
              asset_api.cpp
              community_api.cpp
              network_api.cpp
              market_api.cpp
              ad_api.cpp
              product_api.cpp
              graph_api.cpp
              search_api.cpp
              block_api.cpp
              post_api.cpp
              discussion_api.cpp
              api.cpp
              application.cpp
              impacted.cpp
              plugin.cpp
              ${HEADERS}
           )

target_link_libraries( 
                        node_app 
                        node_chain 
                        node_protocol 
                        node_tags 
                        node_producer 
                        node_mf_plugins 
                        fc 
                        graphene_net 
                        graphene_utilities 
                      )


target_include_directories( node_app
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( 
                                application.cpp 
                                api.cpp 
                                database_api.cpp
                                account_api.cpp
                                asset_api.cpp
                                community_api.cpp
                                network_api.cpp
                                market_api.cpp
                                ad_api.cpp
                                product_api.cpp
                                graph_api.cpp
                                search_api.cpp
                                block_api.cpp
                                post_api.cpp
                                discussion_api.cpp

                                PROPERTIES 
                                COMPILE_FLAGS "/bigobj" 
                              )
endif(MSVC)

INSTALL( TARGETS
   node_app

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/node/app" )
