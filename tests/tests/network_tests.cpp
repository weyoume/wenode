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

BOOST_FIXTURE_TEST_SUITE( network_operation_tests, clean_database_fixture );


   //=======================//
   // === Network Tests === //
   //=======================//


BOOST_AUTO_TEST_CASE( update_network_officer_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: NETWORK OFFICER SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: network officer creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key, bob_public_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key, candice_public_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key, dan_public_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key, elon_public_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key, fred_public_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key, george_public_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key, haz_public_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key, margot_public_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key, peter_public_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key, sam_public_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key, tim_public_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key, veronica_public_owner_key );

      account_membership_operation membership;

      membership.signatory = "alice";
      membership.account = "alice";
      membership.membership_type = STANDARD_MEMBERSHIP;
      membership.months = 1;
      membership.interface = INIT_ACCOUNT;
      membership.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( membership );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_network_officer_operation officer;

      officer.signatory = "alice";
      officer.account = "alice";
      officer.officer_type = DEVELOPMENT;
      officer.details = "details";
      officer.url = "www.url.com";
      officer.json = "{\"json\":\"valid\"}";
      officer.active = true;
      officer.validate();

      tx.operations.push_back( officer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const network_officer_object& alice_officer = db.get_network_officer( "alice" );
      
      BOOST_REQUIRE( alice_officer.account == "alice" );
      BOOST_REQUIRE( alice_officer.officer_type == DEVELOPMENT );
      BOOST_REQUIRE( alice_officer.active == true );
      BOOST_REQUIRE( alice_officer.officer_approved == false );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: network officer creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: network officer approval process" );

      network_officer_vote_operation vote;    // 20 accounts vote for officer

      vote.signatory = "bob";
      vote.account = "bob";
      vote.network_officer = "alice";
      vote.vote_rank = 1;
      vote.approved = true;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "candice";
      vote.account = "candice";

      tx.operations.push_back( vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "dan";
      vote.account = "dan";

      tx.operations.push_back( vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "elon";
      vote.account = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "fred";
      vote.account = "fred";

      tx.operations.push_back( vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "george";
      vote.account = "george";

      tx.operations.push_back( vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "haz";
      vote.account = "haz";

      tx.operations.push_back( vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "isabelle";
      vote.account = "isabelle";

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "jayme";
      vote.account = "jayme";

      tx.operations.push_back( vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "kathryn";
      vote.account = "kathryn";

      tx.operations.push_back( vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "leonie";
      vote.account = "leonie";

      tx.operations.push_back( vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "margot";
      vote.account = "margot";

      tx.operations.push_back( vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "natalie";
      vote.account = "natalie";

      tx.operations.push_back( vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "olivia";
      vote.account = "olivia";

      tx.operations.push_back( vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "peter";
      vote.account = "peter";

      tx.operations.push_back( vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "quentin";
      vote.account = "quentin";

      tx.operations.push_back( vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "rachel";
      vote.account = "rachel";

      tx.operations.push_back( vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "sam";
      vote.account = "sam";

      tx.operations.push_back( vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      vote.signatory = "tim";
      vote.account = "tim";

      tx.operations.push_back( vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "veronica";
      vote.account = "veronica";

      tx.operations.push_back( vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const network_officer_object& alice_officer = db.get_network_officer( "alice" );
      
      BOOST_REQUIRE( alice_officer.account == "alice" );
      BOOST_REQUIRE( alice_officer.officer_type == DEVELOPMENT );
      BOOST_REQUIRE( alice_officer.active == true );
      BOOST_REQUIRE( alice_officer.officer_approved == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: network officer approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: NETWORK OFFICER SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( update_executive_board_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: EXECUTIVE BOARD SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: executive board creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica) 
      (alice2)(bob2)(candice2)(dan2)(elon2)(fred2)(george2)(haz2)(isabelle2)(jayme2)(kathryn2)(leonie2)(margot2)(natalie2)(olivia2)(peter2)(quentin2)(rachel2)(sam2)(tim2)(veronica2));

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key, bob_public_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key, candice_public_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key, dan_public_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key, elon_public_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key, fred_public_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key, george_public_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key, haz_public_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key, margot_public_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key, peter_public_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key, sam_public_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key, tim_public_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key, veronica_public_owner_key );

      fund_stake( "alice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice2", alice2_private_owner_key, alice2_public_owner_key );

      fund_stake( "bob2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob2", bob2_private_owner_key, bob2_public_owner_key );

      fund_stake( "candice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice2", candice2_private_owner_key, candice2_public_owner_key );

      fund_stake( "dan2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan2", dan2_private_owner_key, dan2_public_owner_key );

      fund_stake( "elon2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon2", elon2_private_owner_key, elon2_public_owner_key );

      fund_stake( "fred2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred2", fred2_private_owner_key, fred2_public_owner_key );

      fund_stake( "george2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george2", george2_private_owner_key, george2_public_owner_key );

      fund_stake( "haz2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz2", haz2_private_owner_key, haz2_public_owner_key );

      fund_stake( "isabelle2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle2", isabelle2_private_owner_key, isabelle2_public_owner_key );

      fund_stake( "jayme2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme2", jayme2_private_owner_key, jayme2_public_owner_key );

      fund_stake( "kathryn2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn2", kathryn2_private_owner_key, kathryn2_public_owner_key );

      fund_stake( "leonie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie2", leonie2_private_owner_key, leonie2_public_owner_key );

      fund_stake( "margot2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot2", margot2_private_owner_key, margot2_public_owner_key );

      fund_stake( "natalie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie2", natalie2_private_owner_key, natalie2_public_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( "peter2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter2", peter2_private_owner_key, peter2_public_owner_key );

      fund_stake( "quentin2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin2", quentin2_private_owner_key, quentin2_public_owner_key );

      fund_stake( "rachel2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel2", rachel2_private_owner_key, rachel2_public_owner_key );

      fund_stake( "sam2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam2", sam2_private_owner_key, sam2_public_owner_key );

      fund_stake( "tim2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim2", tim2_private_owner_key, tim2_public_owner_key );

      fund_stake( "veronica2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica2", veronica2_private_owner_key, veronica2_public_owner_key );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.new_account_name = "execboard";
      create.governance_account = INIT_ACCOUNT;
      create.business_type = PUBLIC_BUSINESS;
      create.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      create.business_public_key = alice_public_posting_key;
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.governance_account = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      fund_stake( "execboard", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "execboard", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      tx.operations.clear();
      tx.signatures.clear();

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
      member.membership_type = TOP_MEMBERSHIP;
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();

      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      member.signatory = "bob";
      member.account = "bob";

      tx.operations.push_back( member );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      member.signatory = "candice";
      member.account = "candice";

      tx.operations.push_back( member );
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

      member.signatory = "execboard";
      member.account = "execboard";

      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_network_officer_operation officer;

      officer.signatory = "bob";
      officer.account = "bob";
      officer.officer_type = DEVELOPMENT;
      officer.details = "details";
      officer.url = "www.url.com";
      officer.json = "{\"json\":\"valid\"}";
      officer.active = true;
      officer.validate();

      tx.operations.push_back( officer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = "candice";
      officer.account = "candice";
      officer.officer_type = MARKETING;

      tx.operations.push_back( officer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = "dan";
      officer.account = "dan";
      officer.officer_type = ADVOCACY;

      tx.operations.push_back( officer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_vote_officer_operation vote;

      vote.signatory = "alice";
      vote.account = "alice";
      vote.officer_account = "bob";
      vote.business_account = "execboard";
      vote.vote_rank = 1;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.officer_account = "candice";
      vote.vote_rank = 2;

      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.officer_account = "dan";
      vote.vote_rank = 3;

      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_vote_executive_operation exec_vote;

      exec_vote.signatory = "alice";
      exec_vote.account = "alice";
      exec_vote.executive_account = "bob";
      exec_vote.business_account = "execboard";
      exec_vote.role = CHIEF_DEVELOPMENT_OFFICER;
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = "candice";
      exec_vote.role = CHIEF_MARKETING_OFFICER;
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = "dan";
      exec_vote.role = CHIEF_ADVOCACY_OFFICER;
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_supernode_operation supernode;

      supernode.signatory = "alice";
      supernode.account = "execboard";
      supernode.details = "details";
      supernode.url = "www.url.com";
      supernode.json = "{\"json\":\"valid\"}";
      supernode.validate();

      tx.operations.push_back( supernode );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_interface_operation interface;
      
      interface.signatory = "alice";
      interface.account = "execboard";
      interface.details = "details";
      interface.url = "www.url.com";
      interface.json = "{\"json\":\"valid\"}";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
      gov.signatory = "alice";
      gov.account = "execboard";
      gov.details = "details";
      gov.url = "www.url.com";
      gov.json = "{\"json\":\"valid\"}";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "execboard";
      create.new_account_name = "newuser";
      create.governance_account = "execboard";
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
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
      }

      view_operation view;

      view.signatory = "newuser";
      view.viewer = "newuser";
      view.author = "alice";
      view.permlink = "alicetestpost";
      view.supernode = "execboard";
      view.interface = "execboard";
      view.validate();
      
      for( auto i = 0; i < 100; i++ )
      {
         view.signatory = "newuser"+fc::to_string( i );
         view.viewer = "newuser"+fc::to_string( i );

         tx.operations.push_back( view );
         tx.sign( alice_private_posting_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();
      }

      update_executive_board_operation exec;

      exec.signatory = "alice";
      exec.account = "alice";
      exec.executive = "execboard";
      exec.budget = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      exec.details = "details";
      exec.url = "www.url.com";
      exec.json = "{\"json\":\"valid\"}";
      exec.active = true;
      exec.validate();

      tx.operations.push_back( exec );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const executive_board_object& executive = db.get_executive_board( "execboard" );
      
      BOOST_REQUIRE( executive.account == "execboard" );
      BOOST_REQUIRE( executive.board_approved == false );
      BOOST_REQUIRE( executive.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: executive board creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: executive board approval process" );

      executive_board_vote_operation eb_vote;    // 40 accounts vote for executive board

      eb_vote.signatory = "bob";
      eb_vote.account = "bob";
      eb_vote.executive_board = "execboard";
      eb_vote.vote_rank = 1;
      eb_vote.approved = true;
      eb_vote.validate();

      tx.operations.push_back( eb_vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "candice";
      eb_vote.account = "candice";

      tx.operations.push_back( eb_vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "dan";
      eb_vote.account = "dan";

      tx.operations.push_back( eb_vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "elon";
      eb_vote.account = "elon";

      tx.operations.push_back( eb_vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "fred";
      eb_vote.account = "fred";

      tx.operations.push_back( eb_vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "george";
      eb_vote.account = "george";

      tx.operations.push_back( eb_vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "haz";
      eb_vote.account = "haz";

      tx.operations.push_back( eb_vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "isabelle";
      eb_vote.account = "isabelle";

      tx.operations.push_back( eb_vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "jayme";
      eb_vote.account = "jayme";

      tx.operations.push_back( eb_vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "kathryn";
      eb_vote.account = "kathryn";

      tx.operations.push_back( eb_vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "leonie";
      eb_vote.account = "leonie";

      tx.operations.push_back( eb_vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "margot";
      eb_vote.account = "margot";

      tx.operations.push_back( eb_vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "natalie";
      eb_vote.account = "natalie";

      tx.operations.push_back( eb_vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "olivia";
      eb_vote.account = "olivia";

      tx.operations.push_back( eb_vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "peter";
      eb_vote.account = "peter";

      tx.operations.push_back( eb_vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "quentin";
      eb_vote.account = "quentin";

      tx.operations.push_back( eb_vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "rachel";
      eb_vote.account = "rachel";

      tx.operations.push_back( eb_vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "sam";
      eb_vote.account = "sam";

      tx.operations.push_back( eb_vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      eb_vote.signatory = "tim";
      eb_vote.account = "tim";

      tx.operations.push_back( eb_vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "veronica";
      eb_vote.account = "veronica";

      tx.operations.push_back( eb_vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "bob2";
      eb_vote.account = "bob2";

      tx.operations.push_back( eb_vote );
      tx.sign( bob2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "candice2";
      eb_vote.account = "candice2";

      tx.operations.push_back( eb_vote );
      tx.sign( candice2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "dan2";
      eb_vote.account = "dan2";

      tx.operations.push_back( eb_vote );
      tx.sign( dan2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "elon2";
      eb_vote.account = "elon2";

      tx.operations.push_back( eb_vote );
      tx.sign( elon2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "fred2";
      eb_vote.account = "fred2";

      tx.operations.push_back( eb_vote );
      tx.sign( fred2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "george2";
      eb_vote.account = "george2";

      tx.operations.push_back( eb_vote );
      tx.sign( george2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "haz2";
      eb_vote.account = "haz2";

      tx.operations.push_back( eb_vote );
      tx.sign( haz2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "isabelle2";
      eb_vote.account = "isabelle2";

      tx.operations.push_back( eb_vote );
      tx.sign( isabelle2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "jayme2";
      eb_vote.account = "jayme2";

      tx.operations.push_back( eb_vote );
      tx.sign( jayme2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "kathryn2";
      eb_vote.account = "kathryn2";

      tx.operations.push_back( eb_vote );
      tx.sign( kathryn2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "leonie2";
      eb_vote.account = "leonie2";

      tx.operations.push_back( eb_vote );
      tx.sign( leonie2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "margot2";
      eb_vote.account = "margot2";

      tx.operations.push_back( eb_vote );
      tx.sign( margot2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "natalie2";
      eb_vote.account = "natalie2";

      tx.operations.push_back( eb_vote );
      tx.sign( natalie2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "olivia2";
      eb_vote.account = "olivia2";

      tx.operations.push_back( eb_vote );
      tx.sign( olivia2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "peter2";
      eb_vote.account = "peter2";

      tx.operations.push_back( eb_vote );
      tx.sign( peter2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "quentin2";
      eb_vote.account = "quentin2";

      tx.operations.push_back( eb_vote );
      tx.sign( quentin2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "rachel2";
      eb_vote.account = "rachel2";

      tx.operations.push_back( eb_vote );
      tx.sign( rachel2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "sam2";
      eb_vote.account = "sam2";

      tx.operations.push_back( eb_vote );
      tx.sign( sam2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      eb_vote.signatory = "tim2";
      eb_vote.account = "tim2";

      tx.operations.push_back( eb_vote );
      tx.sign( tim2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "veronica2";
      eb_vote.account = "veronica2";

      tx.operations.push_back( eb_vote );
      tx.sign( veronica2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const executive_board_object& executive = db.get_executive_board( "execboard" );
      
      BOOST_REQUIRE( executive.account == "execboard" );
      BOOST_REQUIRE( executive.board_approved == true );
      BOOST_REQUIRE( executive.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: executive board approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: EXECUTIVE BOARD SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( update_governance_account_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: GOVERNANCE ACCOUNT SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica));

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key, bob_public_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key, candice_public_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key, dan_public_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key, elon_public_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key, fred_public_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key, george_public_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key, haz_public_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key, margot_public_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key, peter_public_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key, sam_public_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key, tim_public_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key, veronica_public_owner_key );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.new_account_name = "govaccount";
      create.governance_account = INIT_ACCOUNT;
      create.business_type = PUBLIC_BUSINESS;
      create.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      create.business_public_key = alice_public_posting_key;
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.governance_account = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      fund_stake( "govaccount", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "govaccount", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      tx.operations.clear();
      tx.signatures.clear();

      account_membership_operation member;

      member.signatory = "govaccount";
      member.account = "govaccount";
      member.membership_type = TOP_MEMBERSHIP;
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();

      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
      gov.signatory = "govaccount";
      gov.account = "govaccount";
      gov.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      gov.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      gov.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_account_object& governance = db.get_governance_account( "govaccount" );
      
      BOOST_REQUIRE( governance.account == "govaccount" );
      BOOST_REQUIRE( governance.account_approved == false );
      BOOST_REQUIRE( governance.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account approval process" );

      subscribe_governance_operation sub;    // 20 accounts subscribe to governance address

      sub.signatory = "bob";
      sub.account = "bob";
      sub.governance_account = "govaccount";
      sub.subscribe = true;
      sub.validate();

      tx.operations.push_back( sub );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "candice";
      sub.account = "candice";

      tx.operations.push_back( sub );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "dan";
      sub.account = "dan";

      tx.operations.push_back( sub );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "elon";
      sub.account = "elon";

      tx.operations.push_back( sub );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "fred";
      sub.account = "fred";

      tx.operations.push_back( sub );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "george";
      sub.account = "george";

      tx.operations.push_back( sub );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "haz";
      sub.account = "haz";

      tx.operations.push_back( sub );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "isabelle";
      sub.account = "isabelle";

      tx.operations.push_back( sub );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "jayme";
      sub.account = "jayme";

      tx.operations.push_back( sub );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "kathryn";
      sub.account = "kathryn";

      tx.operations.push_back( sub );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "leonie";
      sub.account = "leonie";

      tx.operations.push_back( sub );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "margot";
      sub.account = "margot";

      tx.operations.push_back( sub );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "natalie";
      sub.account = "natalie";

      tx.operations.push_back( sub );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "olivia";
      sub.account = "olivia";

      tx.operations.push_back( sub );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "peter";
      sub.account = "peter";

      tx.operations.push_back( sub );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "quentin";
      sub.account = "quentin";

      tx.operations.push_back( sub );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "rachel";
      sub.account = "rachel";

      tx.operations.push_back( sub );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "sam";
      sub.account = "sam";

      tx.operations.push_back( sub );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      sub.signatory = "tim";
      sub.account = "tim";

      tx.operations.push_back( sub );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "veronica";
      sub.account = "veronica";

      tx.operations.push_back( sub );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_account_object& governance = db.get_governance_account( "govaccount" );
      
      BOOST_REQUIRE( governance.account == "govaccount" );
      BOOST_REQUIRE( governance.account_approved == true );
      BOOST_REQUIRE( governance.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: GOVERNANCE ACCOUNT SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( update_supernode_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: SUPERNODE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      update_supernode_operation supernode;
      
      supernode.signatory = "alice";
      supernode.account = "alice";
      supernode.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      supernode.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      supernode.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      supernode.auth_api_endpoint = INIT_AUTH_ENDPOINT;
      supernode.node_api_endpoint = INIT_NODE_ENDPOINT;
      supernode.notification_api_endpoint = INIT_NOTIFICATION_ENDPOINT;
      supernode.ipfs_endpoint = INIT_IPFS_ENDPOINT;
      supernode.bittorrent_endpoint = INIT_BITTORRENT_ENDPOINT;
      supernode.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      tx.operations.push_back( supernode );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const supernode_object& alice_supernode = db.get_supernode( "alice" );
      
      BOOST_REQUIRE( alice_supernode.account == "alice" );
      BOOST_REQUIRE( alice_supernode.daily_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.monthly_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: supernode creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode viewing process" );

      const comment_object& alice_post = comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      view_operation view;

      view.signatory = "bob";
      view.viewer = "bob";
      view.author = "alice";
      view.permlink = "alicetestpost";
      view.interface = INIT_ACCOUNT;
      view.supernode = "alice";
      view.viewed = true;
      view.validate();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      const supernode_object& alice_supernode = db.get_supernode( "alice" );
      
      BOOST_REQUIRE( alice_supernode.account == "alice" );
      BOOST_REQUIRE( alice_supernode.daily_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_supernode.monthly_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_supernode.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: supernode viewing process" );

      BOOST_TEST_MESSAGE( "├── Passed: SUPERNODE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( update_interface_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: INTERFACE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: interface creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
      member.membership_type = TOP_MEMBERSHIP;
      member.months = 1;
      member.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_interface_operation interface;
      
      interface.signatory = "alice";
      interface.account = "alice";
      interface.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      interface.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      interface.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const interface_object& alice_interface = db.get_interface( "alice" );
      
      BOOST_REQUIRE( alice_interface.account == "alice" );
      BOOST_REQUIRE( alice_interface.daily_active_users == 0 );
      BOOST_REQUIRE( alice_interface.monthly_active_users == 0 );
      BOOST_REQUIRE( alice_interface.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: interface creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creating without membership" );
      
      interface.signatory = "bob";
      interface.account = "bob";
      interface.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      interface.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      interface.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";

      tx.operations.push_back( interface );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );    // Bob is not a member, and cannot create an interface
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when creating without membership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode viewing process" );

      const comment_object& alice_post = comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      view_operation view;

      view.signatory = "bob";
      view.viewer = "bob";
      view.author = "alice";
      view.permlink = "alicetestpost";
      view.interface = "alice";
      view.supernode = INIT_ACCOUNT;
      view.viewed = true;
      view.validate();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const interface_object& alice_interface = db.get_interface( "alice" );
      
      BOOST_REQUIRE( alice_interface.account == "alice" );
      BOOST_REQUIRE( alice_interface.daily_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_interface.monthly_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_interface.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: interface viewing process" );

      BOOST_TEST_MESSAGE( "├── Passed: INTERFACE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( update_mediator_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: MEDIATOR SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: mediator creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
      member.membership_type = TOP_MEMBERSHIP;
      member.months = 1;
      member.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_mediator_operation mediator;
      
      mediator.signatory = "alice";
      mediator.account = "alice";
      mediator.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      mediator.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      mediator.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      mediator.mediator_bond = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      mediator.validate();

      tx.operations.push_back( mediator );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const mediator_object& alice_mediator = db.get_mediator( "alice" );
      
      BOOST_REQUIRE( alice_mediator.account == "alice" );
      BOOST_REQUIRE( alice_mediator.mediator_bond == mediator.mediator_bond );
      BOOST_REQUIRE( alice_mediator.created == now() );
      BOOST_REQUIRE( alice_mediator.last_updated == now() );
      BOOST_REQUIRE( alice_mediator.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: mediator creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creating without membership" );
      
      mediator.signatory = "bob";
      mediator.account = "bob";

      tx.operations.push_back( mediator );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );    // Bob is not a member, and cannot create a mediator
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when creating without membership" );

      BOOST_TEST_MESSAGE( "├── Passed: MEDIATOR SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( community_enterprise_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMUNITY ENTERPRISE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica));

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key, bob_public_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key, candice_public_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key, dan_public_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key, elon_public_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key, fred_public_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key, george_public_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key, haz_public_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key, margot_public_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key, peter_public_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key, sam_public_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key, tim_public_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key, veronica_public_owner_key );

      create_community_enterprise_operation create;

      create.signatory = "alice";
      create.creator = "alice";
      create.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      create.proposal_type = FUNDING;
      create.beneficiaries[ "alice" ] = PERCENT_100;
      create.milestones.push_back( std::make_pair( "Begin proposal", 50*PERCENT_1 ) );
      create.milestones.push_back( std::make_pair( "Finish proposal", 50*PERCENT_1 ) );
      create.details = "details";
      create.url = "www.url.com";
      create.json = "{\"json\":\"valid\"}";
      create.begin = now() + fc::days(8);
      create.duration = 14;
      create.daily_budget = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );
      
      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == -1 );
      BOOST_REQUIRE( enterprise.claimed_milestones == 0 );
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal milestone approval" );

      approve_enterprise_milestone_operation approve;

      approve.signatory = "alice";
      approve.account = "alice";
      approve.creator = "alice";
      approve.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      approve.milestone = 0;
      approve.vote_rank = 1;
      approve.approved = true;
      approve.validate();

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "bob";
      approve.account = "bob";

      tx.operations.push_back( approve );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "candice";
      approve.account = "candice";

      tx.operations.push_back( approve );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "dan";
      approve.account = "dan";

      tx.operations.push_back( approve );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "elon";
      approve.account = "elon";

      tx.operations.push_back( approve );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "fred";
      approve.account = "fred";

      tx.operations.push_back( approve );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "george";
      approve.account = "george";

      tx.operations.push_back( approve );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "haz";
      approve.account = "haz";

      tx.operations.push_back( approve );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "isabelle";
      approve.account = "isabelle";

      tx.operations.push_back( approve );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "jayme";
      approve.account = "jayme";

      tx.operations.push_back( approve );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "kathryn";
      approve.account = "kathryn";

      tx.operations.push_back( approve );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "leonie";
      approve.account = "leonie";

      tx.operations.push_back( approve );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "margot";
      approve.account = "margot";

      tx.operations.push_back( approve );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "natalie";
      approve.account = "natalie";

      tx.operations.push_back( approve );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "olivia";
      approve.account = "olivia";

      tx.operations.push_back( approve );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "peter";
      approve.account = "peter";

      tx.operations.push_back( approve );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "quentin";
      approve.account = "quentin";

      tx.operations.push_back( approve );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "rachel";
      approve.account = "rachel";

      tx.operations.push_back( approve );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "sam";
      approve.account = "sam";

      tx.operations.push_back( approve );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      approve.signatory = "tim";
      approve.account = "tim";

      tx.operations.push_back( approve );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "veronica";
      approve.account = "veronica";

      tx.operations.push_back( approve );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const producer_schedule_object& producer_schedule = db.get_producer_schedule();
      db.update_enterprise( enterprise, producer_schedule, props );

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );

      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == 0 );    // initial milestone now approved
      BOOST_REQUIRE( enterprise.claimed_milestones == 0 );
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );

      generate_blocks( BLOCKS_PER_DAY * 8 - 1 );

      BOOST_REQUIRE( enterprise.days_paid == 0 );

      generate_block();

      BOOST_REQUIRE( enterprise.days_paid == 1 );   // Daily budget is paid

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal milestone approval" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise claim milestone" );

      claim_enterprise_milestone_operation claim;

      claim.signatory = "alice";
      claim.creator = "alice";
      claim.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      claim.milestone = 1;
      claim.details = "details";
      claim.validate();

      tx.operations.push_back( claim );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const producer_schedule_object& producer_schedule = db.get_producer_schedule();
      db.update_enterprise( enterprise, producer_schedule, props );

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );

      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == 0 );
      BOOST_REQUIRE( enterprise.claimed_milestones == 1 );   // next milestone claimed
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise claim milestone" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal next milestone approval" );

      approve_enterprise_milestone_operation approve;

      approve.signatory = "alice";
      approve.account = "alice";
      approve.creator = "alice";
      approve.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      approve.milestone = 1;
      approve.details = "details";
      approve.vote_rank = 1;
      approve.approved = true;
      approve.validate();

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "bob";
      approve.account = "bob";

      tx.operations.push_back( approve );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "candice";
      approve.account = "candice";

      tx.operations.push_back( approve );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "dan";
      approve.account = "dan";

      tx.operations.push_back( approve );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "elon";
      approve.account = "elon";

      tx.operations.push_back( approve );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "fred";
      approve.account = "fred";

      tx.operations.push_back( approve );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "george";
      approve.account = "george";

      tx.operations.push_back( approve );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "haz";
      approve.account = "haz";

      tx.operations.push_back( approve );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "isabelle";
      approve.account = "isabelle";

      tx.operations.push_back( approve );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "jayme";
      approve.account = "jayme";

      tx.operations.push_back( approve );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "kathryn";
      approve.account = "kathryn";

      tx.operations.push_back( approve );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "leonie";
      approve.account = "leonie";

      tx.operations.push_back( approve );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "margot";
      approve.account = "margot";

      tx.operations.push_back( approve );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "natalie";
      approve.account = "natalie";

      tx.operations.push_back( approve );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "olivia";
      approve.account = "olivia";

      tx.operations.push_back( approve );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "peter";
      approve.account = "peter";

      tx.operations.push_back( approve );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "quentin";
      approve.account = "quentin";

      tx.operations.push_back( approve );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "rachel";
      approve.account = "rachel";

      tx.operations.push_back( approve );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "sam";
      approve.account = "sam";

      tx.operations.push_back( approve );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      approve.signatory = "tim";
      approve.account = "tim";

      tx.operations.push_back( approve );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "veronica";
      approve.account = "veronica";

      tx.operations.push_back( approve );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const producer_schedule_object& producer_schedule = db.get_producer_schedule();
      db.update_enterprise( enterprise, producer_schedule, props );

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );

      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == 1 );    // next milestone now approved
      BOOST_REQUIRE( enterprise.claimed_milestones == 1 );
      BOOST_REQUIRE( enterprise.days_paid == 1 );
      BOOST_REQUIRE( enterprise.active == true );

      generate_blocks( BLOCKS_PER_DAY * 14 - 1 );

      BOOST_REQUIRE( enterprise.days_paid == 13 );

      generate_block();

      BOOST_REQUIRE( enterprise.days_paid == 14 );   // Daily budget is fully completed

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal next milestone approval" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY ENTERPRISE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()