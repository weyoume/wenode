#include <boost/test/unit_test.hpp>
#include <node/protocol/exceptions.hpp>
#include <node/chain/database.hpp>
#include <node/chain/history_object.hpp>
#include <node/account_history/account_history_plugin.hpp>
#include <graphene/utilities/tempdir.hpp>
#include <fc/crypto/digest.hpp>
#include "../common/database_fixture.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace node;
using namespace node::chain;
using namespace node::protocol;

#define TEST_SHARED_MEM_SIZE (1024 * 1024 * 8)

/**

BOOST_FIXTURE_TEST_SUITE( block_tests, clean_database_fixture );

BOOST_AUTO_TEST_CASE( generate_empty_blocks )
{
   try 
   {
      fc::temp_directory data_dir( graphene::utilities::temp_directory_path() );
      signed_block b;

      signed_block cutoff_block;
      {
         database db;
         db._log_hardforks = false;
         db.open( data_dir.path(), data_dir.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
         b = db.generate_block( db.get_slot_time( 1 ), db.get_scheduled_producer(1), get_private_key( db.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);

         // n.b. we generate MIN_UNDO_HISTORY+1 extra blocks which will be discarded on sav
         for( uint32_t i = 1; i<200; ++i )
         {
            BOOST_CHECK( db.head_block_id() == b.id() );
            string cur_producer = db.get_scheduled_producer( 1 );
            b = db.generate_block( db.get_slot_time( 1 ), cur_producer, get_private_key( cur_producer, PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
            BOOST_CHECK( b.producer == cur_producer );
            uint32_t cutoff_height = db.get_dynamic_global_properties().last_irreversible_block_num;
            if( cutoff_height >= 200 )
            {
               auto block = db.fetch_block_by_number( cutoff_height );
               BOOST_REQUIRE( block.valid() );
               cutoff_block = *block;
               break;
            }
         }
         db.close();
      }
      {
         database db;
         db._log_hardforks = false;
         db.open( data_dir.path(), data_dir.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
         BOOST_CHECK_EQUAL( db.head_block_num(), cutoff_block.block_num() );
         b = cutoff_block;

         for( uint32_t i = 0; i < 200; ++i )
         {
            BOOST_CHECK( db.head_block_id() == b.id() );
            string cur_producer = db.get_scheduled_producer( 1 );
            b = db.generate_block(db.get_slot_time( 1 ), cur_producer, get_private_key( cur_producer, PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
         }
         BOOST_CHECK_EQUAL( db.head_block_num(), cutoff_block.block_num()+200 );
      }
   } 
   catch(fc::exception& e)
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_CASE( undo_block )
{
   try {
      fc::temp_directory data_dir( graphene::utilities::temp_directory_path() );
      {
         database db;
         db._log_hardforks = false;
         db.open( data_dir.path(), data_dir.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
         time_point now = TESTING_GENESIS_TIMESTAMP;
         std::vector< time_point > time_stack;

         for( uint32_t i = 0; i < 5; ++i )
         {
            now = db.get_slot_time(1);
            time_stack.push_back( now );
            auto b = db.generate_block( now, db.get_scheduled_producer( 1 ), get_private_key( db.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing );
         }
         BOOST_CHECK( db.head_block_num() == 5 );
         BOOST_CHECK( db.head_block_time() == now );
         db.pop_block();
         time_stack.pop_back();
         now = time_stack.back();
         BOOST_CHECK( db.head_block_num() == 4 );
         BOOST_CHECK( db.head_block_time() == now );
         db.pop_block();
         time_stack.pop_back();
         now = time_stack.back();
         BOOST_CHECK( db.head_block_num() == 3 );
         BOOST_CHECK( db.head_block_time() == now );
         db.pop_block();
         time_stack.pop_back();
         now = time_stack.back();
         BOOST_CHECK( db.head_block_num() == 2 );
         BOOST_CHECK( db.head_block_time() == now );
         for( uint32_t i = 0; i < 5; ++i )
         {
            now = db.get_slot_time(1);
            time_stack.push_back( now );
            auto b = db.generate_block( now, db.get_scheduled_producer( 1 ), get_private_key( db.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing );
         }
         BOOST_CHECK( db.head_block_num() == 7 );
      }
   } 
   catch(fc::exception& e)
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_CASE( fork_blocks )
{
   try {
      fc::temp_directory data_dir1( graphene::utilities::temp_directory_path() );
      fc::temp_directory data_dir2( graphene::utilities::temp_directory_path() );

      database db1;
      db1._log_hardforks = false;
      db1.open( data_dir1.path(), data_dir1.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
      database db2;
      db2._log_hardforks = false;
      db2.open( data_dir2.path(), data_dir2.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );

      for( uint32_t i = 0; i < 10; ++i )
      {
         auto b = db1.generate_block(db1.get_slot_time(1), db1.get_scheduled_producer(1), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
         try {
            PUSH_BLOCK( db2, b );
         } FC_CAPTURE_AND_RETHROW( ("db2") );
      }
      for( uint32_t i = 10; i < 13; ++i )
      {
         auto b =  db1.generate_block(db1.get_slot_time(1), db1.get_scheduled_producer(1), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
      }
      string db1_tip = db1.head_block_id().str();
      uint32_t next_slot = 3;
      for( uint32_t i = 13; i < 16; ++i )
      {
         auto b =  db2.generate_block(db2.get_slot_time(next_slot), db2.get_scheduled_producer(next_slot), get_private_key( db2.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
         next_slot = 1;
         // notify both databases of the new block.
         // only db2 should switch to the new fork, db1 should not
         PUSH_BLOCK( db1, b );
         BOOST_CHECK_EQUAL(db1.head_block_id().str(), db1_tip);
         BOOST_CHECK_EQUAL(db2.head_block_id().str(), b.id().str());
      }

      //The two databases are on distinct forks now, but at the same height. Make a block on db2, make it invalid, then
      //pass it to db1 and assert that db1 doesn't switch to the new fork.
      signed_block good_block;
      BOOST_CHECK_EQUAL(db1.head_block_num(), 13);
      BOOST_CHECK_EQUAL(db2.head_block_num(), 13);
      {
         auto b = db2.generate_block(db2.get_slot_time(1), db2.get_scheduled_producer(1), get_private_key( db2.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
         good_block = b;
         b.transactions.emplace_back(signed_transaction());
         b.transactions.back().operations.emplace_back(transfer_operation());
         b.sign( get_private_key( db2.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ) );
         BOOST_CHECK_EQUAL(b.block_num(), 14);
         CHECK_THROW(PUSH_BLOCK( db1, b ), fc::exception);
      }
      BOOST_CHECK_EQUAL(db1.head_block_num(), 13);
      BOOST_CHECK_EQUAL(db1.head_block_id().str(), db1_tip);

      // assert that db1 switches to new fork with good block
      BOOST_CHECK_EQUAL(db2.head_block_num(), 14);
      PUSH_BLOCK( db1, good_block );
      BOOST_CHECK_EQUAL(db1.head_block_id().str(), db2.head_block_id().str());
   } 
   catch(fc::exception& e)
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_CASE( switch_forks_undo_create )
{
   try {
      fc::temp_directory dir1( graphene::utilities::temp_directory_path() ),
                         dir2( graphene::utilities::temp_directory_path() );
      database db1,
               db2;
      db1._log_hardforks = false;
      db1.open( dir1.path(), dir1.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
      db1.adjust_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      db2._log_hardforks = false;
      db2.open( dir2.path(), dir2.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
      db2.adjust_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      db1.get_index< account_index >();

      signed_transaction trx;
      
      account_create_operation create;

      create.new_account_name = "alice";
      create.registrar = GENESIS_ACCOUNT_BASE_NAME;
      create.referrer = GENESIS_ACCOUNT_BASE_NAME;
      create.owner_auth = authority( 1, init_account_public_owner_key, 1 );
      create.active_auth = authority( 2, init_account_public_active_key, 2 );
      create.posting_auth = authority( 1, init_account_public_posting_key, 1 );
      create.secure_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.connection_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.friend_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.companion_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      trx.operations.push_back( create );
      trx.set_expiration( db1.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( get_private_key( GENESIS_ACCOUNT_BASE_NAME, OWNER_KEY_STR, INIT_PASSWORD ), db1.get_chain_id() );

      PUSH_TX( db1, trx );

      // generate blocks
      // db1 : A
      // db2 : B C D

      auto b = db1.generate_block(db1.get_slot_time(1), db1.get_scheduled_producer(1), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);

      auto alice_id = db1.get_account( create.new_account_name ).id;
      BOOST_CHECK( db1.get(alice_id).name == "alice" );

      b = db2.generate_block(db2.get_slot_time(1), db2.get_scheduled_producer(1), get_private_key( db2.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
      db1.push_block(b);
      b = db2.generate_block(db2.get_slot_time(1), db2.get_scheduled_producer(1), get_private_key( db2.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
      db1.push_block(b);
      REQUIRE_THROW(db2.get(alice_id), std::exception);
      db1.get(alice_id); /// it should be included in the pending state
      db1.clear_pending(); // clear it so that we can verify it was properly removed from pending state.
      REQUIRE_THROW(db1.get(alice_id), std::exception);

      PUSH_TX( db2, trx );

      b = db2.generate_block(db2.get_slot_time(1), db2.get_scheduled_producer(1), get_private_key( db2.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
      db1.push_block(b);

      BOOST_CHECK( db1.get(alice_id).name == "alice");
      BOOST_CHECK( db2.get(alice_id).name == "alice");
   } 
   catch(fc::exception& e) 
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_CASE( duplicate_transactions )
{
   try {
      fc::temp_directory dir1( graphene::utilities::temp_directory_path() ),
                         dir2( graphene::utilities::temp_directory_path() );
      database db1,
               db2;
      db1._log_hardforks = false;
      db1.open( dir1.path(), dir1.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
      db1.adjust_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      db2._log_hardforks = false;
      db2.open( dir2.path(), dir2.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
      db2.adjust_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      BOOST_CHECK( db1.get_chain_id() == db2.get_chain_id() );

      auto skip_sigs = database::skip_transaction_signatures | database::skip_authority_check;

      signed_transaction trx;

      account_create_operation create;

      create.new_account_name = "alice";
      create.registrar = GENESIS_ACCOUNT_BASE_NAME;
      create.referrer = GENESIS_ACCOUNT_BASE_NAME;
      create.owner_auth = authority( 1, init_account_public_owner_key, 1 );
      create.active_auth = authority( 2, init_account_public_active_key, 2 );
      create.posting_auth = authority( 1, init_account_public_posting_key, 1 );
      create.secure_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.connection_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.friend_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.companion_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      trx.operations.push_back(create);
      trx.set_expiration( db1.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( get_private_key( GENESIS_ACCOUNT_BASE_NAME, OWNER_KEY_STR, INIT_PASSWORD ), db1.get_chain_id() );

      PUSH_TX( db1, trx, skip_sigs );

      trx = decltype(trx)();

      transfer_operation t;

      t.from = GENESIS_ACCOUNT_BASE_NAME;
      t.to = "alice";
      t.amount = asset( 500, SYMBOL_COIN );
      t.memo = "Hello";

      trx.operations.push_back(t);
      trx.set_expiration( db1.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( init_account_private_active_key, db1.get_chain_id() );

      PUSH_TX( db1, trx, skip_sigs );

      CHECK_THROW(PUSH_TX( db1, trx, skip_sigs ), fc::exception);

      auto b = db1.generate_block( db1.get_slot_time(1), db1.get_scheduled_producer( 1 ), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), skip_sigs );
      PUSH_BLOCK( db2, b, skip_sigs );

      CHECK_THROW(PUSH_TX( db1, trx, skip_sigs ), fc::exception);
      CHECK_THROW(PUSH_TX( db2, trx, skip_sigs ), fc::exception);
      BOOST_CHECK_EQUAL( db1.get_liquid_balance( "alice", SYMBOL_COIN ).amount.value, 500 );
      BOOST_CHECK_EQUAL( db2.get_liquid_balance( "alice", SYMBOL_COIN ).amount.value, 500 );
   } 
   catch(fc::exception& e) 
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_CASE( tapos )
{
   try {
      fc::temp_directory dir1( graphene::utilities::temp_directory_path() );
      database db1;
      db1._log_hardforks = false;
      db1.open(dir1.path(), dir1.path(), TEST_SHARED_MEM_SIZE, chainbase::database::read_write );
      db1.adjust_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      auto b = db1.generate_block( db1.get_slot_time(1), db1.get_scheduled_producer( 1 ), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);

      BOOST_TEST_MESSAGE( "Creating a transaction with reference block" );
      idump((db1.head_block_id()));
      signed_transaction trx;
      //This transaction must be in the next block after its reference, or it is invalid.
      trx.set_reference_block( db1.head_block_id() );

      account_create_operation create;

      create.new_account_name = "alice";
      create.registrar = GENESIS_ACCOUNT_BASE_NAME;
      create.referrer = GENESIS_ACCOUNT_BASE_NAME;
      create.owner_auth = authority( 1, init_account_public_owner_key, 1 );
      create.active_auth = authority( 2, init_account_public_active_key, 2 );
      create.posting_auth = authority( 1, init_account_public_posting_key, 1 );
      create.secure_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.connection_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.friend_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.companion_public_key = string( public_key_type( init_account_public_posting_key ) );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      trx.operations.push_back(create);
      trx.set_expiration( db1.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( get_private_key( GENESIS_ACCOUNT_BASE_NAME, OWNER_KEY_STR, INIT_PASSWORD ), db1.get_chain_id() );

      BOOST_TEST_MESSAGE( "Pushing Pending Transaction" );
      idump((trx));
      db1.push_transaction(trx);
      BOOST_TEST_MESSAGE( "Generating a block" );
      b = db1.generate_block(db1.get_slot_time(1), db1.get_scheduled_producer(1), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
      trx.clear();

      transfer_operation t;

      t.from = GENESIS_ACCOUNT_BASE_NAME;
      t.to = "alice";
      t.amount = asset(50,SYMBOL_COIN);
      t.validate();

      trx.operations.push_back(t);
      trx.set_expiration( db1.head_block_time() + fc::seconds(2) );
      trx.sign( init_account_private_active_key, db1.get_chain_id() );
      idump((trx)(db1.head_block_time()));
      b = db1.generate_block(db1.get_slot_time(1), db1.get_scheduled_producer(1), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
      idump((b));
      b = db1.generate_block(db1.get_slot_time(1), db1.get_scheduled_producer(1), get_private_key( db1.get_scheduled_producer(1), PRODUCER_KEY_STR, INIT_PASSWORD ), database::skip_nothing);
      trx.signatures.clear();
      trx.sign( init_account_private_active_key, db1.get_chain_id() );
      BOOST_REQUIRE_THROW( db1.push_transaction(trx, database::skip_transaction_signatures | database::skip_authority_check), fc::exception );
   } 
   catch(fc::exception& e) 
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_FIXTURE_TEST_CASE( optional_tapos, clean_database_fixture )
{
   try
   {
      ACTORS( (alice)(bob) );

      generate_block();

      BOOST_TEST_MESSAGE( "Create transaction" );

      fund_liquid( "alice", asset( 10000, SYMBOL_COIN ) );

      transfer_operation op;

      op.from = "alice";
      op.to = "bob";
      op.amount = asset( 1000, SYMBOL_COIN );
      op.validate();

      signed_transaction tx;
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "ref_block_num=0, ref_block_prefix=0" );

      tx.ref_block_num = 0;
      tx.ref_block_prefix = 0;
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      PUSH_TX( db, tx );

      BOOST_TEST_MESSAGE( "proper ref_block_num, ref_block_prefix" );

      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      PUSH_TX( db, tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "ref_block_num=0, ref_block_prefix=12345678" );

      tx.ref_block_num = 0;
      tx.ref_block_prefix = 0x12345678;
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( PUSH_TX( db, tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "ref_block_num=1, ref_block_prefix=12345678" );

      tx.ref_block_num = 1;
      tx.ref_block_prefix = 0x12345678;
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( PUSH_TX( db, tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "ref_block_num=9999, ref_block_prefix=12345678" );

      tx.ref_block_num = 9999;
      tx.ref_block_prefix = 0x12345678;
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( PUSH_TX( db, tx, database::skip_transaction_dupe_check ), fc::exception );
   }
   catch (fc::exception& e)
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_FIXTURE_TEST_CASE( double_sign_check, clean_database_fixture )
{ try {
   generate_block();
   ACTOR(bob);

   fund_liquid( GENESIS_ACCOUNT_BASE_NAME, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

   share_type amount = 1000;

   transfer_operation t;

   t.from = GENESIS_ACCOUNT_BASE_NAME;
   t.to = "bob";
   t.amount = asset(amount,SYMBOL_COIN);
   t.validate();

   trx.operations.push_back(t);
   trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
   trx.validate();

   db.push_transaction(trx, ~0);

   trx.operations.clear();

   t.from = "bob";
   t.to = GENESIS_ACCOUNT_BASE_NAME;
   t.amount = asset(amount,SYMBOL_COIN);
   t.validate();

   trx.operations.push_back(t);
   trx.validate();

   BOOST_TEST_MESSAGE( "Verify that not-signing causes an exception" );
   REQUIRE_THROW( db.push_transaction(trx, 0), fc::exception );

   BOOST_TEST_MESSAGE( "Verify that double-signing causes an exception" );
   trx.sign( bob_private_active_key, db.get_chain_id() );
   trx.sign( bob_private_active_key, db.get_chain_id() );
   REQUIRE_THROW( db.push_transaction(trx, 0), tx_duplicate_sig );

   BOOST_TEST_MESSAGE( "Verify that signing with an extra, unused key fails" );
   trx.signatures.pop_back();
   trx.sign( generate_private_key("bogus" ), db.get_chain_id() );
   REQUIRE_THROW( db.push_transaction(trx, 0), tx_irrelevant_sig );

   BOOST_TEST_MESSAGE( "Verify that signing once with the proper key passes" );
   trx.signatures.pop_back();
   db.push_transaction(trx, 0);
   trx.sign( bob_private_active_key, db.get_chain_id() );

} FC_LOG_AND_RETHROW() }


BOOST_FIXTURE_TEST_CASE( pop_block_twice, clean_database_fixture )
{
   try
   {
      ACTORS( (alice)(bob) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      uint32_t skip_flags = (
           database::skip_producer_signature
         | database::skip_transaction_signatures
         | database::skip_authority_check
         );

      generate_block( skip_flags );

      signed_transaction trx;

      transfer_operation transfer;

      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "memo";
      transfer.validate();
      
      trx.operations.push_back( transfer );
      trx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      trx.sign( alice_private_active_key, db.get_chain_id() );
      trx.validate();

      db.push_transaction(trx, ~0 );

      generate_block(skip_flags);

      account_create( account_name_type( "candice" ), alice_private_secure_key, alice_public_owner_key );
      generate_block( skip_flags );

      account_create( account_name_type( "dave" ), alice_private_secure_key, alice_public_owner_key );
      generate_block( skip_flags );

      db.pop_block();
      db.pop_block();
   } 
   catch(const fc::exception& e) 
   {
      edump( (e.to_detail_string()) );
      throw;
   }
}

BOOST_FIXTURE_TEST_CASE( rsf_missed_blocks, clean_database_fixture )
{
   try
   {
      generate_block();

      auto rsf = [&]() -> string
      {
         fc::uint128 rsf = db.get_dynamic_global_properties().recent_slots_filled;
         string result = "";
         result.reserve(128);
         for( int i=0; i<128; i++ )
         {
            result += ((rsf.lo & 1) == 0) ? '0' : '1';
            rsf >>= 1;
         }
         return result;
      };

      auto pct = []( uint32_t x ) -> uint32_t
      {
         return uint64_t( PERCENT_100 ) * x / 128;
      };

      BOOST_TEST_MESSAGE("checking initial participation rate" );
      BOOST_CHECK_EQUAL( rsf(),
         "1111111111111111111111111111111111111111111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), PERCENT_100 );

      BOOST_TEST_MESSAGE("Generating a block skipping 1" );
      generate_block( ~database::skip_fork_db, 1 );
      BOOST_CHECK_EQUAL( rsf(),
         "0111111111111111111111111111111111111111111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(127) );

      BOOST_TEST_MESSAGE("Generating a block skipping 1" );
      generate_block( ~database::skip_fork_db, 1 );
      BOOST_CHECK_EQUAL( rsf(),
         "0101111111111111111111111111111111111111111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(126) );

      BOOST_TEST_MESSAGE("Generating a block skipping 2" );
      generate_block( ~database::skip_fork_db, 2 );
      BOOST_CHECK_EQUAL( rsf(),
         "0010101111111111111111111111111111111111111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(124) );

      BOOST_TEST_MESSAGE("Generating a block for skipping 3" );
      generate_block( ~database::skip_fork_db, 3 );
      BOOST_CHECK_EQUAL( rsf(),
         "0001001010111111111111111111111111111111111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(121) );

      BOOST_TEST_MESSAGE("Generating a block skipping 5" );
      generate_block( ~database::skip_fork_db, 5 );
      BOOST_CHECK_EQUAL( rsf(),
         "0000010001001010111111111111111111111111111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(116) );

      BOOST_TEST_MESSAGE("Generating a block skipping 8" );
      generate_block( ~database::skip_fork_db, 8 );
      BOOST_CHECK_EQUAL( rsf(),
         "0000000010000010001001010111111111111111111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(108) );

      BOOST_TEST_MESSAGE("Generating a block skipping 13" );
      generate_block( ~database::skip_fork_db, 13 );
      BOOST_CHECK_EQUAL( rsf(),
         "0000000000000100000000100000100010010101111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(95) );

      BOOST_TEST_MESSAGE("Generating a block skipping none" );
      generate_block();
      BOOST_CHECK_EQUAL( rsf(),
         "1000000000000010000000010000010001001010111111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(95) );

      BOOST_TEST_MESSAGE("Generating a block" );
      generate_block();
      BOOST_CHECK_EQUAL( rsf(),
         "1100000000000001000000001000001000100101011111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(95) );

      generate_block();
      BOOST_CHECK_EQUAL( rsf(),
         "1110000000000000100000000100000100010010101111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(95) );

      generate_block();
      BOOST_CHECK_EQUAL( rsf(),
         "1111000000000000010000000010000010001001010111111111111111111111"
         "1111111111111111111111111111111111111111111111111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(95) );

      generate_block( ~database::skip_fork_db, 64 );
      BOOST_CHECK_EQUAL( rsf(),
         "0000000000000000000000000000000000000000000000000000000000000000"
         "1111100000000000001000000001000001000100101011111111111111111111"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(31) );

      generate_block( ~database::skip_fork_db, 32 );
      BOOST_CHECK_EQUAL( rsf(),
         "0000000000000000000000000000000010000000000000000000000000000000"
         "0000000000000000000000000000000001111100000000000001000000001000"
      );
      BOOST_CHECK_EQUAL( db.producer_participation_rate(), pct(8) );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_FIXTURE_TEST_CASE( skip_block, clean_database_fixture )
{
   try
   {
      BOOST_TEST_MESSAGE( "Skipping blocks through db" );
      BOOST_REQUIRE( db.head_block_num() == 2 );

      int init_block_num = db.head_block_num();
      int miss_blocks = fc::minutes( 1 ).to_seconds() / BLOCK_INTERVAL.to_seconds();

      account_name_type producer = db.get_scheduled_producer( miss_blocks );
      auto block_time = db.get_slot_time( miss_blocks );
      db.generate_block( block_time , producer, get_private_key( producer, PRODUCER_KEY_STR, INIT_PASSWORD ), 0 );

      BOOST_CHECK_EQUAL( db.head_block_num(), init_block_num + 1 );
      BOOST_CHECK( db.head_block_time() == block_time );

      BOOST_TEST_MESSAGE( "Generating a block through fixture" );
      generate_block();

      BOOST_CHECK_EQUAL( db.head_block_num(), init_block_num + 2 );
      BOOST_CHECK( db.head_block_time() == block_time + BLOCK_INTERVAL );
   }
   FC_LOG_AND_RETHROW();
}

BOOST_FIXTURE_TEST_CASE( generate_block_size, clean_database_fixture )
{
   try
   {
      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_median_chain_properties(), [&]( median_chain_property_object& mcpo )
         {
            mcpo.maximum_block_size = MIN_BLOCK_SIZE_LIMIT;
         });
      });

      generate_block();

      signed_transaction tx;

      fund_liquid( GENESIS_ACCOUNT_BASE_NAME, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      transfer_operation op;

      op.from = GENESIS_ACCOUNT_BASE_NAME;
      op.to = TEMP_ACCOUNT;
      op.amount = asset( 1000, SYMBOL_COIN );
      op.validate();

      // tx minus op is 79 bytes
      // op is 33 bytes (32 for op + 1 byte static variant tag)
      // total is 65254
      // Original generation logic only allowed 115 bytes for the header
      // We are targetting a size (minus header) of 65421 which creates a block of "size" 65535
      // This block will actually be larger because the header estimates is too small

      for( size_t i = 0; i < 1975; i++ )
      {
         tx.operations.push_back( op );
      }

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( init_account_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      // Second transaction, tx minus op is 78 (one less byte for operation vector size)
      // We need a 88 byte op. We need a 22 character memo (1 byte for length) 55 = 32 (old op) + 55 + 1
      op.memo = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123";
      tx.clear();
      tx.operations.push_back( op );
      sign( tx, init_account_private_active_key );
      db.push_transaction( tx, 0 );

      generate_block();
      auto head_block = db.fetch_block_by_number( db.head_block_num() );

      // The last transfer should have been delayed due to size
      BOOST_REQUIRE( head_block->transactions.size() == 1 );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()


*/