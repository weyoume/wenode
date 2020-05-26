#include <boost/test/unit_test.hpp>
#include <node/chain/database.hpp>
#include <node/protocol/exceptions.hpp>
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

BOOST_FIXTURE_TEST_SUITE( network_operation_tests, clean_database_fixture );


   //=======================//
   // === Network Tests === //
   //=======================//


BOOST_AUTO_TEST_CASE( update_network_officer_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: NETWORK OFFICER SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Network officer creation" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)
         (margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica) );

      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( alice.name, alice_private_owner_key, alice_public_owner_key );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( bob.name, bob_private_owner_key, bob_public_owner_key );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( candice.name, candice_private_owner_key, candice_public_owner_key );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( dan.name, dan_private_owner_key, dan_public_owner_key );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( elon.name, elon_private_owner_key, elon_public_owner_key );

      fund_stake( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( fred.name, fred_private_owner_key, fred_public_owner_key );

      fund_stake( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( george.name, george_private_owner_key, george_public_owner_key );

      fund_stake( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( haz.name, haz_private_owner_key, haz_public_owner_key );

      fund_stake( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( isabelle.name, isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( jayme.name, jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( kathryn.name, kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( leonie.name, leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( margot.name, margot_private_owner_key, margot_public_owner_key );

      fund_stake( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( natalie.name, natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( olivia.name, olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( peter.name, peter_private_owner_key, peter_public_owner_key );

      fund_stake( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( quentin.name, quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( rachel.name, rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( sam.name, sam_private_owner_key, sam_public_owner_key );

      fund_stake( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( tim.name, tim_private_owner_key, tim_public_owner_key );

      fund_stake( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( veronica.name, veronica_private_owner_key, veronica_public_owner_key );

      account_membership_operation membership;

      membership.signatory = alice.name;
      membership.account = alice.name;
      membership.membership_type = "standard";
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

      officer.signatory = alice.name;
      officer.account = alice.name;
      officer.officer_type = "development";
      officer.details = "details";
      officer.url = "https://www.url.com";
      officer.json = "{ \"valid\": true }";
      officer.active = true;
      officer.validate();

      tx.operations.push_back( officer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const network_officer_object& alice_officer = db.get_network_officer( alice.name );
      
      BOOST_REQUIRE( alice_officer.account == alice.name );
      BOOST_REQUIRE( alice_officer.officer_type == network_officer_role_type::DEVELOPMENT );
      BOOST_REQUIRE( alice_officer.active == true );
      BOOST_REQUIRE( alice_officer.officer_approved == false );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: network officer creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: network officer approval process" );

      network_officer_vote_operation vote;    // 20 accounts vote for officer

      vote.signatory = bob.name;
      vote.account = bob.name;
      vote.network_officer = alice.name;
      vote.vote_rank = 1;
      vote.approved = true;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = candice.name;
      vote.account = candice.name;

      tx.operations.push_back( vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = dan.name;
      vote.account = dan.name;

      tx.operations.push_back( vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = elon.name;
      vote.account = elon.name;

      tx.operations.push_back( vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = fred.name;
      vote.account = fred.name;

      tx.operations.push_back( vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = george.name;
      vote.account = george.name;

      tx.operations.push_back( vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = haz.name;
      vote.account = haz.name;

      tx.operations.push_back( vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = isabelle.name;
      vote.account = isabelle.name;

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = jayme.name;
      vote.account = jayme.name;

      tx.operations.push_back( vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = kathryn.name;
      vote.account = kathryn.name;

      tx.operations.push_back( vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = leonie.name;
      vote.account = leonie.name;

      tx.operations.push_back( vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = margot.name;
      vote.account = margot.name;

      tx.operations.push_back( vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = natalie.name;
      vote.account = natalie.name;

      tx.operations.push_back( vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = olivia.name;
      vote.account = olivia.name;

      tx.operations.push_back( vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = peter.name;
      vote.account = peter.name;

      tx.operations.push_back( vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = quentin.name;
      vote.account = quentin.name;

      tx.operations.push_back( vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = rachel.name;
      vote.account = rachel.name;

      tx.operations.push_back( vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = sam.name;
      vote.account = sam.name;

      tx.operations.push_back( vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = tim.name;
      vote.account = tim.name;

      tx.operations.push_back( vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = veronica.name;
      vote.account = veronica.name;

      tx.operations.push_back( vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      BOOST_REQUIRE( alice_officer.account == alice.name );
      BOOST_REQUIRE( alice_officer.officer_type == network_officer_role_type::DEVELOPMENT );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica) 
      (alice2)(bob2)(candice2)(dan2)(elon2)(fred2)(george2)(haz2)(isabelle2)(jayme2)(kathryn2)(leonie2)(margot2)(natalie2)(olivia2)(peter2)(quentin2)(rachel2)(sam2)(tim2)(veronica2)(execboard));

      fund_stake( alice.name, asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( alice.name, alice_private_owner_key, alice_public_owner_key );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( bob.name, bob_private_owner_key, bob_public_owner_key );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( candice.name, candice_private_owner_key, candice_public_owner_key );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( dan.name, dan_private_owner_key, dan_public_owner_key );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( elon.name, elon_private_owner_key, elon_public_owner_key );

      fund_stake( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( fred.name, fred_private_owner_key, fred_public_owner_key );

      fund_stake( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( george.name, george_private_owner_key, george_public_owner_key );

      fund_stake( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( haz.name, haz_private_owner_key, haz_public_owner_key );

      fund_stake( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( isabelle.name, isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( jayme.name, jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( kathryn.name, kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( leonie.name, leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( margot.name, margot_private_owner_key, margot_public_owner_key );

      fund_stake( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( natalie.name, natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( olivia.name, olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( peter.name, peter_private_owner_key, peter_public_owner_key );

      fund_stake( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( quentin.name, quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( rachel.name, rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( sam.name, sam_private_owner_key, sam_public_owner_key );

      fund_stake( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( tim.name, tim_private_owner_key, tim_public_owner_key );

      fund_stake( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( veronica.name, veronica_private_owner_key, veronica_public_owner_key );

      fund_stake( alice2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( alice2.name, alice2_private_owner_key, alice2_public_owner_key );

      fund_stake( bob2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( bob2.name, bob2_private_owner_key, bob2_public_owner_key );

      fund_stake( candice2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( candice2.name, candice2_private_owner_key, candice2_public_owner_key );

      fund_stake( dan2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( dan2.name, dan2_private_owner_key, dan2_public_owner_key );

      fund_stake( elon2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( elon2.name, elon2_private_owner_key, elon2_public_owner_key );

      fund_stake( fred2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( fred2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( fred2.name, fred2_private_owner_key, fred2_public_owner_key );

      fund_stake( george2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( george2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( george2.name, george2_private_owner_key, george2_public_owner_key );

      fund_stake( haz2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( haz2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( haz2.name, haz2_private_owner_key, haz2_public_owner_key );

      fund_stake( isabelle2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( isabelle2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( isabelle2.name, isabelle2_private_owner_key, isabelle2_public_owner_key );

      fund_stake( jayme2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( jayme2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( jayme2.name, jayme2_private_owner_key, jayme2_public_owner_key );

      fund_stake( kathryn2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( kathryn2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( kathryn2.name, kathryn2_private_owner_key, kathryn2_public_owner_key );

      fund_stake( leonie2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( leonie2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( leonie2.name, leonie2_private_owner_key, leonie2_public_owner_key );

      fund_stake( margot2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( margot2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( margot2.name, margot2_private_owner_key, margot2_public_owner_key );

      fund_stake( natalie2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( natalie2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( natalie2.name, natalie2_private_owner_key, natalie2_public_owner_key );

      fund_stake( olivia2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( olivia2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( olivia2.name, olivia2_private_owner_key, olivia2_public_owner_key );

      fund_stake( peter2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( peter2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( peter2.name, peter2_private_owner_key, peter2_public_owner_key );

      fund_stake( quentin2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( quentin2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( quentin2.name, quentin2_private_owner_key, quentin2_public_owner_key );

      fund_stake( rachel2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( rachel2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( rachel2.name, rachel2_private_owner_key, rachel2_public_owner_key );

      fund_stake( sam2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( sam2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( sam2.name, sam2_private_owner_key, sam2_public_owner_key );

      fund_stake( tim2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( tim2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( tim2.name, tim2_private_owner_key, tim2_public_owner_key );

      fund_stake( veronica2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( veronica2.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( veronica2.name, veronica2_private_owner_key, veronica2_public_owner_key );

      fund_stake( execboard.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( execboard.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( execboard.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( execboard.name, execboard_private_owner_key, execboard_public_owner_key );

      account_business_operation business_create;

      business_create.signatory = execboard.name;
      business_create.account = execboard.name;
      business_create.business_type = "public";
      business_create.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      business_create.business_public_key = string( alice_public_posting_key );
      business_create.init_ceo_account = alice.name;
      business_create.validate();

      signed_transaction tx;
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( business_create );
      tx.sign( execboard_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset_create_operation asset_create;

      asset_create.signatory = execboard.name;
      asset_create.issuer = execboard.name;
      asset_create.symbol = "EXEQ";
      asset_create.asset_type = "equity";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "EXEQ" );
      asset_create.options.display_symbol = "Exec Board Equity";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( execboard_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund_stake( execboard.name, asset( 1000 * BLOCKCHAIN_PRECISION, "EXEQ" ) );
      fund_stake( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, "EXEQ" ) );

      const asset_object& exeq_asset = db.get_asset( "EXEQ" );

      BOOST_REQUIRE( exeq_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( exeq_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( exeq_asset.asset_type == asset_property_type::EQUITY_ASSET );
      BOOST_REQUIRE( exeq_asset.created == now() );
      BOOST_REQUIRE( exeq_asset.last_updated == now() );

      const asset_equity_data_object& exeq_equity = db.get_equity_data( "EXEQ" );

      BOOST_REQUIRE( exeq_equity.business_account == asset_create.issuer );

      account_membership_operation member;

      member.signatory = alice.name;
      member.account = alice.name;
      member.membership_type = "top";
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();

      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      member.signatory = bob.name;
      member.account = bob.name;

      tx.operations.push_back( member );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      member.signatory = candice.name;
      member.account = candice.name;

      tx.operations.push_back( member );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      member.signatory = dan.name;
      member.account = dan.name;

      tx.operations.push_back( member );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      member.signatory = execboard.name;
      member.account = execboard.name;

      tx.operations.push_back( member );
      tx.sign( execboard_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_network_officer_operation officer;

      officer.signatory = bob.name;
      officer.account = bob.name;
      officer.officer_type = "development";
      officer.details = "details";
      officer.url = "https://www.url.com";
      officer.json = "{ \"valid\": true }";
      officer.active = true;
      officer.validate();

      tx.operations.push_back( officer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = candice.name;
      officer.account = candice.name;
      officer.officer_type = "marketing";

      tx.operations.push_back( officer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = dan.name;
      officer.account = dan.name;
      officer.officer_type = "advocacy";

      tx.operations.push_back( officer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_vote_officer_operation vote;

      vote.signatory = alice.name;
      vote.account = alice.name;
      vote.officer_account = bob.name;
      vote.business_account = execboard.name;
      vote.vote_rank = 1;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.officer_account = candice.name;
      vote.vote_rank = 2;

      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.officer_account = dan.name;
      vote.vote_rank = 3;

      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_vote_executive_operation exec_vote;

      exec_vote.signatory = alice.name;
      exec_vote.account = alice.name;
      exec_vote.executive_account = bob.name;
      exec_vote.business_account = execboard.name;
      exec_vote.role = "development";
      exec_vote.vote_rank = 1;
      exec_vote.validate();

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = candice.name;
      exec_vote.role = "marketing";
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = dan.name;
      exec_vote.role = "advocacy";
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_supernode_operation supernode;

      supernode.signatory = alice.name;
      supernode.account = execboard.name;
      supernode.details = "details";
      supernode.url = "https://www.url.com";
      supernode.json = "{ \"valid\": true }";
      supernode.validate();

      tx.operations.push_back( supernode );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_interface_operation interface;
      
      interface.signatory = alice.name;
      interface.account = execboard.name;
      interface.details = "details";
      interface.url = "https://www.url.com";
      interface.json = "{ \"valid\": true }";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
      gov.signatory = alice.name;
      gov.account = execboard.name;
      gov.details = "details";
      gov.url = "https://www.url.com";
      gov.json = "{ \"valid\": true }";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment_create( alice.name, alice_private_posting_key, "alicetestpost" );

      account_create_operation create;

      create.signatory = alice.name;
      create.registrar = execboard.name;
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
      view.author = alice.name;
      view.permlink = "alicetestpost";
      view.supernode = execboard.name;
      view.interface = execboard.name;
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

      exec.signatory = alice.name;
      exec.account = alice.name;
      exec.executive = execboard.name;
      exec.budget = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      exec.details = "details";
      exec.url = "https://www.url.com";
      exec.json = "{ \"valid\": true }";
      exec.active = true;
      exec.validate();

      tx.operations.push_back( exec );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const executive_board_object& executive = db.get_executive_board( execboard.name );
      
      BOOST_REQUIRE( executive.account == execboard.name );
      BOOST_REQUIRE( executive.board_approved == false );
      BOOST_REQUIRE( executive.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: executive board creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: executive board approval process" );

      executive_board_vote_operation eb_vote;    // 40 accounts vote for executive board

      eb_vote.signatory = bob.name;
      eb_vote.account = bob.name;
      eb_vote.executive_board = execboard.name;
      eb_vote.vote_rank = 1;
      eb_vote.approved = true;
      eb_vote.validate();

      tx.operations.push_back( eb_vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = candice.name;
      eb_vote.account = candice.name;

      tx.operations.push_back( eb_vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = dan.name;
      eb_vote.account = dan.name;

      tx.operations.push_back( eb_vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = elon.name;
      eb_vote.account = elon.name;

      tx.operations.push_back( eb_vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = fred.name;
      eb_vote.account = fred.name;

      tx.operations.push_back( eb_vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = george.name;
      eb_vote.account = george.name;

      tx.operations.push_back( eb_vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = haz.name;
      eb_vote.account = haz.name;

      tx.operations.push_back( eb_vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = isabelle.name;
      eb_vote.account = isabelle.name;

      tx.operations.push_back( eb_vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = jayme.name;
      eb_vote.account = jayme.name;

      tx.operations.push_back( eb_vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = kathryn.name;
      eb_vote.account = kathryn.name;

      tx.operations.push_back( eb_vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = leonie.name;
      eb_vote.account = leonie.name;

      tx.operations.push_back( eb_vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = margot.name;
      eb_vote.account = margot.name;

      tx.operations.push_back( eb_vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = natalie.name;
      eb_vote.account = natalie.name;

      tx.operations.push_back( eb_vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = olivia.name;
      eb_vote.account = olivia.name;

      tx.operations.push_back( eb_vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = peter.name;
      eb_vote.account = peter.name;

      tx.operations.push_back( eb_vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = quentin.name;
      eb_vote.account = quentin.name;

      tx.operations.push_back( eb_vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = rachel.name;
      eb_vote.account = rachel.name;

      tx.operations.push_back( eb_vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = sam.name;
      eb_vote.account = sam.name;

      tx.operations.push_back( eb_vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = tim.name;
      eb_vote.account = tim.name;

      tx.operations.push_back( eb_vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = veronica.name;
      eb_vote.account = veronica.name;

      tx.operations.push_back( eb_vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = bob2.name;
      eb_vote.account = bob2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( bob2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = candice2.name;
      eb_vote.account = candice2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( candice2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = dan2.name;
      eb_vote.account = dan2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( dan2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = elon2.name;
      eb_vote.account = elon2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( elon2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = fred2.name;
      eb_vote.account = fred2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( fred2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = george2.name;
      eb_vote.account = george2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( george2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = haz2.name;
      eb_vote.account = haz2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( haz2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = isabelle2.name;
      eb_vote.account = isabelle2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( isabelle2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = jayme2.name;
      eb_vote.account = jayme2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( jayme2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = kathryn2.name;
      eb_vote.account = kathryn2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( kathryn2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = leonie2.name;
      eb_vote.account = leonie2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( leonie2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = margot2.name;
      eb_vote.account = margot2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( margot2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = natalie2.name;
      eb_vote.account = natalie2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( natalie2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = olivia2.name;
      eb_vote.account = olivia2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( olivia2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = peter2.name;
      eb_vote.account = peter2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( peter2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = quentin2.name;
      eb_vote.account = quentin2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( quentin2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = rachel2.name;
      eb_vote.account = rachel2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( rachel2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = sam2.name;
      eb_vote.account = sam2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( sam2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = tim2.name;
      eb_vote.account = tim2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( tim2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = veronica2.name;
      eb_vote.account = veronica2.name;

      tx.operations.push_back( eb_vote );
      tx.sign( veronica2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      BOOST_REQUIRE( executive.account == execboard.name );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica));

      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( alice.name, alice_private_owner_key, alice_public_owner_key );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( bob.name, bob_private_owner_key, bob_public_owner_key );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( candice.name, candice_private_owner_key, candice_public_owner_key );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( dan.name, dan_private_owner_key, dan_public_owner_key );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( elon.name, elon_private_owner_key, elon_public_owner_key );

      fund_stake( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( fred.name, fred_private_owner_key, fred_public_owner_key );

      fund_stake( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( george.name, george_private_owner_key, george_public_owner_key );

      fund_stake( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( haz.name, haz_private_owner_key, haz_public_owner_key );

      fund_stake( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( isabelle.name, isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( jayme.name, jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( kathryn.name, kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( leonie.name, leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( margot.name, margot_private_owner_key, margot_public_owner_key );

      fund_stake( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( natalie.name, natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( olivia.name, olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( peter.name, peter_private_owner_key, peter_public_owner_key );

      fund_stake( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( quentin.name, quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( rachel.name, rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( sam.name, sam_private_owner_key, sam_public_owner_key );

      fund_stake( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( tim.name, tim_private_owner_key, tim_public_owner_key );

      fund_stake( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( veronica.name, veronica_private_owner_key, veronica_public_owner_key );

      account_create_operation create;

      create.signatory = alice.name;
      create.registrar = alice.name;
      create.new_account_name = "govaccount";
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.owner_auth = authority( 1, alice_public_owner_key, 1 );
      create.active_auth = authority( 2, alice_public_active_key, 2 );
      create.posting_auth = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& govaccount = db.get_account( account_name_type( "govaccount" ) );

      fund_stake( govaccount.name, asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( govaccount.name, asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      tx.operations.clear();
      tx.signatures.clear();

      account_membership_operation member;

      member.signatory = govaccount.name;
      member.account = govaccount.name;
      member.membership_type = "top";
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();

      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
      gov.signatory = govaccount.name;
      gov.account = govaccount.name;
      gov.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      gov.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      gov.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_account_object& governance = db.get_governance_account( govaccount.name );
      
      BOOST_REQUIRE( governance.account == govaccount.name );
      BOOST_REQUIRE( governance.account_approved == false );
      BOOST_REQUIRE( governance.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account approval process" );

      subscribe_governance_operation sub;    // 20 accounts subscribe to governance address

      sub.signatory = bob.name;
      sub.account = bob.name;
      sub.governance_account = govaccount.name;
      sub.subscribe = true;
      sub.validate();

      tx.operations.push_back( sub );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = candice.name;
      sub.account = candice.name;

      tx.operations.push_back( sub );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = dan.name;
      sub.account = dan.name;

      tx.operations.push_back( sub );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = elon.name;
      sub.account = elon.name;

      tx.operations.push_back( sub );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = fred.name;
      sub.account = fred.name;

      tx.operations.push_back( sub );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = george.name;
      sub.account = george.name;

      tx.operations.push_back( sub );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = haz.name;
      sub.account = haz.name;

      tx.operations.push_back( sub );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = isabelle.name;
      sub.account = isabelle.name;

      tx.operations.push_back( sub );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = jayme.name;
      sub.account = jayme.name;

      tx.operations.push_back( sub );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = kathryn.name;
      sub.account = kathryn.name;

      tx.operations.push_back( sub );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = leonie.name;
      sub.account = leonie.name;

      tx.operations.push_back( sub );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = margot.name;
      sub.account = margot.name;

      tx.operations.push_back( sub );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = natalie.name;
      sub.account = natalie.name;

      tx.operations.push_back( sub );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = olivia.name;
      sub.account = olivia.name;

      tx.operations.push_back( sub );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = peter.name;
      sub.account = peter.name;

      tx.operations.push_back( sub );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = quentin.name;
      sub.account = quentin.name;

      tx.operations.push_back( sub );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = rachel.name;
      sub.account = rachel.name;

      tx.operations.push_back( sub );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = sam.name;
      sub.account = sam.name;

      tx.operations.push_back( sub );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = tim.name;
      sub.account = tim.name;

      tx.operations.push_back( sub );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = veronica.name;
      sub.account = veronica.name;

      tx.operations.push_back( sub );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      BOOST_REQUIRE( governance.account == govaccount.name );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan) );

      update_supernode_operation supernode;
      
      supernode.signatory = alice.name;
      supernode.account = alice.name;
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

      const supernode_object& alice_supernode = db.get_supernode( alice.name );
      
      BOOST_REQUIRE( alice_supernode.account == alice.name );
      BOOST_REQUIRE( alice_supernode.daily_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.monthly_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: supernode creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode viewing process" );

      comment_create( alice.name, alice_private_posting_key, "alicetestpost" );

      view_operation view;

      view.signatory = bob.name;
      view.viewer = bob.name;
      view.author = alice.name;
      view.permlink = "alicetestpost";
      view.interface = INIT_ACCOUNT;
      view.supernode = alice.name;
      view.viewed = true;
      view.validate();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      BOOST_REQUIRE( alice_supernode.account == alice.name );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation member;

      member.signatory = alice.name;
      member.account = alice.name;
      member.membership_type = "top";
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
      
      interface.signatory = alice.name;
      interface.account = alice.name;
      interface.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      interface.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      interface.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const interface_object& alice_interface = db.get_interface( alice.name );
      
      BOOST_REQUIRE( alice_interface.account == alice.name );
      BOOST_REQUIRE( alice_interface.daily_active_users == 0 );
      BOOST_REQUIRE( alice_interface.monthly_active_users == 0 );
      BOOST_REQUIRE( alice_interface.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: interface creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creating without membership" );
      
      interface.signatory = bob.name;
      interface.account = bob.name;
      interface.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      interface.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      interface.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";

      tx.operations.push_back( interface );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );    // Bob is not a member, and cannot create an interface
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when creating without membership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode viewing process" );

      comment_create( alice.name, alice_private_posting_key, "alicetestpost" );

      view_operation view;

      view.signatory = bob.name;
      view.viewer = bob.name;
      view.author = alice.name;
      view.permlink = "alicetestpost";
      view.interface = alice.name;
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
      
      BOOST_REQUIRE( alice_interface.account == alice.name );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      account_membership_operation member;

      member.signatory = alice.name;
      member.account = alice.name;
      member.membership_type = "top";
      member.months = 1;
      member.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_mediator_operation mediator;
      
      mediator.signatory = alice.name;
      mediator.account = alice.name;
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

      const mediator_object& alice_mediator = db.get_mediator( alice.name );
      
      BOOST_REQUIRE( alice_mediator.account == alice.name );
      BOOST_REQUIRE( alice_mediator.mediator_bond == mediator.mediator_bond );
      BOOST_REQUIRE( alice_mediator.created == now() );
      BOOST_REQUIRE( alice_mediator.last_updated == now() );
      BOOST_REQUIRE( alice_mediator.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: mediator creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creating without membership" );
      
      mediator.signatory = bob.name;
      mediator.account = bob.name;

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica));

      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( alice.name, alice_private_owner_key, alice_public_owner_key );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( bob.name, bob_private_owner_key, bob_public_owner_key );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( candice.name, candice_private_owner_key, candice_public_owner_key );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( dan.name, dan_private_owner_key, dan_public_owner_key );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( elon.name, elon_private_owner_key, elon_public_owner_key );

      fund_stake( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( fred.name, fred_private_owner_key, fred_public_owner_key );

      fund_stake( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( george.name, george_private_owner_key, george_public_owner_key );

      fund_stake( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( haz.name, haz_private_owner_key, haz_public_owner_key );

      fund_stake( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( isabelle.name, isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( jayme.name, jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( kathryn.name, kathryn_private_owner_key, kathryn_public_owner_key );

      fund_stake( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( leonie.name, leonie_private_owner_key, leonie_public_owner_key );

      fund_stake( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( margot.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( margot.name, margot_private_owner_key, margot_public_owner_key );

      fund_stake( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( natalie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( natalie.name, natalie_private_owner_key, natalie_public_owner_key );

      fund_stake( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( olivia.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( olivia.name, olivia_private_owner_key, olivia_public_owner_key );

      fund_stake( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( peter.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( peter.name, peter_private_owner_key, peter_public_owner_key );

      fund_stake( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( quentin.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( quentin.name, quentin_private_owner_key, quentin_public_owner_key );

      fund_stake( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( rachel.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( rachel.name, rachel_private_owner_key, rachel_public_owner_key );

      fund_stake( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( sam.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( sam.name, sam_private_owner_key, sam_public_owner_key );

      fund_stake( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( tim.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( tim.name, tim_private_owner_key, tim_public_owner_key );

      fund_stake( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( veronica.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( veronica.name, veronica_private_owner_key, veronica_public_owner_key );

      generate_block();

      create_community_enterprise_operation create;

      create.signatory = alice.name;
      create.creator = alice.name;
      create.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      create.proposal_type = "funding";
      create.beneficiaries[ alice.name ] = PERCENT_100;
      create.milestone_details.push_back( "Begin proposal" );
      create.milestone_shares.push_back( 50*PERCENT_1 );
      create.milestone_details.push_back( "Finish proposal" );
      create.milestone_shares.push_back( 50*PERCENT_1 );
      create.details = "details";
      create.url = "https://www.url.com";
      create.json = "{ \"valid\": true }";
      create.begin = now() + fc::days(8);
      create.duration = 14;
      create.daily_budget = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_enterprise_object& enterprise = db.get_community_enterprise( create.creator, create.enterprise_id );
      
      BOOST_REQUIRE( enterprise.creator == alice.name );
      BOOST_REQUIRE( enterprise.approved_milestones == -1 );
      BOOST_REQUIRE( enterprise.claimed_milestones == 0 );
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal milestone approval" );

      approve_enterprise_milestone_operation approve;

      approve.signatory = alice.name;
      approve.account = alice.name;
      approve.creator = alice.name;
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

      approve.signatory = bob.name;
      approve.account = bob.name;

      tx.operations.push_back( approve );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = candice.name;
      approve.account = candice.name;

      tx.operations.push_back( approve );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = dan.name;
      approve.account = dan.name;

      tx.operations.push_back( approve );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = elon.name;
      approve.account = elon.name;

      tx.operations.push_back( approve );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = fred.name;
      approve.account = fred.name;

      tx.operations.push_back( approve );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = george.name;
      approve.account = george.name;

      tx.operations.push_back( approve );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = haz.name;
      approve.account = haz.name;

      tx.operations.push_back( approve );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = isabelle.name;
      approve.account = isabelle.name;

      tx.operations.push_back( approve );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = jayme.name;
      approve.account = jayme.name;

      tx.operations.push_back( approve );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = kathryn.name;
      approve.account = kathryn.name;

      tx.operations.push_back( approve );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = leonie.name;
      approve.account = leonie.name;

      tx.operations.push_back( approve );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = margot.name;
      approve.account = margot.name;

      tx.operations.push_back( approve );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = natalie.name;
      approve.account = natalie.name;

      tx.operations.push_back( approve );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = olivia.name;
      approve.account = olivia.name;

      tx.operations.push_back( approve );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = peter.name;
      approve.account = peter.name;

      tx.operations.push_back( approve );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = quentin.name;
      approve.account = quentin.name;

      tx.operations.push_back( approve );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = rachel.name;
      approve.account = rachel.name;

      tx.operations.push_back( approve );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = sam.name;
      approve.account = sam.name;

      tx.operations.push_back( approve );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = tim.name;
      approve.account = tim.name;

      tx.operations.push_back( approve );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = veronica.name;
      approve.account = veronica.name;

      tx.operations.push_back( approve );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const producer_schedule_object& producer_schedule = db.get_producer_schedule();
      db.update_enterprise( enterprise, producer_schedule, props );

      BOOST_REQUIRE( enterprise.creator == alice.name );
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

      claim.signatory = alice.name;
      claim.creator = alice.name;
      claim.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      claim.milestone = 1;
      claim.details = "details";
      claim.validate();

      tx.operations.push_back( claim );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      db.update_enterprise( enterprise, producer_schedule, props );

      BOOST_REQUIRE( enterprise.creator == alice.name );
      BOOST_REQUIRE( enterprise.approved_milestones == 0 );
      BOOST_REQUIRE( enterprise.claimed_milestones == 1 );   // next milestone claimed
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise claim milestone" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal next milestone approval" );

      approve.signatory = alice.name;
      approve.account = alice.name;
      approve.creator = alice.name;
      approve.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      approve.milestone = 1;
      approve.vote_rank = 1;
      approve.approved = true;
      approve.validate();

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = bob.name;
      approve.account = bob.name;

      tx.operations.push_back( approve );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = candice.name;
      approve.account = candice.name;

      tx.operations.push_back( approve );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = dan.name;
      approve.account = dan.name;

      tx.operations.push_back( approve );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = elon.name;
      approve.account = elon.name;

      tx.operations.push_back( approve );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = fred.name;
      approve.account = fred.name;

      tx.operations.push_back( approve );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = george.name;
      approve.account = george.name;

      tx.operations.push_back( approve );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = haz.name;
      approve.account = haz.name;

      tx.operations.push_back( approve );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = isabelle.name;
      approve.account = isabelle.name;

      tx.operations.push_back( approve );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = jayme.name;
      approve.account = jayme.name;

      tx.operations.push_back( approve );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = kathryn.name;
      approve.account = kathryn.name;

      tx.operations.push_back( approve );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = leonie.name;
      approve.account = leonie.name;

      tx.operations.push_back( approve );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = margot.name;
      approve.account = margot.name;

      tx.operations.push_back( approve );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = natalie.name;
      approve.account = natalie.name;

      tx.operations.push_back( approve );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = olivia.name;
      approve.account = olivia.name;

      tx.operations.push_back( approve );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = peter.name;
      approve.account = peter.name;

      tx.operations.push_back( approve );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = quentin.name;
      approve.account = quentin.name;

      tx.operations.push_back( approve );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = rachel.name;
      approve.account = rachel.name;

      tx.operations.push_back( approve );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = sam.name;
      approve.account = sam.name;

      tx.operations.push_back( approve );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = tim.name;
      approve.account = tim.name;

      tx.operations.push_back( approve );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = veronica.name;
      approve.account = veronica.name;

      tx.operations.push_back( approve );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      db.update_enterprise( enterprise, producer_schedule, props );

      BOOST_REQUIRE( enterprise.creator == alice.name );
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