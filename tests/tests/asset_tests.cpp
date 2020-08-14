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

BOOST_FIXTURE_TEST_SUITE( asset_operation_tests, clean_database_fixture );



   //=====================//
   // === Asset Tests === //
   //=====================//



BOOST_AUTO_TEST_CASE( standard_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: STANDARD ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create standard asset" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );
      generate_blocks( TOTAL_PRODUCERS );

      asset_create_operation asset_create;

      asset_create.signatory = "alice";
      asset_create.issuer = "alice";
      asset_create.symbol = "ALICECOIN";
      asset_create.asset_type = "standard";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "ALICECOIN" );

      asset_options options;

      options.display_symbol = "Alice Coin";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.validate();
      asset_create.options = options;
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& alice_asset = db.get_asset( asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( alice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( alice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( alice_asset.asset_type == asset_property_type::STANDARD_ASSET );
      
      BOOST_REQUIRE( asset_create.options.display_symbol == to_string( alice_asset.display_symbol ) );
      BOOST_REQUIRE( asset_create.options.details == to_string( alice_asset.details ) );
      BOOST_REQUIRE( asset_create.options.json == to_string( alice_asset.json ) );
      BOOST_REQUIRE( asset_create.options.url == to_string( alice_asset.url ) );
      BOOST_REQUIRE( alice_asset.created == now() );
      BOOST_REQUIRE( alice_asset.last_updated == now() );

      const asset_credit_pool_object& alice_credit_pool = db.get_credit_pool( asset_symbol_type( "ALICECOIN" ), false );

      BOOST_REQUIRE( alice_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( alice_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& alice_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( alice_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( alice_liquidity_pool.symbol_b == asset_symbol_type( "ALICECOIN" ) );
      BOOST_REQUIRE( alice_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( alice_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, asset_symbol_type( "ALICECOIN" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update standard asset" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      asset_update_operation asset_update;

      asset_update.signatory = "alice";
      asset_update.issuer = "alice";
      asset_update.asset_to_update = "ALICECOIN";

      asset_options new_options;

      new_options.display_symbol = "ALICE";
      new_options.details = "New Details";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";

      asset_update.new_options = new_options;
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& new_alice_asset = db.get_asset( asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( new_alice_asset.last_updated == now() );
      BOOST_REQUIRE( asset_update.new_options.display_symbol == to_string( new_alice_asset.display_symbol ) );
      BOOST_REQUIRE( asset_update.new_options.details == to_string( new_alice_asset.details ) );
      BOOST_REQUIRE( asset_update.new_options.json == to_string( new_alice_asset.json ) );
      BOOST_REQUIRE( asset_update.new_options.url == to_string( new_alice_asset.url ) );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Standard asset" );

      asset_issue_operation asset_issue;

      asset_issue.signatory = "alice";
      asset_issue.issuer = "alice";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "ALICECOIN" ) );
      asset_issue.issue_to_account = "alice";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_issue );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid = get_liquid_balance( "alice", "ALICECOIN" );

      BOOST_REQUIRE( alice_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "ALICECOIN" ) ).get_total_supply() == asset_issue.asset_to_issue + asset_create.credit_liquidity * 3 );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "ALICECOIN" ) ).get_liquid_supply() == asset_issue.asset_to_issue );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Standard asset" );

      asset_reserve_operation asset_reserve;

      asset_reserve.signatory = "alice";
      asset_reserve.payer = "alice";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, asset_symbol_type( "ALICECOIN" ) );
      asset_reserve.validate();

      tx.operations.push_back( asset_reserve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_liquid = get_liquid_balance( "alice", "ALICECOIN" );

      BOOST_REQUIRE( alice_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "ALICECOIN" ) ).get_total_supply() == asset_issue.asset_to_issue + asset_create.credit_liquidity * 3 - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "ALICECOIN" ) ).get_liquid_supply() == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create Asset distribution" );

      asset_distribution_operation distribution;

      distribution.signatory = "alice";
      distribution.issuer = "alice";
      distribution.distribution_asset = "ALICECOIN";
      distribution.fund_asset = SYMBOL_USD;
      distribution.details = "Details";
      distribution.url = "https://www.url.com";
      distribution.json = "{ \"valid\": true }";
      distribution.distribution_rounds = 4;
      distribution.distribution_interval_days = 7;
      distribution.max_intervals_missed = 4;
      distribution.min_input_fund_units = 1;
      distribution.max_input_fund_units = 1000000;

      asset_unit input = asset_unit( account_name_type( "alice" ), BLOCKCHAIN_PRECISION, string( "liquid" ), GENESIS_TIME );
      distribution.input_fund_unit = { input };

      asset_unit output = asset_unit( account_name_type( "sender" ), 1, string( "liquid" ), GENESIS_TIME );
      distribution.output_distribution_unit = { output };

      distribution.min_unit_ratio = 1000000;
      distribution.max_unit_ratio = 1000000000000;
      distribution.min_input_balance_units = 1;
      distribution.max_input_balance_units = 1000000;
      distribution.begin_time = now() + fc::days(31);
      distribution.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( distribution );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_distribution_object& alice_distribution = db.get_asset_distribution( asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( distribution.distribution_asset == alice_distribution.distribution_asset );
      BOOST_REQUIRE( distribution.fund_asset == alice_distribution.fund_asset );
      BOOST_REQUIRE( distribution.details == to_string( alice_distribution.details ) );
      BOOST_REQUIRE( distribution.url == to_string( alice_distribution.url ) );
      BOOST_REQUIRE( distribution.json == to_string( alice_distribution.json ) );
      BOOST_REQUIRE( distribution.distribution_rounds == alice_distribution.distribution_rounds );
      BOOST_REQUIRE( distribution.distribution_interval_days == alice_distribution.distribution_interval_days );
      BOOST_REQUIRE( distribution.max_intervals_missed == alice_distribution.max_intervals_missed );
      BOOST_REQUIRE( distribution.min_input_fund_units == alice_distribution.min_input_fund_units );
      BOOST_REQUIRE( distribution.max_input_fund_units == alice_distribution.max_input_fund_units );
      BOOST_REQUIRE( *distribution.input_fund_unit.begin() == *alice_distribution.input_fund_unit.begin() );
      BOOST_REQUIRE( *distribution.output_distribution_unit.begin() == *alice_distribution.output_distribution_unit.begin() );
      BOOST_REQUIRE( distribution.min_unit_ratio == alice_distribution.min_unit_ratio );
      BOOST_REQUIRE( distribution.max_unit_ratio == alice_distribution.max_unit_ratio );
      BOOST_REQUIRE( distribution.min_input_balance_units == alice_distribution.min_input_balance_units );
      BOOST_REQUIRE( distribution.max_input_balance_units == alice_distribution.max_input_balance_units );
      BOOST_REQUIRE( distribution.begin_time == alice_distribution.begin_time );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create Asset distribution" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Fund Asset Distribution" );

      generate_blocks( db.get_asset_distribution( asset_symbol_type( "ALICECOIN" ) ).begin_time, true );
      generate_blocks( TOTAL_PRODUCERS );

      asset_distribution_fund_operation fund;

      fund.signatory = "bob";
      fund.sender = "bob";
      fund.distribution_asset = "ALICECOIN";
      fund.amount = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      fund.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( fund );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_distribution_balance_object& bob_distribution_balance = db.get_asset_distribution_balance( account_name_type( "bob" ), asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( bob_distribution_balance.sender == fund.sender );
      BOOST_REQUIRE( bob_distribution_balance.distribution_asset == fund.distribution_asset );
      BOOST_REQUIRE( bob_distribution_balance.amount == fund.amount );
      BOOST_REQUIRE( bob_distribution_balance.last_updated == now() );
      BOOST_REQUIRE( bob_distribution_balance.created == now() );

      generate_block();

      fund.signatory = "candice";
      fund.sender = "candice";
      fund.amount = asset( 200 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( fund );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_distribution_balance_object& candice_distribution_balance = db.get_asset_distribution_balance( account_name_type( "candice" ), asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( candice_distribution_balance.sender == fund.sender );
      BOOST_REQUIRE( candice_distribution_balance.distribution_asset == fund.distribution_asset );
      BOOST_REQUIRE( candice_distribution_balance.amount == fund.amount );
      BOOST_REQUIRE( candice_distribution_balance.last_updated == now() );
      BOOST_REQUIRE( candice_distribution_balance.created == now() );

      generate_block();

      fund.signatory = "dan";
      fund.sender = "dan";
      fund.amount = asset( 300 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( fund );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_distribution_balance_object& dan_distribution_balance = db.get_asset_distribution_balance( account_name_type( "dan" ), asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( dan_distribution_balance.sender == fund.sender );
      BOOST_REQUIRE( dan_distribution_balance.distribution_asset == fund.distribution_asset );
      BOOST_REQUIRE( dan_distribution_balance.amount == fund.amount );
      BOOST_REQUIRE( dan_distribution_balance.last_updated == now() );
      BOOST_REQUIRE( dan_distribution_balance.created == now() );

      generate_block();

      fund.signatory = "elon";
      fund.sender = "elon";
      fund.amount = asset( 400 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( fund );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_distribution_balance_object& elon_distribution_balance = db.get_asset_distribution_balance( account_name_type( "elon" ), asset_symbol_type( "ALICECOIN" ) );

      BOOST_REQUIRE( elon_distribution_balance.sender == fund.sender );
      BOOST_REQUIRE( elon_distribution_balance.distribution_asset == fund.distribution_asset );
      BOOST_REQUIRE( elon_distribution_balance.amount == fund.amount );
      BOOST_REQUIRE( elon_distribution_balance.last_updated == now() );
      BOOST_REQUIRE( elon_distribution_balance.created == now() );

      generate_blocks( db.get_asset_distribution( asset_symbol_type( "ALICECOIN" ) ).next_round_time, true );
      generate_block();

      asset bob_alicecoin_balance = get_liquid_balance( "bob", "ALICECOIN" );
      asset candice_alicecoin_balance = get_liquid_balance( "candice", "ALICECOIN" );
      asset dan_alicecoin_balance = get_liquid_balance( "dan", "ALICECOIN" );
      asset elon_alicecoin_balance = get_liquid_balance( "elon", "ALICECOIN" );

      BOOST_REQUIRE( db.get_asset_distribution( asset_symbol_type( "ALICECOIN" ) ).intervals_paid == 1 );
      BOOST_REQUIRE( db.get_asset_distribution( asset_symbol_type( "ALICECOIN" ) ).next_round_time == distribution.begin_time + fc::days( distribution.distribution_interval_days * 2 ) );
      BOOST_REQUIRE( db.get_asset_distribution( asset_symbol_type( "ALICECOIN" ) ).total_funded == asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      BOOST_REQUIRE( db.get_asset_distribution( asset_symbol_type( "ALICECOIN" ) ).total_distributed == bob_alicecoin_balance + candice_alicecoin_balance + dan_alicecoin_balance + elon_alicecoin_balance );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Fund Asset distribution" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update Asset issuer" );

      asset_update_issuer_operation asset_update_issuer;

      asset_update_issuer.signatory = "alice";
      asset_update_issuer.issuer = "alice";
      asset_update_issuer.asset_to_update = "ALICECOIN";
      asset_update_issuer.new_issuer = "bob";
      asset_update_issuer.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_update_issuer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_REQUIRE( db.get_asset( asset_symbol_type( "ALICECOIN" ) ).issuer == asset_update_issuer.new_issuer );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update Asset issuer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: New Issuer issuing asset to own account" );

      asset init_bob_liquid = get_liquid_balance( "bob", "ALICECOIN" );

      asset_issue.signatory = "bob";
      asset_issue.issuer = "bob";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "ALICECOIN" ) );
      asset_issue.issue_to_account = "bob";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      asset bob_liquid = get_liquid_balance( "bob", "ALICECOIN" );      

      BOOST_REQUIRE( bob_liquid == init_bob_liquid + asset_issue.asset_to_issue );

      BOOST_TEST_MESSAGE( "│   ├── Passed: New Issuer issuing asset to own account" );

      BOOST_TEST_MESSAGE( "├── Testing: STANDARD ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( currency_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CURRENCY ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create currency asset" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );

      asset_create_operation asset_create;

      asset_create.signatory = "bob";
      asset_create.issuer = "bob";
      asset_create.symbol = "BOBCOIN";
      asset_create.asset_type = "currency";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "BOBCOIN" );

      asset_options options;

      options.display_symbol = "Bob Coin";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.block_reward = asset( 25 * BLOCKCHAIN_PRECISION, "BOBCOIN" );
      options.validate();

      asset_create.options = options;
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& bob_asset = db.get_asset( asset_symbol_type( "BOBCOIN" ) );

      BOOST_REQUIRE( bob_asset.issuer == NULL_ACCOUNT );
      BOOST_REQUIRE( bob_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( bob_asset.asset_type == asset_property_type::CURRENCY_ASSET );
      BOOST_REQUIRE( bob_asset.created == now() );
      BOOST_REQUIRE( bob_asset.last_updated == now() );

      const asset_credit_pool_object& bob_credit_pool = db.get_credit_pool( asset_symbol_type( "BOBCOIN" ), false );

      BOOST_REQUIRE( bob_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( bob_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& bob_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, asset_symbol_type( "BOBCOIN" ) );

      BOOST_REQUIRE( bob_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( bob_liquidity_pool.symbol_b == asset_symbol_type( "BOBCOIN" ) );
      BOOST_REQUIRE( bob_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( bob_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, asset_symbol_type( "BOBCOIN" ) ) );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create currency asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when attempting to update currency asset" );

      asset_update_operation asset_update;

      asset_update.signatory = "bob";
      asset_update.issuer = "bob";
      asset_update.asset_to_update = "BOBCOIN";

      asset_options new_options;

      new_options.display_symbol = "BOB";
      new_options.details = "New Details";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";
      new_options.block_reward = asset( 25 * BLOCKCHAIN_PRECISION, "BOBCOIN" );
      new_options.validate();
      
      asset_update.new_options = new_options;
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );    // Bob is not issuer, so cannot update

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      /**
       * @todo: Currency issuance values, and correct redemption of amounts for all sources
       */

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when attempting to update currency asset" );

      BOOST_TEST_MESSAGE( "├── Testing: CURRENCY ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( equity_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: EQUITY ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create equity asset" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );

      account_create_operation account_create;

      account_create.signatory = "candice";
      account_create.registrar = "candice";
      account_create.new_account_name = "tropico";
      account_create.referrer = INIT_ACCOUNT;
      account_create.proxy = INIT_ACCOUNT;
      account_create.recovery_account = INIT_ACCOUNT;
      account_create.reset_account = INIT_ACCOUNT;
      account_create.details = "My Details";
      account_create.url = "https://www.url.com";
      account_create.json = "{ \"valid\": true }";
      account_create.json_private = "{ \"valid\": true }";
      account_create.owner_auth = authority( 1, candice_public_owner_key, 1 );
      account_create.active_auth = authority( 1, candice_public_active_key, 1 );
      account_create.posting_auth = authority( 1, candice_public_posting_key, 1 );
      account_create.secure_public_key = string( candice_public_secure_key );
      account_create.connection_public_key = string( candice_public_connection_key );
      account_create.friend_public_key = string( candice_public_friend_key );
      account_create.companion_public_key = string( candice_public_friend_key );
      account_create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      account_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( account_create );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund_liquid( "tropico", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "tropico", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "tropico", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      generate_block();

      account_business_operation account_business;

      account_business.signatory = "tropico";
      account_business.account = "tropico";
      account_business.init_ceo_account = "candice";
      account_business.business_type = "open";
      account_business.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      account_business.business_public_key = string( candice_public_connection_key );
      account_business.active = true;
      account_business.validate();

      tx.operations.push_back( account_business );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::days(2), true );
      generate_block();

      const account_business_object& tropico_business = db.get_account_business( account_name_type( "tropico" ) );

      BOOST_REQUIRE( tropico_business.account == account_business.account );
      BOOST_REQUIRE( tropico_business.executive_board.CHIEF_EXECUTIVE_OFFICER == account_business.init_ceo_account );
      BOOST_REQUIRE( tropico_business.business_type == business_structure_type::OPEN_BUSINESS );

      asset_create_operation asset_create;
      
      asset_create.signatory = "tropico";
      asset_create.issuer = "tropico";
      asset_create.symbol = "TROPICO";
      asset_create.asset_type = "equity";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "TROPICO" ) );

      asset_options options;

      options.display_symbol = "TRO";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.validate();

      asset_create.options = options;
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& candice_asset = db.get_asset( asset_symbol_type( "TROPICO" ) );

      BOOST_REQUIRE( candice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( candice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( candice_asset.asset_type == asset_property_type::EQUITY_ASSET );
      BOOST_REQUIRE( candice_asset.created == now() );
      BOOST_REQUIRE( candice_asset.last_updated == now() );

      const asset_equity_data_object& candice_equity = db.get_equity_data( asset_symbol_type( "TROPICO" ) );

      BOOST_REQUIRE( candice_equity.business_account == asset_create.issuer );

      const asset_credit_pool_object& candice_credit_pool = db.get_credit_pool( asset_symbol_type( "TROPICO" ), false );

      BOOST_REQUIRE( candice_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( candice_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& candice_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, asset_symbol_type( "TROPICO" ) );

      BOOST_REQUIRE( candice_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( candice_liquidity_pool.symbol_b == asset_symbol_type( "TROPICO" ) );
      BOOST_REQUIRE( candice_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( candice_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, asset_symbol_type( "TROPICO" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update equity asset" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      asset_update_operation asset_update;

      asset_update.signatory = "tropico";
      asset_update.issuer = "tropico";
      asset_update.asset_to_update = "TROPICO";

      asset_options new_options;

      new_options.display_symbol = "Tropico Equity";
      new_options.details = "New Details";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";
      new_options.dividend_share_percent = 10 * PERCENT_1;
      new_options.liquid_dividend_percent = 20 * PERCENT_1;
      new_options.savings_dividend_percent = 20 * PERCENT_1;
      new_options.staked_dividend_percent = 60 * PERCENT_1;

      new_options.validate();
      asset_update.new_options = new_options;
      asset_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& new_candice_asset = db.get_asset( asset_symbol_type( "TROPICO" ) );

      BOOST_REQUIRE( new_candice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( new_candice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( new_candice_asset.asset_type == asset_property_type::EQUITY_ASSET );
      BOOST_REQUIRE( to_string( new_candice_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( new_candice_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( new_candice_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( new_candice_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( new_candice_asset.last_updated == now() );

      const asset_equity_data_object& new_candice_equity = db.get_equity_data( asset_symbol_type( "TROPICO" ) );

      BOOST_REQUIRE( new_candice_equity.dividend_share_percent == asset_update.new_options.dividend_share_percent );
      BOOST_REQUIRE( new_candice_equity.liquid_dividend_percent == asset_update.new_options.liquid_dividend_percent );
      BOOST_REQUIRE( new_candice_equity.savings_dividend_percent == asset_update.new_options.savings_dividend_percent );
      BOOST_REQUIRE( new_candice_equity.staked_dividend_percent == asset_update.new_options.staked_dividend_percent );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Equity asset" );

      asset_issue_operation asset_issue;

      asset_issue.signatory = "tropico";
      asset_issue.issuer = "tropico";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "TROPICO" ) );
      asset_issue.issue_to_account = "candice";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset candice_liquid = get_liquid_balance( "candice", "TROPICO" );

      BOOST_REQUIRE( candice_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "TROPICO" ) ).get_total_supply() == asset_issue.asset_to_issue + asset_create.credit_liquidity * 3 );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "TROPICO" ) ).get_liquid_supply() == asset_issue.asset_to_issue );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Equity asset" );

      asset_reserve_operation asset_reserve;

      asset_reserve.signatory = "candice";
      asset_reserve.payer = "candice";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, asset_symbol_type( "TROPICO" ) );

      tx.operations.push_back( asset_reserve );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      candice_liquid = get_liquid_balance( "candice", "TROPICO" );

      BOOST_REQUIRE( candice_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "TROPICO" ) ).get_total_supply() == asset_issue.asset_to_issue + asset_create.credit_liquidity * 3 - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "TROPICO" ) ).get_liquid_supply() == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Equity asset" );

      BOOST_TEST_MESSAGE( "├── Testing: EQUITY ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( bond_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: BOND ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create bond asset" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );

      account_create_operation account_create;

      account_create.signatory = "dan";
      account_create.registrar = "dan";
      account_create.new_account_name = "blocktwo";
      account_create.referrer = INIT_ACCOUNT;
      account_create.proxy = INIT_ACCOUNT;
      account_create.recovery_account = INIT_ACCOUNT;
      account_create.reset_account = INIT_ACCOUNT;
      account_create.details = "My Details";
      account_create.url = "https://www.url.com";
      account_create.json = "{ \"valid\": true }";
      account_create.json_private = "{ \"valid\": true }";
      account_create.owner_auth = authority( 1, dan_private_owner_key.get_public_key(), 1 );
      account_create.active_auth = authority( 1, dan_private_active_key.get_public_key(), 1 );
      account_create.posting_auth = authority( 1, dan_private_posting_key.get_public_key(), 1 );
      account_create.secure_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.connection_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.friend_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.companion_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      account_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( account_create );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund_liquid( "blocktwo", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "blocktwo", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "blocktwo", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      generate_block();

      account_business_operation account_business;

      account_business.signatory = "blocktwo";
      account_business.account = "blocktwo";
      account_business.init_ceo_account = "dan";
      account_business.business_type = "open";
      account_business.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      account_business.business_public_key = string( dan_public_connection_key );
      account_business.active = true;
      account_business.validate();

      tx.operations.push_back( account_business );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_business_object& blocktwo_business = db.get_account_business( account_name_type( "blocktwo" ) );

      BOOST_REQUIRE( blocktwo_business.account == account_business.account );
      BOOST_REQUIRE( blocktwo_business.executive_board.CHIEF_EXECUTIVE_OFFICER == account_business.init_ceo_account );
      BOOST_REQUIRE( blocktwo_business.business_type == business_structure_type::OPEN_BUSINESS );

      generate_blocks( now() + fc::days(2), true );
      generate_block();

      date_type maturity = date_type( now() + fc::days(180) );

      asset_create_operation asset_create;

      asset_create.signatory = "blocktwo";
      asset_create.issuer = "blocktwo";
      asset_create.symbol = "BLOCKBOND";
      asset_create.asset_type = "bond";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "BLOCKBOND" ) );

      asset_options options;

      options.display_symbol = "Block Two Bonds";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.value = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      options.collateralization = 20 * PERCENT_1;
      options.coupon_rate_percent = 5 * PERCENT_1;
      options.maturity_date = date_type( 1, maturity.month, maturity.year );

      asset_create.options = options;
      options.validate();
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& dan_bond_asset = db.get_asset( asset_symbol_type( "BLOCKBOND" ) );

      BOOST_REQUIRE( dan_bond_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_bond_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_bond_asset.asset_type == asset_property_type::BOND_ASSET );
      BOOST_REQUIRE( dan_bond_asset.created == now() );
      BOOST_REQUIRE( dan_bond_asset.last_updated == now() );

      const asset_bond_data_object& bond_data = db.get_bond_data( asset_symbol_type( "BLOCKBOND" ) );

      BOOST_REQUIRE( bond_data.business_account == asset_create.issuer );
      BOOST_REQUIRE( bond_data.value == asset_create.options.value );
      BOOST_REQUIRE( bond_data.collateralization == asset_create.options.collateralization );
      BOOST_REQUIRE( bond_data.coupon_rate_percent == asset_create.options.coupon_rate_percent );
      BOOST_REQUIRE( bond_data.maturity_date == asset_create.options.maturity_date );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create bond asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update bond asset" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      asset_update_operation asset_update;

      asset_update.signatory = "blocktwo";
      asset_update.issuer = "blocktwo";
      asset_update.asset_to_update = "BLOCKBOND";

      asset_options new_options;

      new_options.display_symbol = "Block Two Bonds";
      new_options.details = "New Details";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";
      new_options.value = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      new_options.collateralization = 25 * PERCENT_1;
      new_options.coupon_rate_percent = 6 * PERCENT_1;
      new_options.maturity_date = date_type( 1, maturity.month, maturity.year );

      new_options.validate();
      asset_update.new_options = new_options;
      asset_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& new_dan_bond_asset = db.get_asset( asset_symbol_type( "BLOCKBOND" ) );

      BOOST_REQUIRE( new_dan_bond_asset.issuer == asset_update.issuer );
      BOOST_REQUIRE( new_dan_bond_asset.symbol == asset_update.asset_to_update );
      BOOST_REQUIRE( new_dan_bond_asset.asset_type == asset_property_type::BOND_ASSET );
      BOOST_REQUIRE( to_string( new_dan_bond_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( new_dan_bond_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( new_dan_bond_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( new_dan_bond_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( new_dan_bond_asset.last_updated == now() );

      const asset_bond_data_object& new_bond_data = db.get_bond_data( asset_symbol_type( "BLOCKBOND" ) );

      BOOST_REQUIRE( new_bond_data.business_account == asset_update.issuer );
      BOOST_REQUIRE( new_bond_data.value == asset_update.new_options.value );
      BOOST_REQUIRE( new_bond_data.collateralization == asset_update.new_options.collateralization );
      BOOST_REQUIRE( new_bond_data.coupon_rate_percent == asset_update.new_options.coupon_rate_percent );
      BOOST_REQUIRE( new_bond_data.maturity_date == asset_update.new_options.maturity_date );

      generate_blocks( now() + fc::days(2), true );
      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update bond asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Bond asset" );

      asset_issue_operation asset_issue;

      asset_issue.signatory = "blocktwo";
      asset_issue.issuer = "blocktwo";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "BLOCKBOND" ) );
      asset_issue.issue_to_account = "dan";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_issue );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset dan_liquid = get_liquid_balance( "dan", "BLOCKBOND" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKBOND" ) ).get_total_supply() == asset_issue.asset_to_issue );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKBOND" ) ).get_liquid_supply() == asset_issue.asset_to_issue );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Bond asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Bond asset" );

      asset_reserve_operation asset_reserve;

      asset_reserve.signatory = "dan";
      asset_reserve.payer = "dan";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, asset_symbol_type( "BLOCKBOND" ) );

      tx.operations.push_back( asset_reserve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      dan_liquid = get_liquid_balance( "dan", "BLOCKBOND" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKBOND" ) ).get_total_supply() == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKBOND" ) ).get_liquid_supply() == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Bond asset" );

      BOOST_TEST_MESSAGE( "├── Testing: BOND ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( credit_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CREDIT ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create credit asset" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );

      account_create_operation account_create;

      account_create.signatory = "dan";
      account_create.registrar = "dan";
      account_create.new_account_name = "blocktwo";
      account_create.referrer = INIT_ACCOUNT;
      account_create.proxy = INIT_ACCOUNT;
      account_create.recovery_account = INIT_ACCOUNT;
      account_create.reset_account = INIT_ACCOUNT;
      account_create.details = "My Details";
      account_create.url = "https://www.url.com";
      account_create.json = "{ \"valid\": true }";
      account_create.json_private = "{ \"valid\": true }";
      account_create.owner_auth = authority( 1, dan_private_owner_key.get_public_key(), 1 );
      account_create.active_auth = authority( 1, dan_private_active_key.get_public_key(), 1 );
      account_create.posting_auth = authority( 1, dan_private_posting_key.get_public_key(), 1 );
      account_create.secure_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.connection_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.friend_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.companion_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      account_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( account_create );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund_liquid( "blocktwo", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "blocktwo", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "blocktwo", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      generate_block();

      account_business_operation account_business;

      account_business.signatory = "blocktwo";
      account_business.account = "blocktwo";
      account_business.init_ceo_account = "dan";
      account_business.business_type = "open";
      account_business.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      account_business.business_public_key = string( dan_public_connection_key );
      account_business.active = true;
      account_business.validate();

      tx.operations.push_back( account_business );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_business_object& blocktwo_business = db.get_account_business( account_name_type( "blocktwo" ) );

      BOOST_REQUIRE( blocktwo_business.account == account_business.account );
      BOOST_REQUIRE( blocktwo_business.executive_board.CHIEF_EXECUTIVE_OFFICER == account_business.init_ceo_account );
      BOOST_REQUIRE( blocktwo_business.business_type == business_structure_type::OPEN_BUSINESS );

      generate_blocks( now() + fc::days(2), true );
      generate_block();

      asset_create_operation asset_create;

      asset_create.signatory = "blocktwo";
      asset_create.issuer = "blocktwo";
      asset_create.symbol = "BLOCKTWO";
      asset_create.asset_type = "credit";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "BLOCKTWO" ) );

      asset_options options;

      options.display_symbol = "Block Two";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.buyback_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, "BLOCKTWO") );

      asset_create.options = options;
      options.validate();
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& dan_credit_asset = db.get_asset( asset_symbol_type( "BLOCKTWO" ) );

      BOOST_REQUIRE( dan_credit_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_credit_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_credit_asset.asset_type == asset_property_type::CREDIT_ASSET );
      BOOST_REQUIRE( dan_credit_asset.created == now() );
      BOOST_REQUIRE( dan_credit_asset.last_updated == now() );

      const asset_credit_data_object& dan_credit = db.get_credit_data( asset_symbol_type( "BLOCKTWO" ) );

      BOOST_REQUIRE( dan_credit.business_account == asset_create.issuer );
      BOOST_REQUIRE( dan_credit.buyback_asset == SYMBOL_USD );
      BOOST_REQUIRE( dan_credit.buyback_pool.amount == 0 );

      const asset_credit_pool_object& dan_credit_pool = db.get_credit_pool( asset_symbol_type( "BLOCKTWO" ), false );

      BOOST_REQUIRE( dan_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( dan_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& dan_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, asset_symbol_type( "BLOCKTWO" ) );

      BOOST_REQUIRE( dan_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( dan_liquidity_pool.symbol_b == asset_symbol_type( "BLOCKTWO" ) );
      BOOST_REQUIRE( dan_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( dan_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, asset_symbol_type( "BLOCKTWO" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update credit asset" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      asset_update_operation asset_update;

      asset_update.signatory = "blocktwo";
      asset_update.issuer = "blocktwo";
      asset_update.asset_to_update = "BLOCKTWO";

      asset_options new_options;

      new_options.display_symbol = "Block Two Equity";
      new_options.details = "New Details";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";
      new_options.buyback_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "BLOCKTWO" ) ) );
      new_options.buyback_share_percent = 10 * PERCENT_1;
      new_options.liquid_fixed_interest_rate = 2 * PERCENT_1;
      new_options.liquid_variable_interest_rate = 3 * PERCENT_1;
      new_options.staked_fixed_interest_rate = 2 * PERCENT_1;
      new_options.staked_variable_interest_rate = 12 * PERCENT_1;
      new_options.savings_fixed_interest_rate = 2 * PERCENT_1;
      new_options.savings_variable_interest_rate = 3 * PERCENT_1;
      new_options.var_interest_range = 30 * PERCENT_1;

      new_options.validate();
      asset_update.new_options = new_options;
      asset_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& new_dan_credit_asset = db.get_asset( asset_symbol_type( "BLOCKTWO" ) );

      BOOST_REQUIRE( new_dan_credit_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( new_dan_credit_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( new_dan_credit_asset.asset_type == asset_property_type::CREDIT_ASSET );
      BOOST_REQUIRE( to_string( new_dan_credit_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( new_dan_credit_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( new_dan_credit_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( new_dan_credit_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( new_dan_credit_asset.last_updated == now() );

      const asset_credit_data_object& new_dan_credit = db.get_credit_data( asset_symbol_type( "BLOCKTWO" ) );

      BOOST_REQUIRE( new_dan_credit.buyback_share_percent == asset_update.new_options.buyback_share_percent );
      BOOST_REQUIRE( new_dan_credit.liquid_fixed_interest_rate == asset_update.new_options.liquid_fixed_interest_rate );
      BOOST_REQUIRE( new_dan_credit.liquid_variable_interest_rate == asset_update.new_options.liquid_variable_interest_rate );
      BOOST_REQUIRE( new_dan_credit.staked_fixed_interest_rate == asset_update.new_options.staked_fixed_interest_rate );
      BOOST_REQUIRE( new_dan_credit.staked_variable_interest_rate == asset_update.new_options.staked_variable_interest_rate );
      BOOST_REQUIRE( new_dan_credit.savings_fixed_interest_rate == asset_update.new_options.savings_fixed_interest_rate );
      BOOST_REQUIRE( new_dan_credit.savings_variable_interest_rate == asset_update.new_options.savings_variable_interest_rate );
      BOOST_REQUIRE( new_dan_credit.var_interest_range == asset_update.new_options.var_interest_range );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Credit asset" );

      asset_issue_operation asset_issue;

      asset_issue.signatory = "blocktwo";
      asset_issue.issuer = "blocktwo";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "BLOCKTWO" ) );
      asset_issue.issue_to_account = "dan";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset dan_liquid = get_liquid_balance( "dan", "BLOCKTWO" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKTWO" ) ).get_total_supply() == asset_issue.asset_to_issue + asset_create.credit_liquidity * 3 );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKTWO" ) ).get_liquid_supply() == asset_issue.asset_to_issue );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Credit asset" );

      asset_reserve_operation asset_reserve;

      asset_reserve.signatory = "dan";
      asset_reserve.payer = "dan";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, asset_symbol_type( "BLOCKTWO" ) );

      tx.operations.push_back( asset_reserve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      dan_liquid = get_liquid_balance( "dan", "BLOCKTWO" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKTWO" ) ).get_total_supply() == asset_issue.asset_to_issue + asset_create.credit_liquidity * 3 - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( db.get_dynamic_data( asset_symbol_type( "BLOCKTWO" ) ).get_liquid_supply() == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Payment of interest on credit balances" );

      fund_liquid( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
      fund_savings( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );

      generate_block();

      asset init_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_CREDIT );
      asset init_staked_balance = get_staked_balance( account_name_type( "alice" ), SYMBOL_CREDIT );
      asset init_savings_balance = get_savings_balance( account_name_type( "alice" ), SYMBOL_CREDIT );

      const asset_credit_data_object& credit = db.get_credit_data( SYMBOL_CREDIT );
      asset_symbol_type cs = credit.symbol;
      price buyback = credit.buyback_price;
      price market = db.get_liquidity_pool( credit.buyback_asset, credit.symbol ).base_hour_median_price( credit.buyback_asset );
      
      asset unit = asset( BLOCKCHAIN_PRECISION, credit.symbol );
      share_type range = credit.var_interest_range;     // Percentage range that caps the price divergence between market and buyback
      share_type pr = PERCENT_100;
      share_type hpr = PERCENT_100 / 2;

      share_type mar = ( unit * market ).amount;    // Market price of the credit asset
      share_type buy = ( unit * buyback ).amount;   // Buyback price of the credit asset

      share_type liqf = credit.liquid_fixed_interest_rate;
      share_type staf = credit.staked_fixed_interest_rate;
      share_type savf = credit.savings_fixed_interest_rate;
      share_type liqv = credit.liquid_variable_interest_rate;
      share_type stav = credit.staked_variable_interest_rate;
      share_type savv = credit.savings_variable_interest_rate;

      share_type var_factor = ( ( -hpr * std::min( pr, std::max( -pr, pr * ( mar-buy ) / ( ( ( buy * range ) / pr ) ) ) ) ) / pr ) + hpr;

      uint128_t liq_ir = ( ( liqv * var_factor + liqf * pr ) / pr ).value;
      uint128_t sta_ir = ( ( stav * var_factor + staf * pr ) / pr ).value;
      uint128_t sav_ir = ( ( savv * var_factor + savf * pr ) / pr ).value;

      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_CREDIT );

      time_point start_time = balance.last_interest_time;

      uint128_t liq_b = balance.liquid_balance.value;
      uint128_t sta_b = balance.staked_balance.value;
      uint128_t sav_b = balance.savings_balance.value;

      uint128_t liq_i = ( liq_b * liq_ir ) / uint128_t( PERCENT_100 );
      uint128_t sta_i = ( sta_b * sta_ir ) / uint128_t( PERCENT_100 );
      uint128_t sav_i = ( sav_b * sav_ir ) / uint128_t( PERCENT_100 );

      generate_blocks( CREDIT_INTERVAL_BLOCKS );

      int64_t elapsed_sec = ( db.get_account_balance( "alice", SYMBOL_CREDIT ).last_interest_time - start_time ).to_seconds();
      int64_t year_sec = fc::days(365).to_seconds();

      uint128_t liq_acc = ( liq_i * elapsed_sec ) / year_sec;
      uint128_t sta_acc = ( sta_i * elapsed_sec ) / year_sec;
      uint128_t sav_acc = ( sav_i * elapsed_sec ) / year_sec;

      asset liquid_interest = asset( int64_t( liq_acc.to_uint64() ), cs );
      asset staked_interest = asset( int64_t( sta_acc.to_uint64() ), cs );
      asset savings_interest = asset( int64_t( sav_acc.to_uint64() ), cs );

      asset liquid_balance = get_liquid_balance( "alice", SYMBOL_CREDIT );
      asset staked_balance = get_staked_balance( "alice", SYMBOL_CREDIT );
      asset savings_balance = get_savings_balance( "alice", SYMBOL_CREDIT );

      ilog( "Credit Interest [ Seconds: ${sec} ]: L: ${l} St: ${st} Sa: ${sa}",
         ("sec",elapsed_sec)("l",liquid_interest.to_string())("st",staked_interest.to_string())("sa",savings_interest.to_string()));

      BOOST_REQUIRE( liquid_balance == init_liquid_balance + liquid_interest );
      BOOST_REQUIRE( staked_balance == init_staked_balance + staked_interest );
      BOOST_REQUIRE( savings_balance == init_savings_balance + savings_interest );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Payment of interest on credit balances" );

      BOOST_TEST_MESSAGE( "├── Testing: ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( stablecoin_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: STABLECOIN ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create stablecoin" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "bob", bob_private_owner_key );
      producer_vote( "bob", bob_private_owner_key );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "candice", candice_private_owner_key );
      producer_vote( "candice", candice_private_owner_key );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "dan", dan_private_owner_key );
      producer_vote( "dan", dan_private_owner_key );

      fund_liquid( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "elon", elon_private_owner_key );
      producer_vote( "elon", elon_private_owner_key );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "fred", fred_private_owner_key );
      producer_vote( "fred", fred_private_owner_key );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "george", george_private_owner_key );
      producer_vote( "george", george_private_owner_key );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "haz", haz_private_owner_key );
      producer_vote( "haz", haz_private_owner_key );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "isabelle", isabelle_private_owner_key );
      producer_vote( "isabelle", isabelle_private_owner_key );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "jayme", jayme_private_owner_key );
      producer_vote( "jayme", jayme_private_owner_key );

      fund_liquid( "kathryn", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "kathryn", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "kathryn", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "kathryn", kathryn_private_owner_key );
      producer_vote( "kathryn", kathryn_private_owner_key );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );
      generate_blocks( TOTAL_PRODUCERS );

      asset_publish_feed_operation feed;

      feed.signatory = "alice";
      feed.publisher = "alice";
      feed.symbol = SYMBOL_USD;
      feed.feed.settlement_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );   // init settlement price of 1:1
      feed.validate();

      tx.operations.push_back( feed );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "bob";
      feed.publisher = "bob";

      tx.operations.push_back( feed );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "candice";
      feed.publisher = "candice";

      tx.operations.push_back( feed );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "dan";
      feed.publisher = "dan";

      tx.operations.push_back( feed );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      asset_create_operation asset_create;

      asset_create.signatory = "elon";
      asset_create.issuer = "elon";
      asset_create.symbol = "TSLA";
      asset_create.asset_type = "stablecoin";
      asset_create.coin_liquidity = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 10 * BLOCKCHAIN_PRECISION, asset_symbol_type( "TSLA" ) );

      asset_options options;

      options.display_symbol = "Tesla";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";

      asset_create.options = options;
      options.validate();
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& elon_stablecoin_asset = db.get_asset( asset_symbol_type( "TSLA" ) );

      BOOST_REQUIRE( elon_stablecoin_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_stablecoin_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( elon_stablecoin_asset.asset_type == asset_property_type::STABLECOIN_ASSET );
      BOOST_REQUIRE( elon_stablecoin_asset.created == now() );
      BOOST_REQUIRE( elon_stablecoin_asset.last_updated == now() );

      const asset_stablecoin_data_object& elon_stablecoin = db.get_stablecoin_data( asset_symbol_type( "TSLA" ) );

      BOOST_REQUIRE( elon_stablecoin.backing_asset == SYMBOL_COIN );

      const asset_credit_pool_object& elon_credit_pool = db.get_credit_pool( asset_symbol_type( "TSLA" ), false );

      BOOST_REQUIRE( elon_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( elon_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& elon_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, asset_symbol_type( "TSLA" ) );

      BOOST_REQUIRE( elon_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( elon_liquidity_pool.symbol_b == asset_symbol_type( "TSLA" ) );
      BOOST_REQUIRE( elon_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( elon_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, asset_symbol_type( "TSLA" ) ) );

      call_order_operation call;

      call.signatory = "elon";
      call.owner = "elon";
      call.debt = asset( 30*BLOCKCHAIN_PRECISION, asset_symbol_type( "TSLA" ) );
      call.collateral = asset( 30000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      call.interface = INIT_ACCOUNT;
      call.validate();

      tx.operations.push_back( call );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      feed.signatory = "elon";
      feed.publisher = "elon";
      feed.symbol = asset_symbol_type( "TSLA" );
      feed.feed.settlement_price = price( asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "TSLA" ) ), asset( 420 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      feed.validate();

      tx.operations.push_back( feed );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create stablecoin" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update stablecoin" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      asset_update_operation asset_update;

      asset_update.signatory = "elon";
      asset_update.issuer = "elon";
      asset_update.asset_to_update = "TSLA";

      asset_options new_options;

      new_options.display_symbol = "Tesla";
      new_options.details = "New Details";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";
      new_options.backing_asset = SYMBOL_COIN;
      new_options.feed_lifetime = fc::days(2);
      new_options.minimum_feeds = 3;
      new_options.asset_settlement_delay = fc::hours(6);
      new_options.asset_settlement_offset_percent = 3 * PERCENT_1;
      new_options.maximum_asset_settlement_volume = 10 * PERCENT_1;

      new_options.validate();
      asset_update.new_options = new_options;
      asset_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& new_elon_stablecoin_asset = db.get_asset( asset_symbol_type( "TSLA" ) );

      BOOST_REQUIRE( new_elon_stablecoin_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( new_elon_stablecoin_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( new_elon_stablecoin_asset.asset_type == asset_property_type::STABLECOIN_ASSET );
      BOOST_REQUIRE( to_string( new_elon_stablecoin_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( new_elon_stablecoin_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( new_elon_stablecoin_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( new_elon_stablecoin_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( new_elon_stablecoin_asset.last_updated == now() );

      const asset_stablecoin_data_object& new_elon_stablecoin = db.get_stablecoin_data( asset_symbol_type( "TSLA" ) );

      BOOST_REQUIRE( new_elon_stablecoin.backing_asset == asset_update.new_options.backing_asset );
      BOOST_REQUIRE( new_elon_stablecoin.feed_lifetime == asset_update.new_options.feed_lifetime );
      BOOST_REQUIRE( new_elon_stablecoin.minimum_feeds == asset_update.new_options.minimum_feeds );
      BOOST_REQUIRE( new_elon_stablecoin.asset_settlement_delay == asset_update.new_options.asset_settlement_delay );
      BOOST_REQUIRE( new_elon_stablecoin.asset_settlement_offset_percent == asset_update.new_options.asset_settlement_offset_percent );
      BOOST_REQUIRE( new_elon_stablecoin.maximum_asset_settlement_volume == asset_update.new_options.maximum_asset_settlement_volume );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update stablecoin" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update Stablecoin feed producers" );

      asset_update_feed_producers_operation update_feed;

      update_feed.signatory = "elon";
      update_feed.issuer = "elon";
      update_feed.asset_to_update = "TSLA";
      update_feed.new_feed_producers.insert( "alice" );
      update_feed.new_feed_producers.insert( "bob" );
      update_feed.new_feed_producers.insert( "candice" );
      update_feed.new_feed_producers.insert( "dan" );
      update_feed.new_feed_producers.insert( "elon" );
      update_feed.validate();

      tx.operations.push_back( update_feed );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      for( auto f : update_feed.new_feed_producers )
      {
         BOOST_REQUIRE( new_elon_stablecoin.feeds.count( f ) );
      }

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update Stablecoin feed producers" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Publish stablecoin price feed" );

      feed.signatory = "alice";
      feed.publisher = "alice";

      tx.operations.push_back( feed );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "bob";
      feed.publisher = "bob";

      tx.operations.push_back( feed );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "candice";
      feed.publisher = "candice";

      tx.operations.push_back( feed );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "dan";
      feed.publisher = "dan";

      tx.operations.push_back( feed );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_stablecoin_data( asset_symbol_type( "TSLA" ) ).current_feed == feed.feed );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Publish stablecoin price feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Asset settlement" );

      call.signatory = "alice";
      call.owner = "alice";
      call.debt = asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "TSLA" ) );
      call.collateral = asset( 850 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      call.interface = INIT_ACCOUNT;
      call.validate();

      tx.operations.push_back( call );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      call.signatory = "bob";
      call.owner = "bob";
      call.collateral = asset( 860 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( call );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      call.signatory = "candice";
      call.owner = "candice";
      call.collateral = asset( 870 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( call );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      call.signatory = "dan";
      call.owner = "dan";
      call.collateral = asset( 880 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( call );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      transfer_operation transfer;

      transfer.signatory = "dan";
      transfer.from = "dan";
      transfer.to = "fred";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "TSLA" ) );
      transfer.memo = "Hello";
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      asset_settle_operation settle;

      settle.signatory = "fred";
      settle.account = "fred";
      settle.amount = asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "TSLA" ) );
      settle.interface = INIT_ACCOUNT;
      settle.validate();

      tx.operations.push_back( settle );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_settlement_object& fred_settlement = db.get_asset_settlement( account_name_type( "fred" ), asset_symbol_type( "TSLA" ) );

      BOOST_REQUIRE( fred_settlement.balance == settle.amount );
      BOOST_REQUIRE( fred_settlement.last_updated == now() );
      BOOST_REQUIRE( fred_settlement.created == now() );

      generate_blocks( fred_settlement.settlement_date, true );
      generate_block();

      const auto& settle_idx = db.get_index< asset_settlement_index >().indices().get< by_account_asset >();
      auto settle_itr = settle_idx.find( std::make_tuple( account_name_type( "alice" ), asset_symbol_type( "TSLA" ) ) );

      BOOST_REQUIRE( settle_itr == settle_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Asset settlement" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Asset Global settlement" );

      asset_global_settle_operation global_settle;

      global_settle.signatory = "elon";
      global_settle.issuer = "elon";
      global_settle.asset_to_settle = "TSLA";
      global_settle.settle_price = price( asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "TSLA" ) ), asset( 420 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      global_settle.validate();

      asset init_total_supply = db.get_dynamic_data( asset_symbol_type( "TSLA" ) ).get_total_supply();
      asset init_collateral = init_total_supply * global_settle.settle_price;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( global_settle );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_stablecoin_data( asset_symbol_type( "TSLA" ) ).has_settlement() );
      BOOST_REQUIRE( db.get_stablecoin_data( asset_symbol_type( "TSLA" ) ).settlement_price == init_total_supply / init_collateral );
      BOOST_REQUIRE( db.get_stablecoin_data( asset_symbol_type( "TSLA" ) ).settlement_fund == init_collateral.amount );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Asset Global settlement" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Creation of collateral bid after black swan event" );

      call.signatory = "alice";
      call.owner = "alice";
      call.collateral = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );  // 2x collateralization
      call.debt = asset( 500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      call.interface = INIT_ACCOUNT;
      call.validate();

      tx.operations.push_back( call );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      call.signatory = "bob";
      call.owner = "bob";
      call.collateral = asset( 1500 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );   // 3x collateralization
      call.debt = asset( 500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( call );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      call.signatory = "candice";
      call.owner = "candice";
      call.collateral = asset( 2000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );  // 4x collateralization
      call.debt = asset( 500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( call );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      call.signatory = "dan";
      call.owner = "dan";
      call.collateral = asset( 2500 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );  // 5x collateralization
      call.debt = asset( 500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( call );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      limit_order_operation limit;

      limit.signatory = "alice";
      limit.owner = "alice";
      limit.order_id = "88d551cd-0dc2-46f1-a09c-7d0cd477b550";
      limit.amount_to_sell = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      limit.exchange_rate = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( 3 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );   // USD sell order at 1:3, within SQR
      limit.expiration = now() + fc::days( 30 );
      limit.interface = INIT_ACCOUNT;
      limit.fill_or_kill = false;
      limit.opened = true;

      tx.operations.push_back( limit );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "alice";
      feed.publisher = "alice";
      feed.symbol = SYMBOL_USD;
      feed.feed.settlement_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( 2 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );   // New settlement price of $0.50 MEC/MUSD
      feed.validate();

      tx.operations.push_back( feed );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "bob";
      feed.publisher = "bob";

      tx.operations.push_back( feed );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "candice";
      feed.publisher = "candice";

      tx.operations.push_back( feed );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "dan";
      feed.publisher = "dan";

      tx.operations.push_back( feed );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      const asset_stablecoin_data_object& stablecoin = db.get_stablecoin_data( SYMBOL_USD );

      BOOST_REQUIRE( stablecoin.has_settlement() );     // Stablecoin has now undergone a black swan event

      asset_collateral_bid_operation bid;

      bid.signatory = "alice";
      bid.bidder = "alice";
      bid.collateral = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 2500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      bid.validate();

      tx.operations.push_back( bid );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_collateral_bid_object& alice_col_bid = db.get_asset_collateral_bid( account_name_type( "alice" ), SYMBOL_USD );

      BOOST_REQUIRE( alice_col_bid.bidder == bid.bidder );
      BOOST_REQUIRE( alice_col_bid.collateral == bid.collateral );
      BOOST_REQUIRE( alice_col_bid.debt == bid.debt );
      BOOST_REQUIRE( alice_col_bid.created == now() );
      BOOST_REQUIRE( alice_col_bid.last_updated == now() );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: creation of collateral bid after black swan event" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account does not have required funds" );

      bid.signatory = "bob";
      bid.bidder = "bob";
      bid.collateral = asset( 20000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      
      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account does not have required funds" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when collateral is 0" );

      bid.signatory = "bob";
      bid.bidder = "bob";
      bid.collateral = asset( 0, SYMBOL_COIN );
      bid.debt = asset( 2000, SYMBOL_USD );
      
      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when collateral is 0" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when debt and collateral amount is 0" );

      bid.signatory = "bob";
      bid.bidder = "bob";
      bid.collateral = asset( 0, SYMBOL_COIN );
      bid.debt = asset( 0, SYMBOL_USD );
      
      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when debt and collateral amount is 0" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reviving asset with additional collateral bids" );

      bid.signatory = "bob";
      bid.bidder = "bob";
      bid.collateral = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 2500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_collateral_bid_object& bob_col_bid = db.get_asset_collateral_bid( account_name_type( "bob" ), SYMBOL_USD );

      BOOST_REQUIRE( bob_col_bid.bidder == bid.bidder );
      BOOST_REQUIRE( bob_col_bid.collateral == bid.collateral );
      BOOST_REQUIRE( bob_col_bid.debt == bid.debt );
      BOOST_REQUIRE( bob_col_bid.created == now() );
      BOOST_REQUIRE( bob_col_bid.last_updated == now() );

      generate_block();

      bid.signatory = "candice";
      bid.bidder = "candice";
      bid.collateral = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 2500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( bid );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      bid.signatory = "dan";
      bid.bidder = "dan";
      bid.collateral = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 2500 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( bid );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      bid.signatory = INIT_ACCOUNT;
      bid.bidder = INIT_ACCOUNT;
      bid.collateral = asset( 500000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 250000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      
      tx.operations.push_back( bid );
      tx.sign( init_account_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_stablecoin_data( SYMBOL_USD ).has_settlement() );

      feed.signatory = "alice";
      feed.publisher = "alice";
      feed.symbol = SYMBOL_USD;
      feed.feed.settlement_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      feed.validate();

      tx.operations.push_back( feed );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "bob";
      feed.publisher = "bob";

      tx.operations.push_back( feed );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "candice";
      feed.publisher = "candice";

      tx.operations.push_back( feed );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      feed.signatory = "dan";
      feed.publisher = "dan";

      tx.operations.push_back( feed );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( STABLECOIN_BLOCK_INTERVAL );

      BOOST_REQUIRE( !db.get_stablecoin_data( SYMBOL_USD ).has_settlement() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reviving asset with additional collateral bids" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Selection of median price feed" );

      vector< string > accounts;

      accounts.push_back( "alice" );
      accounts.push_back( "bob" );
      accounts.push_back( "candice" );
      accounts.push_back( "dan" );
      accounts.push_back( "elon" );
      accounts.push_back( "fred" );
      accounts.push_back( "george" );
      accounts.push_back( "haz" );
      accounts.push_back( "isabelle" );
      accounts.push_back( "jayme" );
      accounts.push_back( "kathryn" );

      vector< private_key_type > keys;

      keys.push_back( get_private_key( "alice", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "bob", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "candice", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "dan", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "elon", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "fred", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "george", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "haz", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "isabelle", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "jayme", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "kathryn", "active", INIT_ACCOUNT_PASSWORD ) );

      vector< asset_publish_feed_operation > publish_feeds;
      publish_feeds.reserve( 11 );
      
      vector< signed_transaction > txs;
      txs.reserve( 11 );

      // Upgrade accounts to producers

      for( int i = 0; i < 11; i++ )
      {
         publish_feeds.push_back( asset_publish_feed_operation() );
         publish_feeds[i].signatory = accounts[i];
         publish_feeds[i].publisher = accounts[i];
         publish_feeds[i].symbol = SYMBOL_USD;
         publish_feeds[i].feed.settlement_price = price( asset( 1000 + 100 * i, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
         generate_block();
      }

      for( int i = 0; i < 11; i++ )
      {
         txs.push_back( signed_transaction() );
         txs[i].set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         txs[i].set_reference_block( db.head_block_id() );
         txs[i].operations.push_back( publish_feeds[i] );
         txs[i].sign( keys[i], db.get_chain_id() );
         db.push_transaction( txs[i], 0 );
         generate_block();
      }

      generate_blocks( BLOCKS_PER_HOUR );

      BOOST_REQUIRE( db.get_stablecoin_data( SYMBOL_USD ).current_feed.settlement_price == price( asset( 1500, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) ) );

      BOOST_REQUIRE( db.get_stablecoin_data( SYMBOL_USD ).feeds.size() == 11 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Selection of median price feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Updating median price feed" );

      for ( int i = 0; i < 23; i++ )
      {
         for( int j = 0; j < 11; j++ )
         {
            txs[j].operations.clear();
            txs[j].signatures.clear();

            publish_feeds[j].feed.settlement_price = price( 
               asset( publish_feeds[j].feed.settlement_price.base.amount + 10, SYMBOL_USD ), 
               asset( publish_feeds[j].feed.settlement_price.quote.amount, SYMBOL_COIN ) );

            txs[j].set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
            txs[j].set_reference_block( db.head_block_id() );
            txs[j].operations.push_back( publish_feeds[j] );
            txs[j].sign( keys[j], db.get_chain_id() );
            db.push_transaction( txs[j], 0 );
         }

         generate_blocks( BLOCKS_PER_HOUR  );

         BOOST_REQUIRE( db.get_stablecoin_data( SYMBOL_USD ).current_feed.settlement_price == publish_feeds[5].feed.settlement_price );
      }

      BOOST_TEST_MESSAGE( "│   ├── Passed: Updating median price feed" );

      BOOST_TEST_MESSAGE( "├── Testing: ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( stimulus_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: STIMULUS ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create stimulus asset" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );

      account_create_operation account_create;

      account_create.signatory = "dan";
      account_create.registrar = "dan";
      account_create.new_account_name = "blocktwo";
      account_create.referrer = INIT_ACCOUNT;
      account_create.proxy = INIT_ACCOUNT;
      account_create.recovery_account = INIT_ACCOUNT;
      account_create.reset_account = INIT_ACCOUNT;
      account_create.details = "My Details";
      account_create.url = "https://www.url.com";
      account_create.json = "{ \"valid\": true }";
      account_create.json_private = "{ \"valid\": true }";
      account_create.owner_auth = authority( 1, dan_private_owner_key.get_public_key(), 1 );
      account_create.active_auth = authority( 1, dan_private_active_key.get_public_key(), 1 );
      account_create.posting_auth = authority( 1, dan_private_posting_key.get_public_key(), 1 );
      account_create.secure_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.connection_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.friend_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.companion_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      account_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( account_create );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund_liquid( "blocktwo", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "blocktwo", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "blocktwo", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      generate_block();

      account_business_operation account_business;

      account_business.signatory = "blocktwo";
      account_business.account = "blocktwo";
      account_business.init_ceo_account = "dan";
      account_business.business_type = "open";
      account_business.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      account_business.business_public_key = string( dan_public_connection_key );
      account_business.active = true;
      account_business.validate();

      tx.operations.push_back( account_business );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_business_object& blocktwo_business = db.get_account_business( account_name_type( "blocktwo" ) );

      BOOST_REQUIRE( blocktwo_business.account == account_business.account );
      BOOST_REQUIRE( blocktwo_business.executive_board.CHIEF_EXECUTIVE_OFFICER == account_business.init_ceo_account );
      BOOST_REQUIRE( blocktwo_business.business_type == business_structure_type::OPEN_BUSINESS );

      generate_blocks( now() + fc::days(2), true );
      generate_block();

      asset_create_operation asset_create;

      asset_create.signatory = "blocktwo";
      asset_create.issuer = "blocktwo";
      asset_create.symbol = "STIM";
      asset_create.asset_type = "stimulus";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "STIM" ) );

      asset_options options;

      options.display_symbol = "StimCoin";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.redemption_asset = SYMBOL_USD;
      options.redemption_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "STIM" ) ) );
      options.distribution_list = { "alice", "bob", "candice", "dan" };
      options.redemption_list = { "blocktwo" };
      options.distribution_amount = asset( 10 * BLOCKCHAIN_PRECISION, asset_symbol_type( "STIM" ) );

      asset_create.options = options;
      options.validate();
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& dan_stim_asset = db.get_asset( asset_symbol_type( "STIM" ) );

      BOOST_REQUIRE( dan_stim_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_stim_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_stim_asset.asset_type == asset_property_type::STIMULUS_ASSET );
      BOOST_REQUIRE( dan_stim_asset.created == now() );
      BOOST_REQUIRE( dan_stim_asset.last_updated == now() );

      const asset_stimulus_data_object& dan_stimulus = db.get_stimulus_data( asset_symbol_type( "STIM" ) );

      BOOST_REQUIRE( dan_stimulus.business_account == asset_create.issuer );
      BOOST_REQUIRE( dan_stimulus.redemption_asset == asset_create.options.redemption_asset );
      BOOST_REQUIRE( dan_stimulus.redemption_price == asset_create.options.redemption_price );

      for( auto name : asset_create.options.distribution_list )
      {
         BOOST_REQUIRE( std::find( dan_stimulus.distribution_list.begin(), dan_stimulus.distribution_list.end(), name ) != dan_stimulus.distribution_list.end() );
      }

      for( auto name : asset_create.options.redemption_list )
      {
         BOOST_REQUIRE( std::find( dan_stimulus.redemption_list.begin(), dan_stimulus.redemption_list.end(), name ) != dan_stimulus.redemption_list.end() );
      }
      
      BOOST_REQUIRE( dan_stimulus.distribution_amount == asset_create.options.distribution_amount );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create stimulus asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update stimulus asset" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      asset_update_operation asset_update;

      asset_update.signatory = "blocktwo";
      asset_update.issuer = "blocktwo";
      asset_update.asset_to_update = "STIM";

      asset_options new_options;

      new_options.display_symbol = "StimCoin";
      new_options.details = "New Details";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";
      new_options.redemption_asset = SYMBOL_USD;
      new_options.redemption_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "STIM" ) ) );
      new_options.distribution_list = { "alice", "bob", "candice", "dan", "elon" };
      new_options.redemption_list = { "blocktwo", "tropico" };
      new_options.distribution_amount = asset( 20 * BLOCKCHAIN_PRECISION, asset_symbol_type( "STIM" ) );

      new_options.validate();
      asset_update.new_options = new_options;
      asset_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& new_dan_stim_asset = db.get_asset( asset_symbol_type( "STIM" ) );

      BOOST_REQUIRE( new_dan_stim_asset.issuer == asset_update.issuer );
      BOOST_REQUIRE( new_dan_stim_asset.symbol == asset_update.asset_to_update );
      BOOST_REQUIRE( new_dan_stim_asset.asset_type == asset_property_type::STIMULUS_ASSET );
      BOOST_REQUIRE( to_string( new_dan_stim_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( new_dan_stim_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( new_dan_stim_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( new_dan_stim_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( new_dan_stim_asset.last_updated == now() );

      const asset_stimulus_data_object& new_dan_stimulus = db.get_stimulus_data( asset_symbol_type( "STIM" ) );

      BOOST_REQUIRE( new_dan_stimulus.business_account == asset_update.issuer );
      BOOST_REQUIRE( new_dan_stimulus.redemption_asset == asset_update.new_options.redemption_asset );
      BOOST_REQUIRE( new_dan_stimulus.redemption_price == asset_update.new_options.redemption_price );

      for( auto name : asset_update.new_options.distribution_list )
      {
         BOOST_REQUIRE( std::find( new_dan_stimulus.distribution_list.begin(), new_dan_stimulus.distribution_list.end(), name ) != new_dan_stimulus.distribution_list.end() );
      }

      for( auto name : asset_update.new_options.redemption_list )
      {
         BOOST_REQUIRE( std::find( new_dan_stimulus.redemption_list.begin(), new_dan_stimulus.redemption_list.end(), name ) != new_dan_stimulus.redemption_list.end() );
      }

      BOOST_REQUIRE( new_dan_stimulus.distribution_amount == asset_update.new_options.distribution_amount );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update stimulus asset" );

      BOOST_TEST_MESSAGE( "├── Testing: STIMULUS ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( unique_asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: UNIQUE ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create Unique asset" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( GENESIS_TIME + fc::days(2), true );

      account_create_operation account_create;

      account_create.signatory = "candice";
      account_create.registrar = "candice";
      account_create.new_account_name = "tropico";
      account_create.referrer = INIT_ACCOUNT;
      account_create.proxy = INIT_ACCOUNT;
      account_create.recovery_account = INIT_ACCOUNT;
      account_create.reset_account = INIT_ACCOUNT;
      account_create.details = "My Details";
      account_create.url = "https://www.url.com";
      account_create.json = "{ \"valid\": true }";
      account_create.json_private = "{ \"valid\": true }";
      account_create.owner_auth = authority( 1, candice_public_owner_key, 1 );
      account_create.active_auth = authority( 1, candice_public_active_key, 1 );
      account_create.posting_auth = authority( 1, candice_public_posting_key, 1 );
      account_create.secure_public_key = string( candice_public_secure_key );
      account_create.connection_public_key = string( candice_public_connection_key );
      account_create.friend_public_key = string( candice_public_friend_key );
      account_create.companion_public_key = string( candice_public_friend_key );
      account_create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      account_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( account_create );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund_liquid( "tropico", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "tropico", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "tropico", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      generate_block();

      account_business_operation account_business;

      account_business.signatory = "tropico";
      account_business.account = "tropico";
      account_business.init_ceo_account = "candice";
      account_business.business_type = "open";
      account_business.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      account_business.business_public_key = string( candice_public_connection_key );
      account_business.active = true;
      account_business.validate();

      tx.operations.push_back( account_business );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::days(2), true );
      generate_block();

      const account_business_object& tropico_business = db.get_account_business( account_name_type( "tropico" ) );

      BOOST_REQUIRE( tropico_business.account == account_business.account );
      BOOST_REQUIRE( tropico_business.executive_board.CHIEF_EXECUTIVE_OFFICER == account_business.init_ceo_account );
      BOOST_REQUIRE( tropico_business.business_type == business_structure_type::OPEN_BUSINESS );

      asset_create_operation asset_create;
      
      asset_create.signatory = "tropico";
      asset_create.issuer = "tropico";
      asset_create.symbol = "TROPICO";
      asset_create.asset_type = "equity";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, asset_symbol_type( "TROPICO" ) );

      asset_options options;

      options.display_symbol = "TRO";
      options.details = "Details";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.validate();

      asset_create.options = options;
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::days(2) );
      generate_block();

      asset_create.signatory = "tropico";
      asset_create.issuer = "tropico";
      asset_create.symbol = "MANSION";
      asset_create.asset_type = "unique";
      asset_create.coin_liquidity = asset( 0 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 0 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 0 * BLOCKCHAIN_PRECISION, asset_symbol_type( "MANSION" ) );

      options.display_symbol = "Title Deed of The Tropico Mansion Resort";
      options.details = "Access list enables entry to Tropico Mansion Resort.";
      options.json = "{ \"valid\": true }";
      options.url = "https://www.url.com";
      options.ownership_asset = "TROPICO";
      options.control_list = { "candice", "tropico" };
      options.access_list = { "alice", "bob" };
      options.access_price = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      options.max_supply = BLOCKCHAIN_PRECISION;

      asset_create.options = options;
      options.validate();
      asset_create.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& candice_unique_asset = db.get_asset( asset_symbol_type( "MANSION" ) );

      BOOST_REQUIRE( candice_unique_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( candice_unique_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( candice_unique_asset.asset_type == asset_property_type::UNIQUE_ASSET );
      BOOST_REQUIRE( candice_unique_asset.created == now() );
      BOOST_REQUIRE( candice_unique_asset.last_updated == now() );

      const asset_unique_data_object& mansion_unique_data = db.get_unique_data( asset_symbol_type( "MANSION" ) );

      BOOST_REQUIRE( mansion_unique_data.controlling_owner == asset_create.issuer );
      BOOST_REQUIRE( mansion_unique_data.ownership_asset == asset_create.options.ownership_asset );
      BOOST_REQUIRE( mansion_unique_data.access_price == asset_create.options.access_price );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create Unique asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update Unique asset" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      asset_update_operation asset_update;

      asset_update.signatory = "tropico";
      asset_update.issuer = "tropico";
      asset_update.asset_to_update = "MANSION";

      asset_options new_options;

      new_options.display_symbol = "Title Deed of The Tropico Mansion Resort Complex";
      new_options.details = "Access list enables entry to Tropico Mansion Resort Complex";
      new_options.json = "{\"json\":\"supervalid\"}";
      new_options.url = "https://www.newurl.com";
      new_options.ownership_asset = "TROPICO";
      new_options.control_list = { "candice", "tropico" };
      new_options.access_list = { "alice", "bob", "dan", "elon" };
      new_options.access_price = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      new_options.max_supply = BLOCKCHAIN_PRECISION;

      new_options.validate();
      asset_update.new_options = new_options;
      asset_update.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( asset_update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& new_candice_unique_asset = db.get_asset( asset_symbol_type( "MANSION" ) );

      BOOST_REQUIRE( new_candice_unique_asset.issuer == asset_update.issuer );
      BOOST_REQUIRE( new_candice_unique_asset.symbol == asset_update.asset_to_update );
      BOOST_REQUIRE( new_candice_unique_asset.asset_type == asset_property_type::UNIQUE_ASSET );
      BOOST_REQUIRE( new_candice_unique_asset.last_updated == now() );

      const asset_unique_data_object& new_mansion_unique_data = db.get_unique_data( asset_symbol_type( "MANSION" ) );

      BOOST_REQUIRE( new_mansion_unique_data.controlling_owner == asset_update.issuer );
      BOOST_REQUIRE( new_mansion_unique_data.ownership_asset == asset_update.new_options.ownership_asset );
      BOOST_REQUIRE( new_mansion_unique_data.access_price == asset_update.new_options.access_price );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update Unique asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Unique asset" );

      asset_issue_operation asset_issue;

      asset_issue.signatory = "tropico";
      asset_issue.issuer = "tropico";
      asset_issue.asset_to_issue = asset( BLOCKCHAIN_PRECISION, asset_symbol_type( "MANSION" ) );
      asset_issue.issue_to_account = "candice";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset mansion_liquid = get_liquid_balance( "candice", "MANSION" );

      const asset_dynamic_data_object& mansion_dyn_data = db.get_dynamic_data( asset_symbol_type( "MANSION" ) );

      BOOST_REQUIRE( mansion_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( mansion_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( mansion_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      const asset_unique_data_object& new_mansion_unique_data2 = db.get_unique_data( asset_symbol_type( "MANSION" ) );

      BOOST_REQUIRE( new_mansion_unique_data2.controlling_owner == asset_issue.issue_to_account );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Unique asset" );

      BOOST_TEST_MESSAGE( "├── Testing: UNIQUE ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()