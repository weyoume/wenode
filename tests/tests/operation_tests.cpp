//#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <node/protocol/exceptions.hpp>

#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/hardfork.hpp>
#include <node/chain/node_objects.hpp>

#include <node/chain/util/reward.hpp>

#include <node/witness/witness_objects.hpp>

#include <fc/crypto/digest.hpp>

#include "../common/database_fixture.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace node;
using namespace node::chain;
using namespace node::protocol;
using fc::string;

BOOST_FIXTURE_TEST_SUITE( operation_tests, clean_database_fixture )



   //============================//
   // === Account Operations === //
   //============================//


BOOST_AUTO_TEST_CASE( account_create_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT CREATE" );
      time_point now = db.head_block_time();

      account_create_operation op;
      op.registrar = "alice";
      op.new_account_name = "bob";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      BOOST_TEST_MESSAGE( "│   ├── Testing: owner authority" );
      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );
      BOOST_TEST_MESSAGE( "│   ├── Passed: owner authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: active authority" );
      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
      BOOST_TEST_MESSAGE( "│   ├── Passed: active authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting authority" );
      expected.clear();
      auths.clear();
      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );
      BOOST_TEST_MESSAGE( "│   ├── Passed: posting authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: persona account creation" );

      signed_transaction tx;
      private_key_type priv_key = generate_private_key( "aliceownercorrecthorsebatterystaple" );
      asset init_starting_balance = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      const account_object& init = db.get_account( INIT_ACCOUNT );
      fund( INIT_ACCOUNT, init_starting_balance );
      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      account_create_operation op;

      op.signatory = INIT_ACCOUNT;
      op.registrar = INIT_ACCOUNT;
      op.new_account_name = "alice";
      op.account_type = PERSONA;
      op.referrer = INIT_ACCOUNT;
      op.proxy = INIT_ACCOUNT;
      op.governance_account = INIT_ACCOUNT;
      op.recovery_account = INIT_ACCOUNT;
      op.reset_account = INIT_ACCOUNT;
      op.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      op.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      op.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      op.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      op.owner = authority( 1, priv_key.get_public_key(), 1 );
      op.active = authority( 2, priv_key.get_public_key(), 2 );
      op.posting = authority( 1, priv_key.get_public_key(), 1 );
      op.secure_public_key = priv_key.get_public_key();
      op.connection_public_key = priv_key.get_public_key();
      op.friend_public_key = priv_key.get_public_key();
      op.companion_public_key = priv_key.get_public_key();
      op.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      tx.validate();
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "alice" );
      const account_following_object& acct_follow = db.get_account_following( "alice" );

      BOOST_REQUIRE( acct.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct_follow.account == "alice" );
      BOOST_REQUIRE( acct.account_type = PERSONA );
      BOOST_REQUIRE( acct.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.created == now );
      BOOST_REQUIRE( acct_auth.owner == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.connection_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.friend_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.companion_public_key == priv_key.get_public_key() );
      
      BOOST_REQUIRE( acct.id._id == acct_auth.id._id );
      BOOST_REQUIRE( acct.id._id == acct_follow.id._id );
      

      BOOST_REQUIRE( db.get_staked_balance( "alice", SYMBOL_COIN ) == op.fee );

      const asset& init_liquid = db.get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );
      
      BOOST_REQUIRE( ( init_starting_balance - op.fee ) == init_liquid );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: persona account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: profile account creation" );

      tx.signatures.clear();
      tx.operations.clear();
      op.account_type = PROFILE;
      op.new_account_name = "bob";
      tx.operations.push_back( op );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "bob" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "bob" );
      const account_following_object& acct_follow = db.get_account_following( "bob" );

      BOOST_REQUIRE( acct.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.name == "bob" );
      BOOST_REQUIRE( acct_follow.account == "bob" );
      BOOST_REQUIRE( acct.account_type = PERSONA );
      BOOST_REQUIRE( acct.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.created == now );
      BOOST_REQUIRE( acct_auth.owner == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.connection_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.friend_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.companion_public_key == priv_key.get_public_key() );

      BOOST_REQUIRE( acct.id._id == acct_auth.id._id );
      BOOST_REQUIRE( acct.id._id == acct_follow.id._id );

      BOOST_REQUIRE( db.get_staked_balance( "bob", SYMBOL_COIN ) == op.fee );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: profile account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: business account creation" );

      tx.signatures.clear();
      tx.operations.clear();
      op.account_type = BUSINESS;
      op.officer_vote_threshold = BLOCKCHAIN_PRECISION * 1000;
      op.business_type = PUBLIC_BUSINESS;
      op.business_public_key = priv_key.get_public_key();
      op.new_account_name = "ionstudios";
      tx.operations.push_back( op );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "ionstudios" );
      const account_business_object& acct_bus = db.get_account_business( "ionstudios" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "ionstudios" );
      const account_following_object& acct_follow = db.get_account_following( "ionstudios" );

      BOOST_REQUIRE( acct.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.name == "ionstudios" );
      BOOST_REQUIRE( acct_follow.account == "ionstudios" );
      BOOST_REQUIRE( acct_bus.name == "ionstudios" );
      BOOST_REQUIRE( acct_bus.executive_board.CHIEF_EXECUTIVE_OFFICER == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.account_type = BUSINESS );
      BOOST_REQUIRE( acct.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.created == now );
      BOOST_REQUIRE( acct_auth.owner == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.connection_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.friend_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.companion_public_key == priv_key.get_public_key() );

      BOOST_REQUIRE( acct.id._id == acct_auth.id._id );
      BOOST_REQUIRE( acct.id._id == acct_follow.id._id );
      BOOST_REQUIRE( acct.id._id == acct_bus.id._id );

      BOOST_REQUIRE( db.get_staked_balance( "ionstudios", SYMBOL_COIN ) == op.fee );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: business account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure of duplicate account creation" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure of duplicate account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creator cannot cover fee" );

      tx.signatures.clear();
      tx.operations.clear();

      op.fee = asset( init_liquid + asset( 1 , SYMBOL_COIN ), SYMBOL_COIN );    // Fee slightly greater than liquid balance.

      op.new_account_name = "bob";
      tx.operations.push_back( op );
      tx.sign( init_account_priv_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when creator cannot cover fee" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account fee is insufficient" );

      generate_block();

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_witness_schedule(), [&]( witness_schedule_object& wso )
         {
            wso.median_props.account_creation_fee = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );    // Fee much too high.
         });
      });

      generate_block();

      tx.clear();
      op.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      tx.operations.push_back( op );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account fee is insufficient" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT CREATE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT UPDATE" );
      time_point now = db.head_block_time();

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account posting authority is invalid" );

      ACTORS( (alice)(bob) )

      account_update_operation op;
      op.account = "alice";
      op.details = "Shrek is love."
      op.json = "{\"Shrek\":\"life\"}";
      op.posting = authority();
      op.posting->weight_threshold = 1;
      op.posting->add_authorities( "abcdefghijklmnopq", 1 );
      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      try
      {
         db.push_transaction( tx, 0 );

         BOOST_FAIL( "An exception was not thrown for an invalid account name" );
      }
      catch( fc::exception& ) {}

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account posting authority is invalid" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: account update authorities" );

      op.posting = authority();
      op.validate();

      private_key_type active_key = generate_private_key( "aliceactivecorrecthorsebatterystaple" );

      db.modify( db.get< account_authority_object, by_account >( "alice" ), [&]( account_authority_object& a )
      {
         a.active = authority( 1, active_key.get_public_key(), 1 );
      });

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      db.push_transaction( tx, 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: account update authorities" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signature" );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invalid signature" );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invalid signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when additional irrelevant signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when additional irrelevant signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );
      tx.signatures.clear();
      tx.sign( active_key, db.get_chain_id() );
      tx.sign( active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success on active key" );
      tx.signatures.clear();
      tx.sign( active_key, db.get_chain_id() );    // Single signature with new correct active key
      db.push_transaction( tx, 0 );
      BOOST_TEST_MESSAGE( "│   ├── Passed: success on active key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success on owner key" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );
      BOOST_TEST_MESSAGE( "│   ├── Passed: success on owner key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when updating the owner authority with an active key" );
      tx.signatures.clear();
      tx.operations.clear();
      op.owner = authority( 1, active_key.get_public_key(), 1 );    
      tx.operations.push_back( op );
      tx.sign( active_key, db.get_chain_id() );    // Updates owner to active, signed with active instead of owner
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_owner_auth );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when updating the owner authority with an active key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when owner key and active key are present" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when owner key and active key are present" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0), tx_missing_owner_auth );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when incorrect signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate owner keys are present" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0), tx_duplicate_sig );
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate owner keys are present" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success when updating the owner authority with an owner key" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();
      BOOST_TEST_MESSAGE( "│   ├── Testing: success when updating the owner authority with an owner key" );

      private_key_type new_private_key = generate_private_key( "aliceownerhunter2" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update" );
      tx.signatures.clear();
      tx.operations.clear();

      op.owner = authority( 1, new_private_key.get_public_key(), 1 );
      op.active = authority( 2, new_private_key.get_public_key(), 2 );
      op.secure_public_key = new_private_key.get_public_key();
      op.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      op.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      op.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      op.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";

      tx.operations.push_back( op );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "alice" );

      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct_auth.owner == authority( 1, new_private_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active == authority( 2, new_private_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct.secure_public_key == new_private_key.get_public_key() );
      validate_database();
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when updating a non-existent account" );
      tx.operations.clear();
      tx.signatures.clear();
      op.account = "bob";
      tx.operations.push_back( op );
      tx.sign( new_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception )
      validate_database();
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when updating a non-existent account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account authority does not exist" );
      tx.clear();
      op.account = "alice";
      op.posting = authority();
      op.posting->weight_threshold = 1;
      op.posting->add_authorities( "dave", 1 );
      tx.operations.push_back( op );
      tx.sign( new_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account authority does not exist" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT UPDATE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_membership_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT MEMBERSHIP" );
      time_point now = db.head_block_time();

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account membership" );

      ACTORS( (alice)(bob)(chris)(dave)(elon) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.membership_type = STANDARD_MEMBERSHIP;
      op.months = 1;
      op.interface = INIT_ACCOUNT;

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );

      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct.membership == STANDARD_MEMBERSHIP );
      BOOST_REQUIRE( acct.membership_interface == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recurring_membership == 1 );
      BOOST_REQUIRE( acct.membership_expiration == now + fc::days(30) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account membership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when insufficient funds" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "bob";
      op.account = "bob";

      tx.operations.push_back( op );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when insufficient funds" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBERSHIP" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_vote_executive_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT VOTE EXECUTIVE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account vote executive" );

      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );

      account_vote_executive_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.business_account = INIT_ACCOUNT;
      op.executive_account = INIT_CEO;
      op.role = CHIEF_EXECUTIVE_OFFICER;

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_executive_vote_object& vote = db.get_account_executive_vote( "alice", INIT_ACCOUNT, INIT_CEO );

      BOOST_REQUIRE( vote.account == "alice" );
      BOOST_REQUIRE( vote.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( vote.executive_account ==  INIT_CEO );
      BOOST_REQUIRE( vote.vote_rank == 1 );
      BOOST_REQUIRE( vote.role == CHIEF_EXECUTIVE_OFFICER );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote executive" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no equity voting power" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "bob";
      op.account = "bob";

      tx.operations.push_back( op );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no equity voting power" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel vote" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "alice";
      op.account = "alice";
      op.approved = false;
      op.validate();

      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_executive_vote_object* vote_ptr = db.find_account_executive_vote( "alice", INIT_ACCOUNT, INIT_CEO );
      BOOST_REQUIRE( vote_ptr == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel vote" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT VOTE EXECUTIVE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_vote_officer_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT VOTE OFFICER" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account vote officer" );
      
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );

      account_vote_officer_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.business_account = INIT_ACCOUNT;
      op.officer_account = INIT_CEO;

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_officer_vote_object& vote = db.get_account_officer_vote( "alice", INIT_ACCOUNT, INIT_CEO );

      BOOST_REQUIRE( vote.account == "alice" );
      BOOST_REQUIRE( vote.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( vote.officer_account ==  INIT_CEO );
      BOOST_REQUIRE( vote.vote_rank == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote officer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no equity voting power" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "bob";
      op.account = "bob";

      tx.operations.push_back( op );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no equity voting power" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel vote" );

      tx.operations.clear();
      tx.signatures.clear();
      
      op.signatory = "alice";
      op.account = "alice";
      op.approved = false;
      op.validate();

      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_officer_vote_object* vote_ptr = db.find_account_officer_vote( "alice", INIT_ACCOUNT, INIT_CEO );
      BOOST_REQUIRE( vote_ptr == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel vote" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT VOTE OFFICER" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_member_request_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT MEMBER REQUEST" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account member request" );
      
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );

      account_member_request_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.business_account = INIT_ACCOUNT;
      op.message = "Hello";

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_request_object& req = db.get_account_member_request( "alice", INIT_ACCOUNT );

      BOOST_REQUIRE( req.account == "alice" );
      BOOST_REQUIRE( req.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( req.message == "Hello" );
      BOOST_REQUIRE( req.expiration == now + CONNECTION_REQUEST_DURATION );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account member request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when request already exists" );

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when request already exists" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel request" );

      op.requested = false;
      op.validate();

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_request_object* req = db.find_account_member_request( "alice", INIT_ACCOUNT );
      BOOST_REQUIRE( req == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel request" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBER REQUEST" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_member_invite_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT MEMBER INVITE" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account member invite" );
      
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );

      account_member_invite_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.business_account = INIT_ACCOUNT;
      op.member = "bob";
      op.message = "Hello";
      op.encrypted_business_key = alice_public_owner_key;

      const account_business_object& bus_acc = get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_invite_object& inv = db.get_account_member_invite( "bob", INIT_ACCOUNT );

      BOOST_REQUIRE( inv.account == "alice" );
      BOOST_REQUIRE( inv.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( inv.member == "bob" );
      BOOST_REQUIRE( inv.message == "Hello" );
      BOOST_REQUIRE( inv.expiration == now + CONNECTION_REQUEST_DURATION );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account member invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invite already exists" );

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invite already exists" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel invite" );

      op.invited = false;
      op.validate();

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_invite_object* req = db.find_account_member_invite( "alice", INIT_ACCOUNT );
      BOOST_REQUIRE( req == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel invite" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBER INVITE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_accept_request_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT ACCEPT REQUEST" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account accept request" );
      
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );

      const account_business_object& bus_acc = get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      account_member_request_operation op;
      op.signatory = "bob";
      op.account = "bob";
      op.business_account = INIT_ACCOUNT;
      op.message = "Hello";

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_request_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.member = "bob";
      op.business_account = INIT_ACCOUNT;
      op.encrypted_business_key = alice_public_owner_key;

      op.validate();

      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when unauthorized request acceptance" );

      tx.operations.clear();
      tx.signatures.clear();

      account_member_request_operation op;
      op.signatory = "chris";
      op.account = "chris";
      op.business_account = INIT_ACCOUNT;
      op.message = "Hello";

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( chris_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_request_operation op;
      op.signatory = "dave";     // dave is not an officer, and cannot accept requests
      op.account = "dave";
      op.member = "chris";
      op.business_account = INIT_ACCOUNT;
      op.encrypted_business_key = chris_public_owner_key;

      op.validate();

      tx.operations.push_back( op );
      tx.sign( dave_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when unauthorized request acceptance" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT ACCEPTED REQUEST" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_accept_invite_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT ACCEPT INVITE" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account accept invite" );
      
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );

      const account_business_object& bus_acc = get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      account_member_invite_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.member = "bob";
      op.business_account = INIT_ACCOUNT;
      op.message = "Hello";
      op.encrypted_business_key = alice_public_owner_key;

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation op;
      op.signatory = "bob";
      op.account = "bob";
      op.business_account = INIT_ACCOUNT;
      
      op.validate();

      tx.operations.push_back( op );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invite does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation op;
      op.signatory = "dave";   // dave has not been invited
      op.account = "dave";
      op.business_account = INIT_ACCOUNT;

      op.validate();

      tx.operations.push_back( op );
      tx.sign( dave_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invite does not exist" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT ACCEPT INVITE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_remove_member_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT REMOVE MEMBER" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account remove member" );
      
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );

      const account_business_object& bus_acc = get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
         b.members.insert( "bob" );      // Add bob to members
      });

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );

      account_remove_member_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.member = "bob";
      op.business_account = INIT_ACCOUNT;

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( !bus_acc.is_member( "bob" ) );    // Bob has been removed

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account remove member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when unauthorized remove member" );

      tx.operations.clear();
      tx.signatures.clear();

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.members.insert( "bob" );      // Return bob to members
      });

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );

      account_remove_member_operation op;
      op.signatory = "bob";    // Bob is not an officer and cannot remove alice
      op.account = "bob";
      op.member = "alice";
      op.business_account = INIT_ACCOUNT;

      op.validate();

      tx.operations.push_back( op );
      tx.sign( bob_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when unauthorized remove member" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT REMOVE MEMBER" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_update_list_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT UPDATE LIST" );
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update list" );
      
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon) );

      account_update_list_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.listed_account = "bob";
      op.blacklisted = true;
      op.whitelisted = false;
      
      op.validate();

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_permission_object& acc_perm = get_account_permissions( "alice" );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( "bob" ) != acc_perm.blacklisted_accounts.end() );    // Bob has been blacklisted

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update list" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: remove from list" );

      tx.operations.clear();
      tx.signatures.clear();
      
      op.blacklisted = false;
      op.validate();

      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_permission_object& acc_perm = get_account_permissions( "alice" );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( "bob" ) == acc_perm.blacklisted_accounts.end() );    // Bob has been removed from blacklist

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: remove from list" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT UPDATE LIST" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_witness_vote_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT WITNESS VOTE" );
   
      time_point now = db.head_block_time();
      ACTORS( (alice)(bob)(chris)(dave)(elon)(sam) );

      fund( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( "chris", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "chris", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );

      private_key_type alice_witness_key = generate_private_key( "alicewitnesscorrecthorsebatterystaple" );

      witness_create(
         "alice", 
         alice_private_owner_key, 
         "details", 
         "url", 
         "{\"json\":\"valid\"}", 
         alice_witness_key.get_public_key(), 
         BLOCKCHAIN_PRECISION * 10 
      );

      private_key_type chris_witness_key = generate_private_key( "chriswitnesscorrecthorsebatterystaple" );

      witness_create(
         "chris", 
         chris_private_owner_key, 
         "details", 
         "url", 
         "{\"json\":\"valid\"}", 
         chris_witness_key.get_public_key(), 
         BLOCKCHAIN_PRECISION * 10 
      );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account witness vote" );

      account_witness_vote_operation op;
      op.signatory = "bob";
      op.account = "bob";
      op.witness = "alice";
      op.vote_rank = 1;
      op.approved = true;

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const witness_vote_object& vote = db.get_witness_vote( "bob", "alice" );
      const account_object& acc = db.get_account( "bob" ); 
      const witness_object& witness = db.get_witness( "alice" );
      BOOST_REQUIRE( vote.account == "bob" );
      BOOST_REQUIRE( vote.witness == "alice" );
      BOOST_REQUIRE( vote.vote_rank == 1 );
      BOOST_REQUIRE( acc.witness_vote_count == 1 );
      BOOST_REQUIRE( witness.vote_count == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account witness vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional witness vote" );

      op.witness = "chris";   // Vote for chris at rank one will move alice to rank two

      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const witness_vote_object& vote_a = db.get_witness_vote( "bob", "alice" );
      const witness_vote_object& vote_c = db.get_witness_vote( "bob", "chris" );
      const account_object& acc = db.get_account( "bob" ); 
      const witness_object& witness_a = db.get_witness( "alice" );
      const witness_object& witness_c = db.get_witness( "chris" );
      BOOST_REQUIRE( vote_a.account == "bob" );
      BOOST_REQUIRE( vote_a.witness == "alice" );
      BOOST_REQUIRE( vote_a.vote_rank == 2 );
      BOOST_REQUIRE( vote_c.account == "bob" );
      BOOST_REQUIRE( vote_c.witness == "chris" );
      BOOST_REQUIRE( vote_c.vote_rank == 1 );
      BOOST_REQUIRE( acc.witness_vote_count == 2 );
      BOOST_REQUIRE( witness_a.vote_count == 1 );
      BOOST_REQUIRE( witness_c.vote_count == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: additional witness vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel witness vote" );

      op.approved = false;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const witness_vote_object* vote_ptr = db.find_witness_vote( "bob", "chris" );
      const account_object& acc = db.get_account( "bob" );
      const witness_object& witness = db.get_witness( "chris" );
      BOOST_REQUIRE( vote_ptr == nullptr );
      BOOST_REQUIRE( acc.witness_vote_count == 1 );
      BOOST_REQUIRE( witness.vote_count == 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel witness vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel additional witness vote" );

      op.witness = "alice";

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const witness_vote_object* vote_ptr = db.find_witness_vote( "bob", "alice" );
      const account_object& acc = db.get_account( "bob" );
      const witness_object& witness = db.get_witness( "alice" );
      BOOST_REQUIRE( vote_ptr == nullptr );
      BOOST_REQUIRE( acc.witness_vote_count == 0 );
      BOOST_REQUIRE( witness.vote_count == 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel additional witness vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when attempting to revoke a non-existent vote" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when attempting to revoke a non-existent vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting for a non-existent account" );

      tx.operations.clear();
      tx.signatures.clear();
      op.witness = "eve";
      op.approve = true;
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting for a non-existent account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting for an account that is not a witness" );

      tx.operations.clear();
      tx.signatures.clear();
      op.witness = "elon";
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting for an account that is not a witness" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT WITNESS VOTE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_update_proxy_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT UPDATE PROXY" );
      time_point now = db.head_block_time();
      const dynamic_global_property_object& props = get_dynamic_global_properties();
      price equity_price = props.current_median_equity_price;

      ACTORS( (alice)(bob)(chris)(sam)(dave)(elon) )
      fund( "alice", 1000 * BLOCKCHAIN_PRECISION );
      fund_stake( "alice", 1000 * BLOCKCHAIN_PRECISION );
      fund( "bob", 3000 * BLOCKCHAIN_PRECISION );
      fund_stake( "bob", 3000 * BLOCKCHAIN_PRECISION );
      fund( "chris", 3000 * BLOCKCHAIN_PRECISION );
      fund_stake( "chris", 3000 * BLOCKCHAIN_PRECISION );
      fund( "sam", 5000 * BLOCKCHAIN_PRECISION );
      fund_stake( "sam", 5000 * BLOCKCHAIN_PRECISION );
      fund( "dave", 7000 * BLOCKCHAIN_PRECISION );
      fund_stake( "dave", 7000 * BLOCKCHAIN_PRECISION );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update proxy" );

      account_update_proxy_operation op;
      op.signatory = "bob";
      op.account = "bob";
      op.proxy = "alice";

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );    // bob -> alice

      BOOST_REQUIRE( bob.proxy == "alice" );
      BOOST_REQUIRE( bob.proxied.size() == 0 );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *alice.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( alice, equity_price ) == db.get_voting_power( bob, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: changing proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      op.proxy = "sam";
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );     // bob->sam

      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( bob.proxied.size() == 0 );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( alice.proxied.size() == 0 );
      BOOST_REQUIRE( sam.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: changing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when changing proxy to existing proxy" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when changing proxy to existing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandparent proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "sam";
      op.account = "sam";
      op.proxy = "dave";
      
      tx.operations.push_back( op );
      tx.sign( sam_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );     // bob->sam->dave

      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( *sam.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dave" );
      BOOST_REQUIRE( *dave.proxied.begin() == "sam" );
      BOOST_REQUIRE( dave.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dave, equity_price ) == db.get_voting_power( sam, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandparent proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandchild proxy" );

      // alice \
      // bob->  sam->dave

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "alice";
      op.account = "alice";
      op.proxy = "sam";
      
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.proxy == "sam" );
      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( *sam.proxied.size() == 2 );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) + db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dave" );
      BOOST_REQUIRE( *dave.proxied.begin() == "sam" );
      BOOST_REQUIRE( dave.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dave, equity_price ) == db.get_voting_power( sam, equity_price ) );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandchild proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing a grandchild proxy" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "bob";
      op.account = "bob";
      op.proxy = PROXY_TO_SELF_ACCOUNT;
      
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );    // alice->sam->dave

      BOOST_REQUIRE( alice.proxy == "sam" );
      BOOST_REQUIRE( bob.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == "alice" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dave" );
      BOOST_REQUIRE( *dave.proxied.begin() == "sam" );
      BOOST_REQUIRE( dave.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dave, equity_price ) == db.get_voting_power( sam, equity_price ) );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing a grandchild proxy" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT UPDATE PROXY" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_recovery_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT RECOVERY SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal request account recovery" );
      time_point now = db.head_block_time();

      ACTORS( (alice)(bob)(chris)(sam)(dave) );
      fund( "alice", 1000 * BLOCKCHAIN_PRECISION );
      fund_stake( "alice", 1000 * BLOCKCHAIN_PRECISION );

      account_create_operation acc_create;       // Creating account bob with alice as recovery account

      acc_create.signatory = "alice";
      acc_create.registrar = "alice";
      acc_create.recovery_account = "alice";
      acc_create.reset_account = "alice";
      acc_create.account_type = PERSONA;
      acc_create.new_account_name = "bob";
      acc_create.owner = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      acc_create.active = authority( 1, generate_private_key( "bob_active" ).get_public_key(), 1 );
      acc_create.posting = authority( 1, generate_private_key( "bob_posting" ).get_public_key(), 1 );
      acc_create.secure_public_key = generate_private_key( "bob_secure" ).get_public_key();
      acc_create.connection_public_key = generate_private_key( "bob_connection" ).get_public_key();
      acc_create.friend_public_key = generate_private_key( "bob_friend" ).get_public_key();
      acc_create.companion_public_key = generate_private_key( "bob_companion" ).get_public_key();
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      signed_transaction tx;
      tx.operations.push_back( acc_create );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& bob_auth = db.get< account_authority_object, by_account >( "bob" );
      BOOST_REQUIRE( bob_auth.owner == acc_create.owner );

      // Changing bob's owner authority

      account_update_operation acc_update;
      acc_update.signatory = "bob";
      acc_update.account = "bob";
      acc_update.owner = authority( 1, generate_private_key( "bad_key" ).get_public_key(), 1 );
      acc_update.secure_public_key = generate_private_key( "bob_secure" ).get_public_key();
      acc_update.connection_public_key = generate_private_key( "bob_connection" ).get_public_key();
      acc_update.friend_public_key = generate_private_key( "bob_friend" ).get_public_key();
      acc_update.companion_public_key = generate_private_key( "bob_companion" ).get_public_key();
      acc_update.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_update.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_update.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_update.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_update.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( acc_update );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_auth.owner == *acc_update.owner );

      tx.operations.clear();
      tx.signatures.clear();

      request_account_recovery_operation request;
      request.signatory = "alice";
      request.recovery_account = "alice";
      request.account_to_recover = "bob";
      request.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_auth.owner == *acc_update.owner );

      generate_blocks( now + OWNER_UPDATE_LIMIT );

      tx.operations.clear();
      tx.signatures.clear();

      recover_account_operation recover;
      recover.signatory = "bob";
      recover.account_to_recover = "bob";
      recover.new_owner_authority = request.new_owner_authority;
      recover.recent_owner_authority = acc_create.owner;

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );
      const auto& owner1 = db.get< account_authority_object, by_account >("bob").owner;

      BOOST_REQUIRE( owner1 == recover.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal request account recovery" );

      tx.operations.clear();
      tx.signatures.clear();

      request.new_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when bob does not have new requested authority" );

      generate_blocks( now + OWNER_UPDATE_LIMIT + fc::seconds( BLOCK_INTERVAL ) );

      recover.new_owner_authority = authority( 1, generate_private_key( "idontknow" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "idontknow" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      const auto& owner2 = db.get< account_authority_object, by_account >("bob").owner;
      BOOST_REQUIRE( owner2 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when bob does not have new requested authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when bob does not have old authority" );

      recover.recent_owner_authority = authority( 1, generate_private_key( "idontknow" ).get_public_key(), 1 );
      recover.new_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      tx.sign( generate_private_key( "idontknow" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      const auto& owner3 = db.get< account_authority_object, by_account >("bob").owner;
      BOOST_REQUIRE( owner3 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when bob does not have old authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: using the same old owner auth again for recovery" );

      recover.recent_owner_authority = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      recover.new_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& owner4 = db.get< account_authority_object, by_account >("bob").owner;
      BOOST_REQUIRE( owner4 == recover.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: using the same old owner auth again for recovery" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: creating a recovery request that will expire" );

      request.new_owner_authority = authority( 1, generate_private_key( "expire" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& request_idx = db.get_index< account_recovery_request_index >().indices();
      auto req_itr = request_idx.begin();

      BOOST_REQUIRE( req_itr->account_to_recover == "bob" );
      BOOST_REQUIRE( req_itr->new_owner_authority == authority( 1, generate_private_key( "expire" ).get_public_key(), 1 ) );
      BOOST_REQUIRE( req_itr->expires == now + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD );
      auto expires = req_itr->expires;
      ++req_itr;
      BOOST_REQUIRE( req_itr == request_idx.end() );

      generate_blocks( time_point( expires - BLOCK_INTERVAL ), true );

      const auto& new_request_idx = db.get_index< account_recovery_request_index >().indices();
      BOOST_REQUIRE( new_request_idx.begin() != new_request_idx.end() );    // Request has not yet expired

      generate_block();

      BOOST_REQUIRE( new_request_idx.begin() == new_request_idx.end() );    // Request has now expired

      recover.new_owner_authority = authority( 1, generate_private_key( "expire" ).get_public_key(), 1 );
      recover.recent_owner_authority = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now );
      tx.sign( generate_private_key( "expire" ), db.get_chain_id() );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // Recover won't work, because request expired
      const auto& owner5 = db.get< account_authority_object, by_account >("bob").owner;
      BOOST_REQUIRE( owner5 == authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: creating a recovery request that will expire" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Expiring owner authority history" );

      acc_update.owner = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( acc_update );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now + ( OWNER_AUTH_RECOVERY_PERIOD - ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD ) );
      generate_block();

      request.new_owner_authority = authority( 1, generate_private_key( "last key" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      recover.new_owner_authority = request.new_owner_authority;
      recover.recent_owner_authority = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "last key" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // bob_owner key has expired
      const auto& owner6 = db.get< account_authority_object, by_account >("bob").owner;
      BOOST_REQUIRE( owner6 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      recover.recent_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      tx.sign( generate_private_key( "last key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );     // foo bar key has not yet expired
      const auto& owner7 = db.get< account_authority_object, by_account >("bob").owner;
      BOOST_REQUIRE( owner7 == authority( 1, generate_private_key( "last key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Expiring owner authority history" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: change recovery account" );

      auto change_recovery_account = [&]( const std::string& account_to_recover, const std::string& new_recovery_account )
      {
         change_recovery_account_operation op;
         op.account_to_recover = account_to_recover;
         op.new_recovery_account = new_recovery_account;

         signed_transaction tx;
         tx.operations.push_back( op );
         tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      auto recover_account = [&]( const std::string& account_to_recover, const fc::ecc::private_key& new_owner_key, const fc::ecc::private_key& recent_owner_key )
      {
         recover_account_operation op;
         op.account_to_recover = account_to_recover;
         op.new_owner_authority = authority( 1, public_key_type( new_owner_key.get_public_key() ), 1 );
         op.recent_owner_authority = authority( 1, public_key_type( recent_owner_key.get_public_key() ), 1 );

         signed_transaction tx;
         tx.operations.push_back( op );
         tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( recent_owner_key, db.get_chain_id() );
         // only Alice -> throw
         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
         tx.signatures.clear();
         tx.sign( new_owner_key, db.get_chain_id() );
         // only Sam -> throw
         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
         tx.sign( recent_owner_key, db.get_chain_id() );
         // Alice+Sam -> OK
         db.push_transaction( tx, 0 );
      };

      auto request_account_recovery = [&]( const std::string& recovery_account, const fc::ecc::private_key& recovery_account_key, const std::string& account_to_recover, const public_key_type& new_owner_key )
      {
         request_account_recovery_operation op;
         op.recovery_account    = recovery_account;
         op.account_to_recover  = account_to_recover;
         op.new_owner_authority = authority( 1, new_owner_key, 1 );

         signed_transaction tx;
         tx.operations.push_back( op );
         tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( recovery_account_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      auto change_owner = [&]( const std::string& account, const fc::ecc::private_key& old_private_key, const public_key_type& new_public_key )
      {
         account_update_operation op;
         op.account = account;
         op.owner = authority( 1, new_public_key, 1 );

         signed_transaction tx;
         tx.operations.push_back( op );
         tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( old_private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      // if either/both users do not exist, we shouldn't allow it
      REQUIRE_THROW( change_recovery_account( "alice", "nobody" ), fc::exception );
      REQUIRE_THROW( change_recovery_account( "haxer", "sam" ), fc::exception );
      REQUIRE_THROW( change_recovery_account( "haxer", "nobody" ), fc::exception );
      change_recovery_account( "alice", "sam" );

      fc::ecc::private_key alice_priv1 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k1" ) );
      fc::ecc::private_key alice_priv2 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k2" ) );
      public_key_type alice_pub1 = public_key_type( alice_priv1.get_public_key() );

      generate_blocks( now + OWNER_AUTH_RECOVERY_PERIOD - fc::seconds( BLOCK_INTERVAL ), true );
      // cannot request account recovery until recovery account is approved
      REQUIRE_THROW( request_account_recovery( "sam", sam_private_owner_key, "alice", alice_pub1 ), fc::exception );
      generate_blocks(1);
      // cannot finish account recovery until requested
      REQUIRE_THROW( recover_account( "alice", alice_priv1, alice_private_owner_key ), fc::exception );
      // do the request
      request_account_recovery( "sam", sam_private_owner_key, "alice", alice_pub1 );
      // can't recover with the current owner key
      REQUIRE_THROW( recover_account( "alice", alice_priv1, alice_private_owner_key ), fc::exception );
      // unless we change it!
      change_owner( "alice", alice_private_owner_key, public_key_type( alice_priv2.get_public_key() ) );
      recover_account( "alice", alice_priv1, alice_private_owner_key );

      BOOST_TEST_MESSAGE( "│   ├── Passed: change_recovery_account_operation" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT RECOVERY SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_reset_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT RESET SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account reset" );

      time_point now = db.head_block_time();

      ACTORS( (alice)(bob)(chris)(dave) );
      fund( "alice", 1000 * BLOCKCHAIN_PRECISION );
      fund_stake( "alice", 1000 * BLOCKCHAIN_PRECISION );

      account_create_operation acc_create;       // Creating account bob with alice as reset account

      acc_create.signatory = "alice";
      acc_create.registrar = "alice";
      acc_create.recovery_account = "alice";
      acc_create.reset_account = "alice";
      acc_create.account_type = PERSONA;
      acc_create.new_account_name = "bob";
      acc_create.owner = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      acc_create.active = authority( 1, generate_private_key( "bob_active" ).get_public_key(), 1 );
      acc_create.posting = authority( 1, generate_private_key( "bob_posting" ).get_public_key(), 1 );
      acc_create.secure_public_key = generate_private_key( "bob_secure" ).get_public_key();
      acc_create.connection_public_key = generate_private_key( "bob_connection" ).get_public_key();
      acc_create.friend_public_key = generate_private_key( "bob_friend" ).get_public_key();
      acc_create.companion_public_key = generate_private_key( "bob_companion" ).get_public_key();
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      signed_transaction tx;
      tx.operations.push_back( acc_create );
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& bob_auth = db.get< account_authority_object, by_account >( "bob" );
      BOOST_REQUIRE( bob_auth.owner == acc_create.owner );

      generate_blocks( now + ( fc::days( bob.reset_account_delay_days ) - BLOCK_INTERVAL ) );

      reset_account_operation reset;

      reset.signatory = "alice";
      reset.reset_account = "alice";
      reset.account_to_reset = "bob";
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

      tx.operations.push_back( reset );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet

      generate_block();

      db.push_transaction( tx, 0 );    // reset now valid

      BOOST_REQUIRE( bob_auth.owner == reset.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account reset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: set new reset account" );

      tx.operations.clear();
      tx.signatures.clear();

      set_reset_account_operation set;

      set.signatory = "bob";
      set.account = "bob";
      set.new_reset_account = "chris";

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );
      
      BOOST_REQUIRE( bob.reset_account == "chris" );

      BOOST_TEST_MESSAGE( "│   ├── Passed: set new reset account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional account reset" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now + ( fc::days( bob.reset_account_delay_days ) - BLOCK_INTERVAL ) );

      reset.signatory = "chris";
      reset.reset_account = "chris";
      reset.account_to_reset = "bob";
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key_1" ).get_public_key(), 1 );

      tx.operations.push_back( reset );
      tx.sign( chris_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet

      generate_block();

      db.push_transaction( tx, 0 );    // reset now valid

      BOOST_REQUIRE( bob_auth.owner == reset.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: additional account reset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when setting reset account that does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      set.new_reset_account = "spudington";

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key_1" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when setting reset account that does not exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when setting reset account to current reset account" );

      tx.operations.clear();
      tx.signatures.clear();

      set.new_reset_account = "chris";

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key_1" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when setting reset account to current reset account" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT RESET SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( decline_voting_rights_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: DECLINE VOTING RIGHTS" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: decline voting rights request" );

      time_point now =  db.head_block_time();

      ACTORS( (alice)(bob) );
      generate_block();
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      generate_block();

      decline_voting_rights_operation op;
      op.signatory = "alice";
      op.account = "alice";

      signed_transaction tx;
      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< decline_voting_rights_request_index >().indices().get< by_account >();
      auto req_itr = req_idx.find( "alice" );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->effective_date == now + DECLINE_VOTING_RIGHTS_DURATION );

      BOOST_TEST_MESSAGE( "│   ├── Passed: decline voting rights request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure revoking voting rights with existing request" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure revoking voting rights with existing request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success cancelling a request" );

      op.declined = false;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( "alice" );
      BOOST_REQUIRE( itr == req_idx.end() );    // Request is now removed

      BOOST_TEST_MESSAGE( "│   ├── Passed: success cancelling a request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure cancelling a request that doesn't exist" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // Can't cancel again

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure cancelling a request that doesn't exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account can vote during waiting period" );

      op.decline = true;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now + DECLINE_VOTING_RIGHTS_DURATION - BLOCK_INTERVAL, true );

      BOOST_REQUIRE( db.get_account( "alice" ).can_vote );   // Alice can still vote

      witness_create( 
         "alice", 
         alice_private_owner_key,
         "details",
         "url",
         "json",
         alice_private_owner_key.get_public_key(),
         BLOCKCHAIN_PRECISION * 10 );

      account_witness_vote_operation witness_vote;
      witness_vote.signatory = "alice";
      witness_vote.account = "alice";
      witness_vote.witness = "alice";
      witness_vote.vote_rank = 1;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( witness_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";
      comment.post_type = TEXT_POST;
      comment.language = "en";
      comment.privacy = false;
      comment.reach = TAG_FEED;
      comment.interface = INIT_ACCOUNT;
      comment.rating = GENERAL;
      comment.tags.push_back( "test" );
      comment.json = "{\"json\":\"valid\"}";

      vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.operations.push_back( vote );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& comment_idx = db.get_index< comment_index >().indices().get< by_permlink >();
      const auto& comment_vote_idx = db.get_index< comment_vote_index >().indices().get< by_voter_comment >();
      const auto& witness_vote_idx = db.get_index< witness_vote_index >().indices().get< by_witness_account >();

      auto comment_itr = comment_idx.find( boost::make_tuple( "alice", "test" ) );
      auto comment_vote_itr = comment_vote_idx.find( boost::make_tuple( "alice", comment_itr->id ) );
      auto witness_vote_itr = witness_vote_idx.find( boost::make_tuple( "alice", "alice" ) );

      BOOST_REQUIRE( comment_itr->author == "alice" );
      BOOST_REQUIRE( comment_vote_itr->voter == "alice" );
      BOOST_REQUIRE( witness_vote_itr->account == "alice" );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account can vote during waiting period" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account cannot vote after request is processed" );

      generate_block();
      BOOST_REQUIRE( !db.get_account( "alice" ).can_vote );   // Alice can no longer vote
      validate_database();

      req_itr = request_idx.find( "alice" );

      BOOST_REQUIRE( req_itr == request_idx.end() );   // Request no longer exists

      auto witness_vote_itr = witness_vote_idx.find( boost::make_tuple( "alice", "alice" ) );

      BOOST_REQUIRE( witness_vote_itr == witness_vote_idx.end() );   // Vote has been removed

      tx.operations.clear();
      tx.signatures.clear();

      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( witness_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account cannot vote after request is processed" );

      BOOST_TEST_MESSAGE( "├── Passed: DECLINE VOTING RIGHTS" );
   }
   FC_LOG_AND_RETHROW()
}

/**
 * connection_request_operation,
   connection_accept_operation,
   account_follow_operation,
   tag_follow_operation,
   activity_reward_operation,
   */

BOOST_AUTO_TEST_CASE( connection_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CONNECTION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: create connection request" );

      time_point now =  db.head_block_time();

      ACTORS( (alice)(bob)(chris)(dave)(elon) );
      generate_block();
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      generate_block();

      connection_request_operation request;

      request.signatory = "alice";
      request.account = "alice";
      request.requested_account = "bob";
      request.connection_type = CONNECTION;
      request.message = "Hello";
      request.requested = true;

      signed_transaction tx;

      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< connection_request_index >().indices().get< by_account_req >();
      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->account == "alice" );
      BOOST_REQUIRE( req_itr->requested_account == "bob" );
      BOOST_REQUIRE( req_itr->expiration == now + CONNECTION_REQUEST_DURATION );

      BOOST_TEST_MESSAGE( "│   ├── Passed: create connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept connection request" );

      generate_block();

      connection_accept_operation accept;

      accept.signatory = "bob";
      accept.account = "bob";
      accept.requesting_account = "alice";
      accept.connection_type = CONNECTION;
      accept.connection_id = 
      accept.message = "Hello";
      accept.requested = true;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure revoking voting rights with existing request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success cancelling a request" );

      op.declined = false;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( "alice" );
      BOOST_REQUIRE( itr == req_idx.end() );    // Request is now removed

      BOOST_TEST_MESSAGE( "│   ├── Passed: success cancelling a request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure cancelling a request that doesn't exist" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // Can't cancel again

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure cancelling a request that doesn't exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account can vote during waiting period" );

      op.decline = true;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now + DECLINE_VOTING_RIGHTS_DURATION - BLOCK_INTERVAL, true );

      BOOST_REQUIRE( db.get_account( "alice" ).can_vote );   // Alice can still vote

      witness_create( 
         "alice", 
         alice_private_owner_key,
         "details",
         "url",
         "json",
         alice_private_owner_key.get_public_key(),
         BLOCKCHAIN_PRECISION * 10 );

      account_witness_vote_operation witness_vote;
      witness_vote.signatory = "alice";
      witness_vote.account = "alice";
      witness_vote.witness = "alice";
      witness_vote.vote_rank = 1;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( witness_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";
      comment.post_type = TEXT_POST;
      comment.language = "en";
      comment.privacy = false;
      comment.reach = TAG_FEED;
      comment.interface = INIT_ACCOUNT;
      comment.rating = GENERAL;
      comment.tags.push_back( "test" );
      comment.json = "{\"json\":\"valid\"}";

      vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.operations.push_back( vote );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& comment_idx = db.get_index< comment_index >().indices().get< by_permlink >();
      const auto& comment_vote_idx = db.get_index< comment_vote_index >().indices().get< by_voter_comment >();
      const auto& witness_vote_idx = db.get_index< witness_vote_index >().indices().get< by_witness_account >();

      auto comment_itr = comment_idx.find( boost::make_tuple( "alice", "test" ) );
      auto comment_vote_itr = comment_vote_idx.find( boost::make_tuple( "alice", comment_itr->id ) );
      auto witness_vote_itr = witness_vote_idx.find( boost::make_tuple( "alice", "alice" ) );

      BOOST_REQUIRE( comment_itr->author == "alice" );
      BOOST_REQUIRE( comment_vote_itr->voter == "alice" );
      BOOST_REQUIRE( witness_vote_itr->account == "alice" );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account can vote during waiting period" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account cannot vote after request is processed" );

      generate_block();
      BOOST_REQUIRE( !db.get_account( "alice" ).can_vote );   // Alice can no longer vote
      validate_database();

      req_itr = request_idx.find( "alice" );

      BOOST_REQUIRE( req_itr == request_idx.end() );   // Request no longer exists

      auto witness_vote_itr = witness_vote_idx.find( boost::make_tuple( "alice", "alice" ) );

      BOOST_REQUIRE( witness_vote_itr == witness_vote_idx.end() );   // Vote has been removed

      tx.operations.clear();
      tx.signatures.clear();

      tx.set_expiration( now + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( witness_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account cannot vote after request is processed" );

      BOOST_TEST_MESSAGE( "├── Passed: DECLINE VOTING RIGHTS" );
   }
   FC_LOG_AND_RETHROW()
}



   //============================//
   // === Network Operations === //
   //============================//






   //=====================================//
   // === Post and Comment Operations === //
   //=====================================//



BOOST_AUTO_TEST_CASE( comment_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: comment_validate" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: comment_authorities" );

      ACTORS( (alice)(bob) );
      generate_blocks( 60 / BLOCK_INTERVAL );

      comment_operation op;
      op.author = "alice";
      op.permlink = "lorem";
      op.parent_author = "";
      op.parent_permlink = "ipsum";
      op.title = "Lorem Ipsum";
      op.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      op.json = "{\"foo\":\"bar\"}";

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      BOOST_TEST_MESSAGE( "--- Test failure when no signatures" );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_posting_auth );

      BOOST_TEST_MESSAGE( "--- Test failure when duplicate signatures" );
      tx.sign( alice_post_key, db.get_chain_id() );
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test success with post signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by an additional signature not in the creator's authority" );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by a signature not in the creator's authority" );
      tx.signatures.clear();
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_posting_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: comment_apply" );

      ACTORS( (alice)(bob)(sam) )
      generate_blocks( 60 / BLOCK_INTERVAL );

      comment_operation op;
      op.author = "alice";
      op.permlink = "lorem";
      op.parent_author = "";
      op.parent_permlink = "ipsum";
      op.title = "Lorem Ipsum";
      op.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      op.json = "{\"foo\":\"bar\"}";

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      BOOST_TEST_MESSAGE( "--- Test Alice posting a root comment" );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      BOOST_REQUIRE( alice_comment.author == op.author );
      BOOST_REQUIRE( to_string( alice_comment.permlink ) == op.permlink );
      BOOST_REQUIRE( to_string( alice_comment.parent_permlink ) == op.parent_permlink );
      BOOST_REQUIRE( alice_comment.last_update == db.head_block_time() );
      BOOST_REQUIRE( alice_comment.created == db.head_block_time() );
      BOOST_REQUIRE( alice_comment.net_reward.value == 0 );
      BOOST_REQUIRE( alice_comment.abs_reward.value == 0 );
      BOOST_REQUIRE( alice_comment.cashout_time == fc::time_point( db.head_block_time() + fc::seconds( CONTENT_REWARD_INTERVAL ) ) );

      #ifndef IS_LOW_MEM
         BOOST_REQUIRE( to_string( alice_comment.title ) == op.title );
         BOOST_REQUIRE( to_string( alice_comment.body ) == op.body );
         //BOOST_REQUIRE( alice_comment.json == op.json );
      #else
         BOOST_REQUIRE( to_string( alice_comment.title ) == "" );
         BOOST_REQUIRE( to_string( alice_comment.body ) == "" );
         //BOOST_REQUIRE( alice_comment.json == "" );
      #endif

      validate_database();

      BOOST_TEST_MESSAGE( "--- Test Bob posting a comment on a non-existent comment" );
      op.author = "bob";
      op.permlink = "ipsum";
      op.parent_author = "alice";
      op.parent_permlink = "foobar";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- Test Bob posting a comment on Alice's comment" );
      op.parent_permlink = "lorem";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( "bob", string( "ipsum" ) );

      BOOST_REQUIRE( bob_comment.author == op.author );
      BOOST_REQUIRE( to_string( bob_comment.permlink ) == op.permlink );
      BOOST_REQUIRE( bob_comment.parent_author == op.parent_author );
      BOOST_REQUIRE( to_string( bob_comment.parent_permlink ) == op.parent_permlink );
      BOOST_REQUIRE( bob_comment.last_update == db.head_block_time() );
      BOOST_REQUIRE( bob_comment.created == db.head_block_time() );
      BOOST_REQUIRE( bob_comment.net_reward.value == 0 );
      BOOST_REQUIRE( bob_comment.abs_reward.value == 0 );
      BOOST_REQUIRE( bob_comment.cashout_time == bob_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_comment.root_comment == alice_comment.id );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test Sam posting a comment on Bob's comment" );

      op.author = "sam";
      op.permlink = "dolor";
      op.parent_author = "bob";
      op.parent_permlink = "ipsum";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& sam_comment = db.get_comment( "sam", string( "dolor" ) );

      BOOST_REQUIRE( sam_comment.author == op.author );
      BOOST_REQUIRE( to_string( sam_comment.permlink ) == op.permlink );
      BOOST_REQUIRE( sam_comment.parent_author == op.parent_author );
      BOOST_REQUIRE( to_string( sam_comment.parent_permlink ) == op.parent_permlink );
      BOOST_REQUIRE( sam_comment.last_update == db.head_block_time() );
      BOOST_REQUIRE( sam_comment.created == db.head_block_time() );
      BOOST_REQUIRE( sam_comment.net_reward.value == 0 );
      BOOST_REQUIRE( sam_comment.abs_reward.value == 0 );
      BOOST_REQUIRE( sam_comment.cashout_time == sam_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( sam_comment.root_comment == alice_comment.id );
      validate_database();

      generate_blocks( 60 * 5 / BLOCK_INTERVAL + 1 );

      BOOST_TEST_MESSAGE( "--- Test modifying a comment" );
      const auto& mod_sam_comment = db.get_comment( "sam", string( "dolor" ) );
      const auto& mod_bob_comment = db.get_comment( "bob", string( "ipsum" ) );
      const auto& mod_alice_comment = db.get_comment( "alice", string( "lorem" ) );
      fc::time_point created = mod_sam_comment.created;

      db.modify( mod_sam_comment, [&]( comment_object& com )
      {
         com.net_reward = 10;
         com.abs_reward = 10;
      });

      db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& o)
      {
         o.total_reward_squared = node::chain::util::evaluate_reward_curve( 10 );
      });

      tx.signatures.clear();
      tx.operations.clear();
      op.title = "foo";
      op.body = "bar";
      op.json = "{\"bar\":\"foo\"}";
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( mod_sam_comment.author == op.author );
      BOOST_REQUIRE( to_string( mod_sam_comment.permlink ) == op.permlink );
      BOOST_REQUIRE( mod_sam_comment.parent_author == op.parent_author );
      BOOST_REQUIRE( to_string( mod_sam_comment.parent_permlink ) == op.parent_permlink );
      BOOST_REQUIRE( mod_sam_comment.last_update == db.head_block_time() );
      BOOST_REQUIRE( mod_sam_comment.created == created );
      BOOST_REQUIRE( mod_sam_comment.cashout_time == mod_sam_comment.created + CONTENT_REWARD_INTERVAL );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure posting withing 1 minute" );

      op.permlink = "sit";
      op.parent_author = "";
      op.parent_permlink = "test";
      tx.operations.clear();
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( 60 * 5 / BLOCK_INTERVAL );

      op.permlink = "amet";
      tx.operations.clear();
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      generate_block();
      db.push_transaction( tx, 0 );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_beneficiaries_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Test Comment Beneficiaries Validate" );
      comment_options_operation op;

      op.author = "alice";
      op.permlink = "test";

      BOOST_TEST_MESSAGE( "--- Testing more than 100% weight on a single route" );
      comment_payout_beneficiaries b;
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_100 + 1 ) );
      op.extensions.insert( b );
      REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Testing more than 100% total weight" );
      b.beneficiaries.clear();
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 * 75 ) );
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "sam" ), PERCENT_1 * 75 ) );
      op.extensions.clear();
      op.extensions.insert( b );
      REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Testing maximum number of routes" );
      b.beneficiaries.clear();
      for( size_t i = 0; i < 127; i++ )
      {
         b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "foo" + fc::to_string( i ) ), 1 ) );
      }

      op.extensions.clear();
      std::sort( b.beneficiaries.begin(), b.beneficiaries.end() );
      op.extensions.insert( b );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Testing one too many routes" );
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bar" ), 1 ) );
      std::sort( b.beneficiaries.begin(), b.beneficiaries.end() );
      op.extensions.clear();
      op.extensions.insert( b );
      REQUIRE_THROW( op.validate(), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Testing duplicate accounts" );
      b.beneficiaries.clear();
      b.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 * 2 ) );
      b.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      op.extensions.clear();
      op.extensions.insert( b );
      REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Testing incorrect account sort order" );
      b.beneficiaries.clear();
      b.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      b.beneficiaries.push_back( beneficiary_route_type( "alice", PERCENT_1 ) );
      op.extensions.clear();
      op.extensions.insert( b );
      REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Testing correct account sort order" );
      b.beneficiaries.clear();
      b.beneficiaries.push_back( beneficiary_route_type( "alice", PERCENT_1 ) );
      b.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      op.extensions.clear();
      op.extensions.insert( b );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_beneficiaries_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Test Comment Beneficiaries" );
      ACTORS( (alice)(bob)(sam)(dave) )
      generate_block();

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) ) );

      comment_operation comment;
      vote_operation vote;
      comment_options_operation op;
      comment_payout_beneficiaries b;
      signed_transaction tx;

      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "foobar";

      tx.operations.push_back( comment );
      tx.set_expiration( db.head_block_time() + MIN_TRANSACTION_EXPIRATION_LIMIT );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx );

      BOOST_TEST_MESSAGE( "--- Test failure on more than 8 benefactors" );
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 ) );

      for( size_t i = 0; i < 8; i++ )
      {
         b.beneficiaries.push_back( beneficiary_route_type( account_name_type( INIT_ACCOUNT + fc::to_string( i ) ), PERCENT_1 ) );
      }

      op.author = "alice";
      op.permlink = "test";
      op.allow_curation_rewards = false;
      op.extensions.insert( b );
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), chain::plugin_exception );


      BOOST_TEST_MESSAGE( "--- Test specifying a non-existent benefactor" );
      b.beneficiaries.clear();
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "doug" ), PERCENT_1 ) );
      op.extensions.clear();
      op.extensions.insert( b );
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Test setting when comment has been voted on" );
      vote.author = "alice";
      vote.permlink = "test";
      vote.voter = "bob";
      vote.weight = PERCENT_100;

      b.beneficiaries.clear();
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), 25 * PERCENT_1 ) );
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "sam" ), 50 * PERCENT_1 ) );
      op.extensions.clear();
      op.extensions.insert( b );

      tx.clear();
      tx.operations.push_back( vote );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Test success" );
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx );


      BOOST_TEST_MESSAGE( "--- Test setting when there are already beneficiaries" );
      b.beneficiaries.clear();
      b.beneficiaries.push_back( beneficiary_route_type( account_name_type( "dave" ), 25 * PERCENT_1 ) );
      op.extensions.clear();
      op.extensions.insert( b );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Payout and verify rewards were split properly" );
      tx.clear();
      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time - BLOCK_INTERVAL );

      db_plugin->debug_update( [=]( database& db )
      {
         
      });

      generate_block();

      BOOST_REQUIRE( db.get_account( "bob" ).TMErewardBalance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).USDrewardBalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).rewardBalanceInTME.amount + db.get_account( "sam" ).rewardBalanceInTME.amount == db.get_comment( "alice", string( "test" ) ).beneficiary_payout_value.amount );
      BOOST_REQUIRE( ( db.get_account( "alice" ).USDrewardBalance.amount + db.get_account( "alice" ).rewardBalanceInTME.amount ) == db.get_account( "bob" ).rewardBalanceInTME.amount + 2 );
      BOOST_REQUIRE( ( db.get_account( "alice" ).USDrewardBalance.amount + db.get_account( "alice" ).rewardBalanceInTME.amount ) * 2 == db.get_account( "sam" ).rewardBalanceInTME.amount + 3 );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( vote_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: vote_validate" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( vote_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: vote_authorities" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( vote_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: vote_apply" );

      ACTORS( (alice)(bob)(sam)(dave) )
      generate_block();

      fund_stake( "alice", ASSET( "10.000 TESTS" ) );
      validate_database();
      fund_stake( "bob" , ASSET( "10.000 TESTS" ) );
      fund_stake( "sam" , ASSET( "10.000 TESTS" ) );
      fund_stake( "dave" , ASSET( "10.000 TESTS" ) );
      generate_block();

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      {
         const auto& alice = db.get_account( "alice" );

         signed_transaction tx;
         comment_operation comment_op;
         comment_op.author = "alice";
         comment_op.permlink = "foo";
         comment_op.parent_permlink = "test";
         comment_op.title = "bar";
         comment_op.body = "foo bar";
         tx.operations.push_back( comment_op );
         tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         BOOST_TEST_MESSAGE( "--- Testing voting on a non-existent comment" );

         tx.operations.clear();
         tx.signatures.clear();

         vote_operation op;
         op.voter = "alice";
         op.author = "bob";
         op.permlink = "foo";
         op.weight = PERCENT_100;
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );

         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

         validate_database();

         BOOST_TEST_MESSAGE( "--- Testing voting with a weight of 0" );

         op.weight = (int16_t) 0;
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );

         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

         validate_database();

         BOOST_TEST_MESSAGE( "--- Testing success" );

         auto old_voting_power = alice.voting_power;

         op.weight = PERCENT_100;
         op.author = "alice";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );

         db.push_transaction( tx, 0 );

         auto& alice_comment = db.get_comment( "alice", string( "foo" ) );
         auto itr = vote_idx.find( std::make_tuple( alice_comment.id, alice.id ) );
         int64_t max_vote_denom = ( db.get_dynamic_global_properties().vote_power_reserve_rate * VOTE_REGENERATION_SECONDS ) / (60*60*24);

         BOOST_REQUIRE( alice.voting_power == old_voting_power - ( ( old_voting_power + max_vote_denom - 1 ) / max_vote_denom ) );
         BOOST_REQUIRE( alice.last_vote_time == db.head_block_time() );
         BOOST_REQUIRE( alice_comment.net_reward.value == alice.SCORE.amount.value * ( old_voting_power - alice.voting_power ) / PERCENT_100 );
         BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
         BOOST_REQUIRE( itr->reward == alice.SCORE.amount.value * ( old_voting_power - alice.voting_power ) / PERCENT_100 );
         BOOST_REQUIRE( itr != vote_idx.end() );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test reduced power for quick voting" );

         generate_blocks( db.head_block_time() + MIN_VOTE_INTERVAL_SEC );

         old_voting_power = db.get_account( "alice" ).voting_power;

         comment_op.author = "bob";
         comment_op.permlink = "foo";
         comment_op.title = "bar";
         comment_op.body = "foo bar";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( comment_op );
         tx.sign( bob_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         op.weight = PERCENT_100 / 2;
         op.voter = "alice";
         op.author = "bob";
         op.permlink = "foo";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         const auto& bob_comment = db.get_comment( "bob", string( "foo" ) );
         itr = vote_idx.find( std::make_tuple( bob_comment.id, alice.id ) );

         BOOST_REQUIRE( db.get_account( "alice" ).voting_power == old_voting_power - ( ( old_voting_power + max_vote_denom - 1 ) * PERCENT_100 / ( 2 * max_vote_denom * PERCENT_100 ) ) );
         BOOST_REQUIRE( bob_comment.net_reward.value == alice.SCORE.amount.value * ( old_voting_power - db.get_account( "alice" ).voting_power ) / PERCENT_100 );
         BOOST_REQUIRE( bob_comment.cashout_time == bob_comment.created + CONTENT_REWARD_INTERVAL );
         BOOST_REQUIRE( itr != vote_idx.end() );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test payout time extension on vote" );

         old_voting_power = db.get_account( "bob" ).voting_power;
         auto old_abs_reward = db.get_comment( "alice", string( "foo" ) ).abs_reward.value;

         generate_blocks( db.head_block_time() + fc::seconds( ( CONTENT_REWARD_INTERVAL / 2 ) ), true );

         const auto& new_bob = db.get_account( "bob" );
         const auto& new_alice_comment = db.get_comment( "alice", string( "foo" ) );

         op.weight = PERCENT_100;
         op.voter = "bob";
         op.author = "alice";
         op.permlink = "foo";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( bob_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         itr = vote_idx.find( std::make_tuple( new_alice_comment.id, new_bob.id ) );
         uint128_t new_cashout_time = db.head_block_time().sec_since_epoch() + CONTENT_REWARD_INTERVAL;

         BOOST_REQUIRE( new_bob.voting_power == PERCENT_100 - ( ( PERCENT_100 + max_vote_denom - 1 ) / max_vote_denom ) );
         BOOST_REQUIRE( new_alice_comment.net_reward.value == old_abs_reward + new_bob.SCORE.amount.value * ( old_voting_power - new_bob.voting_power ) / PERCENT_100 );
         BOOST_REQUIRE( new_alice_comment.cashout_time == new_alice_comment.created + CONTENT_REWARD_INTERVAL );
         BOOST_REQUIRE( itr != vote_idx.end() );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test negative vote" );

         const auto& new_sam = db.get_account( "sam" );
         const auto& new_bob_comment = db.get_comment( "bob", string( "foo" ) );

         old_abs_reward = new_bob_comment.abs_reward.value;

         op.weight = -1 * PERCENT_100 / 2;
         op.voter = "sam";
         op.author = "bob";
         op.permlink = "foo";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( sam_private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         itr = vote_idx.find( std::make_tuple( new_bob_comment.id, new_sam.id ) );
         new_cashout_time = db.head_block_time().sec_since_epoch() + CONTENT_REWARD_INTERVAL;
         auto sam_weight /*= ( ( uint128_t( new_sam.SCORE.amount.value ) ) / 400 + 1 ).to_uint64();*/
                         = ( ( uint128_t( new_sam.SCORE.amount.value ) * ( ( PERCENT_100 + max_vote_denom - 1 ) / ( 2 * max_vote_denom ) ) ) / PERCENT_100 ).to_uint64();

         BOOST_REQUIRE( new_sam.voting_power == PERCENT_100 - ( ( PERCENT_100 + max_vote_denom - 1 ) / ( 2 * max_vote_denom ) ) );
         BOOST_REQUIRE( new_bob_comment.net_reward.value == old_abs_reward - sam_weight );
         BOOST_REQUIRE( new_bob_comment.abs_reward.value == old_abs_reward + sam_weight );
         BOOST_REQUIRE( new_bob_comment.cashout_time == new_bob_comment.created + CONTENT_REWARD_INTERVAL );
         BOOST_REQUIRE( itr != vote_idx.end() );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test nested voting on nested comments" );

         old_abs_reward = new_alice_comment.children_abs_reward.value;
         int64_t regenerated_power = (PERCENT_100 * ( db.head_block_time() - db.get_account( "alice").last_vote_time ).to_seconds() ) / VOTE_REGENERATION_SECONDS;
         int64_t used_power = ( db.get_account( "alice" ).voting_power + regenerated_power + max_vote_denom - 1 ) / max_vote_denom;

         comment_op.author = "sam";
         comment_op.permlink = "foo";
         comment_op.title = "bar";
         comment_op.body = "foo bar";
         comment_op.parent_author = "alice";
         comment_op.parent_permlink = "foo";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( comment_op );
         tx.sign( sam_private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         op.weight = PERCENT_100;
         op.voter = "alice";
         op.author = "sam";
         op.permlink = "foo";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         auto new_reward = ( ( fc::uint128_t( db.get_account( "alice" ).SCORE.amount.value ) * used_power ) / PERCENT_100 ).to_uint64();

         BOOST_REQUIRE( db.get_comment( "alice", string( "foo" ) ).cashout_time == db.get_comment( "alice", string( "foo" ) ).created + CONTENT_REWARD_INTERVAL );

         validate_database();

         BOOST_TEST_MESSAGE( "--- Test increasing vote reward" );

         generate_blocks( db.head_block_time() + MIN_VOTE_INTERVAL_SEC );

         auto new_alice = db.get_account( "alice" );
         auto alice_bob_vote = vote_idx.find( std::make_tuple( new_bob_comment.id, new_alice.id ) );
         auto old_vote_reward = alice_bob_vote->reward;
         auto old_net_reward = new_bob_comment.net_reward.value;
         old_abs_reward = new_bob_comment.abs_reward.value;
         used_power = ( ( PERCENT_1 * 25 * ( new_alice.voting_power ) / PERCENT_100 ) + max_vote_denom - 1 ) / max_vote_denom;
         auto alice_voting_power = new_alice.voting_power - used_power;

         op.voter = "alice";
         op.weight = PERCENT_1 * 25;
         op.author = "bob";
         op.permlink = "foo";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
         alice_bob_vote = vote_idx.find( std::make_tuple( new_bob_comment.id, new_alice.id ) );

         new_reward = ( ( fc::uint128_t( new_alice.SCORE.amount.value ) * used_power ) / PERCENT_100 ).to_uint64();

         BOOST_REQUIRE( new_bob_comment.net_reward == old_net_reward - old_vote_reward + new_reward );
         BOOST_REQUIRE( new_bob_comment.abs_reward == old_abs_reward + new_reward );
         BOOST_REQUIRE( new_bob_comment.cashout_time == new_bob_comment.created + CONTENT_REWARD_INTERVAL );
         BOOST_REQUIRE( alice_bob_vote->reward == new_reward );
         BOOST_REQUIRE( alice_bob_vote->last_update == db.head_block_time() );
         BOOST_REQUIRE( alice_bob_vote->vote_percent == op.weight );
         BOOST_REQUIRE( db.get_account( "alice" ).voting_power == alice_voting_power );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test decreasing vote reward" );

         generate_blocks( db.head_block_time() + MIN_VOTE_INTERVAL_SEC );

         old_vote_reward = new_reward;
         old_net_reward = new_bob_comment.net_reward.value;
         old_abs_reward = new_bob_comment.abs_reward.value;
         used_power = ( uint64_t( PERCENT_1 ) * 75 * uint64_t( alice_voting_power ) ) / PERCENT_100;
         used_power = ( used_power + max_vote_denom - 1 ) / max_vote_denom;
         alice_voting_power -= used_power;

         op.weight = PERCENT_1 * -75;
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
         alice_bob_vote = vote_idx.find( std::make_tuple( new_bob_comment.id, new_alice.id ) );

         new_reward = ( ( fc::uint128_t( new_alice.SCORE.amount.value ) * used_power ) / PERCENT_100 ).to_uint64();

         BOOST_REQUIRE( new_bob_comment.net_reward == old_net_reward - old_vote_reward - new_reward );
         BOOST_REQUIRE( new_bob_comment.abs_reward == old_abs_reward + new_reward );
         BOOST_REQUIRE( new_bob_comment.cashout_time == new_bob_comment.created + CONTENT_REWARD_INTERVAL );
         BOOST_REQUIRE( alice_bob_vote->reward == -1 * new_reward );
         BOOST_REQUIRE( alice_bob_vote->last_update == db.head_block_time() );
         BOOST_REQUIRE( alice_bob_vote->vote_percent == op.weight );
         BOOST_REQUIRE( db.get_account( "alice" ).voting_power == alice_voting_power );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test changing a vote to 0 weight (aka: removing a vote)" );

         generate_blocks( db.head_block_time() + MIN_VOTE_INTERVAL_SEC );

         old_vote_reward = alice_bob_vote->reward;
         old_net_reward = new_bob_comment.net_reward.value;
         old_abs_reward = new_bob_comment.abs_reward.value;

         op.weight = 0;
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
         alice_bob_vote = vote_idx.find( std::make_tuple( new_bob_comment.id, new_alice.id ) );

         BOOST_REQUIRE( new_bob_comment.net_reward == old_net_reward - old_vote_reward );
         BOOST_REQUIRE( new_bob_comment.abs_reward == old_abs_reward );
         BOOST_REQUIRE( new_bob_comment.cashout_time == new_bob_comment.created + CONTENT_REWARD_INTERVAL );
         BOOST_REQUIRE( alice_bob_vote->reward == 0 );
         BOOST_REQUIRE( alice_bob_vote->last_update == db.head_block_time() );
         BOOST_REQUIRE( alice_bob_vote->vote_percent == op.weight );
         BOOST_REQUIRE( db.get_account( "alice" ).voting_power == alice_voting_power );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test failure when increasing reward within lockout period" );

         generate_blocks( fc::time_point( ( new_bob_comment.cashout_time - UPVOTE_LOCKOUT_TIME ).sec_since_epoch() + BLOCK_INTERVAL ), true );

         op.weight = PERCENT_100;
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );

         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test success when reducing reward within lockout period" );

         op.weight = -1 * PERCENT_100;
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
         validate_database();

         BOOST_TEST_MESSAGE( "--- Test failure with a new vote within lockout period" );

         op.weight = PERCENT_100;
         op.voter = "dave";
         tx.operations.clear();
         tx.signatures.clear();
         tx.operations.push_back( op );
         tx.sign( dave_private_key, db.get_chain_id() );
         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
         validate_database();
      }
   }
   FC_LOG_AND_RETHROW()
}



   //==========================//
   // === Board Operations === //
   //==========================//






   //================================//
   // === Advertising Operations === //
   //================================//






   //=============================//
   // === Transfer Operations === //
   //=============================//



