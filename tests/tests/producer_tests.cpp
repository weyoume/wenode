//#ifdef IS_TEST_NET

#include <boost/test/unit_test.hpp>
#include <node/chain/node_objects.hpp>
#include <node/protocol/exceptions.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>

//#include <node/chain/hardfork.hpp>

#include <node/chain/util/reward.hpp>
#include <node/witness/witness_objects.hpp>
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



BOOST_AUTO_TEST_CASE( witness_update_operation_tests )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: WITNESS UPDATE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create new witness" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      witness_update_operation witness_update;

      private_key_type signing_key = generate_private_key( "alice_signing_key" );

      witness_update.signatory = "alice";
      witness_update.owner = "alice";
      witness_update.details = "My Details";
      witness_update.url = "www.url.com";
      witness_update.json = "{\"json\":\"valid\"}";
      witness_update.latitude = -37.840935;
      witness_update.longitude = 144.946457;

      chain_properties chain_props;
      chain_props.validate();

      witness_update.props = chain_props;
      witness_update.active = true;
      witness_update.block_signing_key = string( public_key_type( signing_key.get_public_key() ) );
      witness_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( witness_update );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const witness_object& alice_witness = db.get_witness( "alice" );

      alice_witness.props.validate();

      BOOST_REQUIRE( alice_witness.owner == "alice" );
      BOOST_REQUIRE( alice_witness.active == true );
      BOOST_REQUIRE( alice_witness.schedule == witness_object::none );
      BOOST_REQUIRE( alice_witness.last_confirmed_block_num == 0 );
      BOOST_REQUIRE( to_string( alice_witness.details ) == witness_update.details );
      BOOST_REQUIRE( to_string( alice_witness.url ) == witness_update.url );
      BOOST_REQUIRE( to_string( alice_witness.json ) == witness_update.json );
      BOOST_REQUIRE( alice_witness.latitude == witness_update.latitude );
      BOOST_REQUIRE( alice_witness.longitude == witness_update.longitude );
      BOOST_REQUIRE( alice_witness.signing_key == public_key_type( witness_update.block_signing_key ) );
      BOOST_REQUIRE( alice_witness.created == now() );
      BOOST_REQUIRE( alice_witness.last_commit_height == 0);
      BOOST_REQUIRE( alice_witness.last_commit_id == block_id_type() );
      BOOST_REQUIRE( alice_witness.total_blocks == 0 );
      BOOST_REQUIRE( alice_witness.voting_power == 0 );
      BOOST_REQUIRE( alice_witness.vote_count == 0 );
      BOOST_REQUIRE( alice_witness.mining_power == 0 );
      BOOST_REQUIRE( alice_witness.mining_count == 0 );
      BOOST_REQUIRE( alice_witness.last_mining_update == now() );
      BOOST_REQUIRE( alice_witness.last_pow_time == now() );
      BOOST_REQUIRE( alice_witness.recent_txn_stake_weight == 0 );
      BOOST_REQUIRE( alice_witness.last_txn_stake_weight_update == now() );
      BOOST_REQUIRE( alice_witness.accumulated_activity_stake == 0 );
      BOOST_REQUIRE( alice_witness.total_missed == 0 );
      BOOST_REQUIRE( alice_witness.last_aslot == 0 );

      BOOST_REQUIRE( alice_witness.witness_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_witness.witness_virtual_position == 0 );
      BOOST_REQUIRE( alice_witness.witness_virtual_scheduled_time == fc::uint128_t::max_value() );
      BOOST_REQUIRE( alice_witness.miner_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_witness.miner_virtual_position == 0 );
      BOOST_REQUIRE( alice_witness.miner_virtual_scheduled_time == fc::uint128_t::max_value() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create new witness" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update existing Witness and chain properties" );

      witness_update.details = "My New Details";
      witness_update.url = "www.newurl.com";
      witness_update.json = "{\"json\":\"veryvalid\"}";
      witness_update.latitude = -38.840935;
      witness_update.longitude = 145.946457;

      chain_properties chain_props;

      chain_props.maximum_block_size = MAX_TRANSACTION_SIZE * 20000;
      chain_props.credit_min_interest = 2 * PERCENT_1;
      chain_props.credit_variable_interest = 7 * PERCENT_1;
      chain_props.validate();

      witness_update.props = chain_props;
      witness_update.validate();

      tx.operations.push_back( witness_update );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const witness_object& alice_witness = db.get_witness( "alice" );

      alice_witness.props.validate();

      BOOST_REQUIRE( alice_witness.owner == "alice" );
      BOOST_REQUIRE( alice_witness.active == true );
      BOOST_REQUIRE( alice_witness.schedule == witness_object::none );
      BOOST_REQUIRE( alice_witness.last_confirmed_block_num == 0 );
      BOOST_REQUIRE( to_string( alice_witness.details ) == witness_update.details );
      BOOST_REQUIRE( to_string( alice_witness.url ) == witness_update.url );
      BOOST_REQUIRE( to_string( alice_witness.json ) == witness_update.json );
      BOOST_REQUIRE( alice_witness.latitude == witness_update.latitude );
      BOOST_REQUIRE( alice_witness.longitude == witness_update.longitude );
      BOOST_REQUIRE( alice_witness.signing_key == public_key_type( witness_update.block_signing_key ) );
      BOOST_REQUIRE( alice_witness.created == now() );
      BOOST_REQUIRE( alice_witness.last_commit_height == 0);
      BOOST_REQUIRE( alice_witness.last_commit_id == block_id_type() );
      BOOST_REQUIRE( alice_witness.total_blocks == 0 );
      BOOST_REQUIRE( alice_witness.voting_power == 0 );
      BOOST_REQUIRE( alice_witness.vote_count == 0 );
      BOOST_REQUIRE( alice_witness.mining_power == 0 );
      BOOST_REQUIRE( alice_witness.mining_count == 0 );
      BOOST_REQUIRE( alice_witness.last_mining_update == now() );
      BOOST_REQUIRE( alice_witness.last_pow_time == now() );
      BOOST_REQUIRE( alice_witness.recent_txn_stake_weight == 0 );
      BOOST_REQUIRE( alice_witness.last_txn_stake_weight_update == now() );
      BOOST_REQUIRE( alice_witness.accumulated_activity_stake == 0 );
      BOOST_REQUIRE( alice_witness.total_missed == 0 );
      BOOST_REQUIRE( alice_witness.last_aslot == 0 );

      BOOST_REQUIRE( alice_witness.witness_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_witness.witness_virtual_position == 0 );
      BOOST_REQUIRE( alice_witness.witness_virtual_scheduled_time == fc::uint128_t::max_value() );
      BOOST_REQUIRE( alice_witness.miner_virtual_last_update == 0 );
      BOOST_REQUIRE( alice_witness.miner_virtual_position == 0 );
      BOOST_REQUIRE( alice_witness.miner_virtual_scheduled_time == fc::uint128_t::max_value() );

      BOOST_REQUIRE( alice_witness.props.maximum_block_size == witness_update.props.maximum_block_size );
      BOOST_REQUIRE( alice_witness.props.credit_min_interest == witness_update.props.credit_variable_interest );
      BOOST_REQUIRE( alice_witness.props.credit_variable_interest == witness_update.props.credit_variable_interest );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update existing Witness and chain properties" );

      BOOST_TEST_MESSAGE( "├── Testing: WITNESS UPDATE" );
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
      witness_create( "alice", alice_private_owner_key, alice_public_owner_key );

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

      const witness_object& alice_witness = db.get_witness( "alice" );

      alice_witness.props.validate();

      BOOST_REQUIRE( alice_witness.owner == "alice" );
      BOOST_REQUIRE( alice_witness.active == true );
      BOOST_REQUIRE( alice_witness.mining_power > 0 );
      BOOST_REQUIRE( alice_witness.mining_count == 1 );
      BOOST_REQUIRE( alice_witness.last_mining_update == now() );
      
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
      witness_create( "alice", alice_private_owner_key, alice_public_owner_key );

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

      account_witness_vote_operation vote;

      vote.signatory = "alice";
      vote.account = "alice";
      vote.witness = "alice";
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
      create.account_type = PERSONA;
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
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
         witness_create( name, alice_private_active_key, alice_public_active_key );

         vote.signatory = name;
         vote.account = name;
         vote.witness = name;

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



   //===========================//
   // === Custom Operations === //
   //===========================//



