
#include <fc/filesystem.hpp>
#include <fc/optional.hpp>
#include <fc/variant_object.hpp>

#include <node/app/api_context.hpp>
#include <node/app/application.hpp>

#include <node/protocol/block.hpp>
#include <node/chain/block_log.hpp>
#include <node/chain/database.hpp>
#include <node/chain/node_objects.hpp>

#include <graphene/utilities/key_conversion.hpp>

#include <node/plugins/debug_node/debug_node_api.hpp>
#include <node/plugins/debug_node/debug_node_plugin.hpp>

namespace node { namespace plugin { namespace debug_node {

namespace detail {

class debug_private_key_storage : public private_key_storage
{
   public:
      debug_private_key_storage() {}
      virtual ~debug_private_key_storage() {}

      virtual void maybe_get_private_key(
         fc::optional< fc::ecc::private_key >& result,
         const node::protocol::public_key_type& pubkey,
         const std::string& account_name
         ) override;

      std::string                dev_key_prefix;
      std::map< node::protocol::public_key_type, fc::ecc::private_key > key_table;
};

class debug_node_api_impl
{
   public:
      debug_node_api_impl( node::app::application& _app );

      uint32_t debug_push_blocks( const std::string& src_filename, uint32_t count, bool skip_validate_invariants );
      uint32_t debug_generate_blocks( uint32_t count );
      uint32_t debug_generate_blocks_until( const fc::time_point& head_block_time, bool generate_sparsely );
      uint32_t debug_generate_until_block( uint64_t head_block_num, bool generate_sparsely );
      
      fc::optional< node::chain::signed_block > debug_pop_block();
      //void debug_push_block( const node::chain::signed_block& block );
      node::chain::producer_schedule_object debug_get_producer_schedule();
      node::chain::hardfork_property_object debug_get_hardfork_property_object();
      void debug_update_object( const fc::variant_object& update );
      fc::variant_object debug_get_edits();
      void debug_set_edits( const fc::variant_object& edits );
      //void debug_save_db( std::string db_path );
      void debug_stream_json_objects( const std::string& filename );
      void debug_stream_json_objects_flush();
      void debug_set_hardfork( uint32_t hardfork_id );
      bool debug_has_hardfork( uint32_t hardfork_id );
      void debug_get_json_schema( std::string& schema );
      void debug_set_dev_key_prefix( std::string prefix );
      void debug_mine( debug_mine_result& result, const debug_mine_args& args );
      void debug_get_dev_key( get_dev_key_result& result, const get_dev_key_args& args );
      std::shared_ptr< node::plugin::debug_node::debug_node_plugin > get_plugin();

