
#include <node/app/application.hpp>
#include <node/app/plugin.hpp>
#include <node/plugins/debug_node/debug_node_api.hpp>
#include <node/plugins/debug_node/debug_node_plugin.hpp>

#include <fc/io/buffered_iostream.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>

#include <fc/thread/future.hpp>
#include <fc/thread/mutex.hpp>
#include <fc/thread/scoped_lock.hpp>

#include <graphene/utilities/key_conversion.hpp>

#include <sstream>
#include <string>

namespace node { namespace plugin { namespace debug_node {

namespace detail {
class debug_node_plugin_impl
{
   public:
      debug_node_plugin_impl( debug_node_plugin* self );
      virtual ~debug_node_plugin_impl();

      debug_node_plugin* _self;

      uint32_t                                  _mining_threads = 1;
      
      std::vector<std::shared_ptr<fc::thread> > _thread_pool;
};

debug_node_plugin_impl::debug_node_plugin_impl( debug_node_plugin* self )
   : _self(self)
{ }
debug_node_plugin_impl::~debug_node_plugin_impl() {}
}

private_key_storage::private_key_storage() {}
private_key_storage::~private_key_storage() {}

debug_node_plugin::debug_node_plugin( application* app ) : plugin( app )
{
   _my = std::make_shared< detail::debug_node_plugin_impl >( this );
}
debug_node_plugin::~debug_node_plugin() {}

struct debug_mine_state
{
   debug_mine_state();
   virtual ~debug_mine_state();

   std::string                                    miner_account;

   chain::block_id_type                           prev_block;

   chain::x11                                     pow_target;

   fc::promise< chain::x11_proof_of_work >::ptr   work;
   
