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

      generate_blocks( TOTAL_PRODUCERS );

      account_create_operation create;

      create.registrar = "alice";
      create.referrer = "alice";
      create.new_account_name = "bob";

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

      fc::ecc::private_key alice_private_owner_key = get_private_key( "alice", OWNER_KEY_STR, INIT_PASSWORD );
      fc::ecc::private_key alice_private_active_key = get_private_key( "alice", ACTIVE_KEY_STR, INIT_PASSWORD );
      fc::ecc::private_key alice_private_posting_key = get_private_key( "alice", POSTING_KEY_STR, INIT_PASSWORD );
      fc::ecc::private_key alice_private_secure_key = get_private_key( "alice", "secure", INIT_PASSWORD );
      fc::ecc::private_key alice_private_connection_key = get_private_key( "alice", "connection", INIT_PASSWORD );
      fc::ecc::private_key alice_private_friend_key = get_private_key( "alice", "friend", INIT_PASSWORD );
      fc::ecc::private_key alice_private_companion_key = get_private_key( "alice", "companion", INIT_PASSWORD );

      public_key_type alice_public_owner_key = alice_private_owner_key.get_public_key();
      public_key_type alice_public_active_key = alice_private_active_key.get_public_key();
      public_key_type alice_public_posting_key = alice_private_posting_key.get_public_key();
      public_key_type alice_public_secure_key = alice_private_secure_key.get_public_key();
      public_key_type alice_public_connection_key = alice_private_connection_key.get_public_key();
      public_key_type alice_public_friend_key = alice_private_friend_key.get_public_key();
      public_key_type alice_public_companion_key = alice_private_companion_key.get_public_key();

      asset init_liquid_balance = get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );

      create.registrar = INIT_ACCOUNT;
      create.new_account_name = "alice";
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
      create.relationship = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#In a Relationship" ) );
      create.political_alignment = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#Centrist" ) );
      create.owner_auth = authority( 1, alice_public_owner_key, 1 );
      create.active_auth = authority( 2, alice_public_active_key, 2 );
      create.posting_auth = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_secure_key );
      create.connection_public_key = string( alice_public_connection_key );
      create.friend_public_key = string( alice_public_friend_key );
      create.companion_public_key = string( alice_public_companion_key );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      create.validate();
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( init_account_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      const account_object& alice = db.get_account( account_name_type( "alice" ) );
      const account_authority_object& alice_auth = db.get< account_authority_object, by_account >( account_name_type( "alice" ) );
      const account_following_object& alice_follow = db.get_account_following( account_name_type( "alice" ) );

      BOOST_REQUIRE( alice.registrar == INIT_ACCOUNT );
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
      
      BOOST_REQUIRE( get_staked_balance( account_name_type( "alice" ), SYMBOL_COIN ) == create.fee );

      asset init_liquid = get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );
      
      BOOST_REQUIRE( ( init_liquid_balance - create.fee ) == init_liquid );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure of duplicate account creation" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure of duplicate account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when creator cannot cover fee" );

      tx.signatures.clear();
      tx.operations.clear();

      init_liquid = get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );

      create.fee = init_liquid + asset( 1 , SYMBOL_COIN );    // Fee slightly greater than liquid balance.
      create.new_account_name = "bob";

      tx.operations.push_back( create );
      tx.sign( init_account_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.signatures.clear();
      tx.operations.clear();

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

      ACTORS( (alice)(bob) );

      private_key_type new_private_key = get_private_key( "alice", OWNER_KEY_STR, "aliceownerhunter2" );

      account_update_operation update;

      update.account = "alice";
      update.posting_auth.add_authorities( account_name_type( "candice" ), 1 );    // Candice account not made yet
      update.active_auth.add_authorities( account_name_type( "candice" ), 1 );
      update.owner_auth.add_authorities( account_name_type( "candice" ), 1 );
      update.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( update );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.signatures.clear();
      tx.operations.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account posting authority is invalid" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: account update authorities" );

      update.owner_auth = authority( 1, new_private_key.get_public_key(), 1 );
      update.active_auth = authority( 1, new_private_key.get_public_key(), 1 );
      update.posting_auth = authority( 1, new_private_key.get_public_key(), 1 );
      update.secure_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.connection_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.friend_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.companion_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      update.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      update.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      update.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      update.validate();

      tx.operations.push_back( update );   // No signature

      BOOST_TEST_MESSAGE( "│   ├── Passed: account update authorities" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signature" );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invalid signature" );

      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invalid signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when additional irrelevant signature" );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when additional irrelevant signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure on active key" );

      tx.signatures.clear();
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure on active key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Account update success with owner key" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      tx.signatures.clear();
      tx.operations.clear();

      const account_authority_object& alice_auth = db.get< account_authority_object, by_account >( account_name_type( "alice" ) );

      BOOST_REQUIRE( alice_auth.account == "alice" );
      BOOST_REQUIRE( alice_auth.owner_auth == authority( 1, new_private_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( alice_auth.active_auth == authority( 1, new_private_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( alice_auth.posting_auth == authority( 1, new_private_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( alice.secure_public_key == public_key_type( new_private_key.get_public_key() ) );
      BOOST_REQUIRE( alice.connection_public_key == public_key_type( new_private_key.get_public_key() ) );
      BOOST_REQUIRE( alice.friend_public_key == public_key_type( new_private_key.get_public_key() ) );
      BOOST_REQUIRE( alice.companion_public_key == public_key_type( new_private_key.get_public_key() ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Account update success with owner key" );

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
      
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation membership;
      
      membership.account = "alice";
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

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account membership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when insufficient funds" );

      tx.operations.clear();
      tx.signatures.clear();

      membership.account = "bob";

      tx.operations.push_back( membership );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when insufficient funds" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBERSHIP" );
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

      account_update_list_operation list;

      list.account = "alice";
      list.listed_account = "bob";
      list.blacklisted = true;
      list.whitelisted = false;

      list.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( list );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_permission_object& acc_perm = db.get_account_permissions( account_name_type( "alice" ) );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( account_name_type( "bob" ) ) != acc_perm.blacklisted_accounts.end() );    // Bob has been blacklisted

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update list" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: remove from list" );

      tx.operations.clear();
      tx.signatures.clear();
      
      list.blacklisted = false;
      list.validate();

      tx.operations.push_back( list );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( account_name_type( "bob" ) ) == acc_perm.blacklisted_accounts.end() );    // Bob has been removed from blacklist

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
   
      ACTORS( (alice)(bob)(candice)(dan)(elon)(sam) );

      fund_liquid( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key );

      fund_liquid( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );

      fund_liquid( "candice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "candice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key );

      const producer_object& producer_a = db.get_producer( account_name_type( "alice" ) );
      const producer_object& producer_c = db.get_producer( account_name_type( "candice" ) );

      account_producer_vote_operation vote;

      vote.account = "bob";
      vote.producer = "alice";
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

      const producer_vote_object& prod_vote = db.get_producer_vote( account_name_type( "bob" ), account_name_type( "alice" ) );

      BOOST_REQUIRE( prod_vote.account == "bob" );
      BOOST_REQUIRE( prod_vote.producer == "alice" );
      BOOST_REQUIRE( prod_vote.vote_rank == 1 );
      BOOST_REQUIRE( bob.producer_vote_count == 1 );
      BOOST_REQUIRE( producer_a.vote_count == 1 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional producer vote" );

      vote.producer = "candice";   // Vote for candice at rank one will move alice to rank two

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_vote_object& vote_a = db.get_producer_vote( account_name_type( "bob" ), account_name_type( "alice" ) );
      const producer_vote_object& vote_c = db.get_producer_vote( account_name_type( "bob" ), account_name_type( "candice" ) );
      
      BOOST_REQUIRE( vote_a.account == "bob" );
      BOOST_REQUIRE( vote_a.producer == "alice" );
      BOOST_REQUIRE( vote_a.vote_rank == 2 );
      BOOST_REQUIRE( vote_c.account == "bob" );
      BOOST_REQUIRE( vote_c.producer == "candice" );
      BOOST_REQUIRE( vote_c.vote_rank == 1 );
      BOOST_REQUIRE( bob.producer_vote_count == 2 );
      BOOST_REQUIRE( producer_a.vote_count == 1 );
      BOOST_REQUIRE( producer_c.vote_count == 1 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: additional producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel producer vote" );

      vote.approved = false;

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_vote_object* bob_candice_vote_ptr = db.find_producer_vote( account_name_type( "bob" ), account_name_type( "candice" ) );

      BOOST_REQUIRE( bob_candice_vote_ptr == nullptr );
      BOOST_REQUIRE( bob.producer_vote_count == 1 );
      BOOST_REQUIRE( producer_c.vote_count == 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel additional producer vote" );

      vote.producer = "alice";

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const producer_vote_object* bob_alice_vote_ptr = db.find_producer_vote( account_name_type( "bob" ), account_name_type( "alice" ) );

      BOOST_REQUIRE( bob_alice_vote_ptr == nullptr );
      BOOST_REQUIRE( bob.producer_vote_count == 0 );
      BOOST_REQUIRE( producer_a.vote_count == 0 );

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

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting for a non-existent account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting for an account that is not a producer" );

      tx.operations.clear();
      tx.signatures.clear();

      vote.producer = "elon";

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

      ACTORS( (alice)(bob)(candice)(sam)(dan)(elon) )

      fund_liquid( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "sam", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "sam", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_update_proxy_operation proxy;

      proxy.account = "bob";
      proxy.proxy = "alice";
      proxy.validate();

      signed_transaction tx;

      tx.operations.push_back( proxy );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );    // bob -> alice

      BOOST_REQUIRE( bob.proxy == "alice" );
      BOOST_REQUIRE( !bob.proxied.size() );
      BOOST_REQUIRE( alice.proxied.size() );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *alice.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( alice, equity_price ) == db.get_voting_power( bob, equity_price ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: changing proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      proxy.proxy = "sam";

      tx.operations.push_back( proxy );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );     // bob -> sam

      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( !bob.proxied.size() );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( !alice.proxied.size() );
      BOOST_REQUIRE( sam.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: changing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when changing proxy to existing proxy" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when changing proxy to existing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandparent proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      proxy.account = "sam";
      proxy.proxy = "dan";
      proxy.validate();
      
      tx.operations.push_back( proxy );
      tx.sign( sam_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );     // bob->sam->dan

      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( *sam.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) + db.get_voting_power( bob, equity_price ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandparent proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandchild proxy" );

      // alice->sam
      // bob->sam->dan

      tx.operations.clear();
      tx.signatures.clear();

      proxy.account = "alice";
      proxy.proxy = "sam";
      proxy.validate();
      
      tx.operations.push_back( proxy );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.proxy == "sam" );
      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( sam.proxied.size() == 2 );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( alice, equity_price ) + db.get_voting_power( bob, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( alice, equity_price ) + db.get_voting_power( bob, equity_price ) + db.get_voting_power( sam, equity_price ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandchild proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing a grandchild proxy" );

      tx.operations.clear();
      tx.signatures.clear();

      proxy.account = "bob";
      proxy.proxy = PROXY_TO_SELF_ACCOUNT;
      
      tx.operations.push_back( proxy );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );    // alice->sam->dan

      BOOST_REQUIRE( alice.proxy == "sam" );
      BOOST_REQUIRE( bob.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == "alice" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( alice, equity_price ) + db.get_voting_power( sam, equity_price ) );
      
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
      
      ACTORS( (alice)(candice)(sam)(dan) );

      fund_liquid( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_create_operation acc_create;       // Creating account bob with alice as recovery account

      acc_create.registrar = "alice";
      acc_create.referrer = "alice";
      acc_create.recovery_account = "alice";
      acc_create.reset_account = "alice";
      acc_create.new_account_name = "bob";
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

      const account_authority_object& bob_auth = db.get< account_authority_object, by_account >( account_name_type( "bob" ) );
      BOOST_REQUIRE( bob_auth.owner_auth == acc_create.owner_auth );

      // Changing bob's owner authority

      generate_blocks( bob_auth.last_owner_update + OWNER_UPDATE_LIMIT );

      account_update_operation acc_update;

      acc_update.account = "bob";
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

      account_request_recovery_operation request;

      request.recovery_account = "alice";
      request.account_to_recover = "bob";
      request.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );
      request.validate();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_auth.owner_auth == acc_update.owner_auth );

      generate_blocks( now() + OWNER_UPDATE_LIMIT );

      tx.operations.clear();
      tx.signatures.clear();

      account_recover_operation recover;
      
      recover.account_to_recover = "bob";
      recover.new_owner_authority = request.new_owner_authority;
      recover.recent_owner_authority = acc_create.owner_auth;
      recover.validate();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      shared_authority owner1 = db.get< account_authority_object, by_account >( account_name_type( "bob" ) ).owner_auth;

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

      shared_authority owner2 = db.get< account_authority_object, by_account >(account_name_type( "bob" )).owner_auth;
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

      shared_authority owner3 = db.get< account_authority_object, by_account >(account_name_type( "bob" )).owner_auth;
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

      shared_authority owner4 = db.get< account_authority_object, by_account >(account_name_type( "bob" )).owner_auth;
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

      generate_blocks(10);

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

      shared_authority owner5 = db.get< account_authority_object, by_account >(account_name_type( "bob" )).owner_auth;
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
      generate_blocks(10);

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

      shared_authority owner6 = db.get< account_authority_object, by_account >(account_name_type( "bob" )).owner_auth;
      BOOST_REQUIRE( owner6 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      recover.recent_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      tx.sign( generate_private_key( "last key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );     // foo bar key has not yet expired

      shared_authority owner7 = db.get< account_authority_object, by_account >(account_name_type( "bob" )).owner_auth;
      BOOST_REQUIRE( owner7 == authority( 1, generate_private_key( "last key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Expiring owner authority history" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: change recovery account" );

      auto account_recovery_update = [&]( const std::string& account_to_recover, const std::string& new_recovery_account )
      {
         account_recovery_update_operation recovery;

         recovery.account_to_recover = account_to_recover;
         recovery.new_recovery_account = new_recovery_account;

         signed_transaction tx;

         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      auto account_recover = [&]( const std::string& account_to_recover, const fc::ecc::private_key& new_owner_key, const fc::ecc::private_key& recent_owner_key )
      {
         account_recover_operation recovery;

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

      auto request_recovery = [&]( const std::string& recovery_account, const fc::ecc::private_key& recovery_account_key, const std::string& account_to_recover, const public_key_type& new_owner_key )
      {
         account_request_recovery_operation recovery;

         recovery.recovery_account = recovery_account;
         recovery.account_to_recover = account_to_recover;
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
      REQUIRE_THROW( account_recovery_update( "alice", "nobody" ), fc::exception );
      REQUIRE_THROW( account_recovery_update( "haxer", "sam" ), fc::exception );
      REQUIRE_THROW( account_recovery_update( "haxer", "nobody" ), fc::exception );
      account_recovery_update( "alice", "sam" );

      fc::ecc::private_key alice_priv1 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k1" ) );
      fc::ecc::private_key alice_priv2 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k2" ) );
      public_key_type alice_pub1 = public_key_type( alice_priv1.get_public_key() );

      generate_blocks( now() + OWNER_AUTH_RECOVERY_PERIOD, true );

      // cannot request account recovery until recovery account is approved
      REQUIRE_THROW( request_recovery( "sam", sam_private_active_key, "alice", alice_pub1 ), fc::exception );

      generate_block();

      // cannot finish account recovery until requested
      REQUIRE_THROW( account_recover( "alice", alice_priv1, alice_private_owner_key ), fc::exception );

      // do the request
      request_recovery( "sam", sam_private_active_key, "alice", alice_pub1 );

      // can't recover with the current owner key
      REQUIRE_THROW( account_recover( "alice", alice_priv1, alice_private_owner_key ), fc::exception );
      
      // unless we change it!
      change_owner( "alice", alice_private_owner_key, public_key_type( alice_priv2.get_public_key() ) );
      account_recover( "alice", alice_priv1, alice_private_owner_key );

      BOOST_TEST_MESSAGE( "│   ├── Passed: account_recovery_update_operation" );

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

      ACTORS( (alice)(candice)(dan) );

      fund_liquid( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_create_operation acc_create;

      acc_create.registrar = "alice";
      acc_create.referrer = "alice";
      acc_create.recovery_account = "alice";
      acc_create.reset_account = "alice";
      acc_create.new_account_name = "bob";
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

      const account_authority_object& bob_auth = db.get< account_authority_object, by_account >( account_name_type( "bob" ) );

      BOOST_REQUIRE( bob_auth.owner_auth == acc_create.owner_auth );

      const account_object& bob_acc = db.get_account( account_name_type( "bob" ) );

      account_reset_operation reset;

      reset.reset_account = "alice";
      reset.account_to_reset = "bob";
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );
      reset.validate();

      tx.operations.push_back( reset );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet

      generate_blocks( now() + fc::days( bob_acc.reset_delay_days ), true );
      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( reset );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );    // reset now valid
      
      tx.operations.clear();
      tx.signatures.clear();

      const account_authority_object& new_bob_auth = db.get< account_authority_object, by_account >( account_name_type( "bob" ) );

      BOOST_REQUIRE( new_bob_auth.owner_auth == reset.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account reset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: set new reset account" );

      account_reset_update_operation set;

      set.account = "bob";
      set.new_reset_account = "candice";
      set.validate();

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_object& bob_new_acc = db.get_account( account_name_type( "bob" ) );
      
      BOOST_REQUIRE( bob_new_acc.reset_account == set.new_reset_account );

      BOOST_TEST_MESSAGE( "│   ├── Passed: set new reset account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional account reset" );

      reset.reset_account = "candice";
      reset.account_to_reset = "bob";
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key_1" ).get_public_key(), 1 );
      reset.validate();

      tx.operations.push_back( reset );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet because of bob's set reset op

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::days( bob_new_acc.reset_delay_days ), true );
      generate_blocks( 10 );

      tx.operations.push_back( reset );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );    // reset now valid

      BOOST_REQUIRE( new_bob_auth.owner_auth == reset.new_owner_authority );

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

BOOST_AUTO_TEST_CASE( account_decline_voting_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: DECLINE VOTING RIGHTS" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: decline voting rights request" );

      ACTORS( (alice)(bob) );

      fund_liquid( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_liquid( "bob", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( now() + fc::days(2), true );

      account_decline_voting_operation decline;

      decline.account = "alice";
      decline.declined = true;
      decline.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& req_idx = db.get_index< account_decline_voting_request_index >().indices().get< by_account >();
      auto req_itr = req_idx.find( account_name_type( "alice" ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->effective_date == now() + DECLINE_VOTING_RIGHTS_DURATION );

      BOOST_TEST_MESSAGE( "│   ├── Passed: decline voting rights request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure revoking voting rights with existing request" );

      generate_block();

      tx.operations.push_back( decline );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure revoking voting rights with existing request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success cancelling a request" );

      decline.declined = false;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      tx.operations.clear();
      tx.signatures.clear();

      req_itr = req_idx.find( account_name_type( "alice" ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed

      BOOST_TEST_MESSAGE( "│   ├── Passed: success cancelling a request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure cancelling a request that doesn't exist" );

      generate_block();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );   // Can't cancel again

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure cancelling a request that doesn't exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account can vote during waiting period" );

      decline.declined = true;

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( alice.can_vote );   // Alice can still vote

      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      comment_operation comment;

      comment.editor = "alice";
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "test";
      comment.community = INIT_PUBLIC_COMMUNITY;
      comment.title = "test";
      comment.body = "test";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.language = "en";
      comment.public_key = "";
      comment.interface = INIT_ACCOUNT;
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.json = "{ \"valid\": true }";
      comment.latitude = 37.8136;
      comment.longitude = 144.9631;
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = "article";
      options.reach = "tag";
      options.rating = 1;

      comment.options = options;
      comment.validate();

      comment_vote_operation vote;

      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;
      vote.validate();

      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& comment_idx = db.get_index< comment_index >().indices().get< by_permlink >();
      const auto& comment_vote_idx = db.get_index< comment_vote_index >().indices().get< by_voter_comment >();
      const auto& producer_vote_idx = db.get_index< producer_vote_index >().indices().get< by_producer_account >();

      auto comment_itr = comment_idx.find( boost::make_tuple( account_name_type( "alice" ), string( "test" ) ) );
      auto comment_vote_itr = comment_vote_idx.find( boost::make_tuple( account_name_type( "alice" ), comment_itr->id ) );
      auto producer_vote_itr = producer_vote_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "alice" ) ) );

      BOOST_REQUIRE( comment_itr->author == "alice" );
      BOOST_REQUIRE( comment_vote_itr->voter == "alice" );
      BOOST_REQUIRE( producer_vote_itr->account == "alice" );

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account can vote during waiting period" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account cannot vote after request is processed" );

      generate_blocks( now() + DECLINE_VOTING_RIGHTS_DURATION - BLOCK_INTERVAL, true );
      generate_blocks( 10 );

      BOOST_REQUIRE( !alice.can_vote );   // Alice can no longer vote

      req_itr = req_idx.find( account_name_type( "alice" ) );
      BOOST_REQUIRE( req_itr == req_idx.end() );   // Request no longer exists

      producer_vote_itr = producer_vote_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "alice" ) ) );
      BOOST_REQUIRE( producer_vote_itr == producer_vote_idx.end() );   // Vote has been removed

      tx.operations.clear();
      tx.signatures.clear();

      account_producer_vote_operation producer_vote;

      producer_vote.account = "alice";
      producer_vote.producer = "alice";
      producer_vote.vote_rank = 1;
      producer_vote.approved = true;
      producer_vote.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account cannot vote after request is processed" );

      BOOST_TEST_MESSAGE( "├── Passed: DECLINE VOTING RIGHTS" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_connection_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT CONNECTION " );

      BOOST_TEST_MESSAGE( "│   ├── Testing: create connection" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      string alice_private_connection_wif = graphene::utilities::key_to_wif( alice_private_connection_key );
      string bob_private_connection_wif = graphene::utilities::key_to_wif( bob_private_connection_key );
      string alice_private_friend_wif = graphene::utilities::key_to_wif( alice_private_friend_key );
      string bob_private_friend_wif = graphene::utilities::key_to_wif( bob_private_friend_key );
      string alice_private_companion_wif = graphene::utilities::key_to_wif( alice_private_companion_key );
      string bob_private_companion_wif = graphene::utilities::key_to_wif( bob_private_companion_key );

      signed_transaction tx;

      account_connection_operation connection;

      connection.account = "bob";
      connection.connecting_account = "alice";
      connection.connection_type = "connection";
      connection.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      connection.encrypted_key = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#" ) + bob_private_connection_wif );
      connection.message = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#Hello" ) );
      connection.json = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#{ \"valid\": true }" ) );
      connection.connected = true;
      connection.validate();

      tx.set_reference_block( db.head_block_id() );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( connection );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& con_idx = db.get_index< account_connection_index >().indices().get< by_accounts >();
      auto con_itr = con_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "bob" ), connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( to_string( con_itr->message_b ) == connection.message );
      BOOST_REQUIRE( to_string( con_itr->json_b ) == connection.json );
      BOOST_REQUIRE( con_itr->approved_b == true );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == connection.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == connection.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      connection.account = "alice";
      connection.connecting_account = "bob";
      connection.encrypted_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#" ) + alice_private_connection_wif );
      connection.message = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#Hello" ) );
      connection.json = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#{ \"valid\": true }" ) );
      connection.connected = true;
      connection.validate();

      tx.operations.push_back( connection );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      con_itr = con_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "bob" ), connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( to_string( con_itr->message_a ) == connection.message );
      BOOST_REQUIRE( to_string( con_itr->json_a ) == connection.json );
      BOOST_REQUIRE( con_itr->approved_a == true );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == connection.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == connection.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );
      BOOST_REQUIRE( con_itr->approved() == true );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create connection" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create account verification" );

      account_verification_operation verification;
      
      verification.verifier_account = "alice";
      verification.verified_account = "bob";
      verification.shared_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      verification.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( verification );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const account_verification_object alice_verification = db.get_account_verification( account_name_type( "alice" ), account_name_type( "bob" ) );

      BOOST_REQUIRE( verification.verifier_account == alice_verification.verifier_account );
      BOOST_REQUIRE( verification.verified_account == alice_verification.verified_account );
      BOOST_REQUIRE( verification.shared_image == to_string( alice_verification.shared_image ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create account verification" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create friend connection" );

      generate_blocks( now() + fc::days(8) );

      connection.account = "bob";
      connection.connecting_account = "alice";
      connection.connection_type = "friend";
      connection.connection_id = "ab946f00-2373-4096-ae22-1a1a2414f6df";
      connection.encrypted_key = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#" ) + bob_private_friend_wif );
      connection.message = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#Hello" ) );
      connection.json = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#{ \"valid\": true }" ) );
      connection.connected = true;
      connection.validate();

      tx.set_reference_block( db.head_block_id() );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( connection );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      con_itr = con_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "bob" ), connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( to_string( con_itr->message_b ) == connection.message );
      BOOST_REQUIRE( to_string( con_itr->json_b ) == connection.json );
      BOOST_REQUIRE( con_itr->approved_b == true );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == connection.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == connection.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      connection.account = "alice";
      connection.connecting_account = "bob";
      connection.connection_type = "friend";
      connection.encrypted_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#" ) + alice_private_friend_wif );
      connection.message = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#Hello" ) );
      connection.json = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#{ \"valid\": true }" ) );
      connection.validate();

      tx.operations.push_back( connection );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      con_itr = con_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "bob" ), connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( to_string( con_itr->message_a ) == connection.message );
      BOOST_REQUIRE( to_string( con_itr->json_a ) == connection.json );
      BOOST_REQUIRE( con_itr->approved_a == true );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == connection.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == connection.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create friend connection" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create companion connection" );

      generate_blocks( now() + fc::days(8) );

      connection.account = "bob";
      connection.connecting_account = "alice";
      connection.connection_type = "companion";
      connection.connection_id = "be07ef0a-4d27-45a5-a3c8-480c4fbad1ac";
      connection.encrypted_key = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#" ) + bob_private_companion_wif );
      connection.message = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#Hello" ) );
      connection.json = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, string( "#{ \"valid\": true }" ) );
      connection.connected = true;
      connection.validate();

      tx.set_reference_block( db.head_block_id() );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( connection );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      con_itr = con_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "bob" ), connection_tier_type::COMPANION ) );

      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::COMPANION );
      BOOST_REQUIRE( to_string( con_itr->message_b ) == connection.message );
      BOOST_REQUIRE( to_string( con_itr->json_b ) == connection.json );
      BOOST_REQUIRE( con_itr->approved_b == true );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == connection.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == connection.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      connection.account = "alice";
      connection.connecting_account = "bob";
      connection.connection_type = "companion";
      connection.encrypted_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#" ) + alice_private_companion_wif );
      connection.message = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#Hello" ) );
      connection.json = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, string( "#{ \"valid\": true }" ) );
      connection.validate();

      tx.operations.push_back( connection );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      con_itr = con_idx.find( boost::make_tuple( account_name_type( "alice" ), account_name_type( "bob" ), connection_tier_type::COMPANION ) );

      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::COMPANION );
      BOOST_REQUIRE( to_string( con_itr->message_a ) == connection.message );
      BOOST_REQUIRE( to_string( con_itr->json_a ) == connection.json );
      BOOST_REQUIRE( con_itr->approved_a == true );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == connection.encrypted_key );
      BOOST_REQUIRE( to_string( con_itr->connection_id ) == connection.connection_id );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create companion connection" );

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

      account_follow_operation follow;

      follow.follower = "alice";
      follow.following = "bob";
      follow.interface = INIT_ACCOUNT;
      follow.added = true;
      follow.followed = true;
      follow.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( account_name_type( "alice" ) );
      const account_following_object& following_b = db.get_account_following( account_name_type( "bob" ) );

      BOOST_REQUIRE( following_a.is_following( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_follower( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account follow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: mutual following" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.follower = "bob";
      follow.following = "alice";

      tx.operations.push_back( follow );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( following_a.is_follower( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_following( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( following_a.is_mutual( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_mutual( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: mutual following" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when following already followed account" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.follower = "bob";
      follow.following = "alice";

      tx.operations.push_back( follow );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when following already followed account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: unfollowing" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.follower = "bob";
      follow.following = "alice";
      follow.added = false;
      follow.followed = true;

      tx.operations.push_back( follow );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      follow.follower = "alice";
      follow.following = "bob";
      follow.added = false;
      follow.followed = true;

      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( !following_a.is_follower( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_following( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( !following_a.is_following( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_follower( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( !following_a.is_mutual( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_mutual( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: unfollowing" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: filtering" );

      follow.follower = "alice";
      follow.following = "bob";
      follow.added = true;
      follow.followed = false;

      tx.operations.push_back( follow );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( following_a.is_filtered( account_name_type( "bob" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: filtering" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT FOLLOW" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_follow_tag_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TAG FOLLOW" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal tag follow" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_follow_tag_operation follow;

      follow.follower = "alice";
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

      const account_following_object& following_a = db.get_account_following( account_name_type( "alice" ) );
      const account_tag_following_object& following_t = db.get_account_tag_following( tag_name_type( "test" ) );
      
      BOOST_REQUIRE( following_a.is_followed_tag( tag_name_type( "test" ) ) );
      BOOST_REQUIRE( following_t.is_follower( account_name_type( "alice" ) ) );
      
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
      BOOST_REQUIRE( !following_t.is_follower( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: unfollowing" );

      BOOST_TEST_MESSAGE( "├── Passed: TAG FOLLOW" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_activity_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACTIVITY REWARD" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal activity reward claim process" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn) );

      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key );
      producer_vote( "alice", alice_private_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key );
      producer_vote( "bob", bob_private_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key );
      producer_vote( "candice", candice_private_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key );
      producer_vote( "dan", dan_private_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key );
      producer_vote( "elon", elon_private_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key );
      producer_vote( "fred", fred_private_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key );
      producer_vote( "george", george_private_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key );
      producer_vote( "haz", haz_private_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key );
      producer_vote( "isabelle", isabelle_private_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key );
      producer_vote( "jayme", jayme_private_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key );
      producer_vote( "kathryn", kathryn_private_owner_key );

      signed_transaction tx;

      stake_asset_operation stake;

      stake.from = "alice";
      stake.to = "alice";
      stake.amount = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY );
      stake.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( stake );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + STAKE_WITHDRAW_INTERVAL, true );
      generate_block();

      comment_create( "alice", alice_private_posting_key, "alicetestpost" );
      comment_create( "bob", bob_private_posting_key, "bobtestpost" );
      comment_create( "candice", candice_private_posting_key, "candicetestpost" );

      generate_block();

      comment_vote_operation vote;

      vote.voter = "bob";
      vote.author = "alice";
      vote.permlink = "alicetestpost";
      vote.weight = PERCENT_100;
      vote.interface = INIT_ACCOUNT;
      vote.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "alice";
      vote.author = "bob";
      vote.permlink = "bobtestpost";

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "candice";
      vote.author = "alice";
      vote.permlink = "alicetestpost";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "dan";
      
      tx.operations.push_back( vote );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment_view_operation view;

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

      view.viewer = "alice";
      view.author = "candice";
      view.permlink = "candicetestpost";

      tx.operations.push_back( view );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.viewer = "candice";
      view.author = "alice";
      view.permlink = "alicetestpost";

      tx.operations.push_back( view );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.viewer = "dan";

      tx.operations.push_back( view );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.viewer = "elon";

      tx.operations.push_back( view );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_producer_vote_operation producer_vote;

      producer_vote.account = "alice";
      producer_vote.producer = "bob";
      producer_vote.vote_rank = 2;
      producer_vote.approved = true;
      producer_vote.validate();

      tx.operations.push_back( producer_vote );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "candice";
      producer_vote.vote_rank = 3;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "dan";
      producer_vote.vote_rank = 4;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "elon";
      producer_vote.vote_rank = 5;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "fred";
      producer_vote.vote_rank = 6;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "george";
      producer_vote.vote_rank = 7;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "haz";
      producer_vote.vote_rank = 8;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "isabelle";
      producer_vote.vote_rank = 9;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "jayme";
      producer_vote.vote_rank = 10;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "kathryn";
      producer_vote.vote_rank = 11;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_activity_operation activity;

      activity.account = "alice";
      activity.permlink = "alicetestpost";
      activity.interface = INIT_ACCOUNT;
      activity.validate();

      tx.operations.push_back( activity );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const asset_reward_fund_object& rfo = db.get_reward_fund( SYMBOL_COIN );
      
      BOOST_REQUIRE( rfo.recent_activity_claims == uint128_t( BLOCKCHAIN_PRECISION.value * 2 ) );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal activity reward claim process" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when claiming again" );

      tx.operations.push_back( activity );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when claiming again" );

      BOOST_TEST_MESSAGE( "├── Passed: ACTIVITY REWARD" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()