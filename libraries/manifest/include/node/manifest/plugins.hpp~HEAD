
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace node { namespace app {

class abstract_plugin;
class application;

} }

namespace node { namespace plugin {

void initialize_plugin_factories();
std::shared_ptr< node::app::abstract_plugin > create_plugin( const std::string& name, node::app::application* app );
std::vector< std::string > get_available_plugins();

} }