BOOST_AUTO_TEST_CASE( transfer_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_validate" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_authorities )
{
   try
   {
      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );

      BOOST_TEST_MESSAGE( "Testing: transfer_authorities" );

      transfer_operation op;
      op.from = "alice";
      op.to = "bob";
      op.amount = ASSET( "2.500 TESTS" );

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "--- Test failure when no signatures" );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by a signature not in the account's authority" );
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure when duplicate signatures" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by an additional signature not in the creator's authority" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test success with witness signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( signature_stripping )
{
   try
   {
      // Alice, Bob and Sam all have 2-of-3 multisig on corp.
      // Legitimate tx signed by (Alice, Bob) goes through.
      // Sam shouldn't be able to add or remove signatures to get the transaction to process multiple times.

      ACTORS( (alice)(bob)(sam)(corp) )
      fund( "corp", 10000 );

      account_update_operation update_op;
      update_op.account = "corp";
      update_op.active = authority( 2, "alice", 1, "bob", 1, "sam", 1 );

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( update_op );

      tx.sign( corp_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      transfer_operation transfer_op;
      transfer_op.from = "corp";
      transfer_op.to = "sam";
      transfer_op.amount = ASSET( "1.000 TESTS" );

      tx.operations.push_back( transfer_op );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      signature_type alice_sig = tx.signatures.back();
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      signature_type bob_sig = tx.signatures.back();
      tx.sign( sam_private_key, db.get_chain_id() );
      signature_type sam_sig = tx.signatures.back();
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      tx.signatures.clear();
      tx.signatures.push_back( alice_sig );
      tx.signatures.push_back( bob_sig );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.signatures.push_back( alice_sig );
      tx.signatures.push_back( sam_sig );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_apply" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );

      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET(" 0.000 TESTS" ).amount.value );

      signed_transaction tx;
      transfer_operation op;

      op.from = "alice";
      op.to = "bob";
      op.amount = ASSET( "5.000 TESTS" );

      BOOST_TEST_MESSAGE( "--- Test normal transaction" );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "5.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "5.000 TESTS" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Generating a block" );
      generate_block();

      const auto& new_alice = db.get_account( "alice" );
      const auto& new_bob = db.get_account( "bob" );

      BOOST_REQUIRE( new_alice.balance.amount.value == ASSET( "5.000 TESTS" ).amount.value );
      BOOST_REQUIRE( new_bob.balance.amount.value == ASSET( "5.000 TESTS" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test emptying an account" );
      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_REQUIRE( new_alice.balance.amount.value == ASSET( "0.000 TESTS" ).amount.value );
      BOOST_REQUIRE( new_bob.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test transferring non-existent funds" );
      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_REQUIRE( new_alice.balance.amount.value == ASSET( "0.000 TESTS" ).amount.value );
      BOOST_REQUIRE( new_bob.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value );
      validate_database();

   }
   FC_LOG_AND_RETHROW()
}



   //============================//
   // === Balance Operations === //
   //============================//



BOOST_AUTO_TEST_CASE( claim_reward_balance_validate )
{
   try
   {
      claim_reward_balance_operation op;
      op.account = "alice";
      op.TMEreward = ASSET( "0.000 TESTS" );
      op.USDreward = ASSET( "0.000 USD" );
      op.reward = ASSET( "0.000000 VESTS" );


      BOOST_TEST_MESSAGE( "Testing all 0 amounts" );
      REQUIRE_THROW( op.validate(), fc::assert_exception );


      BOOST_TEST_MESSAGE( "Testing single reward claims" );
      op.TMEreward.amount = 1000;
      op.validate();

      op.TMEreward.amount = 0;
      op.USDreward.amount = 1000;
      op.validate();

      op.USDreward.amount = 0;
      op.reward.amount = 1000;
      op.validate();

      op.reward.amount = 0;


      BOOST_TEST_MESSAGE( "Testing wrong TME symbol" );
      op.TMEreward = ASSET( "1.000 WRONG" );
      REQUIRE_THROW( op.validate(), fc::assert_exception );


      BOOST_TEST_MESSAGE( "Testing wrong USD symbol" );
      op.TMEreward = ASSET( "1.000 TESTS" );
      op.USDreward = ASSET( "1.000 WRONG" );
      REQUIRE_THROW( op.validate(), fc::assert_exception );


      BOOST_TEST_MESSAGE( "Testing wrong SCORE symbol" );
      op.USDreward = ASSET( "1.000 USD" );
      op.reward = ASSET( "1.000000 WRONG" );
      REQUIRE_THROW( op.validate(), fc::assert_exception );


      BOOST_TEST_MESSAGE( "Testing a single negative amount" );
      op.TMEreward.amount = 1000;
      op.USDreward.amount = -1000;
      REQUIRE_THROW( op.validate(), fc::assert_exception );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( claim_reward_balance_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: decline_voting_rights_authorities" );

      claim_reward_balance_operation op;
      op.account = "alice";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( claim_reward_balance_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: claim_reward_balance_apply" );
      BOOST_TEST_MESSAGE( "--- Setting up test state" );

      ACTORS( (alice) )
      generate_block();

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) ) );

      db_plugin->debug_update( []( database& db )
      {
         db.modify( db.get_account( "alice" ), []( account_object& a )
         {
            a.TMErewardBalance = ASSET( "10.000 TESTS" );
            a.USDrewardBalance = ASSET( "10.000 USD" );
            a.rewardBalance = ASSET( "10.000000 VESTS" );
            a.rewardBalanceInTME = ASSET( "10.000 TESTS" );
         });
      });

      generate_block();
      validate_database();

      auto alice_TME = db.get_account( "alice" ).balance;
      auto alice_USD = db.get_account( "alice" ).USDbalance;
      auto alice_SCORE = db.get_account( "alice" ).SCORE;


      BOOST_TEST_MESSAGE( "--- Attempting to claim more TME than exists in the reward balance." );

      claim_reward_balance_operation op;
      signed_transaction tx;

      op.account = "alice";
      op.TMEreward = ASSET( "20.000 TESTS" );
      op.USDreward = ASSET( "0.000 USD" );
      op.reward = ASSET( "0.000000 VESTS" );

      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Claiming a partial reward balance" );

      op.TMEreward = ASSET( "0.000 TESTS" );
      op.reward = ASSET( "5.000000 VESTS" );
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == alice_TME + op.TMEreward );
      BOOST_REQUIRE( db.get_account( "alice" ).TMErewardBalance == ASSET( "10.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == alice_USD + op.USDreward );
      BOOST_REQUIRE( db.get_account( "alice" ).USDrewardBalance == ASSET( "10.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).SCORE == alice_SCORE + op.reward );
      BOOST_REQUIRE( db.get_account( "alice" ).rewardBalance == ASSET( "5.000000 VESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).rewardBalanceInTME == ASSET( "5.000 TESTS" ) );
      validate_database();

      alice_SCORE += op.reward;


      BOOST_TEST_MESSAGE( "--- Claiming the full reward balance" );

      op.TMEreward = ASSET( "10.000 TESTS" );
      op.USDreward = ASSET( "10.000 USD" );
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == alice_TME + op.TMEreward );
      BOOST_REQUIRE( db.get_account( "alice" ).TMErewardBalance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == alice_USD + op.USDreward );
      BOOST_REQUIRE( db.get_account( "alice" ).USDrewardBalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).SCORE == alice_SCORE + op.reward );
      BOOST_REQUIRE( db.get_account( "alice" ).rewardBalance == ASSET( "0.000000 VESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).rewardBalanceInTME == ASSET( "0.000 TESTS" ) );
            validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( stake_asset_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: stake_asset_validate" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( stake_asset_authorities )
{
   try
   {
      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );

      BOOST_TEST_MESSAGE( "Testing: stake_asset_authorities" );

      stake_asset_operation op;
      op.from = "alice";
      op.to = "bob";
      op.amount = ASSET( "2.500 TESTS" );

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "--- Test failure when no signatures" );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by a signature not in the account's authority" );
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure when duplicate signatures" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by an additional signature not in the creator's authority" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test success with from signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( stake_asset_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: stake_asset_apply" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );

      const auto& props = db.get_dynamic_global_properties();

      BOOST_REQUIRE( alice.balance == ASSET( "10.000 TESTS" ) );

      auto totalSCORE = asset( props.totalSCORE.amount, SYMBOL_SCORE );
      auto TMEfundForSCOREvalueBalance = asset( props.totalTMEfundForSCORE.amount, SYMBOL_COIN );
      auto alice_SCORE = alice.SCORE;
      auto bob_SCORE = bob.SCORE;

      stake_asset_operation op;
      op.from = "alice";
      op.to = "";
      op.amount = ASSET( "7.500 TESTS" );

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      newSCORE = asset( ( op.amount * ( totalSCORE / TMEfundForSCOREvalueBalance ) ).amount, SYMBOL_SCORE );
      totalSCORE += newSCORE;
      TMEfundForSCOREvalueBalance += op.amount;
      alice_SCORE += newSCORE;

      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "2.500 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.SCORE.amount.value == alice_SCORE.amount.value );
      BOOST_REQUIRE( props.totalTMEfundForSCORE.amount.value == TMEfundForSCOREvalueBalance.amount.value );
      BOOST_REQUIRE( props.totalSCORE.amount.value == totalSCORE.amount.value );
      validate_database();

      op.to = "bob";
      op.amount = asset( 2000, SYMBOL_COIN );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      newSCORE = asset( ( op.amount * ( totalSCORE / TMEfundForSCOREvalueBalance ) ).amount, SYMBOL_SCORE );
      totalSCORE += newSCORE;
      TMEfundForSCOREvalueBalance += op.amount;
      bob_SCORE += newSCORE;

      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "0.500 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.SCORE.amount.value == alice_SCORE.amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "0.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.SCORE.amount.value == bob_SCORE.amount.value );
      BOOST_REQUIRE( props.totalTMEfundForSCORE.amount.value == TMEfundForSCOREvalueBalance.amount.value );
      BOOST_REQUIRE( props.totalSCORE.amount.value == totalSCORE.amount.value );
      validate_database();

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "0.500 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.SCORE.amount.value == alice_SCORE.amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "0.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.SCORE.amount.value == bob_SCORE.amount.value );
      BOOST_REQUIRE( props.totalTMEfundForSCORE.amount.value == TMEfundForSCOREvalueBalance.amount.value );
      BOOST_REQUIRE( props.totalSCORE.amount.value == totalSCORE.amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( unstake_asset_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: unstake_asset_validate" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( unstake_asset_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: unstake_asset_authorities" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );
      fund_stake( "alice", 10000 );

      unstake_asset_operation op;
      op.account = "alice";
      op.SCORE = ASSET( "0.001000 VESTS" );

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with account signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( unstake_asset_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: unstake_asset_apply" );

      ACTORS( (alice) )
      generate_block();
      fund_stake( "alice", ASSET( "10.000 TESTS" ) );

      BOOST_TEST_MESSAGE( "--- Test failure withdrawing negative VESTS" );

      {
      const auto& alice = db.get_account( "alice" );

      unstake_asset_operation op;
      op.account = "alice";
      op.SCORE = asset( -1, SYMBOL_SCORE );

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Test withdraw of existing VESTS" );
      op.SCORE = asset( alice.SCORE.amount / 2, SYMBOL_SCORE );

      auto old_SCORE = alice.SCORE;

      tx.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.SCORE.amount.value == old_SCORE.amount.value );
      BOOST_REQUIRE( alice.SCOREwithdrawRateInTME.amount.value == ( old_SCORE.amount / ( COIN_UNSTAKE_INTERVALS * 2 ) ).value );
      BOOST_REQUIRE( alice.to_withdraw.value == op.SCORE.amount.value );
      BOOST_REQUIRE( alice.next_unstake_time == db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test changing SCORE TME fund withdrawal" );
      tx.operations.clear();
      tx.signatures.clear();

      op.SCORE = asset( alice.SCORE.amount / 3, SYMBOL_SCORE );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.SCORE.amount.value == old_SCORE.amount.value );
      BOOST_REQUIRE( alice.SCOREwithdrawRateInTME.amount.value == ( old_SCORE.amount / ( COIN_UNSTAKE_INTERVALS * 3 ) ).value );
      BOOST_REQUIRE( alice.to_withdraw.value == op.SCORE.amount.value );
      BOOST_REQUIRE( alice.next_unstake_time == db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test withdrawing more SCORE than available" );
      auto old_withdraw_amount = alice.to_withdraw;
      tx.operations.clear();
      tx.signatures.clear();

      op.SCORE = asset( alice.SCORE.amount * 2, SYMBOL_SCORE );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( alice.SCORE.amount.value == old_SCORE.amount.value );
      BOOST_REQUIRE( alice.SCOREwithdrawRateInTME.amount.value == ( old_SCORE.amount / ( COIN_UNSTAKE_INTERVALS * 3 ) ).value );
      BOOST_REQUIRE( alice.next_unstake_time == db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test withdrawing 0 to reset SCORE TME fund withdraw" );
      tx.operations.clear();
      tx.signatures.clear();

      op.SCORE = asset( 0, SYMBOL_SCORE );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.SCORE.amount.value == old_SCORE.amount.value );
      BOOST_REQUIRE( alice.SCOREwithdrawRateInTME.amount.value == 0 );
      BOOST_REQUIRE( alice.to_withdraw.value == 0 );
      BOOST_REQUIRE( alice.next_unstake_time == fc::time_point::maximum() );


      BOOST_TEST_MESSAGE( "--- Test cancelling a withdraw when below the account creation fee" );
      op.SCORE = alice.SCORE;
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      generate_block();
      }

      db_plugin->debug_update( [=]( database& db )
      {
         auto& wso = db.get_witness_schedule();

         db.modify( wso, [&]( witness_schedule_object& w )
         {
            w.median_props.account_creation_fee = ASSET( "10.000 TESTS" );
         });
      }, database::skip_witness_signature );

      unstake_asset_operation op;
      signed_transaction tx;
      op.account = "alice";
      op.SCORE = ASSET( "0.000000 VESTS" );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).SCOREwithdrawRateInTME == ASSET( "0.000000 VESTS" ) );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_to_savings_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_to_savings_validate" );

      transfer_to_savings_operation op;
      op.from = "alice";
      op.to = "alice";
      op.amount = ASSET( "1.000 TESTS" );


      BOOST_TEST_MESSAGE( "failure when 'from' is empty" );
      op.from = "";
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "failure when 'to' is empty" );
      op.from = "alice";
      op.to = "";
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "sucess when 'to' is not empty" );
      op.to = "bob";
      op.validate();


      BOOST_TEST_MESSAGE( "failure when amount is VESTS" );
      op.to = "alice";
      op.amount = ASSET( "1.000 VESTS" );
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "success when amount is USD" );
      op.amount = ASSET( "1.000 USD" );
      op.validate();


      BOOST_TEST_MESSAGE( "success when amount is TESTS" );
      op.amount = ASSET( "1.000 TESTS" );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_to_savings_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_to_savings_authorities" );

      transfer_to_savings_operation op;
      op.from = "alice";
      op.to = "alice";
      op.amount = ASSET( "1.000 TESTS" );

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      expected.insert( "alice" );
      BOOST_REQUIRE( auths == expected );

      auths.clear();
      expected.clear();
      op.from = "bob";
      op.get_required_active_authorities( auths );
      expected.insert( "bob" );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_to_savings_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_to_savings_apply" );

      ACTORS( (alice)(bob) );
      generate_block();

      fund( "alice", ASSET( "10.000 TESTS" ) );
      fund( "alice", ASSET( "10.000 USD" ) );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "10.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == ASSET( "10.000 USD" ) );

      transfer_to_savings_operation op;
      signed_transaction tx;

      BOOST_TEST_MESSAGE( "--- failure with insufficient funds" );
      op.from = "alice";
      op.to = "alice";
      op.amount = ASSET( "20.000 TESTS" );

      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();


      BOOST_TEST_MESSAGE( "--- failure when transferring to non-existent account" );
      op.to = "sam";
      op.amount = ASSET( "1.000 TESTS" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();


      BOOST_TEST_MESSAGE( "--- success transferring TME to self" );
      op.to = "alice";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "9.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).TMEsavingsBalance == ASSET( "1.000 TESTS" ) );
      validate_database();


      BOOST_TEST_MESSAGE( "--- success transferring USD to self" );
      op.amount = ASSET( "1.000 USD" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == ASSET( "9.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDsavingsBalance == ASSET( "1.000 USD" ) );
      validate_database();


      BOOST_TEST_MESSAGE( "--- success transferring TME to other" );
      op.to = "bob";
      op.amount = ASSET( "1.000 TESTS" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "8.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).TMEsavingsBalance == ASSET( "1.000 TESTS" ) );
      validate_database();


      BOOST_TEST_MESSAGE( "--- success transferring USD to other" );
      op.amount = ASSET( "1.000 USD" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == ASSET( "8.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).USDsavingsBalance == ASSET( "1.000 USD" ) );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_from_savings_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_from_savings_validate" );

      transfer_from_savings_operation op;
      op.from = "alice";
      op.request_id = 0;
      op.to = "alice";
      op.amount = ASSET( "1.000 TESTS" );


      BOOST_TEST_MESSAGE( "failure when 'from' is empty" );
      op.from = "";
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "failure when 'to' is empty" );
      op.from = "alice";
      op.to = "";
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "sucess when 'to' is not empty" );
      op.to = "bob";
      op.validate();


      BOOST_TEST_MESSAGE( "failure when amount is VESTS" );
      op.to = "alice";
      op.amount = ASSET( "1.000 VESTS" );
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "success when amount is USD" );
      op.amount = ASSET( "1.000 USD" );
      op.validate();


      BOOST_TEST_MESSAGE( "success when amount is TME" );
      op.amount = ASSET( "1.000 TESTS" );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_from_savings_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_from_savings_authorities" );

      transfer_from_savings_operation op;
      op.from = "alice";
      op.to = "alice";
      op.amount = ASSET( "1.000 TESTS" );

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      expected.insert( "alice" );
      BOOST_REQUIRE( auths == expected );

      auths.clear();
      expected.clear();
      op.from = "bob";
      op.get_required_active_authorities( auths );
      expected.insert( "bob" );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( transfer_from_savings_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_from_savings_apply" );

      ACTORS( (alice)(bob) );
      generate_block();

      fund( "alice", ASSET( "10.000 TESTS" ) );
      fund( "alice", ASSET( "10.000 USD" ) );

      transfer_to_savings_operation save;
      save.from = "alice";
      save.to = "alice";
      save.amount = ASSET( "10.000 TESTS" );

      signed_transaction tx;
      tx.operations.push_back( save );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      save.amount = ASSET( "10.000 USD" );
      tx.clear();
      tx.operations.push_back( save );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );


      BOOST_TEST_MESSAGE( "--- failure when account has insufficient funds" );
      transfer_from_savings_operation op;
      op.from = "alice";
      op.to = "bob";
      op.amount = ASSET( "20.000 TESTS" );
      op.request_id = 0;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure withdrawing to non-existant account" );
      op.to = "sam";
      op.amount = ASSET( "1.000 TESTS" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success withdrawing TME to self" );
      op.to = "alice";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).TMEsavingsBalance == ASSET( "9.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 1 );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).from == op.from );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).to == op.to );
      BOOST_REQUIRE( to_string( db.get_savings_withdraw( "alice", op.request_id ).memo ) == op.memo );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).request_id == op.request_id );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).amount == op.amount );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).complete == db.head_block_time() + SAVINGS_WITHDRAW_TIME );
      validate_database();


      BOOST_TEST_MESSAGE( "--- success withdrawing USD to self" );
      op.amount = ASSET( "1.000 USD" );
      op.request_id = 1;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDsavingsBalance == ASSET( "9.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 2 );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).from == op.from );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).to == op.to );
      BOOST_REQUIRE( to_string( db.get_savings_withdraw( "alice", op.request_id ).memo ) == op.memo );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).request_id == op.request_id );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).amount == op.amount );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).complete == db.head_block_time() + SAVINGS_WITHDRAW_TIME );
      validate_database();


      BOOST_TEST_MESSAGE( "--- failure withdrawing with repeat request id" );
      op.amount = ASSET( "2.000 TESTS" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success withdrawing TME to other" );
      op.to = "bob";
      op.amount = ASSET( "1.000 TESTS" );
      op.request_id = 3;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).TMEsavingsBalance == ASSET( "8.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 3 );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).from == op.from );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).to == op.to );
      BOOST_REQUIRE( to_string( db.get_savings_withdraw( "alice", op.request_id ).memo ) == op.memo );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).request_id == op.request_id );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).amount == op.amount );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).complete == db.head_block_time() + SAVINGS_WITHDRAW_TIME );
      validate_database();


      BOOST_TEST_MESSAGE( "--- success withdrawing USD to other" );
      op.amount = ASSET( "1.000 USD" );
      op.request_id = 4;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDsavingsBalance == ASSET( "8.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 4 );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).from == op.from );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).to == op.to );
      BOOST_REQUIRE( to_string( db.get_savings_withdraw( "alice", op.request_id ).memo ) == op.memo );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).request_id == op.request_id );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).amount == op.amount );
      BOOST_REQUIRE( db.get_savings_withdraw( "alice", op.request_id ).complete == db.head_block_time() + SAVINGS_WITHDRAW_TIME );
      validate_database();


      BOOST_TEST_MESSAGE( "--- withdraw on timeout" );
      generate_blocks( db.head_block_time() + SAVINGS_WITHDRAW_TIME - fc::seconds( BLOCK_INTERVAL ), true );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).USDbalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 4 );
      validate_database();

      generate_block();

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance == ASSET( "1.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).balance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).USDbalance == ASSET( "1.000 USD" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 0 );
      validate_database();


      BOOST_TEST_MESSAGE( "--- savings withdraw request limit" );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      op.to = "alice";
      op.amount = ASSET( "0.001 TESTS" );

      for( int i = 0; i < SAVINGS_WITHDRAW_REQUEST_LIMIT; i++ )
      {
         op.request_id = i;
         tx.clear();
         tx.operations.push_back( op );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
         BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == i + 1 );
      }

      op.request_id = SAVINGS_WITHDRAW_REQUEST_LIMIT;
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == SAVINGS_WITHDRAW_REQUEST_LIMIT );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( cancel_transfer_from_savings_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: cancel_transfer_from_savings_validate" );

      cancel_transfer_from_savings_operation op;
      op.from = "alice";
      op.request_id = 0;


      BOOST_TEST_MESSAGE( "--- failure when 'from' is empty" );
      op.from = "";
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "--- sucess when 'from' is not empty" );
      op.from = "alice";
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( cancel_transfer_from_savings_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: cancel_transfer_from_savings_authorities" );

      cancel_transfer_from_savings_operation op;
      op.from = "alice";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      expected.insert( "alice" );
      BOOST_REQUIRE( auths == expected );

      auths.clear();
      expected.clear();
      op.from = "bob";
      op.get_required_active_authorities( auths );
      expected.insert( "bob" );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( cancel_transfer_from_savings_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: cancel_transfer_from_savings_apply" );

      ACTORS( (alice)(bob) )
      generate_block();

      fund( "alice", ASSET( "10.000 TESTS" ) );

      transfer_to_savings_operation save;
      save.from = "alice";
      save.to = "alice";
      save.amount = ASSET( "10.000 TESTS" );

      transfer_from_savings_operation withdraw;
      withdraw.from = "alice";
      withdraw.to = "bob";
      withdraw.request_id = 1;
      withdraw.amount = ASSET( "3.000 TESTS" );

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( save );
      tx.operations.push_back( withdraw );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 1 );
      BOOST_REQUIRE( db.get_account( "bob" ).savings_withdraw_requests == 0 );


      BOOST_TEST_MESSAGE( "--- Failure when there is no pending request" );
      cancel_transfer_from_savings_operation op;
      op.from = "alice";
      op.request_id = 0;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 1 );
      BOOST_REQUIRE( db.get_account( "bob" ).savings_withdraw_requests == 0 );


      BOOST_TEST_MESSAGE( "--- Success" );
      op.request_id = 1;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).TMEsavingsBalance == ASSET( "10.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).savings_withdraw_requests == 0 );
      BOOST_REQUIRE( db.get_account( "bob" ).balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).TMEsavingsBalance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).savings_withdraw_requests == 0 );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( delegate_asset_validate )
{
   try
   {
      delegate_asset_operation op;

      op.delegator = "alice";
      op.delegatee = "bob";
      op.SCORE = asset( -1, SYMBOL_SCORE );
      REQUIRE_THROW( op.validate(), fc::assert_exception );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( delegate_asset_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: delegate_asset_authorities" );
      signed_transaction tx;
      ACTORS( (alice)(bob) )
      fund_stake( "alice", ASSET( "10000.000000 VESTS" ) );

      delegate_asset_operation op;
      op.SCORE = ASSET( "300.000000 VESTS");
      op.delegator = "alice";
      op.delegatee = "bob";

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "--- Test failure when no signatures" );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with witness signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Test failure when duplicate signatures" );
      tx.operations.clear();
      tx.signatures.clear();
      op.delegatee = "sam";
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by an additional signature not in the creator's authority" );
      tx.signatures.clear();
      tx.sign( init_account_priv_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by a signature not in the creator's authority" );
      tx.signatures.clear();
      tx.sign( init_account_priv_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( delegate_asset_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: delegate_asset_apply" );
      signed_transaction tx;
      ACTORS( (alice)(bob) )
      generate_block();

      fund_stake( "alice", ASSET( "1000.000 TESTS" ) );

      generate_block();

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_witness_schedule(), [&]( witness_schedule_object& w )
         {
            w.median_props.account_creation_fee = ASSET( "1.000 TESTS" );
         });
      });

      generate_block();

      delegate_asset_operation op;
      op.SCORE = ASSET( "10000000.000000 VESTS");
      op.delegator = "alice";
      op.delegatee = "bob";

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      generate_blocks( 1 );
      const account_object& alice_acc = db.get_account( "alice" );
      const account_object& bob_acc = db.get_account( "bob" );

      BOOST_REQUIRE( alice_acc.SCOREDelegated == ASSET( "10000000.000000 VESTS"));
      BOOST_REQUIRE( bob_acc.SCOREreceived == ASSET( "10000000.000000 VESTS"));

      BOOST_TEST_MESSAGE( "--- Test that the delegation object is correct. " );
      auto delegation = db.find< asset_delegation_object, by_delegation >( boost::make_tuple( op.delegator, op.delegatee ) );

      BOOST_REQUIRE( delegation != nullptr );
      BOOST_REQUIRE( delegation->delegator == op.delegator);
      BOOST_REQUIRE( delegation->SCORE  == ASSET( "10000000.000000 VESTS"));

      validate_database();
      tx.clear();
      op.SCORE = ASSET( "20000000.000000 VESTS");
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      generate_blocks(1);

      BOOST_REQUIRE( delegation != nullptr );
      BOOST_REQUIRE( delegation->delegator == op.delegator);
      BOOST_REQUIRE( delegation->SCORE == ASSET( "20000000.000000 VESTS"));
      BOOST_REQUIRE( alice_acc.SCOREDelegated == ASSET( "20000000.000000 VESTS"));
      BOOST_REQUIRE( bob_acc.SCOREreceived == ASSET( "20000000.000000 VESTS"));

      BOOST_TEST_MESSAGE( "--- Test that effective SCORE is accurate and being applied." );
      tx.operations.clear();
      tx.signatures.clear();

      comment_operation comment_op;
      comment_op.author = "alice";
      comment_op.permlink = "foo";
      comment_op.parent_permlink = "test";
      comment_op.title = "bar";
      comment_op.body = "foo bar";
      tx.operations.push_back( comment_op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      tx.signatures.clear();
      tx.operations.clear();
      vote_operation vote_op;
      vote_op.voter = "bob";
      vote_op.author = "alice";
      vote_op.permlink = "foo";
      vote_op.weight = PERCENT_100;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote_op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      auto old_voting_power = bob_acc.voting_power;

      db.push_transaction( tx, 0 );
      generate_blocks(1);

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      auto& alice_comment = db.get_comment( "alice", string( "foo" ) );
      auto itr = vote_idx.find( std::make_tuple( alice_comment.id, bob_acc.id ) );
      BOOST_REQUIRE( alice_comment.net_reward.value == bob_acc.effective_SCORE().amount.value * ( old_voting_power - bob_acc.voting_power ) / PERCENT_100 );
      BOOST_REQUIRE( itr->reward == bob_acc.effective_SCORE().amount.value * ( old_voting_power - bob_acc.voting_power ) / PERCENT_100 );


      generate_block();
      ACTORS( (sam)(dave) )
      generate_block();

      fund_stake( "sam", ASSET( "1000.000 TESTS" ) );

      generate_block();

      auto samSCORE = db.get_account( "sam" ).SCORE;

      BOOST_TEST_MESSAGE( "--- Test failure when delegating 0 VESTS" );
      tx.clear();
      op.delegator = "sam";
      op.delegatee = "dave";
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Testing failure delegating more SCORE than account has." );
      tx.clear();
      op.SCORE = asset( samSCORE.amount + 1, SYMBOL_SCORE );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Test failure delegating SCORE that are part of a power down" );
      tx.clear();
      samSCORE = asset( samSCORE.amount / 2, SYMBOL_SCORE );
      unstake_asset_operation withdraw;
      withdraw.account = "sam";
      withdraw.SCORE = samSCORE;
      tx.operations.push_back( withdraw );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.clear();
      op.SCORE = asset( samSCORE.amount + 2, SYMBOL_SCORE );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );

      tx.clear();
      withdraw.SCORE = ASSET( "0.000000 VESTS" );
      tx.operations.push_back( withdraw );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );


      BOOST_TEST_MESSAGE( "--- Test failure powering down SCORE that are delegated" );
      samSCORE.amount += 1000;
      op.SCORE = samSCORE;
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.clear();
      withdraw.SCORE = asset( samSCORE.amount, SYMBOL_SCORE );
      tx.operations.push_back( withdraw );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::assert_exception );


      BOOST_TEST_MESSAGE( "--- Remove a delegation and ensure it is returned after 1 week" );
      tx.clear();
      op.SCORE = ASSET( "0.000000 VESTS" );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto exp_obj = db.get_index< asset_delegation_expiration_index, by_id >().begin();
      auto end = db.get_index< asset_delegation_expiration_index, by_id >().end();

      BOOST_REQUIRE( exp_obj != end );
      BOOST_REQUIRE( exp_obj->delegator == "sam" );
      BOOST_REQUIRE( exp_obj->SCORE == samSCORE );
      BOOST_REQUIRE( exp_obj->expiration == db.head_block_time() + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( db.get_account( "sam" ).SCOREDelegated == samSCORE );
      BOOST_REQUIRE( db.get_account( "dave" ).SCOREreceived == ASSET( "0.000000 VESTS" ) );
      delegation = db.find< asset_delegation_object, by_delegation >( boost::make_tuple( op.delegator, op.delegatee ) );
      BOOST_REQUIRE( delegation == nullptr );

      generate_blocks( exp_obj->expiration + BLOCK_INTERVAL );

      exp_obj = db.get_index< asset_delegation_expiration_index, by_id >().begin();
      end = db.get_index< asset_delegation_expiration_index, by_id >().end();

      BOOST_REQUIRE( exp_obj == end );
      BOOST_REQUIRE( db.get_account( "sam" ).SCOREDelegated == ASSET( "0.000000 VESTS" ) );
   }
   FC_LOG_AND_RETHROW()
}


   //===========================//
   // === Escrow Operations === //
   //===========================//



