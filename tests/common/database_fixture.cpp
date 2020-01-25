#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <node/chain/node_objects.hpp>
#include <node/chain/history_object.hpp>
#include <node/account_history/account_history_plugin.hpp>
#include <node/witness/witness_plugin.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

#include <node/protocol/config.hpp>

//using namespace node::chain::test;

fc::time_point TESTING_GENESIS_TIMESTAMP = fc::time_point(fc::microseconds(1590242400000000));

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
   auto wit_plugin = app.register_plugin< node::witness::witness_plugin >();
   init_account_pub_key = init_account_priv_key.get_public_key();

   boost::program_options::variables_map options;

   db_plugin->logging = false;
   ahplugin->plugin_initialize( options );
   db_plugin->plugin_initialize( options );
   wit_plugin->plugin_initialize( options );

   open_database();

   generate_block();
   db.set_hardfork( NUM_HARDFORKS );
   generate_block();

   //ahplugin->plugin_startup();
   db_plugin->plugin_startup();

   validate_database();
   } catch ( const fc::exception& e )
   {
      edump( (e.to_detail_string()) );
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
   init_account_pub_key = init_account_priv_key.get_public_key();

   db.open( data_dir->path(), data_dir->path(), size, chainbase::database::read_write, init_account_pub_key );

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

      db.open( _chain_dir, _chain_dir );

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

fc::ecc::private_key database_fixture::generate_private_key(string seed)
{
   static const fc::ecc::private_key committee = fc::ecc::private_key::regenerate( fc::sha256::hash( string( "init_key" ) ) );
   if( seed == "init_key" )
      return committee;
   return fc::ecc::private_key::regenerate( fc::sha256::hash( seed ) );
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
      db.open( data_dir->path(), data_dir->path(), 1024 * 1024 * 8, chainbase::database::read_write, init_account_pub_key ); // 8 MB file for testing
   }
}

void database_fixture::generate_block(uint32_t skip, const fc::ecc::private_key& key, int miss_blocks)
{
   skip |= default_skip;
   db_plugin->debug_generate_blocks( graphene::utilities::key_to_wif( key ), 1, skip, miss_blocks );
}

void database_fixture::generate_blocks( uint32_t block_count )
{
   auto produced = db_plugin->debug_generate_blocks( debug_key, block_count, default_skip, 0 );
   BOOST_REQUIRE( produced == block_count );
}

void database_fixture::generate_blocks(fc::time_point timestamp, bool miss_intermediate_blocks)
{
   db_plugin->debug_generate_blocks_until( debug_key, timestamp, miss_intermediate_blocks, default_skip );
   BOOST_REQUIRE( ( db.head_block_time() - timestamp ) < BLOCK_INTERVAL );
}