BOOST_AUTO_TEST_CASE( custom_authorities )
{
   custom_operation op;
   op.required_auths.insert( "alice" );
   op.required_auths.insert( "bob" );

   flat_set< account_name_type > auths;
   flat_set< account_name_type > expected;

   op.get_required_owner_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   op.get_required_posting_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   expected.insert( "alice" );
   expected.insert( "bob" );
   op.get_required_active_authorities( auths );
   BOOST_REQUIRE( auths == expected );
}

BOOST_AUTO_TEST_CASE( custom_json_authorities )
{
   custom_json_operation op;
   op.required_auths.insert( "alice" );
   op.required_posting_auths.insert( "bob" );

   flat_set< account_name_type > auths;
   flat_set< account_name_type > expected;

   op.get_required_owner_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   expected.insert( "alice" );
   op.get_required_active_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   auths.clear();
   expected.clear();
   expected.insert( "bob" );
   op.get_required_posting_authorities( auths );
   BOOST_REQUIRE( auths == expected );
}

/**
 * 
 * BOOST_AUTO_TEST_CASE( feed_publish_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: feed_publish_validate" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( feed_publish_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: feed_publish_authorities" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );
      witness_create( "alice", alice_private_owner_key, "foo.bar", alice_private_owner_key.get_public_key(), 1000 );

      feed_publish_operation op;
      op.publisher = "alice";
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test success with witness account signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( feed_publish_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: feed_publish_apply" );

      ACTORS( (alice) )
      fund( "alice", 10000 );
      witness_create( "alice", alice_private_owner_key, "foo.bar", alice_private_owner_key.get_public_key(), 1000 );

      BOOST_TEST_MESSAGE( "--- Test publishing price feed" );
      feed_publish_operation op;
      op.publisher = "alice";
      op.exchange_rate = price( ASSET( "1000.000 TESTS" ), ASSET( "1.000 USD" ) ); // 1000 TME : 1 USD

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      witness_object& alice_witness = const_cast< witness_object& >( db.get_witness( "alice" ) );

      BOOST_REQUIRE( alice_witness.USD_exchange_rate == op.exchange_rate );
      BOOST_REQUIRE( alice_witness.last_USD_exchange_update == db.head_block_time() );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure publishing to non-existent witness" );

      tx.operations.clear();
      tx.signatures.clear();
      op.publisher = "bob";
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test updating price feed" );

      tx.operations.clear();
      tx.signatures.clear();
      op.exchange_rate = price( ASSET(" 1500.000 TESTS" ), ASSET( "1.000 USD" ) );
      op.publisher = "alice";
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      alice_witness = const_cast< witness_object& >( db.get_witness( "alice" ) );
      BOOST_REQUIRE( std::abs( alice_witness.USD_exchange_rate.to_real() - op.exchange_rate.to_real() ) < 0.0000005 );
      BOOST_REQUIRE( alice_witness.last_USD_exchange_update == db.head_block_time() );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

*/

