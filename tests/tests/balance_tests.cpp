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

BOOST_FIXTURE_TEST_SUITE( balance_operation_tests, clean_database_fixture );



   //=======================//
   // === Balance Tests === //
   //=======================//



BOOST_AUTO_TEST_CASE( claim_reward_balance_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CLAIM REWARD OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful reward claim" );

      private_key_type producer_private_active_key = get_private_key( GENESIS_ACCOUNT_BASE_NAME, ACTIVE_KEY_STR, INIT_PASSWORD );

      generate_blocks( TOTAL_PRODUCERS );

      asset producer_init_liquid_balance = get_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      asset producer_init_staked_balance = get_staked_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      asset producer_init_reward_balance = get_reward_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );

      signed_transaction tx;

      claim_reward_balance_operation claim;

      claim.account = GENESIS_ACCOUNT_BASE_NAME;
      claim.reward = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      claim.validate();

      tx.operations.push_back( claim );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( producer_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset producer_liquid_balance = get_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      asset producer_staked_balance = get_staked_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      asset producer_reward_balance = get_reward_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      
      BOOST_REQUIRE( producer_liquid_balance + producer_staked_balance + producer_reward_balance == 
         producer_init_liquid_balance + producer_init_staked_balance + producer_init_reward_balance );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful reward claim" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when reward claim is higher than balance" );
   
      claim.reward = asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( claim );
      tx.sign( producer_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when reward claim is higher than balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: claiming entire reward balance" );

      producer_init_liquid_balance = get_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      producer_init_staked_balance = get_staked_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      producer_init_reward_balance = get_reward_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );

      claim.reward = producer_init_reward_balance;

      tx.operations.push_back( claim );
      tx.sign( producer_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      tx.operations.clear();
      tx.signatures.clear();

      producer_liquid_balance = get_liquid_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      producer_staked_balance = get_staked_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      producer_reward_balance = get_reward_balance( GENESIS_ACCOUNT_BASE_NAME, SYMBOL_COIN );
      
      BOOST_REQUIRE( producer_liquid_balance + producer_staked_balance == 
         producer_init_liquid_balance + producer_init_staked_balance + producer_init_reward_balance );
      BOOST_REQUIRE( producer_reward_balance.amount == 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: claiming entire reward balance" );

      BOOST_TEST_MESSAGE( "├── Passed: CLAIM REWARD OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( stake_asset_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: STAKE ASSET OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful asset stake" );

      ACTORS( (alice)(bob)(candice) );

      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_staked_balance = get_staked_balance( "bob", SYMBOL_COIN );

      stake_asset_operation stake;

      stake.from = "alice";
      stake.to = "bob";
      stake.amount = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      stake.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( stake );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_staked_balance = get_staked_balance( "alice", SYMBOL_COIN );
      asset bob_staked_balance = get_staked_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( bob_staked_balance - bob_init_staked_balance == stake.amount );
      BOOST_REQUIRE( alice_init_liquid_balance - alice_liquid_balance == stake.amount );

      stake.from = "alice";
      stake.to = "alice";
      stake.amount = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY );
      stake.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( stake );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      alice_init_liquid_balance = alice_liquid_balance;
      asset alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_EQUITY );
      alice_staked_balance = get_staked_balance( "bob", SYMBOL_EQUITY );

      const account_balance_object& alice_balance = db.get_account_balance( account_name_type( "alice" ), SYMBOL_EQUITY );

      BOOST_REQUIRE( alice_balance.stake_rate == stake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_stake == stake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_staked == 0 );

      generate_blocks( alice_balance.next_stake_time, true );
      generate_block();

      alice_init_liquid_balance = alice_liquid_balance;
      alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_EQUITY );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_EQUITY );

      BOOST_REQUIRE( alice_init_liquid_balance - alice_liquid_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_staked_balance - alice_init_staked_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_balance.stake_rate == stake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_stake == stake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_staked == stake.amount.amount / 4 );

      generate_blocks( alice_balance.next_stake_time, true );
      generate_block();

      alice_init_liquid_balance = alice_liquid_balance;
      alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_EQUITY );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_EQUITY );

      BOOST_REQUIRE( alice_init_liquid_balance - alice_liquid_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_staked_balance - alice_init_staked_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_balance.stake_rate == stake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_stake == stake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_staked == stake.amount.amount / 2 );

      generate_blocks( alice_balance.next_stake_time, true );
      generate_block();

      alice_init_liquid_balance = alice_liquid_balance;
      alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_EQUITY );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_EQUITY );

      BOOST_REQUIRE( alice_init_liquid_balance - alice_liquid_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_staked_balance - alice_init_staked_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_balance.stake_rate == stake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_stake == stake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_staked == 3 * ( stake.amount.amount / 4 ) );

      generate_blocks( alice_balance.next_stake_time, true );
      generate_block();

      alice_init_liquid_balance = alice_liquid_balance;
      alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_EQUITY );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_EQUITY );

      BOOST_REQUIRE( alice_init_liquid_balance - alice_liquid_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_staked_balance - alice_init_staked_balance == stake.amount / 4 );
      BOOST_REQUIRE( alice_balance.stake_rate == 0 );
      BOOST_REQUIRE( alice_balance.next_stake_time == fc::time_point::maximum() );
      BOOST_REQUIRE( alice_balance.to_stake == 0 );
      BOOST_REQUIRE( alice_balance.total_staked == 0 );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful asset stake" );

      BOOST_TEST_MESSAGE( "├── Passed: STAKE ASSET OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( unstake_asset_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: UNSTAKE ASSET OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful asset unstake" );

      ACTORS( (alice)(bob)(candice) );

      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      unstake_asset_operation unstake;

      unstake.from = "alice";
      unstake.to = "alice";
      unstake.amount = asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );     // Unstake entire balance
      unstake.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( unstake );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_staked_balance = get_staked_balance( "alice", SYMBOL_COIN );
      
      const account_balance_object& alice_balance = db.get_account_balance( account_name_type( "alice" ), SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.unstake_rate == unstake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_unstake == unstake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == 0 );

      generate_blocks( alice_balance.next_unstake_time, true );
      generate_block();

      asset alice_init_liquid_balance = alice_liquid_balance;
      asset alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_init_staked_balance - alice_staked_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_liquid_balance - alice_init_liquid_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_balance.unstake_rate == unstake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_unstake == unstake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == unstake.amount.amount / 4 );

      generate_blocks( alice_balance.next_unstake_time, true );
      generate_block();

      alice_init_liquid_balance = alice_liquid_balance;
      alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_init_staked_balance - alice_staked_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_liquid_balance - alice_init_liquid_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_balance.unstake_rate == unstake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_unstake == unstake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == unstake.amount.amount / 2 );

      generate_blocks( alice_balance.next_unstake_time, true );
      generate_block();

      alice_init_liquid_balance = alice_liquid_balance;
      alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_init_staked_balance - alice_staked_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_liquid_balance - alice_init_liquid_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_balance.unstake_rate == unstake.amount.amount / 4 );
      BOOST_REQUIRE( alice_balance.to_unstake == unstake.amount.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == 3 * ( unstake.amount.amount / 4 ) );

      generate_blocks( alice_balance.next_unstake_time, true );
      generate_block();

      alice_init_liquid_balance = alice_liquid_balance;
      alice_init_staked_balance = alice_staked_balance;
      alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      alice_staked_balance = get_staked_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_init_staked_balance - alice_staked_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_liquid_balance - alice_init_liquid_balance == unstake.amount / 4 );
      BOOST_REQUIRE( alice_balance.unstake_rate == 0 );
      BOOST_REQUIRE( alice_balance.next_unstake_time == fc::time_point::maximum() );
      BOOST_REQUIRE( alice_balance.to_unstake == 0 );
      BOOST_REQUIRE( alice_balance.total_unstaked == 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful asset unstake" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: asset unstake routes" );

      asset bob_init_liquid_balance = get_liquid_balance( account_name_type( "bob" ), SYMBOL_COIN );
      asset bob_init_staked_balance = get_staked_balance( account_name_type( "bob" ), SYMBOL_COIN );
      asset candice_init_liquid_balance = get_liquid_balance( account_name_type( "candice" ), SYMBOL_COIN );

      unstake_asset_route_operation route;

      route.from = "bob";
      route.to = "candice";
      route.percent = 50 * PERCENT_1;
      route.auto_stake = false;
      route.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( route );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& route_idx = db.get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
      auto route_itr = route_idx.find( boost::make_tuple( route.from, route.to ) );
      const unstake_asset_route_object& asset_route = *route_itr;

      BOOST_REQUIRE( asset_route.from == route.from );
      BOOST_REQUIRE( asset_route.to == route.to );
      BOOST_REQUIRE( asset_route.percent == route.percent );

      unstake.from = "bob";
      unstake.to = "bob";
      unstake.amount = bob_init_staked_balance;
      unstake.validate();

      tx.operations.push_back( unstake );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& bob_balance = db.get_account_balance( account_name_type( "bob" ), SYMBOL_COIN );

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == 0 );

      generate_blocks( bob_balance.next_unstake_time, true );
      generate_block();

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == bob_init_staked_balance.amount / 4 );

      generate_blocks( bob_balance.next_unstake_time, true );
      generate_block();

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == bob_init_staked_balance.amount / 2 );

      generate_blocks( bob_balance.next_unstake_time, true );
      generate_block();

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == 3 * ( bob_init_staked_balance.amount / 4 ) );

      generate_blocks( bob_balance.next_unstake_time, true );
      generate_block();

      BOOST_REQUIRE( bob_balance.unstake_rate == 0 );
      BOOST_REQUIRE( bob_balance.next_unstake_time == fc::time_point::maximum() );
      BOOST_REQUIRE( bob_balance.to_unstake == 0 );
      BOOST_REQUIRE( bob_balance.total_unstaked == 0 );
      BOOST_REQUIRE( bob_balance.liquid_balance == bob_init_liquid_balance.amount + bob_init_staked_balance.amount / 2 );

      const account_balance_object& candice_balance = db.get_account_balance( account_name_type( "candice" ), SYMBOL_COIN );

      BOOST_REQUIRE( candice_balance.liquid_balance == candice_init_liquid_balance.amount + bob_init_staked_balance.amount / 2 );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: asset unstake routes" );

      BOOST_TEST_MESSAGE( "├── Passed: UNSTAKE ASSET OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( transfer_to_savings_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSER TO SAVINGS OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer to own savings" );

      ACTORS( (alice)(bob)(candice) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      asset alice_init_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );
      
      signed_transaction tx;

      transfer_to_savings_operation transfer;

      transfer.from = "alice";
      transfer.to = "alice";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& alice_balance = db.get_account_balance( account_name_type( "alice" ), SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.liquid_balance == alice_init_liquid_balance.amount - transfer.amount.amount );
      BOOST_REQUIRE( alice_balance.savings_balance == transfer.amount.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer to own savings" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer to other account's savings" );

      alice_init_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );

      transfer.to = "bob";

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& bob_balance = db.get_account_balance( account_name_type( "bob" ), SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.liquid_balance == alice_init_liquid_balance.amount - transfer.amount.amount );
      BOOST_REQUIRE( bob_balance.savings_balance == transfer.amount.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer to other account's savings" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure with insufficient liquid funds" );

      transfer.to = "alice";
      transfer.amount = asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure with insufficient liquid funds" );

      BOOST_TEST_MESSAGE( "├── Passed: TRANSER TO SAVINGS OPERATION" );

   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( transfer_from_savings_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSER FROM SAVINGS OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer from own savings" );

      ACTORS( (alice)(bob)(candice) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_savings( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_savings( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_savings( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      asset alice_init_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_savings_balance = get_savings_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );
      
      signed_transaction tx;

      transfer_from_savings_operation transfer;

      transfer.from = "alice";
      transfer.to = "alice";
      transfer.request_id = "e02a03a8-0843-44cb-9700-b2d6d11e24c1";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.transferred = true;
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const savings_withdraw_object& withdraw1 = db.get_savings_withdraw( transfer.from, transfer.request_id );

      BOOST_REQUIRE( withdraw1.from == transfer.from );
      BOOST_REQUIRE( withdraw1.to == transfer.to );
      BOOST_REQUIRE( to_string( withdraw1.memo ) == transfer.memo );
      BOOST_REQUIRE( to_string( withdraw1.request_id ) == transfer.request_id );
      BOOST_REQUIRE( withdraw1.amount == transfer.amount );
      BOOST_REQUIRE( withdraw1.complete == now() + SAVINGS_WITHDRAW_TIME );

      generate_blocks( withdraw1.complete, true );
      generate_block();

      asset alice_liquid_balance = get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_savings_balance = get_savings_balance( "alice", SYMBOL_COIN );

      const auto& withdraw_idx = db.get_index< savings_withdraw_index >().indices().get< by_request_id >();

      auto withdraw_itr = withdraw_idx.find( boost::make_tuple( transfer.from, transfer.request_id ) );

      BOOST_REQUIRE( withdraw_itr == withdraw_idx.end() );

      BOOST_REQUIRE( alice_savings_balance == alice_init_savings_balance - transfer.amount );
      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance + transfer.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer from own savings" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer from savings to another account" );

      alice_init_savings_balance = get_savings_balance( "alice", SYMBOL_COIN );
      bob_init_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );

      transfer.to = "bob";
      transfer.request_id = "9198e9b4-7dac-4856-9f15-c7275735a500";

      tx.operations.push_back( transfer );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const savings_withdraw_object& withdraw2 = db.get_savings_withdraw( transfer.from, transfer.request_id );

      BOOST_REQUIRE( withdraw2.from == transfer.from );
      BOOST_REQUIRE( withdraw2.to == transfer.to );
      BOOST_REQUIRE( to_string( withdraw2.memo ) == transfer.memo );
      BOOST_REQUIRE( to_string( withdraw2.request_id ) == transfer.request_id );
      BOOST_REQUIRE( withdraw2.amount == transfer.amount );
      BOOST_REQUIRE( withdraw2.complete == now() + SAVINGS_WITHDRAW_TIME );

      generate_blocks( withdraw2.complete, true );
      generate_block();
      
      alice_savings_balance = get_savings_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = get_liquid_balance( "bob", SYMBOL_COIN );
      
      BOOST_REQUIRE( alice_savings_balance == alice_init_savings_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer from savings to another account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure with insufficient savings funds" );

      transfer.to = "alice";
      transfer.amount = asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure with insufficient savings funds" );

      BOOST_TEST_MESSAGE( "├── Passed: TRANSER FROM SAVINGS OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( delegate_asset_operations_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: DELEGATE ASSET OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful asset delegation" );

      ACTORS( (alice)(bob)(candice) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      asset alice_init_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );
      asset alice_init_delegated_balance = db.get_delegated_balance( account_name_type( "alice" ), SYMBOL_COIN );
      asset alice_init_receiving_balance = db.get_receiving_balance( account_name_type( "alice" ), SYMBOL_COIN );
      share_type alice_init_voting_power = db.get_voting_power( account_name_type( "alice" ), SYMBOL_COIN  );

      asset bob_init_liquid_balance = get_liquid_balance( account_name_type( "bob" ), SYMBOL_COIN );
      asset bob_init_delegated_balance = db.get_delegated_balance( account_name_type( "bob" ), SYMBOL_COIN );
      asset bob_init_receiving_balance = db.get_receiving_balance( account_name_type( "bob" ), SYMBOL_COIN );
      share_type bob_init_voting_power = db.get_voting_power( account_name_type( "bob" ), SYMBOL_COIN  );
      
      signed_transaction tx;

      delegate_asset_operation delegate;

      delegate.delegator = "alice";
      delegate.delegatee = "bob";
      delegate.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      delegate.validate();

      tx.operations.push_back( delegate );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_delegation_object& delegation = db.get_asset_delegation( account_name_type( "alice" ), account_name_type( "bob" ), SYMBOL_COIN );

      asset alice_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );
      asset alice_delegated_balance = db.get_delegated_balance( account_name_type( "alice" ), SYMBOL_COIN );
      asset alice_receiving_balance = db.get_receiving_balance( account_name_type( "alice" ), SYMBOL_COIN );
      share_type alice_voting_power = db.get_voting_power( account_name_type( "alice" ), SYMBOL_COIN  );

      asset bob_liquid_balance = get_liquid_balance( account_name_type( "bob" ), SYMBOL_COIN );
      asset bob_delegated_balance = db.get_delegated_balance( account_name_type( "bob" ), SYMBOL_COIN );
      asset bob_receiving_balance = db.get_receiving_balance( account_name_type( "bob" ), SYMBOL_COIN );
      share_type bob_voting_power = db.get_voting_power( account_name_type( "bob" ), SYMBOL_COIN  );

      BOOST_REQUIRE( delegation.delegator == delegate.delegator );
      BOOST_REQUIRE( delegation.delegatee == delegate.delegatee );
      BOOST_REQUIRE( delegation.amount == delegate.amount );
      BOOST_REQUIRE( alice_delegated_balance == bob_receiving_balance );
      BOOST_REQUIRE( alice_voting_power == alice_init_voting_power - delegate.amount.amount );
      BOOST_REQUIRE( bob_voting_power == bob_init_voting_power + delegate.amount.amount );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful asset delegation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when delegating more than staked balance" );

      delegate.amount = asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( delegate );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when delegating more than staked balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure delegating staked balance that are being unstaked" );

      alice_init_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_init_delegated_balance = db.get_delegated_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_init_receiving_balance = db.get_receiving_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_init_voting_power = db.get_voting_power( account_name_type( "alice" ), SYMBOL_COIN  );

      unstake_asset_operation withdraw;

      withdraw.from = "alice";
      withdraw.to = "alice";
      withdraw.amount = asset( 9000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( withdraw );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      delegate.amount = asset( 9000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      
      tx.operations.push_back( delegate );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      withdraw.amount = asset( 0, SYMBOL_COIN );

      tx.operations.push_back( withdraw );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
     
      tx.operations.push_back( delegate );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure delegating staked balance that are being unstaked" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure powering down stake that is delegated" );

      withdraw.amount = asset( 90000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( withdraw );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure powering down stake that is delegated" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Remove a delegation" );

      alice_init_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_init_delegated_balance = db.get_delegated_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_init_receiving_balance = db.get_receiving_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_init_voting_power = db.get_voting_power( account_name_type( "alice" ), SYMBOL_COIN  );

      bob_init_liquid_balance = get_liquid_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_init_delegated_balance = db.get_delegated_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_init_receiving_balance = db.get_receiving_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_init_voting_power = db.get_voting_power( account_name_type( "bob" ), SYMBOL_COIN  );

      delegate.amount = asset( 0, SYMBOL_COIN );

      tx.operations.push_back( delegate );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      auto exp_obj = db.get_index< asset_delegation_expiration_index, by_id >().begin();
      auto end = db.get_index< asset_delegation_expiration_index, by_id >().end();

      alice_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_delegated_balance = db.get_delegated_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_receiving_balance = db.get_receiving_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_voting_power = db.get_voting_power( account_name_type( "alice" ), SYMBOL_COIN  );

      bob_liquid_balance = get_liquid_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_delegated_balance = db.get_delegated_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_receiving_balance = db.get_receiving_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_voting_power = db.get_voting_power( account_name_type( "bob" ), SYMBOL_COIN  );

      const auto& delegation_idx = db.get_index< asset_delegation_index >().indices().get< by_delegator >();
      auto delegation_itr = delegation_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "bob" ), SYMBOL_COIN ) );

      BOOST_REQUIRE( delegation_itr == delegation_idx.end() );

      BOOST_REQUIRE( exp_obj != end );
      BOOST_REQUIRE( exp_obj->delegator == delegate.delegator );
      BOOST_REQUIRE( exp_obj->delegatee == delegate.delegatee );
      BOOST_REQUIRE( exp_obj->amount == alice_init_delegated_balance );
      BOOST_REQUIRE( exp_obj->expiration == now() + fc::days(1) );
      
      BOOST_REQUIRE( alice_delegated_balance.amount == bob_receiving_balance.amount );

      generate_blocks( exp_obj->expiration + BLOCK_INTERVAL );

      exp_obj = db.get_index< asset_delegation_expiration_index, by_id >().begin();
      end = db.get_index< asset_delegation_expiration_index, by_id >().end();

      alice_liquid_balance = get_liquid_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_delegated_balance = db.get_delegated_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_receiving_balance = db.get_receiving_balance( account_name_type( "alice" ), SYMBOL_COIN );
      alice_voting_power = db.get_voting_power( account_name_type( "alice" ), SYMBOL_COIN  );

      bob_liquid_balance = get_liquid_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_delegated_balance = db.get_delegated_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_receiving_balance = db.get_receiving_balance( account_name_type( "bob" ), SYMBOL_COIN );
      bob_voting_power = db.get_voting_power( account_name_type( "bob" ), SYMBOL_COIN  );

      BOOST_REQUIRE( exp_obj == end );
      BOOST_REQUIRE( alice_delegated_balance.amount == 0 );
      BOOST_REQUIRE( bob_receiving_balance.amount == 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Remove a delegation" );

      BOOST_TEST_MESSAGE( "├── Passed: DELEGATE ASSET OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()