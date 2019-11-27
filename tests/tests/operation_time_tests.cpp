#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <node/protocol/exceptions.hpp>

#include <node/chain/block_summary_object.hpp>
#include <node/chain/database.hpp>
#include <node/chain/hardfork.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/node_objects.hpp>

#include <node/chain/util/reward.hpp>

#include <node/plugins/debug_node/debug_node_plugin.hpp>

#include <fc/crypto/digest.hpp>

#include "../common/database_fixture.hpp"

#include <cmath>

using namespace node;
using namespace node::chain;
using namespace node::protocol;

BOOST_FIXTURE_TEST_SUITE( operation_time_tests, clean_database_fixture )

BOOST_AUTO_TEST_CASE( comment_payout_equalize )
{
   try
   {
      ACTORS( (alice)(bob)(dave)
              (ulysses)(vivian)(wendy) )

      struct author_actor
      {
         author_actor(
            const std::string& n,
            fc::ecc::private_key pk,
            fc::optional<asset> mpay = fc::optional<asset>() )
            : name(n), private_key(pk), max_accepted_payout(mpay) {}
         std::string             name;
         fc::ecc::private_key    private_key;
         fc::optional< asset >   max_accepted_payout;
      };

      struct voter_actor
      {
         voter_actor( const std::string& n, fc::ecc::private_key pk, std::string fa )
            : name(n), private_key(pk), favorite_author(fa) {}
         std::string             name;
         fc::ecc::private_key    private_key;
         std::string             favorite_author;
      };


      std::vector< author_actor > authors;
      std::vector< voter_actor > voters;

      authors.emplace_back( "alice", alice_private_key );
      authors.emplace_back( "bob"  , bob_private_key, ASSET( "0.000 USD" ) );
      authors.emplace_back( "dave" , dave_private_key );
      voters.emplace_back( "ulysses", ulysses_private_key, "alice");
      voters.emplace_back( "vivian" , vivian_private_key , "bob"  );
      voters.emplace_back( "wendy"  , wendy_private_key  , "dave" );

      // A,B,D : posters
      // U,V,W : voters

      // set a ridiculously high TME price ($1 / satoshi) to disable dust threshold
      set_price_feed( price( ASSET( "0.001 TESTS" ), ASSET( "1.000 USD" ) ) );

      for( const auto& voter : voters )
      {
         fund( voter.name, 10000 );
         score( voter.name, 10000 );
      }

      // authors all write in the same block, but Bob declines payout
      for( const auto& author : authors )
      {
         signed_transaction tx;
         comment_operation com;
         com.author = author.name;
         com.permlink = "mypost";
         com.parent_author = ROOT_POST_PARENT;
         com.parent_permlink = "test";
         com.title = "Hello from "+author.name;
         com.body = "Hello, my name is "+author.name;
         tx.operations.push_back( com );

         if( author.max_accepted_payout.valid() )
         {
            comment_options_operation copt;
            copt.author = com.author;
            copt.permlink = com.permlink;
            copt.max_accepted_payout = *(author.max_accepted_payout);
            tx.operations.push_back( copt );
         }

         tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
         tx.sign( author.private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      }

      generate_blocks(1);

      // voters all vote in the same block with the same stake
      for( const auto& voter : voters )
      {
         signed_transaction tx;
         vote_operation vote;
         vote.voter = voter.name;
         vote.author = voter.favorite_author;
         vote.permlink = "mypost";
         vote.weight = PERCENT_100;
         tx.operations.push_back( vote );
         tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
         tx.sign( voter.private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      }

      auto TMEreward = db.get_dynamic_global_properties().total_reward_fund;

      // generate a few blocks to seed the reward fund
      generate_blocks(10);
      //const auto& rf = db.get< reward_fund_object, by_name >( REWARD_FUND_NAME );
      //idump( (rf) );

      generate_blocks( db.get_comment( "alice", string( "mypost" ) ).cashout_time, true );
      /*
      for( const auto& author : authors )
      {
         const account_object& a = db.get_account(author.name);
         ilog( "${n} : ${TME} ${USD}", ("n", author.name)("TME", a.TMErewardBalance)("USD", a.USDrewardBalance) );
      }
      for( const auto& voter : voters )
      {
         const account_object& a = db.get_account(voter.name);
         ilog( "${n} : ${TME} ${USD}", ("n", voter.name)("TME", a.TMErewardBalance)("USD", a.USDrewardBalance) );
      }
      */

      const account_object& alice_account = db.get_account("alice");
      const account_object& bob_account   = db.get_account("bob");
      const account_object& dave_account  = db.get_account("dave");

      BOOST_CHECK( alice_account.USDrewardBalance == ASSET( "14288.000 USD" ) );
      BOOST_CHECK( bob_account.USDrewardBalance == ASSET( "0.000 USD" ) );
      BOOST_CHECK( dave_account.USDrewardBalance == alice_account.USDrewardBalance );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_payout_dust )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: comment_payout_dust" );

      ACTORS( (alice)(bob) )
      generate_block();

      score( "alice", ASSET( "10.000 TESTS" ) );
      score( "bob", ASSET( "10.000 TESTS" ) );

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) ) );

      generate_block();
      validate_database();

      comment_operation comment;
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";
      vote_operation vote;
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = 81 * PERCENT_1;

      signed_transaction tx;
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      comment.author = "bob";
      vote.voter = "bob";
      vote.author = "bob";
      vote.weight = 59 * PERCENT_1;

      tx.clear();
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time );

      // If comments are paid out independent of order, then the last satoshi of TME cannot be divided among them
      const auto rf = db.get< reward_fund_object, by_name >( REWARD_FUND_NAME );
      BOOST_REQUIRE( rf.content_reward_balance == ASSET( "0.001 TESTS" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "Done" );
   }
   FC_LOG_AND_RETHROW()
}

/*
BOOST_AUTO_TEST_CASE( reward_funds )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: reward_funds" );

      ACTORS( (alice)(bob) )
      generate_block();

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) ) );
      generate_block();

      comment_operation comment;
      vote_operation vote;
      signed_transaction tx;

      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "foo";
      comment.body = "bar";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = PERCENT_100;
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( 5 );

      comment.author = "bob";
      comment.parent_author = "alice";
      vote.voter = "bob";
      vote.author = "bob";
      tx.clear();
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time );

      {
         const auto& post_rf = db.get< reward_fund_object, by_name >( REWARD_FUND_NAME );
         const auto& comment_rf = db.get< reward_fund_object, by_name >( COMMENT_REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.content_reward_balance.amount == 0 );
         BOOST_REQUIRE( comment_rf.content_reward_balance.amount > 0 );
         BOOST_REQUIRE( db.get_account( "alice" ).USDrewardBalance.amount > 0 );
         BOOST_REQUIRE( db.get_account( "bob" ).USDrewardBalance.amount == 0 );
         validate_database();
      }

      generate_blocks( db.get_comment( "bob", string( "test" ) ).cashout_time );

      {
         const auto& post_rf = db.get< reward_fund_object, by_name >( REWARD_FUND_NAME );
         const auto& comment_rf = db.get< reward_fund_object, by_name >( COMMENT_REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.content_reward_balance.amount > 0 );
         BOOST_REQUIRE( comment_rf.content_reward_balance.amount == 0 );
         BOOST_REQUIRE( db.get_account( "alice" ).USDrewardBalance.amount > 0 );
         BOOST_REQUIRE( db.get_account( "bob" ).USDrewardBalance.amount > 0 );
         validate_database();
      }
   }
   FC_LOG_AND_RETHROW()
}
*/