BOOST_AUTO_TEST_CASE( account_bandwidth )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: account_bandwidth" );
      ACTORS( (alice)(bob) )
      generate_block();
      fund_stake( "alice", ASSET( "10.000 TESTS" ) );
      fund( "alice", ASSET( "10.000 TESTS" ) );
      fund_stake( "bob", ASSET( "10.000 TESTS" ) );

      generate_block();
      db.skip_transaction_delta_check = false;

      BOOST_TEST_MESSAGE( "--- Test first tx in block" );

      signed_transaction tx;
      transfer_operation op;

      op.from = "alice";
      op.to = "bob";
      op.amount = ASSET( "1.000 TESTS" );

      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      auto last_bandwidth_update = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).last_bandwidth_update;
      auto average_bandwidth = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).average_bandwidth;
      BOOST_REQUIRE( last_bandwidth_update == db.head_block_time() );
      BOOST_REQUIRE( average_bandwidth == fc::raw::pack_size( tx ) * 10 * BANDWIDTH_PRECISION );
      auto total_bandwidth = average_bandwidth;

      BOOST_TEST_MESSAGE( "--- Test second tx in block" );

      op.amount = ASSET( "0.100 TESTS" );
      tx.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      last_bandwidth_update = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).last_bandwidth_update;
      average_bandwidth = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).average_bandwidth;
      BOOST_REQUIRE( last_bandwidth_update == db.head_block_time() );
      BOOST_REQUIRE( average_bandwidth == total_bandwidth + fc::raw::pack_size( tx ) * 10 * BANDWIDTH_PRECISION );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()