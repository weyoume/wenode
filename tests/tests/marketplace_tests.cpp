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

BOOST_FIXTURE_TEST_SUITE( marketplace_operation_tests, clean_database_fixture );


   //===========================//
   // === Marketplace Tests === //
   //===========================//



BOOST_AUTO_TEST_CASE( marketplace_operation_sequence_tests )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ESCROW TRANSFER OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: creation of escrow transfer proposal" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      signed_transaction tx;

      escrow_transfer_operation transfer;

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17";
      transfer.json = "{\"json\":\"valid\"}";
      transfer.memo = "Hello";
      transfer.acceptance_time = now() + fc::days(1);
      transfer.escrow_expiration = now() + fc::days(8);
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const escrow_object& escrow1 = db.get_escrow( "alice", string( "6b3b3da0-660a-41a1-b6a2-221a71c0cc17" ) );

      BOOST_REQUIRE( escrow1.to == transfer.to );
      BOOST_REQUIRE( escrow1.from == transfer.from );
      BOOST_REQUIRE( escrow1.payment == transfer.amount );
      BOOST_REQUIRE( escrow1.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow1.escrow_expiration == transfer.escrow_expiration );
      BOOST_REQUIRE( escrow1.balance.amount == 0 );
      BOOST_REQUIRE( escrow1.is_approved() == false );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: creation of escrow transfer proposal" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: approval of escrow transfer proposal" );

      account_membership_operation member;

      member.signatory = "candice";
      member.account = "candice";
      member.membership_type = "top";
      member.months = 1;
      member.validate();

      tx.operations.push_back( member );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_mediator_operation mediator;
      
      mediator.signatory = "candice";
      mediator.account = "candice";
      mediator.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      mediator.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      mediator.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      mediator.mediator_bond = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      mediator.validate();

      tx.operations.push_back( mediator );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      member.signatory = "dan";
      member.account = "dan";

      tx.operations.push_back( member );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      mediator.signatory = "dan";
      mediator.account = "dan";

      tx.operations.push_back( mediator );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const mediator_object& candice_mediator = db.get_mediator( "candice" );
      const mediator_object& dan_mediator = db.get_mediator( "dan" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset candice_init_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );
      asset dan_init_liquid_balance = db.get_liquid_balance( "dan", SYMBOL_COIN );

      escrow_approve_operation approve;

      approve.signatory = "alice";
      approve.account = "alice";
      approve.mediator = "candice";
      approve.escrow_from = "alice";
      approve.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17";
      approve.approved = true;
      approve.validate();

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "bob";
      approve.account = "bob";
      approve.mediator = "dan";

      tx.operations.push_back( approve );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "candice";
      approve.account = "candice";
      approve.mediator = "candice";

      tx.operations.push_back( approve );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "dan";
      approve.account = "dan";
      approve.mediator = "dan";

      tx.operations.push_back( approve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset escrow_bond = asset( ( escrow1.payment.amount * median_props.escrow_bond_percent ) / PERCENT_100, escrow1.payment.symbol );

      BOOST_REQUIRE( escrow1.to == "bob" );
      BOOST_REQUIRE( escrow1.from == "alice" );
      BOOST_REQUIRE( escrow1.to_mediator == "dan" );
      BOOST_REQUIRE( escrow1.from_mediator == "candice" );
      BOOST_REQUIRE( escrow1.payment == transfer.amount );
      BOOST_REQUIRE( escrow1.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow1.escrow_expiration == transfer.escrow_expiration );
      BOOST_REQUIRE( escrow1.balance == escrow1.payment + 4 * escrow_bond );
      BOOST_REQUIRE( escrow1.from_approved() == true );
      BOOST_REQUIRE( escrow1.to_approved() == true );
      BOOST_REQUIRE( escrow1.from_mediator_approved() == true );
      BOOST_REQUIRE( escrow1.to_mediator_approved() == true );
      BOOST_REQUIRE( escrow1.is_approved() == true );

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset candice_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );
      asset dan_liquid_balance = db.get_liquid_balance( "dan", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - ( transfer.amount + escrow_bond ) ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( candice_liquid_balance == ( candice_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( dan_liquid_balance == ( dan_init_liquid_balance - escrow_bond ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: approval of escrow transfer proposal" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: standard release of escrow funds by FROM account" );

      escrow_release_operation release;

      release.signatory = "alice";
      release.account = "alice";
      release.escrow_from = "alice";
      release.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17";
      release.release_percent = PERCENT_100;
      release.validate();

      tx.operations.push_back( release );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      candice_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );
      dan_liquid_balance = db.get_liquid_balance( "dan", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - transfer.amount ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance + transfer.amount ) );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance );
      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance );

      const auto& escrow_idx = db.get_index< escrow_index >().indices().get< by_from_id >();
      auto escrow_itr = escrow_idx.find( std::make_tuple( "alice", string( "6b3b3da0-660a-41a1-b6a2-221a71c0cc17" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );
      BOOST_REQUIRE( candice_mediator.last_escrow_from == transfer.from );
      BOOST_REQUIRE( to_string( candice_mediator.last_escrow_id ) == transfer.escrow_id );
      BOOST_REQUIRE( dan_mediator.last_escrow_from == transfer.from );
      BOOST_REQUIRE( to_string( dan_mediator.last_escrow_id ) == transfer.escrow_id );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: standard release of escrow funds by FROM account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when from account has insufficient balance" );

      transfer.signatory = "elon";
      transfer.account = "elon";
      transfer.from = "elon";
      transfer.to = "fred";
      transfer.escrow_id = "01eee083-5680-4740-ada3-46adda0994bd";

      tx.operations.push_back( transfer );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when from account has insufficient balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when to and from are the same account" );

      transfer.signatory = "elon";
      transfer.account = "elon";
      transfer.from = "elon";
      transfer.to = "elon";

      tx.operations.push_back( transfer );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when to and from are the same account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when account is not To or From" );

      transfer.signatory = "elon";
      transfer.account = "elon";
      transfer.from = "alice";
      transfer.to = "bob";
      
      tx.operations.push_back( transfer );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when account is not To or From" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure trying to repeal after approval" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "alice";
      approve.account = "alice";
      approve.mediator = "candice";
      approve.escrow_from = "alice";
      approve.escrow_id = "01eee083-5680-4740-ada3-46adda0994bd";
      approve.approved = true;

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const escrow_object& escrow2 = db.get_escrow( "alice", string( "01eee083-5680-4740-ada3-46adda0994bd" ) );

      BOOST_REQUIRE( escrow2.to == transfer.to );
      BOOST_REQUIRE( escrow2.from == transfer.from );
      BOOST_REQUIRE( escrow2.to_mediator == "dan" );
      BOOST_REQUIRE( escrow2.from_mediator == "candice" );
      BOOST_REQUIRE( escrow2.payment == transfer.amount );
      BOOST_REQUIRE( escrow2.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow2.escrow_expiration == transfer.escrow_expiration ); 

      tx.operations.clear();
      tx.signatures.clear();

      approve.approved = false;

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure trying to repeal after approval" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Success refunding repealed escrow" );

      approve.signatory = "bob";
      approve.account = "bob";
      approve.mediator = "dan";
      approve.escrow_from = "alice";
      approve.escrow_id = "01eee083-5680-4740-ada3-46adda0994bd";
      approve.approved = false;

      tx.operations.push_back( approve );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      escrow_itr = escrow_idx.find( std::make_tuple( "alice", string( "01eee083-5680-4740-ada3-46adda0994bd" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Success refunding repealed escrow" );
   
      BOOST_TEST_MESSAGE( "│   ├── Testing: Automatic refund when escrow is not approved before deadline" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.escrow_id = "8ebbd965-739a-48dd-abeb-67d0363fdae8";
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const escrow_object& escrow3 = db.get_escrow( "alice", string( "8ebbd965-739a-48dd-abeb-67d0363fdae8" ) );

      BOOST_REQUIRE( escrow3.to == transfer.to );
      BOOST_REQUIRE( escrow3.from == transfer.from );
      BOOST_REQUIRE( escrow3.payment == transfer.amount );
      BOOST_REQUIRE( escrow3.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow3.escrow_expiration == transfer.escrow_expiration ); 

      generate_blocks( transfer.acceptance_time + BLOCK_INTERVAL );

      escrow_itr = escrow_idx.find( std::make_tuple( "alice", string( "8ebbd965-739a-48dd-abeb-67d0363fdae8" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Automatic refund when escrow is not approved before deadline" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when releasing unapproved escrow" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      release.signatory = "alice";
      release.account = "alice";
      release.escrow_from = "alice";
      release.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      release.release_percent = 0;

      tx.operations.push_back( release );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when releasing unapproved escrow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Success editing unapproved escrow" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( 1001 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const escrow_object& escrow4 = db.get_escrow( "alice", string( "98351a27-d0d7-456a-b732-4fb414a0e639" ) );

      BOOST_REQUIRE( escrow4.to == transfer.to );
      BOOST_REQUIRE( escrow4.from == transfer.from );
      BOOST_REQUIRE( escrow4.payment == transfer.amount );
      BOOST_REQUIRE( escrow4.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow4.escrow_expiration == transfer.escrow_expiration ); 

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Success editing unapproved escrow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when non-particpant account attempts to approve" );

      approve.signatory = "elon";
      approve.account = "elon";
      approve.mediator = "candice";
      approve.escrow_from = "alice";
      approve.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      approve.approved = true;

      tx.operations.push_back( approve );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when when non-particpant account attempts to approve" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when approving with an invalid mediator" );

      approve.signatory = "alice";
      approve.account = "alice";
      approve.mediator = "elon";   // Elon is not a mediator
      approve.escrow_from = "alice";
      approve.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      approve.approved = true;

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when approving with an invalid mediator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when releasing approved escrow back to self without dispute" );

      approve.signatory = "alice";
      approve.account = "alice";
      approve.mediator = "candice";
      approve.escrow_from = "alice";
      approve.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      approve.approved = true;

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "bob";
      approve.account = "bob";
      approve.mediator = "dan";

      tx.operations.push_back( approve );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "candice";
      approve.account = "candice";
      approve.mediator = "candice";

      tx.operations.push_back( approve );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "dan";
      approve.account = "dan";
      approve.mediator = "dan";

      tx.operations.push_back( approve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      release.signatory = "alice";
      release.account = "alice";
      release.escrow_from = "alice";
      release.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      release.release_percent = 0;      // FROM must release with PERCENT_100 before dispute

      tx.operations.push_back( release );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      release.signatory = "bob";
      release.account = "bob";
      release.escrow_from = "alice";
      release.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      release.release_percent = PERCENT_100;      // TO must release with 0 before dispute

      tx.operations.push_back( release );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when releasing approved escrow back to self without dispute" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when editing approved escrow" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when editing approved escrow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Successfully disputing approved escrow" );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.new_account_name = "newuser";
      create.governance_account = INIT_ACCOUNT;
      create.owner_auth = authority( 1, alice_public_owner_key, 1 );
      create.active_auth = authority( 1, alice_public_active_key, 1 );
      create.posting_auth = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      for( auto i = 0; i < 100; i++ )
      {
         create.new_account_name = "newuser"+fc::to_string( i );

         tx.operations.push_back( create );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         fund( "newuser"+fc::to_string( i ), asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         fund_stake( "newuser"+fc::to_string( i ), asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

         member.signatory = "newuser"+fc::to_string( i );
         member.account = "newuser"+fc::to_string( i );

         tx.operations.push_back( member );
         tx.sign( alice_private_active_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         mediator.signatory = "newuser"+fc::to_string( i );
         mediator.account = "newuser"+fc::to_string( i );

         tx.operations.push_back( mediator );
         tx.sign( alice_private_active_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();
      }

      escrow_dispute_operation dispute;

      dispute.signatory = "alice";
      dispute.account = "alice";
      dispute.escrow_from = "alice";
      dispute.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";

      tx.operations.push_back( dispute );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      escrow_bond = asset( ( escrow4.payment.amount * median_props.escrow_bond_percent ) / PERCENT_100, escrow4.payment.symbol );

      BOOST_REQUIRE( escrow4.to == "bob" );
      BOOST_REQUIRE( escrow4.from == "alice" );
      BOOST_REQUIRE( escrow4.to_mediator == "dan" );
      BOOST_REQUIRE( escrow4.from_mediator == "candice" );
      BOOST_REQUIRE( escrow4.payment == transfer.amount );
      BOOST_REQUIRE( escrow4.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow4.escrow_expiration == transfer.escrow_expiration );
      BOOST_REQUIRE( escrow4.dispute_release_time == now() + ESCROW_DISPUTE_DURATION );
      BOOST_REQUIRE( escrow4.balance == escrow4.payment + 4 * escrow_bond );
      BOOST_REQUIRE( escrow4.from_approved() == true );
      BOOST_REQUIRE( escrow4.to_approved() == true );
      BOOST_REQUIRE( escrow4.from_mediator_approved() == true );
      BOOST_REQUIRE( escrow4.to_mediator_approved() == true );
      BOOST_REQUIRE( escrow4.is_approved() == true );
      BOOST_REQUIRE( escrow4.disputed == true );
      BOOST_REQUIRE( escrow4.mediators.size() == ESCROW_DISPUTE_MEDIATOR_AMOUNT );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Successfully disputing approved escrow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when escrow is already disputed" );

      dispute.signatory = "bob";
      dispute.account = "bob";
      dispute.escrow_from = "alice";
      dispute.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";

      tx.operations.push_back( dispute );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when escrow is already disputed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Dispute resolution mediators approving transfer" );

      for( account_name_type m : escrow4.mediators )
      {
         approve.signatory = m;
         approve.account = m;
         approve.mediator = m;

         tx.operations.push_back( approve );
         tx.sign( alice_private_active_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         flat_map< account_name_type, bool > approvals = escrow4.approvals;

         BOOST_REQUIRE( approvals[ m ] == true );
      }

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Dispute resolution mediators approving transfer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Sucessful fund release from dispute" );

      release.signatory = "alice";
      release.account = "alice";
      release.escrow_from = "alice";
      release.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      release.release_percent = 0;

      tx.operations.push_back( release );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      release.signatory = "bob";
      release.account = "bob";
      release.release_percent = PERCENT_100;

      tx.operations.push_back( release );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      release.signatory = "candice";
      release.account = "candice";
      release.release_percent = 25 * PERCENT_1;

      tx.operations.push_back( release );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      release.signatory = "dan";
      release.account = "dan";
      release.release_percent = 75 * PERCENT_1;

      tx.operations.push_back( release );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      for( account_name_type m : escrow4.mediators )
      {
         release.signatory = m;
         release.account = m;
         release.release_percent = 10 * PERCENT_1;

         tx.operations.push_back( release );
         tx.sign( alice_private_active_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         flat_map< account_name_type, uint16_t > release_percentages = escrow4.release_percentages;

         BOOST_REQUIRE( release_percentages[ m ] == release.release_percent );
      }

      generate_blocks( escrow4.dispute_release_time + BLOCK_INTERVAL );

      escrow_itr = escrow_idx.find( std::make_tuple( "alice", string( "98351a27-d0d7-456a-b732-4fb414a0e639" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );
      BOOST_REQUIRE( candice_mediator.last_escrow_from == transfer.from );
      BOOST_REQUIRE( to_string( candice_mediator.last_escrow_id ) == transfer.escrow_id );
      BOOST_REQUIRE( dan_mediator.last_escrow_from == transfer.from );
      BOOST_REQUIRE( to_string( dan_mediator.last_escrow_id ) == transfer.escrow_id );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Sucessful fund release from dispute" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when disputing after escrow expiration" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.escrow_id = "efab7764-63af-4e6a-95e4-b4dd7c23e40b";
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "alice";
      approve.account = "alice";
      approve.mediator = "candice";
      approve.escrow_from = "alice";
      approve.escrow_id = "efab7764-63af-4e6a-95e4-b4dd7c23e40b";
      approve.approved = true;

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "bob";
      approve.account = "bob";
      approve.mediator = "dan";

      tx.operations.push_back( approve );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "candice";
      approve.account = "candice";
      approve.mediator = "candice";

      tx.operations.push_back( approve );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "dan";
      approve.account = "dan";
      approve.mediator = "dan";

      tx.operations.push_back( approve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( transfer.escrow_expiration + BLOCK_INTERVAL );

      const escrow_object& escrow5 = db.get_escrow( "alice", string( "efab7764-63af-4e6a-95e4-b4dd7c23e40b" ) );

      escrow_bond = asset( ( escrow5.payment.amount * median_props.escrow_bond_percent ) / PERCENT_100, escrow5.payment.symbol );

      BOOST_REQUIRE( escrow5.to == "bob" );
      BOOST_REQUIRE( escrow5.from == "alice" );
      BOOST_REQUIRE( escrow5.to_mediator == "dan" );
      BOOST_REQUIRE( escrow5.from_mediator == "candice" );
      BOOST_REQUIRE( escrow5.payment == transfer.amount );
      BOOST_REQUIRE( escrow5.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow5.escrow_expiration == transfer.escrow_expiration );
      BOOST_REQUIRE( escrow5.dispute_release_time == now() + ESCROW_DISPUTE_DURATION );
      BOOST_REQUIRE( escrow5.balance == escrow5.payment + 4 * escrow_bond );
      BOOST_REQUIRE( escrow5.from_approved() == true );
      BOOST_REQUIRE( escrow5.to_approved() == true );
      BOOST_REQUIRE( escrow5.from_mediator_approved() == true );
      BOOST_REQUIRE( escrow5.to_mediator_approved() == true );
      BOOST_REQUIRE( escrow5.is_approved() == true );

      dispute.signatory = "bob";
      dispute.account = "bob";
      dispute.escrow_from = "alice";
      dispute.escrow_id = "efab7764-63af-4e6a-95e4-b4dd7c23e40b";

      tx.operations.push_back( dispute );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when disputing after escrow expiration" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Release to self after escrow expiration" );

      release.signatory = "alice";
      release.account = "alice";
      release.escrow_from = "alice";
      release.escrow_id = "efab7764-63af-4e6a-95e4-b4dd7c23e40b";
      release.release_percent = 0;

      tx.operations.push_back( release );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      escrow_itr = escrow_idx.find( std::make_tuple( "alice", string( "efab7764-63af-4e6a-95e4-b4dd7c23e40b" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );
      BOOST_REQUIRE( candice_mediator.last_escrow_from == transfer.from );
      BOOST_REQUIRE( to_string( candice_mediator.last_escrow_id ) == transfer.escrow_id );
      BOOST_REQUIRE( dan_mediator.last_escrow_from == transfer.from );
      BOOST_REQUIRE( to_string( dan_mediator.last_escrow_id ) == transfer.escrow_id );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Release to self after escrow expiration" );

      BOOST_TEST_MESSAGE( "├── Passed: ESCROW TRANSFER OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()