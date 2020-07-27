#include <boost/test/unit_test.hpp>
#include <node/protocol/exceptions.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/util/reward.hpp>
#include <fc/crypto/digest.hpp>
#include "../common/database_fixture.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace node;
using namespace node::chain;
using namespace node::protocol;
using std::string;

BOOST_FIXTURE_TEST_SUITE( transfer_operation_tests, clean_database_fixture );



   //========================//
   // === Transfer Tests === //
   //========================//



BOOST_AUTO_TEST_CASE( transfer_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(corp) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "corp", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "corp", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      signed_transaction tx;

      transfer_operation transfer;

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signatures" );

      tx.operations.push_back( transfer );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by a signature not in the account's active authority" );

      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by a signature not in the account's active authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by an additional signature not in the creator's authority" );
      
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by an additional signature not in the creator's authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: multi-sig transfer" );

      // Alice, Bob and candice all have 2-of-3 multisig on corp.
      // Legitimate tx signed by (Alice, Bob) goes through.
      // Sam shouldn't be able to add or remove signatures to get the transaction to process multiple times.

      account_update_operation update_op;

      update_op.signatory = "corp";
      update_op.account = "corp";
      update_op.active_auth = authority( 2, "alice", 1, "bob", 1, "candice", 1 );
      update_op.validate();

      tx.operations.push_back( update_op );
      tx.sign( corp_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "corp";
      transfer.from = "corp";
      transfer.to = "candice";

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: multi-sig transfer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: larger transfer amount send" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance + transfer.amount );

      BOOST_TEST_MESSAGE( "│   ├── Passed: larger transfer amount send" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when sending greater than liquid balance" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = asset( 20000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when sending greater than liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sending exactly entire liquid balance" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = alice_init_liquid_balance;
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( alice_liquid_balance.amount == 0 );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance + transfer.amount );

      BOOST_TEST_MESSAGE( "│   ├── Passed: sending exactly entire liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure transferring with zero liquid balance" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = asset( 1 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure transferring with zero liquid balance" );

      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( transfer_request_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER REQUEST OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer request" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      transfer_request_operation request;

      request.signatory = "alice";
      request.to = "alice";
      request.from = "bob";
      request.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      request.memo = "Hello";
      request.request_id = "4d7248b3-c89d-4bce-8e5f-547c75b5628e";
      request.expiration = now() + fc::days(7);
      request.requested = true;
      request.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_request_object& req_obj = db.get_transfer_request( request.to, request.request_id );

      BOOST_REQUIRE( req_obj.amount == request.amount );
      BOOST_REQUIRE( req_obj.to == request.to );
      BOOST_REQUIRE( req_obj.from == request.from );
      BOOST_REQUIRE( to_string( req_obj.memo ) == request.memo );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept transfer request" );

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      transfer_accept_operation accept;

      accept.signatory = "bob";
      accept.from = "bob";
      accept.to = "alice";
      accept.request_id = "4d7248b3-c89d-4bce-8e5f-547c75b5628e";
      accept.accepted = true;
      accept.validate();
      
      tx.operations.push_back( accept );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance + request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance - request.amount );
      BOOST_REQUIRE( db.get_transfer_request( request.to, request.request_id ).paid );

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept transfer request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting greater than liquid balance" );

      request.amount = asset( 20000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      request.validate();
      
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting greater than liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: requesting exactly entire liquid balance" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      request.amount = bob_init_liquid_balance;
      request.request_id = "ba8a97bb-f36b-4f80-a70d-610edc8a1f0d";
      request.expiration = now() + fc::days(7);
      request.requested = true;
      request.validate();
      
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.request_id = "ba8a97bb-f36b-4f80-a70d-610edc8a1f0d";

      tx.operations.push_back( accept );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance + request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance.amount == 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: requesting exactly entire liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure requesting transfer with zero liquid balance" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );      // Bob has no funds remaining

      request.amount = asset( 1 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure requesting transfer with zero liquid balance" );

      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER REQUEST OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( transfer_recurring_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER RECURRING OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful recurring transfer creation and completion" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      signed_transaction tx;

      transfer_recurring_operation transfer;

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.transfer_id = "5072f90d-bbd9-4688-99c3-74721f58009c";
      transfer.memo = "Hello";
      transfer.begin = now() + fc::minutes(10);
      transfer.payments = 10;
      transfer.interval = fc::days(1);
      transfer.active = true;
      transfer.extensible = false;
      transfer.fill_or_kill = false;
      transfer.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj = db.get_transfer_recurring( transfer.from, transfer.transfer_id );

      BOOST_REQUIRE( transfer_obj.from == transfer.from );
      BOOST_REQUIRE( transfer_obj.to == transfer.to );
      BOOST_REQUIRE( transfer_obj.amount == transfer.amount );
      BOOST_REQUIRE( to_string( transfer_obj.memo ) == transfer.memo );
      BOOST_REQUIRE( transfer_obj.begin == transfer.begin );
      BOOST_REQUIRE( transfer_obj.payments == transfer.payments );
      BOOST_REQUIRE( transfer_obj.payments_remaining == transfer.payments );
      BOOST_REQUIRE( transfer_obj.interval == transfer.interval );
      BOOST_REQUIRE( transfer_obj.next_transfer == transfer.begin );

      generate_blocks( transfer_obj.begin, true );
      generate_block();

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 9 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 2 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 2 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 8 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 3 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 3 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 7 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 4 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 4 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 6 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 5 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 5 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 5 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 6 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 6 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 4 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 7 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 7 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 3 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 8 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 8 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 2 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 9 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 9 * transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 1 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 10 * transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 10 * transfer.amount );

      const auto& transfer_idx = db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
      auto transfer_itr = transfer_idx.find( std::make_tuple( transfer.from, transfer.transfer_id ) );

      BOOST_REQUIRE( transfer_itr == transfer_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful recurring transfer creation and completion" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful fill or kill cancellation" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      transfer.transfer_id = "d3a8fb9e-180f-4de3-8465-ac173f4dbf3a";
      transfer.amount = alice_init_liquid_balance;
      transfer.begin = now() + fc::minutes(10);
      transfer.fill_or_kill = true;    // Fill or kill will cancel recurring payment when one is missed
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj2 = db.get_transfer_recurring( transfer.from, transfer.transfer_id );

      BOOST_REQUIRE( transfer_obj2.from == transfer.from );
      BOOST_REQUIRE( transfer_obj2.to == transfer.to );
      BOOST_REQUIRE( transfer_obj2.amount == transfer.amount );
      BOOST_REQUIRE( to_string( transfer_obj2.memo ) == transfer.memo );
      BOOST_REQUIRE( transfer_obj2.begin == transfer.begin );
      BOOST_REQUIRE( transfer_obj2.payments == transfer.payments );
      BOOST_REQUIRE( transfer_obj2.payments_remaining == transfer.payments );
      BOOST_REQUIRE( transfer_obj2.interval == transfer.interval );
      BOOST_REQUIRE( transfer_obj2.next_transfer == transfer.begin );
      BOOST_REQUIRE( transfer_obj2.fill_or_kill == true );

      generate_blocks( transfer_obj2.begin, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 9 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );

      // Fill or kill will cause cancellation

      transfer_itr = transfer_idx.find( std::make_tuple( transfer.from, transfer.transfer_id ) );

      BOOST_REQUIRE( transfer_itr == transfer_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful fill or kill cancellation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when sending greater than liquid balance" );

      transfer.signatory = "candice";
      transfer.from = "candice";
      transfer.to = "bob";
      transfer.transfer_id = "c5a13b05-6b5b-4870-9184-41d96fd99e85";
      transfer.amount = asset( 200000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.begin = now() + fc::minutes(10);
      transfer.fill_or_kill = false;
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( transfer );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when sending greater than liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sending exactly entire liquid balance non-extensible" );

      asset candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      transfer.amount = candice_init_liquid_balance;
      transfer.begin = now() + fc::minutes(10);
      
      tx.operations.push_back( transfer );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj3 = db.get_transfer_recurring( transfer.from, transfer.transfer_id );

      BOOST_REQUIRE( transfer_obj3.from == transfer.from );
      BOOST_REQUIRE( transfer_obj3.to == transfer.to );
      BOOST_REQUIRE( transfer_obj3.amount == transfer.amount );
      BOOST_REQUIRE( to_string( transfer_obj3.memo ) == transfer.memo );
      BOOST_REQUIRE( transfer_obj3.begin == transfer.begin );
      BOOST_REQUIRE( transfer_obj3.payments == transfer.payments );
      BOOST_REQUIRE( transfer_obj3.payments_remaining == transfer.payments );
      BOOST_REQUIRE( transfer_obj3.interval == transfer.interval );
      BOOST_REQUIRE( transfer_obj3.next_transfer == transfer.begin );

      generate_blocks( transfer_obj3.begin, true );
      generate_block();

      asset candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 9 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 8 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: sending exactly entire liquid balance non-extensible" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure transferring with zero liquid balance" );

      candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      transfer.amount = asset( 1 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.transfer_id = "9d5e1655-ba79-433f-92bd-5615bec0cf9d";
      transfer.begin = now() + fc::minutes(10);
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( transfer );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure transferring with zero liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful recurring transfer extension" );

      asset dan_init_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      transfer.signatory = "dan";
      transfer.from = "dan";
      transfer.to = "bob";
      transfer.transfer_id = "7927b7a7-e2b1-42c5-9a15-cfc232c8d56e";
      transfer.amount = dan_init_liquid_balance;
      transfer.extensible = true;
      transfer.fill_or_kill = false;
      transfer.begin = now() + fc::minutes(10);
      
      tx.operations.push_back( transfer );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj4 = db.get_transfer_recurring( transfer.from, transfer.transfer_id );

      BOOST_REQUIRE( transfer_obj4.from == transfer.from );
      BOOST_REQUIRE( transfer_obj4.to == transfer.to );
      BOOST_REQUIRE( transfer_obj4.amount == transfer.amount );
      BOOST_REQUIRE( to_string( transfer_obj4.memo ) == transfer.memo );
      BOOST_REQUIRE( transfer_obj4.begin == transfer.begin );
      BOOST_REQUIRE( transfer_obj4.payments == transfer.payments );
      BOOST_REQUIRE( transfer_obj4.payments_remaining == transfer.payments );
      BOOST_REQUIRE( transfer_obj4.interval == transfer.interval );
      BOOST_REQUIRE( transfer_obj4.next_transfer == transfer.begin );
      BOOST_REQUIRE( transfer_obj4.extensible == true );

      time_point prev_end = transfer_obj4.end;

      generate_blocks( transfer_obj4.begin, true );
      generate_block();

      asset dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 9 );

      generate_blocks( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).next_transfer, true );
      generate_block();

      dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).payments_remaining == 9 );
      BOOST_REQUIRE( db.get_transfer_recurring( transfer.from, transfer.transfer_id ).end == prev_end + db.get_transfer_recurring( transfer.from, transfer.transfer_id ).interval );   // End time extended

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful recurring transfer extension" );

      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER RECURRING OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( transfer_recurring_request_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER RECURRING REQUEST OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful recurring transfer request" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      signed_transaction tx;

      transfer_recurring_request_operation request;

      request.signatory = "bob";
      request.to = "bob";
      request.from = "alice";
      request.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      request.request_id = "db631874-fc87-4cff-a0da-3a87b069b4c6";
      request.memo = "Hello";
      request.begin = now() + fc::days(1);
      request.payments = 2;
      request.interval = fc::days(1);
      request.expiration = now() + fc::days(1);
      request.requested = true;
      request.extensible = false;
      request.fill_or_kill = false;
      request.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_request_object& request_obj = db.get_transfer_recurring_request( request.to, request.request_id );

      BOOST_REQUIRE( request_obj.from == request.from );
      BOOST_REQUIRE( request_obj.to == request.to );
      BOOST_REQUIRE( request_obj.amount == request.amount );
      BOOST_REQUIRE( to_string( request_obj.memo ) == request.memo );
      BOOST_REQUIRE( request_obj.begin == request.begin );
      BOOST_REQUIRE( request_obj.payments == request.payments );
      BOOST_REQUIRE( request_obj.interval == request.interval );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful recurring transfer request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept recurring transfer request" );

      transfer_recurring_accept_operation accept;

      accept.signatory = "alice";
      accept.from = "alice";
      accept.to = "bob";
      accept.request_id = "db631874-fc87-4cff-a0da-3a87b069b4c6";
      accept.accepted = true;
      accept.validate();
      
      tx.operations.push_back( accept );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj = db.get_transfer_recurring( request.from, request.request_id );

      BOOST_REQUIRE( transfer_obj.from == request.from );
      BOOST_REQUIRE( transfer_obj.to == request.to );
      BOOST_REQUIRE( transfer_obj.amount == request.amount );
      BOOST_REQUIRE( to_string( transfer_obj.memo ) == request.memo );
      BOOST_REQUIRE( transfer_obj.begin == request.begin );
      BOOST_REQUIRE( transfer_obj.payments == request.payments );
      BOOST_REQUIRE( transfer_obj.payments_remaining == request.payments );
      BOOST_REQUIRE( transfer_obj.interval == request.interval );
      BOOST_REQUIRE( transfer_obj.next_transfer == request.begin );
      BOOST_REQUIRE( transfer_obj.extensible == false );
      BOOST_REQUIRE( transfer_obj.fill_or_kill == false );
      
      generate_blocks( transfer_obj.begin, true );
      generate_block();

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + request.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( request.from, request.request_id ).payments_remaining == 1 );

      generate_blocks( db.get_transfer_recurring( request.from, request.request_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - 2 * request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + 2 * request.amount );
      
      const auto& transfer_idx = db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
      auto transfer_itr = transfer_idx.find( std::make_tuple( request.from, request.request_id ) );

      BOOST_REQUIRE( transfer_itr == transfer_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful recurring transfer creation and completion" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful fill or kill cancellation" );

      alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      request.request_id = "d36c3c32-c033-4960-82f8-9fcb585c912c";
      request.begin = now() + fc::days(1);
      request.amount = alice_init_liquid_balance;
      request.fill_or_kill = true;
      request.extensible = false;
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_request_object& request_obj2 = db.get_transfer_recurring_request( request.to, request.request_id );

      BOOST_REQUIRE( request_obj2.from == request.from );
      BOOST_REQUIRE( request_obj2.to == request.to );
      BOOST_REQUIRE( request_obj2.amount == request.amount );
      BOOST_REQUIRE( to_string( request_obj2.memo ) == request.memo );
      BOOST_REQUIRE( request_obj2.begin == request.begin );
      BOOST_REQUIRE( request_obj2.payments == request.payments );
      BOOST_REQUIRE( request_obj2.interval == request.interval );
      
      accept.request_id = "d36c3c32-c033-4960-82f8-9fcb585c912c";
      accept.accepted = true;
      
      tx.operations.push_back( accept );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj2 = db.get_transfer_recurring( request.from, request.request_id );

      BOOST_REQUIRE( transfer_obj2.from == request.from );
      BOOST_REQUIRE( transfer_obj2.to == request.to );
      BOOST_REQUIRE( transfer_obj2.amount == request.amount );
      BOOST_REQUIRE( to_string( transfer_obj2.memo ) == request.memo );
      BOOST_REQUIRE( transfer_obj2.begin == request.begin );
      BOOST_REQUIRE( transfer_obj2.payments == request.payments );
      BOOST_REQUIRE( transfer_obj2.payments_remaining == request.payments );
      BOOST_REQUIRE( transfer_obj2.interval == request.interval );
      BOOST_REQUIRE( transfer_obj2.next_transfer == request.begin );
      BOOST_REQUIRE( transfer_obj2.fill_or_kill == true );

      generate_blocks( transfer_obj2.begin, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + request.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( request.from, request.request_id ).payments_remaining == 1 );

      generate_blocks( db.get_transfer_recurring( request.from, request.request_id ).next_transfer, true );
      generate_block();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + request.amount );

      transfer_itr = transfer_idx.find( std::make_tuple( request.from, request.request_id ) );

      BOOST_REQUIRE( transfer_itr == transfer_idx.end() );    // Fill or kill will cause cancellation

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful fill or kill cancellation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting greater than liquid balance" );

      request.signatory = "bob";
      request.to = "bob";
      request.from = "candice";
      request.request_id = "c6d1839b-d504-42c5-b9de-92092abbe836";
      request.begin = now() + fc::days(1);
      request.amount = asset( 200000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      request.payments = 10;
      request.fill_or_kill = false;
      request.extensible = false;
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting greater than liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sending exactly entire liquid balance non-extensible" );

      asset candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      request.amount = candice_init_liquid_balance;
      
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "candice";
      accept.from = "candice";
      accept.to = "bob";
      accept.request_id = "c6d1839b-d504-42c5-b9de-92092abbe836";
      accept.accepted = true;
      
      tx.operations.push_back( accept );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj3 = db.get_transfer_recurring( request.from, request.request_id );

      BOOST_REQUIRE( transfer_obj3.from == request.from );
      BOOST_REQUIRE( transfer_obj3.to == request.to );
      BOOST_REQUIRE( transfer_obj3.amount == request.amount );
      BOOST_REQUIRE( to_string( transfer_obj3.memo ) == request.memo );
      BOOST_REQUIRE( transfer_obj3.begin == request.begin );
      BOOST_REQUIRE( transfer_obj3.payments == request.payments );
      BOOST_REQUIRE( transfer_obj3.payments_remaining == request.payments );
      BOOST_REQUIRE( transfer_obj3.interval == request.interval );
      BOOST_REQUIRE( transfer_obj3.next_transfer == request.begin );

      generate_blocks( transfer_obj3.begin, true );
      generate_block();

      asset candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + request.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( request.from, request.request_id ).payments_remaining == 9 );

      generate_blocks( db.get_transfer_recurring( request.from, request.request_id ).next_transfer, true );
      generate_block();

      candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      // Account does not have enough to make second payment, but transfer is not extensible or fill or kill

      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + request.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( request.from, request.request_id ).payments_remaining == 8 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: sending exactly entire liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure transferring with zero liquid balance" );

      candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      request.amount = asset( 1 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      request.request_id = "639d3cdd-7620-4cce-a1b2-6ae359d47a17";
      request.begin = now() + fc::days(1);
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure transferring with zero liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful recurring transfer extension" );

      asset dan_init_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      request.signatory = "bob";
      request.to = "bob";
      request.from = "dan";
      request.request_id = "639d3cdd-7620-4cce-a1b2-6ae359d47a17";
      request.amount = dan_init_liquid_balance;
      request.begin = now() + fc::days(1);
      request.extensible = true;
      request.fill_or_kill = false;
      
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "dan";
      accept.from = "dan";
      accept.to = "bob";
      accept.request_id = "639d3cdd-7620-4cce-a1b2-6ae359d47a17";
      accept.accepted = true;
      
      tx.operations.push_back( accept );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const transfer_recurring_object& transfer_obj4 = db.get_transfer_recurring( request.from, request.request_id );

      BOOST_REQUIRE( transfer_obj4.from == request.from );
      BOOST_REQUIRE( transfer_obj4.to == request.to );
      BOOST_REQUIRE( transfer_obj4.amount == request.amount );
      BOOST_REQUIRE( to_string( transfer_obj4.memo ) == request.memo );
      BOOST_REQUIRE( transfer_obj4.begin == request.begin );
      BOOST_REQUIRE( transfer_obj4.payments == request.payments );
      BOOST_REQUIRE( transfer_obj4.payments_remaining == request.payments );
      BOOST_REQUIRE( transfer_obj4.interval == request.interval );
      BOOST_REQUIRE( transfer_obj4.next_transfer == request.begin );
      BOOST_REQUIRE( transfer_obj4.extensible == true );

      time_point prev_end = transfer_obj4.end;

      generate_blocks( transfer_obj4.begin, true );
      generate_block();

      asset dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + request.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( request.from, request.request_id ).payments_remaining == 9 );

      generate_blocks( db.get_transfer_recurring( request.from, request.request_id ).next_transfer, true );
      generate_block();

      dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance - request.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + request.amount );
      BOOST_REQUIRE( db.get_transfer_recurring( request.from, request.request_id ).payments_remaining == 9 );
      BOOST_REQUIRE( db.get_transfer_recurring( request.from, request.request_id ).end == prev_end + db.get_transfer_recurring( request.from, request.request_id ).interval );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful recurring transfer extension" );

      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER RECURRING REQUEST OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()