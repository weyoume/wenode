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

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.community_privacy = "open_public";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Alice Open Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( create.name );

      BOOST_REQUIRE( alice_community.founder == create.founder );
      BOOST_REQUIRE( alice_community.name == create.name );
      BOOST_REQUIRE( alice_community.community_privacy == community_privacy_type::OPEN_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( alice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpubliccommunity";
      create.community_privacy = "exclusive_public";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Bob Public Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( create.name );

      BOOST_REQUIRE( bob_community.founder == create.founder );
      BOOST_REQUIRE( bob_community.name == create.name );
      BOOST_REQUIRE( bob_community.community_privacy == community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( bob_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.community_privacy = "open_private";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Candice Private Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( create.name );

      BOOST_REQUIRE( candice_community.founder == create.founder );
      BOOST_REQUIRE( candice_community.name == create.name );
      BOOST_REQUIRE( candice_community.community_privacy == community_privacy_type::OPEN_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( candice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.community_privacy = "exclusive_private";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Dan Exclusive Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( create.name );

      BOOST_REQUIRE( dan_community.founder == create.founder );
      BOOST_REQUIRE( dan_community.name == create.name );
      BOOST_REQUIRE( dan_community.community_privacy == community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( dan_community.created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful community creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_CREATE_INTERVAL - BLOCK_INTERVAL );

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "mysecondcommunity";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "My Second Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
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
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "My Second Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( elon_private_secure_key, elon_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
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

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.community_privacy = "open_public";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Alice Open Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpubliccommunity";
      create.community_privacy = "exclusive_public";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Bob Public Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.community_privacy = "open_private";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Candice Private Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.community_privacy = "exclusive_private";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Dan Exclusive Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
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
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = "aliceopencommunity";
      comment.public_key = string();
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
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
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      
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
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      
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
      update.community_member_key = string( get_public_key( update.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_moderator_key = string( get_public_key( update.community, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_admin_key = string( get_public_key( update.community, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.display_name = "Bob Public Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, public_key_type( update.community_member_key ), "#{ \"valid\": true }" );
      update.tags.insert( "test" );
      update.pinned_author = "alice";
      update.pinned_permlink = "lorem";
      update.validate();

      tx.operations.push_back( update );
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
      update.pinned_author = "bob";
      update.community = "bobpubliccommunity";
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
      update.pinned_author = "candice";
      update.community = "candiceprivatecommunity";
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
      update.pinned_author = "dan";
      update.community = "danexclusivecommunity";
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

      BOOST_TEST_MESSAGE( "│   ├── Testing: Founder invite members" );

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

      string alice_private_connection_wif = graphene::utilities::key_to_wif( alice_private_connection_key );
      string bob_private_connection_wif = graphene::utilities::key_to_wif( bob_private_connection_key );
      string candice_private_connection_wif = graphene::utilities::key_to_wif( candice_private_connection_key );
      string dan_private_connection_wif = graphene::utilities::key_to_wif( dan_private_connection_key );
      string elon_private_connection_wif = graphene::utilities::key_to_wif( elon_private_connection_key );
      string fred_private_connection_wif = graphene::utilities::key_to_wif( fred_private_connection_key );

      generate_blocks( now() + fc::days(2), true );

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.community_privacy = "open_public";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Alice Open Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( create.name );
      const community_member_object& alice_community_member = db.get_community_member( create.name );

      BOOST_REQUIRE( alice_community.founder == create.founder );
      BOOST_REQUIRE( alice_community.community_privacy == community_privacy_type::OPEN_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( alice_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( alice_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( alice_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( alice_community_member.founder == create.founder );
      BOOST_REQUIRE( alice_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( alice_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( alice_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( alice_community_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpubliccommunity";
      create.community_privacy = "exclusive_public";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Bob Public Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( create.name );
      const community_member_object& bob_community_member = db.get_community_member( create.name );

      BOOST_REQUIRE( bob_community.founder == create.founder );
      BOOST_REQUIRE( bob_community.community_privacy == community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( bob_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( bob_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( bob_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( bob_community_member.founder == create.founder );
      BOOST_REQUIRE( bob_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( bob_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( bob_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( bob_community_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.community_privacy = "open_private";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Candice Private Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( create.name );
      const community_member_object& candice_community_member = db.get_community_member( create.name );

      BOOST_REQUIRE( candice_community.founder == create.founder );
      BOOST_REQUIRE( candice_community.community_privacy == community_privacy_type::OPEN_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( candice_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( candice_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( candice_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( candice_community_member.founder == create.founder );
      BOOST_REQUIRE( candice_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( candice_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( candice_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( candice_community_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.community_privacy = "exclusive_private";
      create.community_member_key = string( get_public_key( create.name, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_moderator_key = string( get_public_key( create.name, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.community_admin_key = string( get_public_key( create.name, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      create.display_name = "Dan Exclusive Community";
      create.details = "details";
      create.url = "https://www.url.com";
      create.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      create.json = "{ \"valid\": true }";
      create.json_private = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, public_key_type( create.community_member_key ), "#{ \"valid\": true }" );
      create.tags.insert( "test" );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_object& dan_community = db.get_community( create.name );
      const community_member_object& dan_community_member = db.get_community_member( create.name );

      BOOST_REQUIRE( dan_community.founder == create.founder );
      BOOST_REQUIRE( dan_community.community_privacy == community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( dan_community.community_member_key == public_key_type( create.community_member_key ) );
      BOOST_REQUIRE( dan_community.community_moderator_key == public_key_type( create.community_moderator_key ) );
      BOOST_REQUIRE( dan_community.community_admin_key == public_key_type( create.community_admin_key ) );

      BOOST_REQUIRE( dan_community_member.founder == create.founder );
      BOOST_REQUIRE( dan_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( dan_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( dan_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( dan_community_member.is_subscriber( create.founder ) );

      community_join_invite_operation invite;

      invite.signatory = "alice";
      invite.account = "alice";
      invite.member = "elon";
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, elon_public_secure_key, alice_private_connection_wif );
      invite.invited = true;
      invite.validate();

      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& invite_idx = db.get_index< community_join_invite_index >().indices().get< by_member_community >();

      auto invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "bob";
      invite.account = "bob";
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, elon_public_secure_key, bob_private_connection_wif );
   
      tx.operations.push_back( invite );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "candice";
      invite.account = "candice";
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = get_encrypted_message( candice_private_secure_key, candice_public_secure_key, elon_public_secure_key, candice_private_connection_wif );
   
      tx.operations.push_back( invite );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "dan";
      invite.account = "dan";
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, elon_public_secure_key, dan_private_connection_wif );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = "dan";
      invite.account = "dan";
      invite.member = "fred";
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = get_encrypted_message( dan_private_secure_key, dan_public_secure_key, fred_public_secure_key, dan_private_connection_wif );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder invite members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when non-member sends invites" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = get_encrypted_message( elon_private_secure_key, elon_public_secure_key, george_public_secure_key, alice_private_connection_wif );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = get_encrypted_message( elon_private_secure_key, elon_public_secure_key, haz_public_secure_key, bob_private_connection_wif );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = get_encrypted_message( elon_private_secure_key, elon_public_secure_key, isabelle_public_secure_key, candice_private_connection_wif );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = get_encrypted_message( elon_private_secure_key, elon_public_secure_key, jayme_public_secure_key, dan_private_connection_wif );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when non-member sends invites" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community join request" );

      community_join_request_operation request;

      request.signatory = "fred";
      request.account = "fred";
      request.community = "aliceopencommunity";
      request.message = "Hello";
      request.requested = true;
      request.validate();

      tx.operations.push_back( request );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& request_idx = db.get_index< community_join_request_index >().indices().get< by_account_community >();
      auto request_itr = request_idx.find( boost::make_tuple( request.account, request.community ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "bobpubliccommunity";
   
      tx.operations.push_back( request );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      request_itr = request_idx.find( boost::make_tuple( request.account, request.community ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "candiceprivatecommunity";
   
      tx.operations.push_back( request );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      request_itr = request_idx.find( boost::make_tuple( request.account, request.community ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "danexclusivecommunity";
   
      tx.operations.push_back( request );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // No join requests for exclusive community

      request_itr = request_idx.find( boost::make_tuple( request.account, request.community ) );

      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: community join request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept invite by incoming member" );

      community_invite_accept_operation invite_accept;

      invite_accept.signatory = "elon";
      invite_accept.account = "elon";
      invite_accept.community = "aliceopencommunity";
      invite_accept.accepted = true;
      invite_accept.validate();

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( invite_accept.community ).is_member( invite_accept.account ) );
      invite_itr = invite_idx.find( boost::make_tuple( invite_accept.account, invite_accept.community ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "bobpubliccommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( invite_accept.community ).is_member( invite_accept.account ) );
      invite_itr = invite_idx.find( boost::make_tuple( invite_accept.account, invite_accept.community ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "candiceprivatecommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( invite_accept.community ).is_member( invite_accept.account ) );
      invite_itr = invite_idx.find( boost::make_tuple( invite_accept.account, invite_accept.community ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "danexclusivecommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( invite_accept.community ).is_member( invite_accept.account ) );
      invite_itr = invite_idx.find( boost::make_tuple( invite_accept.account, invite_accept.community ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.signatory = "fred";
      invite_accept.account = "fred";

      tx.operations.push_back( invite_accept );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( invite_accept.community ).is_member( invite_accept.account ) );
      invite_itr = invite_idx.find( boost::make_tuple( invite_accept.account, invite_accept.community ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept invite by incoming member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder accept join request" );

      community_join_accept_operation join_accept;

      join_accept.signatory = "alice";
      join_accept.account = "alice";
      join_accept.member = "fred";
      join_accept.community = "aliceopencommunity";
      join_accept.encrypted_community_key = string( alice_public_posting_key );
      join_accept.accepted = true;
      join_accept.validate();

      tx.operations.push_back( join_accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( join_accept.community ).is_member( join_accept.member ) );
      request_itr = request_idx.find( boost::make_tuple( join_accept.member, join_accept.community ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      join_accept.signatory = "bob";
      join_accept.account = "bob";
      join_accept.community = "bobpubliccommunity";
      join_accept.encrypted_community_key = string( bob_public_posting_key );

      tx.operations.push_back( join_accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( join_accept.community ).is_member( join_accept.member ) );
      request_itr = request_idx.find( boost::make_tuple( join_accept.member, join_accept.community ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      join_accept.signatory = "candice";
      join_accept.account = "candice";
      join_accept.community = "candiceprivatecommunity";
      join_accept.encrypted_community_key = string( candice_public_posting_key );
      
      tx.operations.push_back( join_accept );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( join_accept.community ).is_member( join_accept.member ) );
      request_itr = request_idx.find( boost::make_tuple( join_accept.member, join_accept.community ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder accept join request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling member sending invites" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );
      
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling member sending invites" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder add moderator" );

      community_add_mod_operation add_mod;

      add_mod.signatory = "alice";
      add_mod.account = "alice";
      add_mod.community = "aliceopencommunity";
      add_mod.moderator = "elon";
      add_mod.added = true;
      add_mod.validate();

      tx.operations.push_back( add_mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = "bob";
      add_mod.account = "bob";
      add_mod.community = "bobpubliccommunity";

      tx.operations.push_back( add_mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = "candice";
      add_mod.account = "candice";
      add_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = "dan";
      add_mod.account = "dan";
      add_mod.community = "danexclusivecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder vote moderator" );

      community_vote_mod_operation vote_mod;

      vote_mod.signatory = "alice";
      vote_mod.account = "alice";
      vote_mod.community = "aliceopencommunity";
      vote_mod.moderator = "elon";
      vote_mod.vote_rank = 1;
      vote_mod.approved = true;
      vote_mod.validate();

      tx.operations.push_back( vote_mod );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      flat_map< account_name_type, share_type > m = db.get_community_member( vote_mod.community ).mod_weight;
      BOOST_REQUIRE( m[ vote_mod.moderator ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "bob";
      vote_mod.account = "bob";
      vote_mod.community = "bobpubliccommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      m = db.get_community_member( vote_mod.community ).mod_weight;
      BOOST_REQUIRE( m[ vote_mod.moderator ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "candice";
      vote_mod.account = "candice";
      vote_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      m = db.get_community_member( vote_mod.community ).mod_weight;
      BOOST_REQUIRE( m[ vote_mod.moderator ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "dan";
      vote_mod.account = "dan";
      vote_mod.community = "danexclusivecommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      m = db.get_community_member( vote_mod.community ).mod_weight;
      BOOST_REQUIRE( m[ vote_mod.moderator ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder vote moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling moderator sending invites and failure when repeated invite" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling moderator sending invites and failure when repeated invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder add administrator" );

      community_add_admin_operation admin;

      admin.signatory = "alice";
      admin.account = "alice";
      admin.community = "aliceopencommunity";
      admin.admin = "elon";
      admin.added = true;
      admin.validate();

      tx.operations.push_back( admin );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( admin.community ).is_administrator( admin.admin ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "bob";
      admin.account = "bob";
      admin.community = "bobpubliccommunity";

      tx.operations.push_back( admin );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( admin.community ).is_administrator( admin.admin ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "candice";
      admin.account = "candice";
      admin.community = "candiceprivatecommunity";

      tx.operations.push_back( admin );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( admin.community ).is_administrator( admin.admin ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "dan";
      admin.account = "dan";
      admin.community = "danexclusivecommunity";

      tx.operations.push_back( admin );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( admin.community ).is_administrator( admin.admin ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder add administrator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling administrators sending invites and failure when repeated invite" );

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( invite.member, invite.community ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling moderator sending invites and failure when repeated invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment creation handling for members and non-members" );

      generate_blocks( now() + fc::minutes(11), true );

      comment_operation comment;

      comment.signatory = "george";
      comment.author = "george";
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
      comment.parent_permlink = "lorem";
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
      tx.sign( george_private_posting_key, db.get_chain_id() );       // Non-members can create posts in open community
      db.push_transaction( tx, 0 );

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
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "isabelle";
      comment.author = "isabelle";
      comment.community = "candiceprivatecommunity";
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "jayme";
      comment.author = "jayme";
      comment.community = "danexclusivecommunity";
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "alice";
      comment.author = "alice";
      comment.community = "aliceopencommunity";
      comment.public_key = string();
      options.reach = "tag";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( alice_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "bob";
      comment.author = "bob";
      comment.community = "bobpubliccommunity";
      comment.public_key = string();
      options.reach = "tag";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( bob_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "candice";
      comment.author = "candice";
      comment.community = "candiceprivatecommunity";
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( candice_comment.community == comment.community );
      BOOST_REQUIRE( candice_comment.is_encrypted() );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "dan";
      comment.author = "dan";
      comment.community = "danexclusivecommunity";
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      options.reach = "community";
      comment.options = options;

      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& dan_comment = db.get_comment( comment.author, comment.permlink );

      BOOST_REQUIRE( dan_comment.community == comment.community );
      BOOST_REQUIRE( dan_comment.is_encrypted() );

      tx.operations.clear();
      tx.signatures.clear();

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
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_alice_vote = db.get_comment_vote( vote.voter, alice_comment.id );

      BOOST_REQUIRE( george_alice_vote.voter == vote.voter );
      BOOST_REQUIRE( george_alice_vote.comment == alice_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "haz";
      vote.voter = "haz";
      vote.author = "bob";

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_bob_vote = db.get_comment_vote( vote.voter, bob_comment.id );

      BOOST_REQUIRE( haz_bob_vote.voter == vote.voter );
      BOOST_REQUIRE( haz_bob_vote.comment == bob_comment.id );

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
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_view = db.get_comment_view( view.viewer, alice_comment.id );

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

      const comment_view_object& haz_view = db.get_comment_view( view.viewer, bob_comment.id );

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
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_share = db.get_comment_share( share.sharer, alice_comment.id );

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

      const comment_share_object& haz_bob_share = db.get_comment_share( share.sharer, bob_comment.id );

      BOOST_REQUIRE( haz_bob_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_bob_share.comment == bob_comment.id );
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

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment share handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment creation handling by members" );

      community_invite_accept_operation accept;

      accept.signatory = "george";
      accept.account = "george";
      accept.community = "aliceopencommunity";
      accept.accepted = true;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "haz";
      accept.account = "haz";
      accept.community = "bobpubliccommunity";

      tx.operations.push_back( accept );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "isabelle";
      accept.account = "isabelle";
      accept.community = "candiceprivatecommunity";

      tx.operations.push_back( accept );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "jayme";
      accept.account = "jayme";
      accept.community = "danexclusivecommunity";

      tx.operations.push_back( accept );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
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
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
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
      comment.public_key = string( get_public_key( comment.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
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

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment creation handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment vote handling by members" );

      vote.signatory = "george";
      vote.voter = "george";
      vote.author = "george";
      vote.permlink = "ipsum";
      
      tx.operations.push_back( vote );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_vote = db.get_comment_vote( vote.voter, george_comment2.id );

      BOOST_REQUIRE( george_vote.voter == vote.voter );
      BOOST_REQUIRE( george_vote.comment == george_comment2.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "haz";
      vote.voter = "haz";
      vote.author = "haz";

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_vote = db.get_comment_vote( vote.voter, haz_comment.id );

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

      const comment_vote_object& isabelle_vote = db.get_comment_vote( vote.voter, isabelle_comment.id );

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

      const comment_vote_object& jayme_vote = db.get_comment_vote( vote.voter, jayme_comment.id );

      BOOST_REQUIRE( jayme_vote.voter == vote.voter );
      BOOST_REQUIRE( jayme_vote.comment == jayme_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment vote handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment view handling by members" );

      view.signatory = "george";
      view.viewer = "george";
      view.author = "george";
      view.permlink = "ipsum";

      tx.operations.push_back( view );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_george_view = db.get_comment_view( view.viewer, george_comment2.id );

      BOOST_REQUIRE( george_george_view.viewer == view.viewer );
      BOOST_REQUIRE( george_george_view.comment == george_comment2.id );
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

      const comment_view_object& haz_haz_view = db.get_comment_view( view.viewer, haz_comment.id );

      BOOST_REQUIRE( haz_haz_view.viewer == view.viewer );
      BOOST_REQUIRE( haz_haz_view.comment == haz_comment.id );
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

      const comment_view_object& isabelle_isabelle_view = db.get_comment_view( view.viewer, isabelle_comment.id );

      BOOST_REQUIRE( isabelle_isabelle_view.viewer == view.viewer );
      BOOST_REQUIRE( isabelle_isabelle_view.comment == isabelle_comment.id );
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

      const comment_view_object& jayme_jayme_view = db.get_comment_view( view.viewer, jayme_comment.id );

      BOOST_REQUIRE( jayme_jayme_view.viewer == view.viewer );
      BOOST_REQUIRE( jayme_jayme_view.comment == jayme_comment.id );
      BOOST_REQUIRE( jayme_jayme_view.supernode == view.supernode );
      BOOST_REQUIRE( jayme_jayme_view.interface == view.interface );
      BOOST_REQUIRE( jayme_jayme_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment view handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment share handling by members" );

      share.signatory = "george";
      share.sharer = "george";
      share.author = "george";
      share.permlink = "ipsum";
      
      tx.operations.push_back( share );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_george_share = db.get_comment_share( share.sharer, george_comment2.id );

      BOOST_REQUIRE( george_george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_george_share.comment == george_comment2.id );
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

      const comment_share_object& haz_haz_share = db.get_comment_share( share.sharer, haz_comment.id );

      BOOST_REQUIRE( haz_haz_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_haz_share.comment == haz_comment.id );
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

      const comment_share_object& isabelle_isabelle_share = db.get_comment_share( share.sharer, isabelle_comment.id );

      BOOST_REQUIRE( isabelle_isabelle_share.sharer == share.sharer );
      BOOST_REQUIRE( isabelle_isabelle_share.comment == isabelle_comment.id );
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

      const comment_share_object& jayme_jayme_share = db.get_comment_share( share.sharer, jayme_comment.id );

      BOOST_REQUIRE( jayme_jayme_share.sharer == share.sharer );
      BOOST_REQUIRE( jayme_jayme_share.comment == jayme_comment.id );
      BOOST_REQUIRE( jayme_jayme_share.interface == share.interface );
      BOOST_REQUIRE( jayme_jayme_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

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
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& tag_idx = db.get_index< comment_moderation_index >().indices().get< by_comment_moderator >();
      auto tag_itr = tag_idx.find( std::make_tuple( george_comment2.id, tag.moderator ) );

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

      tag_itr = tag_idx.find( std::make_tuple( haz_comment.id, tag.moderator ) );

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

      tag_itr = tag_idx.find( std::make_tuple( isabelle_comment.id, tag.moderator ) );

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

      tag_itr = tag_idx.find( std::make_tuple( jayme_comment.id, tag.moderator ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: moderator tag by community moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator add moderator" );

      add_mod.signatory = "elon";
      add_mod.account = "elon";
      add_mod.community = "aliceopencommunity";
      add_mod.moderator = "fred";
      add_mod.added = true;
      add_mod.validate();

      tx.operations.push_back( add_mod );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      add_mod.community = "bobpubliccommunity";

      tx.operations.push_back( add_mod );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      add_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      add_mod.community = "danexclusivecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator update community" );

      community_update_operation update;

      update.signatory = "elon";
      update.account = "elon";
      update.community = "aliceopencommunity";
      update.community_member_key = string( get_public_key( update.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_moderator_key = string( get_public_key( update.community, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_admin_key = string( get_public_key( update.community, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.display_name = "Alice Public Community";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.profile_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.cover_image = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      update.json = "{ \"valid\": true }";
      update.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, public_key_type( update.community_member_key ), "#{ \"valid\": true }" );
      update.tags.insert( "test" );
      update.pinned_author = "alice";
      update.pinned_permlink = "lorem";
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

      update.pinned_author = "bob";
      update.community = "bobpubliccommunity";
      update.community_member_key = string( get_public_key( update.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_moderator_key = string( get_public_key( update.community, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_admin_key = string( get_public_key( update.community, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
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

      update.pinned_author = "candice";
      update.community = "candiceprivatecommunity";
      update.community_member_key = string( get_public_key( update.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_moderator_key = string( get_public_key( update.community, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_admin_key = string( get_public_key( update.community, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
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

      update.pinned_author = "dan";
      update.community = "danexclusivecommunity";
      update.community_member_key = string( get_public_key( update.community, MEMBER_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_moderator_key = string( get_public_key( update.community, MODERATOR_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
      update.community_admin_key = string( get_public_key( update.community, ADMIN_KEY_STR, INIT_ACCOUNT_PASSWORD ) );
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

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator update community" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove moderator" );

      add_mod.signatory = "alice";
      add_mod.account = "alice";
      add_mod.community = "aliceopencommunity";
      add_mod.moderator = "fred";
      add_mod.added = false;
      add_mod.validate();

      tx.operations.push_back( add_mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_member( add_mod.moderator ) );
      BOOST_REQUIRE( !db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = "bob";
      add_mod.account = "bob";
      add_mod.community = "bobpubliccommunity";

      tx.operations.push_back( add_mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_member( add_mod.moderator ) );
      BOOST_REQUIRE( !db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = "candice";
      add_mod.account = "candice";
      add_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_member( add_mod.moderator ) );
      BOOST_REQUIRE( !db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = "dan";
      add_mod.account = "dan";
      add_mod.community = "danexclusivecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( add_mod.community ).is_member( add_mod.moderator ) );
      BOOST_REQUIRE( !db.get_community_member( add_mod.community ).is_moderator( add_mod.moderator ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove member" );

      community_remove_member_operation remove;

      remove.signatory = "alice";
      remove.account = "alice";
      remove.community = "aliceopencommunity";
      remove.member = "fred";
      remove.validate();

      tx.operations.push_back( remove );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Open Public boards cannot remove members

      BOOST_REQUIRE( db.get_community_member( remove.community ).is_member( remove.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "bob";
      remove.account = "bob";
      remove.community = "bobpubliccommunity";

      tx.operations.push_back( remove );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !db.get_community_member( remove.community ).is_member( remove.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "candice";
      remove.account = "candice";
      remove.community = "candiceprivatecommunity";

      tx.operations.push_back( remove );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !db.get_community_member( remove.community ).is_member( remove.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "dan";
      remove.account = "dan";
      remove.community = "danexclusivecommunity";

      tx.operations.push_back( remove );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !db.get_community_member( remove.community ).is_member( remove.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove moderator" );

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
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "bobpubliccommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "candiceprivatecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "danexclusivecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( subscribe.community ).is_subscriber( subscribe.account ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: community subscription" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder blacklist account" );

      community_blacklist_operation blacklist;

      blacklist.signatory = "alice";
      blacklist.account = "alice";
      blacklist.member = "fred";
      blacklist.community = "aliceopencommunity";
      blacklist.validate();

      tx.operations.push_back( blacklist );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Open Public boards cannot blacklist
   
      BOOST_REQUIRE( !db.get_community_member( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "bob";
      blacklist.account = "bob";
      blacklist.community = "bobpubliccommunity";

      tx.operations.push_back( blacklist );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "candice";
      blacklist.account = "candice";
      blacklist.community = "candiceprivatecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "dan";
      blacklist.account = "dan";
      blacklist.community = "danexclusivecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( blacklist.community ).is_blacklisted( blacklist.member ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder blacklist account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: transfer community ownership" );

      community_transfer_ownership_operation transfer;

      generate_blocks( now() + fc::minutes(11), true );
      generate_block();

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.community = "aliceopencommunity";
      transfer.new_founder = "elon";
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( transfer.community ).founder == transfer.new_founder );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "bob";
      transfer.account = "bob";
      transfer.community = "bobpubliccommunity";

      tx.operations.push_back( transfer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( transfer.community ).founder == transfer.new_founder );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "candice";
      transfer.account = "candice";
      transfer.community = "candiceprivatecommunity";

      tx.operations.push_back( transfer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( transfer.community ).founder == transfer.new_founder );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "dan";
      transfer.account = "dan";
      transfer.community = "danexclusivecommunity";

      tx.operations.push_back( transfer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_community_member( transfer.community ).founder == transfer.new_founder );

      tx.operations.clear();
      tx.signatures.clear();

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
      event.event_start_time = now() + fc::days(1);
      event.event_end_time = now() + fc::days(1) + fc::hours(8);
      event.active = true;
      event.validate();

      tx.operations.push_back( event );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_event_object& alice_event = db.get_community_event( event.community, event.event_id );

      BOOST_REQUIRE( event.account == alice_event.account );
      BOOST_REQUIRE( event.community == alice_event.community );
      BOOST_REQUIRE( event.event_id == to_string( alice_event.event_id ) );
      BOOST_REQUIRE( event.event_name == to_string( alice_event.event_name ) );
      BOOST_REQUIRE( event.location == to_string( alice_event.location ) );
      BOOST_REQUIRE( event.latitude == alice_event.latitude );
      BOOST_REQUIRE( event.longitude == alice_event.longitude );
      BOOST_REQUIRE( event.details == to_string( alice_event.details ) );
      BOOST_REQUIRE( event.url == to_string( alice_event.url ) );
      BOOST_REQUIRE( event.json == to_string( alice_event.json ) );
      BOOST_REQUIRE( event.event_start_time == alice_event.event_start_time );
      BOOST_REQUIRE( event.event_end_time == alice_event.event_end_time );
      BOOST_REQUIRE( alice_event.active );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create community event" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Attend community event" );

      community_event_attend_operation attend;

      attend.signatory = "george";
      attend.account = "george";
      attend.community = "aliceopencommunity";
      attend.event_id = "a14e898a-9d92-4461-8011-64a43521c051";
      attend.interested = true;
      attend.attending = true;
      attend.not_attending = false;
      attend.validate();

      tx.operations.push_back( attend );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const community_event_object& new_alice_event = db.get_community_event( attend.community, attend.event_id );

      BOOST_REQUIRE( new_alice_event.is_interested( attend.account ) );
      BOOST_REQUIRE( new_alice_event.is_attending( attend.account ) );
      BOOST_REQUIRE( !new_alice_event.is_not_attending( attend.account ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Attend community event" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY MANAGEMENT OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()