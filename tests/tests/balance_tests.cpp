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

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_reward( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_staked_balance = db.get_staked_balance( "alice", SYMBOL_COIN );
      asset alice_init_reward_balance = db.get_reward_balance( "alice", SYMBOL_COIN );

      signed_transaction tx;

      claim_reward_balance_operation claim;

      claim.signatory = "alice";
      claim.account = "alice";
      claim.reward = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      claim.validate();

      tx.operations.push_back( claim );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_staked_balance = db.get_staked_balance( "alice", SYMBOL_COIN );
      asset alice_reward_balance = db.get_reward_balance( "alice", SYMBOL_COIN );
      
      BOOST_REQUIRE( alice_liquid_balance + alice_staked_balance + alice_reward_balance == 
         alice_init_liquid_balance + alice_init_staked_balance + alice_init_reward_balance );

      generate_block();
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful reward claim" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when reward claim is higher than balance" );
   
      claim.reward = asset( 1000000* BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( claim );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when reward claim is higher than balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: claiming entire reward balance" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_staked_balance = db.get_staked_balance( "alice", SYMBOL_COIN );
      asset alice_init_reward_balance = db.get_reward_balance( "alice", SYMBOL_COIN );

      claim.reward = alice_init_reward_balance;

      tx.operations.push_back( claim );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_staked_balance = db.get_staked_balance( "alice", SYMBOL_COIN );
      asset alice_reward_balance = db.get_reward_balance( "alice", SYMBOL_COIN );
      
      BOOST_REQUIRE( alice_liquid_balance + alice_staked_balance == 
         alice_init_liquid_balance + alice_init_staked_balance + alice_init_reward_balance );
      BOOST_REQUIRE( alice_reward_balance.amount == 0 );

      generate_block();
      validate_database();

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

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice) );

      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_reward( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_staked_balance = db.get_staked_balance( "alice", SYMBOL_COIN );
      asset alice_init_reward_balance = db.get_reward_balance( "alice", SYMBOL_COIN );

      signed_transaction tx;

      stake_asset_operation stake;

      stake.signatory = "alice";
      stake.from = "alice";
      stake.to = "bob";
      stake.amount = alice_init_liquid_balance;
      stake.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( stake );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( balance.stake_rate == alice_init_liquid_balance.amount / 4 );
      BOOST_REQUIRE( balance.next_stake_time == now() + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( balance.to_stake == alice_init_liquid_balance.amount );
      BOOST_REQUIRE( balance.total_staked == 0 );

      time_point prev_time = balance.next_stake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL - BLOCK_INTERVAL );

      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( balance.stake_rate == alice_init_liquid_balance.amount / 4 );
      BOOST_REQUIRE( balance.next_stake_time == prev_time );
      BOOST_REQUIRE( balance.to_stake == alice_init_liquid_balance.amount );
      BOOST_REQUIRE( balance.total_staked == 0 );

      generate_block();

      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( balance.stake_rate == alice_init_liquid_balance.amount / 4 );
      BOOST_REQUIRE( balance.next_stake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( balance.to_stake == alice_init_liquid_balance.amount );
      BOOST_REQUIRE( balance.total_staked == alice_init_liquid_balance.amount / 4 );

      prev_time = balance.next_stake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( balance.stake_rate == alice_init_liquid_balance.amount / 4 );
      BOOST_REQUIRE( balance.next_stake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( balance.to_stake == alice_init_liquid_balance.amount );
      BOOST_REQUIRE( balance.total_staked == alice_init_liquid_balance.amount / 2 );

      prev_time = balance.next_stake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( balance.stake_rate == alice_init_liquid_balance.amount / 4 );
      BOOST_REQUIRE( balance.next_stake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( balance.to_stake == alice_init_liquid_balance.amount );
      BOOST_REQUIRE( balance.total_staked == 3 * ( alice_init_liquid_balance.amount / 4 ) );

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( balance.stake_rate == 0 );
      BOOST_REQUIRE( balance.next_stake_time == fc::time_point::maximum() );
      BOOST_REQUIRE( balance.to_stake == 0 );
      BOOST_REQUIRE( balance.total_staked == 0 );

      BOOST_REQUIRE( balance.liquid_balance == 0 );
      
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

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice) );

      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_staked_balance = db.get_staked_balance( "alice", SYMBOL_COIN );

      signed_transaction tx;

      unstake_asset_operation unstake;

      unstake.signatory = "alice";
      unstake.from = "alice";
      unstake.to = "alice";
      unstake.amount = alice_init_staked_balance;
      unstake.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( unstake );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.unstake_rate == alice_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( alice_balance.next_unstake_time == now() + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( alice_balance.to_unstake == alice_init_staked_balance.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == 0 );

      time_point prev_time = alice_balance.next_unstake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL - BLOCK_INTERVAL );

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.unstake_rate == alice_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( alice_balance.next_unstake_time == prev_time );
      BOOST_REQUIRE( alice_balance.to_unstake == alice_init_staked_balance.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == 0 );

      generate_block();

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.unstake_rate == alice_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( alice_balance.next_unstake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( alice_balance.to_unstake == alice_init_staked_balance.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == alice_init_staked_balance.amount / 4 );

      prev_time = alice_balance.next_unstake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.unstake_rate == alice_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( alice_balance.next_unstake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( alice_balance.to_unstake == alice_init_staked_balance.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == alice_init_staked_balance.amount / 2 );

      prev_time = alice_balance.next_unstake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.unstake_rate == alice_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( alice_balance.next_unstake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( alice_balance.to_unstake == alice_init_staked_balance.amount );
      BOOST_REQUIRE( alice_balance.total_unstaked == 3 * ( alice_init_staked_balance.amount / 4 ) );

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.unstake_rate == 0 );
      BOOST_REQUIRE( alice_balance.next_unstake_time == fc::time_point::maximum() );
      BOOST_REQUIRE( alice_balance.to_unstake == 0 );
      BOOST_REQUIRE( alice_balance.total_unstaked == 0 );
      BOOST_REQUIRE( alice_balance.staked_balance == 0 );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful asset unstake" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: asset unstake routes" );

      asset bob_init_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset bob_init_staked_balance = db.get_staked_balance( "bob", SYMBOL_COIN );

      asset candice_init_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );
      asset candice_init_staked_balance = db.get_staked_balance( "candice", SYMBOL_COIN );

      unstake_asset_route_operation route;

      route.signatory = "bob";
      route.from_account = "bob";
      route.to_account = "candice";
      route.percent = 50 * PERCENT_1;
      route.auto_stake = false;
      route.validate();

      tx.operations.push_back( route );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& route_idx = db.get_index< unstake_asset_route_index >().indices().get< by_withdraw_route >();
      auto route_itr = route_idx.find( boost::make_tuple( route.from_account, route.to_account ) );

      BOOST_REQUIRE( route_itr != route_idx.end() );
      BOOST_REQUIRE( route_itr->from_account == route.from_account );
      BOOST_REQUIRE( route_itr->to_account == route.to_account );
      BOOST_REQUIRE( route_itr->percent == route.percent );

      unstake.signatory = "bob";
      unstake.from = "bob";
      unstake.to = "bob";
      unstake.amount = bob_init_staked_balance;
      unstake.validate();

      tx.operations.push_back( unstake );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.next_unstake_time == now() + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == 0 );

      time_point prev_time = bob_balance.next_unstake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL - BLOCK_INTERVAL );

      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.next_unstake_time == prev_time );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == 0 );

      generate_block();

      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.next_unstake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == bob_init_staked_balance.amount / 4 );

      prev_time = bob_balance.next_unstake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.next_unstake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == bob_init_staked_balance.amount / 2 );

      prev_time = bob_balance.next_unstake_time;

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( bob_balance.unstake_rate == bob_init_staked_balance.amount / 4 );
      BOOST_REQUIRE( bob_balance.next_unstake_time == prev_time + STAKE_WITHDRAW_INTERVAL );
      BOOST_REQUIRE( bob_balance.to_unstake == bob_init_staked_balance.amount );
      BOOST_REQUIRE( bob_balance.total_unstaked == 3 * ( bob_init_staked_balance.amount / 4 ) );

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL );

      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( bob_balance.unstake_rate == 0 );
      BOOST_REQUIRE( bob_balance.next_unstake_time == fc::time_point::maximum() );
      BOOST_REQUIRE( bob_balance.to_unstake == 0 );
      BOOST_REQUIRE( bob_balance.total_unstaked == 0 );
      BOOST_REQUIRE( bob_balance.staked_balance == 0 );
      BOOST_REQUIRE( bob_balance.liquid_balance == bob_init_liquid_balance.amount + bob_init_staked_balance.amount / 2 );

      const account_balance_object& candice_balance = db.get_account_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( candice_balance.liquid_balance == candice_init_liquid_balance.amount + bob_init_staked_balance.amount / 2 );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: asset unstake routes" );

      BOOST_TEST_MESSAGE( "├── Passed: UNSTAKE ASSET OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( transfer_to_savings_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSER TO SAVINGS OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer to own savings" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      
      signed_transaction tx;

      transfer_to_savings_operation transfer;

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "alice";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.liquid_balance == alice_init_liquid_balance.amount - transfer.amount.amount );
      BOOST_REQUIRE( alice_balance.savings_balance == transfer.amount.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer to own savings" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer to other account's savings" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );

      transfer.to = "bob";

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );
      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.liquid_balance == alice_init_liquid_balance.amount - transfer.amount.amount );
      BOOST_REQUIRE( bob_balance.savings_balance == transfer.amount.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer to other account's savings" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure with insufficient liquid funds" );

      transfer.to = "alice";
      transfer.amount = asset( 1000000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

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


BOOST_AUTO_TEST_CASE( transfer_from_savings_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSER FROM SAVINGS OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer from own savings" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_savings( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_savings( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_savings( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_savings_balance = db.get_savings_balance( "alice", SYMBOL_COIN );
      
      signed_transaction tx;

      transfer_from_savings_operation transfer;

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "alice";
      transfer.request_id = "e02a03a8-0843-44cb-9700-b2d6d11e24c1";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.transferred = true;
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const savings_withdraw_object& withdraw = db.get_savings_withdraw( "alice", "e02a03a8-0843-44cb-9700-b2d6d11e24c1" );

      BOOST_REQUIRE( withdraw.from == transfer.from );
      BOOST_REQUIRE( withdraw.to == transfer.to );
      BOOST_REQUIRE( to_string( withdraw.memo ) == transfer.memo );
      BOOST_REQUIRE( to_string( withdraw.request_id ) == transfer.request_id );
      BOOST_REQUIRE( withdraw.amount == transfer.amount );
      BOOST_REQUIRE( withdraw.complete == now() + SAVINGS_WITHDRAW_TIME );

      time_point prev_complete = withdraw.complete;

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.savings_balance == alice_init_savings_balance.amount - transfer.amount.amount );

      generate_blocks( now() + SAVINGS_WITHDRAW_TIME - BLOCK_INTERVAL );

      const savings_withdraw_object& withdraw = db.get_savings_withdraw( "alice", "e02a03a8-0843-44cb-9700-b2d6d11e24c1" );

      BOOST_REQUIRE( withdraw.from == transfer.from );
      BOOST_REQUIRE( withdraw.to == transfer.to );
      BOOST_REQUIRE( to_string( withdraw.memo ) == transfer.memo );
      BOOST_REQUIRE( to_string( withdraw.request_id ) == transfer.request_id );
      BOOST_REQUIRE( withdraw.amount == transfer.amount );
      BOOST_REQUIRE( withdraw.complete == prev_complete );

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      generate_block();

      const auto& withdraw_idx = db.get_index< savings_withdraw_index >().indices().get< by_request_id >();
      auto withdraw_itr = withdraw_idx.find( boost::make_tuple( withdraw.from, withdraw.request_id ) );

      BOOST_REQUIRE( withdraw_itr != withdraw_idx.end() );

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.savings_balance == alice_init_savings_balance.amount - transfer.amount.amount );
      BOOST_REQUIRE( alice_balance.liquid_balance == alice_init_liquid_balance.amount + transfer.amount.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer from own savings" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer from savings to another account" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );

      transfer.to = "bob";

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_balance_object& alice_balance = db.get_account_balance( "alice", SYMBOL_COIN );
      const account_balance_object& bob_balance = db.get_account_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_balance.liquid_balance == alice_init_liquid_balance.amount - transfer.amount.amount );
      BOOST_REQUIRE( bob_balance.liquid_balance == bob_init_liquid_balance.amount + transfer.amount.amount );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer from savings to another account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure with insufficient liquid funds" );

      transfer.to = "alice";
      transfer.amount = asset( 1000000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure with insufficient liquid funds" );

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

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice) );

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      fund( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_delegated_balance = db.get_delegated_balance( "alice", SYMBOL_COIN );
      asset alice_init_receiving_balance = db.get_receiving_balance( "alice", SYMBOL_COIN );
      share_type alice_init_voting_power = db.get_voting_power( "alice" );

      asset bob_init_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset bob_init_delegated_balance = db.get_delegated_balance( "bob", SYMBOL_COIN );
      asset bob_init_receiving_balance = db.get_receiving_balance( "bob", SYMBOL_COIN );
      share_type bob_init_voting_power = db.get_voting_power( "bob" );
      
      signed_transaction tx;

      delegate_asset_operation delegate;

      delegate.signatory = "alice";
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

      const asset_delegation_object& delegation = db.get_asset_delegation( "alice", "bob", SYMBOL_COIN );

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_delegated_balance = db.get_delegated_balance( "alice", SYMBOL_COIN );
      asset alice_receiving_balance = db.get_receiving_balance( "alice", SYMBOL_COIN );
      share_type alice_voting_power = db.get_voting_power( "alice" );

      asset bob_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset bob_delegated_balance = db.get_delegated_balance( "bob", SYMBOL_COIN );
      asset bob_receiving_balance = db.get_receiving_balance( "bob", SYMBOL_COIN );
      share_type bob_voting_power = db.get_voting_power( "bob" );

      BOOST_REQUIRE( delegation.delegator == delegate.delegator );
      BOOST_REQUIRE( delegation.delegatee == delegate.delegatee );
      BOOST_REQUIRE( delegation.amount == delegate.amount );
      BOOST_REQUIRE( alice_delegated_balance == bob_receiving_balance );
      BOOST_REQUIRE( alice_voting_power == alice_init_voting_power - delegate.amount.amount );
      BOOST_REQUIRE( bob_voting_power == bob_init_voting_power + delegate.amount.amount );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful asset delegation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when delegating more than staked balance" );

      delegate.amount = asset( 1000000* BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( delegate );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when delegating more than staked balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure delegating staked balance that are being unstaked" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_delegated_balance = db.get_delegated_balance( "alice", SYMBOL_COIN );
      asset alice_init_receiving_balance = db.get_receiving_balance( "alice", SYMBOL_COIN );
      share_type alice_init_voting_power = db.get_voting_power( "alice" );

      unstake_asset_operation withdraw;

      withdraw.signatory = "alice";
      withdraw.from = "alice";
      withdraw.to = "alice";
      withdraw.amount = asset( 90000* BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( withdraw );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      delegate.amount = asset( 90000* BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      
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
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure delegating staked balance that are being unstaked" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure powering down stake that is delegated" );

      withdraw.amount = asset( 90000* BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( withdraw );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure powering down stake that is delegated" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Remove a delegation" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_init_delegated_balance = db.get_delegated_balance( "alice", SYMBOL_COIN );
      asset alice_init_receiving_balance = db.get_receiving_balance( "alice", SYMBOL_COIN );
      share_type alice_init_voting_power = db.get_voting_power( "alice" );

      asset bob_init_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset bob_init_delegated_balance = db.get_delegated_balance( "bob", SYMBOL_COIN );
      asset bob_init_receiving_balance = db.get_receiving_balance( "bob", SYMBOL_COIN );
      share_type bob_init_voting_power = db.get_voting_power( "bob" );

      delegate.amount = asset( 0, SYMBOL_COIN );

      tx.operations.push_back( delegate );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      auto exp_obj = db.get_index< asset_delegation_expiration_index, by_id >().begin();
      auto end = db.get_index< asset_delegation_expiration_index, by_id >().end();

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_delegated_balance = db.get_delegated_balance( "alice", SYMBOL_COIN );
      asset alice_receiving_balance = db.get_receiving_balance( "alice", SYMBOL_COIN );
      share_type alice_voting_power = db.get_voting_power( "alice" );

      asset bob_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset bob_delegated_balance = db.get_delegated_balance( "bob", SYMBOL_COIN );
      asset bob_receiving_balance = db.get_receiving_balance( "bob", SYMBOL_COIN );
      share_type bob_voting_power = db.get_voting_power( "bob" );

      const auto& delegation_idx = db.get_index< asset_delegation_index >().indices().get< by_delegation >();
      auto delegation_itr = delegation_idx.find( boost::make_tuple( "alice", "bob", SYMBOL_COIN ) );

      BOOST_REQUIRE( delegation_itr == delegation_idx.end() );

      BOOST_REQUIRE( exp_obj != end );
      BOOST_REQUIRE( exp_obj->delegator == delegate.delegator );
      BOOST_REQUIRE( exp_obj->delegatee == delegate.delegatee );
      BOOST_REQUIRE( exp_obj->amount == alice_init_delegated_balance );
      BOOST_REQUIRE( exp_obj->expiration == now() + CONTENT_REWARD_INTERVAL );
      
      BOOST_REQUIRE( alice_delegated_balance.amount == bob_receiving_balance.amount );

      generate_blocks( exp_obj->expiration + BLOCK_INTERVAL );

      exp_obj = db.get_index< asset_delegation_expiration_index, by_id >().begin();
      end = db.get_index< asset_delegation_expiration_index, by_id >().end();

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset alice_delegated_balance = db.get_delegated_balance( "alice", SYMBOL_COIN );
      asset alice_receiving_balance = db.get_receiving_balance( "alice", SYMBOL_COIN );
      share_type alice_voting_power = db.get_voting_power( "alice" );

      asset bob_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );
      asset bob_delegated_balance = db.get_delegated_balance( "bob", SYMBOL_COIN );
      asset bob_receiving_balance = db.get_receiving_balance( "bob", SYMBOL_COIN );
      share_type bob_voting_power = db.get_voting_power( "bob" );

      BOOST_REQUIRE( exp_obj == end );
      BOOST_REQUIRE( alice_delegated_balance.amount == 0 );
      BOOST_REQUIRE( bob_receiving_balance.amount == 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Remove a delegation" );

      BOOST_TEST_MESSAGE( "├── Passed: DELEGATE ASSET OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()