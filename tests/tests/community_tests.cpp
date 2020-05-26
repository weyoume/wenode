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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      community_create_operation create;

      create.signatory = alice.name;
      create.founder = alice.name;
      create.name = "aliceopencommunity";
      create.community_privacy = "open_public";
      create.community_public_key = string( alice_public_posting_key );
      create.json = "{ \"valid\": true }";
      create.json_private = "{ \"valid\": true }";
      create.details = "details";
      create.url = "https://www.url.com";
      create.reward_currency = SYMBOL_COIN;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( "aliceopencommunity" );

      BOOST_REQUIRE( alice_community.founder == create.founder );
      BOOST_REQUIRE( alice_community.name == create.name );
      BOOST_REQUIRE( alice_community.community_privacy == community_privacy_type::OPEN_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( alice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = bob.name;
      create.founder = bob.name;
      create.name = "bobpubliccommunity";
      create.community_privacy = "exclusive_public";
      create.community_public_key = string( bob_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( "bobpubliccommunity" );

      BOOST_REQUIRE( bob_community.founder == create.founder );
      BOOST_REQUIRE( bob_community.name == create.name );
      BOOST_REQUIRE( bob_community.community_privacy == community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( bob_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = candice.name;
      create.founder = candice.name;
      create.name = "candiceprivatecommunity";
      create.community_privacy = "open_private";
      create.community_public_key = string( candice_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( "candiceprivatecommunity" );

      BOOST_REQUIRE( candice_community.founder == create.founder );
      BOOST_REQUIRE( candice_community.name == create.name );
      BOOST_REQUIRE( candice_community.community_privacy == community_privacy_type::OPEN_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( candice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = dan.name;
      create.founder = dan.name;
      create.name = "danexclusivecommunity";
      create.community_privacy = "exclusive_private";
      create.community_public_key = string( dan_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( "danexclusivecommunity" );

      BOOST_REQUIRE( dan_community.founder == create.founder );
      BOOST_REQUIRE( dan_community.name == create.name );
      BOOST_REQUIRE( dan_community.community_privacy == community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( dan_community.created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful community creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_CREATE_INTERVAL - BLOCK_INTERVAL );

      create.signatory = alice.name;
      create.founder = alice.name;
      create.name = "mysecondcommunity";
      create.community_public_key = string( alice_public_posting_key );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success after MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks(10);

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      tx.operations.push_back( create );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success after MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community name already exists" );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = elon.name;
      create.founder = elon.name;
      create.name = "aliceopencommunity";
      create.community_public_key = string( elon_public_posting_key );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( elon_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

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

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      community_create_operation create;

      create.signatory = alice.name;
      create.founder = alice.name;
      create.name = "aliceopencommunity";
      create.community_privacy = "open_public";
      create.community_public_key = string( alice_public_posting_key );
      create.json = "{ \"valid\": true }";
      create.json_private = "{ \"valid\": true }";
      create.details = "details";
      create.url = "https://www.url.com";
      create.reward_currency = SYMBOL_COIN;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = bob.name;
      create.founder = bob.name;
      create.name = "bobpubliccommunity";
      create.community_privacy = "exclusive_public";
      create.community_public_key = string( bob_public_posting_key );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = candice.name;
      create.founder = candice.name;
      create.name = "candiceprivatecommunity";
      create.community_privacy = "open_private";
      create.community_public_key = string( candice_public_posting_key );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = dan.name;
      create.founder = dan.name;
      create.name = "danexclusivecommunity";
      create.community_privacy = "exclusive_private";
      create.community_public_key = string( dan_public_posting_key );
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment_operation comment;

      comment.signatory = alice.name;
      comment.author = alice.name;
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.url = "https://www.url.com";
      comment.community = "aliceopencommunity";
      comment.public_key = "";
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{ \"valid\": true }";
      comment.latitude = 37.8136;
      comment.longitude = 144.9631;
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.reply_price = asset( 0, SYMBOL_COIN );
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

      comment.signatory = bob.name;
      comment.author = bob.name;
      comment.public_key = "";
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = candice.name;
      comment.author = candice.name;
      comment.public_key = string( candice_public_posting_key );
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = dan.name;
      comment.author = dan.name;
      comment.public_key = string( dan_public_posting_key );
      comment.community = "danexclusivecommunity";

      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_UPDATE_INTERVAL );

      community_update_operation update;

      update.signatory = alice.name;
      update.account = alice.name;
      update.community = "aliceopencommunity";
      update.json = "{ \"valid\": true }";
      update.json_private = "{ \"valid\": true }";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.pinned_author = alice.name;
      update.pinned_permlink = "lorem";
      update.reward_currency = SYMBOL_COIN;
      update.max_rating = 9;
      update.flags = 0;
      update.permissions = COMMUNITY_PERMISSION_MASK;
      update.validate();

      tx.operations.push_back( update );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( "aliceopencommunity" );

      BOOST_REQUIRE( alice_community.founder == update.account );
      BOOST_REQUIRE( alice_community.name == update.community );
      BOOST_REQUIRE( to_string( alice_community.details ) == update.details );
      BOOST_REQUIRE( to_string( alice_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = bob.name;
      update.account = bob.name;
      update.pinned_author = bob.name;
      update.community = "bobpubliccommunity";
      update.validate();

      tx.operations.push_back( update );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( "bobpubliccommunity" );

      BOOST_REQUIRE( bob_community.founder == update.account );
      BOOST_REQUIRE( bob_community.name == update.community );
      BOOST_REQUIRE( to_string( bob_community.details ) == update.details );
      BOOST_REQUIRE( to_string( bob_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = candice.name;
      update.account = candice.name;
      update.pinned_author = candice.name;
      update.community = "candiceprivatecommunity";
      update.validate();

      tx.operations.push_back( update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( "candiceprivatecommunity" );

      BOOST_REQUIRE( candice_community.founder == update.account );
      BOOST_REQUIRE( candice_community.name == update.community );
      BOOST_REQUIRE( to_string( candice_community.details ) == update.details );
      BOOST_REQUIRE( to_string( candice_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = dan.name;
      update.account = dan.name;
      update.pinned_author = dan.name;
      update.community = "danexclusivecommunity";
      update.validate();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( "danexclusivecommunity" );

      BOOST_REQUIRE( dan_community.founder == update.account );
      BOOST_REQUIRE( dan_community.name == update.community );
      BOOST_REQUIRE( to_string( dan_community.details ) == update.details );
      BOOST_REQUIRE( to_string( dan_community.url ) == update.url );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder community update sequence" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community update before MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_UPDATE_INTERVAL - BLOCK_INTERVAL );

      update.details = "even more updated details";

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community update before MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success after MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks(10);

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( dan_community.founder == update.account );
      BOOST_REQUIRE( dan_community.name == update.community );
      BOOST_REQUIRE( to_string( dan_community.details ) == update.details );
      BOOST_REQUIRE( to_string( dan_community.url ) == update.url );
      BOOST_REQUIRE( dan_community.created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success after MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community name already exists" );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = elon.name;
      create.founder = elon.name;
      create.name = "aliceopencommunity";

      tx.operations.push_back( create );
      tx.sign( elon_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community name already exists" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY UPDATE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( community_management_sequence_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY MANAGEMENT OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder invite members" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn)(leonie)(margot)(natalie) );

      fund_stake( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( alice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( bob.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( candice.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( dan.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( elon.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( fred.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( george.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( haz.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( isabelle.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( jayme.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( kathryn.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( leonie.name, asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      community_create_operation create;

      create.signatory = alice.name;
      create.founder = alice.name;
      create.name = "aliceopencommunity";
      create.community_privacy = "open_public";
      create.community_public_key = string( alice_public_posting_key );
      create.json = "{ \"valid\": true }";
      create.json_private = "{ \"valid\": true }";
      create.details = "details";
      create.url = "https://www.url.com";
      create.reward_currency = SYMBOL_COIN;
      create.max_rating = 9;
      create.flags = 0;
      create.permissions = COMMUNITY_PERMISSION_MASK;
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( "aliceopencommunity" );
      const community_member_object& alice_community_member = db.get_community_member( "aliceopencommunity" );

      BOOST_REQUIRE( alice_community.founder == create.founder );
      BOOST_REQUIRE( alice_community.community_privacy == community_privacy_type::OPEN_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( alice_community.community_public_key == public_key_type( create.community_public_key ) );

      BOOST_REQUIRE( alice_community_member.founder == create.founder );
      BOOST_REQUIRE( alice_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( alice_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( alice_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( alice_community_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = bob.name;
      create.founder = bob.name;
      create.name = "bobpubliccommunity";
      create.community_privacy = "exclusive_public";
      create.community_public_key = string( bob_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( "bobpubliccommunity" );
      const community_member_object& bob_community_member = db.get_community_member( "bobpubliccommunity" );

      BOOST_REQUIRE( bob_community.founder == create.founder );
      BOOST_REQUIRE( bob_community.community_privacy == community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY );
      BOOST_REQUIRE( bob_community.community_public_key == public_key_type( create.community_public_key ) );

      BOOST_REQUIRE( bob_community_member.founder == create.founder );
      BOOST_REQUIRE( bob_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( bob_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( bob_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( bob_community_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = candice.name;
      create.founder = candice.name;
      create.name = "candiceprivatecommunity";
      create.community_privacy = "open_private";
      create.community_public_key = string( candice_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( "candiceprivatecommunity" );
      const community_member_object& candice_community_member = db.get_community_member( "candiceprivatecommunity" );

      BOOST_REQUIRE( candice_community.founder == create.founder );
      BOOST_REQUIRE( candice_community.community_privacy == community_privacy_type::OPEN_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( candice_community.community_public_key == public_key_type( create.community_public_key ) );

      BOOST_REQUIRE( candice_community_member.founder == create.founder );
      BOOST_REQUIRE( candice_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( candice_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( candice_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( candice_community_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = dan.name;
      create.founder = dan.name;
      create.name = "danexclusivecommunity";
      create.community_privacy = "exclusive_private";
      create.community_public_key = string( dan_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( "danexclusivecommunity" );
      const community_member_object& dan_community_member = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( dan_community.founder == create.founder );
      BOOST_REQUIRE( dan_community.community_privacy == community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY );
      BOOST_REQUIRE( dan_community.community_public_key == public_key_type( create.community_public_key ) );

      BOOST_REQUIRE( dan_community_member.founder == create.founder );
      BOOST_REQUIRE( dan_community_member.is_administrator( create.founder ) );
      BOOST_REQUIRE( dan_community_member.is_moderator( create.founder ) );
      BOOST_REQUIRE( dan_community_member.is_member( create.founder ) );
      BOOST_REQUIRE( dan_community_member.is_subscriber( create.founder ) );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( create );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      community_join_invite_operation invite;

      invite.signatory = alice.name;
      invite.account = alice.name;
      invite.member = elon.name;
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;
      invite.validate();

      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& invite_idx = db.get_index< community_join_invite_index >().indices().get< by_member_community >();

      auto invite_itr = invite_idx.find( boost::make_tuple( elon.name, "aliceopencommunity" ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = bob.name;
      invite.account = bob.name;
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( elon.name, "bobpubliccommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = candice.name;
      invite.account = candice.name;
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( elon.name, "candiceprivatecommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = dan.name;
      invite.account = dan.name;
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( elon.name, "danexclusivecommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.signatory = dan.name;
      invite.account = dan.name;
      invite.member = fred.name;
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( fred.name, "danexclusivecommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder invite members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when non-member sends invites" );

      invite.signatory = elon.name;
      invite.account = elon.name;
      invite.member = george.name;
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = haz.name;
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = isabelle.name;
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = jayme.name;
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when non-member sends invites" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community join request" );

      community_join_request_operation request;

      request.signatory = fred.name;
      request.account = fred.name;
      request.community = "aliceopencommunity";
      request.message = "Hello";
      request.requested = true;
      request.validate();

      tx.operations.push_back( request );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& request_idx = db.get_index< community_join_request_index >().indices().get< by_account_community >();

      auto request_itr = request_idx.find( boost::make_tuple( fred.name, "aliceopencommunity" ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "bobpubliccommunity";
   
      tx.operations.push_back( request );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      request_itr = request_idx.find( boost::make_tuple( fred.name, "bobpubliccommunity" ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "candiceprivatecommunity";
   
      tx.operations.push_back( request );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      request_itr = request_idx.find( boost::make_tuple( fred.name, "candiceprivatecommunity" ) );

      BOOST_REQUIRE( request_itr != request_idx.end() );
      BOOST_REQUIRE( request_itr->account == request.account );
      BOOST_REQUIRE( request_itr->community == request.community );
      BOOST_REQUIRE( request_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      request.community = "danexclusivecommunity";
   
      tx.operations.push_back( request );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // No join requests for exclusive community

      request_itr = request_idx.find( boost::make_tuple( fred.name, "danexclusivecommunity" ) );

      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: community join request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept invite by incoming member" );

      community_invite_accept_operation invite_accept;

      invite_accept.signatory = elon.name;
      invite_accept.account = elon.name;
      invite_accept.community = "aliceopencommunity";
      invite_accept.accepted = true;
      invite_accept.validate();

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );

      BOOST_REQUIRE( community_member_a.is_member( elon.name ) );
      invite_itr = invite_idx.find( boost::make_tuple( elon.name, "aliceopencommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "bobpubliccommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );

      BOOST_REQUIRE( community_member_b.is_member( elon.name ) );
      invite_itr = invite_idx.find( boost::make_tuple( elon.name, "bobpubliccommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "candiceprivatecommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );

      BOOST_REQUIRE( community_member_c.is_member( elon.name ) );
      invite_itr = invite_idx.find( boost::make_tuple( elon.name, "candiceprivatecommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "danexclusivecommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );

      BOOST_REQUIRE( community_member_d.is_member( elon.name ) );
      invite_itr = invite_idx.find( boost::make_tuple( elon.name, "danexclusivecommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.signatory = fred.name;
      invite_accept.account = fred.name;

      tx.operations.push_back( invite_accept );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.is_member( fred.name ) );
      invite_itr = invite_idx.find( boost::make_tuple( fred.name, "danexclusivecommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept invite by incoming member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder accept join request" );

      community_join_accept_operation join_accept;

      join_accept.signatory = alice.name;
      join_accept.account = alice.name;
      join_accept.member = fred.name;
      join_accept.community = "aliceopencommunity";
      join_accept.encrypted_community_key = string( alice_public_posting_key );
      join_accept.accepted = true;
      join_accept.validate();

      tx.operations.push_back( join_accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.is_member( fred.name ) );
      request_itr = request_idx.find( boost::make_tuple( fred.name, "aliceopencommunity" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      join_accept.signatory = bob.name;
      join_accept.account = bob.name;
      join_accept.community = "bobpubliccommunity";
      join_accept.encrypted_community_key = string( bob_public_posting_key );

      tx.operations.push_back( join_accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.is_member( fred.name ) );
      request_itr = request_idx.find( boost::make_tuple( fred.name, "bobpubliccommunity" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      join_accept.signatory = candice.name;
      join_accept.account = candice.name;
      join_accept.community = "candiceprivatecommunity";
      join_accept.encrypted_community_key = string( candice_public_posting_key );
      
      tx.operations.push_back( join_accept );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.is_member( fred.name ) );
      request_itr = request_idx.find( boost::make_tuple( fred.name, "candiceprivatecommunity" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder accept join request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling member sending invites" );

      invite.signatory = elon.name;
      invite.account = elon.name;
      invite.member = george.name;
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( george.name, "aliceopencommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = haz.name;
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( haz.name, "bobpubliccommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = isabelle.name;
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = jayme.name;
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling member sending invites" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder add moderator" );

      community_add_mod_operation add_mod;

      add_mod.signatory = alice.name;
      add_mod.account = alice.name;
      add_mod.community = "aliceopencommunity";
      add_mod.moderator = elon.name;
      add_mod.added = true;
      add_mod.validate();

      tx.operations.push_back( add_mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.is_moderator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = bob.name;
      add_mod.account = bob.name;
      add_mod.community = "bobpubliccommunity";

      tx.operations.push_back( add_mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.is_moderator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = candice.name;
      add_mod.account = candice.name;
      add_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.is_moderator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = dan.name;
      add_mod.account = dan.name;
      add_mod.community = "danexclusivecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.is_moderator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder vote moderator" );

      community_vote_mod_operation vote_mod;

      vote_mod.signatory = alice.name;
      vote_mod.account = alice.name;
      vote_mod.community = "aliceopencommunity";
      vote_mod.moderator = elon.name;
      vote_mod.vote_rank = 1;
      vote_mod.approved = true;
      vote_mod.validate();

      tx.operations.push_back( vote_mod );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      flat_map< account_name_type, share_type > m = community_member_a.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = bob.name;
      vote_mod.account = bob.name;
      vote_mod.community = "bobpubliccommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      m = community_member_b.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = candice.name;
      vote_mod.account = candice.name;
      vote_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      m = community_member_c.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = dan.name;
      vote_mod.account = dan.name;
      vote_mod.community = "danexclusivecommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      m = community_member_d.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder vote moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling moderator sending invites and failure when repeated invite" );

      invite.signatory = elon.name;
      invite.account = elon.name;
      invite.member = george.name;
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = haz.name;
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = isabelle.name;
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( isabelle.name, "candiceprivatecommunity" ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = jayme.name;
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling moderator sending invites and failure when repeated invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder add administrator" );

      community_add_admin_operation admin;

      admin.signatory = alice.name;
      admin.account = alice.name;
      admin.community = "aliceopencommunity";
      admin.admin = elon.name;
      admin.added = true;
      admin.validate();

      tx.operations.push_back( admin );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.is_administrator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = bob.name;
      admin.account = bob.name;
      admin.community = "bobpubliccommunity";

      tx.operations.push_back( admin );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.is_administrator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = candice.name;
      admin.account = candice.name;
      admin.community = "candiceprivatecommunity";

      tx.operations.push_back( admin );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.is_administrator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = dan.name;
      admin.account = dan.name;
      admin.community = "danexclusivecommunity";

      tx.operations.push_back( admin );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.is_administrator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder add administrator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: handling administrators sending invites and failure when repeated invite" );

      invite.signatory = elon.name;
      invite.account = elon.name;
      invite.member = george.name;
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = haz.name;
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = isabelle.name;
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Invite already exists

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = jayme.name;
      invite.community = "danexclusivecommunity";
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      invite_itr = invite_idx.find( boost::make_tuple( jayme.name, "danexclusivecommunity" ) );

      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling moderator sending invites and failure when repeated invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment creation handling for members and non-members" );

      comment_operation comment;

      comment.signatory = george.name;
      comment.author = george.name;
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.url = "https://www.url.com";
      comment.community = "aliceopencommunity";
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "lorem";
      comment.json = "{ \"valid\": true }";
      comment.latitude = 37.8136;
      comment.longitude = 144.9631;
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.reply_price = asset( 0, SYMBOL_COIN );
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

      const comment_object& george_comment = db.get_comment( george.name, string( "lorem" ) );

      BOOST_REQUIRE( george_comment.author == george.name );
      BOOST_REQUIRE( george_comment.community == "aliceopencommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = haz.name;
      comment.author = haz.name;
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = isabelle.name;
      comment.author = isabelle.name;
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = jayme.name;
      comment.author = jayme.name;
      comment.community = "danexclusivecommunity";

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = alice.name;
      comment.author = alice.name;
      comment.community = "aliceopencommunity";

      tx.operations.push_back( comment );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( alice.name, string( "lorem" ) );

      BOOST_REQUIRE( alice_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = bob.name;
      comment.author = bob.name;
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( bob.name, string( "lorem" ) );

      BOOST_REQUIRE( bob_comment.community == comment.community );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = candice.name;
      comment.author = candice.name;
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( candice.name, string( "lorem" ) );

      BOOST_REQUIRE( candice_comment.community == comment.community );
      BOOST_REQUIRE( candice_comment.is_encrypted() );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = dan.name;
      comment.author = dan.name;
      comment.community = "danexclusivecommunity";

      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& dan_comment = db.get_comment( dan.name, string( "lorem" ) );

      BOOST_REQUIRE( dan_comment.community == comment.community );
      BOOST_REQUIRE( dan_comment.is_encrypted() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment creation handling for members and non-members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment vote handling before becoming members" );

      vote_operation vote;

      vote.signatory = george.name;
      vote.voter = george.name;
      vote.author = alice.name;
      vote.permlink = "lorem";
      vote.weight = PERCENT_100;
      vote.interface = INIT_ACCOUNT;
      vote.validate();
      
      tx.operations.push_back( vote );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_alice_vote = db.get_comment_vote( george.name, alice_comment.id );

      BOOST_REQUIRE( george_alice_vote.voter == vote.voter );
      BOOST_REQUIRE( george_alice_vote.comment == alice_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = haz.name;
      vote.voter = haz.name;
      vote.author = bob.name;

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_bob_vote = db.get_comment_vote( haz.name, bob_comment.id );

      BOOST_REQUIRE( haz_bob_vote.voter == vote.voter );
      BOOST_REQUIRE( haz_bob_vote.comment == bob_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = isabelle.name;
      vote.voter = isabelle.name;
      vote.author = candice.name;

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot vote on posts

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = jayme.name;
      vote.voter = jayme.name;
      vote.author = dan.name;

      tx.operations.push_back( vote );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot vote on posts

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment vote handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment view handling before becoming members" );

      view_operation view;

      view.signatory = george.name;
      view.viewer = george.name;
      view.author = alice.name;
      view.permlink = "lorem";
      view.interface = INIT_ACCOUNT;
      view.supernode = INIT_ACCOUNT;
      view.viewed = true;
      view.validate();
      
      tx.operations.push_back( view );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_view = db.get_comment_view( george.name, alice_comment.id );

      BOOST_REQUIRE( george_view.viewer == view.viewer );
      BOOST_REQUIRE( george_view.comment == alice_comment.id );
      BOOST_REQUIRE( george_view.supernode == view.supernode );
      BOOST_REQUIRE( george_view.interface == view.interface );
      BOOST_REQUIRE( george_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = haz.name;
      view.viewer = haz.name;
      view.author = bob.name;

      tx.operations.push_back( view );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& haz_view = db.get_comment_view( haz.name, bob_comment.id );

      BOOST_REQUIRE( haz_view.viewer == view.viewer );
      BOOST_REQUIRE( haz_view.comment == bob_comment.id );
      BOOST_REQUIRE( haz_view.supernode == view.supernode );
      BOOST_REQUIRE( haz_view.interface == view.interface );
      BOOST_REQUIRE( haz_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = isabelle.name;
      view.viewer = isabelle.name;
      view.author = candice.name;

      tx.operations.push_back( view );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot view posts

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = jayme.name;
      view.viewer = jayme.name;
      view.author = dan.name;

      tx.operations.push_back( view );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot view posts

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment view handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment share handling before becoming members" );

      share_operation share;

      share.signatory = george.name;
      share.sharer = george.name;
      share.author = alice.name;
      share.permlink = "lorem";
      share.reach = "follow";
      share.interface = INIT_ACCOUNT;
      share.shared = true;
      share.validate();
      
      tx.operations.push_back( share );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_share = db.get_comment_share( george.name, alice_comment.id );
      BOOST_REQUIRE( george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_share.comment == alice_comment.id );
      BOOST_REQUIRE( george_share.interface == share.interface );
      BOOST_REQUIRE( george_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = haz.name;
      share.sharer = haz.name;
      share.author = bob.name;

      tx.operations.push_back( share );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& haz_bob_share = db.get_comment_share( haz.name, bob_comment.id );

      BOOST_REQUIRE( haz_bob_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_bob_share.comment == bob_comment.id );
      BOOST_REQUIRE( haz_bob_share.interface == share.interface );
      BOOST_REQUIRE( haz_bob_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = isabelle.name;
      share.sharer = isabelle.name;
      share.author = candice.name;

      tx.operations.push_back( share );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot share posts

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = jayme.name;
      share.sharer = jayme.name;
      share.author = dan.name;

      tx.operations.push_back( share );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot share posts

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment share handling before becoming members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment creation handling by members" );

      community_invite_accept_operation accept;

      accept.signatory = george.name;
      accept.account = george.name;
      accept.community = "aliceopencommunity";
      accept.accepted = true;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = haz.name;
      accept.account = haz.name;
      accept.community = "bobpubliccommunity";

      tx.operations.push_back( accept );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = isabelle.name;
      accept.account = isabelle.name;
      accept.community = "candiceprivatecommunity";

      tx.operations.push_back( accept );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = jayme.name;
      accept.account = jayme.name;
      accept.community = "danexclusivecommunity";

      tx.operations.push_back( accept );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_ROOT_COMMENT_INTERVAL );

      comment.signatory = george.name;
      comment.author = george.name;
      comment.community = "aliceopencommunity";
      comment.permlink = "ipsum";

      tx.operations.push_back( comment );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& george_comment2 = db.get_comment( george.name, string( "ipsum" ) );

      BOOST_REQUIRE( george_comment2.author == george.name );
      BOOST_REQUIRE( george_comment2.community == "aliceopencommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = haz.name;
      comment.author = haz.name;
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& haz_comment = db.get_comment( haz.name, string( "ipsum" ) );

      BOOST_REQUIRE( haz_comment.author == haz.name );
      BOOST_REQUIRE( haz_comment.community == "bobpubliccommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = isabelle.name;
      comment.author = isabelle.name;
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& isabelle_comment = db.get_comment( isabelle.name, string( "ipsum" ) );

      BOOST_REQUIRE( isabelle_comment.author == isabelle.name );
      BOOST_REQUIRE( isabelle_comment.community == "candiceprivatecommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = jayme.name;
      comment.author = jayme.name;
      comment.community = "danexclusivecommunity";

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& jayme_comment = db.get_comment( jayme.name, string( "ipsum" ) );

      BOOST_REQUIRE( jayme_comment.author == jayme.name );
      BOOST_REQUIRE( jayme_comment.community == "danexclusivecommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment creation handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment vote handling by members" );

      vote.signatory = george.name;
      vote.voter = george.name;
      vote.author = george.name;
      vote.permlink = "ipsum";
      
      tx.operations.push_back( vote );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& george_vote = db.get_comment_vote( george.name, george_comment2.id );

      BOOST_REQUIRE( george_vote.voter == vote.voter );
      BOOST_REQUIRE( george_vote.comment == george_comment2.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = haz.name;
      vote.voter = haz.name;
      vote.author = haz.name;

      tx.operations.push_back( vote );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& haz_vote = db.get_comment_vote( haz.name, haz_comment.id );

      BOOST_REQUIRE( haz_vote.voter == vote.voter );
      BOOST_REQUIRE( haz_vote.comment == haz_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = isabelle.name;
      vote.voter = isabelle.name;
      vote.author = isabelle.name;

      tx.operations.push_back( vote );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& isabelle_vote = db.get_comment_vote( isabelle.name, isabelle_comment.id );

      BOOST_REQUIRE( isabelle_vote.voter == vote.voter );
      BOOST_REQUIRE( isabelle_vote.comment == isabelle_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = jayme.name;
      vote.voter = jayme.name;
      vote.author = jayme.name;

      tx.operations.push_back( vote );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_vote_object& jayme_vote = db.get_comment_vote( jayme.name, jayme_comment.id );

      BOOST_REQUIRE( jayme_vote.voter == vote.voter );
      BOOST_REQUIRE( jayme_vote.comment == jayme_comment.id );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment vote handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment view handling by members" );

      view.signatory = george.name;
      view.viewer = george.name;
      view.author = george.name;
      view.permlink = "lorem";

      tx.operations.push_back( view );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& george_george_view = db.get_comment_view( george.name, george_comment2.id );

      BOOST_REQUIRE( george_george_view.viewer == view.viewer );
      BOOST_REQUIRE( george_george_view.comment == george_comment2.id );
      BOOST_REQUIRE( george_george_view.supernode == view.supernode );
      BOOST_REQUIRE( george_george_view.interface == view.interface );
      BOOST_REQUIRE( george_george_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = haz.name;
      view.viewer = haz.name;
      view.author = haz.name;

      tx.operations.push_back( view );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& haz_haz_view = db.get_comment_view( haz.name, haz_comment.id );

      BOOST_REQUIRE( haz_haz_view.viewer == view.viewer );
      BOOST_REQUIRE( haz_haz_view.comment == haz_comment.id );
      BOOST_REQUIRE( haz_haz_view.supernode == view.supernode );
      BOOST_REQUIRE( haz_haz_view.interface == view.interface );
      BOOST_REQUIRE( haz_haz_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = isabelle.name;
      view.viewer = isabelle.name;
      view.author = isabelle.name;

      tx.operations.push_back( view );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& isabelle_isabelle_view = db.get_comment_view( isabelle.name, isabelle_comment.id );

      BOOST_REQUIRE( isabelle_isabelle_view.viewer == view.viewer );
      BOOST_REQUIRE( isabelle_isabelle_view.comment == isabelle_comment.id );
      BOOST_REQUIRE( isabelle_isabelle_view.supernode == view.supernode );
      BOOST_REQUIRE( isabelle_isabelle_view.interface == view.interface );
      BOOST_REQUIRE( isabelle_isabelle_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = jayme.name;
      view.viewer = jayme.name;
      view.author = jayme.name;

      tx.operations.push_back( view );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_view_object& jayme_jayme_view = db.get_comment_view( jayme.name, jayme_comment.id );

      BOOST_REQUIRE( jayme_jayme_view.viewer == view.viewer );
      BOOST_REQUIRE( jayme_jayme_view.comment == jayme_comment.id );
      BOOST_REQUIRE( jayme_jayme_view.supernode == view.supernode );
      BOOST_REQUIRE( jayme_jayme_view.interface == view.interface );
      BOOST_REQUIRE( jayme_jayme_view.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment view handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment share handling by members" );

      share.signatory = george.name;
      share.sharer = george.name;
      share.author = george.name;
      share.permlink = "ipsum";
      
      tx.operations.push_back( share );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& george_george_share = db.get_comment_share( george.name, george_comment.id );

      BOOST_REQUIRE( george_george_share.sharer == share.sharer );
      BOOST_REQUIRE( george_george_share.comment == george_comment.id );
      BOOST_REQUIRE( george_george_share.interface == share.interface );
      BOOST_REQUIRE( george_george_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = haz.name;
      share.sharer = haz.name;
      share.author = haz.name;

      tx.operations.push_back( share );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& haz_haz_share = db.get_comment_share( haz.name, haz_comment.id );

      BOOST_REQUIRE( haz_haz_share.sharer == share.sharer );
      BOOST_REQUIRE( haz_haz_share.comment == haz_comment.id );
      BOOST_REQUIRE( haz_haz_share.interface == share.interface );
      BOOST_REQUIRE( haz_haz_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = isabelle.name;
      share.sharer = isabelle.name;
      share.author = isabelle.name;

      tx.operations.push_back( share );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& isabelle_isabelle_share = db.get_comment_share( isabelle.name, isabelle_comment.id );

      BOOST_REQUIRE( isabelle_isabelle_share.sharer == share.sharer );
      BOOST_REQUIRE( isabelle_isabelle_share.comment == isabelle_comment.id );
      BOOST_REQUIRE( isabelle_isabelle_share.interface == share.interface );
      BOOST_REQUIRE( isabelle_isabelle_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      share.signatory = jayme.name;
      share.sharer = jayme.name;
      share.author = jayme.name;

      tx.operations.push_back( share );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_share_object& jayme_jayme_share = db.get_comment_share( jayme.name, jayme_comment.id );

      BOOST_REQUIRE( jayme_jayme_share.sharer == share.sharer );
      BOOST_REQUIRE( jayme_jayme_share.comment == jayme_comment.id );
      BOOST_REQUIRE( jayme_jayme_share.interface == share.interface );
      BOOST_REQUIRE( jayme_jayme_share.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment share handling by members" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: moderator tag by community moderator" );

      moderation_tag_operation tag;

      tag.signatory = elon.name;
      tag.moderator = elon.name;
      tag.author = george.name;
      tag.permlink = "ipsum";
      tag.tags.push_back( "nsfw" );
      tag.rating = 9;
      tag.details = "Post is NSFW";
      tag.interface = INIT_ACCOUNT;
      tag.filter = false;
      tag.applied = true;
      tag.validate();

      tx.operations.push_back( tag );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& tag_idx = db.get_index< moderation_tag_index >().indices().get< by_comment_moderator >();

      auto tag_itr = tag_idx.find( std::make_tuple( george_comment2.id, elon.name ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = haz.name;

      tx.operations.push_back( tag );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tag_itr = tag_idx.find( std::make_tuple( haz_comment.id, elon.name ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = isabelle.name;

      tx.operations.push_back( tag );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tag_itr = tag_idx.find( std::make_tuple( isabelle_comment.id, elon.name ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      tag.author = jayme.name;

      tx.operations.push_back( tag );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tag_itr = tag_idx.find( std::make_tuple( jayme_comment.id, elon.name ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: moderator tag by community moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator add moderator" );

      add_mod.signatory = elon.name;
      add_mod.account = elon.name;
      add_mod.community = "aliceopencommunity";
      add_mod.moderator = fred.name;
      add_mod.added = true;
      add_mod.validate();

      tx.operations.push_back( add_mod );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.is_moderator( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.community = "bobpubliccommunity";

      tx.operations.push_back( add_mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.is_moderator( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.is_moderator( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.community = "danexclusivecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.is_moderator( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator update community" );

      community_update_operation update;

      update.signatory = elon.name;
      update.account = elon.name;
      update.community = "aliceopencommunity";
      update.community_public_key = string( alice_public_posting_key );
      update.json = "{ \"valid\": true }";
      update.json_private = "{ \"valid\": true }";
      update.details = "updated details";
      update.url = "https://www.newurl.com";
      update.pinned_author = alice.name;
      update.pinned_permlink = "lorem";
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice_community.founder == update.account );
      BOOST_REQUIRE( alice_community.name == update.community );
      BOOST_REQUIRE( to_string( alice_community.details ) == update.details );
      BOOST_REQUIRE( to_string( alice_community.url ) == update.url );
      BOOST_REQUIRE( alice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = bob.name;
      update.community = "bobpubliccommunity";
      update.community_public_key = string( bob_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_community.founder == update.account );
      BOOST_REQUIRE( bob_community.name == update.community );
      BOOST_REQUIRE( to_string( bob_community.details ) == update.details );
      BOOST_REQUIRE( to_string( bob_community.url ) == update.url );
      BOOST_REQUIRE( bob_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = candice.name;
      update.community = "candiceprivatecommunity";
      update.community_public_key = string( candice_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( candice_community.founder == update.account );
      BOOST_REQUIRE( candice_community.name == update.community );
      BOOST_REQUIRE( to_string( candice_community.details ) == update.details );
      BOOST_REQUIRE( to_string( candice_community.url ) == update.url );
      BOOST_REQUIRE( candice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = dan.name;
      update.community = "danexclusivecommunity";
      update.community_public_key = string( dan_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( dan_community.founder == update.account );
      BOOST_REQUIRE( dan_community.name == update.community );
      BOOST_REQUIRE( to_string( dan_community.details ) == update.details );
      BOOST_REQUIRE( to_string( dan_community.url ) == update.url );
      BOOST_REQUIRE( dan_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator update community" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove moderator" );

      add_mod.signatory = alice.name;
      add_mod.account = alice.name;
      add_mod.community = "aliceopencommunity";
      add_mod.moderator = fred.name;
      add_mod.added = false;
      add_mod.validate();

      tx.operations.push_back( add_mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.is_member( fred.name ) );
      BOOST_REQUIRE( !community_member_a.is_moderator( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = bob.name;
      add_mod.account = bob.name;
      add_mod.community = "bobpubliccommunity";

      tx.operations.push_back( add_mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.is_member( fred.name ) );
      BOOST_REQUIRE( !community_member_b.is_moderator( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = candice.name;
      add_mod.account = candice.name;
      add_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.is_member( fred.name ) );
      BOOST_REQUIRE( !community_member_c.is_moderator( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      add_mod.signatory = dan.name;
      add_mod.account = dan.name;
      add_mod.community = "danexclusivecommunity";

      tx.operations.push_back( add_mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.is_member( fred.name ) );
      BOOST_REQUIRE( !community_member_d.is_moderator( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove member" );

      community_remove_member_operation remove;

      remove.signatory = alice.name;
      remove.account = alice.name;
      remove.community = "aliceopencommunity";
      remove.member = fred.name;
      remove.validate();

      tx.operations.push_back( remove );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !community_member_a.is_member( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = bob.name;
      remove.account = bob.name;
      remove.community = "bobpubliccommunity";

      tx.operations.push_back( remove );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !community_member_b.is_member( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = candice.name;
      remove.account = candice.name;
      remove.community = "candiceprivatecommunity";

      tx.operations.push_back( remove );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !community_member_c.is_member( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = dan.name;
      remove.account = dan.name;
      remove.community = "danexclusivecommunity";

      tx.operations.push_back( remove );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !community_member_d.is_member( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder remove moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: community subscription" );

      community_subscribe_operation subscribe;

      subscribe.signatory = elon.name;
      subscribe.account = elon.name;
      subscribe.community = "aliceopencommunity";
      subscribe.interface = INIT_ACCOUNT;
      subscribe.added = true;
      subscribe.subscribed = true;
      subscribe.validate();

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.is_subscriber( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "bobpubliccommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.is_subscriber( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "candiceprivatecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.is_subscriber( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "danexclusivecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.is_subscriber( elon.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: community subscription" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder blacklist account" );

      community_blacklist_operation blacklist;

      blacklist.signatory = alice.name;
      blacklist.account = alice.name;
      blacklist.member = fred.name;
      blacklist.community = "aliceopencommunity";
      blacklist.validate();

      tx.operations.push_back( blacklist );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.is_blacklisted( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = bob.name;
      blacklist.account = bob.name;
      blacklist.community = "bobpubliccommunity";

      tx.operations.push_back( blacklist );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.is_blacklisted( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = candice.name;
      blacklist.account = candice.name;
      blacklist.community = "candiceprivatecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.is_blacklisted( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = dan.name;
      blacklist.account = dan.name;
      blacklist.community = "danexclusivecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.is_blacklisted( fred.name ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder blacklist account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: transfer community ownership" );

      community_transfer_ownership_operation transfer;

      transfer.signatory = alice.name;
      transfer.account = alice.name;
      transfer.community = "aliceopencommunity";
      transfer.new_founder = elon.name;
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_a.founder == elon.name );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = bob.name;
      transfer.account = bob.name;
      transfer.community = "bobpubliccommunity";

      tx.operations.push_back( transfer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_b.founder == elon.name );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = candice.name;
      transfer.account = candice.name;
      transfer.community = "candiceprivatecommunity";

      tx.operations.push_back( transfer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_c.founder == elon.name );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = dan.name;
      transfer.account = dan.name;
      transfer.community = "danexclusivecommunity";

      tx.operations.push_back( transfer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( community_member_d.founder == elon.name );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: transfer community ownership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Create community event" );

      community_event_operation event;

      event.signatory = alice.name;
      event.account = alice.name;
      event.community = "aliceopencommunity";
      event.event_name = "Alice's Party";
      event.location = "Alice's House";
      event.latitude = 37.8136;
      event.longitude = 144.9631;
      event.details = "Open house - BYO drinks";
      event.url = "https://www.staggeringbeauty.com";
      event.json = "{ \"valid\": true }";
      event.event_start_time = now() + fc::days(1);
      event.event_end_time = now() + fc::days(1) + fc::hours(8);
      event.validate();

      tx.operations.push_back( event );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_event_object& alice_event = db.get_community_event( "aliceopencommunity" );

      BOOST_REQUIRE( event.account == alice_event.account );
      BOOST_REQUIRE( event.community == alice_event.community );
      BOOST_REQUIRE( event.event_name == to_string( alice_event.event_name ) );
      BOOST_REQUIRE( event.location == to_string( alice_event.location ) );
      BOOST_REQUIRE( event.latitude == alice_event.latitude );
      BOOST_REQUIRE( event.longitude == alice_event.longitude );
      BOOST_REQUIRE( event.details == to_string( alice_event.details ) );
      BOOST_REQUIRE( event.url == to_string( alice_event.url ) );
      BOOST_REQUIRE( event.json == to_string( alice_event.json ) );
      BOOST_REQUIRE( event.event_start_time == alice_event.event_start_time );
      BOOST_REQUIRE( event.event_end_time == alice_event.event_end_time );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Create community event" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Attend community event" );

      community_event_attend_operation attend;

      attend.signatory = george.name;
      attend.account = george.name;
      attend.community = "aliceopencommunity";
      attend.interested = true;
      attend.attending = true;
      attend.not_attending = true;
      attend.validate();

      tx.operations.push_back( attend );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice_event.is_interested( george.name ) );
      BOOST_REQUIRE( alice_event.is_attending( george.name ) );
      BOOST_REQUIRE( !alice_event.is_not_attending( george.name ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Attend community event" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY MANAGEMENT OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()