
#include <eznode/app/api_context.hpp>
#include <eznode/app/application.hpp>

#include <eznode/plugins/raw_block/raw_block_api.hpp>
#include <eznode/plugins/raw_block/raw_block_plugin.hpp>

namespace ezira { namespace plugin { namespace raw_block {

namespace detail {

class raw_block_api_impl
{
   public:
      raw_block_api_impl( eznode::app::application& _app );

      std::shared_ptr< eznode::plugin::raw_block::raw_block_plugin > get_plugin();

      eznode::app::application& app;
};

raw_block_api_impl::raw_block_api_impl( eznode::app::application& _app ) : app( _app )
{}

std::shared_ptr< eznode::plugin::raw_block::raw_block_plugin > raw_block_api_impl::get_plugin()
{
   return app.get_plugin< raw_block_plugin >( "raw_block" );
}

} // detail

raw_block_api::raw_block_api( const eznode::app::api_context& ctx )
{
   my = std::make_shared< detail::raw_block_api_impl >(ctx.app);
}

get_raw_block_result raw_block_api::get_raw_block( get_raw_block_args args )
{
   get_raw_block_result result;
   std::shared_ptr< eznode::chain::database > db = my->app.chain_database();

   fc::optional<chain::signed_block> block = db->fetch_block_by_number( args.block_num );
   if( !block.valid() )
   {
      return result;
   }
   std::vector<char> serialized_block = fc::raw::pack( *block );
   result.raw_block = fc::base64_encode( std::string(
      &serialized_block[0], &serialized_block[0] + serialized_block.size()) );
   result.block_id = block->id();
   result.previous = block->previous;
   result.timestamp = block->timestamp;
   return result;
}

void raw_block_api::push_raw_block( std::string block_b64 )
{
   std::shared_ptr< eznode::chain::database > db = my->app.chain_database();

   std::string block_bin = fc::base64_decode( block_b64 );
   fc::datastream<const char*> ds( block_bin.c_str(), block_bin.size() );
   chain::signed_block block;
   fc::raw::unpack( ds, block );

   db->push_block( block );

   return;
}

void raw_block_api::on_api_startup() { }

} } } // eznode::plugin::raw_block
