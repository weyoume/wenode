#include <node/app/api_context.hpp>
#include <node/app/application.hpp>
#include <node/app/database_api.hpp>

#include <node/protocol/get_config.hpp>

#include <node/chain/util/reward.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hex.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include <cctype>
#include <cfenv>
#include <iostream>

namespace node { namespace app {

applied_operation::applied_operation() {}

applied_operation::applied_operation( const operation_object& op_obj ): 
   trx_id( op_obj.trx_id ),
   block( op_obj.block ),
   trx_in_block( op_obj.trx_in_block ),
   op_in_trx( op_obj.op_in_trx ),
   virtual_op( op_obj.virtual_op ),
   timestamp( op_obj.timestamp )
{
   //fc::raw::unpack( op_obj.serialized_op, op );     // g++ refuses to compile this as ambiguous
   op = fc::raw::unpack< operation >( op_obj.serialized_op );
}

void find_accounts( set< string >& accounts, const discussion& d ) 
{
   accounts.insert( d.author );
}


   //======================//
   // === Constructors === //
   //======================//


database_api::database_api( const node::app::api_context& ctx )
   : my( new database_api_impl( ctx ) ) {}

database_api::~database_api() {}

database_api_impl::database_api_impl( const node::app::api_context& ctx )
   : _db( *ctx.app.chain_database() )
{
   wlog( "creating database api ${x}", ("x",int64_t(this)) );

   _disable_get_block = ctx.app._disable_get_block;
}

database_api_impl::~database_api_impl()
{
   elog( "freeing database api ${x}", ("x",int64_t(this)) );
}

void database_api::on_api_startup() {}

uint256_t to256( const fc::uint128& t )
{
   uint256_t results( t.high_bits() );
   results <<= 65;
   results += t.low_bits();
   return results;
}



   //=================//
   // === Globals === //
   //=================//
   

fc::variant_object database_api::get_config()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_config();
   });
}

fc::variant_object database_api_impl::get_config()const
{
   return node::protocol::get_config();
}

dynamic_global_property_api_obj database_api::get_dynamic_global_properties()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_dynamic_global_properties();
   });
}

dynamic_global_property_api_obj database_api_impl::get_dynamic_global_properties()const
{
   return dynamic_global_property_api_obj( _db.get( dynamic_global_property_id_type() ), _db );
}

median_chain_property_api_obj database_api::get_median_chain_properties()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_median_chain_properties();
   });
}

median_chain_property_api_obj database_api_impl::get_median_chain_properties()const
{
   return median_chain_property_api_obj( _db.get_median_chain_properties() );
}

producer_schedule_api_obj database_api::get_producer_schedule()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_producer_schedule();
   });
}

producer_schedule_api_obj database_api_impl::get_producer_schedule()const
{
   return producer_schedule_api_obj( _db.get_producer_schedule() );
}

hardfork_version database_api::get_hardfork_version()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_hardfork_version();
   });
}

hardfork_version database_api_impl::get_hardfork_version()const
{
   const hardfork_property_object& hpo = _db.get( hardfork_property_id_type() );
   return hpo.current_hardfork_version;
}

scheduled_hardfork database_api::get_next_scheduled_hardfork() const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_next_scheduled_hardfork();
   });
}

scheduled_hardfork database_api_impl::get_next_scheduled_hardfork() const
{
   scheduled_hardfork shf;
   const hardfork_property_object& hpo = _db.get( hardfork_property_id_type() );
   shf.hf_version = hpo.next_hardfork;
   shf.live_time = hpo.next_hardfork_time;
   return shf;
}


   //=======================//
   // === Subscriptions === //
   //=======================//


void database_api::set_block_applied_callback( std::function<void( const variant& block_id)> cb )
{
   my->_db.with_read_lock( [&]()
   {
      my->set_block_applied_callback( cb );
   });
}

void database_api_impl::on_applied_block( const chain::signed_block& b )
{
   try
   {
      _block_applied_callback( fc::variant(signed_block_header(b) ) );
   }
   catch( ... )
   {
      _block_applied_connection.release();
   }
}

void database_api_impl::set_block_applied_callback( std::function<void( const variant& block_header)> cb )
{
   _block_applied_callback = cb;
   _block_applied_connection = connect_signal( _db.applied_block, *this, &database_api_impl::on_applied_block );
}

} } // node::app