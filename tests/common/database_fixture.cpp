#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <node/chain/node_objects.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/history_object.hpp>
#include <node/account_history/account_history_plugin.hpp>
#include <node/producer/producer_plugin.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

#include <node/protocol/config.hpp>

//using namespace node::chain::test;

fc::time_point TESTING_GENESIS_TIMESTAMP = fc::time_point(fc::microseconds(1588839092000000));

namespace node { namespace chain {

using std::cout;
using std::cerr;

clean_database_fixture::clean_database_fixture()
{
   try {
      int argc = boost::unit_test::framework::master_test_suite().argc;
      char** argv = boost::unit_test::framework::master_test_suite().argv;
      for( int i=1; i<argc; i++ )
      {
         const std::string arg = argv[i];
         if( arg == "--record-assert-trip" )
            fc::enable_record_assert_trip = true;
         if( arg == "--show-test-names" )
            std::cout << "running test " << boost::unit_test::framework::current_test_case().p_name << std::endl;
      }
      auto ahplugin = app.register_plugin< node::account_history::account_history_plugin >();
      db_plugin = app.register_plugin< node::plugin::debug_node::debug_node_plugin >();
      auto producer_plugin = app.register_plugin< node::producer::producer_plugin >();

      boost::program_options::variables_map options;

      db_plugin->logging = false;
      ahplugin->plugin_initialize( options );
      db_plugin->plugin_initialize( options );
      producer_plugin->plugin_initialize( options );

      open_database();

      db_plugin->plugin_startup();

      db.set_hardfork( NUM_HARDFORKS );

      generate_liquid( INIT_ACCOUNT, asset( 3000000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      generate_liquid( INIT_ACCOUNT, asset( 1000000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      generate_liquid( INIT_ACCOUNT, asset( 1000000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );

      call_order_operation call;

      call.signatory = INIT_ACCOUNT;
      call.owner = INIT_ACCOUNT;
      call.collateral = asset( 2000000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );  // 2x collateralization
      call.debt = asset( 1000000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      call.interface = INIT_ACCOUNT;
      call.validate();

      trx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.set_reference_block( db.head_block_id() );
      trx.operations.push_back( call );
      trx.sign( init_account_private_active_key, db.get_chain_id() );
      db.push_transaction( trx, 0 );

      trx.operations.clear();
      trx.signatures.clear();

      //ahplugin->plugin_startup();

      validate_database();
   } 
   catch( const fc::exception& e )
   {
      edump( ( e.to_detail_string() ) );
      throw;
   }

   return;
}

clean_database_fixture::~clean_database_fixture()
{ try {
   // If we're unwinding due to an exception, don't do any more checks.
   // This way, boost test's last checkpoint tells us approximately where the error was.
   if( !std::uncaught_exception() )
   {
      BOOST_CHECK( db.get_node_properties().skip_flags == database::skip_nothing );
   }

   if( data_dir )
      db.close();
   return;
} FC_CAPTURE_AND_RETHROW() }

void clean_database_fixture::resize_shared_mem( uint64_t size )
{
   db.wipe( data_dir->path(), data_dir->path(), true );
   int argc = boost::unit_test::framework::master_test_suite().argc;
   char** argv = boost::unit_test::framework::master_test_suite().argv;
   for( int i=1; i<argc; i++ )
   {
      const std::string arg = argv[i];
      if( arg == "--record-assert-trip" )
         fc::enable_record_assert_trip = true;
      if( arg == "--show-test-names" )
         std::cout << "running test " << boost::unit_test::framework::current_test_case().p_name << std::endl;
   }

   db.open( data_dir->path(), data_dir->path(), size, chainbase::database::read_write );

   boost::program_options::variables_map options;

   generate_block();
   db.set_hardfork( NUM_HARDFORKS );
   generate_block();
   
   validate_database();
}

live_database_fixture::live_database_fixture()
{
   try
   {
      ilog( "Loading saved chain" );
      _chain_dir = fc::current_path() / "test_blockchain";
      FC_ASSERT( fc::exists( _chain_dir ), "Requires blockchain to test on in ./test_blockchain" );

      auto ahplugin = app.register_plugin< node::account_history::account_history_plugin >();
      ahplugin->plugin_initialize( boost::program_options::variables_map() );

      db.open( _chain_dir, _chain_dir, 1024 * 1024 * 8, chainbase::database::read_write );

      validate_database();
      generate_block();

      ilog( "Done loading saved chain" );
   }
   FC_LOG_AND_RETHROW()
}

live_database_fixture::~live_database_fixture()
{
   try
   {
      // If we're unwinding due to an exception, don't do any more checks.
      // This way, boost test's last checkpoint tells us approximately where the error was.
      if( !std::uncaught_exception() )
      {
         BOOST_CHECK( db.get_node_properties().skip_flags == database::skip_nothing );
      }

      db.pop_block();
      db.close();
      return;
   }
   FC_LOG_AND_RETHROW()
}

string database_fixture::generate_anon_acct_name()
{
   // names of the form "anon-acct-x123"
   return "anon-acct-x" + std::to_string( anon_acct_count++ );
}

void database_fixture::open_database()
{
   if( !data_dir ) 
   {
      data_dir = fc::temp_directory( graphene::utilities::temp_directory_path() );
      db._log_hardforks = false;
      db.open( data_dir->path(), data_dir->path(), 1024 * 1024 * 512, chainbase::database::read_write ); // 512 MB file for testing
   }
}

void database_fixture::generate_block()
{
   db_plugin->debug_generate_blocks( 1, 0, 0 );
}

void database_fixture::generate_block( uint32_t skip )
{
   skip |= default_skip;
   db_plugin->debug_generate_blocks( 1, skip, 0 );
}

void database_fixture::generate_block( uint32_t skip, int miss_blocks )
{
   skip |= default_skip;
   db_plugin->debug_generate_blocks( 1, skip, miss_blocks );
}

void database_fixture::generate_blocks( uint32_t block_count )
{
   auto produced = db_plugin->debug_generate_blocks( block_count, default_skip, 0 );
   BOOST_REQUIRE( produced == block_count );
}

void database_fixture::generate_blocks( uint32_t block_count, int miss_blocks )
{
   auto produced = db_plugin->debug_generate_blocks( block_count, default_skip, miss_blocks );
   BOOST_REQUIRE( produced == block_count );
}

void database_fixture::generate_blocks( fc::time_point timestamp, bool miss_intermediate_blocks )
{
   if( timestamp < db.head_block_time() + fc::days(90))
   {
      db_plugin->debug_generate_blocks_until( timestamp, miss_intermediate_blocks, default_skip );
      BOOST_REQUIRE( ( db.head_block_time() - timestamp ) < BLOCK_INTERVAL );
   }
   else
   {
      time_point month = db.head_block_time() + fc::days(90);
      db_plugin->debug_generate_blocks_until( month, miss_intermediate_blocks, default_skip );
      BOOST_REQUIRE( ( db.head_block_time() - month ) < BLOCK_INTERVAL );
   }
}

void database_fixture::generate_until_block( uint64_t head_block_num, bool miss_intermediate_blocks )
{
   db_plugin->debug_generate_until_block( head_block_num, miss_intermediate_blocks, default_skip );
   BOOST_REQUIRE( db.head_block_num() == head_block_num );
}

const account_object& database_fixture::account_create(
   const string& new_account_name,
   const private_key_type& private_secure_key,
   const public_key_type& public_owner_key,
   const public_key_type& public_active_key,
   const public_key_type& public_posting_key,
   const public_key_type& public_secure_key,
   const public_key_type& public_connection_key,
   const public_key_type& public_friend_key,
   const public_key_type& public_companion_key )
{
   try
   {
      account_create_operation op;

      op.signatory = INIT_ACCOUNT;
      op.registrar = INIT_ACCOUNT;
      op.new_account_name = new_account_name;
      op.referrer = INIT_ACCOUNT;
      op.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      op.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      op.details = "Account Details";
      op.url = "https://www.url.com";
      op.json = "{ \"valid\": true }";
      op.json_private = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#{ \"valid\": true }" );
      op.first_name = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#First" );
      op.last_name = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#Name" );
      op.gender = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#Male" );
      op.date_of_birth = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#14-03-1980" );
      op.email = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#firstname.lastname@email.com" );
      op.phone = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#0400111222" );
      op.nationality = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#Australia" );
      op.relationship = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#In a Relationship" );
      op.political_alignment = get_encrypted_message( private_secure_key, public_secure_key, public_connection_key, "#Centrist" );
      op.owner_auth = authority( 1, public_owner_key, 1 );
      op.active_auth = authority( 1, public_active_key, 1 );
      op.posting_auth = authority( 1, public_posting_key, 1 );
      op.secure_public_key = string( public_secure_key );
      op.connection_public_key = string( public_connection_key );
      op.friend_public_key = string( public_friend_key );
      op.companion_public_key = string( public_companion_key );
      op.delegation = asset( 0, SYMBOL_COIN );

      size_t name_length = new_account_name.size();
      asset acc_fee = db.get_median_chain_properties().account_creation_fee;

      if( is_premium_account_name( new_account_name ) )    // Double fee per character less than 8 characters.
      {
         acc_fee.amount = share_type( acc_fee.amount.value << uint16_t( 8 - name_length ) );
      }

      op.fee = asset( std::max( acc_fee.amount, BLOCKCHAIN_PRECISION ), SYMBOL_COIN );
      op.validate();
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.set_reference_block( db.head_block_id() );
      trx.sign( init_account_private_owner_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      const account_object& acct = db.get_account( new_account_name );

      return acct;
   }
   FC_CAPTURE_AND_RETHROW( (new_account_name) )
}

const account_object& database_fixture::account_create(
   const string& name,
   const private_key_type& private_secure_key,
   const public_key_type& key )
{
   return account_create( name, private_secure_key, key, key, key, key, key, key, key );
}

const community_object& database_fixture::community_create(
   const string& name,
   const string& founder,
   const private_key_type& founder_key,
   const public_key_type& community_key,
   const string& community_privacy,
   const string& details,
   const string& url,
   const string& json )
{
   try
   {
      community_create_operation op;

      op.signatory = founder;
      op.founder = founder;
      op.name = name;
      op.community_privacy = "open_public";
      op.community_public_key = string( community_key );
      op.json = json;
      op.json_private = json;
      op.details = details;
      op.url = url;
      op.validate();
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.set_reference_block( db.head_block_id() );
      trx.sign( founder_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      const community_object& community = db.get_community( name );

      return community;
   }
   FC_CAPTURE_AND_RETHROW( (name)(founder) )
}

const asset_object& database_fixture::asset_create(
   const string& symbol,
   const string& issuer,
   const private_key_type& issuer_key,
   const string& asset_type,
   const string& details,
   const string& url,
   const string& json,
   const share_type& liquidity )
{
   try
   {
      asset_create_operation op;

      op.signatory = issuer;
      op.issuer = issuer;
      op.symbol = symbol;
      op.asset_type = asset_type;
      op.coin_liquidity = asset( liquidity, SYMBOL_COIN );
      op.usd_liquidity = asset( liquidity, SYMBOL_USD );
      op.credit_liquidity = asset( liquidity, SYMBOL_CREDIT );
      op.options.display_symbol = symbol;
      op.options.json = json;
      op.options.details = details;
      op.options.url = url;
      op.validate();
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.set_reference_block( db.head_block_id() );
      trx.sign( issuer_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      const asset_object& asset = db.get_asset( symbol );

      return asset;
   }
   FC_CAPTURE_AND_RETHROW()
}

const producer_object& database_fixture::producer_create(
   const string& owner,
   const private_key_type& owner_key )
{
   try
   {
      producer_update_operation op;

      op.signatory = owner;
      op.owner = owner;
      op.details = "details";
      op.url = "https://www.url.com";
      op.json = "{ \"valid\": true }";
      op.block_signing_key = string( node::protocol::get_public_key( owner, "producer", INIT_ACCOUNT_PASSWORD ) );
      op.latitude = 37.8136;
      op.longitude = 144.9631;
      op.active = true;
      op.validate();

      trx.operations.push_back( op );
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.set_reference_block( db.head_block_id() );
      trx.sign( owner_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      
      trx.operations.clear();
      trx.signatures.clear();

      return db.get_producer( owner );
   }
   FC_CAPTURE_AND_RETHROW( (owner) )
}


const producer_vote_object& database_fixture::producer_vote(
   const string& owner,
   const private_key_type& owner_key )
{
   try
   {
      account_producer_vote_operation op;

      op.signatory = owner;
      op.account = owner;
      op.producer = owner;
      op.vote_rank = 1;
      op.validate();

      trx.operations.clear();
      trx.signatures.clear();
      op.validate();

      trx.operations.push_back( op );
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.set_reference_block( db.head_block_id() );
      trx.sign( owner_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      
      trx.operations.clear();
      trx.signatures.clear();

      return db.get_producer_vote( owner, owner );
   }
   FC_CAPTURE_AND_RETHROW( (owner) )
}

const comment_object& database_fixture::comment_create(
      const string& author, 
      const private_key_type& author_key,
      const string& permlink )
{
   try
   {
      comment_operation op;

      op.signatory = author;
      op.author = author;
      op.permlink = permlink;
      op.parent_author = ROOT_POST_PARENT;
      op.parent_permlink = permlink;
      op.title = "test";
      op.body = "test";
      op.community = INIT_COMMUNITY;
      op.public_key = "";
      op.language = "en";
      op.interface = INIT_ACCOUNT;
      op.tags.push_back( tag_name_type( "test" ) );
      op.json = "{ \"valid\": true }";
      op.url = "https://www.url.com";
      op.latitude = 37.8136;
      op.longitude = 144.9631;
      op.comment_price = asset( 0, SYMBOL_COIN );
      op.reply_price = asset( 0, SYMBOL_COIN );
      op.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = "article";
      options.reach = "tag";
      options.reply_connection = "public";
      options.rating = 1;

      op.options = options;

      op.validate();
      trx.operations.push_back( op );
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.set_reference_block( db.head_block_id() );
      trx.sign( author_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      return db.get_comment( author, permlink );
   }
   FC_CAPTURE_AND_RETHROW( (author)(permlink) )
}


void database_fixture::generate_liquid(
   const string& account_name,
   const asset& amount )
{
   db.adjust_liquid_balance( account_name, amount );
}

void database_fixture::generate_stake(
   const string& account_name,
   const asset& amount )
{
   db.adjust_staked_balance( account_name, amount );
}

void database_fixture::generate_savings(
   const string& account_name,
   const asset& amount )
{
   db.adjust_savings_balance( account_name, amount );
}

void database_fixture::generate_reward(
   const string& account_name,
   const asset& amount )
{
   db.adjust_reward_balance( account_name, amount );
}


void database_fixture::fund_liquid(
   const string& account_name,
   const asset& amount )
{
   transfer_operation op;

   op.signatory = INIT_ACCOUNT;
   op.from = INIT_ACCOUNT;
   op.to = account_name;
   op.amount = amount;
   op.memo = "Funding";
   op.validate();
   
   trx.operations.push_back( op );
   
   trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
   trx.set_reference_block( db.head_block_id() );
   trx.sign( init_account_private_active_key, db.get_chain_id() );
   trx.validate();
   db.push_transaction( trx, 0 );
   trx.operations.clear();
   trx.signatures.clear();
}

void database_fixture::fund_stake(
   const string& account_name,
   const asset& amount )
{
   stake_asset_operation op;

   op.signatory = INIT_ACCOUNT;
   op.from = INIT_ACCOUNT;
   op.to = account_name;
   op.amount = amount;
   op.validate();
   
   trx.operations.push_back( op );
   
   trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
   trx.set_reference_block( db.head_block_id() );
   trx.sign( init_account_private_active_key, db.get_chain_id() );
   trx.validate();
   db.push_transaction( trx, 0 );
   trx.operations.clear();
   trx.signatures.clear();
}

void database_fixture::fund_savings(
   const string& account_name,
   const asset& amount )
{
   transfer_to_savings_operation op;

   op.signatory = INIT_ACCOUNT;
   op.from = INIT_ACCOUNT;
   op.to = account_name;
   op.amount = amount;
   op.memo = "Funding";
   op.validate();
   
   trx.operations.push_back( op );
   
   trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
   trx.set_reference_block( db.head_block_id() );
   trx.sign( init_account_private_active_key, db.get_chain_id() );
   trx.validate();
   db.push_transaction( trx, 0 );
   trx.operations.clear();
   trx.signatures.clear();
}

void database_fixture::proxy( const string& account, const string& proxy )
{
   try
   {
      account_update_proxy_operation op;
      op.account = account;
      op.proxy = proxy;
      op.validate();

      trx.operations.push_back( op );
      db.push_transaction( trx, ~0 );
      trx.operations.clear();
   } FC_CAPTURE_AND_RETHROW( (account)(proxy) )
}

asset database_fixture::get_liquid_balance( const string& account_name, const string& symbol )const
{
  return db.get_liquid_balance( account_name, symbol );
}

asset database_fixture::get_staked_balance( const string& account_name, const string& symbol )const
{
  return db.get_staked_balance( account_name, symbol );
}

asset database_fixture::get_savings_balance( const string& account_name, const string& symbol )const
{
  return db.get_savings_balance( account_name, symbol );
}

asset database_fixture::get_reward_balance( const string& account_name, const string& symbol )const
{
  return db.get_reward_balance( account_name, symbol );
}

time_point database_fixture::now()const
{
  return db.head_block_time();
}

void database_fixture::sign(signed_transaction& trx, const fc::ecc::private_key& key)
{
   trx.sign( key, db.get_chain_id() );
}

vector< operation > database_fixture::get_last_operations( uint32_t num_ops )
{
   vector< operation > ops;
   const auto& acc_hist_idx = db.get_index< account_history_index >().indices().get< by_id >();
   auto itr = acc_hist_idx.end();

   while( itr != acc_hist_idx.begin() && ops.size() < num_ops )
   {
      itr--;
      ops.push_back( fc::raw::unpack< node::chain::operation >( db.get(itr->op).serialized_op ) );
   }

   return ops;
}

void database_fixture::validate_database( void )
{
   try
   {
      db.validate_invariants();
   }
   FC_LOG_AND_RETHROW();
}


string database_fixture::get_encrypted_message( 
   string from_private_key, 
   string from_public_key, 
   string to_public_key, 
   string message ) const
{
   if( message.size() > 0 && message[0] == '#' )
   {
      encrypted_message_data m;

      m.from = public_key_type( from_public_key );
      m.to = public_key_type( to_public_key );
      m.nonce = fc::time_point::now().time_since_epoch().count();
      fc::optional< fc::ecc::private_key > privkey = graphene::utilities::wif_to_key( from_private_key );

      if( privkey.valid() )
      {
         fc::sha512 shared_secret = privkey->get_shared_secret( m.to );
         fc::sha512::encoder enc;
         fc::raw::pack( enc, m.nonce );
         fc::raw::pack( enc, shared_secret );
         fc::sha512 encrypt_key = enc.result();

         m.encrypted = fc::aes_encrypt( encrypt_key, fc::raw::pack( message.substr(1) ) );

         m.check = fc::sha256::hash( encrypt_key )._hash[0];

         return m;
      }
      else 
      {
         return message;
      }
   }
   else 
   {
      return message;
   }
}

string database_fixture::get_encrypted_message( 
   private_key_type from_private_key, 
   public_key_type from_public_key, 
   public_key_type to_public_key, 
   string message ) const
{
   if( message.size() > 0 && message[0] == '#' )
   {
      encrypted_message_data m;

      m.from = from_public_key;
      m.to = to_public_key;
      m.nonce = fc::time_point::now().time_since_epoch().count();
      fc::optional< fc::ecc::private_key > privkey = from_private_key;

      if( privkey.valid() )
      {
         fc::sha512 shared_secret = privkey->get_shared_secret( m.to );
         fc::sha512::encoder enc;
         fc::raw::pack( enc, m.nonce );
         fc::raw::pack( enc, shared_secret );
         fc::sha512 encrypt_key = enc.result();

         m.encrypted = fc::aes_encrypt( encrypt_key, fc::raw::pack( message.substr(1) ) );

         m.check = fc::sha256::hash( encrypt_key )._hash[0];

         return m;
      }
      else 
      {
         return message;
      }
   }
   else 
   {
      return message;
   }
}


namespace test {

bool _push_block( database& db, const signed_block& b, uint32_t skip_flags /* = 0 */ )
{
   return db.push_block( b, skip_flags);
}

void _push_transaction( database& db, const signed_transaction& tx, uint32_t skip_flags /* = 0 */ )
{ try {
   db.push_transaction( tx, skip_flags );
} FC_CAPTURE_AND_RETHROW((tx)) }

} // node::chain::test

} } // node::chain
