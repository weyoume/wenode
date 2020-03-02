//#ifdef IS_TEST_NET

#include <boost/test/unit_test.hpp>
#include <node/chain/node_objects.hpp>
#include <node/protocol/exceptions.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/util/reward.hpp>
#include <fc/crypto/digest.hpp>
#include <tests/common/database_fixture.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace node;
using namespace node::chain;
using namespace node::protocol;
using std::string;

BOOST_FIXTURE_TEST_SUITE( producer_operation_tests, clean_database_fixture );



   //================================//
   // === Block Production Tests === //
   //================================//



BOOST_AUTO_TEST_CASE( producer_update_operation_tests )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: PRODUCER UPDATE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create new Producer" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      producer_update_operation producer_update;

      private_key_type signing_key = generate_private_key( "alice_signing_key" );

      producer_update.signatory = "alice";
      producer_update.owner = "alice";
      producer_update.details = "My Details";
      producer_update.url = "www.url.com";
      producer_update.json = "{\"json\":\"valid\"}";
      producer_update.latitude = -37.840935;
      producer_update.longitude = 144.946457;

      chain_properties chain_props;
      chain_props.validate();

      producer_update.props = chain_props;
      producer_update.active = true;
      producer_update.block_signing_key = string( public_key_type( signing_key.get_public_key() ) );
      producer_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( producer_update );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_object& alice_producer = db.get_producer( "alice" );

      alice_producer.props.validate();

      BOOST_REQUIRE( alice_producer.owner == "alice" );
      BOOST_REQUIRE( alice_producer.active == true );
      BOOST_REQUIRE( alice_producer.schedule == producer_object::none );
      BOOST_REQUIRE( alice_producer.last_confirmed_block_num == 0 );
      BOOST_REQUIRE( to_string( alice_producer.details ) == producer_update.details );
      BOOST_REQUIRE( to_string( alice_producer.url ) == producer_update.url );
      BOOST_REQUIRE( to_string( alice_producer.json ) == producer_update.json );
      BOOST_REQUIRE( alice_producer.latitude == producer_update.latitude );
      BOOST_REQUIRE( alice_producer.longitude == producer_update.longitude );
      BOOST_REQUIRE( alice_producer.signing_key == public_key_type( producer_update.block_signing_key ) );
      BOOST_REQUIRE( alice_producer.created == now() );
      BOOST_REQUIRE( alice_producer.last_commit_height == 0);
      BOOST_REQUIRE( alice_producer.last_commit_id == block_id_type() );
      BOOST_REQUIRE( alice_producer.total_blocks == 0 );
      BOOST_REQUIRE( alice_producer.voting_power == 0 );
      BOOST_REQUIRE( alice_producer.vote_count == 0 );
      BOOST_REQUIRE( alice_producer.mining_power == 0 );
      BOOST_REQUIRE( alice_producer.mining_count == 0 );
      BOOST_REQUIRE( alice_producer.last_mining_update == now() );
      BOOST_REQUIRE( alice_producer.last_pow_time == now() );
      BOOST_REQUIRE( alice_producer.recent_txn_stake_weight == 0 );
      BOOST_REQUIRE( alice_producer.last_txn_stake_weight_update == now() );
      BOOST_REQUIRE( alice_producer.accumulated_activity_stake == 0 );
      BOOST_REQUIRE( alice_producer.total_missed == 0 );
      BOOST_REQUIRE( alice_producer.last_aslot == 0 );

      BOOST_REQUIRE( alice_producer.voting_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_producer.voting_virtual_position == 0 );
      BOOST_REQUIRE( alice_producer.voting_virtual_scheduled_time == fc::uint128_t::max_value() );
      BOOST_REQUIRE( alice_producer.mining_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_producer.mining_virtual_position == 0 );
      BOOST_REQUIRE( alice_producer.mining_virtual_scheduled_time == fc::uint128_t::max_value() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create new Producer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update existing Producer and chain properties" );

      producer_update.details = "My New Details";
      producer_update.url = "www.newurl.com";
      producer_update.json = "{\"json\":\"veryvalid\"}";
      producer_update.latitude = -38.840935;
      producer_update.longitude = 145.946457;

      chain_properties chain_props;

      chain_props.maximum_block_size = MAX_TRANSACTION_SIZE * 20000;
      chain_props.credit_min_interest = 2 * PERCENT_1;
      chain_props.credit_variable_interest = 7 * PERCENT_1;
      chain_props.validate();

      producer_update.props = chain_props;
      producer_update.validate();

      tx.operations.push_back( producer_update );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_object& alice_producer = db.get_producer( "alice" );

      alice_producer.props.validate();

      BOOST_REQUIRE( alice_producer.owner == "alice" );
      BOOST_REQUIRE( alice_producer.active == true );
      BOOST_REQUIRE( alice_producer.schedule == producer_object::none );
      BOOST_REQUIRE( alice_producer.last_confirmed_block_num == 0 );
      BOOST_REQUIRE( to_string( alice_producer.details ) == producer_update.details );
      BOOST_REQUIRE( to_string( alice_producer.url ) == producer_update.url );
      BOOST_REQUIRE( to_string( alice_producer.json ) == producer_update.json );
      BOOST_REQUIRE( alice_producer.latitude == producer_update.latitude );
      BOOST_REQUIRE( alice_producer.longitude == producer_update.longitude );
      BOOST_REQUIRE( alice_producer.signing_key == public_key_type( producer_update.block_signing_key ) );
      BOOST_REQUIRE( alice_producer.created == now() );
      BOOST_REQUIRE( alice_producer.last_commit_height == 0);
      BOOST_REQUIRE( alice_producer.last_commit_id == block_id_type() );
      BOOST_REQUIRE( alice_producer.total_blocks == 0 );
      BOOST_REQUIRE( alice_producer.voting_power == 0 );
      BOOST_REQUIRE( alice_producer.vote_count == 0 );
      BOOST_REQUIRE( alice_producer.mining_power == 0 );
      BOOST_REQUIRE( alice_producer.mining_count == 0 );
      BOOST_REQUIRE( alice_producer.last_mining_update == now() );
      BOOST_REQUIRE( alice_producer.last_pow_time == now() );
      BOOST_REQUIRE( alice_producer.recent_txn_stake_weight == 0 );
      BOOST_REQUIRE( alice_producer.last_txn_stake_weight_update == now() );
      BOOST_REQUIRE( alice_producer.accumulated_activity_stake == 0 );
      BOOST_REQUIRE( alice_producer.total_missed == 0 );
      BOOST_REQUIRE( alice_producer.last_aslot == 0 );

      BOOST_REQUIRE( alice_producer.voting_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_producer.voting_virtual_position == 0 );
      BOOST_REQUIRE( alice_producer.voting_virtual_scheduled_time == fc::uint128_t::max_value() );
      BOOST_REQUIRE( alice_producer.mining_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_producer.mining_virtual_position == 0 );
      BOOST_REQUIRE( alice_producer.mining_virtual_scheduled_time == fc::uint128_t::max_value() );

      BOOST_REQUIRE( alice_producer.props.maximum_block_size == producer_update.props.maximum_block_size );
      BOOST_REQUIRE( alice_producer.props.credit_min_interest == producer_update.props.credit_variable_interest );
      BOOST_REQUIRE( alice_producer.props.credit_variable_interest == producer_update.props.credit_variable_interest );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update existing Producer and chain properties" );

      BOOST_TEST_MESSAGE( "├── Testing: PRODUCER UPDATE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( proof_of_work_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: PROOF OF WORK" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create proof of work" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      generate_block();
      
      signed_transaction tx;

      uint128_t target_pow = db.pow_difficulty();
      block_id_type head_block_id = db.head_block_id();

      proof_of_work work;

      work.create( head_block_id, "alice", 0 );
      
      for( auto n = 1; work.pow_summary >= target_pow; n++ )
      {
         work.create( head_block_id, "alice", n );
      }

      proof_of_work_operation proof;

      proof.work = work;

      chain_properties chain_props;
      chain_props.validate();

      proof.props = chain_props;
      proof.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( proof );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_object& alice_producer = db.get_producer( "alice" );

      alice_producer.props.validate();

      BOOST_REQUIRE( alice_producer.owner == "alice" );
      BOOST_REQUIRE( alice_producer.active == true );
      BOOST_REQUIRE( alice_producer.mining_power > 0 );
      BOOST_REQUIRE( alice_producer.mining_count == 1 );
      BOOST_REQUIRE( alice_producer.last_mining_update == now() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create proof of work" );

      BOOST_TEST_MESSAGE( "├── Testing: PROOF OF WORK" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( verify_block_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: VERIFY BLOCK OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Verify block" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      generate_block();
      
      signed_transaction tx;

      account_producer_vote_operation vote;

      vote.signatory = "alice";
      vote.account = "alice";
      vote.producer = "alice";
      vote.vote_rank = 1;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "bob";
      vote.account = "bob";

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "candice";
      vote.account = "candice";

      tx.operations.push_back( vote );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "dan";
      vote.account = "dan";

      tx.operations.push_back( vote );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "elon";
      vote.account = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      verify_block_operation verify;

      verify.signatory = "alice";
      verify.producer = "alice";
      verify.block_id = db.head_block_id();
      verify.block_height = db.head_block_num();
      verify.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      tx.operations.push_back( verify );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const block_validation_object& validation = db.get_block_validation( "alice", verify.block_height );

      BOOST_REQUIRE( validation.producer == verify.producer );
      BOOST_REQUIRE( validation.block_id == verify.block_id );
      BOOST_REQUIRE( validation.block_height == verify.block_height );
      BOOST_REQUIRE( validation.committed == false );
      BOOST_REQUIRE( validation.created == now() );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.new_account_name = "newuser";
      create.owner_auth = authority( 1, alice_public_owner_key, 1 );
      create.active_auth = authority( 2, alice_public_active_key, 2 );
      create.posting_auth = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      flat_set< transaction_id_type > verifications;

      for( auto i = 0; i < 100; i++ )
      {
         string name = "newuser"+fc::to_string( i );
         create.new_account_name = name;

         tx.operations.push_back( create );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         fund( name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         fund_stake( name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         producer_create( name, alice_private_active_key, alice_public_active_key );

         vote.signatory = name;
         vote.account = name;
         vote.producer = name;

         tx.operations.push_back( vote );
         tx.sign( alice_private_active_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         verify.signatory = name;
         verify.producer = name;

         tx.sign( alice_private_active_key, db.get_chain_id() );
         tx.operations.push_back( verify );
         db.push_transaction( tx, 0 );

         verifications.insert( tx.id() );

         tx.operations.clear();
         tx.signatures.clear();

         if( i % 20 == 0 )
         {
            generate_block();
         }
      }
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Verify block" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Commit block" );

      generate_block();
      
      commit_block_operation commit;

      commit.signatory = "alice";
      commit.producer = "alice";
      commit.block_id = verify.block_id;
      commit.block_height = verify.block_height;
      commit.verifications = verifications;
      commit.commitment_stake = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      commit.validate();

      tx.operations.push_back( commit );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const block_validation_object& validation = db.get_block_validation( "alice", commit.block_height );

      BOOST_REQUIRE( validation.producer == commit.producer );
      BOOST_REQUIRE( validation.block_id == commit.block_id );
      BOOST_REQUIRE( validation.block_height == commit.block_height );
      BOOST_REQUIRE( validation.commitment_stake == commit.commitment_stake );
      BOOST_REQUIRE( validation.committed == true );
      BOOST_REQUIRE( validation.commit_time == now() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Commit block" );

      BOOST_TEST_MESSAGE( "├── Testing: VERIFY BLOCK OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()