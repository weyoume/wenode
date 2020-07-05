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



BOOST_AUTO_TEST_CASE( escrow_transfer_operation_sequence_tests )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ESCROW TRANSFER OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: creation of escrow transfer proposal" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      signed_transaction tx;

      escrow_transfer_operation transfer;

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17";
      transfer.json = "{ \"valid\": true }";
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

      const escrow_object& escrow1 = db.get_escrow( account_name_type( "alice" ), string( "6b3b3da0-660a-41a1-b6a2-221a71c0cc17" ) );

      BOOST_REQUIRE( escrow1.to == transfer.to );
      BOOST_REQUIRE( escrow1.from == transfer.from );
      BOOST_REQUIRE( escrow1.payment == transfer.amount );
      BOOST_REQUIRE( escrow1.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow1.escrow_expiration == transfer.escrow_expiration );
      BOOST_REQUIRE( escrow1.balance.amount == 0 );
      BOOST_REQUIRE( escrow1.is_approved() == false );

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

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );
      asset candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      asset dan_init_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );

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

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );
      asset candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      asset dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - ( transfer.amount + escrow_bond ) ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( candice_liquid_balance == ( candice_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( dan_liquid_balance == ( dan_init_liquid_balance - escrow_bond ) );

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

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );
      candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_COIN );
      dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - transfer.amount ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance + transfer.amount ) );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance );
      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance );

      const auto& escrow_idx = db.get_index< escrow_index >().indices().get< by_from_id >();
      auto escrow_itr = escrow_idx.find( std::make_tuple( account_name_type( "alice" ), string( "6b3b3da0-660a-41a1-b6a2-221a71c0cc17" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

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

      tx.operations.clear();
      tx.signatures.clear();

      const escrow_object& escrow2 = db.get_escrow( account_name_type( "alice" ), string( "01eee083-5680-4740-ada3-46adda0994bd" ) );

      BOOST_REQUIRE( escrow2.to == transfer.to );
      BOOST_REQUIRE( escrow2.from == transfer.from );
      BOOST_REQUIRE( escrow2.from_mediator == "candice" );
      BOOST_REQUIRE( escrow2.payment == transfer.amount );
      BOOST_REQUIRE( escrow2.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow2.escrow_expiration == transfer.escrow_expiration );

      approve.approved = false;

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

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

      escrow_itr = escrow_idx.find( std::make_tuple( account_name_type( "alice" ), string( "01eee083-5680-4740-ada3-46adda0994bd" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

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

      const escrow_object& escrow3 = db.get_escrow( account_name_type( "alice" ), string( "8ebbd965-739a-48dd-abeb-67d0363fdae8" ) );

      BOOST_REQUIRE( escrow3.to == transfer.to );
      BOOST_REQUIRE( escrow3.from == transfer.from );
      BOOST_REQUIRE( escrow3.payment == transfer.amount );
      BOOST_REQUIRE( escrow3.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow3.escrow_expiration == transfer.escrow_expiration );

      generate_blocks( transfer.acceptance_time + BLOCK_INTERVAL );

      escrow_itr = escrow_idx.find( std::make_tuple( account_name_type( "alice" ), string( "8ebbd965-739a-48dd-abeb-67d0363fdae8" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Automatic refund when escrow is not approved before deadline" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when releasing unapproved escrow" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";
      transfer.acceptance_time = now() + fc::days(1);
      transfer.escrow_expiration = now() + fc::days(8);
      
      tx.operations.push_back( transfer );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when releasing unapproved escrow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Success editing unapproved escrow" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( 1001 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.escrow_id = "98351a27-d0d7-456a-b732-4fb414a0e639";

      asset escrow_amount = transfer.amount;
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const escrow_object& escrow4 = db.get_escrow( account_name_type( "alice" ), string( "98351a27-d0d7-456a-b732-4fb414a0e639" ) );

      BOOST_REQUIRE( escrow4.to == transfer.to );
      BOOST_REQUIRE( escrow4.from == transfer.from );
      BOOST_REQUIRE( escrow4.payment == transfer.amount );
      BOOST_REQUIRE( escrow4.acceptance_time == transfer.acceptance_time );
      BOOST_REQUIRE( escrow4.escrow_expiration == transfer.escrow_expiration ); 

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
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

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
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

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

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when editing approved escrow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Successfully disputing approved escrow" );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.referrer = "alice";
      create.new_account_name = "newuser";
      create.owner_auth = authority( 1, alice_public_owner_key, 1 );
      create.active_auth = authority( 1, alice_public_active_key, 1 );
      create.posting_auth = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 4 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      for( auto i = 0; i < 100; i++ )
      {
         create.new_account_name = "newuser"+fc::to_string( i );

         tx.operations.push_back( create );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         fund_liquid( "newuser"+fc::to_string( i ), asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         fund_stake( "newuser"+fc::to_string( i ), asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

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
      dispute.validate();

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
      BOOST_REQUIRE( escrow4.payment == escrow_amount );
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

      escrow_itr = escrow_idx.find( std::make_tuple( account_name_type( "alice" ), string( "98351a27-d0d7-456a-b732-4fb414a0e639" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Sucessful fund release from dispute" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when disputing after escrow expiration" );

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.escrow_id = "efab7764-63af-4e6a-95e4-b4dd7c23e40b";
      transfer.acceptance_time = now() + fc::days(1);
      transfer.escrow_expiration = now() + fc::days(8);
      
      tx.operations.push_back( transfer );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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

      generate_blocks( transfer.escrow_expiration + BLOCK_INTERVAL, true );

      const escrow_object& escrow5 = db.get_escrow( account_name_type( "alice" ), string( "efab7764-63af-4e6a-95e4-b4dd7c23e40b" ) );

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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

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

      escrow_itr = escrow_idx.find( std::make_tuple( account_name_type( "alice" ), string( "efab7764-63af-4e6a-95e4-b4dd7c23e40b" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Release to self after escrow expiration" );

      BOOST_TEST_MESSAGE( "├── Passed: ESCROW TRANSFER OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( product_sale_operation_sequence_tests )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: PRODUCT OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: creation of product" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      signed_transaction tx;

      product_sale_operation product;

      product.signatory = "alice";
      product.account = "alice";
      product.product_id = "98a65f5a-85e7-4c53-8d64-1ce393a5ae8c";
      product.name = "Artisanal Widget";
      product.url = "https://www.url.com";
      product.json = "{ \"valid\": true }";
      product.product_variants = { "red_widget", "blue_widget" };
      product.product_details = "Coloured Widget, Extremely Artisanal.";
      product.product_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      product.product_prices = { asset( 35 * BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( 35 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) };
      product.stock_available = { 100, 100 };
      product.delivery_variants = { "standard", "express" };
      product.delivery_details = "Standard: Shipped within 3 working days. Express: Shipped next day.";
      product.delivery_prices = { asset( 2 * BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( 5 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) };
      product.active = true;
      product.validate();

      tx.operations.push_back( product );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const product_sale_object& alice_product = db.get_product_sale( account_name_type( "alice" ), string( "98a65f5a-85e7-4c53-8d64-1ce393a5ae8c" ) );

      BOOST_REQUIRE( product.account == alice_product.account );
      BOOST_REQUIRE( product.product_id == to_string( alice_product.product_id ) );
      BOOST_REQUIRE( product.name == to_string( alice_product.name ) );
      BOOST_REQUIRE( product.url == to_string( alice_product.url ) );
      BOOST_REQUIRE( product.json == to_string( alice_product.json ) );
      BOOST_REQUIRE( product.product_image == to_string( alice_product.product_image ) );
      BOOST_REQUIRE( product.delivery_details == to_string( alice_product.delivery_details ) );
      BOOST_REQUIRE( product.product_details == to_string( alice_product.product_details ) );

      for( size_t i = 0; i < product.product_variants.size(); i++ )
      {
         BOOST_REQUIRE( product.product_variants[i] == alice_product.product_variants[i] );
         BOOST_REQUIRE( product.product_prices[i] == alice_product.product_prices[i] );
         BOOST_REQUIRE( product.stock_available[i] == alice_product.stock_available[i] );
      }

      for( size_t i = 0; i < product.delivery_variants.size(); i++ )
      {
         BOOST_REQUIRE( product.delivery_variants[i] == alice_product.delivery_variants[i] );
         BOOST_REQUIRE( product.delivery_prices[i] == alice_product.delivery_prices[i] );
      }

      BOOST_TEST_MESSAGE( "│   ├── Passed: Creation of product" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Purchase of product" );

      product_purchase_operation purchase;

      purchase.signatory = "bob";
      purchase.buyer = "bob";
      purchase.order_id = "cfebe3a5-4b06-4dcb-916c-d8737f600701";
      purchase.seller = "alice";
      purchase.product_id = "98a65f5a-85e7-4c53-8d64-1ce393a5ae8c";
      purchase.order_variants = { "red_widget" };
      purchase.order_size = { 1 };
      purchase.memo = "Delivery memo";
      purchase.json = "{ \"valid\": true }";
      purchase.shipping_address = "1 Flinders Street Melbourne 3000 VIC";
      purchase.delivery_variant = "standard";
      purchase.acceptance_time = now() + fc::days(1);
      purchase.escrow_expiration = now() + fc::days(8);
      purchase.validate();

      tx.operations.push_back( purchase );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const product_purchase_object& bob_purchase = db.get_product_purchase( account_name_type( "bob" ), string( "cfebe3a5-4b06-4dcb-916c-d8737f600701" ) );

      BOOST_REQUIRE( purchase.buyer == bob_purchase.buyer );
      BOOST_REQUIRE( purchase.order_id == to_string( bob_purchase.order_id ) );
      BOOST_REQUIRE( purchase.order_variants[0] == bob_purchase.order_variants[0] );
      BOOST_REQUIRE( purchase.order_size[0] == bob_purchase.order_size[0] );
      BOOST_REQUIRE( purchase.memo == to_string( bob_purchase.memo ) );
      BOOST_REQUIRE( purchase.json == to_string( bob_purchase.json ) );
      BOOST_REQUIRE( purchase.shipping_address == to_string( bob_purchase.shipping_address ) );
      BOOST_REQUIRE( purchase.delivery_variant == bob_purchase.delivery_variant );
      BOOST_REQUIRE( purchase.delivery_details == to_string( bob_purchase.delivery_details ) );
      BOOST_REQUIRE( product.product_prices[0] + product.delivery_prices[0] == bob_purchase.order_value );

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

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_USD );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_USD );
      asset candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_USD );
      asset dan_init_liquid_balance = get_liquid_balance( "dan", SYMBOL_USD );

      escrow_approve_operation approve;

      approve.signatory = "alice";
      approve.account = "alice";
      approve.mediator = "candice";
      approve.escrow_from = "bob";
      approve.escrow_id = "cfebe3a5-4b06-4dcb-916c-d8737f600701";
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

      const escrow_object& bob_escrow = db.get_escrow( account_name_type( "bob" ), string( "cfebe3a5-4b06-4dcb-916c-d8737f600701" ) );
      asset escrow_bond = asset( ( bob_purchase.order_value.amount * median_props.escrow_bond_percent ) / PERCENT_100, bob_escrow.payment.symbol );

      BOOST_REQUIRE( bob_escrow.to == "alice" );
      BOOST_REQUIRE( bob_escrow.from == "bob" );
      BOOST_REQUIRE( bob_escrow.to_mediator == "candice" );
      BOOST_REQUIRE( bob_escrow.from_mediator == "dan" );
      BOOST_REQUIRE( bob_escrow.payment == bob_purchase.order_value );
      BOOST_REQUIRE( bob_escrow.acceptance_time == purchase.acceptance_time );
      BOOST_REQUIRE( bob_escrow.escrow_expiration == purchase.escrow_expiration );
      BOOST_REQUIRE( bob_escrow.balance == bob_escrow.payment + 4 * escrow_bond );
      BOOST_REQUIRE( bob_escrow.from_approved() == true );
      BOOST_REQUIRE( bob_escrow.to_approved() == true );
      BOOST_REQUIRE( bob_escrow.from_mediator_approved() == true );
      BOOST_REQUIRE( bob_escrow.to_mediator_approved() == true );
      BOOST_REQUIRE( bob_escrow.is_approved() == true );

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_USD );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_USD );
      asset candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_USD );
      asset dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_USD );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - ( bob_purchase.order_value + escrow_bond ) ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( candice_liquid_balance == ( candice_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( dan_liquid_balance == ( dan_init_liquid_balance - escrow_bond ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: approval of escrow transfer proposal" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: standard release of escrow funds by FROM account" );

      escrow_release_operation release;

      release.signatory = "bob";
      release.account = "bob";
      release.escrow_from = "bob";
      release.escrow_id = "cfebe3a5-4b06-4dcb-916c-d8737f600701";
      release.release_percent = PERCENT_100;
      release.validate();

      tx.operations.push_back( release );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_USD );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_USD );
      candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_USD );
      dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_USD );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - bob_purchase.order_value ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance + bob_purchase.order_value ) );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance );
      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance );

      const auto& escrow_idx = db.get_index< escrow_index >().indices().get< by_from_id >();
      auto escrow_itr = escrow_idx.find( std::make_tuple( account_name_type( "bob" ), string( "cfebe3a5-4b06-4dcb-916c-d8737f600701" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Release of escrow funds by FROM account" );

      BOOST_TEST_MESSAGE( "├── Passed: ESCROW TRANSFER OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( product_auction_operation_sequence_tests )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: PRODUCT AUCTION OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: creation of product auction" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      signed_transaction tx;

      product_auction_sale_operation auction;

      auction.signatory = "alice";
      auction.account = "alice";
      auction.auction_id = "b65388d9-a99a-49a8-9f2e-761246a1d777";
      auction.auction_type = "open";
      auction.name = "Artisanal Widget";
      auction.url = "https://www.url.com";
      auction.json = "{ \"valid\": true }";
      auction.product_details = "Red Coloured Widget, Extremely Artisanal.";
      auction.product_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      auction.reserve_bid = asset( 25 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      auction.maximum_bid = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      auction.delivery_variants = { "standard", "express" };
      auction.delivery_details = "Standard: Shipped within 3 working days. Express: Shipped next day.";
      auction.delivery_prices = { asset( 2 * BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( 5 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) };
      auction.final_bid_time = now() + fc::days(8);
      auction.completion_time = now() + fc::days(8);
      auction.validate();

      tx.operations.push_back( auction );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const product_auction_sale_object& alice_auction = db.get_product_auction_sale( account_name_type( "alice" ), string( "b65388d9-a99a-49a8-9f2e-761246a1d777" ) );

      BOOST_REQUIRE( auction.account == alice_auction.account );
      BOOST_REQUIRE( auction.auction_id == to_string( alice_auction.auction_id ) );
      BOOST_REQUIRE( auction.name == to_string( alice_auction.name ) );
      BOOST_REQUIRE( auction.url == to_string( alice_auction.url ) );
      BOOST_REQUIRE( auction.json == to_string( alice_auction.json ) );
      BOOST_REQUIRE( auction.product_details == to_string( alice_auction.product_details ) );
      BOOST_REQUIRE( auction.reserve_bid == alice_auction.reserve_bid );
      BOOST_REQUIRE( auction.maximum_bid == alice_auction.maximum_bid );
      BOOST_REQUIRE( auction.final_bid_time == alice_auction.final_bid_time );
      BOOST_REQUIRE( auction.completion_time == alice_auction.completion_time );
      BOOST_REQUIRE( auction.product_image == to_string( alice_auction.product_image ) );
      BOOST_REQUIRE( auction.delivery_details == to_string( alice_auction.delivery_details ) );

      for( size_t i = 0; i < auction.delivery_variants.size(); i++ )
      {
         BOOST_REQUIRE( auction.delivery_variants[i] == alice_auction.delivery_variants[i] );
         BOOST_REQUIRE( auction.delivery_prices[i] == alice_auction.delivery_prices[i] );
      }

      BOOST_TEST_MESSAGE( "│   ├── Passed: Creation of auction" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Bid on auction" );

      product_auction_bid_operation bid;

      bid.signatory = "bob";
      bid.buyer = "bob";
      bid.bid_id = "4cf6928b-32be-4a77-a900-bbb8e97d1cb8";
      bid.seller = "alice";
      bid.auction_id = "b65388d9-a99a-49a8-9f2e-761246a1d777";
      bid.bid_price_commitment = commitment_type();
      bid.blinding_factor = blind_factor_type();
      bid.public_bid_amount = 25 * BLOCKCHAIN_PRECISION;
      bid.memo = "Delivery memo";
      bid.json = "{ \"valid\": true }";
      bid.shipping_address = "1 Flinders Street Melbourne 3000 VIC";
      bid.delivery_variant = "standard";
      bid.delivery_details = "Leave Widget on doorstep.";
      bid.validate();

      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const product_auction_bid_object& bob_bid = db.get_product_auction_bid( account_name_type( "bob" ), string( "4cf6928b-32be-4a77-a900-bbb8e97d1cb8" ) );

      BOOST_REQUIRE( bid.buyer == bob_bid.buyer );
      BOOST_REQUIRE( bid.bid_id == to_string( bob_bid.bid_id ) );
      BOOST_REQUIRE( bid.seller == bob_bid.seller );
      BOOST_REQUIRE( bid.auction_id == to_string( bob_bid.auction_id ) );
      BOOST_REQUIRE( bid.bid_price_commitment == bob_bid.bid_price_commitment );
      BOOST_REQUIRE( bid.blinding_factor == bob_bid.blinding_factor );
      BOOST_REQUIRE( bid.public_bid_amount == bob_bid.public_bid_amount );
      BOOST_REQUIRE( bid.memo == to_string( bob_bid.memo ) );
      BOOST_REQUIRE( bid.json == to_string( bob_bid.json ) );
      BOOST_REQUIRE( bid.shipping_address == to_string( bob_bid.shipping_address ) );
      BOOST_REQUIRE( bid.delivery_variant == bob_bid.delivery_variant );
      BOOST_REQUIRE( bid.delivery_details == to_string( bob_bid.delivery_details ) );

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

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_USD );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_USD );
      asset candice_init_liquid_balance = get_liquid_balance( "candice", SYMBOL_USD );
      asset dan_init_liquid_balance = get_liquid_balance( "dan", SYMBOL_USD );

      generate_blocks( auction.completion_time + fc::minutes(2) );

      BOOST_REQUIRE( bob_bid.winning_bid );

      escrow_approve_operation approve;

      approve.signatory = "alice";
      approve.account = "alice";
      approve.mediator = "candice";
      approve.escrow_from = "bob";
      approve.escrow_id = "4cf6928b-32be-4a77-a900-bbb8e97d1cb8";
      approve.approved = true;
      approve.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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

      const escrow_object& bob_escrow = db.get_escrow( account_name_type( "bob" ), string( "4cf6928b-32be-4a77-a900-bbb8e97d1cb8" ) );
      asset escrow_bond = asset( ( bob_bid.order_value().amount * median_props.escrow_bond_percent ) / PERCENT_100, bob_escrow.payment.symbol );

      BOOST_REQUIRE( bob_escrow.to == "alice" );
      BOOST_REQUIRE( bob_escrow.from == "bob" );
      BOOST_REQUIRE( bob_escrow.to_mediator == "candice" );
      BOOST_REQUIRE( bob_escrow.from_mediator == "dan" );
      BOOST_REQUIRE( bob_escrow.payment == bob_bid.order_value() );
      BOOST_REQUIRE( bob_escrow.balance == bob_escrow.payment + 4 * escrow_bond );
      BOOST_REQUIRE( bob_escrow.from_approved() == true );
      BOOST_REQUIRE( bob_escrow.to_approved() == true );
      BOOST_REQUIRE( bob_escrow.from_mediator_approved() == true );
      BOOST_REQUIRE( bob_escrow.to_mediator_approved() == true );
      BOOST_REQUIRE( bob_escrow.is_approved() == true );

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_USD );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_USD );
      asset candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_USD );
      asset dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_USD );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance - ( bob_bid.order_value() + escrow_bond ) ) );
      BOOST_REQUIRE( candice_liquid_balance == ( candice_init_liquid_balance - escrow_bond ) );
      BOOST_REQUIRE( dan_liquid_balance == ( dan_init_liquid_balance - escrow_bond ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: approval of escrow transfer proposal" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Release of escrow funds by FROM account" );

      escrow_release_operation release;

      release.signatory = "bob";
      release.account = "bob";
      release.escrow_from = "bob";
      release.escrow_id = "4cf6928b-32be-4a77-a900-bbb8e97d1cb8";
      release.release_percent = PERCENT_100;
      release.validate();

      tx.operations.push_back( release );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_USD );
      bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_USD );
      candice_liquid_balance = get_liquid_balance( "candice", SYMBOL_USD );
      dan_liquid_balance = get_liquid_balance( "dan", SYMBOL_USD );

      BOOST_REQUIRE( alice_liquid_balance == ( alice_init_liquid_balance - bob_bid.order_value() ) );
      BOOST_REQUIRE( bob_liquid_balance == ( bob_init_liquid_balance + bob_bid.order_value() ) );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance );
      BOOST_REQUIRE( dan_liquid_balance == dan_init_liquid_balance );

      const auto& escrow_idx = db.get_index< escrow_index >().indices().get< by_from_id >();
      auto escrow_itr = escrow_idx.find( std::make_tuple( account_name_type( "bob" ), string( "b65388d9-a99a-49a8-9f2e-761246a1d777" ) ) );

      BOOST_REQUIRE( escrow_itr == escrow_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Release of escrow funds by FROM account" );

      BOOST_TEST_MESSAGE( "├── Passed: PRODUCT AUCTION OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()