BOOST_AUTO_TEST_CASE( recent_content_claims_decay )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: recent_reward_2decay" );
      ACTORS( (alice)(bob) )
      generate_block();

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) ) );
      generate_block();

      comment_operation comment;
      vote_operation vote;
      signed_transaction tx;

      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "foo";
      comment.body = "bar";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = PERCENT_100;
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto alice_vSCORE = util::evaluate_reward_curve( db.get_comment( "alice", string( "test" ) ).net_reward.value,
         db.get< reward_fund_object, by_name >( REWARD_FUND_NAME ).author_reward_curve,
         db.get< reward_fund_object, by_name >( REWARD_FUND_NAME ).content_constant );

      generate_blocks( 5 );

      comment.author = "bob";
      vote.voter = "bob";
      vote.author = "bob";
      tx.clear();
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time );

      {
         const auto& post_rf = db.get< reward_fund_object, by_name >( REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.recent_content_claims == alice_vSCORE );
         validate_database();
      }

      auto bob_cashout_time = db.get_comment( "bob", string( "test" ) ).cashout_time;
      auto bob_vSCORE = util::evaluate_reward_curve( db.get_comment( "bob", string( "test" ) ).net_reward.value,
         db.get< reward_fund_object, by_name >( REWARD_FUND_NAME ).author_reward_curve,
         db.get< reward_fund_object, by_name >( REWARD_FUND_NAME ).content_constant );

      generate_block();

      while( db.head_block_time() < bob_cashout_time )
      {
         alice_vSCORE -= ( alice_vSCORE * BLOCK_INTERVAL ) / RECENT_REWARD_DECAY_RATE.to_seconds();
         const auto& post_rf = db.get< reward_fund_object, by_name >( REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.recent_content_claims == alice_vSCORE );

         generate_block();

      }

      {
         alice_vSCORE -= ( alice_vSCORE * BLOCK_INTERVAL ) / RECENT_REWARD_DECAY_RATE.to_seconds();
         const auto& post_rf = db.get< reward_fund_object, by_name >( REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.recent_content_claims == alice_vSCORE + bob_vSCORE );
         validate_database();
      }
   }
   FC_LOG_AND_RETHROW()
}

/*BOOST_AUTO_TEST_CASE( comment_payout )
{
   try
   {
      ACTORS( (alice)(bob)(sam)(dave) )
      fund( "alice", 10000 );
      score( "alice", 10000 );
      fund( "bob", 7500 );
      score( "bob", 7500 );
      fund( "sam", 8000 );
      score( "sam", 8000 );
      fund( "dave", 5000 );
      score( "dave", 5000 );

      price exchange_rate( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );
      set_price_feed( exchange_rate );

      signed_transaction tx;

      BOOST_TEST_MESSAGE( "Creating comments." );

      comment_operation com;
      com.author = "alice";
      com.permlink = "test";
      com.parent_author = "";
      com.parent_permlink = "test";
      com.title = "foo";
      com.body = "bar";
      tx.operations.push_back( com );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      com.author = "bob";
      tx.operations.push_back( com );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "Voting for comments." );

      vote_operation vote;
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = PERCENT_100;
      tx.operations.push_back( vote );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "sam";
      vote.author = "alice";
      tx.operations.push_back( vote );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "bob";
      vote.author = "bob";
      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.voter = "dave";
      vote.author = "bob";
      tx.operations.push_back( vote );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Generate blocks up until first payout" );

      //generate_blocks( db.get_comment( "bob", string( "test" ) ).cashout_time - BLOCK_INTERVAL, true );

      auto TMEreward = db.get_dynamic_global_properties().total_reward_fund + ASSET( "1.667 TESTS" );
      auto total_reward_squared = db.get_dynamic_global_properties().total_reward_squared;
      auto bob_comment_reward = db.get_comment( "bob", string( "test" ) ).net_reward;
      auto bob_SCORE = db.get_account( "bob" ).SCORE;
      auto bob_USDbalance = db.get_account( "bob" ).USDbalance;

      auto bob_comment_payout = asset( ( ( uint128_t( bob_comment_reward.value ) * bob_comment_reward.value * TMEreward.amount.value ) / total_reward_squared ).to_uint64(), SYMBOL_COIN );
      auto bob_comment_discussion_rewards = asset( bob_comment_payout.amount / 4, SYMBOL_COIN );
      bob_comment_payout -= bob_comment_discussion_rewards;
      auto bob_comment_USDreward = db.asset_to_USD( asset( bob_comment_payout.amount / 2, SYMBOL_COIN ) );
      auto bob_comment_TME_fund_for_SCORE_reward = ( bob_comment_payout - asset( bob_comment_payout.amount / 2, SYMBOL_COIN) ) * db.get_dynamic_global_properties().get_SCORE_price();

      BOOST_TEST_MESSAGE( "Cause first payout" );

      generate_block();

      BOOST_REQUIRE( db.get_dynamic_global_properties().total_reward_fund == TMEreward - bob_comment_payout );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).total_payout_value == bob_comment_TME_fund_for_SCORE_reward * db.get_dynamic_global_properties().get_SCORE_price() + bob_comment_USDreward * exchange_rate );
      BOOST_REQUIRE( db.get_account( "bob" ).SCORE == bob_SCORE + bob_comment_TME_fund_for_SCORE_reward );
      BOOST_REQUIRE( db.get_account( "bob" ).USDbalance == bob_USDbalance + bob_comment_USDreward );

      BOOST_TEST_MESSAGE( "Testing no payout when less than $0.02" );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "alice";
      vote.author = "alice";
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "dave";
      vote.author = "bob";
      vote.weight = PERCENT_1;
      tx.operations.push_back( vote );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "bob", string( "test" ) ).cashout_time - BLOCK_INTERVAL, true );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "bob";
      vote.author = "alice";
      vote.weight = PERCENT_100;
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "sam";
      tx.operations.push_back( vote );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "dave";
      tx.operations.push_back( vote );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      bob_SCORE = db.get_account( "bob" ).SCORE;
      bob_USDbalance = db.get_account( "bob" ).USDbalance;

      validate_database();

      generate_block();

      BOOST_REQUIRE( bob_SCORE.amount.value == db.get_account( "bob" ).SCORE.amount.value );
      BOOST_REQUIRE( bob_USDbalance.amount.value == db.get_account( "bob" ).USDbalance.amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}*/

