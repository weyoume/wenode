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

BOOST_FIXTURE_TEST_SUITE( account_operation_tests, clean_database_fixture );


   //=======================//
   // === Account Tests === //
   //=======================//


BOOST_AUTO_TEST_CASE( account_create_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT CREATE" );

      account_create_operation create;

      create.signatory = account_name_type( "alice" );
      create.registrar = account_name_type( "alice" );
      create.new_account_name = account_name_type( "bob" );

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      BOOST_TEST_MESSAGE( "│   ├── Testing: owner authority" );

      expected.insert( account_name_type( "alice" ) );
      create.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      BOOST_TEST_MESSAGE( "│   ├── Passed: owner authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: active authority" );

      expected.clear();
      auths.clear();
      create.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      BOOST_TEST_MESSAGE( "│   ├── Passed: active authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting authority" );

      create.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Posting authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Account creation" );

      signed_transaction tx;
   
      asset init_starting_balance = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      fc::ecc::private_key alice_private_owner_key = get_private_key( "alice", "owner", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key alice_private_active_key = get_private_key( "alice", "active", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key alice_private_posting_key = get_private_key( "alice", "posting", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key alice_private_secure_key = get_private_key( "alice", "secure", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key alice_private_connection_key = get_private_key( "alice", "connection", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key alice_private_friend_key = get_private_key( "alice", "friend", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key alice_private_companion_key = get_private_key( "alice", "companion", INIT_ACCOUNT_PASSWORD );

      public_key_type alice_public_owner_key = alice_private_owner_key.get_public_key();
      public_key_type alice_public_active_key = alice_private_active_key.get_public_key();
      public_key_type alice_public_posting_key = alice_private_posting_key.get_public_key();
      public_key_type alice_public_secure_key = alice_private_secure_key.get_public_key();
      public_key_type alice_public_connection_key = alice_private_connection_key.get_public_key();
      public_key_type alice_public_friend_key = alice_private_friend_key.get_public_key();
      public_key_type alice_public_companion_key = alice_private_companion_key.get_public_key();
      
      fund( INIT_ACCOUNT, init_starting_balance );

      asset init_liquid_balance = db.get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );

      create.signatory = INIT_ACCOUNT;
      create.registrar = INIT_ACCOUNT;
      create.new_account_name = account_name_type( "alice" );
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#{\"cookie_price\":\"3.50000000 MUSD\"}" ) );
      create.first_name = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#Alice" ) );
      create.last_name = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#Smith" ) );
      create.gender = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#Female" ) );
      create.date_of_birth = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#14-03-1980" ) );
      create.email = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#alice@gmail.com" ) );
      create.phone = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#0400111222" ) );
      create.nationality = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#Australia" ) );
      create.owner_auth = authority( 1, alice_public_owner_key, 1 );
      create.active_auth = authority( 2, alice_public_active_key, 2 );
      create.posting_auth = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_secure_key );
      create.connection_public_key = string( alice_public_connection_key );
      create.friend_public_key = string( alice_public_friend_key );
      create.companion_public_key = string( alice_public_companion_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( init_account_private_owner_key, db.get_chain_id() );
      tx.validate();
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      const account_object& alice = db.get_account( account_name_type( "alice" ) );
      const account_authority_object& alice_auth = db.get< account_authority_object, by_account >( account_name_type( alice.name ) );
      const account_following_object& alice_follow = db.get_account_following( account_name_type( alice.name ) );

      BOOST_REQUIRE( alice.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( alice.name == create.new_account_name );
      BOOST_REQUIRE( alice_follow.account == create.new_account_name );
      BOOST_REQUIRE( alice.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( alice.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( alice.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( alice.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( alice.created == now() );
      BOOST_REQUIRE( alice.last_updated == now() );
      BOOST_REQUIRE( alice_auth.owner_auth == authority( 1, alice_public_owner_key, 1 ) );
      BOOST_REQUIRE( alice_auth.active_auth == authority( 2, alice_public_active_key, 2 ) );
      BOOST_REQUIRE( alice_auth.posting_auth == authority( 1, alice_public_posting_key, 1 ) );
      BOOST_REQUIRE( alice.secure_public_key == alice_public_secure_key );
      BOOST_REQUIRE( alice.connection_public_key == alice_public_connection_key );
      BOOST_REQUIRE( alice.friend_public_key == alice_public_friend_key );
      BOOST_REQUIRE( alice.companion_public_key == alice_public_companion_key );
      BOOST_REQUIRE( alice.id._id == alice_auth.id._id );
      BOOST_REQUIRE( alice.id._id == alice_follow.id._id );
      
      BOOST_REQUIRE( db.get_staked_balance( alice.name, SYMBOL_COIN ) == create.fee );

      asset init_liquid = db.get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );
      
      BOOST_REQUIRE( ( init_liquid_balance - create.fee ) == init_liquid );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Business Account creation" );

      fc::ecc::private_key ionstudios_private_owner_key = get_private_key( "ionstudios", "owner", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key ionstudios_private_active_key = get_private_key( "ionstudios", "active", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key ionstudios_private_posting_key = get_private_key( "ionstudios", "posting", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key ionstudios_private_secure_key = get_private_key( "ionstudios", "secure", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key ionstudios_private_connection_key = get_private_key( "ionstudios", "connection", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key ionstudios_private_friend_key = get_private_key( "ionstudios", "friend", INIT_ACCOUNT_PASSWORD );
      fc::ecc::private_key ionstudios_private_companion_key = get_private_key( "ionstudios", "companion", INIT_ACCOUNT_PASSWORD );

      public_key_type ionstudios_public_owner_key = ionstudios_private_owner_key.get_public_key();
      public_key_type ionstudios_public_active_key = ionstudios_private_active_key.get_public_key();
      public_key_type ionstudios_public_posting_key = ionstudios_private_posting_key.get_public_key();
      public_key_type ionstudios_public_secure_key = ionstudios_private_secure_key.get_public_key();
      public_key_type ionstudios_public_connection_key = ionstudios_private_connection_key.get_public_key();
      public_key_type ionstudios_public_friend_key = ionstudios_private_friend_key.get_public_key();
      public_key_type ionstudios_public_companion_key = ionstudios_private_companion_key.get_public_key();

      create.signatory = INIT_ACCOUNT;
      create.registrar = INIT_ACCOUNT;
      create.new_account_name = account_name_type( "ionstudios" );
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: Ion Studios Incorporated.";
      create.url = "https://ion.io";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#{\"cookie_price\":\"3.50000000 MUSD\"}" ) );
      create.first_name = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#Ionstudios" ) );
      create.last_name = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#Inc." ) );
      create.gender = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#Female" ) );
      create.date_of_birth = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#14-03-1980" ) );
      create.email = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#ionstudios@gmail.com" ) );
      create.phone = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#0400111222" ) );
      create.nationality = get_encrypted_message( ionstudios_private_secure_key, ionstudios_public_secure_key, ionstudios_public_connection_key, string( "#Australia" ) );
      create.owner_auth = authority( 1, ionstudios_public_owner_key, 1 );
      create.active_auth = authority( 2, ionstudios_public_active_key, 2 );
      create.posting_auth = authority( 1, ionstudios_public_posting_key, 1 );
      create.secure_public_key = string( ionstudios_public_secure_key );
      create.connection_public_key = string( ionstudios_public_connection_key );
      create.friend_public_key = string( ionstudios_public_friend_key );
      create.companion_public_key = string( ionstudios_public_companion_key );
      create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      tx.operations.push_back( create );
      tx.sign( init_account_private_owner_key, db.get_chain_id() );
      tx.validate();
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      account_business_operation business_create;

      business_create.signatory = "ionstudios";
      business_create.account = "ionstudios";
      business_create.business_type = "public";
      business_create.officer_vote_threshold = BLOCKCHAIN_PRECISION * 1000;
      business_create.business_public_key = string( get_public_key( "ionstudios", "business", INIT_ACCOUNT_PASSWORD )  );
      business_create.init_ceo_account = alice.name;
      business_create.validate();
      
      tx.operations.push_back( business_create );
      tx.sign( ionstudios_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      const account_object& ionstudios_acc = db.get_account( "ionstudios" );
      const account_business_object& ionstudios_acc_bus = db.get_account_business( "ionstudios" );
      const account_authority_object& ionstudios_acc_auth = db.get< account_authority_object, by_account >( "ionstudios" );
      const account_following_object& ionstudios_acc_follow = db.get_account_following( "ionstudios" );

      BOOST_REQUIRE( ionstudios_acc.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( ionstudios_acc.name == "ionstudios" );
      BOOST_REQUIRE( ionstudios_acc_follow.account == "ionstudios" );
      BOOST_REQUIRE( ionstudios_acc_bus.account == "ionstudios" );
      BOOST_REQUIRE( ionstudios_acc_bus.executive_board.CHIEF_EXECUTIVE_OFFICER == business_create.init_ceo_account );
      BOOST_REQUIRE( string( ionstudios_acc_bus.business_public_key ) == business_create.business_public_key );
      BOOST_REQUIRE( ionstudios_acc.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( ionstudios_acc.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( ionstudios_acc.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( ionstudios_acc.reset_account == INIT_ACCOUNT );

      BOOST_REQUIRE( ionstudios_acc.last_updated == now() );
      BOOST_REQUIRE( ionstudios_acc_auth.owner_auth == authority( 1, ionstudios_public_owner_key, 1 ) );
      BOOST_REQUIRE( ionstudios_acc_auth.active_auth == authority( 2, ionstudios_public_active_key, 2 ) );
      BOOST_REQUIRE( ionstudios_acc_auth.posting_auth == authority( 1, ionstudios_public_posting_key, 1 ) );
      BOOST_REQUIRE( ionstudios_acc.secure_public_key == ionstudios_public_secure_key );
      BOOST_REQUIRE( ionstudios_acc.connection_public_key == ionstudios_public_connection_key );
      BOOST_REQUIRE( ionstudios_acc.friend_public_key == ionstudios_public_friend_key );
      BOOST_REQUIRE( ionstudios_acc.companion_public_key == ionstudios_public_companion_key );

      BOOST_REQUIRE( ionstudios_acc.id._id == ionstudios_acc_auth.id._id );
      BOOST_REQUIRE( ionstudios_acc.id._id == ionstudios_acc_follow.id._id );

      BOOST_REQUIRE( db.get_staked_balance( ionstudios_acc.name, SYMBOL_COIN ) == create.fee );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Business account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure of duplicate account creation" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure of duplicate account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when creator cannot cover fee" );

      tx.signatures.clear();
      tx.operations.clear();

      init_liquid = db.get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );

      create.fee = init_liquid + asset( 1 , SYMBOL_COIN );    // Fee slightly greater than liquid balance.

      create.new_account_name = account_name_type( "bob" );
      tx.operations.push_back( create );
      tx.sign( init_account_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.signatures.clear();
      tx.operations.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when creator cannot cover fee" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account fee is insufficient" );

      generate_block();

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_median_chain_properties(), [&]( median_chain_property_object& mcpo )
         {
            mcpo.account_creation_fee = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );    // Fee much too high.
         });
      });

      generate_block();

      tx.clear();
      create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      tx.operations.push_back( create );
      tx.sign( init_account_private_active_key, db.get_chain_id() );

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob) );

      account_update_operation update;

      update.signatory = alice.name;
      update.account = alice.name;
      update.details = "Details.";
      update.json = "{ \"valid\": true }";
      update.posting_auth.add_authorities( account_name_type( "candice" ), 1 );    // Candice account not made yet
      update.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( update );
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

      update.posting_auth = authority();
      update.validate();

      private_key_type active_key = generate_private_key( "aliceactivecorrecthorsebatterystaple" );

      db.modify( db.get< account_authority_object, by_account >( alice.name ), [&]( account_authority_object& a )
      {
         a.active_auth = authority( 1, active_key.get_public_key(), 1 );
      });

      tx.signatures.clear();
      tx.operations.clear();

      tx.operations.push_back( update );   // No signature
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: account update authorities" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signature" );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_owner_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invalid signature" );

      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_owner_auth );

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

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure on active key" );

      tx.signatures.clear();
      tx.sign( active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_owner_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure on active key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success on owner key" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "│   ├── Passed: success on owner key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when updating the owner authority with an active key" );

      tx.signatures.clear();
      update.owner_auth = authority( 1, active_key.get_public_key(), 1 );

      tx.operations.push_back( update );
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

      update.owner_auth = authority( 1, new_private_key.get_public_key(), 1 );
      update.active_auth = authority( 2, new_private_key.get_public_key(), 2 );
      update.posting_auth = authority( 2, new_private_key.get_public_key(), 2 );
      update.secure_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.connection_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.friend_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.companion_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      update.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      update.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      update.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";

      tx.operations.push_back( update );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_authority_object& alice_auth = db.get< account_authority_object, by_account >( alice.name );

      BOOST_REQUIRE( alice_auth.account == alice.name );
      BOOST_REQUIRE( alice_auth.owner_auth == authority( 1, new_private_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( alice_auth.active_auth == authority( 2, new_private_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( alice.secure_public_key == public_key_type( new_private_key.get_public_key() ) );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when updating a non-existant account" );

      tx.operations.clear();
      tx.signatures.clear();

      update.account = "bob";
      tx.operations.push_back( update );
      tx.sign( new_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception )
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when updating a non-existant account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account authority does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      update.account = alice.name;
      update.posting_auth = authority();
      update.posting_auth.weight_threshold = 1;
      update.posting_auth.add_authorities( "dan", 1 );

      tx.operations.push_back( update );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );
      fund( alice.name, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation membership;
      
      membership.signatory = alice.name;
      membership.account = alice.name;
      membership.membership_type = "standard";
      membership.months = 1;
      membership.interface = INIT_ACCOUNT;
      membership.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( membership );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.membership == membership_tier_type::STANDARD_MEMBERSHIP );
      BOOST_REQUIRE( alice.membership_interface == INIT_ACCOUNT );
      BOOST_REQUIRE( alice.recurring_membership == 1 );
      BOOST_REQUIRE( alice.membership_expiration == now() + fc::days(30) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account membership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when insufficient funds" );

      tx.operations.clear();
      tx.signatures.clear();

      membership.signatory = bob.name;
      membership.account = bob.name;

      tx.operations.push_back( membership );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when insufficient funds" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBERSHIP" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_member_request_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT MEMBER REQUEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account member request" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_member_request_operation request;

      request.signatory = alice.name;
      request.account = alice.name;
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";

      request.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_request_object& alice_request = db.get_account_member_request( alice.name, INIT_ACCOUNT );

      BOOST_REQUIRE( alice_request.account == alice.name );
      BOOST_REQUIRE( alice_request.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( alice_request.message == "Hello" );
      BOOST_REQUIRE( alice_request.expiration == now() + CONNECTION_REQUEST_DURATION );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account member request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when request already exists" );

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when request already exists" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel request" );

      request.requested = false;
      request.validate();

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_request_object* alice_request_ptr = db.find_account_member_request( alice.name, INIT_ACCOUNT );

      BOOST_REQUIRE( alice_request_ptr == nullptr );

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund( alice.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_EQUITY ) );
      fund_stake( alice.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_EQUITY ) );
      fund( bob.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_EQUITY ) );
      fund_stake( bob.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_EQUITY ) );

      fund( alice.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_COIN ) );
      fund( bob.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_COIN ) );
      fund_stake( bob.name, asset( BLOCKCHAIN_PRECISION * 10000, SYMBOL_COIN ) );

      private_key_type init_ceo_private_secure_key = get_private_key( INIT_CEO, "secure", INIT_ACCOUNT_PASSWORD );
      private_key_type init_ceo_private_active_key = get_private_key( INIT_CEO, "active", INIT_ACCOUNT_PASSWORD );
      public_key_type init_ceo_public_secure_key = get_public_key( INIT_CEO, "secure", INIT_ACCOUNT_PASSWORD );
      string init_account_private_business_wif = graphene::utilities::key_to_wif( get_private_key( INIT_ACCOUNT, "business", INIT_ACCOUNT_PASSWORD ) );

      account_member_invite_operation invite;

      invite.signatory = INIT_CEO;
      invite.account = INIT_CEO;
      invite.business_account = INIT_ACCOUNT;
      invite.member = alice.name;
      invite.message = "Hello";
      invite.encrypted_business_key = get_encrypted_message( init_ceo_private_secure_key, init_ceo_public_secure_key, alice_public_secure_key, init_account_private_business_wif );
      invite.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( invite );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_invite_object& alice_invite = db.get_account_member_invite( alice.name, INIT_ACCOUNT );

      BOOST_REQUIRE( alice_invite.account == INIT_CEO );
      BOOST_REQUIRE( alice_invite.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( alice_invite.member == alice.name );
      BOOST_REQUIRE( alice_invite.message == "Hello" );
      BOOST_REQUIRE( alice_invite.expiration == now() + CONNECTION_REQUEST_DURATION );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account member invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invite already exists" );

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( invite );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invite already exists" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel invite" );

      invite.invited = false;
      invite.validate();

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( invite );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_invite_object* alice_invite_ptr = db.find_account_member_invite( alice.name, INIT_ACCOUNT );

      BOOST_REQUIRE( alice_invite_ptr == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel invite" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBER INVITE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( business_account_management_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: BUSINESS ACCOUNT MANAGEMENT SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account accept invite" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( alice.name, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( bob.name, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( INIT_CEO, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( INIT_CEO, asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      private_key_type init_ceo_private_secure_key = get_private_key( INIT_CEO, "secure", INIT_ACCOUNT_PASSWORD );
      private_key_type init_ceo_private_active_key = get_private_key( INIT_CEO, "active", INIT_ACCOUNT_PASSWORD );
      public_key_type init_ceo_public_secure_key = get_public_key( INIT_CEO, "secure", INIT_ACCOUNT_PASSWORD );
      string init_account_private_business_wif = graphene::utilities::key_to_wif( get_private_key( INIT_ACCOUNT, "business", INIT_ACCOUNT_PASSWORD ) );

      account_member_invite_operation invite;

      invite.signatory = INIT_CEO;
      invite.account = INIT_CEO;
      invite.member = alice.name;
      invite.business_account = INIT_ACCOUNT;
      invite.message = "Hello";
      invite.encrypted_business_key = get_encrypted_message( init_ceo_private_secure_key, init_ceo_public_secure_key, alice_public_secure_key, init_account_private_business_wif );
      invite.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( invite );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation accept_invite;

      accept_invite.signatory = alice.name;
      accept_invite.account = alice.name;
      accept_invite.business_account = INIT_ACCOUNT;
      accept_invite.validate();

      tx.operations.push_back( accept_invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bus_acc.is_member( alice.name ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invite does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      accept_invite.signatory = dan.name;   // dan has not been invited
      accept_invite.account = dan.name;
      accept_invite.business_account = INIT_ACCOUNT;
      accept_invite.validate();

      tx.operations.push_back( accept_invite );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invite does not exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account accept request" );

      account_member_request_operation request;

      request.signatory = bob.name;
      request.account = bob.name;
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";
      request.validate();

      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_request_operation accept_request;

      accept_request.signatory = INIT_CEO;
      accept_request.account = INIT_CEO;
      accept_request.member = bob.name;
      accept_request.business_account = INIT_ACCOUNT;
      accept_request.encrypted_business_key = get_encrypted_message( init_ceo_private_secure_key, init_ceo_public_secure_key, bob_public_secure_key, init_account_private_business_wif );
      accept_request.validate();

      tx.operations.push_back( accept_request );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bus_acc.is_member( bob.name ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when unauthorized request acceptance" );

      tx.operations.clear();
      tx.signatures.clear();

      request.signatory = candice.name;
      request.account = candice.name;
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";
      request.validate();

      tx.operations.push_back( request );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept_request.signatory = dan.name;     // dan is not an officer, and cannot accept requests
      accept_request.account = dan.name;
      accept_request.member = candice.name;
      accept_request.business_account = INIT_ACCOUNT;
      accept_request.encrypted_business_key = string( candice_public_owner_key );
      accept_request.validate();

      tx.operations.push_back( accept_request );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when unauthorized request acceptance" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account vote officer" );

      account_vote_officer_operation vote_officer;

      vote_officer.signatory = INIT_CEO;
      vote_officer.account = INIT_CEO;
      vote_officer.business_account = INIT_ACCOUNT;
      vote_officer.officer_account = alice.name;
      vote_officer.vote_rank = 1;
      vote_officer.approved = true;
      vote_officer.validate();

      tx.operations.push_back( vote_officer );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_officer_vote_object& off_vote = db.get_account_officer_vote( INIT_CEO, INIT_ACCOUNT, alice.name );

      BOOST_REQUIRE( off_vote.account == vote_officer.account );
      BOOST_REQUIRE( off_vote.business_account == vote_officer.business_account );
      BOOST_REQUIRE( off_vote.officer_account == vote_officer.officer_account );
      BOOST_REQUIRE( off_vote.vote_rank == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote officer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no equity voting power" );

      tx.operations.clear();
      tx.signatures.clear();

      vote_officer.signatory = bob.name;
      vote_officer.account = bob.name;

      tx.operations.push_back( vote_officer );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no equity voting power" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel vote" );

      tx.operations.clear();
      tx.signatures.clear();
      
      vote_officer.signatory = alice.name;
      vote_officer.account = alice.name;
      vote_officer.approved = false;
      vote_officer.validate();

      tx.operations.push_back( vote_officer );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_officer_vote_object* officer_vote_ptr = db.find_account_officer_vote( INIT_CEO, INIT_ACCOUNT, alice.name );
      BOOST_REQUIRE( officer_vote_ptr == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account vote executive" );

      account_vote_executive_operation vote_executive;

      vote_executive.signatory = INIT_CEO;
      vote_executive.account = INIT_CEO;
      vote_executive.business_account = INIT_ACCOUNT;
      vote_executive.executive_account = alice.name;
      vote_executive.role = "operating";
      vote_executive.vote_rank = 1;
      vote_executive.approved = true;
      vote_executive.validate();

      tx.operations.push_back( vote_executive );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_executive_vote_object& exec_vote = db.get_account_executive_vote( INIT_CEO, INIT_ACCOUNT, alice.name );

      BOOST_REQUIRE( exec_vote.account == vote_executive.account );
      BOOST_REQUIRE( exec_vote.business_account == vote_executive.business_account );
      BOOST_REQUIRE( exec_vote.executive_account == vote_executive.executive_account );
      BOOST_REQUIRE( exec_vote.vote_rank == 1 );
      BOOST_REQUIRE( exec_vote.role == executive_role_type::CHIEF_OPERATING_OFFICER );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote executive" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when not a business member" );

      tx.operations.clear();
      tx.signatures.clear();

      vote_executive.signatory = bob.name;
      vote_executive.account = bob.name;

      tx.operations.push_back( vote_executive );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when not a business member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel vote" );

      tx.operations.clear();
      tx.signatures.clear();

      vote_executive.signatory = INIT_CEO;
      vote_executive.account = INIT_CEO;
      vote_executive.approved = false;
      vote_executive.validate();

      tx.operations.push_back( vote_executive );
      tx.sign( init_ceo_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_executive_vote_object* executive_vote_ptr = db.find_account_executive_vote( INIT_CEO, INIT_ACCOUNT, alice.name );
      BOOST_REQUIRE( executive_vote_ptr == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when unauthorized remove member" );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( bus_acc.is_member( alice.name ) );

      account_remove_member_operation remove;

      remove.signatory = bob.name;    // Bob is not an officer and cannot remove init ceo
      remove.account = bob.name;
      remove.member = alice.name;
      remove.business_account = INIT_ACCOUNT;
      remove.validate();

      tx.operations.push_back( remove );
      tx.sign( bob_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when unauthorized remove member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account remove member" );

      remove.signatory = alice.name;
      remove.account = alice.name;
      remove.member = bob.name;
      remove.business_account = INIT_ACCOUNT;
      remove.validate();

      tx.operations.push_back( remove );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !bus_acc.is_member( bob.name ) );    // bob has been removed

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account remove member" );

      BOOST_TEST_MESSAGE( "├── Passed: BUSINESS ACCOUNT MANAGEMENT SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_update_list_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT UPDATE LIST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update list" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_update_list_operation list;

      list.signatory = alice.name;
      list.account = alice.name;
      list.listed_account = bob.name;
      list.blacklisted = true;
      list.whitelisted = false;

      list.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( list );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_permission_object& acc_perm = db.get_account_permissions( alice.name );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( bob.name ) != acc_perm.blacklisted_accounts.end() );    // Bob has been blacklisted

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update list" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: remove from list" );

      tx.operations.clear();
      tx.signatures.clear();
      
      list.blacklisted = false;
      list.validate();

      tx.operations.push_back( list );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( bob.name ) == acc_perm.blacklisted_accounts.end() );    // Bob has been removed from blacklist

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: remove from list" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT UPDATE LIST" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_producer_vote_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT PRODUCER VOTE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account producer vote" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
   
      ACTORS( (alice)(bob)(candice)(dan)(elon)(sam) );

      fund( alice.name, asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( bob.name, asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( bob.name, asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( candice.name, asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( candice.name, asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );

      private_key_type alice_producer_key = generate_private_key( "aliceproducercorrecthorsebatterystaple" );

      producer_create(
         alice.name, 
         alice_private_owner_key, 
         alice_producer_key.get_public_key()
      );

      private_key_type candice_producer_key = generate_private_key( "candiceproducercorrecthorsebatterystaple" );

      producer_create(
         candice.name, 
         candice_private_owner_key, 
         candice_producer_key.get_public_key()
      );

      const producer_object& producer_a = db.get_producer( alice.name );
      const producer_object& producer_c = db.get_producer( candice.name );

      account_producer_vote_operation vote;

      vote.signatory = bob.name;
      vote.account = bob.name;
      vote.producer = alice.name;
      vote.vote_rank = 1;
      vote.approved = true;
      vote.validate();

      signed_transaction tx;

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_vote_object& prod_vote = db.get_producer_vote( bob.name, alice.name );

      BOOST_REQUIRE( prod_vote.account == bob.name );
      BOOST_REQUIRE( prod_vote.producer == alice.name );
      BOOST_REQUIRE( prod_vote.vote_rank == 1 );
      BOOST_REQUIRE( bob.producer_vote_count == 1 );
      BOOST_REQUIRE( producer_a.vote_count == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional producer vote" );

      vote.producer = candice.name;   // Vote for candice at rank one will move alice to rank two

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_vote_object& vote_a = db.get_producer_vote( bob.name, alice.name );
      const producer_vote_object& vote_c = db.get_producer_vote( bob.name, candice.name );
      
      BOOST_REQUIRE( vote_a.account == bob.name );
      BOOST_REQUIRE( vote_a.producer == alice.name );
      BOOST_REQUIRE( vote_a.vote_rank == 2 );
      BOOST_REQUIRE( vote_c.account == bob.name );
      BOOST_REQUIRE( vote_c.producer == candice.name );
      BOOST_REQUIRE( vote_c.vote_rank == 1 );
      BOOST_REQUIRE( bob.producer_vote_count == 2 );
      BOOST_REQUIRE( producer_a.vote_count == 1 );
      BOOST_REQUIRE( producer_c.vote_count == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: additional producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel producer vote" );

      vote.approved = false;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_vote_object* bob_candice_vote_ptr = db.find_producer_vote( bob.name, candice.name );
      BOOST_REQUIRE( bob_candice_vote_ptr == nullptr );
      BOOST_REQUIRE( bob.producer_vote_count == 1 );
      BOOST_REQUIRE( producer_c.vote_count == 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel additional producer vote" );

      vote.producer = alice.name;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_vote_object* bob_alice_vote_ptr = db.find_producer_vote( bob.name, alice.name );
      BOOST_REQUIRE( bob_alice_vote_ptr == nullptr );
      BOOST_REQUIRE( bob.producer_vote_count == 0 );
      BOOST_REQUIRE( producer_a.vote_count == 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel additional producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when attempting to revoke a non-existent vote" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when attempting to revoke a non-existent vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting for a non-existent account" );

      tx.operations.clear();
      tx.signatures.clear();

      vote.producer = "eve";
      vote.approved = true;

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting for a non-existent account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting for an account that is not a producer" );

      tx.operations.clear();
      tx.signatures.clear();

      vote.producer = elon.name;
      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting for an account that is not a producer" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT PRODUCER VOTE" );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(sam)(dan)(elon) )

      fund( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( bob.name, asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( candice.name, asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( candice.name, asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( sam.name, asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( sam.name, asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( dan.name, asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( dan.name, asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_update_proxy_operation proxy;

      proxy.signatory = bob.name;
      proxy.account = bob.name;
      proxy.proxy = alice.name;
      proxy.validate();

      signed_transaction tx;

      tx.operations.push_back( proxy );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );    // bob -> alice

      BOOST_REQUIRE( bob.proxy == alice.name );
      BOOST_REQUIRE( !bob.proxied.size() );
      BOOST_REQUIRE( alice.proxied.size() );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *alice.proxied.begin() == bob.name );
      BOOST_REQUIRE( db.get_proxied_voting_power( alice, equity_price ) == db.get_voting_power( bob, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: changing proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      proxy.proxy = sam.name;
      tx.operations.push_back( proxy );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );     // bob->sam

      BOOST_REQUIRE( bob.proxy == sam.name );
      BOOST_REQUIRE( bob.proxied.size() == 0 );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( alice.proxied.size() == 0 );
      BOOST_REQUIRE( sam.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == bob.name );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: changing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when changing proxy to existing proxy" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when changing proxy to existing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandparent proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      proxy.signatory = sam.name;
      proxy.account = sam.name;
      proxy.proxy = dan.name;
      
      tx.operations.push_back( proxy );
      tx.sign( sam_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );     // bob->sam->dan

      BOOST_REQUIRE( bob.proxy == sam.name );
      BOOST_REQUIRE( *sam.proxied.begin() == bob.name );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      BOOST_REQUIRE( sam.proxy == dan.name );
      BOOST_REQUIRE( *dan.proxied.begin() == sam.name );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandparent proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandchild proxy" );

      // alice
      // bob->  sam->dan

      tx.operations.clear();
      tx.signatures.clear();

      proxy.signatory = alice.name;
      proxy.account = alice.name;
      proxy.proxy = sam.name;
      
      tx.operations.push_back( proxy );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.proxy == sam.name );
      BOOST_REQUIRE( bob.proxy == sam.name );
      BOOST_REQUIRE( sam.proxied.size() == 2 );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) + db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == dan.name );
      BOOST_REQUIRE( *dan.proxied.begin() == sam.name );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandchild proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing a grandchild proxy" );

      tx.operations.clear();
      tx.signatures.clear();

      proxy.signatory = bob.name;
      proxy.account = bob.name;
      proxy.proxy = PROXY_TO_SELF_ACCOUNT;
      
      tx.operations.push_back( proxy );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );    // alice->sam->dan

      BOOST_REQUIRE( alice.proxy == sam.name );
      BOOST_REQUIRE( bob.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == alice.name );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == dan.name );
      BOOST_REQUIRE( *dan.proxied.begin() == sam.name );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      
      ACTORS( (alice)(candice)(sam)(dan) );
      fund( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_create_operation acc_create;       // Creating account bob with alice as recovery account

      acc_create.signatory = alice.name;
      acc_create.registrar = alice.name;
      acc_create.referrer = alice.name;
      acc_create.recovery_account = alice.name;
      acc_create.reset_account = alice.name;
      acc_create.new_account_name = account_name_type( "bob" );
      acc_create.owner_auth = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      acc_create.active_auth = authority( 1, generate_private_key( "bob_active" ).get_public_key(), 1 );
      acc_create.posting_auth = authority( 1, generate_private_key( "bob_posting" ).get_public_key(), 1 );
      acc_create.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_create.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_create.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_create.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( 32 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      acc_create.validate();

      signed_transaction tx;

      tx.operations.push_back( acc_create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& bob = db.get_account( account_name_type( "bob" ) );

      const account_authority_object& bob_auth = db.get< account_authority_object, by_account >( bob.name );
      BOOST_REQUIRE( bob_auth.owner_auth == acc_create.owner_auth );

      // Changing bob's owner authority

      generate_blocks( bob_auth.last_owner_update + OWNER_UPDATE_LIMIT );

      account_update_operation acc_update;

      acc_update.signatory = bob.name;
      acc_update.account = bob.name;
      acc_update.owner_auth = authority( 1, generate_private_key( "bad_key" ).get_public_key(), 1 );
      acc_update.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_update.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_update.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_update.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_update.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_update.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_update.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_update.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_update.validate();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( acc_update );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_auth.owner_auth == acc_update.owner_auth );

      tx.operations.clear();
      tx.signatures.clear();

      request_account_recovery_operation request;

      request.signatory = alice.name;
      request.recovery_account = alice.name;
      request.account_to_recover = bob.name;
      request.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );
      request.validate();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_auth.owner_auth == acc_update.owner_auth );

      generate_blocks( now() + OWNER_UPDATE_LIMIT );

      tx.operations.clear();
      tx.signatures.clear();

      recover_account_operation recover;
      
      recover.signatory = bob.name;
      recover.account_to_recover = bob.name;
      recover.new_owner_authority = request.new_owner_authority;
      recover.recent_owner_authority = acc_create.owner_auth;
      recover.validate();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      shared_authority owner1 = db.get< account_authority_object, by_account >(bob.name).owner_auth;

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
      shared_authority owner2 = db.get< account_authority_object, by_account >(bob.name).owner_auth;
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
      shared_authority owner3 = db.get< account_authority_object, by_account >(bob.name).owner_auth;
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

      shared_authority owner4 = db.get< account_authority_object, by_account >(bob.name).owner_auth;
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

      BOOST_REQUIRE( req_itr->account_to_recover == bob.name );
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
      shared_authority owner5 = db.get< account_authority_object, by_account >(bob.name).owner_auth;
      BOOST_REQUIRE( owner5 == authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: creating a recovery request that will expire" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Expiring owner authority history" );

      acc_update.owner_auth = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

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
      shared_authority owner6 = db.get< account_authority_object, by_account >(bob.name).owner_auth;
      BOOST_REQUIRE( owner6 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      recover.recent_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      tx.sign( generate_private_key( "last key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );     // foo bar key has not yet expired
      shared_authority owner7 = db.get< account_authority_object, by_account >(bob.name).owner_auth;
      BOOST_REQUIRE( owner7 == authority( 1, generate_private_key( "last key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Expiring owner authority history" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: change recovery account" );

      auto change_recovery_account = [&]( const std::string& account_to_recover, const std::string& new_recovery_account )
      {
         change_recovery_account_operation recovery;
         recovery.account_to_recover = account_to_recover;
         recovery.new_recovery_account = new_recovery_account;

         signed_transaction tx;
         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      auto recover_account = [&]( const std::string& account_to_recover, const fc::ecc::private_key& new_owner_key, const fc::ecc::private_key& recent_owner_key )
      {
         recover_account_operation recovery;
         recovery.account_to_recover = account_to_recover;
         recovery.new_owner_authority = authority( 1, public_key_type( new_owner_key.get_public_key() ), 1 );
         recovery.recent_owner_authority = authority( 1, public_key_type( recent_owner_key.get_public_key() ), 1 );

         signed_transaction tx;
         tx.operations.push_back( recovery );
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
         request_account_recovery_operation recovery;
         recovery.recovery_account    = recovery_account;
         recovery.account_to_recover  = account_to_recover;
         recovery.new_owner_authority = authority( 1, new_owner_key, 1 );

         signed_transaction tx;
         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( recovery_account_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      auto change_owner = [&]( const std::string& account, const fc::ecc::private_key& old_private_key, const public_key_type& new_public_key )
      {
         account_update_operation recovery;
         recovery.account = account;
         recovery.owner_auth = authority( 1, new_public_key, 1 );

         signed_transaction tx;
         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( old_private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      // if either/both users do not exist, we shouldn't allow it
      REQUIRE_THROW( change_recovery_account( alice.name, "nobody" ), fc::exception );
      REQUIRE_THROW( change_recovery_account( "haxer", sam.name ), fc::exception );
      REQUIRE_THROW( change_recovery_account( "haxer", "nobody" ), fc::exception );
      change_recovery_account( alice.name, sam.name );

      fc::ecc::private_key alice_priv1 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k1" ) );
      fc::ecc::private_key alice_priv2 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k2" ) );
      public_key_type alice_pub1 = public_key_type( alice_priv1.get_public_key() );

      generate_blocks( now() + OWNER_AUTH_RECOVERY_PERIOD - BLOCK_INTERVAL, true );
      // cannot request account recovery until recovery account is approved
      REQUIRE_THROW( request_account_recovery( sam.name, sam_private_owner_key, alice.name, alice_pub1 ), fc::exception );
      generate_blocks(1);
      // cannot finish account recovery until requested
      REQUIRE_THROW( recover_account( alice.name, alice_priv1, alice_private_owner_key ), fc::exception );
      // do the request
      request_account_recovery( sam.name, sam_private_owner_key, alice.name, alice_pub1 );
      // can't recover with the current owner key
      REQUIRE_THROW( recover_account( alice.name, alice_priv1, alice_private_owner_key ), fc::exception );
      // unless we change it!
      change_owner( alice.name, alice_private_owner_key, public_key_type( alice_priv2.get_public_key() ) );
      recover_account( alice.name, alice_priv1, alice_private_owner_key );

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(candice)(dan) );

      fund( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_create_operation acc_create;       // Creating account bob with alice as reset account

      acc_create.signatory = alice.name;
      acc_create.registrar = alice.name;
      acc_create.referrer = alice.name;
      acc_create.recovery_account = alice.name;
      acc_create.reset_account = alice.name;
      acc_create.new_account_name = account_name_type( "bob" );
      acc_create.owner_auth = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      acc_create.active_auth = authority( 1, generate_private_key( "bob_active" ).get_public_key(), 1 );
      acc_create.posting_auth = authority( 1, generate_private_key( "bob_posting" ).get_public_key(), 1 );
      acc_create.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_create.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_create.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_create.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( 32 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      acc_create.validate();

      signed_transaction tx;

      tx.operations.push_back( acc_create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_object& bob = db.get_account( account_name_type( "bob" ) );

      const account_authority_object& bob_auth = db.get< account_authority_object, by_account >( bob.name );
      BOOST_REQUIRE( bob_auth.owner_auth == acc_create.owner_auth );

      const account_object& bob_acc = db.get_account( bob.name );

      generate_blocks( now() + ( fc::days( bob_acc.reset_account_delay_days ) - BLOCK_INTERVAL ) );

      reset_account_operation reset;

      reset.signatory = alice.name;
      reset.reset_account = alice.name;
      reset.account_to_reset = bob.name;
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

      tx.operations.push_back( reset );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet

      generate_block();

      db.push_transaction( tx, 0 );    // reset now valid

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( bob_auth.owner_auth == reset.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account reset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: set new reset account" );

      set_reset_account_operation set;

      set.signatory = bob.name;
      set.account = bob.name;
      set.new_reset_account = candice.name;

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );
      
      BOOST_REQUIRE( bob_acc.reset_account == candice.name );

      BOOST_TEST_MESSAGE( "│   ├── Passed: set new reset account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional account reset" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + ( fc::days( bob_acc.reset_account_delay_days ) - BLOCK_INTERVAL ) );

      reset.signatory = candice.name;
      reset.reset_account = candice.name;
      reset.account_to_reset = bob.name;
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key_1" ).get_public_key(), 1 );

      tx.operations.push_back( reset );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet

      generate_block();

      db.push_transaction( tx, 0 );    // reset now valid

      BOOST_REQUIRE( bob_auth.owner_auth == reset.new_owner_authority );

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

      set.new_reset_account = candice.name;

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob) );

      generate_block();
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      generate_block();

      decline_voting_rights_operation decline;

      decline.signatory = alice.name;
      decline.account = alice.name;
      decline.declined = true;
      decline.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< decline_voting_rights_request_index >().indices().get< by_account >();
      auto req_itr = req_idx.find( alice.name );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->effective_date == now() + DECLINE_VOTING_RIGHTS_DURATION );

      BOOST_TEST_MESSAGE( "│   ├── Passed: decline voting rights request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure revoking voting rights with existing request" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure revoking voting rights with existing request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success cancelling a request" );

      decline.declined = false;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( alice.name );
      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed

      BOOST_TEST_MESSAGE( "│   ├── Passed: success cancelling a request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure cancelling a request that doesn't exist" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // Can't cancel again

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure cancelling a request that doesn't exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account can vote during waiting period" );

      decline.declined = true;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now() + DECLINE_VOTING_RIGHTS_DURATION - BLOCK_INTERVAL, true );

      BOOST_REQUIRE( db.get_account( alice.name ).can_vote );   // Alice can still vote

      producer_create( 
         alice.name, 
         alice_private_owner_key,
         alice_private_owner_key.get_public_key()
         );

      account_producer_vote_operation producer_vote;

      producer_vote.signatory = alice.name;
      producer_vote.account = alice.name;
      producer_vote.producer = alice.name;
      producer_vote.vote_rank = 1;
      producer_vote.validate();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment_operation comment;

      comment.signatory = alice.name;
      comment.author = alice.name;
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";
      comment.language = "en";
      comment.interface = INIT_ACCOUNT;
      comment.tags.push_back( "test" );
      comment.json = "{ \"valid\": true }";

      comment_options options;

      options.post_type = "article";
      options.reach = "tag";
      options.rating = 1;

      comment.options = options;
      comment.validate();

      vote_operation vote;

      vote.signatory = alice.name;
      vote.voter = alice.name;
      vote.author = alice.name;
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
      const auto& producer_vote_idx = db.get_index< producer_vote_index >().indices().get< by_producer_account >();

      auto comment_itr = comment_idx.find( boost::make_tuple( alice.name, string( "test" ) ) );
      auto comment_vote_itr = comment_vote_idx.find( boost::make_tuple( alice.name, comment_itr->id ) );
      auto producer_vote_itr = producer_vote_idx.find( boost::make_tuple( alice.name, alice.name ) );

      BOOST_REQUIRE( comment_itr->author == alice.name );
      BOOST_REQUIRE( comment_vote_itr->voter == alice.name );
      BOOST_REQUIRE( producer_vote_itr->account == alice.name );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account can vote during waiting period" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account cannot vote after request is processed" );

      generate_block();
      BOOST_REQUIRE( !db.get_account( alice.name ).can_vote );   // Alice can no longer vote
      validate_database();

      req_itr = req_idx.find( alice.name );
      BOOST_REQUIRE( req_itr == req_idx.end() );   // Request no longer exists

      producer_vote_itr = producer_vote_idx.find( boost::make_tuple( alice.name, alice.name ) );
      BOOST_REQUIRE( producer_vote_itr == producer_vote_idx.end() );   // Vote has been removed

      tx.operations.clear();
      tx.signatures.clear();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( producer_vote );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      generate_block();
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      string alice_private_connection_wif = graphene::utilities::key_to_wif( alice_private_connection_key );
      string bob_private_connection_wif = graphene::utilities::key_to_wif( bob_private_connection_key );
      string alice_private_friend_wif = graphene::utilities::key_to_wif( alice_private_friend_key );
      string bob_private_friend_wif = graphene::utilities::key_to_wif( bob_private_friend_key );
      string alice_private_companion_wif = graphene::utilities::key_to_wif( alice_private_companion_key );
      string bob_private_companion_wif = graphene::utilities::key_to_wif( bob_private_companion_key );

      generate_block();

      connection_request_operation request;

      request.signatory = alice.name;
      request.account = alice.name;
      request.requested_account = bob.name;
      request.connection_type = "connection";
      request.message = "Hello";
      request.requested = true;
      request.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< connection_request_index >().indices().get< by_account_req >();
      auto req_itr = req_idx.find( boost::make_tuple( alice.name, bob.name ) );
      const auto& con_idx = db.get_index< connection_index >().indices().get< by_accounts >();
      auto con_itr = con_idx.find( boost::make_tuple( alice.name, bob.name, connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->account == alice.name );
      BOOST_REQUIRE( req_itr->requested_account == bob.name );
      BOOST_REQUIRE( req_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( req_itr->expiration == now() + CONNECTION_REQUEST_DURATION );
      BOOST_REQUIRE( con_itr == con_idx.end() );    // Connection not created yet

      BOOST_TEST_MESSAGE( "│   ├── Passed: create connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      connection_accept_operation accept;

      accept.signatory = bob.name;
      accept.account = bob.name;
      accept.requesting_account = alice.name;
      accept.connection_type = "connection";
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, bob_private_connection_wif );
      accept.connected = true;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( boost::make_tuple( alice.name, bob.name ) );
      con_itr = con_idx.find( boost::make_tuple( alice.name, bob.name, connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed
      BOOST_REQUIRE( con_itr->account_a == alice.name );
      BOOST_REQUIRE( con_itr->account_b == bob.name );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == accept.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == accept.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: match connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = alice.name;
      accept.account = alice.name;
      accept.requesting_account = bob.name;
      accept.encrypted_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, alice_private_connection_wif );
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      req_itr = req_idx.find( boost::make_tuple( alice.name, bob.name ) );
      con_itr = con_idx.find( boost::make_tuple( alice.name, bob.name, connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed
      BOOST_REQUIRE( con_itr->account_a == alice.name );
      BOOST_REQUIRE( con_itr->account_b == bob.name );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == accept.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == accept.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: match connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create account verification" );

      account_verification_operation verification;
      
      verification.signatory = alice.name;
      verification.verifier_account = alice.name;
      verification.verified_account = bob.name;
      verification.shared_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      verification.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( verification );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      validate_database();

      const account_verification_object alice_verification = db.get_account_verification( alice.name, bob.name );

      BOOST_REQUIRE( verification.verifier_account == alice_verification.verifier_account );
      BOOST_REQUIRE( verification.verified_account == alice_verification.verified_account );
      BOOST_REQUIRE( verification.shared_image == to_string( alice_verification.shared_image ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create account verification" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting existing connection type" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting existing connection type" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting friend connection before request duration" );

      generate_blocks( now() + CONNECTION_REQUEST_DURATION - BLOCK_INTERVAL, true );

      request.connection_type = "friend";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting friend connection before request duration" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: friend connection request" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( boost::make_tuple( alice.name, bob.name ) );
      con_itr = con_idx.find( boost::make_tuple( alice.name, bob.name, connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->account == alice.name );
      BOOST_REQUIRE( req_itr->requested_account == bob.name );
      BOOST_REQUIRE( req_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( req_itr->expiration == now() + CONNECTION_REQUEST_DURATION );
      BOOST_REQUIRE( con_itr == con_idx.end() );    // Connection not created yet

      BOOST_TEST_MESSAGE( "│   ├── Passed: friend connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept friend connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = bob.name;
      accept.account = bob.name;
      accept.requesting_account = alice.name;
      accept.connection_type = "friend";
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, bob_private_friend_wif );
      accept.connected = true;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( boost::make_tuple( alice.name, bob.name ) );
      con_itr = con_idx.find( boost::make_tuple( alice.name, bob.name, connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );
      BOOST_REQUIRE( con_itr->account_a == alice.name );
      BOOST_REQUIRE( con_itr->account_b == bob.name );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == accept.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == accept.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept friend connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: match friend connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = alice.name;
      accept.account = alice.name;
      accept.requesting_account = bob.name;
      accept.connection_type = "friend";
      accept.encrypted_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, alice_private_friend_wif );
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( boost::make_tuple( alice.name, bob.name ) );
      con_itr = con_idx.find( boost::make_tuple( alice.name, bob.name, connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );
      BOOST_REQUIRE( con_itr->account_a == alice.name );
      BOOST_REQUIRE( con_itr->account_b == bob.name );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == accept.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == accept.connection_id );
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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob) );

      account_follow_operation follow;

      follow.signatory = alice.name;
      follow.follower = alice.name;
      follow.following = bob.name;
      follow.interface = INIT_ACCOUNT;
      follow.added = true;
      follow.followed = true;
      follow.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( alice.name );
      const account_following_object& following_b = db.get_account_following( bob.name );

      BOOST_REQUIRE( following_a.is_following( account_name_type( bob.name ) ) );
      BOOST_REQUIRE( following_b.is_follower( account_name_type( alice.name ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account follow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: mutual following" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = bob.name;
      follow.follower = bob.name;
      follow.following = alice.name;

      tx.operations.push_back( follow );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( following_a.is_follower( account_name_type( bob.name ) ) );
      BOOST_REQUIRE( following_b.is_following( account_name_type( alice.name ) ) );
      BOOST_REQUIRE( following_a.is_mutual( account_name_type( bob.name ) ) );
      BOOST_REQUIRE( following_b.is_mutual( account_name_type( alice.name ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: mutual following" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when following already followed account" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = bob.name;
      follow.follower = bob.name;
      follow.following = alice.name;

      tx.operations.push_back( follow );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when following already followed account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: unfollowing" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = bob.name;
      follow.follower = bob.name;
      follow.following = alice.name;
      follow.added = false;
      follow.followed = true;

      tx.operations.push_back( follow );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = alice.name;
      follow.follower = alice.name;
      follow.following = bob.name;
      follow.added = false;
      follow.followed = true;

      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( !following_a.is_follower( account_name_type( bob.name ) ) );
      BOOST_REQUIRE( !following_b.is_following( account_name_type( alice.name ) ) );
      BOOST_REQUIRE( !following_a.is_following( account_name_type( bob.name ) ) );
      BOOST_REQUIRE( !following_b.is_follower( account_name_type( alice.name ) ) );
      BOOST_REQUIRE( !following_a.is_mutual( account_name_type( bob.name ) ) );
      BOOST_REQUIRE( !following_b.is_mutual( account_name_type( alice.name ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: unfollowing" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: filtering" );

      follow.signatory = alice.name;
      follow.follower = alice.name;
      follow.following = bob.name;
      follow.added = true;
      follow.followed = false;

      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( following_a.is_filtered( account_name_type( bob.name ) ) );

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      tag_follow_operation follow;

      follow.signatory = alice.name;
      follow.follower = alice.name;
      follow.tag = "test";
      follow.interface = INIT_ACCOUNT;
      follow.added = true;
      follow.followed = true;
      follow.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( alice.name );
      const tag_following_object& following_t = db.get_tag_following( "test" );
      
      BOOST_REQUIRE( following_a.is_followed_tag( tag_name_type( "test" ) ) );
      BOOST_REQUIRE( following_t.is_follower( account_name_type( alice.name ) ) );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal tag follow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when following already followed tag" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when following already followed tag" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: unfollowing" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.added = false;

      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      
      BOOST_REQUIRE( !following_a.is_followed_tag( tag_name_type( "test" ) ) );
      BOOST_REQUIRE( !following_t.is_follower( account_name_type( alice.name ) ) );

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn) );

      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( alice.name, alice_private_owner_key, alice_public_owner_key );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( bob.name, bob_private_owner_key, bob_public_owner_key );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( candice.name, candice_private_owner_key, candice_public_owner_key );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( dan.name, dan_private_owner_key, dan_public_owner_key );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( elon.name, elon_private_owner_key, elon_public_owner_key );

      fund_stake( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( fred.name, fred_private_owner_key, fred_public_owner_key );

      fund_stake( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( george.name, george_private_owner_key, george_public_owner_key );

      fund_stake( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( haz.name, haz_private_owner_key, haz_public_owner_key );

      fund_stake( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( isabelle.name, isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( jayme.name, jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( kathryn.name, kathryn_private_owner_key, kathryn_public_owner_key );

      generate_block();

      comment_create( alice.name, alice_private_posting_key, "alicetestpost" );
      const comment_object& bob_post = comment_create( bob.name, bob_private_posting_key, "bobtestpost" );
      const comment_object& candice_post = comment_create( candice.name, candice_private_posting_key, "candicetestpost" );

      vote_operation vote;

      vote.signatory = bob.name;
      vote.voter = bob.name;
      vote.author = alice.name;
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

      vote.signatory = alice.name;
      vote.voter = alice.name;
      vote.author = bob.name;
      vote.permlink = "bobtestpost";

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = candice.name;
      vote.voter = candice.name;
      vote.author = alice.name;
      vote.permlink = "alicetestpost";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = dan.name;
      vote.voter = dan.name;
      
      tx.operations.push_back( vote );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = elon.name;
      vote.voter = elon.name;

      tx.operations.push_back( vote );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view_operation view;

      view.signatory = bob.name;
      view.viewer = bob.name;
      view.author = alice.name;
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

      view.signatory = alice.name;
      view.viewer = alice.name;
      view.author = candice.name;
      view.permlink = "candicetestpost";

      tx.operations.push_back( view );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = candice.name;
      view.viewer = candice.name;
      view.author = alice.name;
      view.permlink = "alicetestpost";

      tx.operations.push_back( view );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = dan.name;
      view.viewer = dan.name;

      tx.operations.push_back( view );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = elon.name;
      view.viewer = elon.name;

      tx.operations.push_back( view );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      db.update_comment_metrics();     // calculate median values.

      account_producer_vote_operation producer_vote;

      producer_vote.signatory = alice.name;
      producer_vote.account = alice.name;
      producer_vote.producer = bob.name;
      producer_vote.vote_rank = 1;
      producer_vote.approved = true;
      producer_vote.validate();

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = candice.name;
      producer_vote.vote_rank = 2;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = dan.name;
      producer_vote.vote_rank = 3;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = elon.name;
      producer_vote.vote_rank = 5;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = fred.name;
      producer_vote.vote_rank = 6;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = george.name;
      producer_vote.vote_rank = 7;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = haz.name;
      producer_vote.vote_rank = 8;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = isabelle.name;
      producer_vote.vote_rank = 9;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = jayme.name;
      producer_vote.vote_rank = 10;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = kathryn.name;
      producer_vote.vote_rank = 11;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      activity_reward_operation activity;

      activity.signatory = alice.name;
      activity.account = alice.name;
      activity.permlink = "alicetestpost";
      activity.vote_id = bob_post.id._id;
      activity.view_id = candice_post.id._id;
      activity.interface = INIT_ACCOUNT;
      activity.validate();

      tx.operations.push_back( activity );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const reward_fund_object& rfo = db.get_reward_fund( SYMBOL_COIN );
      
      BOOST_REQUIRE( alice.last_activity_reward == now() );
      BOOST_REQUIRE( rfo.recent_activity_claims == uint128_t( BLOCKCHAIN_PRECISION.value ) );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal activity reward claim process" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when claiming again" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( activity );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when claiming again" );

      BOOST_TEST_MESSAGE( "├── Passed: ACTIVITY REWARD" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()