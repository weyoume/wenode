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



BOOST_AUTO_TEST_CASE( asset_operation_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ASSET OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create standard asset" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

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
      asset_create.asset_type = "standard";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "ALICECOIN" );
      asset_create.options.display_symbol = "Alice Coin";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
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
      BOOST_REQUIRE( alice_asset.asset_type == asset_property_type::STANDARD_ASSET );
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
      asset_update.new_options.url = "https://www.newurl.com";
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( alice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( alice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( alice_asset.asset_type == asset_property_type::STANDARD_ASSET );
      BOOST_REQUIRE( to_string( alice_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( alice_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( alice_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( alice_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( alice_asset.last_updated == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update standard asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create currency asset" );

      asset_create.signatory = "bob";
      asset_create.issuer = "bob";
      asset_create.symbol = "BOBCOIN";
      asset_create.asset_type = "currency";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "BOBCOIN" );
      asset_create.options.display_symbol = "Bob Coin";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& bob_asset = db.get_asset( "BOBCOIN" );

      BOOST_REQUIRE( bob_asset.issuer == NULL_ACCOUNT );
      BOOST_REQUIRE( bob_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( bob_asset.asset_type == asset_property_type::CURRENCY_ASSET );
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

      asset_update.signatory = "bob";
      asset_update.issuer = "bob";
      asset_update.asset_to_update = "BOBCOIN";
      asset_update.new_options.display_symbol = "BobCoin";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "https://www.newurl.com";
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

      tx.operations.push_back( account_create );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      account_business_operation account_business;

      account_business.signatory = "tropico";
      account_business.account = "tropico";
      account_business.init_ceo_account = "candice";
      account_business.business_type = "open";
      account_business.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      account_business.business_public_key = string( candice_public_connection_key );
      account_business.validate();

      tx.operations.push_back( account_business );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_business_object& tropico_business = db.get_account_business( "tropico" );

      BOOST_REQUIRE( tropico_business.account == account_business.account );
      BOOST_REQUIRE( tropico_business.executive_board.CHIEF_EXECUTIVE_OFFICER == account_business.init_ceo_account );
      BOOST_REQUIRE( tropico_business.business_type == business_structure_type::OPEN_BUSINESS );
      
      asset_create.signatory = "tropico";
      asset_create.issuer = "tropico";
      asset_create.symbol = "TROPICO";
      asset_create.asset_type = "equity";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "TROPICO" );
      asset_create.options.display_symbol = "Tropico";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& candice_asset = db.get_asset( "TROPICO" );

      BOOST_REQUIRE( candice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( candice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( candice_asset.asset_type == asset_property_type::EQUITY_ASSET );
      BOOST_REQUIRE( candice_asset.created == now() );
      BOOST_REQUIRE( candice_asset.last_updated == now() );

      const asset_equity_data_object& candice_equity = db.get_equity_data( "TROPICO" );

      BOOST_REQUIRE( candice_equity.business_account == asset_create.issuer );

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
      asset_update.new_options.url = "https://www.newurl.com";
      asset_update.new_options.dividend_share_percent = 10 * PERCENT_1;
      asset_update.new_options.liquid_dividend_percent = 20 * PERCENT_1;
      asset_update.new_options.savings_dividend_percent = 20 * PERCENT_1;
      asset_update.new_options.staked_dividend_percent = 60 * PERCENT_1;
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( candice_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( candice_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( candice_asset.asset_type == asset_property_type::EQUITY_ASSET );
      BOOST_REQUIRE( to_string( candice_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( candice_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( candice_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( candice_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( candice_asset.last_updated == now() );

      BOOST_REQUIRE( candice_equity.dividend_share_percent == asset_update.new_options.dividend_share_percent );
      BOOST_REQUIRE( candice_equity.liquid_dividend_percent == asset_update.new_options.liquid_dividend_percent );
      BOOST_REQUIRE( candice_equity.savings_dividend_percent == asset_update.new_options.savings_dividend_percent );
      BOOST_REQUIRE( candice_equity.staked_dividend_percent == asset_update.new_options.staked_dividend_percent );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create bond asset" );

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

      tx.operations.push_back( account_create );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_business.signatory = "blocktwo";
      account_business.account = "blocktwo";
      account_business.init_ceo_account = "dan";
      account_business.business_type = "open";
      account_business.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      account_business.business_public_key = string( dan_public_connection_key );
      account_business.validate();

      tx.operations.push_back( account_business );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_business_object& blocktwo_business = db.get_account_business( "blocktwo" );

      BOOST_REQUIRE( blocktwo_business.account == account_business.account );
      BOOST_REQUIRE( blocktwo_business.executive_board.CHIEF_EXECUTIVE_OFFICER == account_business.init_ceo_account );
      BOOST_REQUIRE( blocktwo_business.business_type == business_structure_type::OPEN_BUSINESS );

      date_type today = date_type( now() + fc::days( 30 ) );

      asset_create.signatory = "blocktwo";
      asset_create.issuer = "blocktwo";
      asset_create.symbol = "BLOCKBOND";
      asset_create.asset_type = "bond";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "BLOCKBOND" );
      asset_create.options.display_symbol = "Block Two Bonds";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.options.value = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.options.collateralization = 20 * PERCENT_1;
      asset_create.options.coupon_rate_percent = 5 * PERCENT_1;
      asset_create.options.maturity_date = date_type( 1, today.month, today.year );
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& dan_bond_asset = db.get_asset( "BLOCKBOND" );

      BOOST_REQUIRE( dan_bond_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_bond_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_bond_asset.asset_type == asset_property_type::BOND_ASSET );
      BOOST_REQUIRE( dan_bond_asset.created == now() );
      BOOST_REQUIRE( dan_bond_asset.last_updated == now() );

      const asset_bond_data_object& bond_data = db.get_bond_data( "BLOCKBOND" );

      BOOST_REQUIRE( bond_data.business_account == asset_create.issuer );
      BOOST_REQUIRE( bond_data.value == asset_create.options.value );
      BOOST_REQUIRE( bond_data.collateralization == asset_create.options.collateralization );
      BOOST_REQUIRE( bond_data.coupon_rate_percent == asset_create.options.coupon_rate_percent );
      BOOST_REQUIRE( bond_data.maturity_date == asset_create.options.maturity_date );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create bond asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update bond asset" );

      asset_update.signatory = "blocktwo";
      asset_update.issuer = "blocktwo";
      asset_update.asset_to_update = "BLOCKBOND";
      asset_update.new_options.display_symbol = "Block Two Bonds";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "https://www.newurl.com";
      asset_update.new_options.value = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_update.new_options.collateralization = 25 * PERCENT_1;
      asset_update.new_options.coupon_rate_percent = 6 * PERCENT_1;
      asset_update.new_options.maturity_date = date_type( 1, today.month , today.year );
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( dan_bond_asset.issuer == asset_update.issuer );
      BOOST_REQUIRE( dan_bond_asset.symbol == asset_update.asset_to_update );
      BOOST_REQUIRE( dan_bond_asset.asset_type == asset_property_type::BOND_ASSET );
      BOOST_REQUIRE( to_string( dan_bond_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( dan_bond_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( dan_bond_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( dan_bond_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( dan_bond_asset.last_updated == now() );

      BOOST_REQUIRE( bond_data.business_account == asset_update.issuer );
      BOOST_REQUIRE( bond_data.value == asset_update.new_options.value );
      BOOST_REQUIRE( bond_data.collateralization == asset_update.new_options.collateralization );
      BOOST_REQUIRE( bond_data.coupon_rate_percent == asset_update.new_options.coupon_rate_percent );
      BOOST_REQUIRE( bond_data.maturity_date == asset_update.new_options.maturity_date );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update bond asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create credit asset" );

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

      tx.operations.push_back( account_create );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset_create.signatory = "blocktwo";
      asset_create.issuer = "blocktwo";
      asset_create.symbol = "BLOCKTWO";
      asset_create.asset_type = "credit";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "BLOCKTWO" );
      asset_create.options.display_symbol = "Block Two";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& dan_asset = db.get_asset( "BLOCKTWO" );

      BOOST_REQUIRE( dan_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_asset.asset_type == asset_property_type::CREDIT_ASSET );
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
      asset_update.new_options.url = "https://www.newurl.com";
      asset_update.new_options.buyback_share_percent = 10 * PERCENT_1;
      asset_update.new_options.liquid_fixed_interest_rate = 2 * PERCENT_1;
      asset_update.new_options.liquid_variable_interest_rate = 3 * PERCENT_1;
      asset_update.new_options.staked_fixed_interest_rate = 2 * PERCENT_1;
      asset_update.new_options.staked_variable_interest_rate = 12 * PERCENT_1;
      asset_update.new_options.savings_fixed_interest_rate = 2 * PERCENT_1;
      asset_update.new_options.savings_variable_interest_rate = 3 * PERCENT_1;
      asset_update.new_options.var_interest_range = 30 * PERCENT_1;
      asset_update.validate();

      tx.operations.push_back( asset_update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( dan_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( dan_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( dan_asset.asset_type == asset_property_type::CREDIT_ASSET );
      BOOST_REQUIRE( to_string( dan_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( dan_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( dan_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( dan_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( dan_asset.last_updated == now() );

      BOOST_REQUIRE( dan_credit.buyback_share_percent == asset_update.new_options.buyback_share_percent );
      BOOST_REQUIRE( dan_credit.liquid_fixed_interest_rate == asset_update.new_options.liquid_fixed_interest_rate );
      BOOST_REQUIRE( dan_credit.liquid_variable_interest_rate == asset_update.new_options.liquid_variable_interest_rate );
      BOOST_REQUIRE( dan_credit.staked_fixed_interest_rate == asset_update.new_options.staked_fixed_interest_rate );
      BOOST_REQUIRE( dan_credit.staked_variable_interest_rate == asset_update.new_options.staked_variable_interest_rate );
      BOOST_REQUIRE( dan_credit.savings_fixed_interest_rate == asset_update.new_options.savings_fixed_interest_rate );
      BOOST_REQUIRE( dan_credit.savings_variable_interest_rate == asset_update.new_options.savings_variable_interest_rate );
      BOOST_REQUIRE( dan_credit.var_interest_range == asset_update.new_options.var_interest_range );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create stablecoin" );

      asset_create.signatory = "elon";
      asset_create.issuer = "elon";
      asset_create.symbol = "TSLA";
      asset_create.asset_type = "stablecoin";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "TSLA" );
      asset_create.options.display_symbol = "Tesla";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& elon_asset = db.get_asset( "TSLA" );

      BOOST_REQUIRE( elon_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( elon_asset.asset_type == asset_property_type::STABLECOIN_ASSET );
      BOOST_REQUIRE( elon_asset.created == now() );
      BOOST_REQUIRE( elon_asset.last_updated == now() );

      const asset_stablecoin_data_object& elon_stablecoin = db.get_stablecoin_data( "TSLA" );

      BOOST_REQUIRE( elon_stablecoin.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_stablecoin.backing_asset == SYMBOL_COIN );

      const asset_credit_pool_object& elon_credit_pool = db.get_credit_pool( "TSLA", false );

      BOOST_REQUIRE( elon_credit_pool.base_balance == asset_create.credit_liquidity );
      BOOST_REQUIRE( elon_credit_pool.borrowed_balance.amount == 0 );

      const asset_liquidity_pool_object& elon_liquidity_pool = db.get_liquidity_pool( SYMBOL_COIN, "TSLA" );

      BOOST_REQUIRE( elon_liquidity_pool.symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( elon_liquidity_pool.symbol_b == "TSLA" );
      BOOST_REQUIRE( elon_liquidity_pool.balance_a == asset_create.coin_liquidity );
      BOOST_REQUIRE( elon_liquidity_pool.balance_b == asset( asset_create.coin_liquidity.amount, "TSLA" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create stablecoin" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update stablecoin" );

      asset_update.signatory = "elon";
      asset_update.issuer = "elon";
      asset_update.asset_to_update = "TSLA";
      asset_update.new_options.display_symbol = "Tesla";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "https://www.newurl.com";
      asset_update.new_options.backing_asset = SYMBOL_COIN;
      asset_update.new_options.feed_lifetime = fc::days(2);
      asset_update.new_options.minimum_feeds = 3;
      asset_update.new_options.asset_settlement_delay = fc::hours(6);
      asset_update.new_options.asset_settlement_offset_percent = 3 * PERCENT_1;
      asset_update.new_options.maximum_asset_settlement_volume = 10 * PERCENT_1;

      tx.operations.push_back( asset_update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( elon_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( elon_asset.asset_type == asset_property_type::STABLECOIN_ASSET );
      BOOST_REQUIRE( to_string( elon_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( elon_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( elon_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( elon_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( elon_asset.last_updated == now() );

      BOOST_REQUIRE( elon_stablecoin.backing_asset == asset_update.new_options.backing_asset );
      BOOST_REQUIRE( elon_stablecoin.feed_lifetime == asset_update.new_options.feed_lifetime );
      BOOST_REQUIRE( elon_stablecoin.minimum_feeds == asset_update.new_options.minimum_feeds );
      BOOST_REQUIRE( elon_stablecoin.asset_settlement_delay == asset_update.new_options.asset_settlement_delay );
      BOOST_REQUIRE( elon_stablecoin.asset_settlement_offset_percent == asset_update.new_options.asset_settlement_offset_percent );
      BOOST_REQUIRE( elon_stablecoin.maximum_asset_settlement_volume == asset_update.new_options.maximum_asset_settlement_volume );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update stablecoin" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create stimulus asset" );

      asset_create.signatory = "elon";
      asset_create.issuer = "elon";
      asset_create.symbol = "STIM";
      asset_create.asset_type = "stimulus";
      asset_create.coin_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 1000 * BLOCKCHAIN_PRECISION, "STIM" );
      asset_create.options.display_symbol = "StimCoin";
      asset_create.options.details = "Details";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.options.redemption_asset = SYMBOL_USD;
      asset_create.options.redemption_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, "STIM" ) );
      asset_create.options.distribution_list = { "alice", "bob", "candice" };
      asset_create.options.redemption_list = { "tropico" };
      asset_create.options.distribution_amount = asset( 10 * BLOCKCHAIN_PRECISION, "STIM" );

      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& elon_stim_asset = db.get_asset( "STIM" );

      BOOST_REQUIRE( elon_stim_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_stim_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( elon_stim_asset.asset_type == asset_property_type::STIMULUS_ASSET );
      BOOST_REQUIRE( elon_stim_asset.created == now() );
      BOOST_REQUIRE( elon_stim_asset.last_updated == now() );

      const asset_stimulus_data_object& elon_stimulus = db.get_stimulus_data( "STIM" );

      BOOST_REQUIRE( elon_stimulus.business_account == asset_create.issuer );
      BOOST_REQUIRE( elon_stimulus.redemption_asset == SYMBOL_USD );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create stimulus asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update stimulus asset" );

      asset_update.signatory = "elon";
      asset_update.issuer = "elon";
      asset_update.asset_to_update = "STIM";
      asset_update.new_options.display_symbol = "StimCoin";
      asset_update.new_options.details = "New Details";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "https://www.newurl.com";
      asset_update.new_options.redemption_asset = SYMBOL_USD;
      asset_update.new_options.redemption_price = price( asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ), asset( BLOCKCHAIN_PRECISION, "STIM" ) );
      asset_update.new_options.distribution_list = { "alice", "bob", "candice", "elon" };
      asset_update.new_options.redemption_list = { "blocktwo", "tropico" };
      asset_update.new_options.distribution_amount = asset( 20 * BLOCKCHAIN_PRECISION, "STIM" );

      tx.operations.push_back( asset_update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( elon_stim_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( elon_stim_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( elon_stim_asset.asset_type == asset_property_type::STIMULUS_ASSET );
      BOOST_REQUIRE( to_string( elon_stim_asset.display_symbol ) == asset_update.new_options.display_symbol );
      BOOST_REQUIRE( to_string( elon_stim_asset.details ) == asset_update.new_options.details );
      BOOST_REQUIRE( to_string( elon_stim_asset.json ) == asset_update.new_options.json );
      BOOST_REQUIRE( to_string( elon_stim_asset.url ) == asset_update.new_options.url );
      BOOST_REQUIRE( elon_stim_asset.last_updated == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update stimulus asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create Unique asset" );

      asset_create.signatory = "tropico";
      asset_create.issuer = "tropico";
      asset_create.symbol = "MANSION";
      asset_create.asset_type = "unique";
      asset_create.coin_liquidity = asset( 0 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      asset_create.usd_liquidity = asset( 0 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.credit_liquidity = asset( 0 * BLOCKCHAIN_PRECISION, "MANSION" );
      asset_create.options.display_symbol = "Title Deed of The Tropico Mansion Resort";
      asset_create.options.details = "Access list enables entry to Tropico Mansion Resort.";
      asset_create.options.json = "{ \"valid\": true }";
      asset_create.options.url = "https://www.url.com";
      asset_create.options.ownership_asset = "TROPICO";
      asset_create.options.control_list = { "candice", "tropico" };
      asset_create.options.access_list = { "alice", "bob" };
      asset_create.options.access_price = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      asset_create.validate();

      tx.operations.push_back( asset_create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_object& candice_unique_asset = db.get_asset( "MANSION" );

      BOOST_REQUIRE( candice_unique_asset.issuer == asset_create.issuer );
      BOOST_REQUIRE( candice_unique_asset.symbol == asset_create.symbol );
      BOOST_REQUIRE( candice_unique_asset.asset_type == asset_property_type::UNIQUE_ASSET );
      BOOST_REQUIRE( candice_unique_asset.created == now() );
      BOOST_REQUIRE( candice_unique_asset.last_updated == now() );

      const asset_unique_data_object& mansion_unique_data = db.get_unique_data( "MANSION" );

      BOOST_REQUIRE( mansion_unique_data.controlling_owner == asset_create.issuer );
      BOOST_REQUIRE( mansion_unique_data.ownership_asset == asset_create.options.ownership_asset );
      BOOST_REQUIRE( mansion_unique_data.access_price == asset_create.options.access_price );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create Unique asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Update Unique asset" );

      asset_update.signatory = "tropico";
      asset_update.issuer = "tropico";
      asset_update.asset_to_update = "MANSION";
      asset_update.new_options.display_symbol = "StimCoin";
      asset_update.new_options.details = "Title Deed of The Tropico Mansion Resort Complex";
      asset_update.new_options.json = "{\"json\":\"supervalid\"}";
      asset_update.new_options.url = "https://www.newurl.com";
      asset_update.new_options.ownership_asset = "TROPICO";
      asset_update.new_options.control_list = { "candice", "tropico" };
      asset_update.new_options.access_list = { "alice", "bob", "dan", "elon" };
      asset_update.new_options.access_price = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( asset_update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( candice_unique_asset.issuer == asset_update.issuer );
      BOOST_REQUIRE( candice_unique_asset.symbol == asset_update.asset_to_update );
      BOOST_REQUIRE( candice_unique_asset.asset_type == asset_property_type::UNIQUE_ASSET );
      BOOST_REQUIRE( candice_unique_asset.created == now() );
      BOOST_REQUIRE( candice_unique_asset.last_updated == now() );

      BOOST_REQUIRE( mansion_unique_data.controlling_owner == asset_update.issuer );
      BOOST_REQUIRE( mansion_unique_data.ownership_asset == asset_update.new_options.ownership_asset );
      BOOST_REQUIRE( mansion_unique_data.access_price == asset_update.new_options.access_price );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update Unique asset" );

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

      asset alice_liquid = db.get_liquid_balance( "alice", "ALICECOIN" );

      const asset_dynamic_data_object& alice_dyn_data = db.get_dynamic_data( "ALICECOIN" );

      BOOST_REQUIRE( alice_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( alice_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( alice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Standard asset" );

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

      alice_liquid = db.get_liquid_balance( "alice", "ALICECOIN" );

      BOOST_REQUIRE( alice_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( alice_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( alice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Standard asset" );

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

      asset candice_liquid = db.get_liquid_balance( "candice", "TROPICO" );

      const asset_dynamic_data_object& candice_dyn_data = db.get_dynamic_data( "TROPICO" );

      BOOST_REQUIRE( candice_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( candice_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( candice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Equity asset" );

      asset_reserve.signatory = "candice";
      asset_reserve.payer = "candice";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, "TROPICO" );

      tx.operations.push_back( asset_reserve );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      candice_liquid = db.get_liquid_balance( "candice", "TROPICO" );

      BOOST_REQUIRE( candice_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( candice_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( candice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Unique asset" );

      asset_issue.signatory = "tropico";
      asset_issue.issuer = "tropico";
      asset_issue.asset_to_issue = asset( BLOCKCHAIN_PRECISION, "MANSION" );
      asset_issue.issue_to_account = "candice";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset mansion_liquid = db.get_liquid_balance( "candice", "MANSION" );

      const asset_dynamic_data_object& mansion_dyn_data = db.get_dynamic_data( "MANSION" );

      BOOST_REQUIRE( mansion_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( mansion_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( mansion_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( mansion_unique_data.controlling_owner == asset_issue.issue_to_account );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Unique asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Equity asset" );

      asset_reserve.signatory = "candice";
      asset_reserve.payer = "candice";
      asset_reserve.amount_to_reserve = asset( BLOCKCHAIN_PRECISION, "MANSION" );

      tx.operations.push_back( asset_reserve );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      candice_liquid = db.get_liquid_balance( "candice", "TROPICO" );

      BOOST_REQUIRE( candice_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( candice_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( candice_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Equity asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Issue Bond asset" );

      asset_issue.signatory = "blocktwo";
      asset_issue.issuer = "blocktwo";
      asset_issue.asset_to_issue = asset( 1000 * BLOCKCHAIN_PRECISION, "BLOCKBOND" );
      asset_issue.issue_to_account = "dan";
      asset_issue.memo = "Hello";
      asset_issue.validate();

      tx.operations.push_back( asset_issue );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset dan_liquid = db.get_liquid_balance( "dan", "BLOCKBOND" );

      const asset_dynamic_data_object& dan_bond_dyn_data = db.get_dynamic_data( "BLOCKBOND" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( dan_bond_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( dan_bond_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Bond asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Bond asset" );

      asset_reserve.signatory = "dan";
      asset_reserve.payer = "dan";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, "BLOCKBOND" );

      tx.operations.push_back( asset_reserve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      dan_liquid = db.get_liquid_balance( "dan", "BLOCKBOND" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( dan_bond_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( dan_bond_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Bond asset" );

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

      dan_liquid = db.get_liquid_balance( "dan", "BLOCKTWO" );

      const asset_dynamic_data_object& dan_credit_dyn_data = db.get_dynamic_data( "BLOCKTWO" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue );
      BOOST_REQUIRE( dan_credit_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount );
      BOOST_REQUIRE( dan_credit_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Issue Credit asset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reserve Credit asset" );

      asset_reserve.signatory = "dan";
      asset_reserve.payer = "dan";
      asset_reserve.amount_to_reserve = asset( 500 * BLOCKCHAIN_PRECISION, "BLOCKTWO" );

      tx.operations.push_back( asset_reserve );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      dan_liquid = db.get_liquid_balance( "dan", "BLOCKTWO" );

      BOOST_REQUIRE( dan_liquid == asset_issue.asset_to_issue - asset_reserve.amount_to_reserve );
      BOOST_REQUIRE( dan_credit_dyn_data.get_total_supply().amount == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );
      BOOST_REQUIRE( dan_credit_dyn_data.liquid_supply == asset_issue.asset_to_issue.amount - asset_reserve.amount_to_reserve.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reserve Credit asset" );

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
      asset_unit input = asset_unit( account_name_type( "alice" ), 100000000, string( "liquid" ), GENESIS_TIME );
      distribution.input_fund_unit = { input };
      asset_unit output = asset_unit( account_name_type( "sender" ), 1, string( "liquid" ), GENESIS_TIME );
      distribution.output_distribution_unit = { output };
      distribution.min_unit_ratio = 1000000;
      distribution.max_unit_ratio = 1000000000000;
      distribution.min_input_balance_units = 1;
      distribution.max_input_balance_units = 1000000;
      distribution.begin_time = now() + fc::days(31);
      distribution.validate();

      tx.operations.push_back( distribution );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_distribution_object& alice_distribution = db.get_asset_distribution( "ALICECOIN" );

      BOOST_REQUIRE( distribution.issuer == alice_distribution.issuer );
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
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create Asset distribution" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Fund Asset Distribution" );

      asset_distribution_fund_operation fund;

      fund.signatory = "bob";
      fund.sender = "bob";
      fund.distribution_asset = "ALICECOIN";
      fund.amount = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      fund.validate();

      tx.operations.push_back( fund );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      fund.sender = "candice";

      tx.operations.push_back( fund );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_distribution_balance_object& bob_distribution_balance = db.get_asset_distribution_balance( "bob", "ALICECOIN" );
      const asset_distribution_balance_object& candice_distribution_balance = db.get_asset_distribution_balance( "candice", "ALICECOIN" );

      BOOST_REQUIRE( bob_distribution_balance.sender == "bob" );
      BOOST_REQUIRE( bob_distribution_balance.distribution_asset == SYMBOL_USD );
      BOOST_REQUIRE( bob_distribution_balance.amount == asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      BOOST_REQUIRE( candice_distribution_balance.sender == "candice" );
      BOOST_REQUIRE( candice_distribution_balance.distribution_asset == SYMBOL_USD );
      BOOST_REQUIRE( candice_distribution_balance.amount == asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      validate_database();

      generate_blocks( alice_distribution.next_round_time + fc::minutes(1) );

      asset bob_alicecoin_balance = db.get_liquid_balance( "bob", "ALICECOIN" );
      asset candice_alicecoin_balance = db.get_liquid_balance( "candice", "ALICECOIN" );

      BOOST_REQUIRE( alice_distribution.intervals_paid == 1 );
      BOOST_REQUIRE( alice_distribution.next_round_time == distribution.begin_time + fc::days( distribution.distribution_interval_days * 2 ) );
      BOOST_REQUIRE( alice_distribution.total_funded == bob_distribution_balance.amount + candice_distribution_balance.amount );
      BOOST_REQUIRE( alice_distribution.total_distributed == bob_alicecoin_balance + candice_alicecoin_balance );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Fund Asset distribution" );

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

      BOOST_REQUIRE( alice_asset.issuer == asset_update_issuer.new_issuer );

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

      asset bob_liquid = db.get_liquid_balance( "bob", "ALICECOIN" );

      BOOST_REQUIRE( bob_liquid == asset_issue.asset_to_issue );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: New Issuer issuing asset to own account" );

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

      BOOST_REQUIRE( elon_asset.last_updated == now() );

      for( auto f : update_feed.new_feed_producers )
      {
         BOOST_REQUIRE( elon_stablecoin.feeds.count( f ) );
      }
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Update Stablecoin feed producers" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Publish stablecoin price feed" );

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

      BOOST_REQUIRE( elon_stablecoin.current_feed == feed.feed );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Publish stablecoin price feed" );

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

      const asset_settlement_object& fred_settlement = db.get_asset_settlement( "fred", "TSLA" );

      BOOST_REQUIRE( fred_settlement.balance == settle.amount );
      BOOST_REQUIRE( fred_settlement.last_updated == now() );
      BOOST_REQUIRE( fred_settlement.created == now() );

      generate_blocks( fred_settlement.settlement_date + BLOCK_INTERVAL );

      const auto& settle_idx = db.get_index< asset_settlement_index >().indices().get< by_account_asset >();
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

      BOOST_REQUIRE( elon_stablecoin.has_settlement() );
      BOOST_REQUIRE( elon_stablecoin.settlement_fund.value > 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Asset Global settlement" );

      BOOST_TEST_MESSAGE( "├── Testing: ASSET OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( asset_collateral_bid_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: BID COLLATERAL" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: creation of collateral bid after black swan event" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "bob", bob_private_owner_key, bob_public_owner_key );
      
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "candice", candice_private_owner_key, candice_public_owner_key );

      fund( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      fund_stake( "dan", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      producer_create( "dan", dan_private_owner_key, dan_public_owner_key );
      
      signed_transaction tx;

      asset_publish_feed_operation feed;     // Begin by triggering a black swan event on the USD asset

      feed.signatory = "alice";
      feed.publisher = "alice";
      feed.symbol = SYMBOL_USD;
      feed.feed.settlement_price = price( asset(1,SYMBOL_USD),asset(1,SYMBOL_COIN) );   // init settlement price of 1:1
      feed.validate();

      tx.operations.push_back( feed );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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

      call_order_operation call;    // Spread of collateralized call orders

      call.signatory = "alice";
      call.owner = "alice";
      call.collateral = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );  // 2x collateralization
      call.debt = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      call.interface = INIT_ACCOUNT;
      call.validate();

      tx.operations.push_back( call );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      call.signatory = "bob";
      call.owner = "bob";
      call.collateral = asset( 15000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );   // 3x collateralization
      call.debt = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( call );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      call.signatory = "candice";
      call.owner = "candice";
      call.collateral = asset( 20000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );  // 4x collateralization
      call.debt = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( call );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      call.signatory = "dan";
      call.owner = "dan";
      call.collateral = asset( 25000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );  // 5x collateralization
      call.debt = asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( call );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      feed.signatory = "alice";
      feed.publisher = "alice";
      feed.feed.settlement_price = price( asset(1,SYMBOL_USD),asset(2,SYMBOL_COIN) );   // New settlement price of 1:2

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

      limit_order_operation limit;

      limit.signatory = "alice";
      limit.owner = "alice";
      limit.order_id = "88d551cd-0dc2-46f1-a09c-7d0cd477b550";
      limit.amount_to_sell = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      limit.exchange_rate = price( asset( 1, SYMBOL_USD ), asset( 3, SYMBOL_COIN ) );   // USD sell order at 1:3, within SQR
      limit.expiration = now() + fc::days( 30 );
      limit.interface = INIT_ACCOUNT;
      limit.fill_or_kill = false;
      limit.opened = true;

      tx.operations.push_back( limit );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_stablecoin_data_object& stablecoin = db.get_stablecoin_data( SYMBOL_USD );

      BOOST_REQUIRE( stablecoin.has_settlement() );     // Stablecoin has now undergone a black swan event

      asset_collateral_bid_operation bid;

      bid.signatory = "alice";
      bid.bidder = "alice";
      bid.collateral = asset( 20000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      bid.validate();

      tx.operations.push_back( bid );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_collateral_bid_object& alice_col_bid = db.get_asset_collateral_bid( "alice", SYMBOL_USD );

      BOOST_REQUIRE( alice_col_bid.bidder == bid.bidder );
      BOOST_REQUIRE( alice_col_bid.collateral == bid.collateral );
      BOOST_REQUIRE( alice_col_bid.debt == bid.debt );
      BOOST_REQUIRE( alice_col_bid.created == now() );
      BOOST_REQUIRE( alice_col_bid.last_updated == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: creation of collateral bid after black swan event" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account does not have required funds" );

      bid.signatory = "bob";
      bid.bidder = "bob";
      bid.collateral = asset( 1000000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 20000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );
      
      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account does not have required funds" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when collateral is 0" );

      bid.signatory = "bob";
      bid.bidder = "bob";
      bid.collateral = asset( 0, SYMBOL_COIN );
      bid.debt = asset( 20000, SYMBOL_USD );
      
      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when debt and collateral amount is 0" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reviving asset with additional collateral bids" );

      bid.signatory = "bob";
      bid.bidder = "bob";
      bid.collateral = asset( 20000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.debt = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD );

      tx.operations.push_back( bid );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_collateral_bid_object& bob_col_bid = db.get_asset_collateral_bid( "bob", SYMBOL_USD );

      BOOST_REQUIRE( bob_col_bid.bidder == bid.bidder );
      BOOST_REQUIRE( bob_col_bid.collateral == bid.collateral );
      BOOST_REQUIRE( bob_col_bid.debt == bid.debt );
      BOOST_REQUIRE( bob_col_bid.created == now() );
      BOOST_REQUIRE( bob_col_bid.last_updated == now() );
      
      generate_blocks( STABLECOIN_BLOCK_INTERVAL + 1 );

      BOOST_REQUIRE( stablecoin.has_settlement() );  // Requires price feed before revival

      generate_blocks( STABLECOIN_BLOCK_INTERVAL - 1 );

      feed.signatory = "alice";
      feed.publisher = "alice";
      feed.feed.settlement_price = price( asset(1,SYMBOL_USD),asset(2,SYMBOL_COIN) );   // New settlement price of 1:2 Collateral value falls 50%

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

      generate_block();

      BOOST_REQUIRE( !stablecoin.has_settlement() );  // Asset is now revived.

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reviving asset with additional collateral bids" );

      BOOST_TEST_MESSAGE( "├── Testing: BID COLLATERAL" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()