/*
BOOST_AUTO_TEST_CASE( comment_payout )
{
   try
   {
      ACTORS( (alice)(bob)(sam)(dave) )
      fund( "alice", 10000 );
      score( "alice", 10000 );
      fund( "bob", 7500 );
      score( "bob", 7500 );
      fund( "sam", 8000 );
      score( "sam", 8000 );
      fund( "dave", 5000 );
      score( "dave", 5000 );

      price exchange_rate( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );
      set_price_feed( exchange_rate );

      auto gpo = db.get_dynamic_global_properties();

      signed_transaction tx;

      BOOST_TEST_MESSAGE( "Creating comments. " );

      comment_operation com;
      com.author = "alice";
      com.permlink = "test";
      com.parent_permlink = "test";
      com.title = "foo";
      com.body = "bar";
      tx.operations.push_back( com );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      com.author = "bob";
      tx.operations.push_back( com );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "First round of votes." );

      tx.operations.clear();
      tx.signatures.clear();
      vote_operation vote;
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = PERCENT_100;
      tx.operations.push_back( vote );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "dave";
      tx.operations.push_back( vote );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "bob";
      vote.author = "bob";
      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "sam";
      tx.operations.push_back( vote );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Generating blocks..." );

      generate_blocks( fc::time_point( db.head_block_time().sec_since_epoch() + CONTENT_REWARD_INTERVAL_SECONDS / 2 ), true );

      BOOST_TEST_MESSAGE( "Second round of votes." );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "alice";
      vote.author = "bob";
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( vote );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "bob";
      vote.author = "alice";
      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "sam";
      tx.operations.push_back( vote );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Generating more blocks..." );

      generate_blocks( db.get_comment( "bob", string( "test" ) ).cashout_time - ( BLOCK_INTERVAL / 2 ), true );

      BOOST_TEST_MESSAGE( "Check comments have not been paid out." );

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "sam" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "dave" ) ).id  ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "alice" ) ).id ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "bob" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "sam" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value > 0 );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).net_reward.value > 0 );
      validate_database();

      auto TMEreward = db.get_dynamic_global_properties().total_reward_fund + ASSET( "2.000 TESTS" );
      auto total_reward_squared = db.get_dynamic_global_properties().total_reward_squared;
      auto bob_comment_vote_total = db.get_comment( "bob", string( "test" ) ).total_vote_weight;
      auto bob_comment_reward = db.get_comment( "bob", string( "test" ) ).net_reward;
      auto bob_USDbalance = db.get_account( "bob" ).USDbalance;
      auto alice_SCORE = db.get_account( "alice" ).SCORE;
      auto bob_SCORE = db.get_account( "bob" ).SCORE;
      auto sam_SCORE = db.get_account( "sam" ).SCORE;
      auto dave_SCORE = db.get_account( "dave" ).SCORE;

      auto bob_comment_payout = asset( ( ( uint128_t( bob_comment_reward.value ) * bob_comment_reward.value * TMEreward.amount.value ) / total_reward_squared ).to_uint64(), SYMBOL_COIN );
      auto bob_comment_vote_rewards = asset( bob_comment_payout.amount / 2, SYMBOL_COIN );
      bob_comment_payout -= bob_comment_vote_rewards;
      auto bob_comment_USDreward = asset( bob_comment_payout.amount / 2, SYMBOL_COIN ) * exchange_rate;
      auto bob_comment_TME_fund_for_SCORE_reward = ( bob_comment_payout - asset( bob_comment_payout.amount / 2, SYMBOL_COIN ) ) * db.get_dynamic_global_properties().get_SCORE_price();
      auto unclaimed_payments = bob_comment_vote_rewards;
      auto alice_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * bob_comment_vote_rewards.amount.value ) / bob_comment_vote_total ), SYMBOL_COIN );
      auto alice_vote_TME_fund_for_SCORE = alice_vote_reward * db.get_dynamic_global_properties().get_SCORE_price();
      auto bob_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * bob_comment_vote_rewards.amount.value ) / bob_comment_vote_total ), SYMBOL_COIN );
      auto bob_vote_TME_fund_for_SCORE = bob_vote_reward * db.get_dynamic_global_properties().get_SCORE_price();
      auto sam_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "sam" ) ).id ) )->weight ) * bob_comment_vote_rewards.amount.value ) / bob_comment_vote_total ), SYMBOL_COIN );
      auto sam_vote_TME_fund_for_SCORE = sam_vote_reward * db.get_dynamic_global_properties().get_SCORE_price();
      unclaimed_payments -= ( alice_vote_reward + bob_vote_reward + sam_vote_reward );

      BOOST_TEST_MESSAGE( "Generate one block to cause a payout" );

      generate_block();

      auto bob_comment_reward = get_last_operations( 1 )[0].get< comment_reward_operation >();

      BOOST_REQUIRE( db.get_dynamic_global_properties().total_reward_fund.amount.value == TMEreward.amount.value - ( bob_comment_payout + bob_comment_vote_rewards - unclaimed_payments ).amount.value );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).total_payout_value.amount.value == ( ( bob_comment_TME_fund_for_SCORE_reward * db.get_dynamic_global_properties().get_SCORE_price() ) + ( bob_comment_USDreward * exchange_rate ) ).amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).USDbalance.amount.value == ( bob_USDbalance + bob_comment_USDreward ).amount.value );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value > 0 );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).net_reward.value == 0 );
      BOOST_REQUIRE( db.get_account( "alice" ).SCORE.amount.value == ( alice_SCORE + alice_vote_TME_fund_for_SCORE ).amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).SCORE.amount.value == ( bob_SCORE + bob_vote_TME_fund_for_SCORE + bob_comment_TME_fund_for_SCORE_reward ).amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).SCORE.amount.value == ( sam_SCORE + sam_vote_TME_fund_for_SCORE ).amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).SCORE.amount.value == dave_SCORE.amount.value );
      BOOST_REQUIRE( bob_comment_reward.author == "bob" );
      BOOST_REQUIRE( bob_comment_reward.permlink == "test" );
      BOOST_REQUIRE( bob_comment_reward.payout.amount.value == bob_comment_USDreward.amount.value );
      BOOST_REQUIRE( bob_comment_reward.SCOREpayout.amount.value == bob_comment_TME_fund_for_SCORE_reward.amount.value );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "sam" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "dave" ) ).id  ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "alice" ) ).id ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "bob" ) ).id   ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "sam" ) ).id   ) ) == vote_idx.end() );
      validate_database();

      BOOST_TEST_MESSAGE( "Generating blocks up to next comment payout" );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time - ( BLOCK_INTERVAL / 2 ), true );

      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "sam" ) ).id   ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "dave" ) ).id  ) ) != vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "alice" ) ).id ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "bob" ) ).id   ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "sam" ) ).id   ) ) == vote_idx.end() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value > 0 );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).net_reward.value == 0 );
      validate_database();

      BOOST_TEST_MESSAGE( "Generate block to cause payout" );

      TMEreward = db.get_dynamic_global_properties().total_reward_fund + ASSET( "2.000 TESTS" );
      total_reward_squared = db.get_dynamic_global_properties().total_reward_squared;
      auto alice_comment_vote_total = db.get_comment( "alice", string( "test" ) ).total_vote_weight;
      auto alice_comment_reward = db.get_comment( "alice", string( "test" ) ).net_reward;
      auto alice_USDbalance = db.get_account( "alice" ).USDbalance;
      alice_SCORE = db.get_account( "alice" ).SCORE;
      bob_SCORE = db.get_account( "bob" ).SCORE;
      sam_SCORE = db.get_account( "sam" ).SCORE;
      dave_SCORE = db.get_account( "dave" ).SCORE;

      u256 rs( alice_comment_reward.value );
      u256 rf( TMEreward.amount.value );
      u256 trs2 = total_reward_squared.hi;
      trs2 = ( trs2 << 64 ) + total_reward_squared.lo;
      auto rs2 = rs*rs;

      auto alice_comment_payout = asset( static_cast< uint64_t >( ( rf * rs2 ) / trs2 ), SYMBOL_COIN );
      auto alice_comment_vote_rewards = asset( alice_comment_payout.amount / 2, SYMBOL_COIN );
      alice_comment_payout -= alice_comment_vote_rewards;
      auto alice_comment_USDreward = asset( alice_comment_payout.amount / 2, SYMBOL_COIN ) * exchange_rate;
      auto alice_comment_TME_fund_for_SCORE_reward = ( alice_comment_payout - asset( alice_comment_payout.amount / 2, SYMBOL_COIN ) ) * db.get_dynamic_global_properties().get_SCORE_price();
      unclaimed_payments = alice_comment_vote_rewards;
      alice_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_COIN );
      alice_vote_TME_fund_for_SCORE = alice_vote_reward * db.get_dynamic_global_properties().get_SCORE_price();
      bob_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_COIN );
      bob_vote_TME_fund_for_SCORE = bob_vote_reward * db.get_dynamic_global_properties().get_SCORE_price();
      sam_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "sam" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_COIN );
      sam_vote_TME_fund_for_SCORE = sam_vote_reward * db.get_dynamic_global_properties().get_SCORE_price();
      auto dave_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "dave" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_COIN );
      auto dave_vote_TME_fund_for_SCORE = dave_vote_reward * db.get_dynamic_global_properties().get_SCORE_price();
      unclaimed_payments -= ( alice_vote_reward + bob_vote_reward + sam_vote_reward + dave_vote_reward );

      generate_block();
      auto alice_comment_reward = get_last_operations( 1 )[0].get< comment_reward_operation >();

      BOOST_REQUIRE( ( db.get_dynamic_global_properties().total_reward_fund + alice_comment_payout + alice_comment_vote_rewards - unclaimed_payments ).amount.value == TMEreward.amount.value );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).total_payout_value.amount.value == ( ( alice_comment_TME_fund_for_SCORE_reward * db.get_dynamic_global_properties().get_SCORE_price() ) + ( alice_comment_USDreward * exchange_rate ) ).amount.value );
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance.amount.value == ( alice_USDbalance + alice_comment_USDreward ).amount.value );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value == 0 );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value == 0 );
      BOOST_REQUIRE( db.get_account( "alice" ).SCORE.amount.value == ( alice_SCORE + alice_vote_TME_fund_for_SCORE + alice_comment_TME_fund_for_SCORE_reward ).amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).SCORE.amount.value == ( bob_SCORE + bob_vote_TME_fund_for_SCORE ).amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).SCORE.amount.value == ( sam_SCORE + sam_vote_TME_fund_for_SCORE ).amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).SCORE.amount.value == ( dave_SCORE + dave_vote_TME_fund_for_SCORE ).amount.value );
      BOOST_REQUIRE( alice_comment_reward.author == "alice" );
      BOOST_REQUIRE( alice_comment_reward.permlink == "test" );
      BOOST_REQUIRE( alice_comment_reward.payout.amount.value == alice_comment_USDreward.amount.value );
      BOOST_REQUIRE( alice_comment_reward.SCOREpayout.amount.value == alice_comment_TME_fund_for_SCORE_reward.amount.value );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id   ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "sam" ) ).id   ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "dave" ) ).id  ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "alice" ) ).id ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "bob" ) ).id   ) ) == vote_idx.end() );
      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id,   db.get_account( "sam" ) ).id   ) ) == vote_idx.end() );
      validate_database();

      BOOST_TEST_MESSAGE( "Testing no payout when less than $0.02" );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "alice";
      vote.author = "alice";
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "dave";
      vote.author = "bob";
      vote.weight = PERCENT_1;
      tx.operations.push_back( vote );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "bob", string( "test" ) ).cashout_time - BLOCK_INTERVAL, true );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "bob";
      vote.author = "alice";
      vote.weight = PERCENT_100;
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "sam";
      tx.operations.push_back( vote );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote.voter = "dave";
      tx.operations.push_back( vote );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      bob_SCORE = db.get_account( "bob" ).SCORE;
      auto bob_USD = db.get_account( "bob" ).USDbalance;

      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "dave" ) ).id ) ) != vote_idx.end() );
      validate_database();

      generate_block();

      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "dave" ) ).id ) ) == vote_idx.end() );
      BOOST_REQUIRE( bob_SCORE.amount.value == db.get_account( "bob" ).SCORE.amount.value );
      BOOST_REQUIRE( bob_USD.amount.value == db.get_account( "bob" ).USDbalance.amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( nested_comments )
{
   try
   {
      ACTORS( (alice)(bob)(sam)(dave) )
      fund( "alice", 10000 );
      score( "alice", 10000 );
      fund( "bob", 10000 );
      score( "bob", 10000 );
      fund( "sam", 10000 );
      score( "sam", 10000 );
      fund( "dave", 10000 );
      score( "dave", 10000 );

      price exchange_rate( ASSET( "1.000 TESTS" ), ASSET( "1.000 USD" ) );
      set_price_feed( exchange_rate );

      signed_transaction tx;
      comment_operation comment_op;
      comment_op.author = "alice";
      comment_op.permlink = "test";
      comment_op.parent_permlink = "test";
      comment_op.title = "foo";
      comment_op.body = "bar";
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( comment_op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment_op.author = "bob";
      comment_op.parent_author = "alice";
      comment_op.parent_permlink = "test";
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( comment_op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment_op.author = "sam";
      comment_op.parent_author = "bob";
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( comment_op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment_op.author = "dave";
      comment_op.parent_author = "sam";
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( comment_op );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_operation vote_op;
      vote_op.voter = "alice";
      vote_op.author = "alice";
      vote_op.permlink = "test";
      vote_op.weight = PERCENT_100;
      tx.operations.push_back( vote_op );
      vote_op.author = "bob";
      tx.operations.push_back( vote_op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote_op.voter = "bob";
      vote_op.author = "alice";
      tx.operations.push_back( vote_op );
      vote_op.author = "bob";
      tx.operations.push_back( vote_op );
      vote_op.author = "dave";
      vote_op.weight = PERCENT_1 * 20;
      tx.operations.push_back( vote_op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      vote_op.voter = "sam";
      vote_op.author = "bob";
      vote_op.weight = PERCENT_100;
      tx.operations.push_back( vote_op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time - fc::seconds( BLOCK_INTERVAL ), true );

      auto gpo = db.get_dynamic_global_properties();
      uint128_t TMEreward = gpo.total_reward_fund.amount.value + ASSET( "2.000 TESTS" ).amount.value;
      uint128_t total_reward_squared = gpo.total_reward_squared;

      auto alice_comment = db.get_comment( "alice", string( "test" ) );
      auto bob_comment = db.get_comment( "bob", string( "test" ) );
      auto sam_comment = db.get_comment( "sam", string( "test" ) );
      auto dave_comment = db.get_comment( "dave", string( "test" ) );

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      // Calculate total comment rewards and voting rewards.
      auto alice_comment_reward = ( ( TMEreward * alice_comment.net_reward.value * alice_comment.net_reward.value ) / total_reward_squared ).to_uint64();
      total_reward_squared -= uint128_t( alice_comment.net_reward.value ) * ( alice_comment.net_reward.value );
      TMEreward -= alice_comment_reward;
      auto alice_comment_vote_rewards = alice_comment_reward / 2;
      alice_comment_reward -= alice_comment_vote_rewards;

      auto alice_vote_alice_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * alice_comment_vote_rewards ) / alice_comment.total_vote_weight ), SYMBOL_COIN );
      auto bob_vote_alice_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * alice_comment_vote_rewards ) / alice_comment.total_vote_weight ), SYMBOL_COIN );
      TMEreward += alice_comment_vote_rewards - ( alice_vote_alice_reward + bob_vote_alice_reward ).amount.value;

      auto bob_comment_reward = ( ( TMEreward * bob_comment.net_reward.value * bob_comment.net_reward.value ) / total_reward_squared ).to_uint64();
      total_reward_squared -= uint128_t( bob_comment.net_reward.value ) * bob_comment.net_reward.value;
      TMEreward -= bob_comment_reward;
      auto bob_comment_vote_rewards = bob_comment_reward / 2;
      bob_comment_reward -= bob_comment_vote_rewards;

      auto alice_vote_bob_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * bob_comment_vote_rewards ) / bob_comment.total_vote_weight ), SYMBOL_COIN );
      auto bob_vote_bob_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * bob_comment_vote_rewards ) / bob_comment.total_vote_weight ), SYMBOL_COIN );
      auto sam_vote_bob_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "sam" ) ).id ) )->weight ) * bob_comment_vote_rewards ) / bob_comment.total_vote_weight ), SYMBOL_COIN );
      TMEreward += bob_comment_vote_rewards - ( alice_vote_bob_reward + bob_vote_bob_reward + sam_vote_bob_reward ).amount.value;

      auto dave_comment_reward = ( ( TMEreward * dave_comment.net_reward.value * dave_comment.net_reward.value ) / total_reward_squared ).to_uint64();
      total_reward_squared -= uint128_t( dave_comment.net_reward.value ) * dave_comment.net_reward.value;
      TMEreward -= dave_comment_reward;
      auto dave_comment_vote_rewards = dave_comment_reward / 2;
      dave_comment_reward -= dave_comment_vote_rewards;

      auto bob_vote_dave_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "dave", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * dave_comment_vote_rewards ) / dave_comment.total_vote_weight ), SYMBOL_COIN );
      TMEreward += dave_comment_vote_rewards - bob_vote_dave_reward.amount.value;

      // Calculate rewards paid to parent posts
      auto alice_pays_alice_USD = alice_comment_reward / 2;
      auto alice_pays_alice_SCORE = alice_comment_reward - alice_pays_alice_USD;
      auto bob_pays_bob_USD = bob_comment_reward / 2;
      auto bob_pays_bob_SCORE = bob_comment_reward - bob_pays_bob_USD;
      auto dave_pays_dave_USD = dave_comment_reward / 2;
      auto dave_pays_dave_SCORE = dave_comment_reward - dave_pays_dave_USD;

      auto bob_pays_alice_USD = bob_pays_bob_USD / 2;
      auto bob_pays_alice_SCORE = bob_pays_bob_SCORE / 2;
      bob_pays_bob_USD -= bob_pays_alice_USD;
      bob_pays_bob_SCORE -= bob_pays_alice_SCORE;

      auto dave_pays_sam_USD = dave_pays_dave_USD / 2;
      auto dave_pays_sam_SCORE = dave_pays_dave_SCORE / 2;
      dave_pays_dave_USD -= dave_pays_sam_USD;
      dave_pays_dave_SCORE -= dave_pays_sam_SCORE;
      auto dave_pays_bob_USD = dave_pays_sam_USD / 2;
      auto dave_pays_bob_SCORE = dave_pays_sam_SCORE / 2;
      dave_pays_sam_USD -= dave_pays_bob_USD;
      dave_pays_sam_SCORE -= dave_pays_bob_SCORE;
      auto dave_pays_alice_USD = dave_pays_bob_USD / 2;
      auto dave_pays_alice_SCORE = dave_pays_bob_SCORE / 2;
      dave_pays_bob_USD -= dave_pays_alice_USD;
      dave_pays_bob_SCORE -= dave_pays_alice_SCORE;

      // Calculate total comment payouts
      auto alice_comment_total_payout = db.asset_to_USD( asset( alice_pays_alice_USD + alice_pays_alice_SCORE, SYMBOL_COIN ) );
      alice_comment_total_payout += db.asset_to_USD( asset( bob_pays_alice_USD + bob_pays_alice_SCORE, SYMBOL_COIN ) );
      alice_comment_total_payout += db.asset_to_USD( asset( dave_pays_alice_USD + dave_pays_alice_SCORE, SYMBOL_COIN ) );
      auto bob_comment_total_payout = db.asset_to_USD( asset( bob_pays_bob_USD + bob_pays_bob_SCORE, SYMBOL_COIN ) );
      bob_comment_total_payout += db.asset_to_USD( asset( dave_pays_bob_USD + dave_pays_bob_SCORE, SYMBOL_COIN ) );
      auto sam_comment_total_payout = db.asset_to_USD( asset( dave_pays_sam_USD + dave_pays_sam_SCORE, SYMBOL_COIN ) );
      auto dave_comment_total_payout = db.asset_to_USD( asset( dave_pays_dave_USD + dave_pays_dave_SCORE, SYMBOL_COIN ) );

      auto alice_starting_TME_fund_for_SCORE = db.get_account( "alice" ).SCORE;
      auto alice_starting_USD = db.get_account( "alice" ).USDbalance;
      auto bob_starting_TME_fund_for_SCORE = db.get_account( "bob" ).SCORE;
      auto bob_starting_USD = db.get_account( "bob" ).USDbalance;
      auto sam_starting_TME_fund_for_SCORE = db.get_account( "sam" ).SCORE;
      auto sam_starting_USD = db.get_account( "sam" ).USDbalance;
      auto dave_starting_TME_fund_for_SCORE = db.get_account( "dave" ).SCORE;
      auto dave_starting_USD = db.get_account( "dave" ).USDbalance;

      generate_block();

      gpo = db.get_dynamic_global_properties();

      // Calculate SCORE rewards from voting.
      auto alice_vote_alice_TME_fund_for_SCORE = alice_vote_alice_reward * gpo.get_SCORE_price();
      auto bob_vote_alice_TME_fund_for_SCORE = bob_vote_alice_reward * gpo.get_SCORE_price();
      auto alice_vote_bob_TME_fund_for_SCORE = alice_vote_bob_reward * gpo.get_SCORE_price();
      auto bob_vote_bob_TME_fund_for_SCORE = bob_vote_bob_reward * gpo.get_SCORE_price();
      auto sam_vote_bob_TME_fund_for_SCORE = sam_vote_bob_reward * gpo.get_SCORE_price();
      auto bob_vote_dave_TME_fund_for_SCORE = bob_vote_dave_reward * gpo.get_SCORE_price();

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).total_payout_value.amount.value == alice_comment_total_payout.amount.value );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).total_payout_value.amount.value == bob_comment_total_payout.amount.value );
      BOOST_REQUIRE( db.get_comment( "sam", string( "test" ) ).total_payout_value.amount.value == sam_comment_total_payout.amount.value );
      BOOST_REQUIRE( db.get_comment( "dave", string( "test" ) ).total_payout_value.amount.value == dave_comment_total_payout.amount.value );

      // ops 0-3, 5-6, and 10 are comment reward ops
      auto ops = get_last_operations( 13 );

      BOOST_TEST_MESSAGE( "Checking Virtual Operation Correctness" );

      curate_reward_operation cur_vop;
      comment_reward_operation com_vop = ops[0].get< comment_reward_operation >();

      BOOST_REQUIRE( com_vop.author == "alice" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "dave" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_alice_USD );
      BOOST_REQUIRE( ( com_vop.SCOREpayout * gpo.get_SCORE_price() ).amount.value == dave_pays_alice_SCORE );

      com_vop = ops[1].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "bob" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "dave" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_bob_USD );
      BOOST_REQUIRE( ( com_vop.SCOREpayout * gpo.get_SCORE_price() ).amount.value == dave_pays_bob_SCORE );

      com_vop = ops[2].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "sam" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "dave" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_sam_USD );
      BOOST_REQUIRE( ( com_vop.SCOREpayout * gpo.get_SCORE_price() ).amount.value == dave_pays_sam_SCORE );

      com_vop = ops[3].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "dave" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "dave" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_dave_USD );
      BOOST_REQUIRE( ( com_vop.SCOREpayout * gpo.get_SCORE_price() ).amount.value == dave_pays_dave_SCORE );

      cur_vop = ops[4].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "bob" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == bob_vote_dave_TME_fund_for_SCORE.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "dave" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      com_vop = ops[5].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "alice" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "bob" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == bob_pays_alice_USD );
      BOOST_REQUIRE( ( com_vop.SCOREpayout * gpo.get_SCORE_price() ).amount.value == bob_pays_alice_SCORE );

      com_vop = ops[6].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "bob" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "bob" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == bob_pays_bob_USD );
      BOOST_REQUIRE( ( com_vop.SCOREpayout * gpo.get_SCORE_price() ).amount.value == bob_pays_bob_SCORE );

      cur_vop = ops[7].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "sam" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == sam_vote_bob_TME_fund_for_SCORE.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "bob" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      cur_vop = ops[8].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "bob" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == bob_vote_bob_TME_fund_for_SCORE.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "bob" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      cur_vop = ops[9].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "alice" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == alice_vote_bob_TME_fund_for_SCORE.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "bob" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      com_vop = ops[10].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "alice" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "alice" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == alice_pays_alice_USD );
      BOOST_REQUIRE( ( com_vop.SCOREpayout * gpo.get_SCORE_price() ).amount.value == alice_pays_alice_SCORE );

      cur_vop = ops[11].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "bob" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == bob_vote_alice_TME_fund_for_SCORE.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "alice" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      cur_vop = ops[12].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "alice" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == alice_vote_alice_TME_fund_for_SCORE.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "alice" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      BOOST_TEST_MESSAGE( "Checking account balances" );

      auto alice_USDtotal = alice_starting_USD + asset( alice_pays_alice_USD + bob_pays_alice_USD + dave_pays_alice_USD, SYMBOL_COIN ) * exchange_rate;
      auto alice_totalSCORE = alice_starting_TME_fund_for_SCORE + asset( alice_pays_alice_SCORE + bob_pays_alice_SCORE + dave_pays_alice_SCORE + alice_vote_alice_reward.amount + alice_vote_bob_reward.amount, SYMBOL_COIN ) * gpo.get_SCORE_price();
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance.amount.value == alice_USDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "alice" ).SCORE.amount.value == alice_totalSCORE.amount.value );

      auto bob_USDtotal = bob_starting_USD + asset( bob_pays_bob_USD + dave_pays_bob_USD, SYMBOL_COIN ) * exchange_rate;
      auto bob_totalSCORE = bob_starting_TME_fund_for_SCORE + asset( bob_pays_bob_SCORE + dave_pays_bob_SCORE + bob_vote_alice_reward.amount + bob_vote_bob_reward.amount + bob_vote_dave_reward.amount, SYMBOL_COIN ) * gpo.get_SCORE_price();
      BOOST_REQUIRE( db.get_account( "bob" ).USDbalance.amount.value == bob_USDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).SCORE.amount.value == bob_totalSCORE.amount.value );

      auto sam_USDtotal = sam_starting_USD + asset( dave_pays_sam_USD, SYMBOL_COIN ) * exchange_rate;
      auto sam_totalSCORE = bob_starting_TME_fund_for_SCORE + asset( dave_pays_sam_SCORE + sam_vote_bob_reward.amount, SYMBOL_COIN ) * gpo.get_SCORE_price();
      BOOST_REQUIRE( db.get_account( "sam" ).USDbalance.amount.value == sam_USDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).SCORE.amount.value == sam_totalSCORE.amount.value );

      auto dave_USDtotal = dave_starting_USD + asset( dave_pays_dave_USD, SYMBOL_COIN ) * exchange_rate;
      auto dave_totalSCORE = dave_starting_TME_fund_for_SCORE + asset( dave_pays_dave_SCORE, SYMBOL_COIN ) * gpo.get_SCORE_price();
      BOOST_REQUIRE( db.get_account( "dave" ).USDbalance.amount.value == dave_USDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).SCORE.amount.value == dave_totalSCORE.amount.value );
   }
   FC_LOG_AND_RETHROW()
}
*/


