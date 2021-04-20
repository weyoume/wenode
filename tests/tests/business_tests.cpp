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

BOOST_FIXTURE_TEST_SUITE( business_operation_tests, clean_database_fixture );


   //========================//
   // === Business Tests === //
   //========================//


BOOST_AUTO_TEST_CASE( business_management_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: BUSINESS ACCOUNT MANAGEMENT SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create new Business Account" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( INIT_CEO, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( INIT_CEO, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      
      private_key_type init_ceo_private_active_key = get_private_key( INIT_CEO, ACTIVE_KEY_STR, INIT_PASSWORD );
      
      string init_account_private_business_wif = graphene::utilities::key_to_wif( get_private_key( INIT_ACCOUNT, "business", INIT_PASSWORD ) );

      generate_blocks( 2 * BLOCKS_PER_HOUR );

      signed_transaction tx;

      asset_options options;

      options.display_symbol = "BUSINESS";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.buyback_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, "BUSCR") );
      options.validate();

      business_create_operation business_create;

      business_create.founder = "alice";
      business_create.new_business_name = "aliceland";
      business_create.new_business_trading_name = "BUSINESS ACCOUNT";
      business_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      business_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      business_create.secure_public_key = string( alice_public_posting_key );
      business_create.connection_public_key = string( alice_public_posting_key );
      business_create.friend_public_key = string( alice_public_posting_key );
      business_create.companion_public_key = string( alice_public_posting_key );
      business_create.interface = INIT_ACCOUNT;
      business_create.equity_asset = "BUSEQ";
      business_create.equity_revenue_share = 5 * PERCENT_1;
      business_create.equity_options = options;
      business_create.credit_asset = "BUSCR";
      business_create.credit_revenue_share = 5 * PERCENT_1;
      business_create.credit_options = options;
      business_create.public_community = "aliceland.discussion";
      business_create.public_display_name = "Business Discussion";
      business_create.public_community_member_key = string( alice_public_posting_key );
      business_create.public_community_moderator_key = string( alice_public_posting_key );
      business_create.public_community_admin_key = string( alice_public_posting_key );
      business_create.public_community_secure_key = string( alice_public_posting_key );
      business_create.public_community_standard_premium_key = string( alice_public_posting_key );
      business_create.public_community_mid_premium_key = string( alice_public_posting_key );
      business_create.public_community_top_premium_key = string( alice_public_posting_key );
      business_create.private_community = "aliceland.private";
      business_create.private_display_name = "Business Discussion";
      business_create.private_community_member_key = string( alice_public_posting_key );
      business_create.private_community_moderator_key = string( alice_public_posting_key );
      business_create.private_community_admin_key = string( alice_public_posting_key );
      business_create.private_community_secure_key = string( alice_public_posting_key );
      business_create.private_community_standard_premium_key = string( alice_public_posting_key );
      business_create.private_community_mid_premium_key = string( alice_public_posting_key );
      business_create.private_community_top_premium_key = string( alice_public_posting_key );
      business_create.reward_currency = SYMBOL_COIN;
      business_create.standard_membership_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      business_create.mid_membership_price = asset( 10*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      business_create.top_membership_price = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      business_create.coin_liquidity = asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      business_create.usd_liquidity = asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_USD );
      business_create.credit_liquidity = asset( 1000*BLOCKCHAIN_PRECISION, "BUSEQ" );
      business_create.fee = asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      business_create.delegation = asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      business_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( business_create );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create new Business Account" );

      /** BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no equity voting power" );

      stake_asset_operation stake;

      stake.from = INIT_CEO;
      stake.to = INIT_CEO;
      stake.amount = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY );
      stake.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( stake );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL - BLOCK_INTERVAL, true );    // enable equity to stake
      generate_blocks( 10 );

      business_director_vote_operation business_director_vote;

      business_director_vote.account = "alice";
      business_director_vote.business = INIT_BUSINESS;
      business_director_vote.director = "alice";
      business_director_vote.vote_rank = 1;
      business_director_vote.approved = true;
      business_director_vote.validate();

      tx.operations.push_back( business_director_vote );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const business_director_vote_object& director_vote = db.get_business_director_vote( INIT_CEO, INIT_ACCOUNT, account_name_type( "alice" ) );

      BOOST_REQUIRE( director_vote.account == business_director_vote.account );
      BOOST_REQUIRE( director_vote.business == business_director_vote.business );
      BOOST_REQUIRE( director_vote.director == business_director_vote.director );
      BOOST_REQUIRE( director_vote.vote_rank == 1 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote officer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no equity voting power" );

      business_director_vote.account = "bob";

      tx.operations.push_back( business_director_vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no equity voting power" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account vote executive" );

      business_executive_vote_operation vote_executive;

      vote_executive.director = INIT_CEO;
      vote_executive.business = INIT_ACCOUNT;
      vote_executive.executive = "alice";
      vote_executive.approved = true;
      vote_executive.validate();

      tx.operations.push_back( vote_executive );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const business_executive_vote_object& exec_vote = db.get_business_executive_vote( INIT_CEO, INIT_ACCOUNT );

      BOOST_REQUIRE( exec_vote.director == vote_executive.director );
      BOOST_REQUIRE( exec_vote.business == vote_executive.business );
      BOOST_REQUIRE( exec_vote.executive == vote_executive.executive );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote executive" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when not a business member" );

      vote_executive.director = "bob";

      tx.operations.push_back( vote_executive );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when not a business member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel executive vote" );

      vote_executive.director = INIT_CEO;
      vote_executive.approved = false;
      vote_executive.validate();

      tx.operations.push_back( vote_executive );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const business_executive_vote_object* executive_vote_ptr = db.find_business_executive_vote( INIT_CEO, INIT_ACCOUNT );
      BOOST_REQUIRE( executive_vote_ptr == nullptr );
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel executive vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel officer vote" );
      
      business_director_vote.account = INIT_CEO;
      business_director_vote.business = INIT_ACCOUNT;
      business_director_vote.director = "alice";
      business_director_vote.vote_rank = 1;
      business_director_vote.approved = false;
      business_director_vote.validate();

      tx.operations.push_back( business_director_vote );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const business_director_vote_object* officer_vote_ptr = db.find_business_director_vote( INIT_CEO, INIT_ACCOUNT, account_name_type( "alice" ) );
      BOOST_REQUIRE( officer_vote_ptr == nullptr );
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel officer vote" );

      BOOST_TEST_MESSAGE( "├── Passed: BUSINESS ACCOUNT MANAGEMENT SEQUENCE" );
      **/
   }
   FC_LOG_AND_RETHROW()
} 


BOOST_AUTO_TEST_SUITE_END()