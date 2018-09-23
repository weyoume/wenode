#include <boost/container/flat_map.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <node/manifest/plugins.hpp>

namespace node { namespace plugin {


std::shared_ptr< node::app::abstract_plugin > create_raw_block_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_auth_util_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_blockchain_statistics_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_witness_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_private_message_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_account_history_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_follow_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_account_statistics_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_debug_node_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_block_info_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_tags_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_delayed_node_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_account_by_key_plugin( node::app::application* app );

std::shared_ptr< node::app::abstract_plugin > create_market_history_plugin( node::app::application* app );


boost::container::flat_map< std::string, std::function< std::shared_ptr< node::app::abstract_plugin >( node::app::application* app ) > > plugin_factories_by_name;

void initialize_plugin_factories()
{
   
   plugin_factories_by_name[ "raw_block" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_raw_block_plugin( app );
   };
   
   plugin_factories_by_name[ "auth_util" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_auth_util_plugin( app );
   };
   
   plugin_factories_by_name[ "blockchain_statistics" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_blockchain_statistics_plugin( app );
   };
   
   plugin_factories_by_name[ "witness" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_witness_plugin( app );
   };
   
   plugin_factories_by_name[ "private_message" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_private_message_plugin( app );
   };
   
   plugin_factories_by_name[ "account_history" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_account_history_plugin( app );
   };
   
   plugin_factories_by_name[ "follow" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_follow_plugin( app );
   };
   
   plugin_factories_by_name[ "account_statistics" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_account_statistics_plugin( app );
   };
   
   plugin_factories_by_name[ "debug_node" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_debug_node_plugin( app );
   };
   
   plugin_factories_by_name[ "block_info" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_block_info_plugin( app );
   };
   
   plugin_factories_by_name[ "tags" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_tags_plugin( app );
   };
   
   plugin_factories_by_name[ "delayed_node" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_delayed_node_plugin( app );
   };
   
   plugin_factories_by_name[ "account_by_key" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_account_by_key_plugin( app );
   };
   
   plugin_factories_by_name[ "market_history" ] = []( node::app::application* app ) -> std::shared_ptr< node::app::abstract_plugin >
   {
      return create_market_history_plugin( app );
   };
   
}

std::shared_ptr< node::app::abstract_plugin > create_plugin( const std::string& name, node::app::application* app )
{
   auto it = plugin_factories_by_name.find( name );
   if( it == plugin_factories_by_name.end() )
      return std::shared_ptr< node::app::abstract_plugin >();
   return it->second( app );
}

std::vector< std::string > get_available_plugins()
{
   std::vector< std::string > result;
   for( const auto& e : plugin_factories_by_name )
      result.push_back( e.first );
   return result;
}

} }