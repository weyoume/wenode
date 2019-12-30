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

BOOST_FIXTURE_TEST_SUITE( operation_tests, clean_database_fixture );



   //============================//
   // === Account Operations === //
   //============================//


BOOST_AUTO_TEST_CASE( account_create_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT CREATE" );

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
      op.secure_public_key = string( public_key_type( priv_key.get_public_key() ) );
      op.connection_public_key = string( public_key_type( priv_key.get_public_key() ) );
      op.friend_public_key = string( public_key_type( priv_key.get_public_key() ) );
      op.companion_public_key = string( public_key_type( priv_key.get_public_key() ) );
      op.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
      BOOST_REQUIRE( acct.created == now() );
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
      BOOST_REQUIRE( acct.created == now() );
      BOOST_REQUIRE( acct_auth.owner == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.connection_public_key == public_key_type( priv_key.get_public_key() ));
      BOOST_REQUIRE( acct.friend_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.companion_public_key == public_key_type( priv_key.get_public_key() ) );
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
      BOOST_REQUIRE( acct_bus.account == "ionstudios" );
      BOOST_REQUIRE( acct_bus.executive_board.CHIEF_EXECUTIVE_OFFICER == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.account_type = BUSINESS );
      BOOST_REQUIRE( acct.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.created == now() );
      BOOST_REQUIRE( acct_auth.owner == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.connection_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.friend_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.companion_public_key == public_key_type( priv_key.get_public_key() ) );

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
      
      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account posting authority is invalid" );

      ACTORS( (alice)(bob) );

      account_update_operation op;
      op.account = "alice";
      op.details = "Shrek is love.";
      op.json = "{\"Shrek\":\"life\"}";
      op.posting = authority();
      op.posting->weight_threshold = 1;
      op.posting->add_authorities( "abcdefghijklmnopq", 1 );
      op.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

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
      tx.sign( alice_private_posting_key, db.get_chain_id() );
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

      BOOST_TEST_MESSAGE( "│   ├── Passed: success when updating the owner authority with an owner key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update" );

      private_key_type new_private_key = generate_private_key( "aliceownerhunter2" );

      tx.signatures.clear();
      tx.operations.clear();

      op.owner = authority( 1, new_private_key.get_public_key(), 1 );
      op.active = authority( 2, new_private_key.get_public_key(), 2 );
      op.secure_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      op.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      op.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      op.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      op.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";

      tx.operations.push_back( op );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "alice" );

      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct_auth.owner == authority( 1, new_private_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active == authority( 2, new_private_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct.secure_public_key == public_key_type( new_private_key.get_public_key() ) );
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

      tx.operations.clear();
      tx.signatures.clear();

      op.account = "alice";
      op.posting = authority();
      op.posting->weight_threshold = 1;
      op.posting->add_authorities( "dan", 1 );
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
      
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account membership" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.membership_type = STANDARD_MEMBERSHIP;
      op.months = 1;
      op.interface = INIT_ACCOUNT;

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );

      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct.membership == STANDARD_MEMBERSHIP );
      BOOST_REQUIRE( acct.membership_interface == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recurring_membership == 1 );
      BOOST_REQUIRE( acct.membership_expiration == now() + fc::days(30) );

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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );
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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );
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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_member_request_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.business_account = INIT_ACCOUNT;
      op.message = "Hello";

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_request_object& req = db.get_account_member_request( "alice", INIT_ACCOUNT );

      BOOST_REQUIRE( req.account == "alice" );
      BOOST_REQUIRE( req.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( req.message == "Hello" );
      BOOST_REQUIRE( req.expiration == now() + CONNECTION_REQUEST_DURATION );

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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_member_invite_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.business_account = INIT_ACCOUNT;
      op.member = "bob";
      op.message = "Hello";
      op.encrypted_business_key = string( alice_public_owner_key );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      op.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_invite_object& inv = db.get_account_member_invite( "bob", INIT_ACCOUNT );

      BOOST_REQUIRE( inv.account == "alice" );
      BOOST_REQUIRE( inv.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( inv.member == "bob" );
      BOOST_REQUIRE( inv.message == "Hello" );
      BOOST_REQUIRE( inv.expiration == now() + CONNECTION_REQUEST_DURATION );

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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      account_member_request_operation request;

      request.signatory = "bob";
      request.account = "bob";
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";
      request.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_request_operation accept;

      accept.signatory = "alice";
      accept.account = "alice";
      accept.member = "bob";
      accept.business_account = INIT_ACCOUNT;
      accept.encrypted_business_key = string( alice_public_owner_key );
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when unauthorized request acceptance" );

      tx.operations.clear();
      tx.signatures.clear();

      account_member_request_operation request;

      request.signatory = "candice";
      request.account = "candice";
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";
      request.validate();

      signed_transaction tx;
      tx.operations.push_back( request );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_request_operation accept;

      accept.signatory = "dan";     // dan is not an officer, and cannot accept requests
      accept.account = "dan";
      accept.member = "candice";
      accept.business_account = INIT_ACCOUNT;
      accept.encrypted_business_key = string( candice_public_owner_key );
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( dan_private_active_key, db.get_chain_id() );

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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      account_member_invite_operation invite;

      invite.signatory = "alice";
      invite.account = "alice";
      invite.member = "bob";
      invite.business_account = INIT_ACCOUNT;
      invite.message = "Hello";
      invite.encrypted_business_key = string( alice_public_owner_key );
      invite.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation accept;
      accept.signatory = "bob";
      accept.account = "bob";
      accept.business_account = INIT_ACCOUNT;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invite does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation op;

      op.signatory = "dan";   // dan has not been invited
      op.account = "dan";
      op.business_account = INIT_ACCOUNT;

      op.validate();

      tx.operations.push_back( op );
      tx.sign( dan_private_active_key, db.get_chain_id() );

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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

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
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_update_list_operation op;
      op.signatory = "alice";
      op.account = "alice";
      op.listed_account = "bob";
      op.blacklisted = true;
      op.whitelisted = false;
      
      op.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_permission_object& acc_perm = db.get_account_permissions( "alice" );

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

      const account_permission_object& acc_perm = db.get_account_permissions( "alice" );

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

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account witness vote" );
   
      ACTORS( (alice)(bob)(candice)(dan)(elon)(sam) );

      fund( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( "candice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "candice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );

      private_key_type alice_witness_key = generate_private_key( "alicewitnesscorrecthorsebatterystaple" );

      witness_create(
         "alice", 
         alice_private_owner_key, 
         alice_witness_key.get_public_key(), 
         BLOCKCHAIN_PRECISION * 10 
      );

      private_key_type candice_witness_key = generate_private_key( "candicewitnesscorrecthorsebatterystaple" );

      witness_create(
         "candice", 
         candice_private_owner_key, 
         candice_witness_key.get_public_key(), 
         BLOCKCHAIN_PRECISION * 10 
      );

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

      op.witness = "candice";   // Vote for candice at rank one will move alice to rank two

      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const witness_vote_object& vote_a = db.get_witness_vote( "bob", "alice" );
      const witness_vote_object& vote_c = db.get_witness_vote( "bob", "candice" );
      const account_object& acc = db.get_account( "bob" ); 
      const witness_object& witness_a = db.get_witness( "alice" );
      const witness_object& witness_c = db.get_witness( "candice" );
      BOOST_REQUIRE( vote_a.account == "bob" );
      BOOST_REQUIRE( vote_a.witness == "alice" );
      BOOST_REQUIRE( vote_a.vote_rank == 2 );
      BOOST_REQUIRE( vote_c.account == "bob" );
      BOOST_REQUIRE( vote_c.witness == "candice" );
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

      const witness_vote_object* vote_ptr = db.find_witness_vote( "bob", "candice" );
      const account_object& acc = db.get_account( "bob" );
      const witness_object& witness = db.get_witness( "candice" );
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
      op.approved = true;

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

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update proxy" );
      
      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      price equity_price = props.current_median_equity_price;

      ACTORS( (alice)(bob)(candice)(sam)(dan)(elon) )
      fund( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "candice", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "sam", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "sam", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "dan", asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_update_proxy_operation op;

      op.signatory = "bob";
      op.account = "bob";
      op.proxy = "alice";

      signed_transaction tx;
      tx.operations.push_back( op );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
      op.proxy = "dan";
      
      tx.operations.push_back( op );
      tx.sign( sam_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );     // bob->sam->dan

      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( *sam.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandparent proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandchild proxy" );

      // alice \
      // bob->  sam->dan

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
      BOOST_REQUIRE( sam.proxied.size() == 2 );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) + db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
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

      db.push_transaction( tx, 0 );    // alice->sam->dan

      BOOST_REQUIRE( alice.proxy == "sam" );
      BOOST_REQUIRE( bob.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == "alice" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
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
      
      ACTORS( (alice)(bob)(candice)(sam)(dan) );
      fund( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

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
      acc_create.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_create.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_create.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_create.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      signed_transaction tx;
      tx.operations.push_back( acc_create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& bob_auth = db.get< account_authority_object, by_account >( "bob" );
      BOOST_REQUIRE( bob_auth.owner == acc_create.owner );

      // Changing bob's owner authority

      account_update_operation acc_update;

      acc_update.signatory = "bob";
      acc_update.account = "bob";
      acc_update.owner = authority( 1, generate_private_key( "bad_key" ).get_public_key(), 1 );
      acc_update.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_update.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_update.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_update.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_update.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_update.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_update.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_update.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";

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

      generate_blocks( now() + OWNER_UPDATE_LIMIT );

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

      generate_blocks( now() + OWNER_UPDATE_LIMIT + BLOCK_INTERVAL );

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
      BOOST_REQUIRE( req_itr->expiration == now() + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD );

      time_point expires = req_itr->expiration;
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
      tx.set_expiration( now() );

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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now() + ( OWNER_AUTH_RECOVERY_PERIOD - ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD ) );
      generate_block();

      request.new_owner_authority = authority( 1, generate_private_key( "last key" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      recover.new_owner_authority = request.new_owner_authority;
      recover.recent_owner_authority = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "last key" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // bob_owner key has expired
      const auto& owner6 = db.get< account_authority_object, by_account >("bob").owner;
      BOOST_REQUIRE( owner6 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      recover.recent_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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

      generate_blocks( now() + OWNER_AUTH_RECOVERY_PERIOD - BLOCK_INTERVAL, true );
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

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

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
      acc_create.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_create.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_create.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_create.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      signed_transaction tx;
      tx.operations.push_back( acc_create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& bob_auth = db.get< account_authority_object, by_account >( "bob" );
      BOOST_REQUIRE( bob_auth.owner == acc_create.owner );

      generate_blocks( now() + ( fc::days( bob.reset_account_delay_days ) - BLOCK_INTERVAL ) );

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
      set.new_reset_account = "candice";

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );
      
      BOOST_REQUIRE( bob.reset_account == "candice" );

      BOOST_TEST_MESSAGE( "│   ├── Passed: set new reset account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional account reset" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + ( fc::days( bob.reset_account_delay_days ) - BLOCK_INTERVAL ) );

      reset.signatory = "candice";
      reset.reset_account = "candice";
      reset.account_to_reset = "bob";
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key_1" ).get_public_key(), 1 );

      tx.operations.push_back( reset );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
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

      set.new_reset_account = "candice";

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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< decline_voting_rights_request_index >().indices().get< by_account >();
      auto req_itr = req_idx.find( "alice" );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->effective_date == now() + DECLINE_VOTING_RIGHTS_DURATION );

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
      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed

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

      op.declined = true;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now() + DECLINE_VOTING_RIGHTS_DURATION - BLOCK_INTERVAL, true );

      BOOST_REQUIRE( db.get_account( "alice" ).can_vote );   // Alice can still vote

      witness_create( 
         "alice", 
         alice_private_owner_key,
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
      comment.language = "en";
      comment.interface = INIT_ACCOUNT;
      comment.tags.push_back( "test" );
      comment.json = "{\"json\":\"valid\"}";

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;

      comment_options options;
      comment.options = options;

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

      auto req_itr = req_idx.find( "alice" );
      BOOST_REQUIRE( req_itr == req_idx.end() );   // Request no longer exists

      auto witness_vote_itr = witness_vote_idx.find( boost::make_tuple( "alice", "alice" ) );
      BOOST_REQUIRE( witness_vote_itr == witness_vote_idx.end() );   // Vote has been removed

      tx.operations.clear();
      tx.signatures.clear();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
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

BOOST_AUTO_TEST_CASE( connection_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CONNECTION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: create connection request" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );
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

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< connection_request_index >().indices().get< by_account_req >();
      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      const auto& con_idx = db.get_index< connection_index >().indices().get< by_accounts >();
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", CONNECTION ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->account == "alice" );
      BOOST_REQUIRE( req_itr->requested_account == "bob" );
      BOOST_REQUIRE( req_itr->connection_type == CONNECTION );
      BOOST_REQUIRE( req_itr->expiration == now() + CONNECTION_REQUEST_DURATION );
      BOOST_REQUIRE( con_itr == con_idx.end() );    // Connection not created yet

      BOOST_TEST_MESSAGE( "│   ├── Passed: create connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      connection_accept_operation accept;

      accept.signatory = "bob";
      accept.account = "bob";
      accept.requesting_account = "alice";
      accept.connection_type = CONNECTION;
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", CONNECTION ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == CONNECTION );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: match connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "alice";
      accept.account = "alice";
      accept.requesting_account = "bob";

      tx.operations.push_back( accept );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", CONNECTION ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == CONNECTION );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: match connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting existing connection type" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting existing connection type" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting friend connection before request duration" );

      generate_blocks( now() + CONNECTION_REQUEST_DURATION - BLOCK_INTERVAL, true );

      request.connection_type = FRIEND;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting friend connection before request duration" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: friend connection request" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", FRIEND ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->account == "alice" );
      BOOST_REQUIRE( req_itr->requested_account == "bob" );
      BOOST_REQUIRE( req_itr->connection_type == FRIEND );
      BOOST_REQUIRE( req_itr->expiration == now() + CONNECTION_REQUEST_DURATION );
      BOOST_REQUIRE( con_itr == con_idx.end() );    // Connection not created yet

      BOOST_TEST_MESSAGE( "│   ├── Passed: friend connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept friend connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "bob";
      accept.account = "bob";
      accept.requesting_account = "alice";
      accept.connection_type = FRIEND;
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", FRIEND ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == FRIEND );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept friend connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: match friend connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "alice";
      accept.account = "alice";
      accept.requesting_account = "bob";
      accept.connection_type = FRIEND;

      tx.operations.push_back( accept );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", FRIEND ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == FRIEND );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: match friend request" );

      BOOST_TEST_MESSAGE( "├── Passed: CONNECTION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_follow_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT FOLLOW" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account follow" );

      ACTORS( (alice)(bob) );

      account_follow_operation op;

      op.signatory = "alice";
      op.follower = "alice";
      op.following = "bob";
      op.interface = INIT_ACCOUNT;
      op.added = true;
      op.followed = true;

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( "alice" );
      const account_following_object& following_b = db.get_account_following( "bob" );

      BOOST_REQUIRE( following_a.is_following( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_follower( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account follow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: mutual following" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "bob";
      op.follower = "bob";
      op.following = "alice";

      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( following_a.is_follower( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_following( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( following_a.is_mutual( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_mutual( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: mutual following" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when following already followed account" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "bob";
      op.follower = "bob";
      op.following = "alice";

      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when following already followed account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: unfollowing" );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "bob";
      op.follower = "bob";
      op.following = "alice";
      op.added = false;
      op.followed = true;

      tx.operations.push_back( op );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      op.signatory = "alice";
      op.follower = "alice";
      op.following = "bob";
      op.added = false;
      op.followed = true;

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !following_a.is_follower( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_following( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( !following_a.is_following( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_follower( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( !following_a.is_mutual( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_mutual( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: unfollowing" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: filtering" );

      op.signatory = "alice";
      op.follower = "alice";
      op.following = "bob";
      
      op.added = true;
      op.followed = false;

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( account_name_type( "alice" ) );
      const account_following_object& following_b = db.get_account_following( account_name_type( "bob" ) );

      BOOST_REQUIRE( following_a.is_filtered( account_name_type( "bob" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: filtering" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT FOLLOW" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( tag_follow_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TAG FOLLOW" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal tag follow" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      tag_follow_operation op;

      op.signatory = "alice";
      op.follower = "alice";
      op.tag = "test";
      op.interface = INIT_ACCOUNT;
      op.added = true;
      op.followed = true;

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( "alice" );
      const tag_following_object& following_t = db.get_tag_following( "test" );
      
      BOOST_REQUIRE( following_a.is_following( tag_name_type( "test" ) ) );
      BOOST_REQUIRE( following_t.is_follower( account_name_type( "alice" ) ) );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal tag follow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when following already followed tag" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when following already followed tag" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: unfollowing" );

      tx.operations.clear();
      tx.signatures.clear();

      op.added = false;

      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( "alice" );
      const tag_following_object& following_t = db.get_tag_following( "test" );
      
      BOOST_REQUIRE( !following_a.is_following( tag_name_type( "test" ) ) );
      BOOST_REQUIRE( !following_t.is_follower( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: unfollowing" );

      BOOST_TEST_MESSAGE( "├── Passed: TAG FOLLOW" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( activity_reward_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACTIVITY REWARD" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal activity reward claim process" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "alice", alice_private_owner_key, alice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "bob", bob_private_owner_key, bob_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "candice", candice_private_owner_key, candice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "dan", dan_private_owner_key, dan_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "elon", elon_private_owner_key, elon_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "fred", fred_private_owner_key, fred_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "george", george_private_owner_key, george_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "haz", haz_private_owner_key, haz_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "jayme", jayme_private_owner_key, jayme_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      const comment_object& alice_post = comment_create( "alice", alice_private_posting_key, "alicetestpost" );
      const comment_object& bob_post = comment_create( "bob", bob_private_posting_key, "bobtestpost" );
      const comment_object& candice_post = comment_create( "candice", candice_private_posting_key, "candicetestpost" );

      vote_operation vote;

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.author = "alice";
      vote.permlink = "alicetestpost";
      vote.weight = PERCENT_100;
      vote.interface = INIT_ACCOUNT;
      vote.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "bob";
      vote.permlink = "bobtestpost";

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.author = "alice";
      vote.permlink = "alicetestpost";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "dan";
      vote.voter = "dan";
      
      tx.operations.push_back( vote );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "elon";
      vote.voter = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view_operation view;

      view.signatory = "bob";
      view.viewer = "bob";
      view.author = "alice";
      view.permlink = "alicetestpost";
      view.interface = INIT_ACCOUNT;
      view.supernode = INIT_ACCOUNT;
      view.viewed = true;
      view.validate();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "alice";
      view.viewer = "alice";
      view.author = "candice";
      view.permlink = "candicetestpost";

      tx.operations.push_back( view );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "candice";
      view.viewer = "candice";
      view.author = "alice";
      view.permlink = "alicetestpost";

      tx.operations.push_back( view );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "dan";
      view.viewer = "dan";

      tx.operations.push_back( view );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "elon";
      view.viewer = "elon";

      tx.operations.push_back( view );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      db.update_comment_metrics();     // calculate median values.

      account_witness_vote_operation wv;

      wv.signatory = "alice";
      wv.account = "alice";
      wv.witness = "bob";
      wv.vote_rank = 1;
      wv.approved = true;
      wv.validate();

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "candice";
      wv.vote_rank = 2;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "dan";
      wv.vote_rank = 3;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "elon";
      wv.vote_rank = 5;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "fred";
      wv.vote_rank = 6;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "george";
      wv.vote_rank = 7;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "haz";
      wv.vote_rank = 8;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "isabelle";
      wv.vote_rank = 9;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "jayme";
      wv.vote_rank = 10;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      wv.witness = "kathryn";
      wv.vote_rank = 11;

      tx.operations.push_back( wv );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      activity_reward_operation activity;

      activity.signatory = "alice";
      activity.account = "alice";
      activity.permlink = "alicetestpost";
      activity.vote_id = bob_post.id._id;
      activity.view_id = candice_post.id._id;
      activity.interface = INIT_ACCOUNT;

      tx.operations.push_back( activity );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const reward_fund_object& rfo = db.get_reward_fund();
      
      BOOST_REQUIRE( alice.last_activity_reward == now() );
      BOOST_REQUIRE( rfo.recent_activity_claims == BLOCKCHAIN_PRECISION );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal activity reward claim process" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when claiming again" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( activity );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when claiming again" );

      BOOST_TEST_MESSAGE( "├── Passed: ACTIVITY REWARD" );
   }
   FC_LOG_AND_RETHROW()
}


   //============================//
   // === Network Operations === //
   //============================//


BOOST_AUTO_TEST_CASE( update_network_officer_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: NETWORK OFFICER SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: network officer creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "alice", alice_private_owner_key, alice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "bob", bob_private_owner_key, bob_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "candice", candice_private_owner_key, candice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "dan", dan_private_owner_key, dan_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "elon", elon_private_owner_key, elon_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "fred", fred_private_owner_key, fred_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "george", george_private_owner_key, george_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "haz", haz_private_owner_key, haz_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "jayme", jayme_private_owner_key, jayme_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "leonie", leonie_private_owner_key, leonie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "margot", margot_private_owner_key, margot_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "natalie", natalie_private_owner_key, natalie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "olivia", olivia_private_owner_key, olivia_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "peter", peter_private_owner_key, peter_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "quentin", quentin_private_owner_key, quentin_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "rachel", rachel_private_owner_key, rachel_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "sam", sam_private_owner_key, sam_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "tim", tim_private_owner_key, tim_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "veronica", veronica_private_owner_key, veronica_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      account_membership_operation membership;

      membership.signatory = "alice";
      membership.account = "alice";
      membership.membership_type = STANDARD_MEMBERSHIP;
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

      update_network_officer_operation officer;

      officer.signatory = "alice";
      officer.account = "alice";
      officer.officer_type = DEVELOPMENT;
      officer.details = "details";
      officer.url = "www.url.com";
      officer.json = "{\"json\":\"valid\"}";
      officer.active = true;
      officer.validate();

      tx.operations.push_back( officer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const network_officer_object& alice_officer = db.get_network_officer( "alice" );
      
      BOOST_REQUIRE( alice_officer.account == "alice" );
      BOOST_REQUIRE( alice_officer.officer_type == DEVELOPMENT );
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

      vote.signatory = "tim";
      vote.account = "tim";

      tx.operations.push_back( vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
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

      const network_officer_object& alice_officer = db.get_network_officer( "alice" );
      
      BOOST_REQUIRE( alice_officer.account == "alice" );
      BOOST_REQUIRE( alice_officer.officer_type == DEVELOPMENT );
      BOOST_REQUIRE( alice_officer.active == true );
      BOOST_REQUIRE( alice_officer.officer_approved == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: network officer approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: NETWORK OFFICER SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( update_executive_board_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: EXECUTIVE BOARD SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: executive board creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica) 
      (alice2)(bob2)(candice2)(dan2)(elon2)(fred2)(george2)(haz2)(isabelle2)(jayme2)(kathryn2)(leonie2)(margot2)(natalie2)(olivia2)(peter2)(quentin2)(rachel2)(sam2)(tim2)(veronica2));

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "alice", alice_private_owner_key, alice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "bob", bob_private_owner_key, bob_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "candice", candice_private_owner_key, candice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "dan", dan_private_owner_key, dan_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "elon", elon_private_owner_key, elon_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "fred", fred_private_owner_key, fred_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "george", george_private_owner_key, george_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "haz", haz_private_owner_key, haz_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "jayme", jayme_private_owner_key, jayme_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "leonie", leonie_private_owner_key, leonie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "margot", margot_private_owner_key, margot_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "natalie", natalie_private_owner_key, natalie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "olivia", olivia_private_owner_key, olivia_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "peter", peter_private_owner_key, peter_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "quentin", quentin_private_owner_key, quentin_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "rachel", rachel_private_owner_key, rachel_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "sam", sam_private_owner_key, sam_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "tim", tim_private_owner_key, tim_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "veronica", veronica_private_owner_key, veronica_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "alice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "alice2", alice2_private_owner_key, alice2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "bob2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "bob2", bob2_private_owner_key, bob2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "candice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "candice2", candice2_private_owner_key, candice2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "dan2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "dan2", dan2_private_owner_key, dan2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "elon2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "elon2", elon2_private_owner_key, elon2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "fred2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "fred2", fred2_private_owner_key, fred2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "george2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "george2", george2_private_owner_key, george2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "haz2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "haz2", haz2_private_owner_key, haz2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "isabelle2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "isabelle2", isabelle2_private_owner_key, isabelle2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "jayme2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "jayme2", jayme2_private_owner_key, jayme2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "kathryn2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "kathryn2", kathryn2_private_owner_key, kathryn2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "leonie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "leonie2", leonie2_private_owner_key, leonie2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "margot2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "margot2", margot2_private_owner_key, margot2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "natalie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "natalie2", natalie2_private_owner_key, natalie2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "olivia", olivia_private_owner_key, olivia_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "peter2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "peter2", peter2_private_owner_key, peter2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "quentin2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "quentin2", quentin2_private_owner_key, quentin2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "rachel2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "rachel2", rachel2_private_owner_key, rachel2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "sam2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "sam2", sam2_private_owner_key, sam2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "tim2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "tim2", tim2_private_owner_key, tim2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "veronica2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica2", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "veronica2", veronica2_private_owner_key, veronica2_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.new_account_name = "execboard";
      create.account_type = BUSINESS;
      create.governance_account = INIT_ACCOUNT;
      create.business_type = PUBLIC_BUSINESS;
      create.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      create.business_public_key = alice_public_posting_key;
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.governance_account = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      fund_stake( "execboard", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "execboard", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      tx.operations.clear();
      tx.signatures.clear();

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
      member.membership_type = TOP_MEMBERSHIP;
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();

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
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_network_officer_operation officer;

      officer.signatory = "bob";
      officer.account = "bob";
      officer.officer_type = DEVELOPMENT;
      officer.details = "details";
      officer.url = "www.url.com";
      officer.json = "{\"json\":\"valid\"}";
      officer.active = true;
      officer.validate();

      tx.operations.push_back( officer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = "candice";
      officer.account = "candice";
      officer.officer_type = MARKETING;

      tx.operations.push_back( officer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      officer.signatory = "dan";
      officer.account = "dan";
      officer.officer_type = ADVOCACY;

      tx.operations.push_back( officer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

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
      exec_vote.role = CHIEF_DEVELOPMENT_OFFICER;
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = "candice";
      exec_vote.role = CHIEF_MARKETING_OFFICER;
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      exec_vote.executive_account = "dan";
      exec_vote.role = CHIEF_ADVOCACY_OFFICER;
      exec_vote.vote_rank = 1;

      tx.operations.push_back( exec_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_supernode_operation supernode;

      supernode.signatory = "alice";
      supernode.account = "execboard";
      supernode.details = "details";
      supernode.url = "www.url.com";
      supernode.json = "{\"json\":\"valid\"}";
      supernode.validate();

      tx.operations.push_back( supernode );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_interface_operation interface;
      
      interface.signatory = "alice";
      interface.account = "execboard";
      interface.details = "details";
      interface.url = "www.url.com";
      interface.json = "{\"json\":\"valid\"}";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
      gov.signatory = "alice";
      gov.account = "execboard";
      gov.details = "details";
      gov.url = "www.url.com";
      gov.json = "{\"json\":\"valid\"}";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "execboard";
      create.new_account_name = "newuser";
      create.account_type = PERSONA;
      create.governance_account = "execboard";
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      for( auto i = 0; i < 100; i++ )
      {
         create.new_account_name = "newuser"+fc::to_string( i );

         tx.operations.push_back( create );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();
      }

      view_operation view;

      view.signatory = "newuser";
      view.viewer = "newuser";
      view.author = "alice";
      view.permlink = "alicetestpost";
      view.supernode = "execboard";
      view.interface = "execboard";
      view.validate();
      
      for( auto i = 0; i < 100; i++ )
      {
         view.signatory = "newuser"+fc::to_string( i );
         view.viewer = "newuser"+fc::to_string( i );

         tx.operations.push_back( view );
         tx.sign( alice_private_posting_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();
      }

      update_executive_board_operation exec;

      exec.signatory = "alice";
      exec.account = "alice";
      exec.executive = "execboard";
      exec.budget = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      exec.details = "details";
      exec.url = "www.url.com";
      exec.json = "{\"json\":\"valid\"}";
      exec.active = true;
      exec.validate();

      tx.operations.push_back( exec );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const executive_board_object& executive = db.get_executive_board( "execboard" );
      
      BOOST_REQUIRE( executive.account == "execboard" );
      BOOST_REQUIRE( executive.board_approved == false );
      BOOST_REQUIRE( executive.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: executive board creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: executive board approval process" );

      executive_board_vote_operation eb_vote;    // 40 accounts vote for executive board

      eb_vote.signatory = "bob";
      eb_vote.account = "bob";
      eb_vote.executive_board = "execboard";
      eb_vote.vote_rank = 1;
      eb_vote.approved = true;
      eb_vote.validate();

      tx.operations.push_back( eb_vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "candice";
      eb_vote.account = "candice";

      tx.operations.push_back( eb_vote );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "dan";
      eb_vote.account = "dan";

      tx.operations.push_back( eb_vote );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "elon";
      eb_vote.account = "elon";

      tx.operations.push_back( eb_vote );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "fred";
      eb_vote.account = "fred";

      tx.operations.push_back( eb_vote );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "george";
      eb_vote.account = "george";

      tx.operations.push_back( eb_vote );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "haz";
      eb_vote.account = "haz";

      tx.operations.push_back( eb_vote );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "isabelle";
      eb_vote.account = "isabelle";

      tx.operations.push_back( eb_vote );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "jayme";
      eb_vote.account = "jayme";

      tx.operations.push_back( eb_vote );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "kathryn";
      eb_vote.account = "kathryn";

      tx.operations.push_back( eb_vote );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "leonie";
      eb_vote.account = "leonie";

      tx.operations.push_back( eb_vote );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "margot";
      eb_vote.account = "margot";

      tx.operations.push_back( eb_vote );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "natalie";
      eb_vote.account = "natalie";

      tx.operations.push_back( eb_vote );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "olivia";
      eb_vote.account = "olivia";

      tx.operations.push_back( eb_vote );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "peter";
      eb_vote.account = "peter";

      tx.operations.push_back( eb_vote );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "quentin";
      eb_vote.account = "quentin";

      tx.operations.push_back( eb_vote );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "rachel";
      eb_vote.account = "rachel";

      tx.operations.push_back( eb_vote );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "sam";
      eb_vote.account = "sam";

      tx.operations.push_back( eb_vote );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      eb_vote.signatory = "tim";
      eb_vote.account = "tim";

      tx.operations.push_back( eb_vote );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "veronica";
      eb_vote.account = "veronica";

      tx.operations.push_back( eb_vote );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "bob2";
      eb_vote.account = "bob2";

      tx.operations.push_back( eb_vote );
      tx.sign( bob2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "candice2";
      eb_vote.account = "candice2";

      tx.operations.push_back( eb_vote );
      tx.sign( candice2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "dan2";
      eb_vote.account = "dan2";

      tx.operations.push_back( eb_vote );
      tx.sign( dan2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "elon2";
      eb_vote.account = "elon2";

      tx.operations.push_back( eb_vote );
      tx.sign( elon2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "fred2";
      eb_vote.account = "fred2";

      tx.operations.push_back( eb_vote );
      tx.sign( fred2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "george2";
      eb_vote.account = "george2";

      tx.operations.push_back( eb_vote );
      tx.sign( george2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "haz2";
      eb_vote.account = "haz2";

      tx.operations.push_back( eb_vote );
      tx.sign( haz2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "isabelle2";
      eb_vote.account = "isabelle2";

      tx.operations.push_back( eb_vote );
      tx.sign( isabelle2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "jayme2";
      eb_vote.account = "jayme2";

      tx.operations.push_back( eb_vote );
      tx.sign( jayme2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "kathryn2";
      eb_vote.account = "kathryn2";

      tx.operations.push_back( eb_vote );
      tx.sign( kathryn2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "leonie2";
      eb_vote.account = "leonie2";

      tx.operations.push_back( eb_vote );
      tx.sign( leonie2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "margot2";
      eb_vote.account = "margot2";

      tx.operations.push_back( eb_vote );
      tx.sign( margot2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "natalie2";
      eb_vote.account = "natalie2";

      tx.operations.push_back( eb_vote );
      tx.sign( natalie2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "olivia2";
      eb_vote.account = "olivia2";

      tx.operations.push_back( eb_vote );
      tx.sign( olivia2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "peter2";
      eb_vote.account = "peter2";

      tx.operations.push_back( eb_vote );
      tx.sign( peter2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "quentin2";
      eb_vote.account = "quentin2";

      tx.operations.push_back( eb_vote );
      tx.sign( quentin2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "rachel2";
      eb_vote.account = "rachel2";

      tx.operations.push_back( eb_vote );
      tx.sign( rachel2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "sam2";
      eb_vote.account = "sam2";

      tx.operations.push_back( eb_vote );
      tx.sign( sam2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      eb_vote.signatory = "tim2";
      eb_vote.account = "tim2";

      tx.operations.push_back( eb_vote );
      tx.sign( tim2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      eb_vote.signatory = "veronica2";
      eb_vote.account = "veronica2";

      tx.operations.push_back( eb_vote );
      tx.sign( veronica2_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const executive_board_object& executive = db.get_executive_board( "execboard" );
      
      BOOST_REQUIRE( executive.account == "execboard" );
      BOOST_REQUIRE( executive.board_approved == true );
      BOOST_REQUIRE( executive.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: executive board approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: EXECUTIVE BOARD SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( update_governance_account_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: GOVERNANCE ACCOUNT SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica));

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "alice", alice_private_owner_key, alice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "bob", bob_private_owner_key, bob_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "candice", candice_private_owner_key, candice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "dan", dan_private_owner_key, dan_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "elon", elon_private_owner_key, elon_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "fred", fred_private_owner_key, fred_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "george", george_private_owner_key, george_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "haz", haz_private_owner_key, haz_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "jayme", jayme_private_owner_key, jayme_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "leonie", leonie_private_owner_key, leonie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "margot", margot_private_owner_key, margot_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "natalie", natalie_private_owner_key, natalie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "olivia", olivia_private_owner_key, olivia_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "peter", peter_private_owner_key, peter_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "quentin", quentin_private_owner_key, quentin_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "rachel", rachel_private_owner_key, rachel_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "sam", sam_private_owner_key, sam_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "tim", tim_private_owner_key, tim_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "veronica", veronica_private_owner_key, veronica_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.new_account_name = "govaccount";
      create.account_type = BUSINESS;
      create.governance_account = INIT_ACCOUNT;
      create.business_type = PUBLIC_BUSINESS;
      create.officer_vote_threshold = BLOCKCHAIN_PRECISION;
      create.business_public_key = alice_public_posting_key;
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.governance_account = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      fund_stake( "govaccount", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "govaccount", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      tx.operations.clear();
      tx.signatures.clear();

      account_membership_operation member;

      member.signatory = "govaccount";
      member.account = "govaccount";
      member.membership_type = TOP_MEMBERSHIP;
      member.months = 1;
      member.interface = INIT_ACCOUNT;
      member.validate();

      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
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

      const governance_account_object& governance = db.get_governance_account( "govaccount" );
      
      BOOST_REQUIRE( governance.account == "govaccount" );
      BOOST_REQUIRE( governance.account_approved == false );
      BOOST_REQUIRE( governance.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: governance account approval process" );

      subscribe_governance_operation sub;    // 20 accounts subscribe to governance address

      sub.signatory = "bob";
      sub.account = "bob";
      sub.governance_account = "govaccount";
      sub.subscribe = true;
      sub.validate();

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

      sub.signatory = "tim";
      sub.account = "tim";

      tx.operations.push_back( sub );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
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

      const governance_account_object& governance = db.get_governance_account( "govaccount" );
      
      BOOST_REQUIRE( governance.account == "govaccount" );
      BOOST_REQUIRE( governance.account_approved == true );
      BOOST_REQUIRE( governance.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: governance account approval process" );

      BOOST_TEST_MESSAGE( "├── Passed: GOVERNANCE ACCOUNT SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( update_supernode_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: SUPERNODE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      update_supernode_operation supernode;
      
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

      const supernode_object& alice_supernode = db.get_supernode( "alice" );
      
      BOOST_REQUIRE( alice_supernode.account == "alice" );
      BOOST_REQUIRE( alice_supernode.daily_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.monthly_active_users == 0 );
      BOOST_REQUIRE( alice_supernode.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: supernode creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: supernode viewing process" );

      const comment_object& alice_post = comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      view_operation view;

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
      
      const supernode_object& alice_supernode = db.get_supernode( "alice" );
      
      BOOST_REQUIRE( alice_supernode.account == "alice" );
      BOOST_REQUIRE( alice_supernode.daily_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_supernode.monthly_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_supernode.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: supernode viewing process" );

      BOOST_TEST_MESSAGE( "├── Passed: SUPERNODE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( update_interface_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: INTERFACE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: interface creation" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation member;

      member.signatory = "alice";
      member.account = "alice";
      member.membership_type = TOP_MEMBERSHIP;
      member.months = 1;
      member.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( member );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_interface_operation interface;
      
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

      const interface_object& alice_interface = db.get_interface( "alice" );
      
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

      const comment_object& alice_post = comment_create( "alice", alice_private_posting_key, "alicetestpost" );

      view_operation view;

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

      const interface_object& alice_interface = db.get_interface( "alice" );
      
      BOOST_REQUIRE( alice_interface.account == "alice" );
      BOOST_REQUIRE( alice_interface.daily_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_interface.monthly_active_users == PERCENT_100 );
      BOOST_REQUIRE( alice_interface.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: interface viewing process" );

      BOOST_TEST_MESSAGE( "├── Passed: INTERFACE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( community_enterprise_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMUNITY ENTERPRISE SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie)(olivia)(peter)(quentin)(rachel)(sam)(tim)(veronica));

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "alice", alice_private_owner_key, alice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "bob", bob_private_owner_key, bob_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "candice", candice_private_owner_key, candice_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "dan", dan_private_owner_key, dan_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "elon", elon_private_owner_key, elon_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "fred", fred_private_owner_key, fred_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "george", george_private_owner_key, george_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "haz", haz_private_owner_key, haz_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "jayme", jayme_private_owner_key, jayme_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "leonie", leonie_private_owner_key, leonie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "margot", margot_private_owner_key, margot_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "natalie", natalie_private_owner_key, natalie_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "olivia", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "olivia", olivia_private_owner_key, olivia_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "peter", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "peter", peter_private_owner_key, peter_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "quentin", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "quentin", quentin_private_owner_key, quentin_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "rachel", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "rachel", rachel_private_owner_key, rachel_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "sam", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "sam", sam_private_owner_key, sam_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "tim", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "tim", tim_private_owner_key, tim_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      fund_stake( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "veronica", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      witness_create( "veronica", veronica_private_owner_key, veronica_public_owner_key, BLOCKCHAIN_PRECISION * 10 );

      create_community_enterprise_operation create;

      create.signatory = "alice";
      create.creator = "alice";
      create.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      create.proposal_type = FUNDING;
      create.beneficiaries[ "alice" ] = PERCENT_100;
      create.milestones.push_back( std::make_pair( "Begin proposal", 50*PERCENT_1 ) );
      create.milestones.push_back( std::make_pair( "Finish proposal", 50*PERCENT_1 ) );
      create.details = "details";
      create.url = "www.url.com";
      create.json = "{\"json\":\"valid\"}";
      create.begin = now() + fc::days(8);
      create.duration = 14;
      create.daily_budget = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_CREDIT );
      create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );
      
      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == -1 );
      BOOST_REQUIRE( enterprise.claimed_milestones == 0 );
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal milestone approval" );

      approve_enterprise_milestone_operation approve;

      approve.signatory = "alice";
      approve.account = "alice";
      approve.creator = "alice";
      approve.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      approve.milestone = 0;
      approve.details = "details";
      approve.vote_rank = 1;
      approve.approved = true;
      approve.validate();

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "bob";
      approve.account = "bob";

      tx.operations.push_back( approve );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "candice";
      approve.account = "candice";

      tx.operations.push_back( approve );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "dan";
      approve.account = "dan";

      tx.operations.push_back( approve );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "elon";
      approve.account = "elon";

      tx.operations.push_back( approve );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "fred";
      approve.account = "fred";

      tx.operations.push_back( approve );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "george";
      approve.account = "george";

      tx.operations.push_back( approve );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "haz";
      approve.account = "haz";

      tx.operations.push_back( approve );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "isabelle";
      approve.account = "isabelle";

      tx.operations.push_back( approve );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "jayme";
      approve.account = "jayme";

      tx.operations.push_back( approve );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "kathryn";
      approve.account = "kathryn";

      tx.operations.push_back( approve );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "leonie";
      approve.account = "leonie";

      tx.operations.push_back( approve );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "margot";
      approve.account = "margot";

      tx.operations.push_back( approve );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "natalie";
      approve.account = "natalie";

      tx.operations.push_back( approve );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "olivia";
      approve.account = "olivia";

      tx.operations.push_back( approve );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "peter";
      approve.account = "peter";

      tx.operations.push_back( approve );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "quentin";
      approve.account = "quentin";

      tx.operations.push_back( approve );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "rachel";
      approve.account = "rachel";

      tx.operations.push_back( approve );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "sam";
      approve.account = "sam";

      tx.operations.push_back( approve );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      approve.signatory = "tim";
      approve.account = "tim";

      tx.operations.push_back( approve );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "veronica";
      approve.account = "veronica";

      tx.operations.push_back( approve );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const witness_schedule_object& witness_schedule = db.get_witness_schedule();
      db.update_enterprise( enterprise, witness_schedule, props );

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );

      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == 0 );    // initial milestone now approved
      BOOST_REQUIRE( enterprise.claimed_milestones == 0 );
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );

      generate_blocks( BLOCKS_PER_DAY * 8 - 1 );

      BOOST_REQUIRE( enterprise.days_paid == 0 );

      generate_block();

      BOOST_REQUIRE( enterprise.days_paid == 1 );   // Daily budget is paid

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal milestone approval" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise claim milestone" );

      claim_enterprise_milestone_operation claim;

      claim.signatory = "alice";
      claim.creator = "alice";
      claim.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      claim.milestone = 1;
      claim.details = "details";
      claim.validate();

      tx.operations.push_back( claim );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const witness_schedule_object& witness_schedule = db.get_witness_schedule();
      db.update_enterprise( enterprise, witness_schedule, props );

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );

      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == 0 );
      BOOST_REQUIRE( enterprise.claimed_milestones == 1 );   // next milestone claimed
      BOOST_REQUIRE( enterprise.days_paid == 0 );
      BOOST_REQUIRE( enterprise.active == true );

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise claim milestone" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community enterprise proposal next milestone approval" );

      approve_enterprise_milestone_operation approve;

      approve.signatory = "alice";
      approve.account = "alice";
      approve.creator = "alice";
      approve.enterprise_id = "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7";
      approve.milestone = 1;
      approve.details = "details";
      approve.vote_rank = 1;
      approve.approved = true;
      approve.validate();

      tx.operations.push_back( approve );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "bob";
      approve.account = "bob";

      tx.operations.push_back( approve );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "candice";
      approve.account = "candice";

      tx.operations.push_back( approve );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "dan";
      approve.account = "dan";

      tx.operations.push_back( approve );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "elon";
      approve.account = "elon";

      tx.operations.push_back( approve );
      tx.sign( elon_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "fred";
      approve.account = "fred";

      tx.operations.push_back( approve );
      tx.sign( fred_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "george";
      approve.account = "george";

      tx.operations.push_back( approve );
      tx.sign( george_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "haz";
      approve.account = "haz";

      tx.operations.push_back( approve );
      tx.sign( haz_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "isabelle";
      approve.account = "isabelle";

      tx.operations.push_back( approve );
      tx.sign( isabelle_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "jayme";
      approve.account = "jayme";

      tx.operations.push_back( approve );
      tx.sign( jayme_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "kathryn";
      approve.account = "kathryn";

      tx.operations.push_back( approve );
      tx.sign( kathryn_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "leonie";
      approve.account = "leonie";

      tx.operations.push_back( approve );
      tx.sign( leonie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "margot";
      approve.account = "margot";

      tx.operations.push_back( approve );
      tx.sign( margot_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "natalie";
      approve.account = "natalie";

      tx.operations.push_back( approve );
      tx.sign( natalie_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "olivia";
      approve.account = "olivia";

      tx.operations.push_back( approve );
      tx.sign( olivia_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "peter";
      approve.account = "peter";

      tx.operations.push_back( approve );
      tx.sign( peter_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "quentin";
      approve.account = "quentin";

      tx.operations.push_back( approve );
      tx.sign( quentin_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "rachel";
      approve.account = "rachel";

      tx.operations.push_back( approve );
      tx.sign( rachel_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "sam";
      approve.account = "sam";

      tx.operations.push_back( approve );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      approve.signatory = "tim";
      approve.account = "tim";

      tx.operations.push_back( approve );
      tx.sign( tim_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      approve.signatory = "veronica";
      approve.account = "veronica";

      tx.operations.push_back( approve );
      tx.sign( veronica_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      const witness_schedule_object& witness_schedule = db.get_witness_schedule();
      db.update_enterprise( enterprise, witness_schedule, props );

      const community_enterprise_object& enterprise = db.get_community_enterprise( "alice", "b54f0fa9-8ef3-4f0f-800c-0026c88fe9b7" );

      BOOST_REQUIRE( enterprise.creator == "alice" );
      BOOST_REQUIRE( enterprise.approved_milestones == 1 );    // next milestone now approved
      BOOST_REQUIRE( enterprise.claimed_milestones == 1 );
      BOOST_REQUIRE( enterprise.days_paid == 1 );
      BOOST_REQUIRE( enterprise.active == true );

      generate_blocks( BLOCKS_PER_DAY * 14 - 1 );

      BOOST_REQUIRE( enterprise.days_paid == 13 );

      generate_block();

      BOOST_REQUIRE( enterprise.days_paid == 14 );   // Daily budget is fully completed

      BOOST_TEST_MESSAGE( "│   ├── Passed: community enterprise proposal next milestone approval" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY ENTERPRISE SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



   //=====================================//
   // === Post and Comment Operations === //
   //=====================================//



BOOST_AUTO_TEST_CASE( comment_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMENT OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signatures" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( fc::seconds( 60 ).count() / BLOCK_INTERVAL.count() );

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = INIT_BOARD;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"foo\":\"bar\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( comment );
   
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_posting_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );

      tx.sign( alice_private_posting_key, db.get_chain_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by an additional signature not in the creator's authority" );

      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by an additional signature not in the creator's authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by a signature not in the creator's authority" );

      tx.signatures.clear();
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_posting_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by a signature not in the creator's authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Success posting a root comment" );

      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      BOOST_REQUIRE( alice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( alice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( alice_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( alice_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( alice_comment.ipfs[0] ) == comment.ipfs[0] );
      BOOST_REQUIRE( to_string( alice_comment.magnet[0] ) == comment.magnet[0] );
      BOOST_REQUIRE( alice_comment.board == comment.board );
      BOOST_REQUIRE( alice_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( alice_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( alice_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( alice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( alice_comment.json ) == comment.json );
      BOOST_REQUIRE( alice_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( alice_comment.premium_price == comment.premium_price );

      BOOST_REQUIRE( alice_comment.last_update == now() );
      BOOST_REQUIRE( alice_comment.created == now() );
      BOOST_REQUIRE( alice_comment.active == now() );

      BOOST_REQUIRE( alice_comment.depth == 0 );
      BOOST_REQUIRE( alice_comment.children == 0 );
      BOOST_REQUIRE( alice_comment.net_votes == 0 );
      BOOST_REQUIRE( alice_comment.view_count == 0 );
      BOOST_REQUIRE( alice_comment.share_count == 0 );

      BOOST_REQUIRE( alice_comment.net_reward == 0 );
      BOOST_REQUIRE( alice_comment.vote_power == 0 );
      BOOST_REQUIRE( alice_comment.view_power == 0 );
      BOOST_REQUIRE( alice_comment.share_power == 0 );
      BOOST_REQUIRE( alice_comment.comment_power == 0 );
      
      BOOST_REQUIRE( alice_comment.cashout_time == fc::time_point( now() + CONTENT_REWARD_INTERVAL ) );
      BOOST_REQUIRE( alice_comment.cashouts_received == 0 );

      BOOST_REQUIRE( alice_comment.total_vote_weight == 0 );
      BOOST_REQUIRE( alice_comment.total_view_weight == 0 );
      BOOST_REQUIRE( alice_comment.total_share_weight == 0 );
      BOOST_REQUIRE( alice_comment.total_comment_weight == 0 );

      BOOST_REQUIRE( alice_comment.total_payout_value.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.curator_payout_value.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.beneficiary_payout_value.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.content_rewards.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.percent_liquid == PERCENT_100 );
      BOOST_REQUIRE( alice_comment.reward == 0 );
      BOOST_REQUIRE( alice_comment.weight == 0 );
      BOOST_REQUIRE( alice_comment.max_weight == 0 );

      BOOST_REQUIRE( alice_comment.author_reward_percent == props.median_props.author_reward_percent );
      BOOST_REQUIRE( alice_comment.vote_reward_percent == props.median_props.vote_reward_percent );
      BOOST_REQUIRE( alice_comment.view_reward_percent == props.median_props.view_reward_percent );
      BOOST_REQUIRE( alice_comment.share_reward_percent == props.median_props.share_reward_percent );
      BOOST_REQUIRE( alice_comment.comment_reward_percent == props.median_props.comment_reward_percent );
      BOOST_REQUIRE( alice_comment.storage_reward_percent == props.median_props.storage_reward_percent );
      BOOST_REQUIRE( alice_comment.moderator_reward_percent == props.median_props.moderator_reward_percent );

      BOOST_REQUIRE( alice_comment.allow_replies == true );
      BOOST_REQUIRE( alice_comment.allow_votes == true );
      BOOST_REQUIRE( alice_comment.allow_views == true );
      BOOST_REQUIRE( alice_comment.allow_shares == true );
      BOOST_REQUIRE( alice_comment.allow_curation_rewards == true );
      BOOST_REQUIRE( alice_comment.root == true );
      BOOST_REQUIRE( alice_comment.deleted == false );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Success posting a root comment" );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when posting a comment on a non-existent comment" );

      comment.signatory = "bob";
      comment.author = "bob";
      comment.permlink = "ipsum";
      comment.parent_author = "alice";
      comment.parent_permlink = "foobar";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when posting a comment on a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting a comment on previous comment" );

      comment.parent_permlink = "lorem";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( "bob", string( "ipsum" ) );

      BOOST_REQUIRE( bob_comment.author == comment.author );
      BOOST_REQUIRE( to_string( bob_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( bob_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( bob_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( bob_comment.ipfs[0] ) == comment.ipfs[0] );
      BOOST_REQUIRE( to_string( bob_comment.magnet[0] ) == comment.magnet[0] );
      BOOST_REQUIRE( bob_comment.board == comment.board );
      BOOST_REQUIRE( bob_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( bob_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( bob_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( bob_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( bob_comment.json ) == comment.json );
      BOOST_REQUIRE( bob_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( bob_comment.premium_price == comment.premium_price );

      BOOST_REQUIRE( bob_comment.last_update == now() );
      BOOST_REQUIRE( bob_comment.created == now() );
      BOOST_REQUIRE( bob_comment.active == now() );

      BOOST_REQUIRE( bob_comment.depth == 1 );
      BOOST_REQUIRE( bob_comment.children == 0 );
      BOOST_REQUIRE( bob_comment.net_votes == 0 );
      BOOST_REQUIRE( bob_comment.view_count == 0 );
      BOOST_REQUIRE( bob_comment.share_count == 0 );

      BOOST_REQUIRE( bob_comment.net_reward == 0 );
      BOOST_REQUIRE( bob_comment.vote_power == 0 );
      BOOST_REQUIRE( bob_comment.view_power == 0 );
      BOOST_REQUIRE( bob_comment.share_power == 0 );
      BOOST_REQUIRE( bob_comment.comment_power == 0 );
      
      BOOST_REQUIRE( bob_comment.cashout_time == fc::time_point( now() + CONTENT_REWARD_INTERVAL ) );
      BOOST_REQUIRE( bob_comment.cashouts_received == 0 );

      BOOST_REQUIRE( bob_comment.total_vote_weight == 0 );
      BOOST_REQUIRE( bob_comment.total_view_weight == 0 );
      BOOST_REQUIRE( bob_comment.total_share_weight == 0 );
      BOOST_REQUIRE( bob_comment.total_comment_weight == 0 );

      BOOST_REQUIRE( bob_comment.total_payout_value.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.curator_payout_value.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.beneficiary_payout_value.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.content_rewards.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.percent_liquid == PERCENT_100 );

      BOOST_REQUIRE( bob_comment.author_reward_percent == props.median_props.author_reward_percent );
      BOOST_REQUIRE( bob_comment.vote_reward_percent == props.median_props.vote_reward_percent );
      BOOST_REQUIRE( bob_comment.view_reward_percent == props.median_props.view_reward_percent );
      BOOST_REQUIRE( bob_comment.share_reward_percent == props.median_props.share_reward_percent );
      BOOST_REQUIRE( bob_comment.comment_reward_percent == props.median_props.comment_reward_percent );
      BOOST_REQUIRE( bob_comment.storage_reward_percent == props.median_props.storage_reward_percent );
      BOOST_REQUIRE( bob_comment.moderator_reward_percent == props.median_props.moderator_reward_percent );

      BOOST_REQUIRE( bob_comment.allow_replies == true );
      BOOST_REQUIRE( bob_comment.allow_votes == true );
      BOOST_REQUIRE( bob_comment.allow_views == true );
      BOOST_REQUIRE( bob_comment.allow_shares == true );
      BOOST_REQUIRE( bob_comment.allow_curation_rewards == true );
      BOOST_REQUIRE( bob_comment.root == false );
      BOOST_REQUIRE( bob_comment.deleted == false );
      BOOST_REQUIRE( bob_comment.root_comment == alice_comment.id );

      BOOST_REQUIRE( alice_comment.children == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: posting a comment on previous comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting a comment on additional previous comment" );

      comment.signatory = "candice";
      comment.author = "candice";
      comment.permlink = "dolor";
      comment.parent_author = "bob";
      comment.parent_permlink = "ipsum";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( "candice", string( "dolor" ) );

      BOOST_REQUIRE( candice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( candice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( candice_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( candice_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( candice_comment.ipfs[0] ) == comment.ipfs[0] );
      BOOST_REQUIRE( to_string( candice_comment.magnet[0] ) == comment.magnet[0] );
      BOOST_REQUIRE( candice_comment.board == comment.board );
      BOOST_REQUIRE( candice_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( candice_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( candice_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( candice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( candice_comment.json ) == comment.json );
      BOOST_REQUIRE( candice_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( candice_comment.premium_price == comment.premium_price );

      BOOST_REQUIRE( candice_comment.last_update == now() );
      BOOST_REQUIRE( candice_comment.created == now() );
      BOOST_REQUIRE( candice_comment.active == now() );

      BOOST_REQUIRE( candice_comment.depth == 2 );
      BOOST_REQUIRE( candice_comment.children == 0 );
      BOOST_REQUIRE( candice_comment.net_votes == 0 );
      BOOST_REQUIRE( candice_comment.view_count == 0 );
      BOOST_REQUIRE( candice_comment.share_count == 0 );

      BOOST_REQUIRE( candice_comment.net_reward == 0 );
      BOOST_REQUIRE( candice_comment.vote_power == 0 );
      BOOST_REQUIRE( candice_comment.view_power == 0 );
      BOOST_REQUIRE( candice_comment.share_power == 0 );
      BOOST_REQUIRE( candice_comment.comment_power == 0 );
      
      BOOST_REQUIRE( candice_comment.cashout_time == fc::time_point( now() + CONTENT_REWARD_INTERVAL ) );
      BOOST_REQUIRE( candice_comment.cashouts_received == 0 );

      BOOST_REQUIRE( candice_comment.total_vote_weight == 0 );
      BOOST_REQUIRE( candice_comment.total_view_weight == 0 );
      BOOST_REQUIRE( candice_comment.total_share_weight == 0 );
      BOOST_REQUIRE( candice_comment.total_comment_weight == 0 );

      BOOST_REQUIRE( candice_comment.total_payout_value.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.curator_payout_value.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.beneficiary_payout_value.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.content_rewards.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.percent_liquid == PERCENT_100 );

      BOOST_REQUIRE( candice_comment.author_reward_percent == props.median_props.author_reward_percent );
      BOOST_REQUIRE( candice_comment.vote_reward_percent == props.median_props.vote_reward_percent );
      BOOST_REQUIRE( candice_comment.view_reward_percent == props.median_props.view_reward_percent );
      BOOST_REQUIRE( candice_comment.share_reward_percent == props.median_props.share_reward_percent );
      BOOST_REQUIRE( candice_comment.comment_reward_percent == props.median_props.comment_reward_percent );
      BOOST_REQUIRE( candice_comment.storage_reward_percent == props.median_props.storage_reward_percent );
      BOOST_REQUIRE( candice_comment.moderator_reward_percent == props.median_props.moderator_reward_percent );

      BOOST_REQUIRE( candice_comment.allow_replies == true );
      BOOST_REQUIRE( candice_comment.allow_votes == true );
      BOOST_REQUIRE( candice_comment.allow_views == true );
      BOOST_REQUIRE( candice_comment.allow_shares == true );
      BOOST_REQUIRE( candice_comment.allow_curation_rewards == true );
      BOOST_REQUIRE( candice_comment.root == false );
      BOOST_REQUIRE( candice_comment.deleted == false );
      BOOST_REQUIRE( candice_comment.root_comment == alice_comment.id );

      BOOST_REQUIRE( bob_comment.children == 1 );
      BOOST_REQUIRE( alice_comment.children == 2 );

      validate_database();

      generate_blocks( 5 * BLOCKS_PER_MINUTE + 1 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: posting a comment on additional previous comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: modifying a comment" );

      fc::time_point created = candice_comment.created;

      tx.signatures.clear();
      tx.operations.clear();

      comment.title = "foo";
      comment.body = "bar";
      comment.json = "{\"bar\":\"foo\"}";
      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( "candice", string( "dolor" ) );

      BOOST_REQUIRE( candice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( candice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( candice_comment.parent_author == comment.parent_author );
      BOOST_REQUIRE( to_string( candice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( candice_comment.last_update == now() );
      BOOST_REQUIRE( candice_comment.created == created );
      BOOST_REQUIRE( candice_comment.cashout_time == candice_comment.created + CONTENT_REWARD_INTERVAL );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: modifying a comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure posting again within time limit" );

      comment.permlink = "sit";
      comment.parent_author = "";
      comment.parent_permlink = "test";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( 5 * BLOCKS_PER_MINUTE );

      comment.permlink = "amet";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      generate_block();
      db.push_transaction( tx, 0 );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure posting again within time limit" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: more than 100% weight on a single route" );

      comment_options options;

      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_100 + 1 ) );

      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: more than 100% weight on a single route" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: more than 100% total weight" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 * 75 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "sam" ), PERCENT_1 * 75 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: more than 100% total weight" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: maximum number of routes" );

      options.beneficiaries.clear();

      for( size_t i = 0; i < 127; i++ )
      {
         options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "foo" + fc::to_string( i ) ), 1 ) );
      }

      std::sort( options.beneficiaries.begin(), options.beneficiaries.end() );
      options.validate();

      BOOST_TEST_MESSAGE( "│   ├── Passed: maximum number of routes" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: one too many routes" );

      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bar" ), 1 ) );
      std::sort( options.beneficiaries.begin(), options.beneficiaries.end() );

      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: one too many routes" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: duplicate accounts" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 * 2 ) );
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: duplicate accounts" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: incorrect account sort order" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( "alice", PERCENT_1 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: incorrect account sort order" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: correct account sort order" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( "alice", PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      
      options.validate();

      BOOST_TEST_MESSAGE( "│   ├── Passed: correct account sort order" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: specifying a non-existent benefactor" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "doug" ), PERCENT_1 ) );

      options.validate();
      comment.options = options;

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: specifying a non-existent benefactor" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: setting when comment has been voted on" );

      vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "candice";
      vote.permlink = "dolor";
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;
      vote.validate();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();
      
      options.beneficiaries.clear();

      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), 25 * PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "alice" ), 50 * PERCENT_1 ) );

      options.validate();
      comment.options = options;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: setting when comment has been voted on" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success when altering beneficiaries before voting" );

      comment.signatory = "bob";
      comment.author = "bob";
      comment.permlink = "ipsum";
      comment.parent_author = "alice";
      comment.parent_permlink = "foobar";

      tx.signatures.clear();
      tx.operations.clear();

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success when altering beneficiaries before voting" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when there are already beneficiaries" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "dan" ), 25 * PERCENT_1 ) );
      comment.options = options;

      tx.signatures.clear();
      tx.operations.clear();

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when there are already beneficiaries" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Payout and verify rewards were split properly" );

      tx.signatures.clear();
      tx.operations.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      time_point cashout_time = candice_comment.cashout_time;

      generate_blocks( cashout_time - BLOCK_INTERVAL );

      BOOST_REQUIRE( candice_comment.cashouts_received == 0 );

      asset alice_reward = db.get_reward_balance( "alice", SYMBOL_COIN );
      asset bob_reward = db.get_reward_balance( "bob", SYMBOL_COIN );
      asset candice_reward = db.get_reward_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_reward.amount == 0 );
      BOOST_REQUIRE( bob_reward.amount == 0 );
      BOOST_REQUIRE( candice_reward.amount == 0 );

      generate_block();

      BOOST_REQUIRE( candice_comment.cashouts_received == 1 );

      asset alice_reward = db.get_reward_balance( "alice", SYMBOL_COIN );
      asset bob_reward = db.get_reward_balance( "bob", SYMBOL_COIN );
      asset candice_reward = db.get_reward_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_reward.amount > 0 );
      BOOST_REQUIRE( bob_reward.amount > 0 );
      BOOST_REQUIRE( candice_reward.amount > 0 );

      BOOST_REQUIRE( bob_reward + candice_reward == alice_reward );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Payout and verify rewards were split properly" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMENT OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( message_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: MESSAGE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no connection" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      message_operation message;

      message.signatory = "alice";
      message.sender = "alice";
      message.recipient = "bob";
      message.message = "Hello";
      message.uuid = "6a91e502-1e53-4531-a97a-379ac8a495ff";

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no connection" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: message success with connection" );

      connection_request_operation request;

      request.signatory = "alice";
      request.account = "alice";
      request.requested_account = "bob";
      request.connection_type = CONNECTION;
      request.message = "Hello";
      request.requested = true;

      tx.operations.push_back( request );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      connection_accept_operation accept;

      accept.signatory = "bob";
      accept.account = "bob";
      accept.requesting_account = "alice";
      accept.connection_type = CONNECTION;
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "alice";
      accept.account = "alice";
      accept.requesting_account = "bob";
      accept.connection_type = CONNECTION;
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( message );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 ); 

      const auto& message_idx = db.get_index< message_index >().indices().get< by_sender_uuid >();
      auto message_itr = message_idx.find( std::make_tuple( "alice", "6a91e502-1e53-4531-a97a-379ac8a495ff" ) );

      BOOST_REQUIRE( message_itr != message_idx.end() );
      BOOST_REQUIRE( message_itr->message == message.message );
      BOOST_REQUIRE( message_itr->created == now() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: message success with connection" );

      BOOST_TEST_MESSAGE( "├── Passed: MESSAGE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( vote_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: VOTE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting on a non-existent comment" );
      
      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = INIT_BOARD;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_operation vote;

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.author = "alice";
      vote.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting on a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting with an initial weight of 0" );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting with an initial weight of 0" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful vote with PERCENT_100 weight" );

      auto old_voting_power = bob.voting_power;

      vote.weight = PERCENT_100;
      vote.permlink = "lorem";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      auto bob_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "bob" ) );
      int64_t max_vote_denom = props.median_props.vote_reserve_rate * ( props.median_props.vote_recharge_time.count() / fc::days(1).count() );

      BOOST_REQUIRE( bob.voting_power == old_voting_power - ( ( old_voting_power + max_vote_denom - 1 ) / max_vote_denom ) );
      BOOST_REQUIRE( bob.last_vote_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_vote_itr != vote_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful vote with PERCENT_100 weight" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: negative vote" );

      old_voting_power = candice.voting_power;

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.weight = -1 * PERCENT_100;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "candice" ) );

      BOOST_REQUIRE( candice.voting_power == old_voting_power - ( ( old_voting_power + max_vote_denom - 1 ) / max_vote_denom ) );
      BOOST_REQUIRE( candice.last_vote_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( candice_vote_itr != vote_idx.end() );
      BOOST_REQUIRE( candice_vote_itr->last_update == now() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: negative vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adjusting vote weight" );

      generate_blocks( now() + fc::seconds( MIN_VOTE_INTERVAL_SEC ) );

      vote.weight = PERCENT_1 * 50;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "candice" ) );

      BOOST_REQUIRE( candice_vote_itr->last_update == now() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adjusting vote weight" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: changing vote to 0 weight" );

      generate_blocks( now() + fc::seconds( MIN_VOTE_INTERVAL_SEC ) );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      auto candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "candice" ) );

      BOOST_REQUIRE( candice_vote_itr->last_update == now() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );
      BOOST_REQUIRE( candice_vote_itr->reward == 0 );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: changing vote to 0 weight" );

      BOOST_TEST_MESSAGE( "├── Passed: VOTE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( view_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: VIEW OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when viewing a non-existent comment" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& view_idx = db.get_index< comment_view_index >().indices().get< by_comment_viewer >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = INIT_BOARD;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view_operation view;

      view.signatory = "bob";
      view.viewer = "bob";
      view.author = "alice";
      view.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      view.interface = INIT_ACCOUNT;
      view.supernode = INIT_ACCOUNT;
      view.validate();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when viewing a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful view" );

      auto old_viewing_power = bob.viewing_power;

      view.permlink = "lorem";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      auto bob_view_itr = view_idx.find( std::make_tuple( alice_comment.id, "bob" ) );
      int64_t max_view_denom = props.median_props.view_reserve_rate * ( props.median_props.view_recharge_time.count() / fc::days(1).count() );

      BOOST_REQUIRE( bob.viewing_power == old_viewing_power - ( ( old_viewing_power + max_view_denom - 1 ) / max_view_denom ) );
      BOOST_REQUIRE( bob.last_view_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_view_itr != view_idx.end() );
      BOOST_REQUIRE( bob_view_itr->weight > 0 );
      BOOST_REQUIRE( bob_view_itr->reward > 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful view" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing view" );

      generate_blocks( now() + fc::seconds( MIN_VIEW_INTERVAL_SEC ) );

      view.viewed = false;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto bob_view_itr = view_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( bob_view_itr == view_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing view" );

      BOOST_TEST_MESSAGE( "├── Passed: VIEW OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( share_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: SHARE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when sharing a non-existent comment" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& share_idx = db.get_index< comment_share_index >().indices().get< by_comment_sharer >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = INIT_BOARD;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      share_operation share;

      share.signatory = "bob";
      share.sharer = "bob";
      share.author = "alice";
      share.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      share.reach = FOLLOW_FEED;
      share.interface = INIT_ACCOUNT;

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when sharing a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful share to follow feed" );

      auto old_sharing_power = bob.sharing_power;

      share.permlink = "lorem";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      auto bob_share_itr = share_idx.find( std::make_tuple( alice_comment.id, "bob" ) );
      int64_t max_share_denom = props.median_props.share_reserve_rate * ( props.median_props.share_recharge_time.count() / fc::days(1).count() );

      BOOST_REQUIRE( bob.sharing_power == old_sharing_power - ( ( old_sharing_power + max_share_denom - 1 ) / max_share_denom ) );
      BOOST_REQUIRE( bob.last_share_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_share_itr != share_idx.end() );
      BOOST_REQUIRE( bob_share_itr->weight > 0 );
      BOOST_REQUIRE( bob_share_itr->reward > 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful share to follow feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing share" );

      generate_blocks( now() + fc::seconds( MIN_SHARE_INTERVAL_SEC ) );

      share.shared = false;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto bob_share_itr = share_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( bob_share_itr == share_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing share" );

      BOOST_TEST_MESSAGE( "├── Passed: SHARE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( moderation_tag_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: MODERATION TAG OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when moderating a non-existent comment" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = INIT_BOARD;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      moderation_tag_operation tag;

      tag.signatory = "bob";
      tag.moderator =  "bob";
      tag.author = "alice";
      tag.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      tag.tags.push_back( "nsfw" );
      tag.rating = MATURE;
      tag.details = "Post is NSFW";
      tag.interface = INIT_ACCOUNT;
      tag.filter = false;
      tag.applied = true;
      tag.validate();

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when moderating a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account is not governance account" );

      tx.operations.clear();
      tx.signatures.clear();

      tag.permlink = "lorem";

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account is not governance account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful moderation tag" );

      account_membership_operation mem;

      mem.signatory = "bob";
      mem.account = "bob";
      mem.membership_type = TOP_MEMBERSHIP;
      mem.months = 1;
      mem.interface = INIT_ACCOUNT;
      mem.validate();

      tx.operations.push_back( mem );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
      gov.signatory = "bob";
      gov.account = "bob";
      gov.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      gov.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      gov.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_account_object& governance = db.get_governance_account( "bob" );
      
      BOOST_REQUIRE( governance.active == true );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      const auto& tag_idx = db.get_index< moderation_tag_index >().indices().get< by_comment_moderator >();
      auto tag_itr = tag_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( tag_itr->details == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );
   
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful moderation tag" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing moderation tag" );

      generate_blocks( now() + fc::seconds( MIN_SHARE_INTERVAL_SEC ) );

      tag.applied = false;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto tag_itr = tag_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( tag_itr == tag_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing moderation tag" );

      BOOST_TEST_MESSAGE( "├── Passed: MODERATION TAG OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}



   //==========================//
   // === Board Operations === //
   //==========================//


   
BOOST_AUTO_TEST_CASE( board_create_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: BOARD CREATE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful board creation" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& board_idx = db.get_index< board_index >().indices().get< by_name >();

      signed_transaction tx;

      board_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopenboard";
      create.board_type = BOARD;
      create.board_privacy = OPEN_BOARD;
      create.board_public_key = string( alice_public_posting_key );
      create.json = "{\"json\":\"valid\"}";
      create.json_private = "{\"json\":\"valid\"}";
      create.details = "details";
      create.url = "www.url.com";
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "aliceopenboard" );
      BOOST_REQUIRE( board_itr->founder == create.founder );
      BOOST_REQUIRE( board_itr->name == create.name );
      BOOST_REQUIRE( board_itr->board_privacy == create.board_privacy );
      BOOST_REQUIRE( board_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpublicboard";
      create.board_privacy = PUBLIC_BOARD;
      create.board_public_key = string( bob_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "bobpublicboard" );
      BOOST_REQUIRE( board_itr->founder == create.founder );
      BOOST_REQUIRE( board_itr->name == create.name );
      BOOST_REQUIRE( board_itr->board_privacy == create.board_privacy );
      BOOST_REQUIRE( board_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivateboard";
      create.board_privacy = PRIVATE_BOARD;
      create.board_public_key = string( candice_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "candiceprivateboard" );
      BOOST_REQUIRE( board_itr->founder == create.founder );
      BOOST_REQUIRE( board_itr->name == create.name );
      BOOST_REQUIRE( board_itr->board_privacy == create.board_privacy );
      BOOST_REQUIRE( board_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusiveboard";
      create.board_privacy = EXCLUSIVE_BOARD;
      create.board_public_key = string( dan_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "danexclusiveboard" );
      BOOST_REQUIRE( board_itr->founder == create.founder );
      BOOST_REQUIRE( board_itr->name == create.name );
      BOOST_REQUIRE( board_itr->board_privacy == create.board_privacy );
      BOOST_REQUIRE( board_itr->created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful board creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when board created before MIN_BOARD_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_BOARD_CREATE_INTERVAL - BLOCK_INTERVAL );

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "mysecondboard";
      create.board_public_key = string( alice_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( alice_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when board created before MIN_BOARD_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success after MIN_BOARD_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      tx.operations.push_back( create );
      tx.sign( alice_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when board created before MIN_BOARD_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when board name already exists" );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "elon";
      create.founder = "elon";
      create.name = "aliceopenboard";

      tx.operations.push_back( create );
      tx.sign( elon_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when board name already exists" );

      BOOST_TEST_MESSAGE( "├── Passed: BOARD CREATE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( board_update_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: BOARD UPDATE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder board update sequence" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& board_idx = db.get_index< board_index >().indices().get< by_name >();

      signed_transaction tx;

      board_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopenboard";
      create.board_type = BOARD;
      create.board_privacy = OPEN_BOARD;
      create.board_public_key = string( alice_public_posting_key );
      create.json = "{\"json\":\"valid\"}";
      create.json_private = "{\"json\":\"valid\"}";
      create.details = "details";
      create.url = "www.url.com";
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpublicboard";
      create.board_privacy = PUBLIC_BOARD;
      create.board_public_key = string( bob_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivateboard";
      create.board_privacy = PRIVATE_BOARD;
      create.board_public_key = string( candice_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusiveboard";
      create.board_privacy = EXCLUSIVE_BOARD;
      create.board_public_key = string( dan_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = "aliceopenboard";
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "bob";
      comment.author = "bob";
      comment.board = "bobpublicboard";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "candice";
      comment.author = "candice";
      comment.board = "candiceprivateboard";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "dan";
      comment.author = "dan";
      comment.board = "danexclusiveboard";

      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_BOARD_UPDATE_INTERVAL );

      board_update_operation update;

      update.signatory = "alice";
      update.account = "alice";
      update.board = "aliceopenboard";
      update.board_type = BOARD;
      update.board_privacy = OPEN_BOARD;
      update.board_public_key = string( alice_public_posting_key );
      update.json = "{\"json\":\"valid\"}";
      update.json_private = "{\"json\":\"valid\"}";
      update.details = "updated details";
      update.url = "www.newurl.com";
      update.pinned_author = "alice";
      update.pinned_permlink = "lorem";
      update.validate();

      tx.operations.push_back( update );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "aliceopenboard" );
      BOOST_REQUIRE( board_itr->founder == update.account );
      BOOST_REQUIRE( board_itr->name == update.board );
      BOOST_REQUIRE( board_itr->board_privacy == update.board_privacy );
      BOOST_REQUIRE( board_itr->details == update.details );
      BOOST_REQUIRE( board_itr->url == update.url );
      BOOST_REQUIRE( board_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "bob";
      update.account = "bob";
      update.pinned_author = "bob";
      update.board = "bobpublicboard";
      update.board_privacy = PUBLIC_BOARD;
      update.board_public_key = string( bob_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "bobpublicboard" );
      BOOST_REQUIRE( board_itr->founder == update.account );
      BOOST_REQUIRE( board_itr->name == update.board );
      BOOST_REQUIRE( board_itr->board_privacy == update.board_privacy );
      BOOST_REQUIRE( board_itr->details == update.details );
      BOOST_REQUIRE( board_itr->url == update.url );
      BOOST_REQUIRE( board_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "candice";
      update.account = "candice";
      update.pinned_author = "candice";
      update.board = "candiceprivateboard";
      update.board_privacy = PRIVATE_BOARD;
      update.board_public_key = string( candice_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "candiceprivateboard" );
      BOOST_REQUIRE( board_itr->founder == update.account );
      BOOST_REQUIRE( board_itr->name == update.board );
      BOOST_REQUIRE( board_itr->board_privacy == update.board_privacy );
      BOOST_REQUIRE( board_itr->details == update.details );
      BOOST_REQUIRE( board_itr->url == update.url );
      BOOST_REQUIRE( board_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "dan";
      update.account = "dan";
      update.pinned_author = "dan";
      update.board = "danexclusiveboard";
      update.board_privacy = EXCLUSIVE_BOARD;
      update.board_public_key = string( dan_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "danexclusiveboard" );
      BOOST_REQUIRE( board_itr->founder == update.account );
      BOOST_REQUIRE( board_itr->name == update.board );
      BOOST_REQUIRE( board_itr->board_privacy == update.board_privacy );
      BOOST_REQUIRE( board_itr->details == update.details );
      BOOST_REQUIRE( board_itr->url == update.url );
      BOOST_REQUIRE( board_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder board update sequence" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when board update before MIN_BOARD_UPDATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_BOARD_UPDATE_INTERVAL - BLOCK_INTERVAL );

      update.details = "even more updated details";

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when board update before MIN_BOARD_UPDATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success after MIN_BOARD_UPDATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      auto board_itr = board_idx.find( "danexclusiveboard" );
      BOOST_REQUIRE( board_itr->founder == update.account );
      BOOST_REQUIRE( board_itr->name == update.board );
      BOOST_REQUIRE( board_itr->board_privacy == update.board_privacy );
      BOOST_REQUIRE( board_itr->details == update.details );
      BOOST_REQUIRE( board_itr->url == update.url );
      BOOST_REQUIRE( board_itr->created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success after MIN_BOARD_UPDATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when board name already exists" );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "elon";
      create.founder = "elon";
      create.name = "aliceopenboard";

      tx.operations.push_back( create );
      tx.sign( elon_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when board name already exists" );

      BOOST_TEST_MESSAGE( "├── Passed: BOARD UPDATE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( board_management_sequence_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: BOARD MANAGEMENT OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder invite members" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "margot", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "natalie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      board_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopenboard";
      create.board_type = BOARD;
      create.board_privacy = OPEN_BOARD;
      create.board_public_key = string( alice_public_posting_key );
      create.json = "{\"json\":\"valid\"}";
      create.json_private = "{\"json\":\"valid\"}";
      create.details = "details";
      create.url = "www.url.com";
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& alice_board = db.get_board( "aliceopenboard" );
      const board_member_object& alice_board_member = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( alice_board.founder == create.founder );
      BOOST_REQUIRE( alice_board.board_type == create.board_type );
      BOOST_REQUIRE( alice_board.board_privacy == create.board_privacy );
      BOOST_REQUIRE( alice_board.board_public_key == create.board_public_key );

      BOOST_REQUIRE( alice_board_member.founder == create.founder );
      BOOST_REQUIRE( alice_board_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( alice_board_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( alice_board_member.is_member( create.founder ) );
      BOOST_REQUIRE( alice_board_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpublicboard";
      create.board_privacy = PUBLIC_BOARD;
      create.board_public_key = string( bob_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& bob_board = db.get_board( "bobpublicboard" );
      const board_member_object& bob_board_member = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( bob_board.founder == create.founder );
      BOOST_REQUIRE( bob_board.board_type == create.board_type );
      BOOST_REQUIRE( bob_board.board_privacy == create.board_privacy );
      BOOST_REQUIRE( bob_board.board_public_key == create.board_public_key );

      BOOST_REQUIRE( bob_board_member.founder == create.founder );
      BOOST_REQUIRE( bob_board_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( bob_board_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( bob_board_member.is_member( create.founder ) );
      BOOST_REQUIRE( bob_board_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivateboard";
      create.board_privacy = PRIVATE_BOARD;
      create.board_public_key = string( candice_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& candice_board = db.get_board( "candiceprivateboard" );
      const board_member_object& candice_board_member = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( candice_board.founder == create.founder );
      BOOST_REQUIRE( candice_board.board_type == create.board_type );
      BOOST_REQUIRE( candice_board.board_privacy == create.board_privacy );
      BOOST_REQUIRE( candice_board.board_public_key == create.board_public_key );

      BOOST_REQUIRE( candice_board_member.founder == create.founder );
      BOOST_REQUIRE( candice_board_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( candice_board_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( candice_board_member.is_member( create.founder ) );
      BOOST_REQUIRE( candice_board_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusiveboard";
      create.board_privacy = EXCLUSIVE_BOARD;
      create.board_public_key = string( dan_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& dan_board = db.get_board( "danexclusiveboard" );
      const board_member_object& dan_board_member = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( dan_board.founder == create.founder );
      BOOST_REQUIRE( dan_board.board_type == create.board_type );
      BOOST_REQUIRE( dan_board.board_privacy == create.board_privacy );
      BOOST_REQUIRE( dan_board.board_public_key == create.board_public_key );

      BOOST_REQUIRE( dan_board_member.founder == create.founder );
      BOOST_REQUIRE( dan_board_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( dan_board_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( dan_board_member.is_member( create.founder ) );
      BOOST_REQUIRE( dan_board_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( create );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      board_join_invite_operation invite;

      invite.signatory = "alice";
      invite.account = "alice";
      invite.member = "elon";
      invite.board = "aliceopenboard";
      invite.message = "Hello";
      invite.encrypted_board_key = string( alice_public_posting_key );
      invite.invited = true;
      invite.validate();

      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& invite_idx = db.get_index< board_join_invite_index >().indices().get< by_member_board >();

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "aliceopenboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "bob";
      invite.account = "bob";
      invite.board = "bobpublicboard";
      invite.encrypted_board_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "bobpublicboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "candice";
      invite.account = "candice";
      invite.board = "candiceprivateboard";
      invite.encrypted_board_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "candiceprivateboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "dan";
      invite.account = "dan";
      invite.board = "danexclusiveboard";
      invite.encrypted_board_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "danexclusiveboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "dan";
      invite.account = "dan";
      invite.member = "fred";
      invite.board = "danexclusiveboard";
      invite.encrypted_board_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "fred", "danexclusiveboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder invite members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when non-member sends invites" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.board = "aliceopenboard";
      invite.message = "Hello";
      invite.encrypted_board_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.board = "bobpublicboard";
      invite.encrypted_board_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.board = "candiceprivateboard";
      invite.encrypted_board_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
      invite.board = "danexclusiveboard";
      invite.encrypted_board_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when non-member sends invites" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: board join request" );

      board_join_request_operation request;

      request.signatory = "fred";
      request.account = "fred";
      request.board = "aliceopenboard";
      request.message = "Hello";
      request.requested = true;
      request.validate();

      tx.operations.push_back( request );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& request_idx = db.get_index< board_join_request_index >().indices().get< by_account_board >();

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "aliceopenboard" ) );
      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->board == request.board );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.board = "bobpublicboard";
   
      tx.operations.push_back( request );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "bobpublicboard" ) );
      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->board == request.board );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.board = "candiceprivateboard";
   
      tx.operations.push_back( request );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "candiceprivateboard" ) );
      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->board == request.board );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.board = "danexclusiveboard";
   
      tx.operations.push_back( request );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // No join requests for exclusive board

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "danexclusiveboard" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: board join request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept invite by incoming member" );

      board_invite_accept_operation invite_accept;

      invite_accept.signatory = "elon";
      invite_accept.account = "elon";
      invite_accept.board = "aliceopenboard";
      invite_accept.accepted = true;
      invite_accept.validate();

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "aliceopenboard" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.board = "bobpublicboard";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "bobpublicboard" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.board = "candiceprivateboard";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "candiceprivateboard" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.board = "danexclusiveboard";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "danexclusiveboard" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.signatory = "fred";
      invite_accept.account = "fred";

      tx.operations.push_back( invite_accept );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.is_member( "fred" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "fred", "danexclusiveboard" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept invite by incoming member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder accept join request" );

      board_join_accept_operation join_accept;

      join_accept.signatory = "alice";
      join_accept.account = "alice";
      join_accept.member = "fred";
      join_accept.board = "aliceopenboard";
      join_accept.encrypted_board_key = string( alice_public_posting_key );
      join_accept.accepted = true;
      join_accept.validate();

      tx.operations.push_back( join_accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_member( "fred" ) );
      auto request_itr = request_idx.find( boost::make_tuple( "fred", "aliceopenboard" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      join_accept.signatory = "bob";
      join_accept.account = "bob";
      join_accept.board = "bobpublicboard";
      join_accept.encrypted_board_key = string( bob_public_posting_key );

      tx.operations.push_back( join_accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.is_member( "fred" ) );
      auto request_itr = request_idx.find( boost::make_tuple( "fred", "bobpublicboard" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      join_accept.signatory = "candice";
      join_accept.account = "candice";
      join_accept.board = "candiceprivateboard";
      join_accept.encrypted_board_key = string( candice_public_posting_key );
      
      tx.operations.push_back( join_accept );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.is_member( "fred" ) );
      auto request_itr = request_idx.find( boost::make_tuple( "fred", "candiceprivateboard" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder accept join request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling member sending invites" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.board = "aliceopenboard";
      invite.message = "Hello";
      invite.encrypted_board_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "george", "aliceopenboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.board = "bobpublicboard";
      invite.encrypted_board_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "haz", "bobpublicboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.board = "candiceprivateboard";
      invite.encrypted_board_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
      invite.board = "danexclusiveboard";
      invite.encrypted_board_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling member sending invites" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder add moderator" );

      board_add_mod_operation mod;

      mod.signatory = "alice";
      mod.account = "alice";
      mod.board = "aliceopenboard";
      mod.moderator = "elon";
      mod.added = true;
      mod.validate();

      tx.operations.push_back( mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "bob";
      mod.account = "bob";
      mod.board = "bobpublicboard";

      tx.operations.push_back( mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "candice";
      mod.account = "candice";
      mod.board = "candiceprivateboard";

      tx.operations.push_back( mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "dan";
      mod.account = "dan";
      mod.board = "danexclusiveboard";

      tx.operations.push_back( mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder vote moderator" );

      board_vote_mod_operation vote_mod;

      vote_mod.signatory = "alice";
      vote_mod.account = "alice";
      vote_mod.board = "aliceopenboard";
      vote_mod.moderator = "elon";
      vote_mod.vote_rank = 1;
      vote_mod.approved = true;
      vote_mod.validate();

      tx.operations.push_back( vote_mod );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      flat_map< account_name_type, share_type > m = board_member_a.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "bob";
      vote_mod.account = "bob";
      vote_mod.board = "bobpublicboard";

      tx.operations.push_back( vote_mod );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      m = board_member_b.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "candice";
      vote_mod.account = "candice";
      vote_mod.board = "candiceprivateboard";

      tx.operations.push_back( vote_mod );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      m = board_member_c.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "dan";
      vote_mod.account = "dan";
      vote_mod.board = "danexclusiveboard";

      tx.operations.push_back( vote_mod );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      m = board_member_d.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder vote moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling moderator sending invites and failure when repeated invite" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.board = "aliceopenboard";
      invite.message = "Hello";
      invite.encrypted_board_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.board = "bobpublicboard";
      invite.encrypted_board_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.board = "candiceprivateboard";
      invite.encrypted_board_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "isabelle", "candiceprivateboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
      invite.board = "danexclusiveboard";
      invite.encrypted_board_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling moderator sending invites and failure when repeated invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder add administrator" );

      board_add_admin_operation admin;

      admin.signatory = "alice";
      admin.account = "alice";
      admin.board = "aliceopenboard";
      admin.admin = "elon";
      admin.added = true;
      admin.validate();

      tx.operations.push_back( admin );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "bob";
      admin.account = "bob";
      admin.board = "bobpublicboard";

      tx.operations.push_back( admin );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "candice";
      admin.account = "candice";
      admin.board = "candiceprivateboard";

      tx.operations.push_back( admin );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "dan";
      admin.account = "dan";
      admin.board = "danexclusiveboard";

      tx.operations.push_back( admin );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder add administrator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling administrators sending invites and failure when repeated invite" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.board = "aliceopenboard";
      invite.message = "Hello";
      invite.encrypted_board_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.board = "bobpublicboard";
      invite.encrypted_board_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.board = "candiceprivateboard";
      invite.encrypted_board_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
      invite.board = "danexclusiveboard";
      invite.encrypted_board_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "jayme", "danexclusiveboard" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->board == invite.board );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling moderator sending invites and failure when repeated invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment creation handling before becoming members" );

      comment_operation comment;

      comment.signatory = "george";
      comment.author = "george";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = "aliceopenboard";
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "lorem";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.sign( george_private_posting_key, db.get_chain_id() );       // Non-members can create posts in open board
      db.push_transaction( tx, 0 );

      const comment_object& com = db.get_comment( "george", string( "lorem" ) );
      BOOST_REQUIRE( com.author == "george" );
      BOOST_REQUIRE( com.board == "aliceopenboard" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "haz";
      comment.author = "haz";
      comment.board = "bobpublicboard";

      tx.operations.push_back( comment );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "isabelle";
      comment.author = "isabelle";
      comment.board = "candiceprivateboard";

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "jayme";
      comment.author = "jayme";
      comment.board = "danexclusiveboard";

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "alice";
      comment.author = "alice";
      comment.board = "aliceopenboard";

      tx.operations.push_back( comment );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "bob";
      comment.author = "bob";
      comment.board = "bobpublicboard";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( "bob", string( "lorem" ) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "candice";
      comment.author = "candice";
      comment.board = "candiceprivateboard";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( "candice", string( "lorem" ) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "dan";
      comment.author = "dan";
      comment.board = "danexclusiveboard";

      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& dan_comment = db.get_comment( "dan", string( "lorem" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment creation handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment vote handling before becoming members" );

      vote_operation vote;

      vote.signatory = "george";
      vote.voter = "george";
      vote.author = "alice";
      vote.permlink = "lorem";
      vote.weight = PERCENT_100;
      vote.interface = INIT_ACCOUNT;
      vote.validate();
      
      tx.operations.push_back( vote );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_vote = db.get_comment_vote( "george", alice_comment.id );
      BOOST_REQUIRE( george_vote.voter == vote.voter );
      BOOST_REQUIRE( george_vote.comment == alice_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "haz";
      vote.voter = "haz";
      vote.author = "bob";

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_vote = db.get_comment_vote( "haz", bob_comment.id );
      BOOST_REQUIRE( haz_vote.voter == vote.voter );
      BOOST_REQUIRE( haz_vote.comment == bob_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "isabelle";
      vote.voter = "isabelle";
      vote.author = "candice";

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot vote on posts

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "jayme";
      vote.voter = "jayme";
      vote.author = "dan";

      tx.operations.push_back( vote );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot vote on posts

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment vote handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment view handling before becoming members" );

      view_operation view;

      view.signatory = "george";
      view.viewer = "george";
      view.author = "alice";
      view.permlink = "lorem";
      view.interface = INIT_ACCOUNT;
      view.supernode = INIT_ACCOUNT;
      view.viewed = true;
      view.validate();
      
      tx.operations.push_back( view );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_view = db.get_comment_view( "george", alice_comment.id );
      BOOST_REQUIRE( george_view.viewer == view.viewer );
      BOOST_REQUIRE( george_view.comment == alice_comment.id );
      BOOST_REQUIRE( george_view.supernode == view.supernode );
      BOOST_REQUIRE( george_view.interface == view.interface );
      BOOST_REQUIRE( george_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "haz";
      view.viewer = "haz";
      view.author = "bob";

      tx.operations.push_back( view );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& haz_view = db.get_comment_view( "haz", bob_comment.id );
      BOOST_REQUIRE( haz_view.viewer == view.viewer );
      BOOST_REQUIRE( haz_view.comment == bob_comment.id );
      BOOST_REQUIRE( haz_view.supernode == view.supernode );
      BOOST_REQUIRE( haz_view.interface == view.interface );
      BOOST_REQUIRE( haz_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "isabelle";
      view.viewer = "isabelle";
      view.author = "candice";

      tx.operations.push_back( view );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot view posts

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "jayme";
      view.viewer = "jayme";
      view.author = "dan";

      tx.operations.push_back( view );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot view posts

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment view handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment share handling before becoming members" );

      share_operation share;

      share.signatory = "george";
      share.sharer = "george";
      share.author = "alice";
      share.permlink = "lorem";
      share.reach = FOLLOW_FEED;
      share.interface = INIT_ACCOUNT;
      share.shared = true;
      share.validate();
      
      tx.operations.push_back( share );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_share = db.get_comment_share( "george", alice_comment.id );
      BOOST_REQUIRE( george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_share.comment == alice_comment.id );
      BOOST_REQUIRE( george_share.interface == share.interface );
      BOOST_REQUIRE( george_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "haz";
      share.sharer = "haz";
      share.author = "bob";

      tx.operations.push_back( share );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& haz_share = db.get_comment_share( "haz", bob_comment.id );
      BOOST_REQUIRE( haz_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_share.comment == bob_comment.id );
      BOOST_REQUIRE( haz_share.interface == share.interface );
      BOOST_REQUIRE( haz_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "isabelle";
      share.sharer = "isabelle";
      share.author = "candice";

      tx.operations.push_back( share );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot share posts

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "jayme";
      share.sharer = "jayme";
      share.author = "dan";

      tx.operations.push_back( share );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot share posts

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment share handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment creation handling by members" );

      board_invite_accept_operation accept;

      accept.signatory = "george";
      accept.account = "george";
      accept.board = "aliceopenboard";
      accept.accepted = true;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "haz";
      accept.account = "haz";
      accept.board = "bobpublicboard";

      tx.operations.push_back( accept );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "isabelle";
      accept.account = "isabelle";
      accept.board = "candiceprivateboard";

      tx.operations.push_back( accept );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "jayme";
      accept.account = "jayme";
      accept.board = "danexclusiveboard";

      tx.operations.push_back( accept );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_ROOT_COMMENT_INTERVAL );

      comment_operation comment;

      comment.signatory = "george";
      comment.author = "george";
      comment.board = "aliceopenboard";
      comment.permlink = "ipsum";

      tx.operations.push_back( comment );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& george_comment = db.get_comment( "george", string( "ipsum" ) );
      BOOST_REQUIRE( george_comment.author == "george" );
      BOOST_REQUIRE( george_comment.board == "aliceopenboard" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "haz";
      comment.author = "haz";
      comment.board = "bobpublicboard";

      tx.operations.push_back( comment );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& haz_comment = db.get_comment( "haz", string( "ipsum" ) );
      BOOST_REQUIRE( haz_comment.author == "haz" );
      BOOST_REQUIRE( haz_comment.board == "bobpublicboard" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "isabelle";
      comment.author = "isabelle";
      comment.board = "candiceprivateboard";

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& isabelle_comment = db.get_comment( "isabelle", string( "ipsum" ) );
      BOOST_REQUIRE( isabelle_comment.author == "isabelle" );
      BOOST_REQUIRE( isabelle_comment.board == "candiceprivateboard" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "jayme";
      comment.author = "jayme";
      comment.board = "danexclusiveboard";

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& jayme_comment = db.get_comment( "jayme", string( "ipsum" ) );
      BOOST_REQUIRE( jayme_comment.author == "jayme" );
      BOOST_REQUIRE( jayme_comment.board == "danexclusiveboard" );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment creation handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment vote handling by members" );

      vote.signatory = "george";
      vote.voter = "george";
      vote.author = "george";
      vote.permlink = "ipsum";
      
      tx.operations.push_back( vote );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_vote = db.get_comment_vote( "george", george_comment.id );
      BOOST_REQUIRE( george_vote.voter == vote.voter );
      BOOST_REQUIRE( george_vote.comment == george_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "haz";
      vote.voter = "haz";
      vote.author = "haz";

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_vote = db.get_comment_vote( "haz", haz_comment.id );
      BOOST_REQUIRE( haz_vote.voter == vote.voter );
      BOOST_REQUIRE( haz_vote.comment == haz_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "isabelle";
      vote.voter = "isabelle";
      vote.author = "isabelle";

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& isabelle_vote = db.get_comment_vote( "isabelle", isabelle_comment.id );
      BOOST_REQUIRE( isabelle_vote.voter == vote.voter );
      BOOST_REQUIRE( isabelle_vote.comment == isabelle_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "jayme";
      vote.voter = "jayme";
      vote.author = "jayme";

      tx.operations.push_back( vote );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& jayme_vote = db.get_comment_vote( "jayme", jayme_comment.id );
      BOOST_REQUIRE( jayme_vote.voter == vote.voter );
      BOOST_REQUIRE( jayme_vote.comment == jayme_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment vote handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment view handling by members" );

      view.signatory = "george";
      view.viewer = "george";
      view.author = "george";
      view.permlink = "lorem";

      tx.operations.push_back( view );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_view = db.get_comment_view( "george", george_comment.id );
      BOOST_REQUIRE( george_view.viewer == view.viewer );
      BOOST_REQUIRE( george_view.comment == george_comment.id );
      BOOST_REQUIRE( george_view.supernode == view.supernode );
      BOOST_REQUIRE( george_view.interface == view.interface );
      BOOST_REQUIRE( george_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "haz";
      view.viewer = "haz";
      view.author = "haz";

      tx.operations.push_back( view );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& haz_view = db.get_comment_view( "haz", haz_comment.id );
      BOOST_REQUIRE( haz_view.viewer == view.viewer );
      BOOST_REQUIRE( haz_view.comment == haz_comment.id );
      BOOST_REQUIRE( haz_view.supernode == view.supernode );
      BOOST_REQUIRE( haz_view.interface == view.interface );
      BOOST_REQUIRE( haz_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "isabelle";
      view.viewer = "isabelle";
      view.author = "isabelle";

      tx.operations.push_back( view );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& isabelle_view = db.get_comment_view( "isabelle", isabelle_comment.id );
      BOOST_REQUIRE( isabelle_view.viewer == view.viewer );
      BOOST_REQUIRE( isabelle_view.comment == isabelle_comment.id );
      BOOST_REQUIRE( isabelle_view.supernode == view.supernode );
      BOOST_REQUIRE( isabelle_view.interface == view.interface );
      BOOST_REQUIRE( isabelle_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "jayme";
      view.viewer = "jayme";
      view.author = "jayme";

      tx.operations.push_back( view );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& jayme_view = db.get_comment_view( "jayme", jayme_comment.id );
      BOOST_REQUIRE( jayme_view.viewer == view.viewer );
      BOOST_REQUIRE( jayme_view.comment == jayme_comment.id );
      BOOST_REQUIRE( jayme_view.supernode == view.supernode );
      BOOST_REQUIRE( jayme_view.interface == view.interface );
      BOOST_REQUIRE( jayme_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment view handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment share handling by members" );

      share_operation share;

      share.signatory = "george";
      share.sharer = "george";
      share.author = "george";
      share.permlink = "ipsum";
      
      tx.operations.push_back( share );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_share = db.get_comment_share( "george", george_comment.id );
      BOOST_REQUIRE( george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_share.comment == george_comment.id );
      BOOST_REQUIRE( george_share.interface == share.interface );
      BOOST_REQUIRE( george_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "haz";
      share.sharer = "haz";
      share.author = "haz";

      tx.operations.push_back( share );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& haz_share = db.get_comment_share( "haz", haz_comment.id );
      BOOST_REQUIRE( haz_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_share.comment == haz_comment.id );
      BOOST_REQUIRE( haz_share.interface == share.interface );
      BOOST_REQUIRE( haz_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "isabelle";
      share.sharer = "isabelle";
      share.author = "isabelle";

      tx.operations.push_back( share );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& isabelle_share = db.get_comment_share( "isabelle", isabelle_comment.id );
      BOOST_REQUIRE( isabelle_share.sharer == share.sharer );
      BOOST_REQUIRE( isabelle_share.comment == isabelle_comment.id );
      BOOST_REQUIRE( isabelle_share.interface == share.interface );
      BOOST_REQUIRE( isabelle_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "jayme";
      share.sharer = "jayme";
      share.author = "jayme";

      tx.operations.push_back( share );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_share = db.get_comment_share( "george", george_comment.id );
      BOOST_REQUIRE( george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_share.comment == george_comment.id );
      BOOST_REQUIRE( george_share.interface == share.interface );
      BOOST_REQUIRE( george_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment share handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: moderator tag by board moderator" );

      moderation_tag_operation tag;

      tag.signatory = "elon";
      tag.moderator = "elon";
      tag.author = "george";
      tag.permlink = "ipsum";
      tag.tags.push_back( "nsfw" );
      tag.rating = MATURE;
      tag.details = "Post is NSFW";
      tag.interface = INIT_ACCOUNT;
      tag.filter = false;
      tag.applied = true;
      tag.validate();

      tx.operations.push_back( mod );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& tag_idx = db.get_index< moderation_tag_index >().indices().get< by_comment_moderator >();

      auto tag_itr = tag_idx.find( std::make_tuple( george_comment.id, "elon" ) );
      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( tag_itr->details == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = "haz";

      tx.operations.push_back( mod );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto tag_itr = tag_idx.find( std::make_tuple( haz_comment.id, "elon" ) );
      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( tag_itr->details == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = "isabelle";

      tx.operations.push_back( mod );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto tag_itr = tag_idx.find( std::make_tuple( isabelle_comment.id, "elon" ) );
      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( tag_itr->details == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = "jayme";

      tx.operations.push_back( mod );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto tag_itr = tag_idx.find( std::make_tuple( jayme_comment.id, "elon" ) );
      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( tag_itr->details == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: moderator tag by board moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator add moderator" );

      board_add_mod_operation mod;

      mod.signatory = "elon";
      mod.account = "elon";
      mod.board = "aliceopenboard";
      mod.moderator = "fred";
      mod.added = true;
      mod.validate();

      tx.operations.push_back( mod );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.board = "bobpublicboard";

      tx.operations.push_back( mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.board = "candiceprivateboard";

      tx.operations.push_back( mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.board = "danexclusiveboard";

      tx.operations.push_back( mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator update board" );

      board_update_operation update;

      update.signatory = "elon";
      update.account = "elon";
      update.board = "aliceopenboard";
      update.board_type = BOARD;
      update.board_privacy = OPEN_BOARD;
      update.board_public_key = string( alice_public_posting_key );
      update.json = "{\"json\":\"valid\"}";
      update.json_private = "{\"json\":\"valid\"}";
      update.details = "updated details";
      update.url = "www.newurl.com";
      update.pinned_author = "alice";
      update.pinned_permlink = "lorem";
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& alice_board = db.get_board( "aliceopenboard" );
      BOOST_REQUIRE( alice_board.founder == update.account );
      BOOST_REQUIRE( alice_board.name == update.board );
      BOOST_REQUIRE( alice_board.board_privacy == update.board_privacy );
      BOOST_REQUIRE( alice_board.details == update.details );
      BOOST_REQUIRE( alice_board.url == update.url );
      BOOST_REQUIRE( alice_board.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = "bob";
      update.board = "bobpublicboard";
      update.board_privacy = PUBLIC_BOARD;
      update.board_public_key = string( bob_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& bob_board = db.get_board( "bobpublicboard" );
      BOOST_REQUIRE( bob_board.founder == update.account );
      BOOST_REQUIRE( bob_board.name == update.board );
      BOOST_REQUIRE( bob_board.board_privacy == update.board_privacy );
      BOOST_REQUIRE( bob_board.details == update.details );
      BOOST_REQUIRE( bob_board.url == update.url );
      BOOST_REQUIRE( bob_board.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = "candice";
      update.board = "candiceprivateboard";
      update.board_privacy = PRIVATE_BOARD;
      update.board_public_key = string( candice_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& candice_board = db.get_board( "candiceprivateboard" );
      BOOST_REQUIRE( candice_board.founder == update.account );
      BOOST_REQUIRE( candice_board.name == update.board );
      BOOST_REQUIRE( candice_board.board_privacy == update.board_privacy );
      BOOST_REQUIRE( candice_board.details == update.details );
      BOOST_REQUIRE( candice_board.url == update.url );
      BOOST_REQUIRE( candice_board.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = "dan";
      update.board = "danexclusiveboard";
      update.board_privacy = EXCLUSIVE_BOARD;
      update.board_public_key = string( dan_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_object& dan_board = db.get_board( "danexclusiveboard" );
      BOOST_REQUIRE( dan_board.founder == update.account );
      BOOST_REQUIRE( dan_board.name == update.board );
      BOOST_REQUIRE( dan_board.board_privacy == update.board_privacy );
      BOOST_REQUIRE( dan_board.details == update.details );
      BOOST_REQUIRE( dan_board.url == update.url );
      BOOST_REQUIRE( dan_board.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator update board" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove moderator" );

      board_add_mod_operation mod;

      mod.signatory = "alice";
      mod.account = "alice";
      mod.board = "aliceopenboard";
      mod.moderator = "fred";
      mod.added = false;
      mod.validate();

      tx.operations.push_back( mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !board_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "bob";
      mod.account = "bob";
      mod.board = "bobpublicboard";

      tx.operations.push_back( mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !board_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "candice";
      mod.account = "candice";
      mod.board = "candiceprivateboard";

      tx.operations.push_back( mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !board_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "dan";
      mod.account = "dan";
      mod.board = "danexclusiveboard";

      tx.operations.push_back( mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !board_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove member" );

      board_remove_member_operation remove;

      remove.signatory = "alice";
      remove.account = "alice";
      remove.board = "aliceopenboard";
      remove.member = "fred";
      remove.validate();

      tx.operations.push_back( remove );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( !board_member_a.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "bob";
      remove.account = "bob";
      remove.board = "bobpublicboard";

      tx.operations.push_back( remove );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( !board_member_b.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "candice";
      remove.account = "candice";
      remove.board = "candiceprivateboard";

      tx.operations.push_back( remove );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( !board_member_c.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "dan";
      remove.account = "dan";
      remove.board = "danexclusiveboard";

      tx.operations.push_back( remove );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( !board_member_d.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: board subscription" );

      board_subscribe_operation subscribe;

      subscribe.signatory = "elon";
      subscribe.account = "elon";
      subscribe.board = "aliceopenboard";
      subscribe.interface = INIT_ACCOUNT;
      subscribe.added = true;
      subscribe.subscribed = true;
      subscribe.validate();

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.board = "bobpublicboard";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.board = "candiceprivateboard";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.board = "danexclusiveboard";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: board subscription" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder blacklist account" );

      board_blacklist_operation blacklist;

      blacklist.signatory = "alice";
      blacklist.account = "alice";
      blacklist.member = "fred";
      blacklist.board = "aliceopenboard";
      blacklist.validate();

      tx.operations.push_back( blacklist );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "bob";
      blacklist.account = "bob";
      blacklist.board = "bobpublicboard";

      tx.operations.push_back( blacklist );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "candice";
      blacklist.account = "candice";
      blacklist.board = "candiceprivateboard";

      tx.operations.push_back( blacklist );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "dan";
      blacklist.account = "dan";
      blacklist.board = "danexclusiveboard";

      tx.operations.push_back( blacklist );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder blacklist account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: transfer board ownership" );

      board_transfer_ownership_operation transfer;

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.board = "aliceopenboard";
      transfer.new_founder = "elon";
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_a = db.get_board_member( "aliceopenboard" );
      BOOST_REQUIRE( board_member_a.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "bob";
      transfer.account = "bob";
      transfer.board = "bobpublicboard";

      tx.operations.push_back( transfer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_b = db.get_board_member( "bobpublicboard" );
      BOOST_REQUIRE( board_member_b.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "candice";
      transfer.account = "candice";
      transfer.board = "candiceprivateboard";

      tx.operations.push_back( transfer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_c = db.get_board_member( "candiceprivateboard" );
      BOOST_REQUIRE( board_member_c.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "dan";
      transfer.account = "dan";
      transfer.board = "danexclusiveboard";

      tx.operations.push_back( transfer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const board_member_object& board_member_d = db.get_board_member( "danexclusiveboard" );
      BOOST_REQUIRE( board_member_d.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: transfer board ownership" );

      BOOST_TEST_MESSAGE( "├── Passed: BOARD MANAGEMENT OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



   //================================//
   // === Advertising Operations === //
   //================================//



   BOOST_AUTO_TEST_CASE( ad_operation_sequence_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: AD OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful ad creative creation" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "adcreativepermlink";
      comment.title = "My Creative post";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.board = INIT_BOARD;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "adcreativepermlink";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = ARTICLE_POST;
      options.privacy = false;
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      ad_creative_operation creative;

      creative.signatory = "alice";
      creative.author = "alice";
      creative.format_type = STANDARD_FORMAT;
      creative.creative_id = "8638f626-7c6e-4440-9a67-43ab48939870";
      creative.objective = "creativepermlink";
      creative.creative = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      creative.json = "{\"json\":\"valid\"}";
      creative.active = true;
      creative.validate();

      tx.operations.push_back( creative );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_creative_object& alice_creative = db.get_ad_creative( "alice", "8638f626-7c6e-4440-9a67-43ab48939870" );
      BOOST_REQUIRE( alice_creative.objective == creative.objective );
      BOOST_REQUIRE( alice_creative.creative == creative.creative );
      BOOST_REQUIRE( alice_creative.format_type == creative.format_type );
      BOOST_REQUIRE( alice_creative.last_updated == now() );
      BOOST_REQUIRE( alice_creative.created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful ad creative creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad campaign creation" );

      ad_campaign_operation campaign;

      campaign.signatory = "alice";
      campaign.account = "alice";
      campaign.campaign_id = "da89d680-e9c4-4ae0-95e5-1f47bd1526a0";
      campaign.budget = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      campaign.begin = now();
      campaign.end = ( now() + fc::days(7) );
      campaign.json = "{\"json\":\"valid\"}";
      campaign.interface = INIT_ACCOUNT;
      campaign.active = true;
      campaign.validate();

      tx.operations.push_back( campaign );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_campaign_object& alice_campaign = db.get_ad_campaign( "alice", "da89d680-e9c4-4ae0-95e5-1f47bd1526a0" );
      BOOST_REQUIRE( alice_campaign.budget == campaign.budget );
      BOOST_REQUIRE( alice_campaign.begin == campaign.begin );
      BOOST_REQUIRE( alice_campaign.end == campaign.end );
      BOOST_REQUIRE( alice_campaign.last_updated == now() );
      BOOST_REQUIRE( alice_campaign.created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad campaign creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad audience creation" );

      ad_audience_operation audience;

      audience.signatory = "bob";
      audience.account = "bob";
      audience.audience_id = "0ffe6be9-dcf8-436e-9296-49c83e3d0786";
      audience.json = "{\"json\":\"valid\"}";
      audience.audience.push_back( "candice" );
      audience.audience.push_back( "dan" );
      audience.audience.push_back( "elon" );
      audience.active = true;
      audience.validate();

      tx.operations.push_back( audience );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_audience_object& bob_audience = db.get_ad_audience( "bob", "0ffe6be9-dcf8-436e-9296-49c83e3d0786" );
      for( auto a : audience.audience )
      {
         BOOST_REQUIRE( bob_audience.is_audience( a ) );
      }
      BOOST_REQUIRE( bob_audience.last_updated == now() );
      BOOST_REQUIRE( bob_audience.created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad audience creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creating inventory without interface" );

      ad_inventory_operation inventory;

      inventory.signatory = "bob";
      inventory.provider = "bob";
      inventory.inventory_id = "19ebee83-fc57-404b-a85e-aa8e7f6bbb66";
      inventory.audience_id = "0ffe6be9-dcf8-436e-9296-49c83e3d0786";
      inventory.metric = VIEW_METRIC;
      inventory.json = "{\"json\":\"valid\"}";
      inventory.min_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      inventory.inventory = 100;
      inventory.active = true;
      inventory.validate();

      tx.operations.push_back( inventory );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Inventory requires an interface

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when creating inventory without interface" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad inventory creation" );

      update_interface_operation interface;
      
      interface.signatory = "bob";
      interface.account = "bob";
      interface.details = "details";
      interface.url = "www.url.com";
      interface.json = "{\"json\":\"valid\"}";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      ad_inventory_operation inventory;

      inventory.signatory = "bob";
      inventory.provider = "bob";
      inventory.inventory_id = "19ebee83-fc57-404b-a85e-aa8e7f6bbb66";
      inventory.audience_id = "0ffe6be9-dcf8-436e-9296-49c83e3d0786";
      inventory.metric = VIEW_METRIC;
      inventory.json = "{\"json\":\"valid\"}";
      inventory.min_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      inventory.inventory = 100;
      inventory.active = true;
      inventory.validate();

      tx.operations.push_back( inventory );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_inventory_object& bob_inventory = db.get_ad_inventory( "bob", "19ebee83-fc57-404b-a85e-aa8e7f6bbb66" );
      BOOST_REQUIRE( bob_inventory.min_price == inventory.min_price );
      BOOST_REQUIRE( bob_inventory.metric == inventory.metric );
      BOOST_REQUIRE( bob_inventory.inventory == inventory.inventory );
      BOOST_REQUIRE( bob_inventory.remaining == inventory.inventory );
      BOOST_REQUIRE( bob_inventory.last_updated == now() );
      BOOST_REQUIRE( bob_inventory.created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad inventory creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad bid creation" );

      ad_bid_operation bid;

      bid.signatory = "alice";
      bid.bidder = "alice";
      bid.bid_id = "28bdc74a-097a-40d4-bf49-cc95af3eeec0";
      bid.account = "alice";
      bid.campaign_id = "da89d680-e9c4-4ae0-95e5-1f47bd1526a0";
      bid.creative_id = "8638f626-7c6e-4440-9a67-43ab48939870";
      bid.provider = "bob";
      bid.inventory_id = "19ebee83-fc57-404b-a85e-aa8e7f6bbb66";
      bid.bid_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.requested = 101;
      bid.json = "{\"json\":\"valid\"}";
      bid.expiration = ( now() + fc::days(30) );
      bid.active = true;
      bid.validate();

      tx.operations.push_back( bid );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_bid_object& alice_bid = db.get_ad_bid( "alice", "19ebee83-fc57-404b-a85e-aa8e7f6bbb66" );
      BOOST_REQUIRE( alice_bid.bid_price == bid.bid_price );
      BOOST_REQUIRE( alice_bid.campaign_id == bid.campaign_id );
      BOOST_REQUIRE( alice_bid.creative_id == bid.creative_id );
      BOOST_REQUIRE( alice_bid.inventory_id == bid.inventory_id );
      BOOST_REQUIRE( alice_bid.account == bid.account );
      BOOST_REQUIRE( alice_bid.requested == bid.requested );
      BOOST_REQUIRE( alice_bid.remaining == bid.requested );
      BOOST_REQUIRE( alice_bid.last_updated == now() );
      BOOST_REQUIRE( alice_bid.created == now() );
      BOOST_REQUIRE( alice_bid.expiration == bid.expiration );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad bid creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when ad delivery contains no accounts in audience" );

      account_create_operation create;

      create.signatory = "alice";
      create.registrar = "alice";
      create.new_account_name = "newuser";
      create.account_type = PERSONA;
      create.owner = authority( 1, alice_public_owner_key, 1 );
      create.active = authority( 2, alice_public_active_key, 2 );
      create.posting = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();

      for( auto i = 0; i < 100; i++ )
      {
         create.new_account_name = "newuser"+fc::to_string( i );
         audience.audience.push_back( create.new_account_name );

         tx.operations.push_back( create );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();
      }

      ad_deliver_operation deliver;

      deliver.signatory = "bob";
      deliver.account = "bob";
      deliver.bidder = "alice";
      deliver.bid_id = "28bdc74a-097a-40d4-bf49-cc95af3eeec0";
      deliver.delivery_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
   
      view_operation view;

      view.signatory = "newuser";
      view.viewer = "newuser";
      view.author = "alice";
      view.permlink = "creativepermlink";
      view.supernode = INIT_ACCOUNT;
      view.interface = "bob";
      view.validate();
      
      for( auto i = 0; i < 50; i++ )
      {
         view.signatory = "newuser"+fc::to_string( i );
         view.viewer = "newuser"+fc::to_string( i );

         tx.operations.push_back( view );
         tx.sign( alice_private_posting_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
         deliver.transactions.push_back( tx.id() );      // Add transaction IDs to deliver operation

         tx.operations.clear();
         tx.signatures.clear();
      }

      deliver.validate();
      tx.operations.push_back( deliver );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Accounts are not in audience

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when ad delivery contains no accounts in audience" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad delivery" );

      tx.operations.push_back( audience );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );    // Update audience to include new accounts

      tx.operations.clear();
      tx.signatures.clear();

      const ad_audience_object& bob_audience = db.get_ad_audience( "bob", "0ffe6be9-dcf8-436e-9296-49c83e3d0786" );
      for( auto a : audience.audience )
      {
         BOOST_REQUIRE( bob_audience.is_audience( a ) );
      }
      BOOST_REQUIRE( bob_audience.audience.size() == 103 );
      BOOST_REQUIRE( bob_audience.last_updated == now() );
      BOOST_REQUIRE( bob_audience.created == now() );

      bid.requested = 100;

      tx.operations.push_back( bid );    // Update bid to include new audience accounts
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      deliver.validate();
      tx.operations.push_back( deliver );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      deliver.transactions.clear();

      const ad_bid_object& alice_bid = db.get_ad_bid( "alice", "28bdc74a-097a-40d4-bf49-cc95af3eeec0" );

      BOOST_REQUIRE( alice_bid.bid_price == bid.bid_price );
      BOOST_REQUIRE( alice_bid.campaign_id == bid.campaign_id );
      BOOST_REQUIRE( alice_bid.creative_id == bid.creative_id );
      BOOST_REQUIRE( alice_bid.inventory_id == bid.inventory_id );
      BOOST_REQUIRE( alice_bid.account == bid.account );
      BOOST_REQUIRE( alice_bid.requested == bid.requested );
      BOOST_REQUIRE( alice_bid.remaining == 50 );
      BOOST_REQUIRE( alice_bid.last_updated == now() );
      BOOST_REQUIRE( alice_bid.expiration == bid.expiration );

      const ad_inventory_object& bob_inventory = db.get_ad_inventory( "bob", "19ebee83-fc57-404b-a85e-aa8e7f6bbb66" );
      
      BOOST_REQUIRE( bob_inventory.remaining == 50 );
      BOOST_REQUIRE( bob_inventory.last_updated == now() );

      const ad_campaign_object& alice_campaign = db.get_ad_campaign( "alice", "da89d680-e9c4-4ae0-95e5-1f47bd1526a0" );
      
      BOOST_REQUIRE( alice_campaign.budget == asset( 50*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      BOOST_REQUIRE( alice_campaign.last_updated == now() );
      BOOST_REQUIRE( alice_campaign.created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad delivery" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: ad delivery bid completion" );
      
      for( auto i = 50; i < 100; i++ )
      {
         view.signatory = "newuser"+fc::to_string( i );
         view.viewer = "newuser"+fc::to_string( i );

         tx.operations.push_back( view );
         tx.sign( alice_private_posting_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
         deliver.transactions.push_back( tx.id() );      // Add transaction IDs to deliver operation

         tx.operations.clear();
         tx.signatures.clear();
      }

      deliver.validate();
      tx.operations.push_back( deliver );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& bid_idx = db.get_index< ad_bid_index >().indices().get< by_bid_id >();
      auto bid_itr = bid_idx.find( boost::make_tuple( "alice", "28bdc74a-097a-40d4-bf49-cc95af3eeec0" ) );

      const auto& cam_idx = db.get_index< ad_campaign_index >().indices().get< by_campaign_id >();
      auto cam_itr = cam_idx.find( boost::make_tuple( "alice", "da89d680-e9c4-4ae0-95e5-1f47bd1526a0" ) );

      const auto& inv_idx = db.get_index< ad_inventory_index >().indices().get< by_inventory_id >();
      auto inv_itr = inv_idx.find( boost::make_tuple( "bob", "19ebee83-fc57-404b-a85e-aa8e7f6bbb66" ) );

      BOOST_REQUIRE( bid_itr == bid_idx.end() );
      BOOST_REQUIRE( cam_itr == cam_idx.end() );
      BOOST_REQUIRE( inv_itr == inv_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: ad delivery bid completion" );

      BOOST_TEST_MESSAGE( "├── Passed: AD OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}



   //=============================//
   // === Transfer Operations === //
   //=============================//

   /**
    * transfer_operation,
      transfer_request_operation,
      transfer_accept_operation,
      transfer_recurring_operation,
      transfer_recurring_request_operation,
      transfer_recurring_accept_operation,
      */


BOOST_AUTO_TEST_CASE( transfer_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful transfer" );

      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      ACTORS( (alice)(bob)(candice)(dan)(elon)(corp) );

      fund_stake( "alice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_init_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );

      signed_transaction tx;

      transfer_operation transfer;

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset bob_liquid_balance = db.get_liquid_balance( "bob", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( bob_liquid_balance == bob_init_liquid_balance + transfer.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful transfer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signatures" );

      tx.operations.push_back( transfer );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by a signature not in the account's active authority" );

      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by a signature not in the account's active authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by an additional signature not in the creator's authority" );
      
      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by an additional signature not in the creator's authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when multi-sig signed by insufficient threshold" );

      // Alice, Bob and candice all have 2-of-3 multisig on corp.
      // Legitimate tx signed by (Alice, Bob) goes through.
      // Sam shouldn't be able to add or remove signatures to get the transaction to process multiple times.

      account_update_operation update_op;

      update_op.account = "corp";
      update_op.active = authority( 2, "alice", 1, "bob", 1, "candice", 1 );

      tx.operations.push_back( update_op );
      tx.sign( corp_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      transfer_operation transfer;

      transfer.signatory = "corp";
      transfer.from = "corp";
      transfer.to = "candice";

      tx.operations.push_back( transfer );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      signature_type alice_sig = tx.signatures.back();

      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when multi-sig signed by insufficient threshold" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when multi-sig signed by too many signatures" );

      tx.sign( bob_private_owner_key, db.get_chain_id() );
      signature_type bob_sig = tx.signatures.back();

      tx.sign( candice_private_active_key, db.get_chain_id() );
      signature_type candice_sig = tx.signatures.back();
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when multi-sig signed by too many signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success when multi-sig signed by 2 of 3 keys" );

      tx.signatures.clear();
      tx.signatures.push_back( alice_sig );
      tx.signatures.push_back( bob_sig );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: success when multi-sig signed by 2 of 3 keys" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when multi-sig signature is reused from previous operation" );

      tx.signatures.clear();
      tx.signatures.push_back( alice_sig );
      tx.signatures.push_back( candice_sig );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when multi-sig signature is reused from previous operation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: larger transfer amount send" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_init_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance + transfer.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: larger transfer amount send" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when sending greater than liquid balance" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_init_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = asset( 10000000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when sending greater than liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sending exactly entire liquid balance" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_init_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = alice_init_liquid_balance;
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_liquid_balance == alice_init_liquid_balance - transfer.amount );
      BOOST_REQUIRE( alice_liquid_balance.amount == 0 );
      BOOST_REQUIRE( candice_liquid_balance == candice_init_liquid_balance + transfer.amount );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: sending exactly entire liquid balance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure transferring with zero liquid balance" );

      asset alice_init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_COIN );
      asset candice_init_liquid_balance = db.get_liquid_balance( "candice", SYMBOL_COIN );

      transfer.signatory = "alice";
      transfer.from = "alice";
      transfer.to = "candice";
      transfer.amount = asset( 1 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();
      
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure transferring with zero liquid balance" );

      BOOST_TEST_MESSAGE( "├── Testing: TRANSFER OPERATION" );
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

      comment_operation comment;
      comment.author = "alice";
      comment.permlink = "foo";
      comment.parent_permlink = "test";
      comment.title = "bar";
      comment.body = "foo bar";
      tx.operations.push_back( comment );
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
      ACTORS( (sam)(dan) )
      generate_block();

      fund_stake( "sam", ASSET( "1000.000 TESTS" ) );

      generate_block();

      auto samSCORE = db.get_account( "sam" ).SCORE;

      BOOST_TEST_MESSAGE( "--- Test failure when delegating 0 VESTS" );
      tx.clear();
      op.delegator = "sam";
      op.delegatee = "dan";
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
      BOOST_REQUIRE( db.get_account( "dan" ).SCOREreceived == ASSET( "0.000000 VESTS" ) );
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
      op.who = "dan";
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
      ACTORS( (alice)(bob)(sam)(dan) )
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
      op.to = "dan";
      op.agent = "sam";
      op.who = "dan";
      op.approve = true;

      tx.operations.push_back( op );
      tx.sign( dan_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when agent does not match escrow" );
      op.to = "bob";
      op.agent = "dan";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( op );
      tx.sign( dan_private_key, db.get_chain_id() );
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

      ACTORS( (alice)(bob)(sam)(dan) )
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

      op.to = "dan";
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
      op.agent = "dan";
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

      ACTORS( (alice)(bob)(sam)(dan) )
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
      op.receiver = "dan";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when other attempts to release non-disputed escrow to 'to'" );
      op.receiver = et_op.to;
      op.who = "dan";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( dan_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE("--- failure when other attempts to release non-disputed escrow to 'from' " );
      op.receiver = et_op.from;

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( dan_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when other attempt to release non-disputed escrow to not 'to' or 'from'" );
      op.receiver = "dan";

      tx.clear();
      tx.operations.push_back( op );
      tx.sign( dan_private_key, db.get_chain_id() );
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
      op.receiver = "dan";

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
      op.receiver = "dan";

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
      op.receiver = "dan";
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );


      BOOST_TEST_MESSAGE( "--- failure when agent does not match escrow" );
      tx.clear();
      op.who = "dan";
      op.receiver = et_op.from;
      tx.operations.push_back( op );
      tx.sign( dan_private_key, db.get_chain_id() );
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
      op.receiver = "dan";
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
      op.receiver = "dan";
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
      op.receiver = "dan";
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
      op.url = "www.url.com";
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
      op.url = "www.url.com";
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
      op.url = "www.newurl.com";
      tx.operations.push_back( op );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

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
