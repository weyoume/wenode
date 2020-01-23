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

BOOST_FIXTURE_TEST_SUITE( board_operation_tests, clean_database_fixture );



   //=====================//
   // === Board Tests === //
   //=====================//


   
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


BOOST_AUTO_TEST_SUITE_END()