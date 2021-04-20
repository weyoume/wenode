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

BOOST_FIXTURE_TEST_SUITE( governance_operation_tests, clean_database_fixture );


   //==========================//
   // === Governance Tests === //
   //==========================//


BOOST_AUTO_TEST_CASE( governance_update_account_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: GOVERNANCE ACCOUNT SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)
         (natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(usain)(veronica)(will)(xerxes)(yan)(zach) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key );
      producer_vote( "bob", bob_private_owner_key );

      generate_blocks( 2 * BLOCKS_PER_HOUR );

      signed_transaction tx;

      account_membership_operation member;

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

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

      asset_options options;

      options.display_symbol = "GOVERN";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.buyback_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, "GOVACCOUNTCR") );
      options.validate();

      governance_create_operation governance_create;

      governance_create.founder = "alice";
      governance_create.new_governance_name = "govaccount";
      governance_create.new_governance_display_name = "GOVERNANCE ACCOUNT";
      governance_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      governance_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      governance_create.secure_public_key = string( alice_public_posting_key );
      governance_create.connection_public_key = string( alice_public_posting_key );
      governance_create.friend_public_key = string( alice_public_posting_key );
      governance_create.companion_public_key = string( alice_public_posting_key );
      governance_create.interface = INIT_ACCOUNT;
      governance_create.equity_asset = "GOVACCOUNTEQ";
      governance_create.equity_revenue_share = 5 * PERCENT_1;
      governance_create.equity_options = options;
      governance_create.credit_asset = "GOVACCOUNTCR";
      governance_create.credit_revenue_share = 5 * PERCENT_1;
      governance_create.credit_options = options;
      governance_create.public_community = "govaccount.discussion";
      governance_create.public_display_name = "Governance Discussion";
      governance_create.public_community_member_key = string( alice_public_posting_key );
      governance_create.public_community_moderator_key = string( alice_public_posting_key );
      governance_create.public_community_admin_key = string( alice_public_posting_key );
      governance_create.public_community_secure_key = string( alice_public_posting_key );
      governance_create.public_community_standard_premium_key = string( alice_public_posting_key );
      governance_create.public_community_mid_premium_key = string( alice_public_posting_key );
      governance_create.public_community_top_premium_key = string( alice_public_posting_key );
      governance_create.private_community = "govaccount.private";
      governance_create.private_display_name = "Governance Discussion";
      governance_create.private_community_member_key = string( alice_public_posting_key );
      governance_create.private_community_moderator_key = string( alice_public_posting_key );
      governance_create.private_community_admin_key = string( alice_public_posting_key );
      governance_create.private_community_secure_key = string( alice_public_posting_key );
      governance_create.private_community_standard_premium_key = string( alice_public_posting_key );
      governance_create.private_community_mid_premium_key = string( alice_public_posting_key );
      governance_create.private_community_top_premium_key = string( alice_public_posting_key );
      governance_create.reward_currency = SYMBOL_COIN;
      governance_create.standard_membership_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      governance_create.mid_membership_price = asset( 10*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      governance_create.top_membership_price = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      governance_create.coin_liquidity = asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      governance_create.usd_liquidity = asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_USD );
      governance_create.credit_liquidity = asset( 1000*BLOCKCHAIN_PRECISION, "GOVACCOUNTEQ" );
      governance_create.fee = asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      governance_create.delegation = asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      governance_create.validate();

      tx.operations.push_back( governance_create );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_object& governance = db.get_governance( account_name_type( "govaccount" ) );
      const account_object& governance_account = db.get_account( account_name_type( "govaccount" ) );
      
      BOOST_REQUIRE( governance.account == account_name_type( "govaccount" ) );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account membership request" );

      governance_member_request_operation governance_member_request;

      governance_member_request.account = "alice";
      governance_member_request.governance = "govaccount";
      governance_member_request.interface = INIT_ACCOUNT;
      governance_member_request.message = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, governance_account.secure_public_key, string( "Hello" ) );
      governance_member_request.active = true;
      governance_member_request.validate();

      tx.operations.push_back( governance_member_request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_member_request_object& gov_member_req = db.get_governance_member_request( account_name_type( "alice" ) );
      
      BOOST_REQUIRE( gov_member_req.governance == governance_member_request.governance );

      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account membership request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account membership approval" );

      governance_member_operation governance_member;

      governance_member.governance = "govaccount";
      governance_member.account = "alice";
      governance_member.interface = INIT_ACCOUNT;
      governance_member.approved = true;
      governance_member.validate();

      tx.operations.push_back( governance_member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_member_object& gov_member = db.get_governance_member( account_name_type( "alice" ) );
      
      BOOST_REQUIRE( gov_member.governance == governance_member.governance );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account membership approval" );

      BOOST_TEST_MESSAGE( "├── Passed: GOVERNANCE ACCOUNT SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()