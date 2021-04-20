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

BOOST_FIXTURE_TEST_SUITE( ad_operation_tests, clean_database_fixture );


   //===========================//
   // === Advertising Tests === //
   //===========================//



BOOST_AUTO_TEST_CASE( ad_operation_sequence_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: AD OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful ad creative creation" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "elon", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      comment_operation comment;

      comment.editor = "alice";
      comment.author = "alice";
      comment.permlink = "adcreativepermlink";
      comment.title = "My Creative post";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = INIT_PUBLIC_COMMUNITY;
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "adcreativepermlink";
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
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      ad_creative_operation creative;

      creative.account = "alice";
      creative.author = "alice";
      creative.format_type = "standard";
      creative.creative_id = "8638f626-7c6e-4440-9a67-43ab48939870";
      creative.objective = "adcreativepermlink";
      creative.creative = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      creative.json = "{ \"valid\": true }";
      creative.active = true;
      creative.validate();

      tx.operations.push_back( creative );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_creative_object& alice_creative = db.get_ad_creative( creative.account, creative.creative_id );

      BOOST_REQUIRE( to_string( alice_creative.objective ) == creative.objective );
      BOOST_REQUIRE( to_string( alice_creative.creative ) == creative.creative );
      BOOST_REQUIRE( alice_creative.format_type == ad_format_type::STANDARD_FORMAT );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful ad creative creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful ad campaign creation" );

      ad_campaign_operation campaign;

      campaign.account = "alice";
      campaign.campaign_id = "da89d680-e9c4-4ae0-95e5-1f47bd1526a0";
      campaign.budget = asset( 100*BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      campaign.begin = now();
      campaign.end = ( now() + fc::days(7) );
      campaign.json = "{ \"valid\": true }";
      campaign.interface = INIT_ACCOUNT;
      campaign.active = true;
      campaign.validate();

      tx.operations.push_back( campaign );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_campaign_object& alice_campaign = db.get_ad_campaign( campaign.account, campaign.campaign_id );

      BOOST_REQUIRE( alice_campaign.budget == campaign.budget );
      BOOST_REQUIRE( alice_campaign.begin == campaign.begin );
      BOOST_REQUIRE( alice_campaign.end == campaign.end );

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad campaign creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad audience creation" );

      ad_audience_operation audience;

      audience.account = "bob";
      audience.audience_id = "0ffe6be9-dcf8-436e-9296-49c83e3d0786";
      audience.json = "{ \"valid\": true }";
      audience.audience.push_back( "candice" );
      audience.audience.push_back( "dan" );
      audience.audience.push_back( "elon" );
      audience.active = true;
      audience.validate();

      tx.operations.push_back( audience );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_audience_object& bob_audience = db.get_ad_audience( audience.account, audience.audience_id );

      for( auto a : audience.audience )
      {
         BOOST_REQUIRE( bob_audience.is_audience( a ) );
      }

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad audience creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creating inventory without interface" );

      ad_inventory_operation inventory;

      inventory.provider = "bob";
      inventory.inventory_id = "19ebee83-fc57-404b-a85e-aa8e7f6bbb66";
      inventory.audience_id = "0ffe6be9-dcf8-436e-9296-49c83e3d0786";
      inventory.metric = "view";
      inventory.json = "{ \"valid\": true }";
      inventory.min_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      inventory.inventory = 100;
      inventory.active = true;
      inventory.validate();

      tx.operations.push_back( inventory );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );     // Inventory requires an interface

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when creating inventory without interface" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad inventory creation" );

      account_membership_operation membership;
      
      membership.account = "bob";
      membership.membership_type = "standard";
      membership.months = 1;
      membership.interface = INIT_ACCOUNT;
      membership.validate();

      tx.operations.push_back( membership );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      interface_update_operation interface;
      
      interface.account = "bob";
      interface.details = "details";
      interface.url = "https://www.url.com";
      interface.json = "{ \"valid\": true }";
      interface.validate();

      tx.operations.push_back( interface );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      inventory.provider = "bob";
      inventory.inventory_id = "19ebee83-fc57-404b-a85e-aa8e7f6bbb66";
      inventory.audience_id = "0ffe6be9-dcf8-436e-9296-49c83e3d0786";
      inventory.metric = "view";
      inventory.json = "{ \"valid\": true }";
      inventory.min_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      inventory.inventory = 100;
      inventory.active = true;
      inventory.validate();

      tx.operations.push_back( inventory );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_inventory_object& bob_inventory = db.get_ad_inventory( inventory.provider, inventory.inventory_id );

      BOOST_REQUIRE( bob_inventory.min_price == inventory.min_price );
      BOOST_REQUIRE( bob_inventory.metric == ad_metric_type::VIEW_METRIC );
      BOOST_REQUIRE( bob_inventory.inventory == inventory.inventory );
      BOOST_REQUIRE( bob_inventory.remaining == inventory.inventory );

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad inventory creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful ad bid creation" );

      generate_blocks( GENESIS_PRODUCER_AMOUNT );

      ad_bid_operation bid;

      bid.bidder = "alice";
      bid.bid_id = "28bdc74a-097a-40d4-bf49-cc95af3eeec0";
      bid.account = "alice";
      bid.campaign_id = "da89d680-e9c4-4ae0-95e5-1f47bd1526a0";
      bid.author = "alice";
      bid.creative_id = "8638f626-7c6e-4440-9a67-43ab48939870";
      bid.provider = "bob";
      bid.inventory_id = "19ebee83-fc57-404b-a85e-aa8e7f6bbb66";
      bid.bid_price = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      bid.requested = 99;
      bid.audience_id = "07c968e5-fc0f-478f-8a9c-6fe3a93eace1";
      bid.json = "{ \"valid\": true }";
      bid.expiration = ( now() + fc::days(30) );
      bid.active = true;
      bid.validate();

      tx.operations.push_back( bid );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const ad_bid_object& alice_bid = db.get_ad_bid( bid.bidder, bid.bid_id );

      BOOST_REQUIRE( alice_bid.bid_price == bid.bid_price );
      BOOST_REQUIRE( to_string( alice_bid.campaign_id ) == bid.campaign_id );
      BOOST_REQUIRE( to_string( alice_bid.creative_id ) == bid.creative_id );
      BOOST_REQUIRE( to_string( alice_bid.inventory_id ) == bid.inventory_id );
      BOOST_REQUIRE( to_string( alice_bid.audience_id ) == bid.audience_id );
      BOOST_REQUIRE( alice_bid.account == bid.account );
      BOOST_REQUIRE( alice_bid.requested == bid.requested );
      BOOST_REQUIRE( alice_bid.remaining == bid.requested );
      BOOST_REQUIRE( alice_bid.expiration == bid.expiration );

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful ad bid creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: sucessful partial ad delivery" );

      account_create_operation create;

      create.registrar = "alice";
      create.referrer = "alice";
      create.new_account_name = "newuser";
      create.owner_auth = authority( 1, alice_public_owner_key, 1 );
      create.active_auth = authority( 2, alice_public_active_key, 2 );
      create.posting_auth = authority( 1, alice_public_posting_key, 1 );
      create.secure_public_key = string( alice_public_posting_key );
      create.connection_public_key = string( alice_public_posting_key );
      create.friend_public_key = string( alice_public_posting_key );
      create.companion_public_key = string( alice_public_posting_key );
      create.fee = asset( 8 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
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

      generate_blocks( now() + fc::minutes(1) );

      tx.operations.push_back( audience );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );    // Update audience to include new accounts

      tx.operations.clear();
      tx.signatures.clear();

      bid.requested = 100;

      tx.operations.push_back( bid );    // Update bid to include new audience accounts
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
   
      comment_view_operation view;

      view.viewer = "newuser";
      view.author = "alice";
      view.permlink = "adcreativepermlink";
      view.supernode = INIT_ACCOUNT;
      view.interface = "bob";
      view.validate();
      
      for( auto i = 0; i < 50; i++ )
      {
         view.viewer = "newuser"+fc::to_string( i );

         tx.operations.push_back( view );
         tx.sign( alice_private_posting_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         generate_block();
      }

      const ad_bid_object& new_alice_bid = db.get_ad_bid( bid.bidder, bid.bid_id );
      
      BOOST_REQUIRE( new_alice_bid.bid_price == bid.bid_price );
      BOOST_REQUIRE( to_string( new_alice_bid.campaign_id ) == bid.campaign_id );
      BOOST_REQUIRE( to_string( new_alice_bid.creative_id ) == bid.creative_id );
      BOOST_REQUIRE( to_string( new_alice_bid.inventory_id ) == bid.inventory_id );
      BOOST_REQUIRE( to_string( new_alice_bid.audience_id ) == bid.audience_id );
      BOOST_REQUIRE( new_alice_bid.account == bid.account );
      BOOST_REQUIRE( new_alice_bid.requested == bid.requested );
      BOOST_REQUIRE( new_alice_bid.remaining == 50 );
      BOOST_REQUIRE( new_alice_bid.expiration == bid.expiration );

      BOOST_TEST_MESSAGE( "│   ├── Passed: sucessful partial ad delivery" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: ad delivery bid completion" );
      
      for( auto i = 50; i < 100; i++ )
      {
         view.viewer = "newuser"+fc::to_string( i );

         tx.operations.push_back( view );
         tx.sign( alice_private_posting_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();

         generate_block();
      }

      const auto& bid_idx = db.get_index< ad_bid_index >().indices().get< by_bid_id >();
      auto bid_itr = bid_idx.find( boost::make_tuple( bid.account, bid.bid_id ) );

      const auto& cam_idx = db.get_index< ad_campaign_index >().indices().get< by_campaign_id >();
      auto cam_itr = cam_idx.find( boost::make_tuple( campaign.account, campaign.campaign_id ) );

      const auto& inv_idx = db.get_index< ad_inventory_index >().indices().get< by_inventory_id >();
      auto inv_itr = inv_idx.find( boost::make_tuple( inventory.provider, inventory.inventory_id ) );

      BOOST_REQUIRE( bid_itr == bid_idx.end() );
      BOOST_REQUIRE( cam_itr == cam_idx.end() );
      BOOST_REQUIRE( inv_itr == inv_idx.end() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: ad delivery bid completion" );

      BOOST_TEST_MESSAGE( "├── Passed: AD OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()