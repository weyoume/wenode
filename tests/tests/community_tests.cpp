//#ifdef IS_TEST_NET

#include <boost/test/unit_test.hpp>
#include <node/chain/node_objects.hpp>
#include <node/protocol/exceptions.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>

//#include <node/chain/hardfork.hpp>

#include <node/chain/util/reward.hpp>
#include <node/producer/producer_objects.hpp>
#include <fc/crypto/digest.hpp>
#include <tests/common/database_fixture.hpp>

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

      const auto& community_idx = db.get_index< community_index >().indices().get< by_name >();

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.community_privacy = OPEN_PUBLIC_COMMUNITY;
      create.community_public_key = string( alice_public_posting_key );
      create.json = "{\"json\":\"valid\"}";
      create.json_private = "{\"json\":\"valid\"}";
      create.details = "details";
      create.url = "www.url.com";
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "aliceopencommunity" );
      BOOST_REQUIRE( community_itr->founder == create.founder );
      BOOST_REQUIRE( community_itr->name == create.name );
      BOOST_REQUIRE( community_itr->community_privacy == create.community_privacy );
      BOOST_REQUIRE( community_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "bob";
      create.founder = "bob";
      create.name = "bobpubliccommunity";
      create.community_privacy = EXCLUSIVE_PUBLIC_COMMUNITY;
      create.community_public_key = string( bob_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "bobpubliccommunity" );
      BOOST_REQUIRE( community_itr->founder == create.founder );
      BOOST_REQUIRE( community_itr->name == create.name );
      BOOST_REQUIRE( community_itr->community_privacy == create.community_privacy );
      BOOST_REQUIRE( community_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.community_privacy = OPEN_PRIVATE_COMMUNITY;
      create.community_public_key = string( candice_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_itr->founder == create.founder );
      BOOST_REQUIRE( community_itr->name == create.name );
      BOOST_REQUIRE( community_itr->community_privacy == create.community_privacy );
      BOOST_REQUIRE( community_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.community_privacy = EXCLUSIVE_PRIVATE_COMMUNITY;
      create.community_public_key = string( dan_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "danexclusivecommunity" );
      BOOST_REQUIRE( community_itr->founder == create.founder );
      BOOST_REQUIRE( community_itr->name == create.name );
      BOOST_REQUIRE( community_itr->community_privacy == create.community_privacy );
      BOOST_REQUIRE( community_itr->created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful community creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_CREATE_INTERVAL - BLOCK_INTERVAL );

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "mysecondcommunity";
      create.community_public_key = string( alice_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( alice_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success after MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      tx.operations.push_back( create );
      tx.sign( alice_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when community created before MIN_COMMUNITY_CREATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community name already exists" );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "elon";
      create.founder = "elon";
      create.name = "aliceopencommunity";

      tx.operations.push_back( create );
      tx.sign( elon_private_posting_key, db.get_chain_id() );

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

      const auto& community_idx = db.get_index< community_index >().indices().get< by_name >();

      signed_transaction tx;

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.community_privacy = OPEN_PUBLIC_COMMUNITY;
      create.community_public_key = string( alice_public_posting_key );
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
      create.name = "bobpubliccommunity";
      create.community_privacy = EXCLUSIVE_PUBLIC_COMMUNITY;
      create.community_public_key = string( bob_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "candice";
      create.founder = "candice";
      create.name = "candiceprivatecommunity";
      create.community_privacy = OPEN_PRIVATE_COMMUNITY;
      create.community_public_key = string( candice_public_posting_key );
      
      create.validate();

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "dan";
      create.founder = "dan";
      create.name = "danexclusivecommunity";
      create.community_privacy = EXCLUSIVE_PRIVATE_COMMUNITY;
      create.community_public_key = string( dan_public_posting_key );
      
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
      comment.community = "aliceopencommunity";
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
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "candice";
      comment.author = "candice";
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "dan";
      comment.author = "dan";
      comment.community = "danexclusivecommunity";

      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + MIN_COMMUNITY_UPDATE_INTERVAL );

      community_update_operation update;

      update.signatory = "alice";
      update.account = "alice";
      update.community = "aliceopencommunity";
      update.community_public_key = string( alice_public_posting_key );
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

      auto community_itr = community_idx.find( "aliceopencommunity" );
      BOOST_REQUIRE( community_itr->founder == update.account );
      BOOST_REQUIRE( community_itr->name == update.community );
      BOOST_REQUIRE( community_itr->details == update.details );
      BOOST_REQUIRE( community_itr->url == update.url );
      BOOST_REQUIRE( community_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "bob";
      update.account = "bob";
      update.pinned_author = "bob";
      update.community = "bobpubliccommunity";
      update.community_public_key = string( bob_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "bobpubliccommunity" );
      BOOST_REQUIRE( community_itr->founder == update.account );
      BOOST_REQUIRE( community_itr->name == update.community );
      BOOST_REQUIRE( community_itr->details == update.details );
      BOOST_REQUIRE( community_itr->url == update.url );
      BOOST_REQUIRE( community_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "candice";
      update.account = "candice";
      update.pinned_author = "candice";
      update.community = "candiceprivatecommunity";
      update.community_public_key = string( candice_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_itr->founder == update.account );
      BOOST_REQUIRE( community_itr->name == update.community );
      BOOST_REQUIRE( community_itr->details == update.details );
      BOOST_REQUIRE( community_itr->url == update.url );
      BOOST_REQUIRE( community_itr->created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.signatory = "dan";
      update.account = "dan";
      update.pinned_author = "dan";
      update.community = "danexclusivecommunity";
      update.community_public_key = string( dan_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "danexclusivecommunity" );
      BOOST_REQUIRE( community_itr->founder == update.account );
      BOOST_REQUIRE( community_itr->name == update.community );
      BOOST_REQUIRE( community_itr->details == update.details );
      BOOST_REQUIRE( community_itr->url == update.url );
      BOOST_REQUIRE( community_itr->created == now() );

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

      generate_block();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      auto community_itr = community_idx.find( "danexclusivecommunity" );
      BOOST_REQUIRE( community_itr->founder == update.account );
      BOOST_REQUIRE( community_itr->name == update.community );
      BOOST_REQUIRE( community_itr->details == update.details );
      BOOST_REQUIRE( community_itr->url == update.url );
      BOOST_REQUIRE( community_itr->created == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success after MIN_COMMUNITY_UPDATE_INTERVAL has passed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when community name already exists" );

      tx.operations.clear();
      tx.signatures.clear();

      create.signatory = "elon";
      create.founder = "elon";
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

      community_create_operation create;

      create.signatory = "alice";
      create.founder = "alice";
      create.name = "aliceopencommunity";
      create.community_privacy = OPEN_PUBLIC_COMMUNITY;
      create.community_public_key = string( alice_public_posting_key );
      create.json = "{\"json\":\"valid\"}";
      create.json_private = "{\"json\":\"valid\"}";
      create.details = "details";
      create.url = "www.url.com";
      create.validate();

      tx.operations.push_back( create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& alice_community = db.get_community( "aliceopencommunity" );
      const community_member_object& alice_community_member = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( alice_community.founder == create.founder );
      BOOST_REQUIRE( alice_community.community_privacy == create.community_privacy );
      BOOST_REQUIRE( alice_community.community_public_key == create.community_public_key );

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
      create.community_privacy = EXCLUSIVE_PUBLIC_COMMUNITY;
      create.community_public_key = string( bob_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( "bobpubliccommunity" );
      const community_member_object& bob_community_member = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( bob_community.founder == create.founder );
      BOOST_REQUIRE( bob_community.community_privacy == create.community_privacy );
      BOOST_REQUIRE( bob_community.community_public_key == create.community_public_key );

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
      create.community_privacy = OPEN_PRIVATE_COMMUNITY;
      create.community_public_key = string( candice_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( "candiceprivatecommunity" );
      const community_member_object& candice_community_member = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( candice_community.founder == create.founder );
      BOOST_REQUIRE( candice_community.community_privacy == create.community_privacy );
      BOOST_REQUIRE( candice_community.community_public_key == create.community_public_key );

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
      create.community_privacy = EXCLUSIVE_PRIVATE_COMMUNITY;
      create.community_public_key = string( dan_public_posting_key );

      tx.operations.push_back( create );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( "danexclusivecommunity" );
      const community_member_object& dan_community_member = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( dan_community.founder == create.founder );
      BOOST_REQUIRE( dan_community.community_privacy == create.community_privacy );
      BOOST_REQUIRE( dan_community.community_public_key == create.community_public_key );

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

      invite.signatory = "alice";
      invite.account = "alice";
      invite.member = "elon";
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;
      invite.validate();

      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& invite_idx = db.get_index< community_join_invite_index >().indices().get< by_member_community >();

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "aliceopencommunity" ) );
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
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "bobpubliccommunity" ) );
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
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "candiceprivatecommunity" ) );
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
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "danexclusivecommunity" ) );
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
      invite.encrypted_community_key = string( dan_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto invite_itr = invite_idx.find( boost::make_tuple( "fred", "danexclusivecommunity" ) );
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

      invite.signatory = "elon";
      invite.account = "elon";
      invite.member = "george";
      invite.community = "aliceopencommunity";
      invite.message = "Hello";
      invite.encrypted_community_key = string( alice_public_posting_key );
      invite.invited = true;

      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "haz";
      invite.community = "bobpubliccommunity";
      invite.encrypted_community_key = string( bob_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "isabelle";
      invite.community = "candiceprivatecommunity";
      invite.encrypted_community_key = string( candice_public_posting_key );
   
      tx.operations.push_back( invite );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-member cannot send invitations

      tx.operations.clear();
      tx.signatures.clear();

      invite.member = "jayme";
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

      request.signatory = "fred";
      request.account = "fred";
      request.community = "aliceopencommunity";
      request.message = "Hello";
      request.requested = true;
      request.validate();

      tx.operations.push_back( request );
      tx.sign( fred_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& request_idx = db.get_index< community_join_request_index >().indices().get< by_account_community >();

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "aliceopencommunity" ) );
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

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "bobpubliccommunity" ) );
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

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "candiceprivatecommunity" ) );
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

      auto request_itr = request_idx.find( boost::make_tuple( "fred", "danexclusivecommunity" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "aliceopencommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "bobpubliccommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "bobpubliccommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "candiceprivatecommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "candiceprivatecommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.community = "danexclusivecommunity";

      tx.operations.push_back( invite_accept );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.is_member( "elon" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "elon", "danexclusivecommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      invite_accept.signatory = "fred";
      invite_accept.account = "fred";

      tx.operations.push_back( invite_accept );
      tx.sign( fred_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.is_member( "fred" ) );
      auto invite_itr = invite_idx.find( boost::make_tuple( "fred", "danexclusivecommunity" ) );
      BOOST_REQUIRE( invite_itr == invite_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_member( "fred" ) );
      auto request_itr = request_idx.find( boost::make_tuple( "fred", "aliceopencommunity" ) );
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

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.is_member( "fred" ) );
      auto request_itr = request_idx.find( boost::make_tuple( "fred", "bobpubliccommunity" ) );
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

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.is_member( "fred" ) );
      auto request_itr = request_idx.find( boost::make_tuple( "fred", "candiceprivatecommunity" ) );
      BOOST_REQUIRE( request_itr == request_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      auto invite_itr = invite_idx.find( boost::make_tuple( "george", "aliceopencommunity" ) );
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

      auto invite_itr = invite_idx.find( boost::make_tuple( "haz", "bobpubliccommunity" ) );
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

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: handling member sending invites" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder add moderator" );

      community_add_mod_operation mod;

      mod.signatory = "alice";
      mod.account = "alice";
      mod.community = "aliceopencommunity";
      mod.moderator = "elon";
      mod.added = true;
      mod.validate();

      tx.operations.push_back( mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "bob";
      mod.account = "bob";
      mod.community = "bobpubliccommunity";

      tx.operations.push_back( mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "candice";
      mod.account = "candice";
      mod.community = "candiceprivatecommunity";

      tx.operations.push_back( mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "dan";
      mod.account = "dan";
      mod.community = "danexclusivecommunity";

      tx.operations.push_back( mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      flat_map< account_name_type, share_type > m = community_member_a.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "bob";
      vote_mod.account = "bob";
      vote_mod.community = "bobpubliccommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      m = community_member_b.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "candice";
      vote_mod.account = "candice";
      vote_mod.community = "candiceprivatecommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      m = community_member_c.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_mod.signatory = "dan";
      vote_mod.account = "dan";
      vote_mod.community = "danexclusivecommunity";

      tx.operations.push_back( vote_mod );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      m = community_member_d.mod_weight;
      BOOST_REQUIRE( m[ elon.name ] > 0 );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      auto invite_itr = invite_idx.find( boost::make_tuple( "isabelle", "candiceprivatecommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
      BOOST_REQUIRE( invite_itr->expiration == now() + CONNECTION_REQUEST_DURATION );

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

      validate_database();

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

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "bob";
      admin.account = "bob";
      admin.community = "bobpubliccommunity";

      tx.operations.push_back( admin );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "candice";
      admin.account = "candice";
      admin.community = "candiceprivatecommunity";

      tx.operations.push_back( admin );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      admin.signatory = "dan";
      admin.account = "dan";
      admin.community = "danexclusivecommunity";

      tx.operations.push_back( admin );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.is_administrator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      auto invite_itr = invite_idx.find( boost::make_tuple( "jayme", "danexclusivecommunity" ) );
      BOOST_REQUIRE( invite_itr != invite_idx.end() );
      BOOST_REQUIRE( invite_itr->account == invite.account );
      BOOST_REQUIRE( invite_itr->member == invite.member );
      BOOST_REQUIRE( invite_itr->community == invite.community );
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
      comment.community = "aliceopencommunity";
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
      options.reach = TAG_FEED;
      options.rating = GENERAL;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.sign( george_private_posting_key, db.get_chain_id() );       // Non-members can create posts in open community
      db.push_transaction( tx, 0 );

      const comment_object& com = db.get_comment( "george", string( "lorem" ) );
      BOOST_REQUIRE( com.author == "george" );
      BOOST_REQUIRE( com.community == "aliceopencommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "haz";
      comment.author = "haz";
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "isabelle";
      comment.author = "isabelle";
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "jayme";
      comment.author = "jayme";
      comment.community = "danexclusivecommunity";

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Non-members cannot create posts

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "alice";
      comment.author = "alice";
      comment.community = "aliceopencommunity";

      tx.operations.push_back( comment );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "bob";
      comment.author = "bob";
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( "bob", string( "lorem" ) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "candice";
      comment.author = "candice";
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( "candice", string( "lorem" ) );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "dan";
      comment.author = "dan";
      comment.community = "danexclusivecommunity";

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

      generate_blocks( now() + MIN_ROOT_COMMENT_INTERVAL );

      comment_operation comment;

      comment.signatory = "george";
      comment.author = "george";
      comment.community = "aliceopencommunity";
      comment.permlink = "ipsum";

      tx.operations.push_back( comment );
      tx.sign( george_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& george_comment = db.get_comment( "george", string( "ipsum" ) );
      BOOST_REQUIRE( george_comment.author == "george" );
      BOOST_REQUIRE( george_comment.community == "aliceopencommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "haz";
      comment.author = "haz";
      comment.community = "bobpubliccommunity";

      tx.operations.push_back( comment );
      tx.sign( haz_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& haz_comment = db.get_comment( "haz", string( "ipsum" ) );
      BOOST_REQUIRE( haz_comment.author == "haz" );
      BOOST_REQUIRE( haz_comment.community == "bobpubliccommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "isabelle";
      comment.author = "isabelle";
      comment.community = "candiceprivatecommunity";

      tx.operations.push_back( comment );
      tx.sign( isabelle_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& isabelle_comment = db.get_comment( "isabelle", string( "ipsum" ) );
      BOOST_REQUIRE( isabelle_comment.author == "isabelle" );
      BOOST_REQUIRE( isabelle_comment.community == "candiceprivatecommunity" );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = "jayme";
      comment.author = "jayme";
      comment.community = "danexclusivecommunity";

      tx.operations.push_back( comment );
      tx.sign( jayme_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& jayme_comment = db.get_comment( "jayme", string( "ipsum" ) );
      BOOST_REQUIRE( jayme_comment.author == "jayme" );
      BOOST_REQUIRE( jayme_comment.community == "danexclusivecommunity" );

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

      BOOST_TEST_MESSAGE( "│   ├── Testing: moderator tag by community moderator" );

      moderation_tag_operation tag;

      tag.signatory = "elon";
      tag.moderator = "elon";
      tag.author = "george";
      tag.permlink = "ipsum";
      tag.tags.push_back( "nsfw" );
      tag.rating = ADULT;
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

      BOOST_TEST_MESSAGE( "│   ├── Passed: moderator tag by community moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator add moderator" );

      community_add_mod_operation mod;

      mod.signatory = "elon";
      mod.account = "elon";
      mod.community = "aliceopencommunity";
      mod.moderator = "fred";
      mod.added = true;
      mod.validate();

      tx.operations.push_back( mod );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.community = "bobpubliccommunity";

      tx.operations.push_back( mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.community = "candiceprivatecommunity";

      tx.operations.push_back( mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.community = "danexclusivecommunity";

      tx.operations.push_back( mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.is_moderator( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator add moderator" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: administrator update community" );

      community_update_operation update;

      update.signatory = "elon";
      update.account = "elon";
      update.community = "aliceopencommunity";
      update.community_public_key = string( alice_public_posting_key );
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

      const community_object& alice_community = db.get_community( "aliceopencommunity" );
      BOOST_REQUIRE( alice_community.founder == update.account );
      BOOST_REQUIRE( alice_community.name == update.community );
      BOOST_REQUIRE( alice_community.details == update.details );
      BOOST_REQUIRE( alice_community.url == update.url );
      BOOST_REQUIRE( alice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = "bob";
      update.community = "bobpubliccommunity";
      update.community_public_key = string( bob_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( elon_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& bob_community = db.get_community( "bobpubliccommunity" );
      BOOST_REQUIRE( bob_community.founder == update.account );
      BOOST_REQUIRE( bob_community.name == update.community );
      BOOST_REQUIRE( bob_community.details == update.details );
      BOOST_REQUIRE( bob_community.url == update.url );
      BOOST_REQUIRE( bob_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = "candice";
      update.community = "candiceprivatecommunity";
      update.community_public_key = string( candice_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& candice_community = db.get_community( "candiceprivatecommunity" );
      BOOST_REQUIRE( candice_community.founder == update.account );
      BOOST_REQUIRE( candice_community.name == update.community );
      BOOST_REQUIRE( candice_community.details == update.details );
      BOOST_REQUIRE( candice_community.url == update.url );
      BOOST_REQUIRE( candice_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      update.pinned_author = "dan";
      update.community = "danexclusivecommunity";
      update.community_public_key = string( dan_public_posting_key );
      update.validate();

      tx.operations.push_back( update );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_object& dan_community = db.get_community( "danexclusivecommunity" );
      BOOST_REQUIRE( dan_community.founder == update.account );
      BOOST_REQUIRE( dan_community.name == update.community );
      BOOST_REQUIRE( dan_community.details == update.details );
      BOOST_REQUIRE( dan_community.url == update.url );
      BOOST_REQUIRE( dan_community.created == now() );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: administrator update community" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: founder remove moderator" );

      community_add_mod_operation mod;

      mod.signatory = "alice";
      mod.account = "alice";
      mod.community = "aliceopencommunity";
      mod.moderator = "fred";
      mod.added = false;
      mod.validate();

      tx.operations.push_back( mod );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !community_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "bob";
      mod.account = "bob";
      mod.community = "bobpubliccommunity";

      tx.operations.push_back( mod );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !community_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "candice";
      mod.account = "candice";
      mod.community = "candiceprivatecommunity";

      tx.operations.push_back( mod );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !community_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      mod.signatory = "dan";
      mod.account = "dan";
      mod.community = "danexclusivecommunity";

      tx.operations.push_back( mod );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_a.is_member( "fred" ) );
      BOOST_REQUIRE( !community_member_a.is_moderator( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( !community_member_a.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "bob";
      remove.account = "bob";
      remove.community = "bobpubliccommunity";

      tx.operations.push_back( remove );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( !community_member_b.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "candice";
      remove.account = "candice";
      remove.community = "candiceprivatecommunity";

      tx.operations.push_back( remove );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( !community_member_c.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      remove.signatory = "dan";
      remove.account = "dan";
      remove.community = "danexclusivecommunity";

      tx.operations.push_back( remove );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( !community_member_d.is_member( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "bobpubliccommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "candiceprivatecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      subscribe.community = "danexclusivecommunity";

      tx.operations.push_back( subscribe );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.is_subscriber( "elon" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

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
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "bob";
      blacklist.account = "bob";
      blacklist.community = "bobpubliccommunity";

      tx.operations.push_back( blacklist );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "candice";
      blacklist.account = "candice";
      blacklist.community = "candiceprivatecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      blacklist.signatory = "dan";
      blacklist.account = "dan";
      blacklist.community = "danexclusivecommunity";

      tx.operations.push_back( blacklist );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.is_blacklisted( "fred" ) );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: founder blacklist account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: transfer community ownership" );

      community_transfer_ownership_operation transfer;

      transfer.signatory = "alice";
      transfer.account = "alice";
      transfer.community = "aliceopencommunity";
      transfer.new_founder = "elon";
      transfer.validate();

      tx.operations.push_back( transfer );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_a = db.get_community_member( "aliceopencommunity" );
      BOOST_REQUIRE( community_member_a.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "bob";
      transfer.account = "bob";
      transfer.community = "bobpubliccommunity";

      tx.operations.push_back( transfer );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_b = db.get_community_member( "bobpubliccommunity" );
      BOOST_REQUIRE( community_member_b.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "candice";
      transfer.account = "candice";
      transfer.community = "candiceprivatecommunity";

      tx.operations.push_back( transfer );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_c = db.get_community_member( "candiceprivatecommunity" );
      BOOST_REQUIRE( community_member_c.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      transfer.signatory = "dan";
      transfer.account = "dan";
      transfer.community = "danexclusivecommunity";

      tx.operations.push_back( transfer );
      tx.sign( dan_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const community_member_object& community_member_d = db.get_community_member( "danexclusivecommunity" );
      BOOST_REQUIRE( community_member_d.founder == "elon" );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: transfer community ownership" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMUNITY MANAGEMENT OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()