      node::app::application& app;
      debug_private_key_storage key_storage;
};

void debug_private_key_storage::maybe_get_private_key(
   fc::optional< fc::ecc::private_key >& result,
   const node::protocol::public_key_type& pubkey,
   const std::string& account_name
)
{
   auto it = key_table.find( pubkey );
   if( it != key_table.end() )
   {
      result = it->second;
      return;
   }
   fc::ecc::private_key gen_priv = fc::ecc::private_key::regenerate( fc::sha256::hash( dev_key_prefix + account_name ) );
   chain::public_key_type gen_pub = gen_priv.get_public_key();
   key_table[ gen_pub ] = gen_priv;
   if( (pubkey == node::protocol::public_key_type()) || (gen_pub == pubkey) )
   {
      result = gen_priv;
      return;
   }

   result.reset();
   return;
}

debug_node_api_impl::debug_node_api_impl( node::app::application& _app ) : app( _app )
{
#ifdef INIT_PRIVATE_KEY
   fc::ecc::private_key init_key = INIT_PRIVATE_KEY;
   key_storage.key_table[ init_key.get_public_key() ] = init_key;
#endif

   std::shared_ptr< chain::database > db = app.chain_database();

   const auto& acct_idx = db->get_index< chain::account_index >().indices().get< chain::by_name >();
   auto acct_itr = acct_idx.begin();

   while( acct_itr != acct_idx.end() )     // Inject all Accounts to have init password keys added.
   {
      node::protocol::private_key_type private_owner_key = node::protocol::get_private_key( acct_itr->name, OWNER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      node::protocol::private_key_type private_active_key = node::protocol::get_private_key( acct_itr->name, ACTIVE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      node::protocol::private_key_type private_posting_key = node::protocol::get_private_key( acct_itr->name, POSTING_KEY_STR, INIT_ACCOUNT_PASSWORD );
      node::protocol::private_key_type private_secure_key = node::protocol::get_private_key( acct_itr->name, SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      node::protocol::private_key_type private_connection_key = node::protocol::get_private_key( acct_itr->name, CONNECTION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      node::protocol::private_key_type private_friend_key = node::protocol::get_private_key( acct_itr->name, FRIEND_KEY_STR, INIT_ACCOUNT_PASSWORD );
      node::protocol::private_key_type private_companion_key = node::protocol::get_private_key( acct_itr->name, COMPANION_KEY_STR, INIT_ACCOUNT_PASSWORD );
      node::protocol::private_key_type private_producer_key = node::protocol::get_private_key( acct_itr->name, PRODUCER_KEY_STR, INIT_ACCOUNT_PASSWORD );

      node::protocol::public_key_type public_owner_key = private_owner_key.get_public_key();
      node::protocol::public_key_type public_active_key = private_active_key.get_public_key();
      node::protocol::public_key_type public_posting_key = private_posting_key.get_public_key();
      node::protocol::public_key_type public_secure_key = private_secure_key.get_public_key();
      node::protocol::public_key_type public_connection_key = private_connection_key.get_public_key();
      node::protocol::public_key_type public_friend_key = private_friend_key.get_public_key();
      node::protocol::public_key_type public_companion_key = private_companion_key.get_public_key();
      node::protocol::public_key_type public_producer_key = private_producer_key.get_public_key();

      key_storage.key_table[ public_owner_key ] = private_owner_key;
      key_storage.key_table[ public_active_key ] = private_active_key;
      key_storage.key_table[ public_posting_key ] = private_posting_key;
      key_storage.key_table[ public_secure_key ] = private_secure_key;
      key_storage.key_table[ public_connection_key ] = private_connection_key;
      key_storage.key_table[ public_friend_key ] = private_friend_key;
      key_storage.key_table[ public_companion_key ] = private_companion_key;
      key_storage.key_table[ public_producer_key ] = private_producer_key;

      ilog( "Added Debug Account keys: ${a}",("a",acct_itr->name ) );
   }
}

void debug_node_api_impl::debug_set_dev_key_prefix( std::string prefix )
{
   key_storage.dev_key_prefix = prefix;
   return;
}

void debug_node_api_impl::debug_get_dev_key( get_dev_key_result& result, const get_dev_key_args& args )
{
   fc::ecc::private_key priv = fc::ecc::private_key::regenerate( fc::sha256::hash( key_storage.dev_key_prefix + args.name ) );
   result.private_key = graphene::utilities::key_to_wif( priv );
   result.public_key = priv.get_public_key();
   return;
}

void debug_node_api_impl::debug_mine( debug_mine_result& result, const debug_mine_args& args )
{
   std::shared_ptr< chain::database > db = app.chain_database();

   chain::x11_proof_of_work work;
   work.input.miner_account = args.miner_account;
   work.input.prev_block = db->head_block_id();
   get_plugin()->debug_mine_work( work, db->pow_difficulty() );

   chain::proof_of_work_operation op;
   op.work = work;
      
   const auto& acct_idx  = db->get_index< chain::account_index >().indices().get< chain::by_name >();
   auto acct_it = acct_idx.find( args.miner_account );
   auto acct_auth = db->find< chain::account_authority_object, chain::by_account >( args.miner_account );
   bool has_account = (acct_it != acct_idx.end());

   fc::optional< fc::ecc::private_key > priv;
   if( !has_account )
   {
      // this copies logic from get_dev_key
      priv = fc::ecc::private_key::regenerate( fc::sha256::hash( key_storage.dev_key_prefix + args.miner_account ) );
      op.new_owner_key = string( chain::public_key_type( priv->get_public_key() ) );
   }
   else
   {
      chain::public_key_type pubkey;
      if( acct_auth->active_auth.key_auths.size() != 1 )
      {
         elog( "debug_mine does not understand authority for miner account ${miner}", ("miner", args.miner_account) );
      }
      FC_ASSERT( acct_auth->active_auth.key_auths.size() == 1 );
      pubkey = acct_auth->active_auth.key_auths.begin()->first;
      key_storage.maybe_get_private_key( priv, pubkey, args.miner_account );
   }
   FC_ASSERT( priv.valid(), "debug_node_api does not know private key for miner account ${miner}", ("miner", args.miner_account) );

   chain::signed_transaction tx;
   tx.operations.push_back(op);
   tx.set_reference_block( db->head_block_id() );
   tx.set_expiration( db->head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

   tx.sign( *priv, CHAIN_ID );

   db->push_transaction( tx );
   return;
}

uint32_t debug_node_api_impl::debug_push_blocks( const std::string& src_filename, uint32_t count, bool skip_validate_invariants )
{
   if( count == 0 )
      return 0;

   std::shared_ptr< node::chain::database > db = app.chain_database();
   fc::path src_path = fc::path( src_filename );
   fc::path index_path = fc::path( src_filename + ".index" );
   if( fc::exists(src_path) && fc::exists(index_path) &&
      !fc::is_directory(src_path) && !fc::is_directory(index_path)
     )
   {
      ilog( "Loading ${n} from block_log ${fn}", ("n", count)("fn", src_filename) );
      idump( (src_filename)(count)(skip_validate_invariants) );
      node::chain::block_log log;
      log.open( src_path );
      uint32_t first_block = db->head_block_num()+1;
      uint32_t skip_flags = node::chain::database::skip_nothing;
      if( skip_validate_invariants )
         skip_flags = skip_flags | node::chain::database::skip_validate_invariants;
      for( uint32_t i=0; i<count; i++ )
      {
         //fc::optional< node::chain::signed_block > block = log.read_block( log.get_block_pos( first_block + i ) );
         uint64_t block_pos = log.get_block_pos( first_block + i );
         if( block_pos == node::chain::block_log::npos )
         {
            wlog( "Block database ${fn} only contained ${i} of ${n} requested blocks", ("i", i)("n", count)("fn", src_filename) );
            return i;
         }

         decltype( log.read_block(0) ) result;

         try
         {
            result = log.read_block( block_pos );
         }
         catch( const fc::exception& e )
         {
            elog( "Could not read block ${i} of ${n}", ("i", i)("n", count) );
            continue;
         }

         try
         {
            db->push_block( result.first, skip_flags );
         }
         catch( const fc::exception& e )
         {
            elog( "Got exception pushing block ${bn} : ${bid} (${i} of ${n})", ("bn", result.first.block_num())("bid", result.first.id())("i", i)("n", count) );
            elog( "Exception backtrace: ${bt}", ("bt", e.to_detail_string()) );
         }
      }
      ilog( "Completed loading block_database successfully" );
      return count;
   }
   return 0;
}

uint32_t debug_node_api_impl::debug_generate_blocks( uint32_t count )
{
   return get_plugin()->debug_generate_blocks( count, node::chain::database::skip_nothing, 0, &key_storage );
}

uint32_t debug_node_api_impl::debug_generate_blocks_until( const fc::time_point& head_block_time, bool generate_sparsely )
{
   return get_plugin()->debug_generate_blocks_until( head_block_time, generate_sparsely, node::chain::database::skip_nothing, &key_storage );
}

uint32_t debug_node_api_impl::debug_generate_until_block( uint64_t head_block_num, bool generate_sparsely )
{
   return get_plugin()->debug_generate_until_block( head_block_num, generate_sparsely, node::chain::database::skip_nothing, &key_storage );
}


fc::optional< node::chain::signed_block > debug_node_api_impl::debug_pop_block()
{
   std::shared_ptr< node::chain::database > db = app.chain_database();
   return db->fetch_block_by_number( db->head_block_num() );
}

/*void debug_node_api_impl::debug_push_block( const node::chain::signed_block& block )
{
   app.chain_database()->push_block( block );
}*/

node::chain::producer_schedule_object debug_node_api_impl::debug_get_producer_schedule()
{
   return app.chain_database()->get( node::chain::producer_schedule_id_type() );
}

node::chain::hardfork_property_object debug_node_api_impl::debug_get_hardfork_property_object()
{
   return app.chain_database()->get( node::chain::hardfork_property_id_type() );
}

void debug_node_api_impl::debug_update_object( const fc::variant_object& update )
{
   //get_plugin()->debug_update( update );
}

/*fc::variant_object debug_node_api_impl::debug_get_edits()
{
   fc::mutable_variant_object result;
   get_plugin()->save_debug_updates( result );
   return fc::variant_object( std::move( result ) );
}*/

void debug_node_api_impl::debug_set_edits( const fc::variant_object& edits )
{
   //get_plugin()->load_debug_updates( edits );
}

std::shared_ptr< node::plugin::debug_node::debug_node_plugin > debug_node_api_impl::get_plugin()
{
   return app.get_plugin< debug_node_plugin >( "debug_node" );
}

void debug_node_api_impl::debug_stream_json_objects( const std::string& filename )
{
   //get_plugin()->set_json_object_stream( filename );
}

void debug_node_api_impl::debug_stream_json_objects_flush()
{
   //get_plugin()->flush_json_object_stream();
}

void debug_node_api_impl::debug_set_hardfork( uint32_t hardfork_id )
{
   using namespace node::chain;

   if( hardfork_id > NUM_HARDFORKS )
      return;

   get_plugin()->debug_update( [=]( database& db )
   {
      db.set_hardfork( hardfork_id, false );
   });
}

bool debug_node_api_impl::debug_has_hardfork( uint32_t hardfork_id )
{
   return app.chain_database()->get( node::chain::hardfork_property_id_type() ).last_hardfork >= hardfork_id;
}

void debug_node_api_impl::debug_get_json_schema( std::string& schema )
{
   schema = app.chain_database()->get_json_schema();
}

} // detail

debug_node_api::debug_node_api( const node::app::api_context& ctx )
{
   my = std::make_shared< detail::debug_node_api_impl >(ctx.app);
}

void debug_node_api::on_api_startup() {}

uint32_t debug_node_api::debug_push_blocks( std::string source_filename, uint32_t count, bool skip_validate_invariants )
{
   return my->debug_push_blocks( source_filename, count, skip_validate_invariants );
}

uint32_t debug_node_api::debug_generate_blocks( uint32_t count )
{
   return my->debug_generate_blocks( count );
}

uint32_t debug_node_api::debug_generate_blocks_until( fc::time_point head_block_time, bool generate_sparsely )
{
   return my->debug_generate_blocks_until( head_block_time, generate_sparsely );
}

uint32_t debug_node_api::debug_generate_until_block( uint64_t head_block_num, bool generate_sparsely )
{
   return my->debug_generate_until_block( head_block_num, generate_sparsely );
}

fc::optional< node::chain::signed_block > debug_node_api::debug_pop_block()
{
   return my->debug_pop_block();
}

/*void debug_node_api::debug_push_block( node::chain::signed_block& block )
{
   my->debug_push_block( block );
}*/

node::chain::producer_schedule_object debug_node_api::debug_get_producer_schedule()
{
   return my->debug_get_producer_schedule();
}

node::chain::hardfork_property_object debug_node_api::debug_get_hardfork_property_object()
{
   return my->debug_get_hardfork_property_object();
}

/*
void debug_node_api::debug_update_object( fc::variant_object update )
{
   my->debug_update_object( update );
}
*/

/*
fc::variant_object debug_node_api::debug_get_edits()
{
   return my->debug_get_edits();
}
*/

/*
void debug_node_api::debug_set_edits( fc::variant_object edits )
{
   my->debug_set_edits(edits);
}
*/

void debug_node_api::debug_set_dev_key_prefix( std::string prefix )
{
   my->debug_set_dev_key_prefix( prefix );
}

get_dev_key_result debug_node_api::debug_get_dev_key( get_dev_key_args args )
{
   get_dev_key_result result;
   my->debug_get_dev_key( result, args );
   return result;
}

debug_mine_result debug_node_api::debug_mine( debug_mine_args args )
{
   debug_mine_result result;
   my->debug_mine( result, args );
   return result;
}

/*
void debug_node_api::debug_stream_json_objects( std::string filename )
{
   my->debug_stream_json_objects( filename );
}

void debug_node_api::debug_stream_json_objects_flush()
{
   my->debug_stream_json_objects_flush();
}
*/

void debug_node_api::debug_set_hardfork( uint32_t hardfork_id )
{
   my->debug_set_hardfork( hardfork_id );
}

bool debug_node_api::debug_has_hardfork( uint32_t hardfork_id )
{
   return my->debug_has_hardfork( hardfork_id );
}

std::string debug_node_api::debug_get_json_schema()
{
   std::string result;
   my->debug_get_json_schema( result );
   return result;
}

} } } // node::plugin::debug_node
