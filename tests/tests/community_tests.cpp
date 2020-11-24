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


BOOST_FIXTURE_TEST_SUITE( community_operation_tests, clean_database_fixture );



   //=========================//
   // === Community Tests === //
   //=========================//



BOOST_AUTO_TEST_CASE( community_create_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY CREATE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful community creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( now() + fc::days(2), true );

      public_key_type aliceopencommunity_public_member_key = get_public_key( "aliceopencommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_moderator_key = get_public_key( "aliceopencommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_admin_key = get_public_key( "aliceopencommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_secure_key = get_public_key( "aliceopencommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_standard_premium_key = get_public_key( "aliceopencommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_mid_premium_key = get_public_key( "aliceopencommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_top_premium_key = get_public_key( "aliceopencommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type aliceopencommunity_private_member_key = get_private_key( "aliceopencommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_moderator_key = get_private_key( "aliceopencommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_admin_key = get_private_key( "aliceopencommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_secure_key = get_private_key( "aliceopencommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_standard_premium_key = get_private_key( "aliceopencommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_mid_premium_key = get_private_key( "aliceopencommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_top_premium_key = get_private_key( "aliceopencommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type bobpubliccommunity_public_member_key = get_public_key( "bobpubliccommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_moderator_key = get_public_key( "bobpubliccommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_admin_key = get_public_key( "bobpubliccommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_secure_key = get_public_key( "bobpubliccommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_standard_premium_key = get_public_key( "bobpubliccommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_mid_premium_key = get_public_key( "bobpubliccommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_top_premium_key = get_public_key( "bobpubliccommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type bobpubliccommunity_private_member_key = get_private_key( "bobpubliccommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_moderator_key = get_private_key( "bobpubliccommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_admin_key = get_private_key( "bobpubliccommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_secure_key = get_private_key( "bobpubliccommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_standard_premium_key = get_private_key( "bobpubliccommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_mid_premium_key = get_private_key( "bobpubliccommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_top_premium_key = get_private_key( "bobpubliccommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type candiceprivatecommunity_public_member_key = get_public_key( "candiceprivatecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_moderator_key = get_public_key( "candiceprivatecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_admin_key = get_public_key( "candiceprivatecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_secure_key = get_public_key( "candiceprivatecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_standard_premium_key = get_public_key( "candiceprivatecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_mid_premium_key = get_public_key( "candiceprivatecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_top_premium_key = get_public_key( "candiceprivatecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type candiceprivatecommunity_private_member_key = get_private_key( "candiceprivatecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_moderator_key = get_private_key( "candiceprivatecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_admin_key = get_private_key( "candiceprivatecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_secure_key = get_private_key( "candiceprivatecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_standard_premium_key = get_private_key( "candiceprivatecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_mid_premium_key = get_private_key( "candiceprivatecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_top_premium_key = get_private_key( "candiceprivatecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type danexclusivecommunity_public_member_key = get_public_key( "danexclusivecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_moderator_key = get_public_key( "danexclusivecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_admin_key = get_public_key( "danexclusivecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_secure_key = get_public_key( "danexclusivecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_standard_premium_key = get_public_key( "danexclusivecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_mid_premium_key = get_public_key( "danexclusivecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_top_premium_key = get_public_key( "danexclusivecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type danexclusivecommunity_private_member_key = get_private_key( "danexclusivecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_moderator_key = get_private_key( "danexclusivecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_admin_key = get_private_key( "danexclusivecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_secure_key = get_private_key( "danexclusivecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_standard_premium_key = get_private_key( "danexclusivecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_mid_premium_key = get_private_key( "danexclusivecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_top_premium_key = get_private_key( "danexclusivecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type mysecondcommunity_public_member_key = get_public_key( "mysecondcommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type mysecondcommunity_public_moderator_key = get_public_key( "mysecondcommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type mysecondcommunity_public_admin_key = get_public_key( "mysecondcommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type mysecondcommunity_public_secure_key = get_public_key( "mysecondcommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type mysecondcommunity_public_standard_premium_key = get_public_key( "mysecondcommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type mysecondcommunity_public_mid_premium_key = get_public_key( "mysecondcommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type mysecondcommunity_public_top_premium_key = get_public_key( "mysecondcommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type mysecondcommunity_private_member_key = get_private_key( "mysecondcommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type mysecondcommunity_private_moderator_key = get_private_key( "mysecondcommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type mysecondcommunity_private_admin_key = get_private_key( "mysecondcommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type mysecondcommunity_private_secure_key = get_private_key( "mysecondcommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type mysecondcommunity_private_standard_premium_key = get_private_key( "mysecondcommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type mysecondcommunity_private_mid_premium_key = get_private_key( "mysecondcommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type mysecondcommunity_private_top_premium_key = get_private_key( "mysecondcommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.display_name = "Alice Open Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( aliceopencommunity_private_secure_key, aliceopencommunity_public_secure_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = false;
      create.author_permission = "all";
      create.reply_permission = "all";
      create.vote_permission = "all";
      create.view_permission = "all";
      create.share_permission = "all";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( aliceopencommunity_public_member_key );
      create.community_moderator_key = string( aliceopencommunity_public_moderator_key );
      create.community_admin_key = string( aliceopencommunity_public_admin_key );
      create.community_secure_key = string( aliceopencommunity_public_secure_key );
      create.community_standard_premium_key = string( aliceopencommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( aliceopencommunity_public_mid_premium_key );
      create.community_top_premium_key = string( aliceopencommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( create.name );

      BOOST_REQUIRE( alice_community.founder == create.founder );
      BOOST_REQUIRE( alice_community.name == create.name );
      BOOST_REQUIRE( alice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpubliccommunity";
      create.display_name = "Bob Public Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( bobpubliccommunity_private_secure_key, bobpubliccommunity_public_secure_key, bobpubliccommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = false;
      create.author_permission = "member";
      create.reply_permission = "all";
      create.vote_permission = "all";
      create.view_permission = "all";
      create.share_permission = "all";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( bobpubliccommunity_public_member_key );
      create.community_moderator_key = string( bobpubliccommunity_public_moderator_key );
      create.community_admin_key = string( bobpubliccommunity_public_admin_key );
      create.community_secure_key = string( bobpubliccommunity_public_secure_key );
      create.community_standard_premium_key = string( bobpubliccommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( bobpubliccommunity_public_mid_premium_key );
      create.community_top_premium_key = string( bobpubliccommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( create.name );

      BOOST_REQUIRE( bob_community.founder == create.founder );
      BOOST_REQUIRE( bob_community.name == create.name );
      BOOST_REQUIRE( bob_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.display_name = "Candice Private Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = true;
      create.author_permission = "member";
      create.reply_permission = "member";
      create.vote_permission = "member";
      create.view_permission = "member";
      create.share_permission = "member";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( candiceprivatecommunity_public_member_key );
      create.community_moderator_key = string( candiceprivatecommunity_public_moderator_key );
      create.community_admin_key = string( candiceprivatecommunity_public_admin_key );
      create.community_secure_key = string( candiceprivatecommunity_public_secure_key );
      create.community_standard_premium_key = string( candiceprivatecommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( candiceprivatecommunity_public_mid_premium_key );
      create.community_top_premium_key = string( candiceprivatecommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( create.name );

      BOOST_REQUIRE( candice_community.founder == create.founder );
      BOOST_REQUIRE( candice_community.name == create.name );
      BOOST_REQUIRE( candice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.display_name = "Dan Exclusive Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = true;
      create.author_permission = "member";
      create.reply_permission = "member";
      create.vote_permission = "member";
      create.view_permission = "member";
      create.share_permission = "member";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "administrator";
      create.request_permission = "member";
      create.remove_permission = "administrator";
      create.community_member_key = string( danexclusivecommunity_public_member_key );
      create.community_moderator_key = string( danexclusivecommunity_public_moderator_key );
      create.community_admin_key = string( danexclusivecommunity_public_admin_key );
      create.community_secure_key = string( danexclusivecommunity_public_secure_key );
      create.community_standard_premium_key = string( danexclusivecommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( danexclusivecommunity_public_mid_premium_key );
      create.community_top_premium_key = string( danexclusivecommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( create.name );

      BOOST_REQUIRE( dan_community.founder == create.founder );
      BOOST_REQUIRE( dan_community.name == create.name );
      BOOST_REQUIRE( dan_community.created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful community creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_CREATE_INTERVAL - BLOCK_INTERVAL );

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "mysecondcommunity";
      create.display_name = "My Second Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( mysecondcommunity_private_secure_key, mysecondcommunity_public_secure_key, mysecondcommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = false;
      create.author_permission = "all";
      create.reply_permission = "all";
      create.vote_permission = "all";
      create.view_permission = "all";
      create.share_permission = "all";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( mysecondcommunity_public_member_key );
      create.community_moderator_key = string( mysecondcommunity_public_moderator_key );
      create.community_admin_key = string( mysecondcommunity_public_admin_key );
      create.community_secure_key = string( mysecondcommunity_public_secure_key );
      create.community_standard_premium_key = string( mysecondcommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( mysecondcommunity_public_mid_premium_key );
      create.community_top_premium_key = string( mysecondcommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();
      create.validate();

      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success after MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks(10);

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: success after MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community name already exists" );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "elon";
      create.founder = "elon";
      create.name = "aliceopencommunity";
      create.display_name = "My Second Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( aliceopencommunity_private_secure_key, aliceopencommunity_public_secure_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community name already exists" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY CREATE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( community_update_operation_test )
{
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY UPDATE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder community update sequence" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( now() + fc::days(2), true );

      public_key_type aliceopencommunity_public_member_key = get_public_key( "aliceopencommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_moderator_key = get_public_key( "aliceopencommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_admin_key = get_public_key( "aliceopencommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_secure_key = get_public_key( "aliceopencommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_standard_premium_key = get_public_key( "aliceopencommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_mid_premium_key = get_public_key( "aliceopencommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_top_premium_key = get_public_key( "aliceopencommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type aliceopencommunity_private_member_key = get_private_key( "aliceopencommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_moderator_key = get_private_key( "aliceopencommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_admin_key = get_private_key( "aliceopencommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_secure_key = get_private_key( "aliceopencommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_standard_premium_key = get_private_key( "aliceopencommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_mid_premium_key = get_private_key( "aliceopencommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_top_premium_key = get_private_key( "aliceopencommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type bobpubliccommunity_public_member_key = get_public_key( "bobpubliccommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_moderator_key = get_public_key( "bobpubliccommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_admin_key = get_public_key( "bobpubliccommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_secure_key = get_public_key( "bobpubliccommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_standard_premium_key = get_public_key( "bobpubliccommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_mid_premium_key = get_public_key( "bobpubliccommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_top_premium_key = get_public_key( "bobpubliccommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type bobpubliccommunity_private_member_key = get_private_key( "bobpubliccommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_moderator_key = get_private_key( "bobpubliccommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_admin_key = get_private_key( "bobpubliccommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_secure_key = get_private_key( "bobpubliccommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_standard_premium_key = get_private_key( "bobpubliccommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_mid_premium_key = get_private_key( "bobpubliccommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_top_premium_key = get_private_key( "bobpubliccommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type candiceprivatecommunity_public_member_key = get_public_key( "candiceprivatecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_moderator_key = get_public_key( "candiceprivatecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_admin_key = get_public_key( "candiceprivatecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_secure_key = get_public_key( "candiceprivatecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_standard_premium_key = get_public_key( "candiceprivatecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_mid_premium_key = get_public_key( "candiceprivatecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_top_premium_key = get_public_key( "candiceprivatecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type candiceprivatecommunity_private_member_key = get_private_key( "candiceprivatecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_moderator_key = get_private_key( "candiceprivatecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_admin_key = get_private_key( "candiceprivatecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_secure_key = get_private_key( "candiceprivatecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_standard_premium_key = get_private_key( "candiceprivatecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_mid_premium_key = get_private_key( "candiceprivatecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_top_premium_key = get_private_key( "candiceprivatecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type danexclusivecommunity_public_member_key = get_public_key( "danexclusivecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_moderator_key = get_public_key( "danexclusivecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_admin_key = get_public_key( "danexclusivecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_secure_key = get_public_key( "danexclusivecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_standard_premium_key = get_public_key( "danexclusivecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_mid_premium_key = get_public_key( "danexclusivecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_top_premium_key = get_public_key( "danexclusivecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type danexclusivecommunity_private_member_key = get_private_key( "danexclusivecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_moderator_key = get_private_key( "danexclusivecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_admin_key = get_private_key( "danexclusivecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_secure_key = get_private_key( "danexclusivecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_standard_premium_key = get_private_key( "danexclusivecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_mid_premium_key = get_private_key( "danexclusivecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_top_premium_key = get_private_key( "danexclusivecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.display_name = "Alice Open Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( aliceopencommunity_private_secure_key, aliceopencommunity_public_secure_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = false;
      create.author_permission = "all";
      create.reply_permission = "all";
      create.vote_permission = "all";
      create.view_permission = "all";
      create.share_permission = "all";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( aliceopencommunity_public_member_key );
      create.community_moderator_key = string( aliceopencommunity_public_moderator_key );
      create.community_admin_key = string( aliceopencommunity_public_admin_key );
      create.community_secure_key = string( aliceopencommunity_public_secure_key );
      create.community_standard_premium_key = string( aliceopencommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( aliceopencommunity_public_mid_premium_key );
      create.community_top_premium_key = string( aliceopencommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpubliccommunity";
      create.display_name = "Bob Public Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( bobpubliccommunity_private_secure_key, bobpubliccommunity_public_secure_key, bobpubliccommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = false;
      create.author_permission = "member";
      create.reply_permission = "all";
      create.vote_permission = "all";
      create.view_permission = "all";
      create.share_permission = "all";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( bobpubliccommunity_public_member_key );
      create.community_moderator_key = string( bobpubliccommunity_public_moderator_key );
      create.community_admin_key = string( bobpubliccommunity_public_admin_key );
      create.community_secure_key = string( bobpubliccommunity_public_secure_key );
      create.community_standard_premium_key = string( bobpubliccommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( bobpubliccommunity_public_mid_premium_key );
      create.community_top_premium_key = string( bobpubliccommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.display_name = "Candice Private Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = true;
      create.author_permission = "member";
      create.reply_permission = "member";
      create.vote_permission = "member";
      create.view_permission = "member";
      create.share_permission = "member";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( candiceprivatecommunity_public_member_key );
      create.community_moderator_key = string( candiceprivatecommunity_public_moderator_key );
      create.community_admin_key = string( candiceprivatecommunity_public_admin_key );
      create.community_secure_key = string( candiceprivatecommunity_public_secure_key );
      create.community_standard_premium_key = string( candiceprivatecommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( candiceprivatecommunity_public_mid_premium_key );
      create.community_top_premium_key = string( candiceprivatecommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.display_name = "Dan Exclusive Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = true;
      create.author_permission = "member";
      create.reply_permission = "member";
      create.vote_permission = "member";
      create.view_permission = "member";
      create.share_permission = "member";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "administrator";
      create.request_permission = "member";
      create.remove_permission = "administrator";
      create.community_member_key = string( danexclusivecommunity_public_member_key );
      create.community_moderator_key = string( danexclusivecommunity_public_moderator_key );
      create.community_admin_key = string( danexclusivecommunity_public_admin_key );
      create.community_secure_key = string( danexclusivecommunity_public_secure_key );
      create.community_standard_premium_key = string( danexclusivecommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( danexclusivecommunity_public_mid_premium_key );
      create.community_top_premium_key = string( danexclusivecommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = "aliceopencommunity";
      comment.public_key = string();
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "ipsum";
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

      tx.operations.push_back( comment );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "bob";
      comment.author = "bob";
      comment.public_key = string();
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "candice";
      comment.author = "candice";
      comment.community = "candiceprivatecommunity";
      comment.public_key = string( candiceprivatecommunity_public_member_key );
      
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "dan";
      comment.author = "dan";
      comment.community = "danexclusivecommunity";
      comment.public_key = string( danexclusivecommunity_public_member_key );
      
      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_UPDATE_INTERVAL + fc::minutes(1) );

      community_update_operation update;

      update.signatory = "alice";
      update.account = "alice";
      update.community = "aliceopencommunity";
      update.display_name = "Alice's Updated Open Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( aliceopencommunity_private_secure_key, aliceopencommunity_public_secure_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "alice";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = false;
      update.author_permission = "all";
      update.reply_permission = "all";
      update.vote_permission = "all";
      update.view_permission = "all";
      update.share_permission = "all";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "member";
      update.request_permission = "all";
      update.remove_permission = "administrator";
      update.community_member_key = string( aliceopencommunity_public_member_key );
      update.community_moderator_key = string( aliceopencommunity_public_moderator_key );
      update.community_admin_key = string( aliceopencommunity_public_admin_key );
      update.community_secure_key = string( aliceopencommunity_public_secure_key );
      update.community_standard_premium_key = string( aliceopencommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( aliceopencommunity_public_mid_premium_key );
      update.community_top_premium_key = string( aliceopencommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( update.community );

      BOOST_REQUIRE( alice_community.founder == update.account );
      BOOST_REQUIRE( alice_community.name == update.community );
      BOOST_REQUIRE( to_string( alice_community.details ) == update.details );
      BOOST_REQUIRE( to_string( alice_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "bob";
      update.account = "bob";
      update.community = "bobpubliccommunity";
      update.display_name = "Bob's Updated Public Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( bobpubliccommunity_private_secure_key, bobpubliccommunity_public_secure_key, bobpubliccommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "bob";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = false;
      update.author_permission = "member";
      update.reply_permission = "all";
      update.vote_permission = "all";
      update.view_permission = "all";
      update.share_permission = "all";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "member";
      update.request_permission = "all";
      update.remove_permission = "administrator";
      update.community_member_key = string( bobpubliccommunity_public_member_key );
      update.community_moderator_key = string( bobpubliccommunity_public_moderator_key );
      update.community_admin_key = string( bobpubliccommunity_public_admin_key );
      update.community_secure_key = string( bobpubliccommunity_public_secure_key );
      update.community_standard_premium_key = string( bobpubliccommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( bobpubliccommunity_public_mid_premium_key );
      update.community_top_premium_key = string( bobpubliccommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( update.community );

      BOOST_REQUIRE( bob_community.founder == update.account );
      BOOST_REQUIRE( bob_community.name == update.community );
      BOOST_REQUIRE( to_string( bob_community.details ) == update.details );
      BOOST_REQUIRE( to_string( bob_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "candice";
      update.account = "candice";
      update.community = "candiceprivatecommunity";
      update.display_name = "Candice's Updated Private Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( candiceprivatecommunity_private_secure_key, candiceprivatecommunity_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "candice";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = true;
      update.author_permission = "member";
      update.reply_permission = "member";
      update.vote_permission = "member";
      update.view_permission = "member";
      update.share_permission = "member";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "member";
      update.request_permission = "all";
      update.remove_permission = "administrator";
      update.community_member_key = string( candiceprivatecommunity_public_member_key );
      update.community_moderator_key = string( candiceprivatecommunity_public_moderator_key );
      update.community_admin_key = string( candiceprivatecommunity_public_admin_key );
      update.community_secure_key = string( candiceprivatecommunity_public_secure_key );
      update.community_standard_premium_key = string( candiceprivatecommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( candiceprivatecommunity_public_mid_premium_key );
      update.community_top_premium_key = string( candiceprivatecommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( update.community );

      BOOST_REQUIRE( candice_community.founder == update.account );
      BOOST_REQUIRE( candice_community.name == update.community );
      BOOST_REQUIRE( to_string( candice_community.details ) == update.details );
      BOOST_REQUIRE( to_string( candice_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "dan";
      update.account = "dan";
      update.community = "danexclusivecommunity";
      update.display_name = "Dan's Updated Exclusive Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( danexclusivecommunity_private_secure_key, danexclusivecommunity_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "dan";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = true;
      update.author_permission = "member";
      update.reply_permission = "member";
      update.vote_permission = "member";
      update.view_permission = "member";
      update.share_permission = "member";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "administrator";
      update.request_permission = "member";
      update.remove_permission = "administrator";
      update.community_member_key = string( danexclusivecommunity_public_member_key );
      update.community_moderator_key = string( danexclusivecommunity_public_moderator_key );
      update.community_admin_key = string( danexclusivecommunity_public_admin_key );
      update.community_secure_key = string( danexclusivecommunity_public_secure_key );
      update.community_standard_premium_key = string( danexclusivecommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( danexclusivecommunity_public_mid_premium_key );
      update.community_top_premium_key = string( danexclusivecommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( update.community );

      BOOST_REQUIRE( dan_community.founder == update.account );
      BOOST_REQUIRE( dan_community.name == update.community );
      BOOST_REQUIRE( to_string( dan_community.details ) == update.details );
      BOOST_REQUIRE( to_string( dan_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder community update sequence" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community update before MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      generate_blocks( now() + MIN_COMMUNITY_UPDATE_INTERVAL - BLOCK_INTERVAL );

      update.details = "even more updated details";

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community update before MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success after MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks(5);

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( dan_community.founder == update.account );
      BOOST_REQUIRE( dan_community.name == update.community );
      BOOST_REQUIRE( to_string( dan_community.details ) == update.details );
      BOOST_REQUIRE( to_string( dan_community.url ) == update.url );

      BOOST_TEST_MESSAGE( "│   ├── Passed: success after MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY UPDATE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( community_management_sequence_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY MANAGEMENT OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Founder create communities" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)
         (jayme)(kathryn)(leonie)(margot)(natalie) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "leonie", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      public_key_type aliceopencommunity_public_member_key = get_public_key( "aliceopencommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_moderator_key = get_public_key( "aliceopencommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_admin_key = get_public_key( "aliceopencommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_secure_key = get_public_key( "aliceopencommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_standard_premium_key = get_public_key( "aliceopencommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_mid_premium_key = get_public_key( "aliceopencommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type aliceopencommunity_public_top_premium_key = get_public_key( "aliceopencommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type aliceopencommunity_private_member_key = get_private_key( "aliceopencommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_moderator_key = get_private_key( "aliceopencommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_admin_key = get_private_key( "aliceopencommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_secure_key = get_private_key( "aliceopencommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_standard_premium_key = get_private_key( "aliceopencommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_mid_premium_key = get_private_key( "aliceopencommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type aliceopencommunity_private_top_premium_key = get_private_key( "aliceopencommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type bobpubliccommunity_public_member_key = get_public_key( "bobpubliccommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_moderator_key = get_public_key( "bobpubliccommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_admin_key = get_public_key( "bobpubliccommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_secure_key = get_public_key( "bobpubliccommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_standard_premium_key = get_public_key( "bobpubliccommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_mid_premium_key = get_public_key( "bobpubliccommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type bobpubliccommunity_public_top_premium_key = get_public_key( "bobpubliccommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type bobpubliccommunity_private_member_key = get_private_key( "bobpubliccommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_moderator_key = get_private_key( "bobpubliccommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_admin_key = get_private_key( "bobpubliccommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_secure_key = get_private_key( "bobpubliccommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_standard_premium_key = get_private_key( "bobpubliccommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_mid_premium_key = get_private_key( "bobpubliccommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type bobpubliccommunity_private_top_premium_key = get_private_key( "bobpubliccommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type candiceprivatecommunity_public_member_key = get_public_key( "candiceprivatecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_moderator_key = get_public_key( "candiceprivatecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_admin_key = get_public_key( "candiceprivatecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_secure_key = get_public_key( "candiceprivatecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_standard_premium_key = get_public_key( "candiceprivatecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_mid_premium_key = get_public_key( "candiceprivatecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type candiceprivatecommunity_public_top_premium_key = get_public_key( "candiceprivatecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type candiceprivatecommunity_private_member_key = get_private_key( "candiceprivatecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_moderator_key = get_private_key( "candiceprivatecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_admin_key = get_private_key( "candiceprivatecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_secure_key = get_private_key( "candiceprivatecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_standard_premium_key = get_private_key( "candiceprivatecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_mid_premium_key = get_private_key( "candiceprivatecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type candiceprivatecommunity_private_top_premium_key = get_private_key( "candiceprivatecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      public_key_type danexclusivecommunity_public_member_key = get_public_key( "danexclusivecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_moderator_key = get_public_key( "danexclusivecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_admin_key = get_public_key( "danexclusivecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_secure_key = get_public_key( "danexclusivecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_standard_premium_key = get_public_key( "danexclusivecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_mid_premium_key = get_public_key( "danexclusivecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      public_key_type danexclusivecommunity_public_top_premium_key = get_public_key( "danexclusivecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      private_key_type danexclusivecommunity_private_member_key = get_private_key( "danexclusivecommunity", MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_moderator_key = get_private_key( "danexclusivecommunity", MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_admin_key = get_private_key( "danexclusivecommunity", ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_secure_key = get_private_key( "danexclusivecommunity", SECURE_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_standard_premium_key = get_private_key( "danexclusivecommunity", STANDARD_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_mid_premium_key = get_private_key( "danexclusivecommunity", MID_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );
      private_key_type danexclusivecommunity_private_top_premium_key = get_private_key( "danexclusivecommunity", TOP_PREMIUM_KEY_STR, INIT_ACCOUNT_PASSWORD );

      generate_blocks( now() + fc::days(2), true );

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.display_name = "Alice Open Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( aliceopencommunity_private_secure_key, aliceopencommunity_public_secure_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = false;
      create.author_permission = "all";
      create.reply_permission = "all";
      create.vote_permission = "all";
      create.view_permission = "all";
      create.share_permission = "all";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( aliceopencommunity_public_member_key );
      create.community_moderator_key = string( aliceopencommunity_public_moderator_key );
      create.community_admin_key = string( aliceopencommunity_public_admin_key );
      create.community_secure_key = string( aliceopencommunity_public_secure_key );
      create.community_standard_premium_key = string( aliceopencommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( aliceopencommunity_public_mid_premium_key );
      create.community_top_premium_key = string( aliceopencommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_object& alice_community = db.get_community( create.name );
      const community_permission_object& alice_community_permission = db.get_community_permission( create.name );

      BOOST_REQUIRE( alice_community.founder == create.founder );
      BOOST_REQUIRE( alice_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( alice_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( alice_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( alice_community_permission.founder == create.founder );
      BOOST_REQUIRE( alice_community_permission.is_administrator( create.founder ) );
      BOOST_REQUIRE( alice_community_permission.is_moderator( create.founder ) );
      BOOST_REQUIRE( alice_community_permission.is_member( create.founder ) );
      BOOST_REQUIRE( alice_community_permission.is_subscriber( create.founder ) );

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpubliccommunity";
      create.display_name = "Bob Public Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( bobpubliccommunity_private_secure_key, bobpubliccommunity_public_secure_key, bobpubliccommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = false;
      create.author_permission = "member";
      create.reply_permission = "all";
      create.vote_permission = "all";
      create.view_permission = "all";
      create.share_permission = "all";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( bobpubliccommunity_public_member_key );
      create.community_moderator_key = string( bobpubliccommunity_public_moderator_key );
      create.community_admin_key = string( bobpubliccommunity_public_admin_key );
      create.community_secure_key = string( bobpubliccommunity_public_secure_key );
      create.community_standard_premium_key = string( bobpubliccommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( bobpubliccommunity_public_mid_premium_key );
      create.community_top_premium_key = string( bobpubliccommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_object& bob_community = db.get_community( create.name );
      const community_permission_object& bob_community_permission = db.get_community_permission( create.name );

      BOOST_REQUIRE( bob_community.founder == create.founder );
      BOOST_REQUIRE( bob_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( bob_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( bob_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( bob_community_permission.founder == create.founder );
      BOOST_REQUIRE( bob_community_permission.is_administrator( create.founder ) );
      BOOST_REQUIRE( bob_community_permission.is_moderator( create.founder ) );
      BOOST_REQUIRE( bob_community_permission.is_member( create.founder ) );
      BOOST_REQUIRE( bob_community_permission.is_subscriber( create.founder ) );

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.display_name = "Candice Private Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( candiceprivatecommunity_private_secure_key, candiceprivatecommunity_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = true;
      create.author_permission = "member";
      create.reply_permission = "member";
      create.vote_permission = "member";
      create.view_permission = "member";
      create.share_permission = "member";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "member";
      create.request_permission = "all";
      create.remove_permission = "administrator";
      create.community_member_key = string( candiceprivatecommunity_public_member_key );
      create.community_moderator_key = string( candiceprivatecommunity_public_moderator_key );
      create.community_admin_key = string( candiceprivatecommunity_public_admin_key );
      create.community_secure_key = string( candiceprivatecommunity_public_secure_key );
      create.community_standard_premium_key = string( candiceprivatecommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( candiceprivatecommunity_public_mid_premium_key );
      create.community_top_premium_key = string( candiceprivatecommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_object& candice_community = db.get_community( create.name );
      const community_permission_object& candice_community_permission = db.get_community_permission( create.name );

      BOOST_REQUIRE( candice_community.founder == create.founder );
      BOOST_REQUIRE( candice_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( candice_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( candice_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( candice_community_permission.founder == create.founder );
      BOOST_REQUIRE( candice_community_permission.is_administrator( create.founder ) );
      BOOST_REQUIRE( candice_community_permission.is_moderator( create.founder ) );
      BOOST_REQUIRE( candice_community_permission.is_member( create.founder ) );
      BOOST_REQUIRE( candice_community_permission.is_subscriber( create.founder ) );

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.display_name = "Dan Exclusive Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( danexclusivecommunity_private_secure_key, danexclusivecommunity_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      create.tags.insert( "test" );
      create.private_community = true;
      create.author_permission = "member";
      create.reply_permission = "member";
      create.vote_permission = "member";
      create.view_permission = "member";
      create.share_permission = "member";
      create.message_permission = "member";
      create.poll_permission = "administrator";
      create.event_permission = "administrator";
      create.directive_permission = "member";
      create.add_permission = "administrator";
      create.request_permission = "member";
      create.remove_permission = "administrator";
      create.community_member_key = string( danexclusivecommunity_public_member_key );
      create.community_moderator_key = string( danexclusivecommunity_public_moderator_key );
      create.community_admin_key = string( danexclusivecommunity_public_admin_key );
      create.community_secure_key = string( danexclusivecommunity_public_secure_key );
      create.community_standard_premium_key = string( danexclusivecommunity_public_standard_premium_key );
      create.community_mid_premium_key = string( danexclusivecommunity_public_mid_premium_key );
      create.community_top_premium_key = string( danexclusivecommunity_public_top_premium_key );
      create.interface = INIT_ACCOUNT;
      create.reward_currency = SYMBOL_COIN;
      create.standard_membership_price = MEMBERSHIP_FEE_BASE;
      create.mid_membership_price = MEMBERSHIP_FEE_MID;
      create.top_membership_price = MEMBERSHIP_FEE_TOP;
      create.verifiers.insert( INIT_ACCOUNT );
      create.min_verification_count = 0;
      create.max_verification_distance = 0;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      const community_object& dan_community = db.get_community( create.name );
      const community_permission_object& dan_community_permission = db.get_community_permission( create.name );

      BOOST_REQUIRE( dan_community.founder == create.founder );
      BOOST_REQUIRE( dan_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( dan_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( dan_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( dan_community_permission.founder == create.founder );
      BOOST_REQUIRE( dan_community_permission.is_administrator( create.founder ) );
      BOOST_REQUIRE( dan_community_permission.is_moderator( create.founder ) );
      BOOST_REQUIRE( dan_community_permission.is_member( create.founder ) );
      BOOST_REQUIRE( dan_community_permission.is_subscriber( create.founder ) );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Founder create communities" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Community join requests" );

      community_member_request_operation request;

      request.signatory = "fred";
      request.account = "fred";
      request.community = "aliceopencommunity";
      request.message = get_encrypted_message( fred_private_secure_key, fred_public_secure_key, aliceopencommunity_public_member_key, string( "#Hello" ) );
      request.interface = INIT_ACCOUNT;
      request.member_type = "member";
      request.requested = true;
      request.expiration = now() + fc::days(31);
      request.validate();

      tx.operations.push_back( request );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& request_idx = db.get_index< community_member_request_index >().indices().get< by_account_community_type >();
      auto request_itr = request_idx.find( boost::make_tuple( request.account, request.community, community_permission_type::MEMBER_PERMISSION ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "bobpubliccommunity";
      request.message = get_encrypted_message( fred_private_secure_key, fred_public_secure_key, bobpubliccommunity_public_member_key, string( "#Hello" ) );
   
      tx.operations.push_back( request );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      request_itr = request_idx.find( boost::make_tuple( request.account, request.community, community_permission_type::MEMBER_PERMISSION ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "candiceprivatecommunity";
      request.message = get_encrypted_message( fred_private_secure_key, fred_public_secure_key, candiceprivatecommunity_public_member_key, string( "#Hello" ) );
   
      tx.operations.push_back( request );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      request_itr = request_idx.find( boost::make_tuple( request.account, request.community, community_permission_type::MEMBER_PERMISSION ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "danexclusivecommunity";
      request.message = get_encrypted_message( fred_private_secure_key, fred_public_secure_key, danexclusivecommunity_public_member_key, string( "#Hello" ) );
   
      tx.operations.push_back( request );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // No join requests for exclusive community

      request_itr = request_idx.find( boost::make_tuple( request.account, request.community, community_permission_type::MEMBER_PERMISSION ) );

      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Community join requests" );
      
      BOOST_TEST_MESSAGE( "│   ├── Testing: Founder add community members" );

      community_member_operation community_member;

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.member = "fred";
      community_member.community = "aliceopencommunity";
      community_member.interface = INIT_ACCOUNT;
      community_member.member_type = "member";
      community_member.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_member_key ) );
      community_member.accepted = true;
      community_member.expiration = now() + fc::days(31);
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      request_itr = request_idx.find( boost::make_tuple( community_member.member, community_member.community ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";
      community_member.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_member_key, bobpubliccommunity_public_member_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_member_key ) );

      tx.operations.push_back( community_member );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      request_itr = request_idx.find( boost::make_tuple( community_member.member, community_member.community ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_member_key, candiceprivatecommunity_public_member_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_member_key ) );
      
      tx.operations.push_back( community_member );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      request_itr = request_idx.find( boost::make_tuple( community_member.member, community_member.community ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";
      community_member.encrypted_community_key = get_encrypted_message( danexclusivecommunity_private_member_key, danexclusivecommunity_public_member_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( danexclusivecommunity_private_member_key ) );
      
      tx.operations.push_back( community_member );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      request_itr = request_idx.find( boost::make_tuple( community_member.member, community_member.community ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.member = "elon";
      community_member.community = "aliceopencommunity";
      community_member.interface = INIT_ACCOUNT;
      community_member.member_type = "member";
      community_member.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_member_key ) );
      community_member.accepted = true;
      community_member.expiration = now() + fc::days(31);
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";
      community_member.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_member_key, bobpubliccommunity_public_member_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_member_key ) );

      tx.operations.push_back( community_member );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_member_key, candiceprivatecommunity_public_member_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_member_key ) );
      
      tx.operations.push_back( community_member );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";
      community_member.encrypted_community_key = get_encrypted_message( danexclusivecommunity_private_member_key, danexclusivecommunity_public_member_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( danexclusivecommunity_private_member_key ) );
      
      tx.operations.push_back( community_member );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Founder add community members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Founder add moderator" );

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.community = "aliceopencommunity";
      community_member.member = "elon";
      community_member.member_type = "moderator";
      community_member.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_moderator_key, aliceopencommunity_public_moderator_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_moderator_key ) );
      community_member.accepted = true;
      community_member.expiration = now() + fc::days(31);
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";
      community_member.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_moderator_key, bobpubliccommunity_public_moderator_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_moderator_key ) );

      tx.operations.push_back( community_member );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_moderator_key, candiceprivatecommunity_public_moderator_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_moderator_key ) );

      tx.operations.push_back( community_member );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";
      community_member.encrypted_community_key = get_encrypted_message( danexclusivecommunity_private_moderator_key, danexclusivecommunity_public_moderator_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( danexclusivecommunity_private_moderator_key ) );

      tx.operations.push_back( community_member );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Founder add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Founder vote moderator" );

      community_member_vote_operation community_member_vote;

      community_member_vote.signatory = "alice";
      community_member_vote.account = "alice";
      community_member_vote.community = "aliceopencommunity";
      community_member_vote.member = "elon";
      community_member_vote.interface = INIT_ACCOUNT;
      community_member_vote.vote_rank = 1;
      community_member_vote.approved = true;
      community_member_vote.validate();

      tx.operations.push_back( community_member_vote );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      flat_map< account_name_type, share_type > m = db.get_community_permission( community_member_vote.community ).vote_weight;
      BOOST_REQUIRE( m[ community_member_vote.member ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      community_member_vote.signatory = "bob";
      community_member_vote.account = "bob";
      community_member_vote.community = "bobpubliccommunity";

      tx.operations.push_back( community_member_vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      m = db.get_community_permission( community_member_vote.community ).vote_weight;
      BOOST_REQUIRE( m[ community_member_vote.member ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      community_member_vote.signatory = "candice";
      community_member_vote.account = "candice";
      community_member_vote.community = "candiceprivatecommunity";

      tx.operations.push_back( community_member_vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      m = db.get_community_permission( community_member_vote.community ).vote_weight;
      BOOST_REQUIRE( m[ community_member_vote.member ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      community_member_vote.signatory = "dan";
      community_member_vote.account = "dan";
      community_member_vote.community = "danexclusivecommunity";

      tx.operations.push_back( community_member_vote );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_block();

      m = db.get_community_permission( community_member_vote.community ).vote_weight;
      BOOST_REQUIRE( m[ community_member_vote.member ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Founder vote moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Founder add administrator" );

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.community = "aliceopencommunity";
      community_member.member_type = "administrator";
      community_member.member = "elon";
      community_member.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_admin_key, aliceopencommunity_public_admin_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_admin_key ) );
      community_member.accepted = true;
      community_member.expiration = now() + fc::days(31);
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_administrator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";
      community_member.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_admin_key, bobpubliccommunity_public_admin_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_admin_key ) );

      tx.operations.push_back( community_member );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_administrator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_admin_key, candiceprivatecommunity_public_admin_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_admin_key ) );

      tx.operations.push_back( community_member );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_administrator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";
      community_member.encrypted_community_key = get_encrypted_message( danexclusivecommunity_private_admin_key, danexclusivecommunity_public_admin_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( danexclusivecommunity_private_admin_key ) );

      tx.operations.push_back( community_member );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_administrator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Founder add administrator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Community Federation request success" );

      community_federation_operation federation;

      federation.signatory = "alice";
      federation.account = "alice";
      federation.federation_id = "e643533a-37af-4634-a9a9-8caa756cfbe4";
      federation.community = "aliceopencommunity";
      federation.federated_community = "bobpubliccommunity";
      federation.message = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, bobpubliccommunity_public_member_key, string( "#Hello" ) );
      federation.json = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, bobpubliccommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      federation.federation_type = "member";
      federation.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, bobpubliccommunity_public_member_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_member_key ) );
      federation.share_accounts = true;
      federation.accepted = true;
      federation.validate();

      tx.operations.push_back( federation );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& federation_idx = db.get_index< community_federation_index >().indices().get< by_communities >();
      auto federation_itr = federation_idx.find( boost::make_tuple( federation.community, federation.federated_community ) );
      BOOST_REQUIRE( federation_itr != federation_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      federation.signatory = "alice";
      federation.account = "alice";
      federation.federation_id = "c02fece2-a58b-4393-bcc6-bf2d88b060a7";
      federation.community = "aliceopencommunity";
      federation.federated_community = "candiceprivatecommunity";
      federation.message = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, candiceprivatecommunity_public_member_key, string( "#Hello" ) );
      federation.json = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      federation.federation_type = "member";
      federation.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, candiceprivatecommunity_public_member_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_member_key ) );
      federation.share_accounts = true;
      federation.accepted = true;
      federation.validate();

      tx.operations.push_back( federation );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      federation_itr = federation_idx.find( boost::make_tuple( federation.community, federation.federated_community ) );
      BOOST_REQUIRE( federation_itr != federation_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      federation.signatory = "alice";
      federation.account = "alice";
      federation.federation_id = "713890b9-ecac-42e4-bca9-0b679cdc21b3";
      federation.community = "aliceopencommunity";
      federation.federated_community = "danexclusivecommunity";
      federation.message = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, danexclusivecommunity_public_member_key, string( "#Hello" ) );
      federation.json = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      federation.federation_type = "member";
      federation.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, danexclusivecommunity_public_member_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_member_key ) );
      federation.share_accounts = true;
      federation.accepted = true;
      federation.validate();

      tx.operations.push_back( federation );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      federation_itr = federation_idx.find( boost::make_tuple( federation.community, federation.federated_community ) );
      BOOST_REQUIRE( federation_itr != federation_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Community Federation creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Community Federation mutual acceptance" );

      federation.signatory = "bob";
      federation.account = "bob";
      federation.federation_id = "e643533a-37af-4634-a9a9-8caa756cfbe4";
      federation.community = "bobpubliccommunity";
      federation.federated_community = "aliceopencommunity";
      federation.message = get_encrypted_message( bobpubliccommunity_private_member_key, bobpubliccommunity_public_member_key, aliceopencommunity_public_member_key, string( "#Hello" ) );
      federation.json = get_encrypted_message( bobpubliccommunity_private_member_key, bobpubliccommunity_public_member_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      federation.federation_type = "member";
      federation.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_member_key, bobpubliccommunity_public_member_key, aliceopencommunity_public_member_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_member_key ) );
      federation.share_accounts = true;
      federation.accepted = true;
      federation.validate();

      tx.operations.push_back( federation );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      federation_itr = federation_idx.find( boost::make_tuple( federation.federated_community, federation.community ) );
      BOOST_REQUIRE( federation_itr != federation_idx.end() );

      federation.signatory = "candice";
      federation.account = "candice";
      federation.federation_id = "c02fece2-a58b-4393-bcc6-bf2d88b060a7";
      federation.community = "candiceprivatecommunity";
      federation.federated_community = "aliceopencommunity";
      federation.message = get_encrypted_message( candiceprivatecommunity_private_member_key, candiceprivatecommunity_public_member_key, aliceopencommunity_public_member_key, string( "#Hello" ) );
      federation.json = get_encrypted_message( candiceprivatecommunity_private_member_key, candiceprivatecommunity_public_member_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      federation.federation_type = "member";
      federation.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_member_key, candiceprivatecommunity_public_member_key, aliceopencommunity_public_member_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_member_key ) );
      federation.share_accounts = true;
      federation.accepted = true;
      federation.validate();

      tx.operations.push_back( federation );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      federation_itr = federation_idx.find( boost::make_tuple( federation.federated_community, federation.community ) );
      BOOST_REQUIRE( federation_itr != federation_idx.end() );

      federation.signatory = "dan";
      federation.account = "dan";
      federation.federation_id = "713890b9-ecac-42e4-bca9-0b679cdc21b3";
      federation.community = "danexclusivecommunity";
      federation.federated_community = "aliceopencommunity";
      federation.message = get_encrypted_message( danexclusivecommunity_private_member_key, danexclusivecommunity_public_member_key, aliceopencommunity_public_member_key, string( "#Hello" ) );
      federation.json = get_encrypted_message( danexclusivecommunity_private_member_key, danexclusivecommunity_public_member_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      federation.federation_type = "member";
      federation.encrypted_community_key = get_encrypted_message( danexclusivecommunity_private_member_key, danexclusivecommunity_public_member_key, aliceopencommunity_public_member_key, string( "#" ) + graphene::utilities::key_to_wif( danexclusivecommunity_private_member_key ) );
      federation.share_accounts = true;
      federation.accepted = true;
      federation.validate();

      tx.operations.push_back( federation );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      federation_itr = federation_idx.find( boost::make_tuple( federation.federated_community, federation.community ) );
      BOOST_REQUIRE( federation_itr != federation_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Community Federation accept success" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Comment creation handling for members, non-members, and federated members" );

      generate_blocks( now() + fc::minutes(11), true );

      comment_operation comment;

      comment.signatory = "george";
      comment.author = "george";
      comment.permlink = "lorem";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = "aliceopencommunity";
      comment.public_key = string();
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
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

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );       // Non-members can create posts in open community
      db.push_transaction( tx, 0 );

      generate_blocks( now() + fc::minutes(11) );

      const comment_object& george_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( george_comment.author == comment.author );
      BOOST_REQUIRE( george_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "haz";
      comment.author = "haz";
      comment.community = "bobpubliccommunity";
      comment.public_key = string();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      generate_blocks( now() + fc::minutes(11) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "isabelle";
      comment.author = "isabelle";
      comment.permlink = "lorem";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = get_encrypted_message( isabelle_private_secure_key, isabelle_public_secure_key, candiceprivatecommunity_public_member_key, string( "#Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." ) );
      comment.url = "https://www.url.com";
      comment.url_private = get_encrypted_message( isabelle_private_secure_key, isabelle_public_secure_key, candiceprivatecommunity_public_member_key, string( "#https://www.url.com" ) );
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = get_encrypted_message( isabelle_private_secure_key, isabelle_public_secure_key, candiceprivatecommunity_public_member_key, string( "#QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" ) );
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = get_encrypted_message( isabelle_private_secure_key, isabelle_public_secure_key, candiceprivatecommunity_public_member_key, string( "#magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" ) );
      comment.json = "{ \"valid\": true }";
      comment.json_private = get_encrypted_message( isabelle_private_secure_key, isabelle_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      comment.community = "candiceprivatecommunity";
      comment.public_key = string( candiceprivatecommunity_public_member_key );

      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      generate_blocks( now() + fc::minutes(11) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "jayme";
      comment.author = "jayme";
      comment.permlink = "lorem";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = get_encrypted_message( jayme_private_secure_key, jayme_public_secure_key, danexclusivecommunity_public_member_key, string( "#Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." ) );
      comment.url = "https://www.url.com";
      comment.url_private = get_encrypted_message( jayme_private_secure_key, jayme_public_secure_key, danexclusivecommunity_public_member_key, string( "#https://www.url.com" ) );
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = get_encrypted_message( jayme_private_secure_key, jayme_public_secure_key, danexclusivecommunity_public_member_key, string( "#QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" ) );
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = get_encrypted_message( jayme_private_secure_key, jayme_public_secure_key, danexclusivecommunity_public_member_key, string( "#magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" ) );
      comment.json = "{ \"valid\": true }";
      comment.json_private = get_encrypted_message( jayme_private_secure_key, jayme_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      comment.community = "danexclusivecommunity";
      comment.public_key = string( danexclusivecommunity_public_member_key );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      generate_blocks( now() + fc::minutes(11) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = "";
      comment.url = "https://www.url.com";
      comment.url_private = "";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = "";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = "";
      comment.json = "{ \"valid\": true }";
      comment.json_private = "";
      comment.community = "aliceopencommunity";
      comment.public_key = string();
      options.reach = "tag";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( alice_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "bob";
      comment.author = "bob";
      comment.permlink = "lorem";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = "";
      comment.url = "https://www.url.com";
      comment.url_private = "";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = "";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = "";
      comment.json = "{ \"valid\": true }";
      comment.json_private = "";
      comment.community = "bobpubliccommunity";
      comment.public_key = string();
      options.reach = "tag";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( bob_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "candice";
      comment.author = "candice";
      comment.permlink = "lorem";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." ) );
      comment.url = "https://www.url.com";
      comment.url_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#https://www.url.com" ) );
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" ) );
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" ) );
      comment.json = "{ \"valid\": true }";
      comment.json_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      comment.community = "candiceprivatecommunity";
      comment.public_key = string( candiceprivatecommunity_public_member_key );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( candice_comment.community == comment.community );
      BOOST_REQUIRE( candice_comment.is_encrypted() );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "dan";
      comment.author = "dan";
      comment.permlink = "lorem";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." ) );
      comment.url = "https://www.url.com";
      comment.url_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#https://www.url.com" ) );
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" ) );
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" ) );
      comment.json = "{ \"valid\": true }";
      comment.json_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      comment.community = "danexclusivecommunity";
      comment.public_key = string( danexclusivecommunity_public_member_key );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::minutes(11) );

      const comment_object& dan_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( dan_comment.community == comment.community );
      BOOST_REQUIRE( dan_comment.is_encrypted() );

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "federated-post";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = "";
      comment.url = "https://www.url.com";
      comment.url_private = "";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = "";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = "";
      comment.json = "{ \"valid\": true }";
      comment.json_private = "";
      comment.community = "bobpubliccommunity";
      comment.public_key = string();
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.latitude = 37.8136;
      comment.longitude = 144.9631;
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      options.post_type = "article";
      options.reach = "tag";
      options.rating = 1;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );       // Member of federated community can create posts in public community
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::minutes(11) );

      const comment_object& alice_bob_federated_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( alice_bob_federated_comment.author == comment.author );
      BOOST_REQUIRE( alice_bob_federated_comment.community == comment.community );

      comment.permlink = "federated-post2";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." ) );
      comment.url = "https://www.url.com";
      comment.url_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#https://www.url.com" ) );
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" ) );
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" ) );
      comment.json = "{ \"valid\": true }";
      comment.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      comment.community = "candiceprivatecommunity";
      comment.public_key = string( candiceprivatecommunity_public_member_key );

      options.post_type = "article";
      options.reach = "community";
      options.rating = 1;
      comment.options = options;
      comment.validate();
      
      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );       // Member of federated community can create posts in private community
      db.push_transaction( tx, 0 );

      const comment_object& alice_candice_federated_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( alice_candice_federated_comment.author == comment.author );
      BOOST_REQUIRE( alice_candice_federated_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::minutes(11) );

      comment.permlink = "federated-post3";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.body_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, danexclusivecommunity_public_member_key, string( "#Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." ) );
      comment.url = "https://www.url.com";
      comment.url_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, danexclusivecommunity_public_member_key, string( "#https://www.url.com" ) );
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.ipfs_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, danexclusivecommunity_public_member_key, string( "#QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" ) );
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.magnet_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, danexclusivecommunity_public_member_key, string( "#magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" ) );
      comment.json = "{ \"valid\": true }";
      comment.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      comment.community = "danexclusivecommunity";
      comment.public_key = string( danexclusivecommunity_public_member_key );

      options.post_type = "article";
      options.reach = "community";
      options.rating = 1;
      comment.options = options;
      comment.validate();
      
      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );       // Member of federated community can create posts in exclusive community
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const comment_object& alice_dan_federated_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( alice_dan_federated_comment.author == comment.author );
      BOOST_REQUIRE( alice_dan_federated_comment.community == comment.community );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Comment creation handling for members, non-members, and federated members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Message handling for members and non-members" );

      message_operation message;

      message.signatory = "george";
      message.sender = "george";
      message.community = "aliceopencommunity";
      message.public_key = string( aliceopencommunity_public_member_key );
      message.message = get_encrypted_message( george_private_secure_key, george_public_secure_key, aliceopencommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "f55004be-bf4b-4bc7-b6c6-4a2d16a2ef9a";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create messages

      tx.operations.clear();
      tx.signatures.clear();

      const auto& message_idx = db.get_index< message_index >().indices().get< by_sender_uuid >();
      auto message_itr = message_idx.find( boost::make_tuple( account_name_type( "george" ), string( "f55004be-bf4b-4bc7-b6c6-4a2d16a2ef9a" ) ) );

      BOOST_REQUIRE( message_itr == message_idx.end() );
      
      message.signatory = "haz";
      message.sender = "haz";
      message.community = "bobpubliccommunity";
      message.public_key = string( bobpubliccommunity_public_member_key );
      message.message = get_encrypted_message( haz_private_secure_key, haz_public_secure_key, bobpubliccommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "cea25601-c5b2-4ced-b785-cf27d8124876";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create messages

      tx.operations.clear();
      tx.signatures.clear();

      message_itr = message_idx.find( boost::make_tuple( account_name_type( "haz" ), string( "cea25601-c5b2-4ced-b785-cf27d8124876" ) ) );

      BOOST_REQUIRE( message_itr == message_idx.end() );

      message.signatory = "isabelle";
      message.sender = "isabelle";
      message.community = "candiceprivatecommunity";
      message.public_key = string( candiceprivatecommunity_public_member_key );
      message.message = get_encrypted_message( isabelle_private_secure_key, isabelle_public_secure_key, candiceprivatecommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "2279c867-bae8-486c-9135-639821d7a651";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create messages

      tx.operations.clear();
      tx.signatures.clear();

      message_itr = message_idx.find( boost::make_tuple( account_name_type( "isabelle" ), string( "2279c867-bae8-486c-9135-639821d7a651" ) ) );

      BOOST_REQUIRE( message_itr == message_idx.end() );

      message.signatory = "jayme";
      message.sender = "jayme";
      message.community = "danexclusivecommunity";
      message.public_key = string( danexclusivecommunity_public_member_key );
      message.message = get_encrypted_message( jayme_private_secure_key, jayme_public_secure_key, danexclusivecommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "2e89bafc-1b29-4fc1-9e8c-c26055dc418d";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create messages

      tx.operations.clear();
      tx.signatures.clear();

      message_itr = message_idx.find( boost::make_tuple( account_name_type( "jayme" ), string( "2e89bafc-1b29-4fc1-9e8c-c26055dc418d" ) ) );

      BOOST_REQUIRE( message_itr == message_idx.end() );

      message.signatory = "alice";
      message.sender = "alice";
      message.community = "aliceopencommunity";
      message.public_key = string( aliceopencommunity_public_member_key );
      message.message = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, aliceopencommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "1a068a51-b724-4a64-b53d-acfd1e77d17d";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      message_itr = message_idx.find( boost::make_tuple( account_name_type( "alice" ), string( "1a068a51-b724-4a64-b53d-acfd1e77d17d" ) ) );

      BOOST_REQUIRE( message_itr != message_idx.end() );
      BOOST_REQUIRE( to_string( message_itr->message ) == message.message );

      message.signatory = "bob";
      message.sender = "bob";
      message.community = "bobpubliccommunity";
      message.public_key = string( bobpubliccommunity_public_member_key );
      message.message = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, bobpubliccommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "1e3baa5c-4ba4-47cb-b4e8-8a8b93e442de";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      message_itr = message_idx.find( boost::make_tuple( account_name_type( "bob" ), string( "1e3baa5c-4ba4-47cb-b4e8-8a8b93e442de" ) ) );

      BOOST_REQUIRE( message_itr != message_idx.end() );
      BOOST_REQUIRE( to_string( message_itr->message ) == message.message );

      message.signatory = "candice";
      message.sender = "candice";
      message.community = "candiceprivatecommunity";
      message.public_key = string( candiceprivatecommunity_public_member_key );
      message.message = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, candiceprivatecommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "e6596869-c7a4-4da6-bd3d-bf793b0aff63";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      message_itr = message_idx.find( boost::make_tuple( account_name_type( "candice" ), string( "e6596869-c7a4-4da6-bd3d-bf793b0aff63" ) ) );

      BOOST_REQUIRE( message_itr != message_idx.end() );
      BOOST_REQUIRE( to_string( message_itr->message ) == message.message );

      message.signatory = "dan";
      message.sender = "dan";
      message.community = "danexclusivecommunity";
      message.public_key = string( danexclusivecommunity_public_member_key );
      message.message = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, danexclusivecommunity_public_member_key, string( "#Hello" ) );
      message.uuid = "5f7ed3f7-d24e-40b1-a3e6-ecf13a40223e";
      message.interface = INIT_ACCOUNT;
      message.expiration = now() + fc::days(365);
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      message_itr = message_idx.find( boost::make_tuple( account_name_type( "dan" ), string( "5f7ed3f7-d24e-40b1-a3e6-ecf13a40223e" ) ) );

      BOOST_REQUIRE( message_itr != message_idx.end() );
      BOOST_REQUIRE( to_string( message_itr->message ) == message.message );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment creation handling for members and non-members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment vote handling before becoming members" );

      comment_vote_operation vote;

      vote.signatory = "george";
      vote.voter = "george";
      vote.author = "alice";
      vote.permlink = "lorem";
      vote.weight = PERCENT_100;
      vote.interface = INIT_ACCOUNT;
      vote.validate();
      
      tx.operations.push_back( vote );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_alice_vote = db.get_comment_vote( vote.voter, db.get_comment( vote.author, vote.permlink ).id );

      BOOST_REQUIRE( george_alice_vote.voter == vote.voter );
      BOOST_REQUIRE( george_alice_vote.comment == db.get_comment( vote.author, vote.permlink ).id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "haz";
      vote.voter = "haz";
      vote.author = "bob";

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_bob_vote = db.get_comment_vote( vote.voter, db.get_comment( vote.author, vote.permlink ).id );

      BOOST_REQUIRE( haz_bob_vote.voter == vote.voter );
      BOOST_REQUIRE( haz_bob_vote.comment == db.get_comment( vote.author, vote.permlink ).id );

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

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment vote handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment view handling before becoming members" );

      comment_view_operation view;

      view.signatory = "george";
      view.viewer = "george";
      view.author = "alice";
      view.permlink = "lorem";
      view.interface = INIT_ACCOUNT;
      view.supernode = INIT_ACCOUNT;
      view.viewed = true;
      view.validate();
      
      tx.operations.push_back( view );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_view = db.get_comment_view( view.viewer, db.get_comment( view.author, view.permlink ).id );

      BOOST_REQUIRE( george_view.viewer == view.viewer );
      BOOST_REQUIRE( george_view.comment == db.get_comment( view.author, view.permlink ).id );
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

      const comment_view_object& haz_view = db.get_comment_view( view.viewer, db.get_comment( view.author, view.permlink ).id );

      BOOST_REQUIRE( haz_view.viewer == view.viewer );
      BOOST_REQUIRE( haz_view.comment == db.get_comment( view.author, view.permlink ).id );
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

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment view handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment share handling before becoming members" );

      comment_share_operation share;

      share.signatory = "george";
      share.sharer = "george";
      share.author = "alice";
      share.permlink = "lorem";
      share.reach = "follow";
      share.interface = INIT_ACCOUNT;
      share.shared = true;
      share.validate();
      
      tx.operations.push_back( share );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_share = db.get_comment_share( share.sharer, db.get_comment( share.author, share.permlink ).id );

      BOOST_REQUIRE( george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_share.comment == db.get_comment( share.author, share.permlink ).id );
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

      const comment_share_object& haz_bob_share = db.get_comment_share( share.sharer, db.get_comment( share.author, share.permlink ).id );

      BOOST_REQUIRE( haz_bob_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_bob_share.comment == db.get_comment( share.author, share.permlink ).id );
      BOOST_REQUIRE( haz_bob_share.interface == share.interface );
      BOOST_REQUIRE( haz_bob_share.created == now() );

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

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment share handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment creation handling by members" );

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.community = "aliceopencommunity";
      community_member.member_type = "member";
      community_member.member = "george";
      community_member.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_member_key, aliceopencommunity_public_member_key, george_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_member_key ) );
      community_member.accepted = true;
      community_member.expiration = now() + fc::days(31);
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";
      community_member.member = "haz";
      community_member.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_member_key, bobpubliccommunity_public_member_key, haz_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_member_key ) );

      tx.operations.push_back( community_member );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";
      community_member.member = "isabelle";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_member_key, candiceprivatecommunity_public_member_key, isabelle_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_member_key ) );

      tx.operations.push_back( community_member );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";
      community_member.member = "jayme";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_member_key, candiceprivatecommunity_public_member_key, isabelle_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_member_key ) );

      tx.operations.push_back( community_member );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + fc::minutes(2), true );

      comment.signatory = "george";
      comment.author = "george";
      comment.community = "aliceopencommunity";
      comment.permlink = "ipsum";
      comment.public_key = string();
      options.reach = "tag";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& george_comment2 = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( george_comment2.author == comment.author );
      BOOST_REQUIRE( george_comment2.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "haz";
      comment.author = "haz";
      comment.community = "bobpubliccommunity";
      comment.public_key = string();
      options.reach = "tag";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& haz_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( haz_comment.author == comment.author );
      BOOST_REQUIRE( haz_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "isabelle";
      comment.author = "isabelle";
      comment.community = "candiceprivatecommunity";
      comment.public_key = string( candiceprivatecommunity_public_member_key );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& isabelle_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( isabelle_comment.author == comment.author );
      BOOST_REQUIRE( isabelle_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "jayme";
      comment.author = "jayme";
      comment.community = "danexclusivecommunity";
      comment.public_key = string( danexclusivecommunity_public_member_key );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& jayme_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( jayme_comment.author == comment.author );
      BOOST_REQUIRE( jayme_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment creation handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment vote handling by members" );

      vote.signatory = "george";
      vote.voter = "george";
      vote.author = "george";
      vote.permlink = "ipsum";
      
      tx.operations.push_back( vote );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_vote = db.get_comment_vote( vote.voter, db.get_comment( vote.voter, vote.permlink ).id );

      BOOST_REQUIRE( george_vote.voter == vote.voter );
      BOOST_REQUIRE( george_vote.comment == db.get_comment( vote.voter, vote.permlink ).id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "haz";
      vote.voter = "haz";
      vote.author = "haz";

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_vote = db.get_comment_vote( vote.voter, db.get_comment( vote.voter, vote.permlink ).id );

      BOOST_REQUIRE( haz_vote.voter == vote.voter );
      BOOST_REQUIRE( haz_vote.comment == db.get_comment( vote.voter, vote.permlink ).id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "isabelle";
      vote.voter = "isabelle";
      vote.author = "isabelle";

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& isabelle_vote = db.get_comment_vote( vote.voter, db.get_comment( vote.voter, vote.permlink ).id );

      BOOST_REQUIRE( isabelle_vote.voter == vote.voter );
      BOOST_REQUIRE( isabelle_vote.comment == db.get_comment( vote.voter, vote.permlink ).id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "jayme";
      vote.voter = "jayme";
      vote.author = "jayme";

      tx.operations.push_back( vote );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& jayme_vote = db.get_comment_vote( vote.voter, db.get_comment( vote.voter, vote.permlink ).id );

      BOOST_REQUIRE( jayme_vote.voter == vote.voter );
      BOOST_REQUIRE( jayme_vote.comment == db.get_comment( vote.voter, vote.permlink ).id );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment vote handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment view handling by members" );

      view.signatory = "george";
      view.viewer = "george";
      view.author = "george";
      view.permlink = "ipsum";

      tx.operations.push_back( view );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_george_view = db.get_comment_view( view.viewer, db.get_comment( view.viewer, view.permlink ).id );

      BOOST_REQUIRE( george_george_view.viewer == view.viewer );
      BOOST_REQUIRE( george_george_view.comment == db.get_comment( view.viewer, view.permlink ).id );
      BOOST_REQUIRE( george_george_view.supernode == view.supernode );
      BOOST_REQUIRE( george_george_view.interface == view.interface );
      BOOST_REQUIRE( george_george_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "haz";
      view.viewer = "haz";
      view.author = "haz";

      tx.operations.push_back( view );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& haz_haz_view = db.get_comment_view( view.viewer, db.get_comment( view.viewer, view.permlink ).id );

      BOOST_REQUIRE( haz_haz_view.viewer == view.viewer );
      BOOST_REQUIRE( haz_haz_view.comment == db.get_comment( view.viewer, view.permlink ).id );
      BOOST_REQUIRE( haz_haz_view.supernode == view.supernode );
      BOOST_REQUIRE( haz_haz_view.interface == view.interface );
      BOOST_REQUIRE( haz_haz_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "isabelle";
      view.viewer = "isabelle";
      view.author = "isabelle";

      tx.operations.push_back( view );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& isabelle_isabelle_view = db.get_comment_view( view.viewer, db.get_comment( view.viewer, view.permlink ).id );

      BOOST_REQUIRE( isabelle_isabelle_view.viewer == view.viewer );
      BOOST_REQUIRE( isabelle_isabelle_view.comment == db.get_comment( view.viewer, view.permlink ).id );
      BOOST_REQUIRE( isabelle_isabelle_view.supernode == view.supernode );
      BOOST_REQUIRE( isabelle_isabelle_view.interface == view.interface );
      BOOST_REQUIRE( isabelle_isabelle_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "jayme";
      view.viewer = "jayme";
      view.author = "jayme";

      tx.operations.push_back( view );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& jayme_jayme_view = db.get_comment_view( view.viewer, db.get_comment( view.viewer, view.permlink ).id );

      BOOST_REQUIRE( jayme_jayme_view.viewer == view.viewer );
      BOOST_REQUIRE( jayme_jayme_view.comment == db.get_comment( view.viewer, view.permlink ).id );
      BOOST_REQUIRE( jayme_jayme_view.supernode == view.supernode );
      BOOST_REQUIRE( jayme_jayme_view.interface == view.interface );
      BOOST_REQUIRE( jayme_jayme_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment view handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment share handling by members" );

      share.signatory = "george";
      share.sharer = "george";
      share.author = "george";
      share.permlink = "ipsum";
      
      tx.operations.push_back( share );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_george_share = db.get_comment_share( share.sharer, db.get_comment( share.sharer, share.permlink ).id );

      BOOST_REQUIRE( george_george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_george_share.comment == db.get_comment( share.sharer, share.permlink ).id );
      BOOST_REQUIRE( george_george_share.interface == share.interface );
      BOOST_REQUIRE( george_george_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "haz";
      share.sharer = "haz";
      share.author = "haz";

      tx.operations.push_back( share );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& haz_haz_share = db.get_comment_share( share.sharer, db.get_comment( share.sharer, share.permlink ).id );

      BOOST_REQUIRE( haz_haz_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_haz_share.comment == db.get_comment( share.sharer, share.permlink ).id );
      BOOST_REQUIRE( haz_haz_share.interface == share.interface );
      BOOST_REQUIRE( haz_haz_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "isabelle";
      share.sharer = "isabelle";
      share.author = "isabelle";

      tx.operations.push_back( share );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& isabelle_isabelle_share = db.get_comment_share( share.sharer, db.get_comment( share.sharer, share.permlink ).id );

      BOOST_REQUIRE( isabelle_isabelle_share.sharer == share.sharer );
      BOOST_REQUIRE( isabelle_isabelle_share.comment == db.get_comment( share.sharer, share.permlink ).id );
      BOOST_REQUIRE( isabelle_isabelle_share.interface == share.interface );
      BOOST_REQUIRE( isabelle_isabelle_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = "jayme";
      share.sharer = "jayme";
      share.author = "jayme";

      tx.operations.push_back( share );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& jayme_jayme_share = db.get_comment_share( share.sharer, db.get_comment( share.sharer, share.permlink ).id );

      BOOST_REQUIRE( jayme_jayme_share.sharer == share.sharer );
      BOOST_REQUIRE( jayme_jayme_share.comment == db.get_comment( share.sharer, share.permlink ).id );
      BOOST_REQUIRE( jayme_jayme_share.interface == share.interface );
      BOOST_REQUIRE( jayme_jayme_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment share handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: moderator tag by community moderator" );

      comment_moderation_operation tag;

      tag.signatory = "elon";
      tag.moderator = "elon";
      tag.author = "george";
      tag.permlink = "ipsum";
      tag.tags.push_back( tag_name_type( "nsfw" ) );
      tag.rating = 9;
      tag.details = "Post is NSFW";
      tag.interface = INIT_ACCOUNT;
      tag.filter = false;
      tag.applied = true;
      tag.validate();

      tx.operations.push_back( tag );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& tag_idx = db.get_index< comment_moderation_index >().indices().get< by_comment_moderator >();
      auto tag_itr = tag_idx.find( boost::make_tuple( db.get_comment( tag.author, tag.permlink ).id, tag.moderator ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = "haz";

      tx.operations.push_back( tag );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tag_itr = tag_idx.find( boost::make_tuple( db.get_comment( tag.author, tag.permlink ).id, tag.moderator ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = "isabelle";

      tx.operations.push_back( tag );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tag_itr = tag_idx.find( boost::make_tuple( db.get_comment( tag.author, tag.permlink ).id, tag.moderator ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = "jayme";

      tx.operations.push_back( tag );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tag_itr = tag_idx.find( boost::make_tuple( db.get_comment( tag.author, tag.permlink ).id, tag.moderator ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: moderator tag by community moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator add moderator" );

      community_member.signatory = "elon";
      community_member.account = "elon";
      community_member.community = "aliceopencommunity";
      community_member.member = "fred";
      community_member.member_type = "moderator";
      community_member.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_moderator_key, aliceopencommunity_public_moderator_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_moderator_key ) );
      community_member.accepted = true;
      community_member.expiration = now() + fc::days(31);
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      community_member.community = "bobpubliccommunity";
      community_member.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_moderator_key, bobpubliccommunity_public_moderator_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_moderator_key ) );

      tx.operations.push_back( community_member );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      community_member.community = "candiceprivatecommunity";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_moderator_key, candiceprivatecommunity_public_moderator_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_moderator_key ) );

      tx.operations.push_back( community_member );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      community_member.community = "danexclusivecommunity";
      community_member.encrypted_community_key = get_encrypted_message( danexclusivecommunity_private_moderator_key, danexclusivecommunity_public_moderator_key, fred_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( danexclusivecommunity_private_moderator_key ) );

      tx.operations.push_back( community_member );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator update community" );

      community_update_operation update;

      update.signatory = "elon";
      update.account = "elon";
      update.community = "aliceopencommunity";
      update.display_name = "Alice's Updated Open Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( aliceopencommunity_private_secure_key, aliceopencommunity_public_secure_key, aliceopencommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "alice";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = false;
      update.author_permission = "all";
      update.reply_permission = "all";
      update.vote_permission = "all";
      update.view_permission = "all";
      update.share_permission = "all";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "member";
      update.request_permission = "all";
      update.remove_permission = "administrator";
      update.community_member_key = string( aliceopencommunity_public_member_key );
      update.community_moderator_key = string( aliceopencommunity_public_moderator_key );
      update.community_admin_key = string( aliceopencommunity_public_admin_key );
      update.community_secure_key = string( aliceopencommunity_public_secure_key );
      update.community_standard_premium_key = string( aliceopencommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( aliceopencommunity_public_mid_premium_key );
      update.community_top_premium_key = string( aliceopencommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community( update.community ).name == update.community );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).details ) == update.details );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).url ) == update.url );
      BOOST_REQUIRE( db.get_community( update.community ).pinned_author == update.pinned_author );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).pinned_permlink ) == update.pinned_permlink );
      BOOST_REQUIRE( db.get_community( update.community ).last_updated == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "elon";
      update.account = "elon";
      update.community = "bobpubliccommunity";
      update.display_name = "Bob's Updated Public Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( bobpubliccommunity_private_secure_key, bobpubliccommunity_public_secure_key, bobpubliccommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "bob";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = false;
      update.author_permission = "member";
      update.reply_permission = "member";
      update.vote_permission = "member";
      update.view_permission = "member";
      update.share_permission = "member";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "member";
      update.request_permission = "all";
      update.remove_permission = "administrator";
      update.community_member_key = string( bobpubliccommunity_public_member_key );
      update.community_moderator_key = string( bobpubliccommunity_public_moderator_key );
      update.community_admin_key = string( bobpubliccommunity_public_admin_key );
      update.community_secure_key = string( bobpubliccommunity_public_secure_key );
      update.community_standard_premium_key = string( bobpubliccommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( bobpubliccommunity_public_mid_premium_key );
      update.community_top_premium_key = string( bobpubliccommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community( update.community ).name == update.community );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).details ) == update.details );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).url ) == update.url );
      BOOST_REQUIRE( db.get_community( update.community ).pinned_author == update.pinned_author );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).pinned_permlink ) == update.pinned_permlink );
      BOOST_REQUIRE( db.get_community( update.community ).last_updated == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "elon";
      update.account = "elon";
      update.community = "candiceprivatecommunity";
      update.display_name = "Candice's Updated Private Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( candiceprivatecommunity_private_secure_key, candiceprivatecommunity_public_secure_key, candiceprivatecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "candice";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = true;
      update.author_permission = "member";
      update.reply_permission = "member";
      update.vote_permission = "member";
      update.view_permission = "member";
      update.share_permission = "member";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "member";
      update.request_permission = "all";
      update.remove_permission = "administrator";
      update.community_member_key = string( candiceprivatecommunity_public_member_key );
      update.community_moderator_key = string( candiceprivatecommunity_public_moderator_key );
      update.community_admin_key = string( candiceprivatecommunity_public_admin_key );
      update.community_secure_key = string( candiceprivatecommunity_public_secure_key );
      update.community_standard_premium_key = string( candiceprivatecommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( candiceprivatecommunity_public_mid_premium_key );
      update.community_top_premium_key = string( candiceprivatecommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community( update.community ).name == update.community );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).details ) == update.details );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).url ) == update.url );
      BOOST_REQUIRE( db.get_community( update.community ).pinned_author == update.pinned_author );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).pinned_permlink ) == update.pinned_permlink );
      BOOST_REQUIRE( db.get_community( update.community ).last_updated == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "elon";
      update.account = "elon";
      update.community = "danexclusivecommunity";
      update.display_name = "Dan's Updated Exclusive Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( danexclusivecommunity_private_secure_key, danexclusivecommunity_public_secure_key, danexclusivecommunity_public_member_key, string( "#{ \"valid\": true }" ) );
      update.pinned_author = "dan";
      update.pinned_permlink = "lorem";
      update.tags.insert( "test" );
      update.private_community = true;
      update.author_permission = "member";
      update.reply_permission = "member";
      update.vote_permission = "member";
      update.view_permission = "member";
      update.share_permission = "member";
      update.message_permission = "member";
      update.poll_permission = "administrator";
      update.event_permission = "administrator";
      update.directive_permission = "member";
      update.add_permission = "administrator";
      update.request_permission = "member";
      update.remove_permission = "administrator";
      update.community_member_key = string( danexclusivecommunity_public_member_key );
      update.community_moderator_key = string( danexclusivecommunity_public_moderator_key );
      update.community_admin_key = string( danexclusivecommunity_public_admin_key );
      update.community_secure_key = string( danexclusivecommunity_public_secure_key );
      update.community_standard_premium_key = string( danexclusivecommunity_public_standard_premium_key );
      update.community_mid_premium_key = string( danexclusivecommunity_public_mid_premium_key );
      update.community_top_premium_key = string( danexclusivecommunity_public_top_premium_key );
      update.standard_membership_price = MEMBERSHIP_FEE_BASE;
      update.mid_membership_price = MEMBERSHIP_FEE_MID;
      update.top_membership_price = MEMBERSHIP_FEE_TOP;
      update.verifiers.insert( INIT_ACCOUNT );
      update.min_verification_count = 0;
      update.max_verification_distance = 0;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.active = true;
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community( update.community ).name == update.community );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).details ) == update.details );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).url ) == update.url );
      BOOST_REQUIRE( db.get_community( update.community ).pinned_author == update.pinned_author );
      BOOST_REQUIRE( to_string( db.get_community( update.community ).pinned_permlink ) == update.pinned_permlink );
      BOOST_REQUIRE( db.get_community( update.community ).last_updated == now() );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator update community" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove moderator" );

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.community = "aliceopencommunity";
      community_member.member = "fred";
      community_member.member_type = "moderator";
      community_member.accepted = false;
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";

      tx.operations.push_back( community_member );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";

      tx.operations.push_back( community_member );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";

      tx.operations.push_back( community_member );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).is_member( community_member.member ) );
      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_moderator( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove member" );

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.community = "aliceopencommunity";
      community_member.member = "fred";
      community_member.member_type = "member";
      community_member.accepted = false;
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";

      tx.operations.push_back( community_member );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";

      tx.operations.push_back( community_member );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";

      tx.operations.push_back( community_member );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !db.get_community_permission( community_member.community ).is_member( community_member.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community subscription" );

      community_subscribe_operation subscribe;

      subscribe.signatory = "elon";
      subscribe.account = "elon";
      subscribe.community = "aliceopencommunity";
      subscribe.interface = INIT_ACCOUNT;
      subscribe.added = true;
      subscribe.subscribed = true;
      subscribe.validate();

      tx.operations.push_back( subscribe );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "bobpubliccommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "candiceprivatecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "danexclusivecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: community subscription" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder blacklist account" );

      community_blacklist_operation blacklist;

      blacklist.signatory = "alice";
      blacklist.account = "alice";
      blacklist.member = "fred";
      blacklist.community = "aliceopencommunity";
      blacklist.validate();

      tx.operations.push_back( blacklist );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
   
      BOOST_REQUIRE( db.get_community_permission( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "bob";
      blacklist.account = "bob";
      blacklist.community = "bobpubliccommunity";

      tx.operations.push_back( blacklist );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "candice";
      blacklist.account = "candice";
      blacklist.community = "candiceprivatecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "dan";
      blacklist.account = "dan";
      blacklist.community = "danexclusivecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder blacklist account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: transfer community ownership" );

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      community_member.signatory = "alice";
      community_member.account = "alice";
      community_member.community = "aliceopencommunity";
      community_member.member_type = "founder";
      community_member.member = "elon";
      community_member.encrypted_community_key = get_encrypted_message( aliceopencommunity_private_secure_key, aliceopencommunity_public_secure_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( aliceopencommunity_private_secure_key ) );
      community_member.accepted = true;
      community_member.expiration = now() + fc::days(31);
      community_member.validate();

      tx.operations.push_back( community_member );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).founder == community_member.member );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "bob";
      community_member.account = "bob";
      community_member.community = "bobpubliccommunity";
      community_member.encrypted_community_key = get_encrypted_message( bobpubliccommunity_private_secure_key, bobpubliccommunity_public_secure_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( bobpubliccommunity_private_secure_key ) );

      tx.operations.push_back( community_member );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).founder == community_member.member );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "candice";
      community_member.account = "candice";
      community_member.community = "candiceprivatecommunity";
      community_member.encrypted_community_key = get_encrypted_message( candiceprivatecommunity_private_secure_key, candiceprivatecommunity_public_secure_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( candiceprivatecommunity_private_secure_key ) );

      tx.operations.push_back( community_member );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).founder == community_member.member );

      tx.operations.clear();
      tx.signatures.clear();

      community_member.signatory = "dan";
      community_member.account = "dan";
      community_member.community = "danexclusivecommunity";
      community_member.encrypted_community_key = get_encrypted_message( danexclusivecommunity_private_secure_key, danexclusivecommunity_public_secure_key, elon_public_secure_key, string( "#" ) + graphene::utilities::key_to_wif( danexclusivecommunity_private_secure_key ) );

      tx.operations.push_back( community_member );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_permission( community_member.community ).founder == community_member.member );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      BOOST_TEST_MESSAGE( "│   ├── Passed: transfer community ownership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create community event" );

      community_event_operation event;

      event.signatory = "alice";
      event.account = "alice";
      event.community = "aliceopencommunity";
      event.event_id = "a14e898a-9d92-4461-8011-64a43521c051";
      event.event_name = "Alice's Party";
      event.location = "Alice's House";
      event.latitude = 37.8136;
      event.longitude = 144.9631;
      event.details = "Open house - BYO drinks";
      event.url = "https://www.staggeringbeauty.com";
      event.json = "{ \"valid\": true }";
      event.interface = INIT_ACCOUNT;
      event.event_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      event.event_start_time = now() + fc::days(1);
      event.event_end_time = now() + fc::days(1) + fc::hours(8);
      event.active = true;
      event.validate();

      tx.operations.push_back( event );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_event_object& alice_event = db.get_community_event( event.community, event.event_id );

      BOOST_REQUIRE( event.community == alice_event.community );
      BOOST_REQUIRE( event.event_id == to_string( alice_event.event_id ) );
      BOOST_REQUIRE( event.event_name == to_string( alice_event.event_name ) );
      BOOST_REQUIRE( event.location == to_string( alice_event.location ) );
      BOOST_REQUIRE( event.latitude == alice_event.latitude );
      BOOST_REQUIRE( event.longitude == alice_event.longitude );
      BOOST_REQUIRE( event.details == to_string( alice_event.details ) );
      BOOST_REQUIRE( event.url == to_string( alice_event.url ) );
      BOOST_REQUIRE( event.json == to_string( alice_event.json ) );
      BOOST_REQUIRE( event.interface == alice_event.interface );
      BOOST_REQUIRE( event.event_price == alice_event.event_price );
      BOOST_REQUIRE( event.event_start_time == alice_event.event_start_time );
      BOOST_REQUIRE( event.event_end_time == alice_event.event_end_time );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create community event" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Attend community event" );

      community_event_attend_operation attend;

      attend.signatory = "elon";
      attend.attendee = "elon";
      attend.community = "aliceopencommunity";
      attend.event_id = "a14e898a-9d92-4461-8011-64a43521c051";
      attend.json = "{ \"valid\": true }";
      attend.interface = INIT_ACCOUNT;
      attend.interested = true;
      attend.attending = true;
      attend.active = true;
      attend.validate();

      tx.operations.push_back( attend );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& event_attend_idx = db.get_index< community_event_attend_index >().indices().get< by_event_id >();
      auto event_attend_itr = event_attend_idx.find( boost::make_tuple( attend.community, attend.event_id, attend.attendee ) );

      BOOST_REQUIRE( event_attend_itr != event_attend_idx.end() );

      const community_event_attend_object& elon_attend = *event_attend_itr;

      BOOST_REQUIRE( attend.attendee == elon_attend.attendee );
      BOOST_REQUIRE( attend.community == elon_attend.community );
      BOOST_REQUIRE( attend.event_id == to_string( elon_attend.event_id ) );
      BOOST_REQUIRE( attend.json == to_string( elon_attend.json ) );
      BOOST_REQUIRE( attend.interface == elon_attend.interface );
      BOOST_REQUIRE( attend.interested == elon_attend.interested );
      BOOST_REQUIRE( attend.attending == elon_attend.attending );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Attend community event" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY MANAGEMENT OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()