BOOST_AUTO_TEST_CASE( TME_fund_for_SCORE_withdrawals )
{
   try
   {
      ACTORS( (alice) )
      fund( "alice", 100000 );
      score( "alice", 100000 );

      const auto& new_alice = db.get_account( "alice" );

      BOOST_TEST_MESSAGE( "Setting up withdrawal" );

      signed_transaction tx;
      unstake_asset_operation op;
      op.account = "alice";
      op.SCORE = asset( new_alice.SCORE.amount / 2, SYMBOL_TP );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto next_withdrawal = db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS;
      asset SCORE = new_alice.SCORE;
      asset to_withdraw = op.SCORE;
      asset original_TME_fund_for_SCORE = SCORE;
      asset withdraw_rate = new_alice.SCOREwithdrawRateInTME;

      BOOST_TEST_MESSAGE( "Generating block up to first withdrawal" );
      generate_blocks( next_withdrawal - ( BLOCK_INTERVAL / 2 ), true);

      BOOST_REQUIRE( db.get_account( "alice" ).SCORE.amount.value == SCORE.amount.value );

      BOOST_TEST_MESSAGE( "Generating block to cause withdrawal" );
      generate_block();

   
      auto gpo = db.get_dynamic_global_properties();

      BOOST_REQUIRE( db.get_account( "alice" ).SCORE.amount.value == ( SCORE - withdraw_rate ).amount.value );
      BOOST_REQUIRE( ( withdraw_rate * gpo.get_SCORE_price() ).amount.value - db.get_account( "alice" ).balance.amount.value <= 1 ); // Check a range due to differences in the share price
      BOOST_REQUIRE( fill_op.from_account == "alice" );
      BOOST_REQUIRE( fill_op.to_account == "alice" );
      BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
      BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_SCORE_price() ).amount.value ) <= 1 );
      validate_database();

      BOOST_TEST_MESSAGE( "Generating the rest of the blocks in the withdrawal" );

      SCORE = db.get_account( "alice" ).SCORE;
      auto balance = db.get_account( "alice" ).balance;
      auto old_next_TME_fund_for_SCORE = db.get_account( "alice" ).next_unstake_time;

      for( int i = 1; i < COIN_UNSTAKE_INTERVALS - 1; i++ )
      {
         generate_blocks( db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS );

         const auto& alice = db.get_account( "alice" );

         gpo = db.get_dynamic_global_properties();

         BOOST_REQUIRE( alice.SCORE.amount.value == ( SCORE - withdraw_rate ).amount.value );
         BOOST_REQUIRE( balance.amount.value + ( withdraw_rate * gpo.get_SCORE_price() ).amount.value - alice.balance.amount.value <= 1 );
         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_SCORE_price() ).amount.value ) <= 1 );

         if ( i == COIN_UNSTAKE_INTERVALS - 1 )
            BOOST_REQUIRE( alice.next_unstake_time == fc::time_point::maximum() );
         else
            BOOST_REQUIRE( alice.next_unstake_time.sec_since_epoch() == ( old_next_TME_fund_for_SCORE + SCORE_WITHDRAW_INTERVAL_SECONDS ).sec_since_epoch() );

         validate_database();

         SCORE = alice.SCORE;
         balance = alice.balance;
         old_next_TME_fund_for_SCORE = alice.next_unstake_time;
      }

      if (  to_withdraw.amount.value % withdraw_rate.amount.value != 0 )
      {
         BOOST_TEST_MESSAGE( "Generating one more block to take care of remainder" );
         generate_blocks( db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS, true );
         gpo = db.get_dynamic_global_properties();

         BOOST_REQUIRE( db.get_account( "alice" ).next_unstake_time.sec_since_epoch() == ( old_next_TME_fund_for_SCORE + SCORE_WITHDRAW_INTERVAL_SECONDS ).sec_since_epoch() );
         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_SCORE_price() ).amount.value ) <= 1 );

         generate_blocks( db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS, true );
         gpo = db.get_dynamic_global_properties();

         BOOST_REQUIRE( db.get_account( "alice" ).next_unstake_time.sec_since_epoch() == fc::time_point::maximum().sec_since_epoch() );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == to_withdraw.amount.value % withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_SCORE_price() ).amount.value ) <= 1 );

         validate_database();
      }
      else
      {
         generate_blocks( db.head_block_time() + SCORE_WITHDRAW_INTERVAL_SECONDS, true );

         BOOST_REQUIRE( db.get_account( "alice" ).next_unstake_time.sec_since_epoch() == fc::time_point::maximum().sec_since_epoch() );

         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_SCORE_price() ).amount.value ) <= 1 );
      }

      BOOST_REQUIRE( db.get_account( "alice" ).SCORE.amount.value == ( original_TME_fund_for_SCORE - op.SCORE ).amount.value );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( TME_fund_for_SCORE_withdraw_route )
{
   try
   {
      ACTORS( (alice)(bob)(sam) )

      auto original_TME_fund_for_SCORE = alice.SCORE;

      fund( "alice", 1040000 );
      score( "alice", 1040000 );

      auto withdraw_amount = alice.SCORE - original_TME_fund_for_SCORE;

      BOOST_TEST_MESSAGE( "Setup SCORE withdraw" );
      unstake_asset_operation wv;
      wv.account = "alice";
      wv.SCORE = withdraw_amount;

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( wv );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "Setting up bob destination" );
      unstake_asset_route_operation op;
      op.from_account = "alice";
      op.to_account = "bob";
      op.percent = PERCENT_1 * 50;
      op.auto_stake = true;
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "Setting up sam destination" );
      op.to_account = "sam";
      op.percent = PERCENT_1 * 30;
      op.auto_stake = false;
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Setting up first withdraw" );

      auto SCOREwithdrawRateInTME = alice.SCOREwithdrawRateInTME;
      auto old_alice_balance = alice.balance;
      auto old_alice_TME_fund_for_SCORE = alice.SCORE;
      auto old_bob_balance = bob.balance;
      auto old_bob_TME_fund_for_SCORE = bob.SCORE;
      auto old_sam_balance = sam.balance;
      auto old_sam_TME_fund_for_SCORE = sam.SCORE;
      generate_blocks( alice.next_unstake_time, true );

      {
         const auto& alice = db.get_account( "alice" );
         const auto& bob = db.get_account( "bob" );
         const auto& sam = db.get_account( "sam" );

         BOOST_REQUIRE( alice.SCORE == old_alice_TME_fund_for_SCORE - SCOREwithdrawRateInTME );
         BOOST_REQUIRE( alice.balance == old_alice_balance + asset( ( SCOREwithdrawRateInTME.amount * PERCENT_1 * 20 ) / PERCENT_100, SYMBOL_TP ) * db.get_dynamic_global_properties().get_SCORE_price() );
         BOOST_REQUIRE( bob.SCORE == old_bob_TME_fund_for_SCORE + asset( ( SCOREwithdrawRateInTME.amount * PERCENT_1 * 50 ) / PERCENT_100, SYMBOL_TP ) );
         BOOST_REQUIRE( bob.balance == old_bob_balance );
         BOOST_REQUIRE( sam.SCORE == old_sam_TME_fund_for_SCORE );
         BOOST_REQUIRE( sam.balance ==  old_sam_balance + asset( ( SCOREwithdrawRateInTME.amount * PERCENT_1 * 30 ) / PERCENT_100, SYMBOL_TP ) * db.get_dynamic_global_properties().get_SCORE_price() );

         old_alice_balance = alice.balance;
         old_alice_TME_fund_for_SCORE = alice.SCORE;
         old_bob_balance = bob.balance;
         old_bob_TME_fund_for_SCORE = bob.SCORE;
         old_sam_balance = sam.balance;
         old_sam_TME_fund_for_SCORE = sam.SCORE;
      }

      BOOST_TEST_MESSAGE( "Test failure with greater than 100% destination assignment" );

      tx.operations.clear();
      tx.signatures.clear();

      op.to_account = "sam";
      op.percent = PERCENT_1 * 50 + 1;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "Test from_account receiving no withdraw" );

      tx.operations.clear();
      tx.signatures.clear();

      op.to_account = "sam";
      op.percent = PERCENT_1 * 50;
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_account( "alice" ).next_unstake_time, true );
      {
         const auto& alice = db.get_account( "alice" );
         const auto& bob = db.get_account( "bob" );
         const auto& sam = db.get_account( "sam" );

         BOOST_REQUIRE( alice.SCORE == old_alice_TME_fund_for_SCORE - SCOREwithdrawRateInTME );
         BOOST_REQUIRE( alice.balance == old_alice_balance );
         BOOST_REQUIRE( bob.SCORE == old_bob_TME_fund_for_SCORE + asset( ( SCOREwithdrawRateInTME.amount * PERCENT_1 * 50 ) / PERCENT_100, SYMBOL_TP ) );
         BOOST_REQUIRE( bob.balance == old_bob_balance );
         BOOST_REQUIRE( sam.SCORE == old_sam_TME_fund_for_SCORE );
         BOOST_REQUIRE( sam.balance ==  old_sam_balance + asset( ( SCOREwithdrawRateInTME.amount * PERCENT_1 * 50 ) / PERCENT_100, SYMBOL_TP ) * db.get_dynamic_global_properties().get_SCORE_price() );
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( feed_publish_mean )
{
   try
   {
      resize_shared_mem( 1024 * 1024 * 32 );

      ACTORS( (alice0)(alice1)(alice2)(alice3)(alice4)(alice5)(alice6) )

      BOOST_TEST_MESSAGE( "Setup" );

      generate_blocks( 30 / BLOCK_INTERVAL );

      vector< string > accounts;
      accounts.push_back( "alice0" );
      accounts.push_back( "alice1" );
      accounts.push_back( "alice2" );
      accounts.push_back( "alice3" );
      accounts.push_back( "alice4" );
      accounts.push_back( "alice5" );
      accounts.push_back( "alice6" );

      vector< private_key_type > keys;
      keys.push_back( alice0_private_key );
      keys.push_back( alice1_private_key );
      keys.push_back( alice2_private_key );
      keys.push_back( alice3_private_key );
      keys.push_back( alice4_private_key );
      keys.push_back( alice5_private_key );
      keys.push_back( alice6_private_key );

      vector< feed_publish_operation > ops;
      vector< signed_transaction > txs;

      // Upgrade accounts to witnesses
      for( int i = 0; i < 7; i++ )
      {
         transfer( GENESIS_ACCOUNT_BASE_NAME, accounts[i], 10000 );
         witness_create( accounts[i], keys[i], "foo.bar", keys[i].get_public_key(), 1000 );

         ops.push_back( feed_publish_operation() );
         ops[i].publisher = accounts[i];

         txs.push_back( signed_transaction() );
      }

      ops[0].exchange_rate = price( asset( 100000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );
      ops[1].exchange_rate = price( asset( 105000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );
      ops[2].exchange_rate = price( asset(  98000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );
      ops[3].exchange_rate = price( asset(  97000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );
      ops[4].exchange_rate = price( asset(  99000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );
      ops[5].exchange_rate = price( asset(  97500, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );
      ops[6].exchange_rate = price( asset( 102000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );

      for( int i = 0; i < 7; i++ )
      {
         txs[i].set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
         txs[i].operations.push_back( ops[i] );
         txs[i].sign( keys[i], db.get_chain_id() );
         db.push_transaction( txs[i], 0 );
      }

      BOOST_TEST_MESSAGE( "Jump forward an hour" );

      generate_blocks( BLOCKS_PER_HOUR ); // Jump forward 1 hour
      BOOST_TEST_MESSAGE( "Get feed history object" );
      feed_history_object feed_history = db.get_feed_history();
      BOOST_TEST_MESSAGE( "Check state" );
      BOOST_REQUIRE( feed_history.current_median_history == price( asset( 99000, SYMBOL_COIN), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( feed_history.price_history[ 0 ] == price( asset( 99000, SYMBOL_COIN), asset( 1000, SYMBOL_USD ) ) );
      validate_database();

      for ( int i = 0; i < 23; i++ )
      {
         BOOST_TEST_MESSAGE( "Updating ops" );

         for( int j = 0; j < 7; j++ )
         {
            txs[j].operations.clear();
            txs[j].signatures.clear();
            ops[j].exchange_rate = price( ops[j].exchange_rate.base, asset( ops[j].exchange_rate.quote.amount + 10, SYMBOL_USD ) );
            txs[j].set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
            txs[j].operations.push_back( ops[j] );
            txs[j].sign( keys[j], db.get_chain_id() );
            db.push_transaction( txs[j], 0 );
         }

         BOOST_TEST_MESSAGE( "Generate Blocks" );

         generate_blocks( BLOCKS_PER_HOUR  ); // Jump forward 1 hour

         BOOST_TEST_MESSAGE( "Check feed_history" );

         feed_history = db.get(feed_history_id_type());
         BOOST_REQUIRE( feed_history.current_median_history == feed_history.price_history[ ( i + 1 ) / 2 ] );
         BOOST_REQUIRE( feed_history.price_history[ i + 1 ] == ops[4].exchange_rate );
         validate_database();
      }
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( USD_interest )
{
   try
   {
      ACTORS( (alice)(bob) )
      generate_block();
      score( "alice", ASSET( "10.000 TESTS" ) );
      score( "bob", ASSET( "10.000 TESTS" ) );

      set_price_feed( price( asset::from_string( "1.000 TESTS" ), asset::from_string( "1.000 USD" ) ) );

      BOOST_TEST_MESSAGE( "Testing interest over smallest interest period" );

      signed_transaction tx;

      fund( "alice", ASSET( "31.903 USD" ) );

      auto start_time = db.get_account( "alice" ).USD_seconds_last_update;
      auto alice_USD = db.get_account( "alice" ).USDbalance;

      generate_blocks( db.head_block_time() + fc::seconds( USD_INTEREST_COMPOUND_INTERVAL_SEC ), true );

      transfer_operation transfer;
      transfer.to = "bob";
      transfer.from = "alice";
      transfer.amount = ASSET( "1.000 USD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( transfer );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto gpo = db.get_dynamic_global_properties();
      auto interest_op = get_last_operations( 1 )[0].get< interest_operation >();

      BOOST_REQUIRE( gpo.credit_interest_rate > 0 );
      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance.amount.value == alice_USD.amount.value - ASSET( "1.000 USD" ).amount.value + ( ( ( ( uint128_t( alice_USD.amount.value ) * ( db.head_block_time() - start_time ).to_seconds() ) / SECONDS_PER_YEAR ) * gpo.credit_interest_rate ) / PERCENT_100 ).to_uint64() );
      BOOST_REQUIRE( interest_op.owner == "alice" );
      BOOST_REQUIRE( interest_op.interest.amount.value == db.get_account( "alice" ).USDbalance.amount.value - ( alice_USD.amount.value - ASSET( "1.000 USD" ).amount.value ) );
      validate_database();

      BOOST_TEST_MESSAGE( "Testing interest under interest period" );

      start_time = db.get_account( "alice" ).USD_seconds_last_update;
      alice_USD = db.get_account( "alice" ).USDbalance;

      generate_blocks( db.head_block_time() + fc::seconds( USD_INTEREST_COMPOUND_INTERVAL_SEC / 2 ), true );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance.amount.value == alice_USD.amount.value - ASSET( "1.000 USD" ).amount.value );
      validate_database();

      auto alice_coindays = uint128_t( alice_USD.amount.value ) * ( db.head_block_time() - start_time ).to_seconds();
      alice_USD = db.get_account( "alice" ).USDbalance;
      start_time = db.get_account( "alice" ).USD_seconds_last_update;

      BOOST_TEST_MESSAGE( "Testing longer interest period" );

      generate_blocks( db.head_block_time() + fc::seconds( ( USD_INTEREST_COMPOUND_INTERVAL_SEC * 7 ) / 3 ), true );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance.amount.value == alice_USD.amount.value - ASSET( "1.000 USD" ).amount.value + ( ( ( ( uint128_t( alice_USD.amount.value ) * ( db.head_block_time() - start_time ).to_seconds() + alice_coindays ) / SECONDS_PER_YEAR ) * gpo.credit_interest_rate ) / PERCENT_100 ).to_uint64() );
      validate_database();
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( post_rate_limit )
{
   try
   {
      ACTORS( (alice) )

      fund( "alice", 10000 );
      score( "alice", 10000 );

      comment_operation op;
      op.author = "alice";
      op.permlink = "test1";
      op.parent_author = "";
      op.parent_permlink = "test";
      op.body = "test";

      signed_transaction tx;

      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      op.permlink = "test2";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      tx.operations.clear();
      tx.signatures.clear();

      op.permlink = "test3";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      tx.operations.clear();
      tx.signatures.clear();

      op.permlink = "test4";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      tx.operations.clear();
      tx.signatures.clear();

      op.permlink = "test5";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_freeze )
{
   try
   {
      ACTORS( (alice)(bob)(sam)(dave) )
      fund( "alice", 10000 );
      fund( "bob", 10000 );
      fund( "sam", 10000 );
      fund( "dave", 10000 );

      score( "alice", 10000 );
      score( "bob", 10000 );
      score( "sam", 10000 );
      score( "dave", 10000 );

      auto exchange_rate = price( ASSET( "1.250 TESTS" ), ASSET( "1.000 USD" ) );
      set_price_feed( exchange_rate );

      signed_transaction tx;

      comment_operation comment;
      comment.author = "alice";
      comment.parent_author = "";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.body = "test";

      tx.operations.push_back( comment );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment.body = "test2";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      vote_operation vote;
      vote.weight = PERCENT_100;
      vote.voter = "bob";
      vote.author = "alice";
      vote.permlink = "test";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).last_payout == fc::time_point::min() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time != fc::time_point::min() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time != fc::time_point::maximum() );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time, true );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).last_payout == db.head_block_time() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point::maximum() );

      vote.voter = "sam";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point::maximum() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value == 0 );

      vote.voter = "bob";
      vote.weight = PERCENT_100 * -1;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point::maximum() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value == 0 );

      vote.voter = "dave";
      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( dave_private_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point::maximum() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_reward.value == 0 );

      comment.body = "test4";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( alice_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
   }
   FC_LOG_AND_RETHROW()
}

// This test is too intensive without optimizations. Disable it when we build in debug
#ifndef DEBUG
BOOST_AUTO_TEST_CASE( USD_stability )
{
   try
   {
      resize_shared_mem( 1024 * 1024 * 512 ); // Due to number of blocks in the test, it requires a large file. (64 MB)

      // Using the debug node plugin to manually set account balances to create required market conditions for this test
      auto db_plugin = app.register_plugin< node::plugin::debug_node::debug_node_plugin >();
      boost::program_options::variables_map options;
      db_plugin->logging = false;
      db_plugin->plugin_initialize( options );
      db_plugin->plugin_startup();
      auto debug_key = "5JdouSvkK75TKWrJixYufQgePT21V7BAVWbNUWt3ktqhPmy8Z78"; //get_dev_key debug node

      ACTORS( (alice)(bob)(sam)(dave)(greg) );

      fund( "alice", 10000 );
      fund( "bob", 10000 );

      score( "alice", 10000 );
      score( "bob", 10000 );

      auto exchange_rate = price( ASSET( "1.000 USD" ), ASSET( "10.000 TESTS" ) );
      set_price_feed( exchange_rate );

      BOOST_REQUIRE( db.get_dynamic_global_properties().USD_print_rate == PERCENT_100 );

      comment_operation comment;
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";

      signed_transaction tx;
      tx.operations.push_back( comment );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      vote_operation vote;
      vote.voter = "bob";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = PERCENT_100;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Generating blocks up to comment payout" );

      db_plugin->debug_generate_blocks_until( debug_key, fc::time_point( db.get_comment( comment.author, comment.permlink ).cashout_time.sec_since_epoch() - 2 * BLOCK_INTERVAL ), true, database::skip_witness_signature );

      auto& gpo = db.get_dynamic_global_properties();

      BOOST_TEST_MESSAGE( "Changing sam and gpo to set up market cap conditions" );

      asset USDbalance = asset( ( gpo.virtual_supply.amount * ( USD_STOP_PERCENT + 30 ) ) / PERCENT_100, SYMBOL_COIN ) * exchange_rate;
      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_account( "sam" ), [&]( account_object& a )
         {
            a.USDbalance = USDbalance;
         });
      }, database::skip_witness_signature );

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& gpo )
         {
            gpo.current_USD_supply = USDbalance;
            gpo.virtual_supply = gpo.virtual_supply + USDbalance * exchange_rate;
         });
      }, database::skip_witness_signature );

      validate_database();

      db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );

      auto comment_reward = ( gpo.total_reward_fund.amount + 2000 ) - ( ( gpo.total_reward_fund.amount + 2000 ) * 25 * PERCENT_1 ) / PERCENT_100 ;
      comment_reward /= 2;
      auto USDreward = ( comment_reward * gpo.USD_print_rate ) / PERCENT_100;
      auto alice_USD = db.get_account( "alice" ).USDbalance + db.get_account( "alice" ).USDrewardBalance + asset( USDreward, SYMBOL_COIN ) * exchange_rate;
      auto alice_TME = db.get_account( "alice" ).balance + db.get_account( "alice" ).TMErewardBalance ;

      BOOST_TEST_MESSAGE( "Checking printing USD has slowed" );
      BOOST_REQUIRE( db.get_dynamic_global_properties().USD_print_rate < PERCENT_100 );

      BOOST_TEST_MESSAGE( "Pay out comment and check rewards are paid as TME" );
      db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );

      validate_database();

      BOOST_REQUIRE( db.get_account( "alice" ).USDbalance + db.get_account( "alice" ).USDrewardBalance == alice_USD );
      BOOST_REQUIRE( db.get_account( "alice" ).balance + db.get_account( "alice" ).TMErewardBalance > alice_TME );

      BOOST_TEST_MESSAGE( "Letting percent market cap fall to 2% to verify printing of USD turns back on" );

      // Get close to 1.5% for printing USD to start again, but not all the way
      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_account( "sam" ), [&]( account_object& a )
         {
            a.USDbalance = asset( ( 194 * USDbalance.amount ) / 500, SYMBOL_USD );
         });
      }, database::skip_witness_signature );

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& gpo )
         {
            gpo.current_USD_supply = alice_USD + asset( ( 194 * USDbalance.amount ) / 500, SYMBOL_USD );
         });
      }, database::skip_witness_signature );

      db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );
      validate_database();

      BOOST_REQUIRE( db.get_dynamic_global_properties().USD_print_rate < PERCENT_100 );

      auto last_print_rate = db.get_dynamic_global_properties().USD_print_rate;

      // Keep producing blocks until printing USD is back
      while( ( db.get_dynamic_global_properties().current_USD_supply * exchange_rate ).amount >= ( db.get_dynamic_global_properties().virtual_supply.amount * USD_START_PERCENT ) / PERCENT_100 )
      {
         auto& gpo = db.get_dynamic_global_properties();
         BOOST_REQUIRE( gpo.USD_print_rate >= last_print_rate );
         last_print_rate = gpo.USD_print_rate;
         db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );
         validate_database();
      }

      validate_database();

      BOOST_REQUIRE( db.get_dynamic_global_properties().USD_print_rate == PERCENT_100 );
   }
   FC_LOG_AND_RETHROW()
}
#endif

BOOST_AUTO_TEST_CASE( USD_price_feed_limit )
{
   try
   {
      ACTORS( (alice) );
      generate_block();
      score( "alice", ASSET( "10.000 TESTS" ) );

      price exchange_rate( ASSET( "1.000 USD" ), ASSET( "1.000 TESTS" ) );
      set_price_feed( exchange_rate );

      comment_operation comment;
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";

      vote_operation vote;
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = PERCENT_100;

      signed_transaction tx;
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time, true );

      BOOST_TEST_MESSAGE( "Setting USD percent to greater than 10% market cap." );

      db.skip_price_feed_limit_check = false;
      const auto& gpo = db.get_dynamic_global_properties();
      set_price_feed( new_exchange_rate );
      set_price_feed( new_exchange_rate );

      BOOST_REQUIRE( db.get_feed_history().current_median_history > new_exchange_rate && db.get_feed_history().current_median_history < exchange_rate );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
#endif
