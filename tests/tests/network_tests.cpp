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


BOOST_AUTO_TEST_CASE( network_officer_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: NETWORK OFFICER SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Network officer creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)
      (natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(usain)(veronica)(will)(xerxes)(yan)(zach) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key );
      producer_vote( "bob", bob_private_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key );
      producer_vote( "candice", candice_private_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key );
      producer_vote( "dan", dan_private_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key );
      producer_vote( "elon", elon_private_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key );
      producer_vote( "fred", fred_private_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key );
      producer_vote( "george", george_private_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key );
      producer_vote( "haz", haz_private_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key );
      producer_vote( "isabelle", isabelle_private_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key );
      producer_vote( "jayme", jayme_private_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key );
      producer_vote( "kathryn", kathryn_private_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key );
      producer_vote( "leonie", leonie_private_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key );
      producer_vote( "margot", margot_private_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key );
      producer_vote( "natalie", natalie_private_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key );
      producer_vote( "olivia", olivia_private_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key );
      producer_vote( "peter", peter_private_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key );
      producer_vote( "quentin", quentin_private_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key );
      producer_vote( "rachel", rachel_private_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key );
      producer_vote( "sam", sam_private_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key );
      producer_vote( "tim", tim_private_owner_key );

      fund_stake( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "usain", usain_private_owner_key );
      producer_vote( "usain", usain_private_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key );
      producer_vote( "veronica", veronica_private_owner_key );

      fund_stake( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "will", will_private_owner_key );
      producer_vote( "will", will_private_owner_key );

      fund_stake( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "xerxes", xerxes_private_owner_key );
      producer_vote( "xerxes", xerxes_private_owner_key );

      fund_stake( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "yan", yan_private_owner_key );
      producer_vote( "yan", yan_private_owner_key );

      fund_stake( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "zach", zach_private_owner_key );
      producer_vote( "zach", zach_private_owner_key );

      generate_blocks( TOTAL_PRODUCERS );

      account_membership_operation membership;

      membership.signatory = "alice";
      membership.account = "alice";
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

      network_officer_update_operation officer;

      officer.signatory = "alice";
      officer.account = "alice";
      officer.officer_type = "development";
      officer.reward_currency = SYMBOL_COIN;
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

      const network_officer_object& alice_officer = db.get_network_officer( account_name_type( "alice" ) );
      
      BOOST_REQUIRE( alice_officer.officer_type == network_officer_role_type::DEVELOPMENT );
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

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "tim";
      vote.account = "tim";

      tx.operations.push_back( vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "usain";
      vote.account = "usain";

      tx.operations.push_back( vote );
      tx.sign( usain_private_owner_key, db.get_chain_id() );
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

      vote.signatory = "will";
      vote.account = "will";

      tx.operations.push_back( vote );
      tx.sign( will_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "xerxes";
      vote.account = "xerxes";

      tx.operations.push_back( vote );
      tx.sign( xerxes_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "yan";
      vote.account = "yan";

      tx.operations.push_back( vote );
      tx.sign( yan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "zach";
      vote.account = "zach";

      tx.operations.push_back( vote );
      tx.sign( zach_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      BOOST_REQUIRE( alice_officer.account == "alice" );
      BOOST_REQUIRE( alice_officer.officer_type == network_officer_role_type::DEVELOPMENT );
      BOOST_REQUIRE( alice_officer.active == true );
      BOOST_REQUIRE( alice_officer.officer_approved == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: network officer approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: NETWORK OFFICER SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( executive_board_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: EXECUTIVE BOARD SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: executive board creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)
      (natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(usain)(veronica)(will)(xerxes)(yan)(zach)(execboard) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key );
      producer_vote( "bob", bob_private_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key );
      producer_vote( "candice", candice_private_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key );
      producer_vote( "dan", dan_private_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key );
      producer_vote( "elon", elon_private_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key );
      producer_vote( "fred", fred_private_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key );
      producer_vote( "george", george_private_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key );
      producer_vote( "haz", haz_private_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key );
      producer_vote( "isabelle", isabelle_private_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key );
      producer_vote( "jayme", jayme_private_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key );
      producer_vote( "kathryn", kathryn_private_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key );
      producer_vote( "leonie", leonie_private_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key );
      producer_vote( "margot", margot_private_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key );
      producer_vote( "natalie", natalie_private_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key );
      producer_vote( "olivia", olivia_private_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key );
      producer_vote( "peter", peter_private_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key );
      producer_vote( "quentin", quentin_private_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key );
      producer_vote( "rachel", rachel_private_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key );
      producer_vote( "sam", sam_private_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key );
      producer_vote( "tim", tim_private_owner_key );

      fund_stake( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "usain", usain_private_owner_key );
      producer_vote( "usain", usain_private_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key );
      producer_vote( "veronica", veronica_private_owner_key );

      fund_stake( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "will", will_private_owner_key );
      producer_vote( "will", will_private_owner_key );

      fund_stake( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "xerxes", xerxes_private_owner_key );
      producer_vote( "xerxes", xerxes_private_owner_key );

      fund_stake( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "yan", yan_private_owner_key );
      producer_vote( "yan", yan_private_owner_key );

      fund_stake( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "zach", zach_private_owner_key );
      producer_vote( "zach", zach_private_owner_key );

      fund_stake( "execboard", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "execboard", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "execboard", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "execboard", execboard_private_owner_key );
      producer_vote( "execboard", execboard_private_owner_key );

      generate_blocks( GENESIS_TIME + fc::days(2), true );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
      member.membership_type = "top";
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
      tx.sign( execboard_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_business_operation business_create;

      business_create.signatory = "execboard";
      business_create.account = "execboard";
      business_create.business_type = "public";
      business_create.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      business_create.business_public_key = string( alice_public_connection_key );
      business_create.init_ceo_account = "alice";
      business_create.active = true;
      business_create.validate();

      tx.operations.push_back( business_create );
      tx.sign( execboard_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_business_object& bus_acc = db.get_account_business( "execboard" );

      asset_create_operation asset_create;

      asset_create.signatory = "execboard";
      asset_create.issuer = "execboard";
      asset_create.symbol = "EXEQ";
      asset_create.asset_type = "equity";
      asset_create.coin_liquidity = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 100 * BLOCKCHAIN_PRECISION, asset_symbol_type( "EXEQ" ) );
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

      asset_issue_operation asset_issue;

      asset_issue.signatory = "execboard";
      asset_issue.issuer = "execboard";
      asset_issue.asset_to_issue = asset( 5000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "EXEQ" ) );
      asset_issue.issue_to_account = INIT_ACCOUNT;
      asset_issue.memo = "Issue";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( execboard_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund_stake( "execboard", asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "EXEQ" ) ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "EXEQ" ) ) );

      const asset_object& exeq_asset = db.get_asset( asset_symbol_type( "EXEQ" ) );

      BOOST_REQUIRE( exeq_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( exeq_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( exeq_asset.asset_type == asset_property_type::EQUITY_ASSET );

      const asset_equity_data_object& exeq_equity = db.get_equity_data( asset_symbol_type( "EXEQ" ) );

      BOOST_REQUIRE( exeq_equity.business_account == asset_create.issuer );

      network_officer_update_operation officer;

      officer.signatory = "bob";
      officer.account = "bob";
      officer.officer_type = "development";
      officer.details = "details";
      officer.url = "https://www.url.com";
      officer.json = "{ \"valid\": true }";
      officer.reward_currency = SYMBOL_COIN;
      officer.active = true;
      officer.validate();

      tx.operations.push_back( officer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = "candice";
      officer.account = "candice";
      officer.officer_type = "marketing";

      tx.operations.push_back( officer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = "dan";
      officer.account = "dan";
      officer.officer_type = "advocacy";

      tx.operations.push_back( officer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      string alice_private_connection_wif = graphene::utilities::key_to_wif( alice_private_connection_key );

      account_member_invite_operation invite;

      invite.signatory = "alice";
      invite.account = "alice";
      invite.member = "bob";
      invite.business_account = "execboard";
      invite.message = "Hello";
      invite.encrypted_business_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, alice_private_connection_wif );
      invite.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "candice";
      invite.encrypted_business_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, candice_public_secure_key, alice_private_connection_wif );
      
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "dan";
      invite.encrypted_business_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, dan_public_secure_key, alice_private_connection_wif );
      
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation accept_invite;

      accept_invite.signatory = "bob";
      accept_invite.account = "bob";
      accept_invite.business_account = "execboard";
      accept_invite.validate();

      tx.operations.push_back( accept_invite );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept_invite.signatory = "candice";
      accept_invite.account = "candice";

      tx.operations.push_back( accept_invite );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept_invite.signatory = "dan";
      accept_invite.account = "dan";

      tx.operations.push_back( accept_invite );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( bus_acc.is_member( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( bus_acc.is_member( account_name_type( "candice" ) ) );
      BOOST_REQUIRE( bus_acc.is_member( account_name_type( "dan" ) ) );

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
      exec_vote.role = "development";
      exec_vote.vote_rank = 1;
      exec_vote.validate();

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = "candice";
      exec_vote.role = "marketing";
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = "dan";
      exec_vote.role = "advocacy";
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      supernode_update_operation supernode;

      supernode.signatory = "alice";
      supernode.account = "execboard";
      supernode.details = "details";
      supernode.url = "https://www.url.com";
      supernode.json = "{ \"valid\": true }";
      supernode.validate();

      tx.operations.push_back( supernode );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      interface_update_operation interface;
      
      interface.signatory = "alice";
      interface.account = "execboard";
      interface.details = "details";
      interface.url = "https://www.url.com";
      interface.json = "{ \"valid\": true }";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      governance_update_operation gov;
      
      gov.signatory = "alice";
      gov.account = "execboard";
      gov.details = "details";
      gov.url = "https://www.url.com";
      gov.json = "{ \"valid\": true }";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( TOTAL_PRODUCERS );

      comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "execboard";
      create.referrer = "execboard";
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

      for( auto i = 0; i < 150; i++ )
      {
         create.new_account_name = "newuser"+fc::to_string( i );

         tx.operations.push_back( create );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         generate_block();
      }

      generate_blocks( now() + fc::minutes(1) );

      governance_subscribe_operation subscribe;

      subscribe.signatory = "newuser";
      subscribe.account = "newuser";
      subscribe.governance_account = "execboard";
      subscribe.approved = true;
      subscribe.validate();

      for( auto i = 0; i < 150; i++ )
      {
         string name = "newuser"+fc::to_string( i );

         subscribe.signatory = name;
         subscribe.account = name;

         tx.operations.push_back( subscribe );
         tx.sign( alice_private_active_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         generate_block();
      }

      comment_view_operation view;

      view.signatory = "newuser";
      view.viewer = "newuser";
      view.author = "alice";
      view.permlink = "alicetestpost";
      view.supernode = "execboard";
      view.interface = "execboard";
      view.validate();
      
      for( auto i = 0; i < 150; i++ )
      {
         string name = "newuser"+fc::to_string( i );

         view.signatory = name;
         view.viewer = name;

         tx.operations.push_back( view );
         tx.sign( alice_private_posting_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         generate_block();
      }

      executive_board_update_operation exec;

      exec.signatory = "alice";
      exec.account = "alice";
      exec.executive = "execboard";
      exec.budget = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      exec.details = "details";
      exec.url = "https://www.url.com";
      exec.json = "{ \"valid\": true }";
      exec.active = true;
      exec.validate();

      tx.operations.push_back( exec );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const executive_board_object& executive = db.get_executive_board( account_name_type( "execboard" ) );
      
      BOOST_REQUIRE( !executive.board_approved );
      BOOST_REQUIRE( executive.active );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: executive board creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: executive board approval process" );

      executive_board_vote_operation eb_vote;    // 150 accounts vote for executive board

      eb_vote.signatory = "newuser";
      eb_vote.account = "newuser";
      eb_vote.executive_board = "execboard";
      eb_vote.vote_rank = 1;
      eb_vote.approved = true;
      eb_vote.validate();

      for( auto i = 0; i < 150; i++ )
      {
         eb_vote.signatory = "newuser"+fc::to_string( i );
         eb_vote.account = "newuser"+fc::to_string( i );

         tx.operations.push_back( eb_vote );
         tx.sign( alice_private_active_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         generate_block();
      }

      eb_vote.signatory = "alice";
      eb_vote.account = "alice";
      eb_vote.vote_rank = 2;
      eb_vote.approved = true;
      eb_vote.validate();

      tx.operations.push_back( eb_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "bob";
      eb_vote.account = "bob";

      tx.operations.push_back( eb_vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "candice";
      eb_vote.account = "candice";

      tx.operations.push_back( eb_vote );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "dan";
      eb_vote.account = "dan";

      tx.operations.push_back( eb_vote );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "elon";
      eb_vote.account = "elon";

      tx.operations.push_back( eb_vote );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "fred";
      eb_vote.account = "fred";

      tx.operations.push_back( eb_vote );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "george";
      eb_vote.account = "george";

      tx.operations.push_back( eb_vote );
      tx.sign( george_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "haz";
      eb_vote.account = "haz";

      tx.operations.push_back( eb_vote );
      tx.sign( haz_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "isabelle";
      eb_vote.account = "isabelle";

      tx.operations.push_back( eb_vote );
      tx.sign( isabelle_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "jayme";
      eb_vote.account = "jayme";

      tx.operations.push_back( eb_vote );
      tx.sign( jayme_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "kathryn";
      eb_vote.account = "kathryn";

      tx.operations.push_back( eb_vote );
      tx.sign( kathryn_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "leonie";
      eb_vote.account = "leonie";

      tx.operations.push_back( eb_vote );
      tx.sign( leonie_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "margot";
      eb_vote.account = "margot";

      tx.operations.push_back( eb_vote );
      tx.sign( margot_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "natalie";
      eb_vote.account = "natalie";

      tx.operations.push_back( eb_vote );
      tx.sign( natalie_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "olivia";
      eb_vote.account = "olivia";

      tx.operations.push_back( eb_vote );
      tx.sign( olivia_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "peter";
      eb_vote.account = "peter";

      tx.operations.push_back( eb_vote );
      tx.sign( peter_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "quentin";
      eb_vote.account = "quentin";

      tx.operations.push_back( eb_vote );
      tx.sign( quentin_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "rachel";
      eb_vote.account = "rachel";

      tx.operations.push_back( eb_vote );
      tx.sign( rachel_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "sam";
      eb_vote.account = "sam";

      tx.operations.push_back( eb_vote );
      tx.sign( sam_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "tim";
      eb_vote.account = "tim";

      tx.operations.push_back( eb_vote );
      tx.sign( tim_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "usain";
      eb_vote.account = "usain";

      tx.operations.push_back( eb_vote );
      tx.sign( usain_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "veronica";
      eb_vote.account = "veronica";

      tx.operations.push_back( eb_vote );
      tx.sign( veronica_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "will";
      eb_vote.account = "will";

      tx.operations.push_back( eb_vote );
      tx.sign( will_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "xerxes";
      eb_vote.account = "xerxes";

      tx.operations.push_back( eb_vote );
      tx.sign( xerxes_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "yan";
      eb_vote.account = "yan";

      tx.operations.push_back( eb_vote );
      tx.sign( yan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      eb_vote.signatory = "zach";
      eb_vote.account = "zach";

      tx.operations.push_back( eb_vote );
      tx.sign( zach_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();
      
      BOOST_REQUIRE( db.get_executive_board( eb_vote.executive_board ).active);
      BOOST_REQUIRE( db.get_executive_board( eb_vote.executive_board ).board_approved );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: executive board approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: EXECUTIVE BOARD SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( governance_update_account_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: GOVERNANCE ACCOUNT SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)
         (natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(usain)(veronica)(will)(xerxes)(yan)(zach) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key );
      producer_vote( "bob", bob_private_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key );
      producer_vote( "candice", candice_private_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key );
      producer_vote( "dan", dan_private_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key );
      producer_vote( "elon", elon_private_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key );
      producer_vote( "fred", fred_private_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key );
      producer_vote( "george", george_private_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key );
      producer_vote( "haz", haz_private_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key );
      producer_vote( "isabelle", isabelle_private_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key );
      producer_vote( "jayme", jayme_private_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key );
      producer_vote( "kathryn", kathryn_private_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key );
      producer_vote( "leonie", leonie_private_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key );
      producer_vote( "margot", margot_private_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key );
      producer_vote( "natalie", natalie_private_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key );
      producer_vote( "olivia", olivia_private_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key );
      producer_vote( "peter", peter_private_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key );
      producer_vote( "quentin", quentin_private_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key );
      producer_vote( "rachel", rachel_private_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key );
      producer_vote( "sam", sam_private_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key );
      producer_vote( "tim", tim_private_owner_key );

      fund_stake( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "usain", usain_private_owner_key );
      producer_vote( "usain", usain_private_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key );
      producer_vote( "veronica", veronica_private_owner_key );

      fund_stake( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "will", will_private_owner_key );
      producer_vote( "will", will_private_owner_key );

      fund_stake( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "xerxes", xerxes_private_owner_key );
      producer_vote( "xerxes", xerxes_private_owner_key );

      fund_stake( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "yan", yan_private_owner_key );
      producer_vote( "yan", yan_private_owner_key );

      fund_stake( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "zach", zach_private_owner_key );
      producer_vote( "zach", zach_private_owner_key );

      generate_blocks( TOTAL_PRODUCERS );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
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

      fund_stake( "govaccount", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "govaccount", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      tx.operations.clear();
      tx.signatures.clear();

      account_membership_operation member;

      member.signatory = "govaccount";
      member.account = "govaccount";
      member.membership_type = "top";
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();

      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      governance_update_operation gov;
      
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

      const governance_account_object& governance = db.get_governance_account( account_name_type( "govaccount" ) );
      
      BOOST_REQUIRE( governance.account == account_name_type( "govaccount" ) );
      BOOST_REQUIRE( governance.account_approved == false );
      BOOST_REQUIRE( governance.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account approval process" );

      governance_subscribe_operation sub;

      sub.signatory = "alice";
      sub.account = "alice";
      sub.governance_account = "govaccount";
      sub.approved = true;
      sub.validate();

      tx.operations.push_back( sub );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "bob";
      sub.account = "bob";

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

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "tim";
      sub.account = "tim";

      tx.operations.push_back( sub );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "usain";
      sub.account = "usain";

      tx.operations.push_back( sub );
      tx.sign( usain_private_owner_key, db.get_chain_id() );
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

      sub.signatory = "will";
      sub.account = "will";

      tx.operations.push_back( sub );
      tx.sign( will_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "xerxes";
      sub.account = "xerxes";

      tx.operations.push_back( sub );
      tx.sign( xerxes_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "yan";
      sub.account = "yan";

      tx.operations.push_back( sub );
      tx.sign( yan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      sub.signatory = "zach";
      sub.account = "zach";

      tx.operations.push_back( sub );
      tx.sign( zach_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      BOOST_REQUIRE( governance.account == "govaccount" );
      BOOST_REQUIRE( governance.account_approved == true );
      BOOST_REQUIRE( governance.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: GOVERNANCE ACCOUNT SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( supernode_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: SUPERNODE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      generate_blocks( TOTAL_PRODUCERS );

      supernode_update_operation supernode;
      
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

      const supernode_object& alice_supernode = db.get_supernode( account_name_type( "alice" ) );
      
      BOOST_REQUIRE( alice_supernode.account == "alice" );
      BOOST_REQUIRE( alice_supernode.daily_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.monthly_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: supernode creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode viewing process" );

      comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      comment_view_operation view;

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
      
      BOOST_REQUIRE( alice_supernode.account == "alice" );
      BOOST_REQUIRE( alice_supernode.daily_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_supernode.monthly_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_supernode.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: supernode viewing process" );

      BOOST_TEST_MESSAGE( "├── Passed: SUPERNODE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( interface_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: INTERFACE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: interface creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
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

      interface_update_operation interface;
      
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

      const interface_object& alice_interface = db.get_interface( account_name_type( "alice" ) );
      
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

      comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      comment_view_operation view;

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
      
      BOOST_REQUIRE( alice_interface.account == "alice" );
      BOOST_REQUIRE( alice_interface.daily_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_interface.monthly_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_interface.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: interface viewing process" );

      BOOST_TEST_MESSAGE( "├── Passed: INTERFACE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( mediator_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: MEDIATOR SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: mediator creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
      member.membership_type = "top";
      member.months = 1;
      member.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      mediator_update_operation mediator;
      
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

      const mediator_object& alice_mediator = db.get_mediator( account_name_type( "alice" ) );
      
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


BOOST_AUTO_TEST_CASE( enterprise_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMUNITY ENTERPRISE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)
      (natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(usain)(veronica)(will)(xerxes)(yan)(zach) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key );
      producer_vote( "bob", bob_private_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key );
      producer_vote( "candice", candice_private_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key );
      producer_vote( "dan", dan_private_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key );
      producer_vote( "elon", elon_private_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key );
      producer_vote( "fred", fred_private_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key );
      producer_vote( "george", george_private_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key );
      producer_vote( "haz", haz_private_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key );
      producer_vote( "isabelle", isabelle_private_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key );
      producer_vote( "jayme", jayme_private_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key );
      producer_vote( "kathryn", kathryn_private_owner_key );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "leonie", leonie_private_owner_key );
      producer_vote( "leonie", leonie_private_owner_key );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "margot", margot_private_owner_key );
      producer_vote( "margot", margot_private_owner_key );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "natalie", natalie_private_owner_key );
      producer_vote( "natalie", natalie_private_owner_key );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "olivia", olivia_private_owner_key );
      producer_vote( "olivia", olivia_private_owner_key );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "peter", peter_private_owner_key );
      producer_vote( "peter", peter_private_owner_key );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "quentin", quentin_private_owner_key );
      producer_vote( "quentin", quentin_private_owner_key );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "rachel", rachel_private_owner_key );
      producer_vote( "rachel", rachel_private_owner_key );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "sam", sam_private_owner_key );
      producer_vote( "sam", sam_private_owner_key );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "tim", tim_private_owner_key );
      producer_vote( "tim", tim_private_owner_key );

      fund_stake( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "usain", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "usain", usain_private_owner_key );
      producer_vote( "usain", usain_private_owner_key );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "veronica", veronica_private_owner_key );
      producer_vote( "veronica", veronica_private_owner_key );

      fund_stake( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "will", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "will", will_private_owner_key );
      producer_vote( "will", will_private_owner_key );

      fund_stake( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "xerxes", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "xerxes", xerxes_private_owner_key );
      producer_vote( "xerxes", xerxes_private_owner_key );

      fund_stake( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "yan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "yan", yan_private_owner_key );
      producer_vote( "yan", yan_private_owner_key );

      fund_stake( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "zach", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "zach", zach_private_owner_key );
      producer_vote( "zach", zach_private_owner_key );

      generate_blocks( TOTAL_PRODUCERS );

      enterprise_update_operation create;

      create.signatory = "alice";
      create.account = "alice";
      create.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      create.budget = asset( 1000* BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.details = "details";
      create.url = "https://www.url.com";
      create.json = "{ \"valid\": true }";
      create.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const enterprise_object& enterprise = db.get_enterprise( create.account, create.enterprise_id );

      BOOST_REQUIRE( enterprise.active );
      BOOST_REQUIRE( enterprise.account == create.account );
      BOOST_REQUIRE( to_string( enterprise.enterprise_id ) == create.enterprise_id );
      BOOST_REQUIRE( enterprise.budget == create.budget );
      BOOST_REQUIRE( to_string( enterprise.details ) == create.details );
      BOOST_REQUIRE( to_string( enterprise.url ) == create.url );
      BOOST_REQUIRE( to_string( enterprise.json ) == create.json );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal voting and funding approval" );

      enterprise_vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.account = "alice";
      vote.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      vote.vote_rank = 1;
      vote.approved = true;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "bob";
      vote.voter = "bob";

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "candice";
      vote.voter = "candice";

      tx.operations.push_back( vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "dan";
      vote.voter = "dan";

      tx.operations.push_back( vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "elon";
      vote.voter = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "fred";
      vote.voter = "fred";

      tx.operations.push_back( vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "george";
      vote.voter = "george";

      tx.operations.push_back( vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "haz";
      vote.voter = "haz";

      tx.operations.push_back( vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "isabelle";
      vote.voter = "isabelle";

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "jayme";
      vote.voter = "jayme";

      tx.operations.push_back( vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "kathryn";
      vote.voter = "kathryn";

      tx.operations.push_back( vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "leonie";
      vote.voter = "leonie";

      tx.operations.push_back( vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "margot";
      vote.voter = "margot";

      tx.operations.push_back( vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "natalie";
      vote.voter = "natalie";

      tx.operations.push_back( vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "olivia";
      vote.voter = "olivia";

      tx.operations.push_back( vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "peter";
      vote.voter = "peter";

      tx.operations.push_back( vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "quentin";
      vote.voter = "quentin";

      tx.operations.push_back( vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "rachel";
      vote.voter = "rachel";

      tx.operations.push_back( vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "sam";
      vote.voter = "sam";

      tx.operations.push_back( vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "tim";
      vote.voter = "tim";

      tx.operations.push_back( vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "usain";
      vote.voter = "usain";

      tx.operations.push_back( vote );
      tx.sign( usain_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "veronica";
      vote.voter = "veronica";

      tx.operations.push_back( vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "will";
      vote.voter = "will";

      generate_block();

      tx.operations.push_back( vote );
      tx.sign( will_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "xerxes";
      vote.voter = "xerxes";

      tx.operations.push_back( vote );
      tx.sign( xerxes_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "yan";
      vote.voter = "yan";

      tx.operations.push_back( vote );
      tx.sign( yan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.signatory = "zach";
      vote.voter = "zach";

      tx.operations.push_back( vote );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( zach_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      enterprise_fund_operation fund;

      fund.signatory = "bob";
      fund.funder = "bob";
      fund.account = "alice";
      fund.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      fund.amount = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      fund.validate();

      tx.operations.push_back( fund );
      tx.set_reference_block( db.head_block_id() );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.signatory = "candice";
      fund.funder = "candice";
      
      tx.operations.push_back( fund );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.signatory = "dan";
      fund.funder = "dan";
      
      tx.operations.push_back( fund );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.signatory = "elon";
      fund.funder = "elon";
      
      tx.operations.push_back( fund );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.signatory = "fred";
      fund.funder = "fred";
      
      tx.operations.push_back( fund );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( TOTAL_PRODUCERS );

      const enterprise_object& new_enterprise = db.get_enterprise( create.account, create.enterprise_id );

      BOOST_REQUIRE( new_enterprise.active );
      BOOST_REQUIRE( new_enterprise.total_funding == fund.amount * 5 );
      BOOST_REQUIRE( new_enterprise.approved );

      generate_blocks( ENTERPRISE_BLOCK_INTERVAL );

      BOOST_REQUIRE( db.get_enterprise( create.account, create.enterprise_id ).distributed.amount > 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal approval" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY ENTERPRISE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()