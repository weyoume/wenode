/**
 * #include <boost/test/unit_test.hpp>
#include <node/protocol/exceptions.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/database.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/util/reward.hpp>
#include <node/plugins/debug_node/debug_node_plugin.hpp>
#include <fc/crypto/digest.hpp>
#include "../common/database_fixture.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace node;
using namespace node::chain;
using namespace node::protocol;
using std::string;

BOOST_FIXTURE_TEST_SUITE( processing_tests, clean_database_fixture )



   //==========================//
   // === Processing Tests === //
   //==========================//



BOOST_AUTO_TEST_CASE( recent_content_claims_decay_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CONTENT CLAIMS DECAY" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Ongoing claims for duration of comment rewards" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob) )
      generate_block();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = alice.name;
      comment.author = alice.name;
      comment.permlink = "test";
      comment.title = "Lorem Ipsum";
      comment.body = "Hello";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.url = "https://www.url.com";
      comment.public_key = "";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "test";
      comment.json = "{\"foo\":\"bar\"}";
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
      
      vote_operation vote;

      vote.signatory = alice.name;
      vote.voter = alice.name;
      vote.author = alice.name;
      vote.permlink = "test";
      vote.weight = PERCENT_100;
      vote.validate();

      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const reward_fund_object& rf = db.get_reward_fund( SYMBOL_COIN );
      const median_chain_property_object& median_props = db.get_median_chain_properties();
      const comment_object& alice_comment = db.get_comment( alice.name, comment.permlink );

      uint128_t alice_reward_curve = util::evaluate_reward_curve(
         uint128_t( alice_comment.net_reward.value ),
         alice_comment.cashouts_received,
         rf.author_reward_curve,
         median_props.content_reward_decay_rate,
         rf.content_constant );

      BOOST_REQUIRE( rf.recent_content_claims == alice_reward_curve );

      while( alice_comment.cashouts_received < 30 )
      {
         alice_reward_curve -= ( alice_reward_curve * BLOCK_INTERVAL.count() ) / RECENT_REWARD_DECAY_RATE.count();
         
         BOOST_REQUIRE( rf.recent_content_claims == alice_reward_curve );

         BOOST_REQUIRE( alice_comment.cashouts_received == ( db.head_block_time() - alice_comment.created ).to_seconds() / CONTENT_REWARD_INTERVAL.to_seconds() );

         generate_block();
      }

      validate_database();
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: Ongoing claims for duration of comment rewards" );

      BOOST_TEST_MESSAGE( "├── Passed: CONTENT CLAIMS DECAY" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_payout_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMENT PAYOUT" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment payout" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob) );

      fund( alice.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( bob.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = alice.name;
      comment.author = alice.name;
      comment.permlink = "test";
      comment.title = "Lorem Ipsum";
      comment.body = "Hello";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.url = "https://www.url.com";
      comment.public_key = "";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "test";
      comment.json = "{\"foo\":\"bar\"}";
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

      vote_operation vote;

      vote.signatory = alice.name;
      vote.voter = alice.name;
      vote.author = alice.name;
      vote.permlink = "test";
      vote.weight = 81 * PERCENT_1;

      tx.operations.push_back( comment );
      tx.operations.push_back( vote );

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      comment.signatory = bob.name;
      comment.author = bob.name;

      vote.signatory = bob.name;
      vote.voter = bob.name;
      vote.author = bob.name;
      vote.weight = 59 * PERCENT_1;

      tx.operations.push_back( comment );
      tx.operations.push_back( vote );

      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( alice.name, comment.permlink );

      BOOST_REQUIRE( alice_comment.cashouts_received == 0 );

      generate_blocks( alice_comment.cashout_time + fc::minutes(1), true );

      BOOST_REQUIRE( alice_comment.cashouts_received == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment payout complete" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMENT PAYOUT" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( comment_processing_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMENT PROCESSING" );
      
      BOOST_TEST_MESSAGE( "│   ├── Testing: Comment payout received in equal amounts for equal voting power" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(dan)(ulysses)(vivian)(wendy) );

      struct author_actor
      {
         author_actor(
            const account_name_type& n,
            fc::ecc::private_key pk,
            asset mpay ) :
            name(n),
            private_key(pk),
            max_accepted_payout(mpay) {}
         account_name_type                  name;
         fc::ecc::private_key               private_key;
         asset                              max_accepted_payout;
      };

      struct voter_actor
      {
         voter_actor( 
            const account_name_type& n, 
            fc::ecc::private_key pk,
            string fa ) :
            name(n), 
            private_key(pk), 
            favorite_author(fa) {}
         account_name_type                   name;
         fc::ecc::private_key     private_key;
         string                   favorite_author;
      };


      std::vector< author_actor > authors;
      std::vector< voter_actor > voters;

      authors.emplace_back( "alice", alice_private_posting_key, MAX_ACCEPTED_PAYOUT );
      authors.emplace_back( "bob", bob_private_posting_key, asset( 0, SYMBOL_USD ) );
      authors.emplace_back( "dan", dan_private_posting_key, MAX_ACCEPTED_PAYOUT );
      voters.emplace_back( "ulysses", ulysses_private_posting_key, "alice" );
      voters.emplace_back( "vivian", vivian_private_posting_key, "bob" );
      voters.emplace_back( "wendy", wendy_private_posting_key, "dan" );

      // A,B,D : posters
      // U,V,W : voters

      for( const auto& voter : voters )
      {
         fund( voter.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         fund_stake( voter.name, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      }

      // authors all write in the same block, but Bob declines payout

      for( const auto& author : authors )
      {
         signed_transaction tx;

         comment_operation comment;

         comment.signatory = author.name;
         comment.author = author.name;
         comment.permlink = "test";
         comment.title = "Lorem Ipsum";
         comment.body = "Hello";
         comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
         comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
         comment.url = "https://www.url.com";
         comment.public_key = "";
         comment.community = INIT_COMMUNITY;
         comment.tags.push_back( "test" );
         comment.interface = INIT_ACCOUNT;
         comment.language = "en";
         comment.parent_author = ROOT_POST_PARENT;
         comment.parent_permlink = "test";
         comment.json = "{\"foo\":\"bar\"}";
         comment.latitude = 37.8136;
         comment.longitude = 144.9631;
         comment.comment_price = asset( 0, SYMBOL_COIN );
         comment.reply_price = asset( 0, SYMBOL_COIN );
         comment.premium_price = asset( 0, SYMBOL_COIN );

         comment_options options;

         options.post_type = "article";
         options.reach = "tag";
         options.rating = 1;
         options.max_accepted_payout = author.max_accepted_payout;
         
         comment.options = options;
         comment.validate();

         tx.operations.push_back( comment );
         tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( author.private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();
      }

      // voters all vote in the same block with the same stake
      for( const auto& voter : voters )
      {
         signed_transaction tx;

         vote_operation vote;

         vote.signatory = voter.name;
         vote.voter = voter.name;
         vote.author = voter.favorite_author;
         vote.permlink = "test";
         vote.weight = PERCENT_100;
         vote.validate();

         tx.operations.push_back( vote );
         tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( voter.private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );

         tx.operations.clear();
         tx.signatures.clear();
      }

      const comment_object& alice_comment = db.get_comment( alice.name, string( "test" ) );

      generate_blocks( alice_comment.cashout_time + fc::minutes(1), true );

      for( const auto& author : authors )
      {
         const account_balance_object& bal = db.get_account_balance( author.name, SYMBOL_COIN );
         ilog( "${n} : ${r}", ("n", author.name)("r", bal.reward_balance) );
      }
      for( const auto& voter : voters )
      {
         const account_balance_object& bal = db.get_account_balance( voter.name, SYMBOL_COIN );
         ilog( "${n} : ${r}", ("n", voter.name)("r", bal.reward_balance) );
      }
      
      const account_balance_object& alice_account_balance = db.get_account_balance( alice.name, SYMBOL_COIN );
      const account_balance_object& bob_account_balance = db.get_account_balance( bob.name, SYMBOL_COIN );
      const account_balance_object& dan_account_balance = db.get_account_balance( dan.name, SYMBOL_COIN );

      BOOST_CHECK( alice_account_balance.reward_balance.value > 0 );
      BOOST_CHECK( bob_account_balance.reward_balance.value == 0 );
      BOOST_CHECK( dan_account_balance.get_reward_balance() == alice_account_balance.get_reward_balance() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Comment payout received in equal amounts for equal voting power" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMENT PAYOUT PROCESS" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( nested_comment_payout_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: NESTED COMMENT PAYOUT" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reward fund correctly allocates content rewards for post payouts" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( alice.name, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( alice.name, asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( bob.name, asset( 7500 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( bob.name, asset( 7500 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( candice.name, asset( 8000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( candice.name, asset( 8000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( dan.name, asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( dan.name, asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = alice.name;
      comment.author = alice.name;
      comment.permlink = "test";
      comment.title = "Lorem Ipsum";
      comment.body = "Hello";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.url = "https://www.url.com";
      comment.public_key = "";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "test";
      comment.json = "{\"foo\":\"bar\"}";
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
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_block();

      comment.signatory = "bob";
      comment.author = "bob";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = PERCENT_100;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.author = "alice";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.author = "bob";
      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "dan";
      vote.voter = "dan";
      vote.author = "bob";
      tx.operations.push_back( vote );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const reward_fund_object& rf = db.get_reward_fund( SYMBOL_COIN );
      const comment_object& alice_comment = db.get_comment( alice.name, comment.permlink );
      const comment_object& bob_comment = db.get_comment( bob.name, comment.permlink );

      generate_blocks( alice_comment.cashout_time - BLOCK_INTERVAL, true );

      uint128_t recent_content_claims = rf.recent_content_claims;

      asset reward = rf.content_reward_balance;

      BOOST_REQUIRE( recent_content_claims == 0 );

      generate_block();

      util::comment_reward_context ctx = db.get_comment_reward_context( rf );

      util::fill_comment_reward_context_local_state( ctx, alice_comment );
      asset alice_reward = db.get_comment_reward( ctx );

      util::fill_comment_reward_context_local_state( ctx, bob_comment );
      asset bob_reward = db.get_comment_reward( ctx );

      BOOST_REQUIRE( rf.content_reward_balance == reward - alice_reward );

      recent_content_claims = rf.recent_content_claims;
      reward = rf.content_reward_balance;

      generate_block();

      ctx = db.get_comment_reward_context( rf );

      util::fill_comment_reward_context_local_state( ctx, alice_comment );
      alice_reward = db.get_comment_reward( ctx );

      util::fill_comment_reward_context_local_state( ctx, bob_comment );
      bob_reward = db.get_comment_reward( ctx );

      BOOST_REQUIRE( rf.content_reward_balance == reward - bob_reward );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Reward fund correctly allocates content rewards for post payouts" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Nested Comments Payout" );

      comment.signatory = "candice";
      comment.author = "candice";
      comment.parent_author = "alice";
      
      tx.operations.push_back( comment );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      comment.signatory = "dan";
      comment.author = "dan";
      comment.parent_author = "candice";

      tx.operations.push_back( comment );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "dan";
      vote.voter = "dan";
      vote.author = "dan";

      tx.operations.push_back( vote );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.author = "candice";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const comment_object& candice_comment = db.get_comment( candice.name, comment.permlink );
      const comment_object& dan_comment = db.get_comment( dan.name, comment.permlink );

      reward = rf.content_reward_balance;

      generate_blocks( candice_comment.cashout_time, true );
      
      ctx = db.get_comment_reward_context( rf );

      util::fill_comment_reward_context_local_state( ctx, candice_comment );
      asset candice_reward = db.get_comment_reward( ctx );

      util::fill_comment_reward_context_local_state( ctx, dan_comment );
      asset dan_reward = db.get_comment_reward( ctx );
   
      BOOST_REQUIRE( rf.content_reward_balance == reward - candice_reward - dan_reward );

      BOOST_REQUIRE( alice_comment.total_payout_value == db.asset_to_USD( alice_reward ) );
      BOOST_REQUIRE( bob_comment.total_payout_value == db.asset_to_USD( bob_reward ) );
      BOOST_REQUIRE( candice_comment.total_payout_value == db.asset_to_USD( candice_reward ) );
      BOOST_REQUIRE( dan_comment.total_payout_value == db.asset_to_USD( dan_reward ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Nested Comments Payout" );
      
      BOOST_TEST_MESSAGE( "├── PASSED: NESTED COMMENT PAYOUT" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( asset_feed_publish_mean )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ASSET FEED MEDIAN SELECTION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Selection of median price feed" );

      resize_shared_mem( 1024 * 1024 * 32 );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( fc::seconds( 30 ).count() / BLOCK_INTERVAL.count() );

      vector< string > accounts;

      accounts.push_back( "producer" );
      accounts.push_back( "producer1" );
      accounts.push_back( "producer2" );
      accounts.push_back( "producer3" );
      accounts.push_back( "producer4" );
      accounts.push_back( "producer5" );
      accounts.push_back( "producer6" );

      vector< private_key_type > keys;
      keys.push_back( get_private_key( "producer", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "producer1", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "producer2", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "producer3", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "producer4", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "producer5", "active", INIT_ACCOUNT_PASSWORD ) );
      keys.push_back( get_private_key( "producer6", "active", INIT_ACCOUNT_PASSWORD ) );

      vector< asset_publish_feed_operation > publish_feeds;
      vector< account_producer_vote_operation > producer_votes;
      vector< signed_transaction > txs;

      // Upgrade accounts to producers

      for( int i = 0; i < 7; i++ )
      {
         fund( accounts[i], asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         fund_stake( accounts[i], asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

         producer_votes.push_back( account_producer_vote_operation() );
         producer_votes[i].signatory = accounts[i];
         producer_votes[i].account = accounts[i];
         producer_votes[i].vote_rank = 1;
         producer_votes[i].producer = accounts[i];

         publish_feeds.push_back( asset_publish_feed_operation() );
         publish_feeds[i].signatory = accounts[i];
         publish_feeds[i].publisher = accounts[i];
         publish_feeds[i].symbol = SYMBOL_USD;

         txs.push_back( signed_transaction() );
      }

      publish_feeds[0].feed.settlement_price = price( asset( 500, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      publish_feeds[1].feed.settlement_price = price( asset( 550, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      publish_feeds[2].feed.settlement_price = price( asset( 600, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      publish_feeds[3].feed.settlement_price = price( asset( 650, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );   // Median price
      publish_feeds[4].feed.settlement_price = price( asset( 700, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      publish_feeds[5].feed.settlement_price = price( asset( 750, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      publish_feeds[6].feed.settlement_price = price( asset( 800, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );

      for( int i = 0; i < 7; i++ )
      {
         txs[i].set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         txs[i].operations.push_back( publish_feeds[i] );
         txs[i].operations.push_back( producer_votes[i] );
         txs[i].sign( keys[i], db.get_chain_id() );
         db.push_transaction( txs[i], 0 );
      }

      generate_blocks( BLOCKS_PER_HOUR ); // Jump forward 1 hour

      const asset_stablecoin_data_object& stablecoin = db.get_stablecoin_data( SYMBOL_USD );

      BOOST_REQUIRE( stablecoin.current_feed.settlement_price == price( asset( 650, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) ) );

      BOOST_REQUIRE( stablecoin.feeds.size() == 7 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Selection of median price feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Updating median price feed" );

      for ( int i = 0; i < 23; i++ )
      {
         for( int j = 0; j < 7; j++ )
         {
            txs[j].operations.clear();
            txs[j].signatures.clear();

            publish_feeds[j].feed.settlement_price = price( 
               asset( publish_feeds[j].feed.settlement_price.base.amount + 10, SYMBOL_USD ), 
               asset( publish_feeds[j].feed.settlement_price.quote.amount, SYMBOL_COIN ) );

            txs[j].set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
            txs[j].operations.push_back( publish_feeds[j] );
            txs[j].sign( keys[j], db.get_chain_id() );
            db.push_transaction( txs[j], 0 );
         }

         generate_blocks( BLOCKS_PER_HOUR  ); // Jump forward 1 hour

         BOOST_REQUIRE( stablecoin.current_feed.settlement_price == publish_feeds[4].feed.settlement_price );

         validate_database();
      }

      BOOST_TEST_MESSAGE( "│   ├── Passed: Updating median price feed" );

      BOOST_TEST_MESSAGE( "├── Passed: ASSET FEED MEDIAN SELECTION" );
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( network_credit_interest_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: NETWORK CREDIT INTEREST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Payment of interest on credit balances" );

      fund( INIT_ACCOUNT, asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      ACTORS( (alice) );

      generate_block();

      fund( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
      fund_stake( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
      fund_savings( alice.name, asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );

      time_point now = db.head_block_time();

      signed_transaction tx;

      asset init_liquid_balance = db.get_liquid_balance( alice.name, SYMBOL_CREDIT );
      asset init_staked_balance = db.get_staked_balance( alice.name, SYMBOL_CREDIT );
      asset init_savings_balance = db.get_savings_balance( alice.name, SYMBOL_CREDIT );

      const asset_credit_data_object& credit = db.get_credit_data( SYMBOL_CREDIT );
      asset_symbol_type cs = credit.symbol;
      price buyback = credit.buyback_price;
      price market = db.get_liquidity_pool( credit.buyback_asset, credit.symbol ).base_hour_median_price( credit.buyback_asset );
      
      asset unit = asset( BLOCKCHAIN_PRECISION, credit.symbol );
      share_type range = credit.var_interest_range;     // Percentage range that caps the price divergence between market and buyback
      share_type pr = PERCENT_100;
      share_type hpr = PERCENT_100 / 2;

      share_type mar = ( unit * market ).amount;    // Market price of the credit asset
      share_type buy = ( unit * buyback ).amount;   // Buyback price of the credit asset

      share_type liqf = credit.liquid_fixed_interest_rate;
      share_type staf = credit.staked_fixed_interest_rate;
      share_type savf = credit.savings_fixed_interest_rate;
      share_type liqv = credit.liquid_variable_interest_rate;
      share_type stav = credit.staked_variable_interest_rate;
      share_type savv = credit.savings_variable_interest_rate;

      share_type var_factor = ( ( -hpr * std::min( pr, std::max( -pr, pr * ( mar-buy ) / ( ( ( buy * range ) / pr ) ) ) ) ) / pr ) + hpr;

      share_type liq_ir = liqv * var_factor + liqf;
      share_type sta_ir = stav * var_factor + staf;
      share_type sav_ir = savv * var_factor + savf;
      
      const account_balance_object& balance = db.get_account_balance( "alice", SYMBOL_CREDIT );

      asset liquid_interest = asset( ( ( ( balance.liquid_balance * liq_ir * ( now - balance.last_interest_time ).to_seconds() ) / fc::days(365).to_seconds() ) ) / pr , cs );
      asset staked_interest = asset( ( ( ( balance.staked_balance * sta_ir * ( now - balance.last_interest_time ).to_seconds() ) / fc::days(365).to_seconds() ) ) / pr , cs);
      asset savings_interest = asset( ( ( ( balance.savings_balance * sav_ir * ( now - balance.last_interest_time ).to_seconds() ) / fc::days(365).to_seconds() ) ) / pr , cs);

      generate_blocks( CREDIT_INTERVAL_BLOCKS );

      asset liquid_balance = db.get_liquid_balance( "alice", SYMBOL_CREDIT );
      asset staked_balance = db.get_staked_balance( "alice", SYMBOL_CREDIT );
      asset savings_balance = db.get_savings_balance( "alice", SYMBOL_CREDIT );

      BOOST_REQUIRE( liquid_balance == init_liquid_balance + liquid_interest );
      BOOST_REQUIRE( staked_balance == init_staked_balance + staked_interest );
      BOOST_REQUIRE( savings_balance == init_savings_balance + savings_interest );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Payment of interest on credit balances" );

      BOOST_TEST_MESSAGE( "├── Passed: NETWORK CREDIT INTEREST" );
   }
   FC_LOG_AND_RETHROW();
}


BOOST_AUTO_TEST_SUITE_END()

*/