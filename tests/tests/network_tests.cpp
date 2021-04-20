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

      vote.account = "candice";

      tx.operations.push_back( vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "dan";

      tx.operations.push_back( vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "fred";

      tx.operations.push_back( vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "george";

      tx.operations.push_back( vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "haz";

      tx.operations.push_back( vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "isabelle";

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "jayme";

      tx.operations.push_back( vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "kathryn";

      tx.operations.push_back( vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "leonie";

      tx.operations.push_back( vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "margot";

      tx.operations.push_back( vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "natalie";

      tx.operations.push_back( vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "olivia";

      tx.operations.push_back( vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "peter";

      tx.operations.push_back( vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "quentin";

      tx.operations.push_back( vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "rachel";

      tx.operations.push_back( vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "sam";

      tx.operations.push_back( vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "tim";

      tx.operations.push_back( vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "usain";

      tx.operations.push_back( vote );
      tx.sign( usain_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "veronica";

      tx.operations.push_back( vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "will";

      tx.operations.push_back( vote );
      tx.sign( will_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "xerxes";

      tx.operations.push_back( vote );
      tx.sign( xerxes_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.account = "yan";

      tx.operations.push_back( vote );
      tx.sign( yan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

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


BOOST_AUTO_TEST_CASE( supernode_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: SUPERNODE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      generate_blocks( TOTAL_PRODUCERS );

      supernode_update_operation supernode;
      
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

      vote.voter = "bob";

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "candice";

      tx.operations.push_back( vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "dan";

      tx.operations.push_back( vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "fred";

      tx.operations.push_back( vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "george";

      tx.operations.push_back( vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "haz";

      tx.operations.push_back( vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "isabelle";

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "jayme";

      tx.operations.push_back( vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "kathryn";

      tx.operations.push_back( vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "leonie";

      tx.operations.push_back( vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "margot";

      tx.operations.push_back( vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "natalie";

      tx.operations.push_back( vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "olivia";

      tx.operations.push_back( vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "peter";

      tx.operations.push_back( vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "quentin";

      tx.operations.push_back( vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "rachel";

      tx.operations.push_back( vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "sam";

      tx.operations.push_back( vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "tim";

      tx.operations.push_back( vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "usain";

      tx.operations.push_back( vote );
      tx.sign( usain_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "veronica";

      tx.operations.push_back( vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "will";

      generate_block();

      tx.operations.push_back( vote );
      tx.sign( will_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "xerxes";

      tx.operations.push_back( vote );
      tx.sign( xerxes_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "yan";

      tx.operations.push_back( vote );
      tx.sign( yan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      vote.voter = "zach";

      tx.operations.push_back( vote );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( zach_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      enterprise_fund_operation fund;

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

      fund.funder = "candice";
      
      tx.operations.push_back( fund );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.funder = "dan";
      
      tx.operations.push_back( fund );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.funder = "elon";
      
      tx.operations.push_back( fund );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.funder = "fred";
      
      tx.operations.push_back( fund );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( TOTAL_PRODUCERS );
      generate_block();

      const enterprise_object& new_enterprise = db.get_enterprise( create.account, create.enterprise_id );

      BOOST_REQUIRE( new_enterprise.active );
      BOOST_REQUIRE( new_enterprise.total_funding == fund.amount * 5 );
      BOOST_REQUIRE( new_enterprise.approved );

      generate_blocks( ENTERPRISE_BLOCK_INTERVAL );
      generate_block();

      BOOST_REQUIRE( db.get_enterprise( create.account, create.enterprise_id ).distributed.amount > 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal approval" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY ENTERPRISE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()