//#ifdef IS_TEST_NET

#include <boost/test/unit_test.hpp>
#include <node/chain/node_objects.hpp>
#include <node/protocol/exceptions.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>

//#include <node/chain/hardfork.hpp>

#include <node/chain/util/reward.hpp>
#include <node/witness/witness_objects.hpp>
#include <fc/crypto/digest.hpp>
#include <tests/common/database_fixture.hpp>

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



BOOST_AUTO_TEST_CASE( asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create standard asset" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "elon", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "fred", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "george", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "george", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "george", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "george", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "haz", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "isabelle", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "isabelle", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "isabelle", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "isabelle", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund( "jayme", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "jayme", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "jayme", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "jayme", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      asset_create_operation asset_create;

      asset_create.signatory = "alice";
      asset_create.issuer = "alice";
      asset_create.symbol = "ALICECOIN";
      asset_create.asset_type = STANDARD_ASSET;
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "ALICECOIN" );
      asset_create.common_options.display_symbol = "Alice Coin";
      asset_create.common_options.details = "Details";
      asset_create.common_options.json = "{\"json\":\"valid\"}";
      asset_create.common_options.url = "www.url.com";
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& alice_asset = db.get_asset( "ALICECOIN" );

      BOOST_REQUIRE( alice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( alice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( alice_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( alice_asset.created == now() );
      BOOST_REQUIRE( alice_asset.last_updated == now() );

      const asset_credit_pool_object& alice_credit_pool = db.get_credit_pool( "ALICECOIN", false );

      BOOST_REQUIRE( alice_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( alice_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& alice_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, "ALICECOIN" );

      BOOST_REQUIRE( alice_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( alice_liquidity_pool.symbol_b == "ALICECOIN" );
      BOOST_REQUIRE( alice_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( alice_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, "ALICECOIN" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update standard asset" );

      asset_update_operation asset_update;

      asset_update.signatory = "alice";
      asset_update.issuer = "alice";
      asset_update.asset_to_update = "ALICECOIN";
      asset_update.new_options.display_symbol = "AliceCoin";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "www.newurl.com";
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& alice_asset = db.get_asset( "ALICECOIN" );

      BOOST_REQUIRE( alice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( alice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( alice_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( alice_asset.options.display_symbol == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( alice_asset.options.details == asset_update.new_options.details );
      BOOST_REQUIRE( alice_asset.options.json == asset_update.new_options.json );
      BOOST_REQUIRE( alice_asset.options.url == asset_update.new_options.url );
      BOOST_REQUIRE( alice_asset.last_updated == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create currency asset" );

      asset_create.signatory = "bob";
      asset_create.issuer = "bob";
      asset_create.symbol = "BOBCOIN";
      asset_create.asset_type = CURRENCY_ASSET;
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "BOBCOIN" );
      asset_create.common_options.display_symbol = "Bob Coin";
      asset_create.common_options.details = "Details";
      asset_create.common_options.json = "{\"json\":\"valid\"}";
      asset_create.common_options.url = "www.url.com";

      currency_options currency_opts;
      asset_create.currency_opts = currency_opts;
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& bob_asset = db.get_asset( "BOBCOIN" );

      BOOST_REQUIRE( bob_asset.issuer == NULL_ACCOUNT );
      BOOST_REQUIRE( bob_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( bob_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( bob_asset.created == now() );
      BOOST_REQUIRE( bob_asset.last_updated == now() );

      const asset_credit_pool_object& bob_credit_pool = db.get_credit_pool( "BOBCOIN", false );

      BOOST_REQUIRE( bob_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( bob_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& bob_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, "BOBCOIN" );

      BOOST_REQUIRE( bob_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( bob_liquidity_pool.symbol_b == "BOBCOIN" );
      BOOST_REQUIRE( bob_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( bob_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, "BOBCOIN" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create currency asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when attempting to update currency asset" );

      asset_update_operation asset_update;

      asset_update.signatory = "bob";
      asset_update.issuer = "bob";
      asset_update.asset_to_update = "BOBCOIN";
      asset_update.new_options.display_symbol = "BobCoin";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "www.newurl.com";
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );    // Bob is not issuer, so cannot update

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when attempting to update currency asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create equity asset" );

      account_create_operation account_create;

      account_create.signatory = "candice";
      account_create.registrar = "candice";
      account_create.new_account_name = "tropico";
      account_create.account_type = BUSINESS;
      account_create.referrer = INIT_ACCOUNT;
      account_create.proxy = INIT_ACCOUNT;
      account_create.governance_account = INIT_ACCOUNT;
      account_create.recovery_account = INIT_ACCOUNT;
      account_create.reset_account = INIT_ACCOUNT;
      account_create.details = "My Details";
      account_create.url = "www.url.com";
      account_create.json = "{\"json\":\"valid\"}";
      account_create.json_private = "{\"json\":\"valid\"}";
      account_create.owner = authority( 1, candice_private_owner_key.get_public_key(), 1 );
      account_create.active = authority( 1, candice_private_active_key.get_public_key(), 1 );
      account_create.posting = authority( 1, candice_private_posting_key.get_public_key(), 1 );
      account_create.secure_public_key = string( public_key_type( candice_private_posting_key.get_public_key() ) );
      account_create.connection_public_key = string( public_key_type( candice_private_posting_key.get_public_key() ) );
      account_create.friend_public_key = string( public_key_type( candice_private_posting_key.get_public_key() ) );
      account_create.companion_public_key = string( public_key_type( candice_private_posting_key.get_public_key() ) );
      account_create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      account_create.validate();

      tx.operations.push_back( account_create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_object& tropico = db.get_account( "tropico" );

      asset_create.signatory = "tropico";
      asset_create.issuer = "tropico";
      asset_create.symbol = "TROPICO";
      asset_create.asset_type = EQUITY_ASSET;
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "TROPICO" );
      asset_create.common_options.display_symbol = "Tropico";
      asset_create.common_options.details = "Details";
      asset_create.common_options.json = "{\"json\":\"valid\"}";
      asset_create.common_options.url = "www.url.com";

      equity_options equity_opts;
      asset_create.equity_opts = equity_opts;
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& candice_asset = db.get_asset( "TROPICO" );

      BOOST_REQUIRE( candice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( candice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( candice_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( candice_asset.created == now() );
      BOOST_REQUIRE( candice_asset.last_updated == now() );

      const asset_equity_data_object& candice_equity = db.get_equity_data( "TROPICO" );

      BOOST_REQUIRE( candice_equity.business_account == asset_create.issuer );
      BOOST_REQUIRE( candice_equity.dividend_asset == SYMBOL_USD );
      BOOST_REQUIRE( candice_equity.dividend_pool.amount == 0 );

      const asset_credit_pool_object& candice_credit_pool = db.get_credit_pool( "TROPICO", false );

      BOOST_REQUIRE( candice_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( candice_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& candice_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, "TROPICO" );

      BOOST_REQUIRE( candice_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( candice_liquidity_pool.symbol_b == "TROPICO" );
      BOOST_REQUIRE( candice_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( candice_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, "TROPICO" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update equity asset" );

      asset_update.signatory = "tropico";
      asset_update.issuer = "tropico";
      asset_update.asset_to_update = "TROPICO";
      asset_update.new_options.display_symbol = "Tropico Equity";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "www.newurl.com";

      equity_options equity_opts;
      equity_opts.dividend_share_percent = 10 * PERCENT_1;
      equity_opts.liquid_dividend_percent = 20 * PERCENT_1;
      equity_opts.savings_dividend_percent = 20 * PERCENT_1;
      equity_opts.staked_dividend_percent = 60 * PERCENT_1;

      asset_update.new_equity_opts = equity_opts;
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& candice_asset = db.get_asset( "TROPICO" );

      BOOST_REQUIRE( candice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( candice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( candice_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( candice_asset.options.display_symbol == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( candice_asset.options.details == asset_update.new_options.details );
      BOOST_REQUIRE( candice_asset.options.json == asset_update.new_options.json );
      BOOST_REQUIRE( candice_asset.options.url == asset_update.new_options.url );
      BOOST_REQUIRE( candice_asset.last_updated == now() );

      const asset_equity_data_object& candice_equity = db.get_equity_data( "TROPICO" );

      BOOST_REQUIRE( candice_equity.options.dividend_share_percent == equity_opts.dividend_share_percent );
      BOOST_REQUIRE( candice_equity.options.liquid_dividend_percent == equity_opts.liquid_dividend_percent );
      BOOST_REQUIRE( candice_equity.options.savings_dividend_percent == equity_opts.savings_dividend_percent );
      BOOST_REQUIRE( candice_equity.options.staked_dividend_percent == equity_opts.staked_dividend_percent );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create credit asset" );

      account_create_operation account_create;

      account_create.signatory = "dan";
      account_create.registrar = "dan";
      account_create.new_account_name = "blocktwo";
      account_create.account_type = BUSINESS;
      account_create.referrer = INIT_ACCOUNT;
      account_create.proxy = INIT_ACCOUNT;
      account_create.governance_account = INIT_ACCOUNT;
      account_create.recovery_account = INIT_ACCOUNT;
      account_create.reset_account = INIT_ACCOUNT;
      account_create.details = "My Details";
      account_create.url = "www.url.com";
      account_create.json = "{\"json\":\"valid\"}";
      account_create.json_private = "{\"json\":\"valid\"}";
      account_create.owner = authority( 1, dan_private_owner_key.get_public_key(), 1 );
      account_create.active = authority( 1, dan_private_active_key.get_public_key(), 1 );
      account_create.posting = authority( 1, dan_private_posting_key.get_public_key(), 1 );
      account_create.secure_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.connection_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.friend_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.companion_public_key = string( public_key_type( dan_private_posting_key.get_public_key() ) );
      account_create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      account_create.validate();

      tx.operations.push_back( account_create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_object& blocktwo = db.get_account( "blocktwo" );

      asset_create.signatory = "blocktwo";
      asset_create.issuer = "blocktwo";
      asset_create.symbol = "BLOCKTWO";
      asset_create.asset_type = CREDIT_ASSET;
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "BLOCKTWO" );
      asset_create.common_options.display_symbol = "Block Two";
      asset_create.common_options.details = "Details";
      asset_create.common_options.json = "{\"json\":\"valid\"}";
      asset_create.common_options.url = "www.url.com";

      credit_options op;
      asset_create.credit_opts = op;
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& dan_asset = db.get_asset( "BLOCKTWO" );

      BOOST_REQUIRE( dan_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( dan_asset.created == now() );
      BOOST_REQUIRE( dan_asset.last_updated == now() );

      const asset_credit_data_object& dan_credit = db.get_credit_data( "BLOCKTWO" );

      BOOST_REQUIRE( dan_credit.business_account == asset_create.issuer );
      BOOST_REQUIRE( dan_credit.buyback_asset == SYMBOL_USD );
      BOOST_REQUIRE( dan_credit.buyback_pool.amount == 0 );

      const asset_credit_pool_object& dan_credit_pool = db.get_credit_pool( "BLOCKTWO", false );

      BOOST_REQUIRE( dan_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( dan_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& dan_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, "BLOCKTWO" );

      BOOST_REQUIRE( dan_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( dan_liquidity_pool.symbol_b == "BLOCKTWO" );
      BOOST_REQUIRE( dan_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( dan_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, "BLOCKTWO" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update credit asset" );

      asset_update.signatory = "blocktwo";
      asset_update.issuer = "blocktwo";
      asset_update.asset_to_update = "BLOCKTWO";
      asset_update.new_options.display_symbol = "Block Two Equity";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "www.newurl.com";

      credit_options credit_opts;
      credit_opts.buyback_share_percent = 10 * PERCENT_1;
      credit_opts.liquid_fixed_interest_rate = 2 * PERCENT_1;
      credit_opts.liquid_variable_interest_rate = 3 * PERCENT_1;
      credit_opts.staked_fixed_interest_rate = 2 * PERCENT_1;
      credit_opts.staked_variable_interest_rate = 12 * PERCENT_1;
      credit_opts.savings_fixed_interest_rate = 2 * PERCENT_1;
      credit_opts.savings_variable_interest_rate = 3 * PERCENT_1;
      credit_opts.var_interest_range = 30 * PERCENT_1;

      asset_update.new_credit_opts = credit_opts;
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& dan_asset = db.get_asset( "BLOCKTWO" );

      BOOST_REQUIRE( dan_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( dan_asset.options.display_symbol == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( dan_asset.options.details == asset_update.new_options.details );
      BOOST_REQUIRE( dan_asset.options.json == asset_update.new_options.json );
      BOOST_REQUIRE( dan_asset.options.url == asset_update.new_options.url );
      BOOST_REQUIRE( dan_asset.last_updated == now() );

      const asset_credit_data_object& dan_credit = db.get_credit_data( "BLOCKTWO" );

      BOOST_REQUIRE( dan_credit.options.buyback_share_percent == credit_opts.buyback_share_percent );
      BOOST_REQUIRE( dan_credit.options.liquid_fixed_interest_rate == credit_opts.liquid_fixed_interest_rate );
      BOOST_REQUIRE( dan_credit.options.liquid_variable_interest_rate == credit_opts.liquid_variable_interest_rate );
      BOOST_REQUIRE( dan_credit.options.staked_fixed_interest_rate == credit_opts.staked_fixed_interest_rate );
      BOOST_REQUIRE( dan_credit.options.staked_variable_interest_rate == credit_opts.staked_variable_interest_rate );
      BOOST_REQUIRE( dan_credit.options.savings_fixed_interest_rate == credit_opts.savings_fixed_interest_rate );
      BOOST_REQUIRE( dan_credit.options.savings_variable_interest_rate == credit_opts.savings_variable_interest_rate );
      BOOST_REQUIRE( dan_credit.options.var_interest_range == credit_opts.var_interest_range );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create bitasset" );

      asset_create.signatory = "elon";
      asset_create.issuer = "elon";
      asset_create.symbol = "TSLA";
      asset_create.asset_type = BITASSET_ASSET;
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "TSLA" );
      asset_create.common_options.display_symbol = "Tesla";
      asset_create.common_options.details = "Details";
      asset_create.common_options.json = "{\"json\":\"valid\"}";
      asset_create.common_options.url = "www.url.com";

      bitasset_options bitasset_opts;
      asset_create.bitasset_opts = bitasset_opts;
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& elon_asset = db.get_asset( "TSLA" );

      BOOST_REQUIRE( elon_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( elon_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( elon_asset.created == now() );
      BOOST_REQUIRE( elon_asset.last_updated == now() );

      const asset_bitasset_data_object& elon_bitasset = db.get_bitasset_data( "TSLA" );

      BOOST_REQUIRE( elon_bitasset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_bitasset.backing_asset == SYMBOL_COIN );

      const asset_credit_pool_object& elon_credit_pool = db.get_credit_pool( "TSLA", false );

      BOOST_REQUIRE( elon_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( elon_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& elon_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, "TSLA" );

      BOOST_REQUIRE( elon_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( elon_liquidity_pool.symbol_b == "TSLA" );
      BOOST_REQUIRE( elon_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( elon_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, "TSLA" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create bitasset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update bitasset" );

      asset_update.signatory = "elon";
      asset_update.issuer = "elon";
      asset_update.asset_to_update = "TSLA";
      asset_update.new_options.display_symbol = "Tesla";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "www.newurl.com";

      bitasset_options bitasset_opts;
      bitasset_opts.short_backing_asset = SYMBOL_COIN;
      bitasset_opts.feed_lifetime = fc::days(2);
      bitasset_opts.minimum_feeds = 3;
      bitasset_opts.force_settlement_delay = fc::hours(6);
      bitasset_opts.force_settlement_offset_percent = 3 * PERCENT_1;
      bitasset_opts.maximum_force_settlement_volume = 10 * PERCENT_1;

      asset_create.bitasset_opts = bitasset_opts;

      tx.operations.push_back( asset_update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& elon_asset = db.get_asset( "TSLA" );

      BOOST_REQUIRE( elon_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( elon_asset.asset_type == asset_create.asset_type );
      BOOST_REQUIRE( elon_asset.options.display_symbol == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( elon_asset.options.details == asset_update.new_options.details );
      BOOST_REQUIRE( elon_asset.options.json == asset_update.new_options.json );
      BOOST_REQUIRE( elon_asset.options.url == asset_update.new_options.url );
      BOOST_REQUIRE( elon_asset.last_updated == now() );

      const asset_bitasset_data_object& elon_bitasset = db.get_bitasset_data( "TSLA" );

      BOOST_REQUIRE( elon_bitasset.options.short_backing_asset == bitasset_opts.short_backing_asset );
      BOOST_REQUIRE( elon_bitasset.backing_asset == bitasset_opts.short_backing_asset );
      BOOST_REQUIRE( elon_bitasset.options.feed_lifetime == bitasset_opts.feed_lifetime );
      BOOST_REQUIRE( elon_bitasset.options.minimum_feeds == bitasset_opts.minimum_feeds );
      BOOST_REQUIRE( elon_bitasset.options.force_settlement_delay == bitasset_opts.force_settlement_delay );
      BOOST_REQUIRE( elon_bitasset.options.force_settlement_offset_percent == bitasset_opts.force_settlement_offset_percent );
      BOOST_REQUIRE( elon_bitasset.options.maximum_force_settlement_volume == bitasset_opts.maximum_force_settlement_volume );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update bitasset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Standard asset" );

      asset_issue_operation asset_issue;

      asset_issue.signatory = "alice";
      asset_issue.issuer = "alice";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, "ALICECOIN" );
      asset_issue.issue_to_account = "alice";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset liquid = db.get_liquid_balance( "alice", "ALICECOIN" );

      const asset_dynamic_data_object& alice_dyn_data = db.get_dynamic_data( "ALICECOIN" );

      BOOST_REQUIRE( liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( alice_dyn_data.total_supply == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( alice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Equity asset" );

      asset_issue.signatory = "tropico";
      asset_issue.issuer = "tropico";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, "TROPICO" );
      asset_issue.issue_to_account = "candice";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset liquid = db.get_liquid_balance( "candice", "TROPICO" );

      const asset_dynamic_data_object& candice_dyn_data = db.get_dynamic_data( "TROPICO" );

      BOOST_REQUIRE( liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( candice_dyn_data.total_supply == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( candice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Credit asset" );

      asset_issue.signatory = "blocktwo";
      asset_issue.issuer = "blocktwo";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, "BLOCKTWO" );
      asset_issue.issue_to_account = "dan";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset liquid = db.get_liquid_balance( "dan", "BLOCKTWO" );

      const asset_dynamic_data_object& dan_dyn_data = db.get_dynamic_data( "BLOCKTWO" );

      BOOST_REQUIRE( liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( dan_dyn_data.total_supply == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( dan_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Standard asset" );

      asset_reserve_operation asset_reserve;

      asset_reserve.signatory = "alice";
      asset_reserve.payer = "alice";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, "ALICECOIN" );
      asset_reserve.validate();

      tx.operations.push_back( asset_reserve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset liquid = db.get_liquid_balance( "alice", "ALICECOIN" );

      const asset_dynamic_data_object& alice_dyn_data = db.get_dynamic_data( "ALICECOIN" );

      BOOST_REQUIRE( liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( alice_dyn_data.total_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( alice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Equity asset" );

      asset_reserve.signatory = "candice";
      asset_reserve.payer = "candice";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, "TROPICO" );

      tx.operations.push_back( asset_reserve );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset liquid = db.get_liquid_balance( "candice", "TROPICO" );

      const asset_dynamic_data_object& candice_dyn_data = db.get_dynamic_data( "TROPICO" );

      BOOST_REQUIRE( liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( candice_dyn_data.total_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( candice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Credit asset" );

      asset_reserve.signatory = "dan";
      asset_reserve.payer = "dan";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, "BLOCKTWO" );

      tx.operations.push_back( asset_reserve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset liquid = db.get_liquid_balance( "dan", "BLOCKTWO" );

      const asset_dynamic_data_object& dan_dyn_data = db.get_dynamic_data( "BLOCKTWO" );

      BOOST_REQUIRE( liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( dan_dyn_data.total_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( dan_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update Asset issuer" );

      asset_update_issuer_operation asset_update_issuer;

      asset_update_issuer.signatory = "alice";
      asset_update_issuer.issuer = "alice";
      asset_update_issuer.asset_to_update = "ALICECOIN";
      asset_update_issuer.new_issuer = "bob";
      asset_update_issuer.validate();

      tx.operations.push_back( asset_update_issuer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& alicecoin_asset = db.get_asset( "ALICECOIN" );

      BOOST_REQUIRE( alicecoin_asset.issuer == asset_update_issuer.new_issuer );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update Asset issuer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: New Issuer issuing asset to own account" );

      asset_issue.signatory = "bob";
      asset_issue.issuer = "bob";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, "ALICECOIN" );
      asset_issue.issue_to_account = "bob";
      asset_issue.memo = "Hello";

      tx.operations.push_back( asset_issue );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset liquid = db.get_liquid_balance( "bob", "ALICECOIN" );

      const asset_dynamic_data_object& alice_dyn_data = db.get_dynamic_data( "ALICECOIN" );

      BOOST_REQUIRE( liquid == asset_issue.asset_to_issue );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: New Issuer issuing asset to own account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update Bitasset feed producers" );

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

      const asset_object& elon_asset = db.get_asset( "TSLA" );

      BOOST_REQUIRE( elon_asset.last_updated == now() );

      const asset_bitasset_data_object& elon_bitasset = db.get_bitasset_data( "TSLA" );

      for( auto f : update_feed.new_feed_producers )
      {
         BOOST_REQUIRE( elon_bitasset.feeds.count( f ) );
      }
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update Bitasset feed producers" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Publish bitasset price feed" );

      asset_publish_feed_operation feed;

      feed.signatory = "alice";
      feed.publisher = "alice";
      feed.symbol = "TSLA";
      feed.feed.settlement_price = price( asset(1,"TSLA"),asset(420,SYMBOL_COIN) );
      feed.validate();

      tx.operations.push_back( feed );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      feed.signatory = "bob";
      feed.publisher = "bob";

      tx.operations.push_back( feed );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      feed.signatory = "candice";
      feed.publisher = "candice";

      tx.operations.push_back( feed );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      feed.signatory = "dan";
      feed.publisher = "dan";

      tx.operations.push_back( feed );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const asset_bitasset_data_object& elon_bitasset = db.get_bitasset_data( "TSLA" );

      BOOST_REQUIRE( elon_bitasset.current_feed == feed.feed );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Publish bitasset price feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Asset settlement" );

      call_order_operation call;

      call.signatory = "alice";
      call.owner = "alice";
      call.debt = asset( 100 * BLOCKCHAIN_PRECISION, "TSLA" );
      call.collateral = asset( 85000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      call.interface = INIT_ACCOUNT;
      call.validate();

      tx.operations.push_back( call );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      call.signatory = "bob";
      call.owner = "bob";
      call.collateral = asset( 86000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( call );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      call.signatory = "candice";
      call.owner = "candice";
      call.collateral = asset( 87000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( call );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      call.signatory = "dan";
      call.owner = "dan";
      call.collateral = asset( 88000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( call );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      call.signatory = "elon";
      call.owner = "elon";
      call.collateral = asset( 89000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( call );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      transfer_operation transfer;

      transfer.signatory = "elon";
      transfer.from = "elon";
      transfer.to = "fred";
      transfer.amount = asset( 20 * BLOCKCHAIN_PRECISION, "TSLA" );
      transfer.memo = "Hello";
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset_settle_operation settle;

      settle.signatory = "fred";
      settle.account = "fred";
      settle.amount = asset( 10 * BLOCKCHAIN_PRECISION, "TSLA" );
      settle.validate();

      tx.operations.push_back( settle );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const force_settlement_object& fred_settlement = db.get_force_settlement( "fred", "TSLA" );

      BOOST_REQUIRE( fred_settlement.balance == settle.amount );
      BOOST_REQUIRE( fred_settlement.last_updated == now() );
      BOOST_REQUIRE( fred_settlement.created == now() );

      generate_blocks( fred_settlement.settlement_date + BLOCK_INTERVAL );

      const auto& settle_idx = db.get_index< force_settlement_index >().indices().get< by_account_asset >();
      auto settle_itr = settle_idx.find( std::make_tuple( "alice", "TSLA" ) );

      BOOST_REQUIRE( settle_itr == settle_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Asset settlement" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Asset Global settlement" );

      asset_global_settle_operation global_settle;

      global_settle.signatory = "elon";
      global_settle.issuer = "elon";
      global_settle.asset_to_settle = "TSLA";
      global_settle.settle_price = price( asset( 420 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ), asset( BLOCKCHAIN_PRECISION, "TSLA" ) );
      global_settle.validate();

      tx.operations.push_back( settle );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_bitasset_data_object& elon_bitasset = db.get_bitasset_data( "TSLA" );

      BOOST_REQUIRE( elon_bitasset.has_settlement() );
      BOOST_REQUIRE( elon_bitasset.settlement_fund.amount > 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Asset Global settlement" );

      BOOST_TEST_MESSAGE( "├── Testing: ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()