const account_object& database_fixture::account_create(
   const string& name,
   const string& registrar,
   const string& governance_account,
   const private_key_type& registrar_key,
   const share_type& fee,
   const public_key_type& key,
   const public_key_type& post_key,
   const string& details,
   const string& url,
   const string& json
   )
{
   try
   {
      account_create_operation op;

      op.signatory = registrar;
      op.registrar = registrar;
      op.new_account_name = name;
      op.account_type = PERSONA;
      op.referrer = registrar;
      op.proxy = governance_account;
      op.governance_account = governance_account;
      op.recovery_account = governance_account;
      op.details = details;
      op.url = url;
      op.json = json;
      op.json_private = json;
      op.owner = authority( 1, key, 1 );
      op.active = authority( 1, key, 1 );
      op.posting = authority( 1, post_key, 1 );
      op.secure_public_key = string( key );
      op.connection_public_key = string( key );
      op.friend_public_key = string( key );
      op.companion_public_key = string( key );
      op.fee = asset( fee, SYMBOL_COIN );
      op.delegation = asset( 0, SYMBOL_COIN );
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( registrar_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      const account_object& acct = db.get_account( name );

      return acct;
   }
   FC_CAPTURE_AND_RETHROW( (name)(registrar) )
}

const account_object& database_fixture::account_create(
   const string& name,
   const public_key_type& owner_key,
   const public_key_type& posting_key
)
{
   try
   {
      return account_create(
         name,
         INIT_ACCOUNT,
         INIT_ACCOUNT,
         init_account_priv_key,
         std::max( db.get_witness_schedule().median_props.account_creation_fee.amount, share_type( 100 ) ),
         owner_key,
         posting_key,
         "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.",
         "https://en.wikipedia.org/wiki/Loch_Ness_Monster",
         "{\"cookie_price\":\"3.50 MUSD\"}" );
   }
   FC_CAPTURE_AND_RETHROW( (name) );
}

const account_object& database_fixture::account_create(
   const string& name,
   const public_key_type& key
)
{
   return account_create( name, key, key );
}

const account_object& database_fixture::profile_create(
   const string& name,
   const string& registrar,
   const string& governance_account,
   const private_key_type& registrar_key,
   const share_type& fee,
   const public_key_type& key,
   const public_key_type& post_key,
   const string& details,
   const string& url,
   const string& json
   )
{
   try
   {
      account_create_operation op;

      op.signatory = registrar;
      op.registrar = registrar;
      op.new_account_name = name;
      op.account_type = PROFILE;
      op.referrer = registrar;
      op.proxy = governance_account;
      op.governance_account = governance_account;
      op.recovery_account = governance_account;
      op.details = details;
      op.url = url;
      op.json = json;
      op.json_private = json;
      op.owner = authority( 1, key, 1 );
      op.active = authority( 1, key, 1 );
      op.posting = authority( 1, post_key, 1 );
      op.secure_public_key = string( key );
      op.connection_public_key = string( key );
      op.friend_public_key = string( key );
      op.companion_public_key = string( key );
      op.fee = asset( fee, SYMBOL_COIN );
      op.delegation = asset( 0, SYMBOL_COIN );
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( registrar_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      const account_object& acct = db.get_account( name );

      return acct;
   }
   FC_CAPTURE_AND_RETHROW( (name)(registrar) )
}

const account_object& database_fixture::business_create(
   const string& name,
   const string& registrar,
   const string& governance_account,
   const private_key_type& registrar_key,
   const share_type& fee,
   const public_key_type& key,
   const public_key_type& post_key,
   const string& details,
   const string& url,
   const string& json
   )
{
   try
   {
      account_create_operation op;

      op.signatory = registrar;
      op.registrar = registrar;
      op.new_account_name = name;
      op.account_type = BUSINESS;
      op.referrer = registrar;
      op.proxy = governance_account;
      op.governance_account = governance_account;
      op.recovery_account = governance_account;
      op.details = details;
      op.url = url;
      op.json = json;
      op.json_private = json;
      op.owner = authority( 1, key, 1 );
      op.active = authority( 1, key, 1 );
      op.posting = authority( 1, post_key, 1 );
      op.secure_public_key = string( key );
      op.connection_public_key = string( key );
      op.friend_public_key = string( key );
      op.companion_public_key = string( key );
      op.fee = asset( fee, SYMBOL_COIN );
      op.delegation = asset( 0, SYMBOL_COIN );
      op.business_type = PUBLIC_BUSINESS;
      op.officer_vote_threshold = 10 * BLOCKCHAIN_PRECISION;
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( registrar_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      const account_object& acct = db.get_account( name );

      return acct;
   }
   FC_CAPTURE_AND_RETHROW( (name)(registrar) )
}

const board_object& database_fixture::board_create(
   const string& name,
   const string& founder,
   const private_key_type& founder_key,
   const public_key_type& board_key,
   const string& board_type,
   const string& board_privacy,
   const string& details,
   const string& url,
   const string& json
   )
{
   try
   {
      board_create_operation op;

      op.signatory = founder;
      op.founder = founder;
      op.name = name;
      op.board_type = BOARD;
      op.board_privacy = OPEN_BOARD;
      op.board_public_key = string( board_key );
      op.json = json;
      op.json_private = json;
      op.details = details;
      op.url = url;
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( founder_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      const board_object& board = db.get_board( name );

      return board;
   }
   FC_CAPTURE_AND_RETHROW( (name)(founder) )
}

const asset_object& database_fixture::asset_create(
   const string& symbol,
   const string& issuer,
   const private_key_type& issuer_key,
   const asset_property_type& asset_type,
   const string& details,
   const string& url,
   const string& json,
   const share_type& liquidity
   )
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
      op.common_options.display_symbol = symbol;
      op.common_options.json = json;
      op.common_options.details = details;
      op.common_options.url = url;
      
      trx.operations.push_back( op );
      
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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

const witness_object& database_fixture::witness_create(
   const string& owner,
   const private_key_type& owner_key,
   const public_key_type& signing_key )
{
   try
   {
      witness_update_operation op;

      op.signatory = owner;
      op.owner = owner;
      op.details = "details";
      op.url = "www.url.com";
      op.json = "{\"json\":\"valid\"}";
      op.block_signing_key = string( signing_key );

      trx.operations.push_back( op );
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( owner_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      return db.get_witness( owner );
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
      op.parent_permlink = permlink;
      op.title = "test";
      op.body = "test";
      op.board = INIT_BOARD;
      op.options.post_type = TEXT_POST;
      op.language = "en";
      op.options.reach = TAG_FEED;
      op.interface = INIT_ACCOUNT;
      op.options.rating = GENERAL;
      op.tags.push_back( "test" );
      op.json = "{\"json\":\"valid\"}";

      trx.operations.push_back( op );
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( author_key, db.get_chain_id() );
      trx.validate();
      db.push_transaction( trx, 0 );
      trx.operations.clear();
      trx.signatures.clear();

      return db.get_comment( author, permlink );
   }
   FC_CAPTURE_AND_RETHROW( (author)(permlink) )
}

void database_fixture::fund(
   const string& account_name,
   const asset& amount
   )
{
   try
   {
      db_plugin->debug_update( [=]( database& db)
      {
         db.adjust_liquid_balance( account_name, amount );
      }, default_skip );
   }
   FC_CAPTURE_AND_RETHROW( (account_name)(amount) )
}

void database_fixture::fund_stake(
   const string& account_name,
   const asset& amount
   )
{
   try
   {
      db_plugin->debug_update( [=]( database& db)
      {
         db.adjust_staked_balance( account_name, amount );
      }, default_skip );
   }
   FC_CAPTURE_AND_RETHROW( (account_name)(amount) )
}

void database_fixture::fund_stake(
   const string& account_name,
   const asset& amount
   )
{
   try
   {
      db_plugin->debug_update( [=]( database& db)
      {
         db.adjust_reward_balance( account_name, amount );
      }, default_skip );
   }
   FC_CAPTURE_AND_RETHROW( (account_name)(amount) )
}

void database_fixture::fund_savings(
   const string& account_name,
   const asset& amount
   )
{
   try
   {
      db_plugin->debug_update( [=]( database& db)
      {
         db.adjust_savings_balance( account_name, amount );
      }, default_skip );
   }
   FC_CAPTURE_AND_RETHROW( (account_name)(amount) )
}

void database_fixture::proxy( const string& account, const string& proxy )
{
   try
   {
      account_update_proxy_operation op;
      op.account = account;
      op.proxy = proxy;
      trx.operations.push_back( op );
      db.push_transaction( trx, ~0 );
      trx.operations.clear();
   } FC_CAPTURE_AND_RETHROW( (account)(proxy) )
}

const asset& database_fixture::get_liquid_balance( const string& account_name, const string& symbol )const
{
  return db.get_liquid_balance( account_name, symbol );
}

const asset& database_fixture::get_staked_balance( const string& account_name, const string& symbol )const
{
  return db.get_staked_balance( account_name, symbol );
}

const asset& database_fixture::get_savings_balance( const string& account_name, const string& symbol )const
{
  return db.get_savings_balance( account_name, symbol );
}

const asset& database_fixture::get_reward_balance( const string& account_name, const string& symbol )const
{
  return db.get_reward_balance( account_name, symbol );
}

const time_point& database_fixture::now()const
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