   fc::mutex                                      set_work_mutex;
};

debug_mine_state::debug_mine_state() {}
debug_mine_state::~debug_mine_state() {}

void debug_node_plugin::debug_mine_work( chain::x11_proof_of_work& work, chain::x11 pow_target )
{
   std::shared_ptr< debug_mine_state > mine_state = std::make_shared< debug_mine_state >();
   mine_state->miner_account = work.input.miner_account;
   mine_state->prev_block = work.input.prev_block;
   mine_state->pow_target = pow_target;
   mine_state->work = fc::promise< chain::x11_proof_of_work >::ptr( new fc::promise< chain::x11_proof_of_work >() );

   uint32_t thread_num = 0;
   uint32_t num_threads = _my->_mining_threads;

   wlog( "Mining for worker account ${a} on block ${b} with target ${t} using ${n} threads",
      ("a",work.input.miner_account)("b",work.input.prev_block)("t",pow_target)("n",num_threads));

   uint32_t nonce_start = 0;

   for( auto& t : _my->_thread_pool )
   {
      uint32_t nonce_offset = nonce_start + thread_num;
      uint32_t nonce_stride = num_threads;
      wlog( "Launching thread ${i}", ("i", thread_num) );
      t->async( [mine_state,nonce_offset,nonce_stride]()
      {
         chain::x11_proof_of_work work;
         std::string miner_account = mine_state->miner_account;
         chain::block_id_type prev_block = mine_state->prev_block;
         chain::x11 pow_target = mine_state->pow_target;
         wlog( "Starting thread mining at offset ${o}", ("o", nonce_offset) );
         work.input.prev_block = prev_block;
         work.input.miner_account = miner_account;
         work.input.nonce = nonce_offset;

         while( !(mine_state->work->ready()) )
         {
            work.create( prev_block, miner_account, work.input.nonce );
            if( work.work < pow_target )
            {
               wlog( "Found work with nonce ${n}",
                  ("n", work.input.nonce));
               fc::scoped_lock< fc::mutex > lock(mine_state->set_work_mutex);
               if( !mine_state->work->ready() )
               {
                  mine_state->work->set_value( work );
                  wlog( "Quitting successfully (start nonce was ${n})",
                     ("n", nonce_offset) );
               }
               else
               {
                  wlog( "Quitting, but other thread found nonce first (start nonce was ${n})",
                     ("n", nonce_offset) );
               }
               break;
            }
            work.input.nonce += nonce_stride;
         }
         wlog( "Quitting (start nonce was ${n})", ("n", nonce_offset) );
         return;
      });
      ++thread_num;
   }

   work = mine_state->work->wait();

   wlog( "Finished, work=${w}", ("w", work) );
   return;
}

std::string debug_node_plugin::plugin_name()const
{
   return "debug_node";
}

void debug_node_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg )
{
   cli.add_options()
      ("edit-script,e", boost::program_options::value< std::vector< std::string > >()->composing(), "Database edits to apply on startup (may specify multiple times)");
   cfg.add(cli);
}

void debug_node_plugin::plugin_initialize( const boost::program_options::variables_map& options )
{
   if( options.count("edit-script") > 0 )
   {
      _edit_scripts = options.at("edit-script").as< std::vector< std::string > >();
   }

   if( options.count("mining-threads") > 0 )
   {
      _my->_mining_threads = options.at("mining-threads").as< uint32_t >();
   }

   if( logging ) wlog( "Initializing ${n} mining threads", ("n", _my->_mining_threads) );
   _my->_thread_pool.resize( _my->_mining_threads );
   for( uint32_t i = 0; i < _my->_mining_threads; ++i )
      _my->_thread_pool[i] = std::make_shared<fc::thread>();
}

void debug_node_plugin::plugin_startup()
{
   if( logging ) ilog("debug_node_plugin::plugin_startup() begin");
   chain::database& db = database();

   // connect needed signals

   _applied_block_conn  = db.applied_block.connect([this](const chain::signed_block& b){ on_applied_block(b); });

   app().register_api_factory< debug_node_api >( "debug_node_api" );

   /*for( const std::string& fn : _edit_scripts )
   {
      std::shared_ptr< fc::ifstream > stream = std::make_shared< fc::ifstream >( fc::path(fn) );
      fc::buffered_istream bstream(stream);
      fc::variant v = fc::json::from_stream( bstream, fc::json::strict_parser );
      load_debug_updates( v.get_object() );
   }*/
}

/*
void debug_apply_update( chain::database& db, const fc::variant_object& vo, bool logging )
{
   static const uint8_t
      db_action_nil = 0,
      db_action_create = 1,
      db_action_write = 2,
      db_action_update = 3,
      db_action_delete = 4,
      db_action_set_hardfork = 5;
   if( logging ) wlog( "debug_apply_update:  ${o}", ("o", vo) );

   // "_action" : "create"   object must not exist, unspecified fields take defaults
   // "_action" : "write"    object may exist, is replaced entirely, unspecified fields take defaults
   // "_action" : "update"   object must exist, unspecified fields don't change
   // "_action" : "delete"   object must exist, will be deleted

   // if _action is unspecified:
   // - delete if object contains only ID field
   // - otherwise, write

   graphene::db2::generic_id oid;
   uint8_t action = db_action_nil;
   auto it_id = vo.find("id");
   FC_ASSERT( it_id != vo.end() );

   from_variant( it_id->value(), oid );
   action = ( vo.size() == 1 ) ? db_action_delete : db_action_write;

   from_variant( vo["id"], oid );
   if( vo.size() == 1 )
      action = db_action_delete;

   fc::mutable_variant_object mvo( vo );
   mvo( "id", oid._id );
   auto it_action = vo.find("_action" );
   if( it_action != vo.end() )
   {
      const std::string& str_action = it_action->value().get_string();
      if( str_action == "create" )
         action = db_action_create;
      else if( str_action == "write" )
         action = db_action_write;
      else if( str_action == "update" )
         action = db_action_update;
      else if( str_action == "delete" )
         action = db_action_delete;
      else if( str_action == "set_hardfork" )
         action = db_action_set_hardfork;
   }

   switch( action )
   {
      case db_action_create:

         idx.create( [&]( object& obj )
         {
            idx.object_from_variant( vo, obj );
         } );

         FC_ASSERT( false );
         break;
      case db_action_write:
         db.modify( db.get_object( oid ), [&]( graphene::db::object& obj )
         {
            idx.object_default( obj );
            idx.object_from_variant( vo, obj );
         } );
         FC_ASSERT( false );
         break;
      case db_action_update:
         db.modify_variant( oid, mvo );
         break;
      case db_action_delete:
         db.remove_object( oid );
         break;
      case db_action_set_hardfork:
         {
            uint32_t hardfork_id;
            from_variant( vo[ "hardfork_id" ], hardfork_id );
            db.set_hardfork( hardfork_id, false );
         }
         break;
      default:
         FC_ASSERT( false );
   }
}
*/

uint32_t debug_node_plugin::debug_generate_blocks(
   uint32_t count,
   uint32_t skip,
   uint32_t miss_blocks,
   private_key_storage* key_storage )
{
   if( count == 0 )
   {
      return 0;
   }

   fc::optional< fc::ecc::private_key > producer_private_key;
   node::chain::public_key_type producer_public_key;
   node::chain::database& db = database();
   uint64_t slot = miss_blocks+1;
   uint64_t produced = 0;

   while( produced < count )
   {
      std::string scheduled_producer_name = db.get_scheduled_producer( slot );
      fc::time_point scheduled_time = db.get_slot_time( slot );
      const chain::producer_object& scheduled_producer = db.get_producer( scheduled_producer_name );
      node::chain::public_key_type scheduled_key = scheduled_producer.signing_key;

      producer_private_key = node::protocol::get_private_key( scheduled_producer_name, "producer", INIT_PASSWORD );
   
      FC_ASSERT( producer_private_key.valid(), 
         "Producer private key invalid." );
      producer_public_key = producer_private_key->get_public_key();

      // ilog( "Scheduled Key is: ${sk} Producer Key is: ${pk}", ("sk", scheduled_key)("pk", producer_public_key) );
      
      if( scheduled_key != producer_public_key )
      {
         producer_private_key.reset();

         if( key_storage != nullptr )
         {
            key_storage->maybe_get_private_key( producer_private_key, scheduled_key, scheduled_producer_name );
         }

         if( !producer_private_key.valid() )
         {
            elog( "Skipping ${wit} because I don't know the private key",
               ("wit", scheduled_producer_name) );
            
            continue;
         }
      }

      db.generate_block( scheduled_time, scheduled_producer_name, *producer_private_key, skip );
      ++produced;
   }

   return count;
}

uint32_t debug_node_plugin::debug_generate_blocks_until(
   const fc::time_point& head_block_time,
   bool generate_sparsely,
   uint32_t skip,
   private_key_storage* key_storage )
{
   node::chain::database& db = database();

   if( db.head_block_time() >= head_block_time )
      return 0;

   uint32_t new_blocks = 0;

   if( generate_sparsely )
   {
      new_blocks += debug_generate_blocks( 1, skip );
      auto slots_to_miss = db.get_slot_at_time( head_block_time );
      if( slots_to_miss > 1 )
      {
         slots_to_miss--;
         new_blocks += debug_generate_blocks( 1, skip, slots_to_miss, key_storage );
      }
   }
   else
   {
      while( db.head_block_time() < head_block_time )
      {
         new_blocks += debug_generate_blocks( 1 );
      }
   }

   return new_blocks;
}

uint32_t debug_node_plugin::debug_generate_until_block(
   uint64_t head_block_num,
   bool generate_sparsely,
   uint32_t skip,
   private_key_storage* key_storage )
{
   node::chain::database& db = database();

   if( db.head_block_num() >= head_block_num )
      return 0;

   uint32_t new_blocks = 0;

   if( generate_sparsely )
   {
      new_blocks += debug_generate_blocks( 1, skip );
      auto slots_to_miss = head_block_num - db.head_block_num();
      if( slots_to_miss > 1 )
      {
         slots_to_miss--;
         new_blocks += debug_generate_blocks( 1, skip, slots_to_miss, key_storage );
      }
   }
   else
   {
      while( db.head_block_num() < head_block_num )
      {
         new_blocks += debug_generate_blocks( 1 );
      } 
   }

   return new_blocks;
}

void debug_node_plugin::apply_debug_updates()
{
   // this was a method on database in Graphene
   chain::database& db = database();
   chain::block_id_type head_id = db.head_block_id();
   auto it = _debug_updates.find( head_id );
   if( it == _debug_updates.end() )
      return;
   //for( const fc::variant_object& update : it->second )
   //   debug_apply_update( db, update, logging );
   for( auto& update : it->second )
      update( db );
}

void debug_node_plugin::on_applied_block( const chain::signed_block& b )
{
   try
   {
   if( !_debug_updates.empty() )
      apply_debug_updates();
   }
   FC_LOG_AND_RETHROW()
}

/*void debug_node_plugin::set_json_object_stream( const std::string& filename )
{
   if( _json_object_stream )
   {
      _json_object_stream->close();
      _json_object_stream.reset();
   }
   _json_object_stream = std::make_shared< std::ofstream >( filename );
}*/

/*void debug_node_plugin::flush_json_object_stream()
{
   if( _json_object_stream )
      _json_object_stream->flush();
}*/

/*void debug_node_plugin::save_debug_updates( fc::mutable_variant_object& target )
{
   for( const std::pair< chain::block_id_type, std::vector< fc::variant_object > >& update : _debug_updates )
   {
      fc::variant v;
      fc::to_variant( update.second, v );
      target.set( update.first.str(), v );
   }
}*/

/*void debug_node_plugin::load_debug_updates( const fc::variant_object& target )
{
   for( auto it=target.begin(); it != target.end(); ++it)
   {
      std::vector< fc::variant_object > o;
      fc::from_variant(it->value(), o);
      _debug_updates[ chain::block_id_type( it->key() ) ] = o;
   }
}*/

void debug_node_plugin::plugin_shutdown()
{
   /*if( _json_object_stream )
   {
      _json_object_stream->close();
      _json_object_stream.reset();
   }*/
   return;
}

} } }

DEFINE_PLUGIN( debug_node, node::plugin::debug_node::debug_node_plugin )
