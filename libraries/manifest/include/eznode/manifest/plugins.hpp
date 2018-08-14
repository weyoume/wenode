
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ezira { namespace app {

class abstract_plugin;
class application;

} }

namespace ezira { namespace plugin {

void initialize_plugin_factories();
std::shared_ptr< eznode::app::abstract_plugin > create_plugin( const std::string& name, eznode::app::application* app );
std::vector< std::string > get_available_plugins();

} }