BOOST_AUTO_TEST_CASE( escrow_transfer_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_transfer_validate" );

      escrow_transfer_operation op;
      op.from = "alice";
      op.to = "bob";
      op.amount = ASSET( "1.000 TESTS" );
      op.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17";
      op.agent = "sam";
      op.fee = ASSET( "0.100 TESTS" );
      op.json = "";
      op.ratification_deadline = db.head_block_time() + 100;
      op.escrow_expiration = db.head_block_time() + 200;

      BOOST_TEST_MESSAGE( "--- failure when amount < 0" );
      op.amount.amount = -100;
      REQUIRE_THROW( op.validate(), fc::exception );

      BOOST_TEST_MESSAGE( "--- failure when fee < 0" );
      op.amount.amount = 1000;
      op.fee.amount = -100;
      REQUIRE_THROW( op.validate(), fc::exception );

      BOOST_TEST_MESSAGE( "--- failure when ratification deadline == escrow expiration" );
      op.fee.amount = 100;
      op.ratification_deadline = op.escrow_expiration;
      REQUIRE_THROW( op.validate(), fc::exception );

      BOOST_TEST_MESSAGE( "--- failure when ratification deadline > escrow expiration" );
      op.ratification_deadline = op.escrow_expiration + 100;
      REQUIRE_THROW( op.validate(), fc::exception );

      BOOST_TEST_MESSAGE( "--- success" );
      op.ratification_deadline = op.escrow_expiration - 100;
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_transfer_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_transfer_authorities" );

      escrow_transfer_operation op;
      op.from = "alice";
      op.to = "bob";
      op.USDamount = ASSET( "1.000 USD" );
      op.TMEamount = ASSET( "1.000 TESTS" );
      op.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17"
      op.agent = "sam";
      op.fee = ASSET( "0.100 TESTS" );
      op.json = "";
      op.ratification_deadline = db.head_block_time() + 100;
      op.escrow_expiration = db.head_block_time() + 200;

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      expected.insert( "alice" );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_transfer_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_transfer_apply" );

      ACTORS( (alice)(bob)(sam) )

      fund( "alice", 10000 );

      escrow_transfer_operation op;
      op.from = "alice";
      op.to = "bob";
      op.USDamount = ASSET( "1.000 USD" );
      op.TMEamount = ASSET( "1.000 TESTS" );
      op.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17"
      op.agent = "sam";
      op.fee = ASSET( "0.100 TESTS" );
      op.json = "";
      op.ratification_deadline = db.head_block_time() + 100;
      op.escrow_expiration = db.head_block_time() + 200;

      BOOST_TEST_MESSAGE( "--- failure when from cannot cover USD amount" );
      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- falure when from cannot cover amount + fee" );
      op.USDamount.amount = 0;
      op.TMEamount.amount = 10000;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- failure when ratification deadline is in the past" );
      op.TMEamount.amount = 1000;
      op.ratification_deadline = db.head_block_time() - 200;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- failure when expiration is in the past" );
      op.escrow_expiration = db.head_block_time() - 100;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- success" );
      op.ratification_deadline = db.head_block_time() + 100;
      op.escrow_expiration = db.head_block_time() + 200;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      auto alice_TMEbalance = alice.balance - op.TMEamount - op.fee;
      auto alice_USDbalance = alice.USDbalance - op.USDamount;
      auto bob_TMEbalance = bob.balance;
      auto bob_USDbalance = bob.USDbalance;
      auto sam_TMEbalance = sam.balance;
      auto sam_USDbalance = sam.USDbalance;

      db.push_transaction( tx, 0 );

      const auto& escrow = db.get_escrow( op.from, op.escrow_id );

      BOOST_REQUIRE( escrow.escrow_id == op.escrow_id );
      BOOST_REQUIRE( escrow.from == op.from );
      BOOST_REQUIRE( escrow.to == op.to );
      BOOST_REQUIRE( escrow.agent == op.agent );
      BOOST_REQUIRE( escrow.ratification_deadline == op.ratification_deadline );
      BOOST_REQUIRE( escrow.escrow_expiration == op.escrow_expiration );
      BOOST_REQUIRE( escrow.USDbalance == op.USDamount );
      BOOST_REQUIRE( escrow.TMEbalance == op.TMEamount );
      BOOST_REQUIRE( escrow.pending_fee == op.fee );
      BOOST_REQUIRE( !escrow.to_approved );
      BOOST_REQUIRE( !escrow.agent_approved );
      BOOST_REQUIRE( !escrow.disputed );
      BOOST_REQUIRE( alice.balance == alice_TMEbalance );
      BOOST_REQUIRE( alice.USDbalance == alice_USDbalance );
      BOOST_REQUIRE( bob.balance == bob_TMEbalance );
      BOOST_REQUIRE( bob.USDbalance == bob_USDbalance );
      BOOST_REQUIRE( sam.balance == sam_TMEbalance );
      BOOST_REQUIRE( sam.USDbalance == sam_USDbalance );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_approve_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_approve_validate" );

      escrow_approve_operation op;

      op.from = "alice";
      op.to = "bob";
      op.agent = "sam";
      op.who = "bob";
      op.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17"
      op.approve = true;

      BOOST_TEST_MESSAGE( "--- failure when who is not to or agent" );
      op.who = "dave";
      REQUIRE_THROW( op.validate(), fc::exception );

      BOOST_TEST_MESSAGE( "--- success when who is to" );
      op.who = op.to;
      op.validate();

      BOOST_TEST_MESSAGE( "--- success when who is agent" );
      op.who = op.agent;
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_approve_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_approve_authorities" );

      escrow_approve_operation op;

      op.from = "alice";
      op.to = "bob";
      op.agent = "sam";
      op.who = "bob";
      op.escrow_id = "6b3b3da0-660a-41a1-b6a2-221a71c0cc17"
      op.approve = true;

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      expected.insert( "bob" );
      BOOST_REQUIRE( auths == expected );

      expected.clear();
      auths.clear();

      op.who = "sam";
      op.get_required_active_authorities( auths );
      expected.insert( "sam" );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_approve_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_approve_apply" );
      ACTORS( (alice)(bob)(sam)(dave) )
      fund( "alice", 10000 );

      escrow_transfer_operation et_op;
      et_op.from = "alice";
      et_op.to = "bob";
      et_op.agent = "sam";
      et_op.TMEamount = ASSET( "1.000 TESTS" );
      et_op.fee = ASSET( "0.100 TESTS" );
      et_op.json = "";
      et_op.ratification_deadline = db.head_block_time() + 100;
      et_op.escrow_expiration = db.head_block_time() + 200;

      signed_transaction tx;
      tx.operations.push_back( et_op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      tx.operations.clear();
      tx.signatures.clear();


      BOOST_TEST_MESSAGE( "---failure when to does not match escrow" );
      escrow_approve_operation op;
      op.from = "alice";
      op.to = "dave";
      op.agent = "sam";
      op.who = "dave";
      op.approve = true;

      tx.operations.push_back( op );
      tx.sign( dave_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when agent does not match escrow" );
      op.to = "bob";
      op.agent = "dave";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( dave_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success approving to" );
      op.agent = "sam";
      op.who = "bob";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto& escrow = db.get_escrow( op.from, op.escrow_id );
      BOOST_REQUIRE( escrow.to == "bob" );
      BOOST_REQUIRE( escrow.agent == "sam" );
      BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
      BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
      BOOST_REQUIRE( escrow.USDbalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( escrow.TMEbalance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.100 TESTS" ) );
      BOOST_REQUIRE( escrow.to_approved );
      BOOST_REQUIRE( !escrow.agent_approved );
      BOOST_REQUIRE( !escrow.disputed );


      BOOST_TEST_MESSAGE( "--- failure on repeat approval" );
      tx.signatures.clear();

      tx.set_expiration( db.head_block_time() + BLOCK_INTERVAL );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( escrow.to == "bob" );
      BOOST_REQUIRE( escrow.agent == "sam" );
      BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
      BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
      BOOST_REQUIRE( escrow.USDbalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( escrow.TMEbalance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.100 TESTS" ) );
      BOOST_REQUIRE( escrow.to_approved );
      BOOST_REQUIRE( !escrow.agent_approved );
      BOOST_REQUIRE( !escrow.disputed );


      BOOST_TEST_MESSAGE( "--- failure trying to repeal after approval" );
      tx.signatures.clear();
      tx.operations.clear();

      op.approve = false;

      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( escrow.to == "bob" );
      BOOST_REQUIRE( escrow.agent == "sam" );
      BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
      BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
      BOOST_REQUIRE( escrow.USDbalance == ASSET( "0.000 USD" ) );
      BOOST_REQUIRE( escrow.TMEbalance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.100 TESTS" ) );
      BOOST_REQUIRE( escrow.to_approved );
      BOOST_REQUIRE( !escrow.agent_approved );
      BOOST_REQUIRE( !escrow.disputed );


      BOOST_TEST_MESSAGE( "--- success refunding from because of repeal" );
      tx.signatures.clear();
      tx.operations.clear();

      op.who = op.agent;

      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.get_escrow( op.from, op.escrow_id ), fc::exception );
      BOOST_REQUIRE( alice.balance == ASSET( "10.000 TESTS" ) );
      validate_database();


      BOOST_TEST_MESSAGE( "--- test automatic refund when escrow is not ratified before deadline" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( et_op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( et_op.ratification_deadline + BLOCK_INTERVAL, true );

      REQUIRE_THROW( db.get_escrow( op.from, op.escrow_id ), fc::exception );
      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "10.000 TESTS" ) );
      validate_database();


      BOOST_TEST_MESSAGE( "--- test ratification expiration when escrow is only approved by to" );
      tx.operations.clear();
      tx.signatures.clear();
      et_op.ratification_deadline = db.head_block_time() + 100;
      et_op.escrow_expiration = db.head_block_time() + 200;
      tx.operations.push_back( et_op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      op.who = op.to;
      op.approve = true;
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( et_op.ratification_deadline + BLOCK_INTERVAL, true );

      REQUIRE_THROW( db.get_escrow( op.from, op.escrow_id ), fc::exception );
      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "10.000 TESTS" ) );
      validate_database();


      BOOST_TEST_MESSAGE( "--- test ratification expiration when escrow is only approved by agent" );
      tx.operations.clear();
      tx.signatures.clear();
      et_op.ratification_deadline = db.head_block_time() + 100;
      et_op.escrow_expiration = db.head_block_time() + 200;
      tx.operations.push_back( et_op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      op.who = op.agent;
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( et_op.ratification_deadline + BLOCK_INTERVAL, true );

      REQUIRE_THROW( db.get_escrow( op.from, op.escrow_id ), fc::exception );
      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "10.000 TESTS" ) );
      validate_database();


      BOOST_TEST_MESSAGE( "--- success approving escrow" );
      tx.operations.clear();
      tx.signatures.clear();
      et_op.ratification_deadline = db.head_block_time() + 100;
      et_op.escrow_expiration = db.head_block_time() + 200;
      tx.operations.push_back( et_op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      op.who = op.to;
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      op.who = op.agent;
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      {
         const auto& escrow = db.get_escrow( op.from, op.escrow_id );
         BOOST_REQUIRE( escrow.to == "bob" );
         BOOST_REQUIRE( escrow.agent == "sam" );
         BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
         BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
         BOOST_REQUIRE( escrow.USDbalance == ASSET( "0.000 USD" ) );
         BOOST_REQUIRE( escrow.TMEbalance == ASSET( "1.000 TESTS" ) );
         BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.000 TESTS" ) );
         BOOST_REQUIRE( escrow.to_approved );
         BOOST_REQUIRE( escrow.agent_approved );
         BOOST_REQUIRE( !escrow.disputed );
      }

      BOOST_REQUIRE( db.get_account( "sam" ).balance == et_op.fee );
      validate_database();


      BOOST_TEST_MESSAGE( "--- ratification expiration does not remove an approved escrow" );

      generate_blocks( et_op.ratification_deadline + BLOCK_INTERVAL, true );
      {
         const auto& escrow = db.get_escrow( op.from, op.escrow_id );
         BOOST_REQUIRE( escrow.to == "bob" );
         BOOST_REQUIRE( escrow.agent == "sam" );
         BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
         BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
         BOOST_REQUIRE( escrow.USDbalance == ASSET( "0.000 USD" ) );
         BOOST_REQUIRE( escrow.TMEbalance == ASSET( "1.000 TESTS" ) );
         BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.000 TESTS" ) );
         BOOST_REQUIRE( escrow.to_approved );
         BOOST_REQUIRE( escrow.agent_approved );
         BOOST_REQUIRE( !escrow.disputed );
      }

      BOOST_REQUIRE( db.get_account( "sam" ).balance == et_op.fee );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_dispute_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_dispute_validate" );
      escrow_dispute_operation op;
      op.from = "alice";
      op.to = "bob";
      op.agent = "alice";
      op.who = "alice";

      BOOST_TEST_MESSAGE( "failure when who is not from or to" );
      op.who = "sam";
      REQUIRE_THROW( op.validate(), fc::exception );

      BOOST_TEST_MESSAGE( "success" );
      op.who = "alice";
      op.validate();

      op.who = "bob";
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_dispute_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_dispute_authorities" );
      escrow_dispute_operation op;
      op.from = "alice";
      op.to = "bob";
      op.who = "alice";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      expected.insert( "alice" );
      BOOST_REQUIRE( auths == expected );

      auths.clear();
      expected.clear();
      op.who = "bob";
      op.get_required_active_authorities( auths );
      expected.insert( "bob" );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_dispute_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_dispute_apply" );

      ACTORS( (alice)(bob)(sam)(dave) )
      fund( "alice", 10000 );

      escrow_transfer_operation et_op;
      et_op.from = "alice";
      et_op.to = "bob";
      et_op.agent = "sam";
      et_op.TMEamount = ASSET( "1.000 TESTS" );
      et_op.fee = ASSET( "0.100 TESTS" );
      et_op.ratification_deadline = db.head_block_time() + BLOCK_INTERVAL;
      et_op.escrow_expiration = db.head_block_time() + 2 * BLOCK_INTERVAL;

      escrow_approve_operation ea_b_op;
      ea_b_op.from = "alice";
      ea_b_op.to = "bob";
      ea_b_op.agent = "sam";
      ea_b_op.who = "bob";
      ea_b_op.approve = true;

      signed_transaction tx;
      tx.operations.push_back( et_op );
      tx.operations.push_back( ea_b_op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );


      BOOST_TEST_MESSAGE( "--- failure when escrow has not been approved" );
      escrow_dispute_operation op;
      op.from = "alice";
      op.to = "bob";
      op.agent = "sam";
      op.who = "bob";

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      const auto& escrow = db.get_escrow( et_op.from, et_op.escrow_id );
      BOOST_REQUIRE( escrow.to == "bob" );
      BOOST_REQUIRE( escrow.agent == "sam" );
      BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
      BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
      BOOST_REQUIRE( escrow.USDbalance == et_op.USDamount );
      BOOST_REQUIRE( escrow.TMEbalance == et_op.TMEamount );
      BOOST_REQUIRE( escrow.pending_fee == et_op.fee );
      BOOST_REQUIRE( escrow.to_approved );
      BOOST_REQUIRE( !escrow.agent_approved );
      BOOST_REQUIRE( !escrow.disputed );


      BOOST_TEST_MESSAGE( "--- failure when to does not match escrow" );
      escrow_approve_operation ea_s_op;
      ea_s_op.from = "alice";
      ea_s_op.to = "bob";
      ea_s_op.agent = "sam";
      ea_s_op.who = "sam";
      ea_s_op.approve = true;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( ea_s_op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      op.to = "dave";
      op.who = "alice";
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( escrow.to == "bob" );
      BOOST_REQUIRE( escrow.agent == "sam" );
      BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
      BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
      BOOST_REQUIRE( escrow.USDbalance == et_op.USDamount );
      BOOST_REQUIRE( escrow.TMEbalance == et_op.TMEamount );
      BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( escrow.to_approved );
      BOOST_REQUIRE( escrow.agent_approved );
      BOOST_REQUIRE( !escrow.disputed );


      BOOST_TEST_MESSAGE( "--- failure when agent does not match escrow" );
      op.to = "bob";
      op.who = "alice";
      op.agent = "dave";
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( escrow.to == "bob" );
      BOOST_REQUIRE( escrow.agent == "sam" );
      BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
      BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
      BOOST_REQUIRE( escrow.USDbalance == et_op.USDamount );
      BOOST_REQUIRE( escrow.TMEbalance == et_op.TMEamount );
      BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( escrow.to_approved );
      BOOST_REQUIRE( escrow.agent_approved );
      BOOST_REQUIRE( !escrow.disputed );


      BOOST_TEST_MESSAGE( "--- failure when escrow is expired" );
      generate_blocks( 2 );

      tx.operations.clear();
      tx.signatures.clear();
      op.agent = "sam";
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      {
         const auto& escrow = db.get_escrow( et_op.from, et_op.escrow_id );
         BOOST_REQUIRE( escrow.to == "bob" );
         BOOST_REQUIRE( escrow.agent == "sam" );
         BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
         BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
         BOOST_REQUIRE( escrow.USDbalance == et_op.USDamount );
         BOOST_REQUIRE( escrow.TMEbalance == et_op.TMEamount );
         BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.000 TESTS" ) );
         BOOST_REQUIRE( escrow.to_approved );
         BOOST_REQUIRE( escrow.agent_approved );
         BOOST_REQUIRE( !escrow.disputed );
      }


      BOOST_TEST_MESSAGE( "--- success disputing escrow" );
      et_op.escrow_id = 1;
      et_op.ratification_deadline = db.head_block_time() + BLOCK_INTERVAL;
      et_op.escrow_expiration = db.head_block_time() + 2 * BLOCK_INTERVAL;
      ea_b_op.escrow_id = et_op.escrow_id;
      ea_s_op.escrow_id = et_op.escrow_id;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( et_op );
      tx.operations.push_back( ea_b_op );
      tx.operations.push_back( ea_s_op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      op.escrow_id = et_op.escrow_id;
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      {
         const auto& escrow = db.get_escrow( et_op.from, et_op.escrow_id );
         BOOST_REQUIRE( escrow.to == "bob" );
         BOOST_REQUIRE( escrow.agent == "sam" );
         BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
         BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
         BOOST_REQUIRE( escrow.USDbalance == et_op.USDamount );
         BOOST_REQUIRE( escrow.TMEbalance == et_op.TMEamount );
         BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.000 TESTS" ) );
         BOOST_REQUIRE( escrow.to_approved );
         BOOST_REQUIRE( escrow.agent_approved );
         BOOST_REQUIRE( escrow.disputed );
      }


      BOOST_TEST_MESSAGE( "--- failure when escrow is already under dispute" );
      tx.operations.clear();
      tx.signatures.clear();
      op.who = "bob";
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      {
         const auto& escrow = db.get_escrow( et_op.from, et_op.escrow_id );
         BOOST_REQUIRE( escrow.to == "bob" );
         BOOST_REQUIRE( escrow.agent == "sam" );
         BOOST_REQUIRE( escrow.ratification_deadline == et_op.ratification_deadline );
         BOOST_REQUIRE( escrow.escrow_expiration == et_op.escrow_expiration );
         BOOST_REQUIRE( escrow.USDbalance == et_op.USDamount );
         BOOST_REQUIRE( escrow.TMEbalance == et_op.TMEamount );
         BOOST_REQUIRE( escrow.pending_fee == ASSET( "0.000 TESTS" ) );
         BOOST_REQUIRE( escrow.to_approved );
         BOOST_REQUIRE( escrow.agent_approved );
         BOOST_REQUIRE( escrow.disputed );
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_release_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow release validate" );
      escrow_release_operation op;
      op.from = "alice";
      op.to = "bob";
      op.who = "alice";
      op.agent = "sam";
      op.receiver = "bob";


      BOOST_TEST_MESSAGE( "--- failure when TME < 0" );
      op.TMEamount.amount = -1;
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when USD < 0" );
      op.TMEamount.amount = 0;
      op.USDamount.amount = -1;
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when TME == 0 and USD == 0" );
      op.USDamount.amount = 0;
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when USD is not USD symbol" );
      op.USDamount = ASSET( "1.000 TESTS" );
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when TME is not TME symbol" );
      op.USDamount.symbol = SYMBOL_USD;
      op.TMEamount = ASSET( "1.000 USD" );
      REQUIRE_THROW( op.validate(), fc::exception );


      BOOST_TEST_MESSAGE( "--- success" );
      op.TMEamount.symbol = SYMBOL_COIN;
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_release_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_release_authorities" );
      escrow_release_operation op;
      op.from = "alice";
      op.to = "bob";
      op.who = "alice";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.who = "bob";
      auths.clear();
      expected.clear();
      expected.insert( "bob" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.who = "sam";
      auths.clear();
      expected.clear();
      expected.insert( "sam" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( escrow_release_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: escrow_release_apply" );

      ACTORS( (alice)(bob)(sam)(dave) )
      fund( "alice", 10000 );

      escrow_transfer_operation et_op;
      et_op.from = "alice";
      et_op.to = "bob";
      et_op.agent = "sam";
      et_op.TMEamount = ASSET( "1.000 TESTS" );
      et_op.fee = ASSET( "0.100 TESTS" );
      et_op.ratification_deadline = db.head_block_time() + BLOCK_INTERVAL;
      et_op.escrow_expiration = db.head_block_time() + 2 * BLOCK_INTERVAL;

      signed_transaction tx;
      tx.operations.push_back( et_op );

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );


      BOOST_TEST_MESSAGE( "--- failure releasing funds prior to approval" );
      escrow_release_operation op;
      op.from = et_op.from;
      op.to = et_op.to;
      op.agent = et_op.agent;
      op.who = et_op.from;
      op.receiver = et_op.to;
      op.TMEamount = ASSET( "0.100 TESTS" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      escrow_approve_operation ea_b_op;
      ea_b_op.from = "alice";
      ea_b_op.to = "bob";
      ea_b_op.agent = "sam";
      ea_b_op.who = "bob";

      escrow_approve_operation ea_s_op;
      ea_s_op.from = "alice";
      ea_s_op.to = "bob";
      ea_s_op.agent = "sam";
      ea_s_op.who = "sam";

      tx.clear();
      tx.operations.push_back( ea_b_op );
      tx.operations.push_back( ea_s_op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- failure when 'agent' attempts to release non-disputed escrow to 'to'" );
      op.who = et_op.agent;
      tx.clear();
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE("--- failure when 'agent' attempts to release non-disputed escrow to 'from' " );
      op.receiver = et_op.from;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'agent' attempt to release non-disputed escrow to not 'to' or 'from'" );
      op.receiver = "dave";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when other attempts to release non-disputed escrow to 'to'" );
      op.receiver = et_op.to;
      op.who = "dave";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( dave_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE("--- failure when other attempts to release non-disputed escrow to 'from' " );
      op.receiver = et_op.from;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( dave_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when other attempt to release non-disputed escrow to not 'to' or 'from'" );
      op.receiver = "dave";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( dave_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'to' attemtps to release non-disputed escrow to 'to'" );
      op.receiver = et_op.to;
      op.who = et_op.to;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE("--- failure when 'to' attempts to release non-dispured escrow to 'agent' " );
      op.receiver = et_op.agent;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'to' attempts to release non-disputed escrow to not 'from'" );
      op.receiver = "dave";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success release non-disputed escrow to 'to' from 'from'" );
      op.receiver = et_op.from;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_escrow( op.from, op.escrow_id ).TMEbalance == ASSET( "0.900 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "9.000 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- failure when 'from' attempts to release non-disputed escrow to 'from'" );
      op.receiver = et_op.from;
      op.who = et_op.from;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE("--- failure when 'from' attempts to release non-disputed escrow to 'agent'" );
      op.receiver = et_op.agent;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'from' attempts to release non-disputed escrow to not 'from'" );
      op.receiver = "dave";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success release non-disputed escrow to 'from' from 'to'" );
      op.receiver = et_op.to;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_escrow( op.from, op.escrow_id ).TMEbalance == ASSET( "0.800 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "bob" ).balance == ASSET( "0.100 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- failure when releasing more USD than available" );
      op.TMEamount = ASSET( "1.000 TESTS" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- failure when releasing less TME than available" );
      op.TMEamount = ASSET( "0.000 TESTS" );
      op.USDamount = ASSET( "1.000 USD" );

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'to' attempts to release disputed escrow" );
      escrow_dispute_operation ed_op;
      ed_op.from = "alice";
      ed_op.to = "bob";
      ed_op.agent = "sam";
      ed_op.who = "alice";

      tx.clear();
      tx.operations.push_back( ed_op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.clear();
      op.from = et_op.from;
      op.receiver = et_op.from;
      op.who = et_op.to;
      op.TMEamount = ASSET( "0.100 TESTS" );
      op.USDamount = ASSET( "0.000 USD" );
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'from' attempts to release disputed escrow" );
      tx.clear();
      op.receiver = et_op.to;
      op.who = et_op.from;
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when releasing disputed escrow to an account not 'to' or 'from'" );
      tx.clear();
      op.who = et_op.agent;
      op.receiver = "dave";
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when agent does not match escrow" );
      tx.clear();
      op.who = "dave";
      op.receiver = et_op.from;
      tx.operations.push_back( op );
      tx.sign( dave_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success releasing disputed escrow with agent to 'to'" );
      tx.clear();
      op.receiver = et_op.to;
      op.who = et_op.agent;
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "bob" ).balance == ASSET( "0.200 TESTS" ) );
      BOOST_REQUIRE( db.get_escrow( et_op.from, et_op.escrow_id ).TMEbalance == ASSET( "0.700 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- success releasing disputed escrow with agent to 'from'" );
      tx.clear();
      op.receiver = et_op.from;
      op.who = et_op.agent;
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "9.100 TESTS" ) );
      BOOST_REQUIRE( db.get_escrow( et_op.from, et_op.escrow_id ).TMEbalance == ASSET( "0.600 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- failure when 'to' attempts to release disputed expired escrow" );
      generate_blocks( 2 );

      tx.clear();
      op.receiver = et_op.from;
      op.who = et_op.to;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'from' attempts to release disputed expired escrow" );
      tx.clear();
      op.receiver = et_op.to;
      op.who = et_op.from;
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success releasing disputed expired escrow with agent" );
      tx.clear();
      op.receiver = et_op.from;
      op.who = et_op.agent;
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "9.200 TESTS" ) );
      BOOST_REQUIRE( db.get_escrow( et_op.from, et_op.escrow_id ).TMEbalance == ASSET( "0.500 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- success deleting escrow when balances are both zero" );
      tx.clear();
      op.TMEamount = ASSET( "0.500 TESTS" );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "9.700 TESTS" ) );
      REQUIRE_THROW( db.get_escrow( et_op.from, et_op.escrow_id ), fc::exception );


      tx.clear();
      et_op.ratification_deadline = db.head_block_time() + BLOCK_INTERVAL;
      et_op.escrow_expiration = db.head_block_time() + 2 * BLOCK_INTERVAL;
      tx.operations.push_back( et_op );
      tx.operations.push_back( ea_b_op );
      tx.operations.push_back( ea_s_op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      generate_blocks( 2 );


      BOOST_TEST_MESSAGE( "--- failure when 'agent' attempts to release non-disputed expired escrow to 'to'" );
      tx.clear();
      op.receiver = et_op.to;
      op.who = et_op.agent;
      op.TMEamount = ASSET( "0.100 TESTS" );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'agent' attempts to release non-disputed expired escrow to 'from'" );
      tx.clear();
      op.receiver = et_op.from;
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'agent' attempt to release non-disputed expired escrow to not 'to' or 'from'" );
      tx.clear();
      op.receiver = "dave";
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'to' attempts to release non-dispured expired escrow to 'agent'" );
      tx.clear();
      op.who = et_op.to;
      op.receiver = et_op.agent;
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'to' attempts to release non-disputed expired escrow to not 'from' or 'to'" );
      tx.clear();
      op.receiver = "dave";
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success release non-disputed expired escrow to 'to' from 'to'" );
      tx.clear();
      op.receiver = et_op.to;
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "bob" ).balance == ASSET( "0.300 TESTS" ) );
      BOOST_REQUIRE( db.get_escrow( et_op.from, et_op.escrow_id ).TMEbalance == ASSET( "0.900 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- success release non-disputed expired escrow to 'from' from 'to'" );
      tx.clear();
      op.receiver = et_op.from;
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "8.700 TESTS" ) );
      BOOST_REQUIRE( db.get_escrow( et_op.from, et_op.escrow_id ).TMEbalance == ASSET( "0.800 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- failure when 'from' attempts to release non-disputed expired escrow to 'agent'" );
      tx.clear();
      op.who = et_op.from;
      op.receiver = et_op.agent;
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when 'from' attempts to release non-disputed expired escrow to not 'from' or 'to'" );
      tx.clear();
      op.receiver = "dave";
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- success release non-disputed expired escrow to 'to' from 'from'" );
      tx.clear();
      op.receiver = et_op.to;
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "bob" ).balance == ASSET( "0.400 TESTS" ) );
      BOOST_REQUIRE( db.get_escrow( et_op.from, et_op.escrow_id ).TMEbalance == ASSET( "0.700 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- success release non-disputed expired escrow to 'from' from 'from'" );
      tx.clear();
      op.receiver = et_op.from;
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "8.800 TESTS" ) );
      BOOST_REQUIRE( db.get_escrow( et_op.from, et_op.escrow_id ).TMEbalance == ASSET( "0.600 TESTS" ) );


      BOOST_TEST_MESSAGE( "--- success deleting escrow when balances are zero on non-disputed escrow" );
      tx.clear();
      op.TMEamount = ASSET( "0.600 TESTS" );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "9.400 TESTS" ) );
      REQUIRE_THROW( db.get_escrow( et_op.from, et_op.escrow_id ), fc::exception );
   }
   FC_LOG_AND_RETHROW()
}



   //============================//
   // === Trading Operations === //
   //============================//



BOOST_AUTO_TEST_CASE( limit_order_create_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_create_validate" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( limit_order_create_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_create_authorities" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );

      limit_order_create_operation op;
      op.owner = "alice";
      op.amount_to_sell = ASSET( "1.000 TESTS" );
      op.min_to_receive = ASSET( "1.000 USD" );

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with account signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( limit_order_create_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_create_apply" );

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) ) );

      ACTORS( (alice)(bob) )
      fund( "alice", 1000000 );
      fund( "bob", 1000000 );
      convert( "bob", ASSET("1000.000 TESTS" ) );

      const auto& limit_order_idx = db.get_index< limit_order_index >().indices().get< by_account >();

      BOOST_TEST_MESSAGE( "--- Test failure when account does not have required funds" );
      limit_order_create_operation op;
      signed_transaction tx;

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.min_to_receive = ASSET( "10.000 USD" );
      op.fill_or_kill = false;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "0.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "100.0000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when amount to receive is 0" );

      op.owner = "alice";
      op.min_to_receive = ASSET( "0.000 USD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "1000.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when amount to sell is 0" );

      op.amount_to_sell = ASSET( "0.000 TESTS" );
      op.min_to_receive = ASSET( "10.000 USD" ) ;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "1000.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test success creating limit order that will not be filled" );

      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.min_to_receive = ASSET( "15.000 USD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto limit_order = limit_order_idx.find( std::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == op.amount_to_sell.amount );
      BOOST_REQUIRE( limit_order->sell_price == price( op.amount_to_sell / op.min_to_receive ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure creating limit order with duplicate id" );

      op.amount_to_sell = ASSET( "20.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      limit_order = limit_order_idx.find( std::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 10000 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "10.000 TESTS" ), op.min_to_receive ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test sucess killing an order that will not be filled" );

      op.orderid = 2;
      op.fill_or_kill = true;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test having a partial match to limit order" );
      // Alice has order for 15 USD at a price of 2:3
      // Fill 5 TME for 7.5 USD

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = ASSET( "7.500 USD" );
      op.min_to_receive = ASSET( "5.000 TESTS" );
      op.fill_or_kill = false;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto recent_ops = get_last_operations( 1 );
      auto fill_order_op = recent_ops[0].get< fill_order_operation >();

      limit_order = limit_order_idx.find( std::make_tuple( "alice", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 5000 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "10.000 TESTS" ), ASSET( "15.000 USD" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "7.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "5.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "992.500 USD" ).amount.value );
      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == ASSET( "5.000 TESTS").amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == ASSET( "7.500 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order fully, but the new order partially" );

      op.amount_to_sell = ASSET( "15.000 USD" );
      op.min_to_receive = ASSET( "10.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      limit_order = limit_order_idx.find( std::make_tuple( "bob", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 1 );
      BOOST_REQUIRE( limit_order->for_sale.value == 7500 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "15.000 USD" ), ASSET( "10.000 TESTS" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "15.000 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "977.500 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order and new order fully" );

      op.owner = "alice";
      op.orderid = 3;
      op.amount_to_sell = ASSET( "5.000 TESTS" );
      op.min_to_receive = ASSET( "7.500 USD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", 3 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "bob", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "985.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "22.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "15.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "977.500 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is better." );

      op.owner = "alice";
      op.orderid = 4;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.min_to_receive = ASSET( "11.000 USD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 4;
      op.amount_to_sell = ASSET( "12.000 USD" );
      op.min_to_receive = ASSET( "10.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      limit_order = limit_order_idx.find( std::make_tuple( "bob", 4 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(std::make_tuple( "alice", 4 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 4 );
      BOOST_REQUIRE( limit_order->for_sale.value == 1000 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "12.000 USD" ), ASSET( "10.000 TESTS" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "975.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "33.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "25.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "965.500 USD" ).amount.value );
      validate_database();

      limit_order_cancel_operation can;
      can.owner = "bob";
      can.orderid = 4;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( can );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is worse." );

      auto props = db.get_dynamic_global_properties();
      auto start_USD = props.current_USD_supply;

      op.owner = "alice";
      op.orderid = 5;
      op.amount_to_sell = ASSET( "20.000 TESTS" );
      op.min_to_receive = ASSET( "22.000 USD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 5;
      op.amount_to_sell = ASSET( "12.000 USD" );
      op.min_to_receive = ASSET( "10.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      limit_order = limit_order_idx.find( std::make_tuple( "alice", 5 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(std::make_tuple( "bob", 5 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == 5 );
      BOOST_REQUIRE( limit_order->for_sale.value == 9091 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "20.000 TESTS" ), ASSET( "22.000 USD" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "955.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "45.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "35.909 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "954.500 USD" ).amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( limit_order_create_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_create_authorities" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );

      limit_order_create_operation op;
      op.owner = "alice";
      op.amount_to_sell = ASSET( "1.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with account signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( limit_order_create_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_create_apply" );

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) ) );

      ACTORS( (alice)(bob) )
      fund( "alice", 1000000 );
      fund( "bob", 1000000 );
      convert( "bob", ASSET("1000.000 TESTS" ) );

      const auto& limit_order_idx = db.get_index< limit_order_index >().indices().get< by_account >();

      BOOST_TEST_MESSAGE( "--- Test failure when account does not have required funds" );
      limit_order_create_operation op;
      signed_transaction tx;

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );
      op.fill_or_kill = false;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "0.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "100.0000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when price is 0" );

      op.owner = "alice";
      op.exchange_rate = price( ASSET( "0.000 TESTS" ), ASSET( "1.000 USD" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "1000.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when amount to sell is 0" );

      op.amount_to_sell = ASSET( "0.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "1000.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test success creating limit order that will not be filled" );

      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.exchange_rate = price( ASSET( "2.000 TESTS" ), ASSET( "3.000 USD" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto limit_order = limit_order_idx.find( std::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == op.amount_to_sell.amount );
      BOOST_REQUIRE( limit_order->sell_price == op.exchange_rate );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure creating limit order with duplicate id" );

      op.amount_to_sell = ASSET( "20.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      limit_order = limit_order_idx.find( std::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 10000 );
      BOOST_REQUIRE( limit_order->sell_price == op.exchange_rate );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test sucess killing an order that will not be filled" );

      op.orderid = 2;
      op.fill_or_kill = true;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test having a partial match to limit order" );
      // Alice has order for 15 USD at a price of 2:3
      // Fill 5 TME for 7.5 USD

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = ASSET( "7.500 USD" );
      op.exchange_rate = price( ASSET( "3.000 USD" ), ASSET( "2.000 TESTS" ) );
      op.fill_or_kill = false;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto recent_ops = get_last_operations( 1 );
      auto fill_order_op = recent_ops[0].get< fill_order_operation >();

      limit_order = limit_order_idx.find( std::make_tuple( "alice", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 5000 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "2.000 TESTS" ), ASSET( "3.000 USD" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "7.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "5.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "992.500 USD" ).amount.value );
      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == ASSET( "5.000 TESTS").amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == ASSET( "7.500 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order fully, but the new order partially" );

      op.amount_to_sell = ASSET( "15.000 USD" );
      op.exchange_rate = price( ASSET( "3.000 USD" ), ASSET( "2.000 TESTS" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      limit_order = limit_order_idx.find( std::make_tuple( "bob", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 1 );
      BOOST_REQUIRE( limit_order->for_sale.value == 7500 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "3.000 USD" ), ASSET( "2.000 TESTS" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "990.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "15.000 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "977.500 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order and new order fully" );

      op.owner = "alice";
      op.orderid = 3;
      op.amount_to_sell = ASSET( "5.000 TESTS" );
      op.exchange_rate = price( ASSET( "2.000 TESTS" ), ASSET( "3.000 USD" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", 3 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "bob", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "985.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "22.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "15.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "977.500 USD" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is better." );

      op.owner = "alice";
      op.orderid = 4;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), ASSET( "1.100 USD" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 4;
      op.amount_to_sell = ASSET( "12.000 USD" );
      op.exchange_rate = price( ASSET( "1.200 USD" ), ASSET( "1.000 TESTS" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      limit_order = limit_order_idx.find( std::make_tuple( "bob", 4 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(std::make_tuple( "alice", 4 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 4 );
      BOOST_REQUIRE( limit_order->for_sale.value == 1000 );
      BOOST_REQUIRE( limit_order->sell_price == op.exchange_rate );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "975.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "33.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "25.000 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "965.500 USD" ).amount.value );
      validate_database();

      limit_order_cancel_operation can;
      can.owner = "bob";
      can.orderid = 4;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( can );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is worse." );

      auto props = db.get_dynamic_global_properties();
      auto start_USD = props.current_USD_supply;

      op.owner = "alice";
      op.orderid = 5;
      op.amount_to_sell = ASSET( "20.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), ASSET( "1.100 USD" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 5;
      op.amount_to_sell = ASSET( "12.000 USD" );
      op.exchange_rate = price( ASSET( "1.200 USD" ), ASSET( "1.000 TESTS" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      limit_order = limit_order_idx.find( std::make_tuple( "alice", 5 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(std::make_tuple( "bob", 5 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == 5 );
      BOOST_REQUIRE( limit_order->for_sale.value == 9091 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "1.000 TESTS" ), ASSET( "1.100 USD" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( SYMBOL_USD, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "955.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "45.500 USD" ).amount.value );
      BOOST_REQUIRE( bob.balance.amount.value == ASSET( "35.909 TESTS" ).amount.value );
      BOOST_REQUIRE( bob.USDbalance.amount.value == ASSET( "954.500 USD" ).amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( limit_order_cancel_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_cancel_validate" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( limit_order_cancel_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_cancel_authorities" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );

      limit_order_create_operation c;
      c.owner = "alice";
      c.orderid = 1;
      c.amount_to_sell = ASSET( "1.000 TESTS" );
      c.min_to_receive = ASSET( "1.000 USD" );

      signed_transaction tx;
      tx.operations.push_back( c );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      limit_order_cancel_operation op;
      op.owner = "alice";
      op.orderid = 1;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with account signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( limit_order_cancel_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_cancel_apply" );

      ACTORS( (alice) )
      fund( "alice", 10000 );

      const auto& limit_order_idx = db.get_index< limit_order_index >().indices().get< by_account >();

      BOOST_TEST_MESSAGE( "--- Test cancel non-existent order" );

      limit_order_cancel_operation op;
      signed_transaction tx;

      op.owner = "alice";
      op.orderid = 5;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- Test cancel order" );

      limit_order_create_operation create;
      create.owner = "alice";
      create.orderid = 5;
      create.amount_to_sell = ASSET( "5.000 TESTS" );
      create.min_to_receive = ASSET( "7.500 USD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( create );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", 5 ) ) != limit_order_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( limit_order_idx.find( std::make_tuple( "alice", 5 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value );
      BOOST_REQUIRE( alice.USDbalance.amount.value == ASSET( "0.000 USD" ).amount.value );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_bandwidth )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: account_bandwidth" );
      ACTORS( (alice)(bob) )
      generate_block();
      fund_stake( "alice", ASSET( "10.000 TESTS" ) );
      fund( "alice", ASSET( "10.000 TESTS" ) );
      fund_stake( "bob", ASSET( "10.000 TESTS" ) );

      generate_block();
      db.skip_transaction_delta_check = false;

      BOOST_TEST_MESSAGE( "--- Test first tx in block" );

      signed_transaction tx;
      transfer_operation op;

      op.from = "alice";
      op.to = "bob";
      op.amount = ASSET( "1.000 TESTS" );

      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      auto last_bandwidth_update = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).last_bandwidth_update;
      auto average_bandwidth = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).average_bandwidth;
      BOOST_REQUIRE( last_bandwidth_update == db.head_block_time() );
      BOOST_REQUIRE( average_bandwidth == fc::raw::pack_size( tx ) * 10 * BANDWIDTH_PRECISION );
      auto total_bandwidth = average_bandwidth;

      BOOST_TEST_MESSAGE( "--- Test second tx in block" );

      op.amount = ASSET( "0.100 TESTS" );
      tx.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      last_bandwidth_update = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).last_bandwidth_update;
      average_bandwidth = db.get< witness::account_bandwidth_object, witness::by_account_bandwidth_type >( boost::make_tuple( "alice", witness::bandwidth_type::market ) ).average_bandwidth;
      BOOST_REQUIRE( last_bandwidth_update == db.head_block_time() );
      BOOST_REQUIRE( average_bandwidth == total_bandwidth + fc::raw::pack_size( tx ) * 10 * BANDWIDTH_PRECISION );
   }
   FC_LOG_AND_RETHROW()
}



   //=====================================//
   // === Block Production Operations === //
   //=====================================//



BOOST_AUTO_TEST_CASE( witness_update_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: withness_update_validate" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( witness_update_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: witness_update_authorities" );

      ACTORS( (alice)(bob) );
      fund( "alice", 10000 );

      private_key_type signing_key = generate_private_key( "new_key" );

      witness_update_operation op;
      op.owner = "alice";
      op.url = "foo.bar";
      op.fee = ASSET( "1.000 TESTS" );
      op.block_signing_key = signing_key.get_public_key();

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "--- Test failure when no signatures" );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by a signature not in the account's authority" );
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure when duplicate signatures" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure when signed by an additional signature not in the creator's authority" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test success with witness signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.sign( signing_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( witness_update_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: witness_update_apply" );

      ACTORS( (alice) )
      fund( "alice", 10000 );

      private_key_type signing_key = generate_private_key( "new_key" );

      BOOST_TEST_MESSAGE( "--- Test upgrading an account to a witness" );

      witness_update_operation op;
      op.owner = "alice";
      op.url = "foo.bar";
      op.fee = ASSET( "1.000 TESTS" );
      op.block_signing_key = signing_key.get_public_key();
      op.props.account_creation_fee = MIN_ACCOUNT_CREATION_FEE;
      op.props.maximum_block_size = MIN_BLOCK_SIZE_LIMIT + 100;

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const witness_object& alice_witness = db.get_witness( "alice" );

      BOOST_REQUIRE( alice_witness.owner == "alice" );
      BOOST_REQUIRE( alice_witness.created == db.head_block_time() );
      BOOST_REQUIRE( to_string( alice_witness.url ) == op.url );
      BOOST_REQUIRE( alice_witness.signing_key == op.block_signing_key );
      BOOST_REQUIRE( alice_witness.props.account_creation_fee == op.props.account_creation_fee );
      BOOST_REQUIRE( alice_witness.props.maximum_block_size == op.props.maximum_block_size );
      BOOST_REQUIRE( alice_witness.total_missed == 0 );
      BOOST_REQUIRE( alice_witness.last_aslot == 0 );
      BOOST_REQUIRE( alice_witness.last_confirmed_block_num == 0 );
      BOOST_REQUIRE( alice_witness.pow_worker == 0 );
      BOOST_REQUIRE( alice_witness.votes.value == 0 );
      BOOST_REQUIRE( alice_witness.virtual_last_update == 0 );
      BOOST_REQUIRE( alice_witness.virtual_position == 0 );
      BOOST_REQUIRE( alice_witness.virtual_scheduled_time == fc::uint128_t::max_value() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value ); // No fee
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test updating a witness" );

      tx.signatures.clear();
      tx.operations.clear();
      op.url = "bar.foo";
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice_witness.owner == "alice" );
      BOOST_REQUIRE( alice_witness.created == db.head_block_time() );
      BOOST_REQUIRE( to_string( alice_witness.url ) == "bar.foo" );
      BOOST_REQUIRE( alice_witness.signing_key == op.block_signing_key );
      BOOST_REQUIRE( alice_witness.props.account_creation_fee == op.props.account_creation_fee );
      BOOST_REQUIRE( alice_witness.props.maximum_block_size == op.props.maximum_block_size );
      BOOST_REQUIRE( alice_witness.total_missed == 0 );
      BOOST_REQUIRE( alice_witness.last_aslot == 0 );
      BOOST_REQUIRE( alice_witness.last_confirmed_block_num == 0 );
      BOOST_REQUIRE( alice_witness.pow_worker == 0 );
      BOOST_REQUIRE( alice_witness.votes.value == 0 );
      BOOST_REQUIRE( alice_witness.virtual_last_update == 0 );
      BOOST_REQUIRE( alice_witness.virtual_position == 0 );
      BOOST_REQUIRE( alice_witness.virtual_scheduled_time == fc::uint128_t::max_value() );
      BOOST_REQUIRE( alice.balance.amount.value == ASSET( "10.000 TESTS" ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when upgrading a non-existent account" );

      tx.signatures.clear();
      tx.operations.clear();
      op.owner = "bob";
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( pow_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: pow_validate" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( pow_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: pow_authorities" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( pow_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: pow_apply" );
   }
   FC_LOG_AND_RETHROW()
}


   //===========================//
   // === Custom Operations === //
   //===========================//



BOOST_AUTO_TEST_CASE( custom_authorities )
{
   custom_operation op;
   op.required_auths.insert( "alice" );
   op.required_auths.insert( "bob" );

   flat_set< account_name_type > auths;
   flat_set< account_name_type > expected;

   op.get_required_owner_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   op.get_required_posting_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   expected.insert( "alice" );
   expected.insert( "bob" );
   op.get_required_active_authorities( auths );
   BOOST_REQUIRE( auths == expected );
}

BOOST_AUTO_TEST_CASE( custom_json_authorities )
{
   custom_json_operation op;
   op.required_auths.insert( "alice" );
   op.required_posting_auths.insert( "bob" );

   flat_set< account_name_type > auths;
   flat_set< account_name_type > expected;

   op.get_required_owner_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   expected.insert( "alice" );
   op.get_required_active_authorities( auths );
   BOOST_REQUIRE( auths == expected );

   auths.clear();
   expected.clear();
   expected.insert( "bob" );
   op.get_required_posting_authorities( auths );
   BOOST_REQUIRE( auths == expected );
}

/**
 * 
 * BOOST_AUTO_TEST_CASE( feed_publish_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: feed_publish_validate" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( feed_publish_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: feed_publish_authorities" );

      ACTORS( (alice)(bob) )
      fund( "alice", 10000 );
      witness_create( "alice", alice_private_owner_key, "foo.bar", alice_private_owner_key.get_public_key(), 1000 );

      feed_publish_operation op;
      op.publisher = "alice";
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_post_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test success with witness account signature" );
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( feed_publish_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: feed_publish_apply" );

      ACTORS( (alice) )
      fund( "alice", 10000 );
      witness_create( "alice", alice_private_owner_key, "foo.bar", alice_private_owner_key.get_public_key(), 1000 );

      BOOST_TEST_MESSAGE( "--- Test publishing price feed" );
      feed_publish_operation op;
      op.publisher = "alice";
      op.exchange_rate = price( ASSET( "1000.000 TESTS" ), ASSET( "1.000 USD" ) ); // 1000 TME : 1 USD

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      witness_object& alice_witness = const_cast< witness_object& >( db.get_witness( "alice" ) );

      BOOST_REQUIRE( alice_witness.USD_exchange_rate == op.exchange_rate );
      BOOST_REQUIRE( alice_witness.last_USD_exchange_update == db.head_block_time() );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure publishing to non-existent witness" );

      tx.operations.clear();
      tx.signatures.clear();
      op.publisher = "bob";
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test updating price feed" );

      tx.operations.clear();
      tx.signatures.clear();
      op.exchange_rate = price( ASSET(" 1500.000 TESTS" ), ASSET( "1.000 USD" ) );
      op.publisher = "alice";
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      alice_witness = const_cast< witness_object& >( db.get_witness( "alice" ) );
      BOOST_REQUIRE( std::abs( alice_witness.USD_exchange_rate.to_real() - op.exchange_rate.to_real() ) < 0.0000005 );
      BOOST_REQUIRE( alice_witness.last_USD_exchange_update == db.head_block_time() );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

*/


BOOST_AUTO_TEST_SUITE_END()
//#endif
