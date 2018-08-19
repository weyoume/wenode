#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <eznode/protocol/exceptions.hpp>

#include <eznode/chain/block_summary_object.hpp>
#include <eznode/chain/database.hpp>
#include <eznode/chain/hardfork.hpp>
#include <eznode/chain/history_object.hpp>
#include <eznode/chain/eznode_objects.hpp>

#include <eznode/chain/util/reward.hpp>

#include <eznode/plugins/debug_node/debug_node_plugin.hpp>

#include <fc/crypto/digest.hpp>

#include "../common/database_fixture.hpp"

#include <cmath>

using namespace eznode;
using namespace eznode::chain;
using namespace eznode::protocol;

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
      authors.emplace_back( "bob"  , bob_private_key, ASSET( "0.000 TBD" ) );
      authors.emplace_back( "dave" , dave_private_key );
      voters.emplace_back( "ulysses", ulysses_private_key, "alice");
      voters.emplace_back( "vivian" , vivian_private_key , "bob"  );
      voters.emplace_back( "wendy"  , wendy_private_key  , "dave" );

      // A,B,D : posters
      // U,V,W : voters

      // set a ridiculously high ECO price ($1 / satoshi) to disable dust threshold
      set_price_feed( price( ASSET( "0.001 TESTS" ), ASSET( "1.000 TBD" ) ) );

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

      auto ECOreward = db.get_dynamic_global_properties().total_reward_fund_ECO;

      // generate a few blocks to seed the reward fund
      generate_blocks(10);
      //const auto& rf = db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME );
      //idump( (rf) );

      generate_blocks( db.get_comment( "alice", string( "mypost" ) ).cashout_time, true );
      /*
      for( const auto& author : authors )
      {
         const account_object& a = db.get_account(author.name);
         ilog( "${n} : ${ECO} ${EUSD}", ("n", author.name)("ECO", a.ECOrewardBalance)("EUSD", a.EUSDrewardbalance) );
      }
      for( const auto& voter : voters )
      {
         const account_object& a = db.get_account(voter.name);
         ilog( "${n} : ${ECO} ${EUSD}", ("n", voter.name)("ECO", a.ECOrewardBalance)("EUSD", a.EUSDrewardbalance) );
      }
      */

      const account_object& alice_account = db.get_account("alice");
      const account_object& bob_account   = db.get_account("bob");
      const account_object& dave_account  = db.get_account("dave");

      BOOST_CHECK( alice_account.EUSDrewardbalance == ASSET( "14288.000 TBD" ) );
      BOOST_CHECK( bob_account.EUSDrewardbalance == ASSET( "0.000 TBD" ) );
      BOOST_CHECK( dave_account.EUSDrewardbalance == alice_account.EUSDrewardbalance );
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

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 TBD" ) ) );

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

      // If comments are paid out independent of order, then the last satoshi of ECO cannot be divided among them
      const auto rf = db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME );
      BOOST_REQUIRE( rf.reward_balance == ASSET( "0.001 TESTS" ) );

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

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 TBD" ) ) );
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
         const auto& post_rf = db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME );
         const auto& comment_rf = db.get< reward_fund_object, by_name >( COMMENT_REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.reward_balance.amount == 0 );
         BOOST_REQUIRE( comment_rf.reward_balance.amount > 0 );
         BOOST_REQUIRE( db.get_account( "alice" ).EUSDrewardbalance.amount > 0 );
         BOOST_REQUIRE( db.get_account( "bob" ).EUSDrewardbalance.amount == 0 );
         validate_database();
      }

      generate_blocks( db.get_comment( "bob", string( "test" ) ).cashout_time );

      {
         const auto& post_rf = db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME );
         const auto& comment_rf = db.get< reward_fund_object, by_name >( COMMENT_REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.reward_balance.amount > 0 );
         BOOST_REQUIRE( comment_rf.reward_balance.amount == 0 );
         BOOST_REQUIRE( db.get_account( "alice" ).EUSDrewardbalance.amount > 0 );
         BOOST_REQUIRE( db.get_account( "bob" ).EUSDrewardbalance.amount > 0 );
         validate_database();
      }
   }
   FC_LOG_AND_RETHROW()
}
*/

BOOST_AUTO_TEST_CASE( recent_claims_decay )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: recent_ESCORreward_2decay" );
      ACTORS( (alice)(bob) )
      generate_block();

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 TBD" ) ) );
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

      auto alice_vESCOR = util::evaluate_reward_curve( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value,
         db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME ).authorReward_curve,
         db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME ).content_constant );

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
         const auto& post_rf = db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.recent_claims == alice_vESCOR );
         validate_database();
      }

      auto bob_cashout_time = db.get_comment( "bob", string( "test" ) ).cashout_time;
      auto bob_vESCOR = util::evaluate_reward_curve( db.get_comment( "bob", string( "test" ) ).net_ESCORreward.value,
         db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME ).authorReward_curve,
         db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME ).content_constant );

      generate_block();

      while( db.head_block_time() < bob_cashout_time )
      {
         alice_vESCOR -= ( alice_vESCOR * BLOCK_INTERVAL ) / RECENT_RESCOR_DECAY_RATE_HF19.to_seconds();
         const auto& post_rf = db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.recent_claims == alice_vESCOR );

         generate_block();

      }

      {
         alice_vESCOR -= ( alice_vESCOR * BLOCK_INTERVAL ) / RECENT_RESCOR_DECAY_RATE_HF19.to_seconds();
         const auto& post_rf = db.get< reward_fund_object, by_name >( POST_REWARD_FUND_NAME );

         BOOST_REQUIRE( post_rf.recent_claims == alice_vESCOR + bob_vESCOR );
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

      price exchange_rate( ASSET( "1.000 TESTS" ), ASSET( "1.000 TBD" ) );
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

      auto ECOreward = db.get_dynamic_global_properties().total_reward_fund_ECO + ASSET( "1.667 TESTS" );
      auto total_ESCORreward2 = db.get_dynamic_global_properties().total_ESCORreward2;
      auto bob_comment_ESCORreward = db.get_comment( "bob", string( "test" ) ).net_ESCORreward;
      auto bob_ESCOR = db.get_account( "bob" ).ESCOR;
      auto bob_EUSDbalance = db.get_account( "bob" ).EUSDbalance;

      auto bob_comment_payout = asset( ( ( uint128_t( bob_comment_ESCORreward.value ) * bob_comment_ESCORreward.value * ECOreward.amount.value ) / total_ESCORreward2 ).to_uint64(), SYMBOL_ECO );
      auto bob_comment_discussion_rewards = asset( bob_comment_payout.amount / 4, SYMBOL_ECO );
      bob_comment_payout -= bob_comment_discussion_rewards;
      auto bob_comment_EUSDreward = db.to_EUSD( asset( bob_comment_payout.amount / 2, SYMBOL_ECO ) );
      auto bob_comment_ECO_fund_for_ESCOR_reward = ( bob_comment_payout - asset( bob_comment_payout.amount / 2, SYMBOL_ECO) ) * db.get_dynamic_global_properties().get_ESCOR_price();

      BOOST_TEST_MESSAGE( "Cause first payout" );

      generate_block();

      BOOST_REQUIRE( db.get_dynamic_global_properties().total_reward_fund_ECO == ECOreward - bob_comment_payout );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).total_payout_value == bob_comment_ECO_fund_for_ESCOR_reward * db.get_dynamic_global_properties().get_ESCOR_price() + bob_comment_EUSDreward * exchange_rate );
      BOOST_REQUIRE( db.get_account( "bob" ).ESCOR == bob_ESCOR + bob_comment_ECO_fund_for_ESCOR_reward );
      BOOST_REQUIRE( db.get_account( "bob" ).EUSDbalance == bob_EUSDbalance + bob_comment_EUSDreward );

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

      bob_ESCOR = db.get_account( "bob" ).ESCOR;
      bob_EUSDbalance = db.get_account( "bob" ).EUSDbalance;

      validate_database();

      generate_block();

      BOOST_REQUIRE( bob_ESCOR.amount.value == db.get_account( "bob" ).ESCOR.amount.value );
      BOOST_REQUIRE( bob_EUSDbalance.amount.value == db.get_account( "bob" ).EUSDbalance.amount.value );
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

      price exchange_rate( ASSET( "1.000 TESTS" ), ASSET( "1.000 TBD" ) );
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

      generate_blocks( fc::time_point_sec( db.head_block_time().sec_since_epoch() + CASHOUT_WINDOW_SECONDS / 2 ), true );

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
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value > 0 );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).net_ESCORreward.value > 0 );
      validate_database();

      auto ECOreward = db.get_dynamic_global_properties().total_reward_fund_ECO + ASSET( "2.000 TESTS" );
      auto total_ESCORreward2 = db.get_dynamic_global_properties().total_ESCORreward2;
      auto bob_comment_vote_total = db.get_comment( "bob", string( "test" ) ).total_vote_weight;
      auto bob_comment_ESCORreward = db.get_comment( "bob", string( "test" ) ).net_ESCORreward;
      auto bob_EUSDbalance = db.get_account( "bob" ).EUSDbalance;
      auto alice_ESCOR = db.get_account( "alice" ).ESCOR;
      auto bob_ESCOR = db.get_account( "bob" ).ESCOR;
      auto sam_ESCOR = db.get_account( "sam" ).ESCOR;
      auto dave_ESCOR = db.get_account( "dave" ).ESCOR;

      auto bob_comment_payout = asset( ( ( uint128_t( bob_comment_ESCORreward.value ) * bob_comment_ESCORreward.value * ECOreward.amount.value ) / total_ESCORreward2 ).to_uint64(), SYMBOL_ECO );
      auto bob_comment_vote_rewards = asset( bob_comment_payout.amount / 2, SYMBOL_ECO );
      bob_comment_payout -= bob_comment_vote_rewards;
      auto bob_comment_EUSDreward = asset( bob_comment_payout.amount / 2, SYMBOL_ECO ) * exchange_rate;
      auto bob_comment_ECO_fund_for_ESCOR_reward = ( bob_comment_payout - asset( bob_comment_payout.amount / 2, SYMBOL_ECO ) ) * db.get_dynamic_global_properties().get_ESCOR_price();
      auto unclaimed_payments = bob_comment_vote_rewards;
      auto alice_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * bob_comment_vote_rewards.amount.value ) / bob_comment_vote_total ), SYMBOL_ECO );
      auto alice_vote_ECO_fund_for_ESCOR = alice_vote_reward * db.get_dynamic_global_properties().get_ESCOR_price();
      auto bob_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * bob_comment_vote_rewards.amount.value ) / bob_comment_vote_total ), SYMBOL_ECO );
      auto bob_vote_ECO_fund_for_ESCOR = bob_vote_reward * db.get_dynamic_global_properties().get_ESCOR_price();
      auto sam_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "sam" ) ).id ) )->weight ) * bob_comment_vote_rewards.amount.value ) / bob_comment_vote_total ), SYMBOL_ECO );
      auto sam_vote_ECO_fund_for_ESCOR = sam_vote_reward * db.get_dynamic_global_properties().get_ESCOR_price();
      unclaimed_payments -= ( alice_vote_reward + bob_vote_reward + sam_vote_reward );

      BOOST_TEST_MESSAGE( "Generate one block to cause a payout" );

      generate_block();

      auto bob_comment_reward = get_last_operations( 1 )[0].get< comment_reward_operation >();

      BOOST_REQUIRE( db.get_dynamic_global_properties().total_reward_fund_ECO.amount.value == ECOreward.amount.value - ( bob_comment_payout + bob_comment_vote_rewards - unclaimed_payments ).amount.value );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).total_payout_value.amount.value == ( ( bob_comment_ECO_fund_for_ESCOR_reward * db.get_dynamic_global_properties().get_ESCOR_price() ) + ( bob_comment_EUSDreward * exchange_rate ) ).amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).EUSDbalance.amount.value == ( bob_EUSDbalance + bob_comment_EUSDreward ).amount.value );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value > 0 );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).net_ESCORreward.value == 0 );
      BOOST_REQUIRE( db.get_account( "alice" ).ESCOR.amount.value == ( alice_ESCOR + alice_vote_ECO_fund_for_ESCOR ).amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).ESCOR.amount.value == ( bob_ESCOR + bob_vote_ECO_fund_for_ESCOR + bob_comment_ECO_fund_for_ESCOR_reward ).amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).ESCOR.amount.value == ( sam_ESCOR + sam_vote_ECO_fund_for_ESCOR ).amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).ESCOR.amount.value == dave_ESCOR.amount.value );
      BOOST_REQUIRE( bob_comment_reward.author == "bob" );
      BOOST_REQUIRE( bob_comment_reward.permlink == "test" );
      BOOST_REQUIRE( bob_comment_reward.payout.amount.value == bob_comment_EUSDreward.amount.value );
      BOOST_REQUIRE( bob_comment_reward.ESCORpayout.amount.value == bob_comment_ECO_fund_for_ESCOR_reward.amount.value );
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
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value > 0 );
      BOOST_REQUIRE( db.get_comment( "bob", string( "test" ) ).net_ESCORreward.value == 0 );
      validate_database();

      BOOST_TEST_MESSAGE( "Generate block to cause payout" );

      ECOreward = db.get_dynamic_global_properties().total_reward_fund_ECO + ASSET( "2.000 TESTS" );
      total_ESCORreward2 = db.get_dynamic_global_properties().total_ESCORreward2;
      auto alice_comment_vote_total = db.get_comment( "alice", string( "test" ) ).total_vote_weight;
      auto alice_comment_ESCORreward = db.get_comment( "alice", string( "test" ) ).net_ESCORreward;
      auto alice_EUSDbalance = db.get_account( "alice" ).EUSDbalance;
      alice_ESCOR = db.get_account( "alice" ).ESCOR;
      bob_ESCOR = db.get_account( "bob" ).ESCOR;
      sam_ESCOR = db.get_account( "sam" ).ESCOR;
      dave_ESCOR = db.get_account( "dave" ).ESCOR;

      u256 rs( alice_comment_ESCORreward.value );
      u256 rf( ECOreward.amount.value );
      u256 trs2 = total_ESCORreward2.hi;
      trs2 = ( trs2 << 64 ) + total_ESCORreward2.lo;
      auto rs2 = rs*rs;

      auto alice_comment_payout = asset( static_cast< uint64_t >( ( rf * rs2 ) / trs2 ), SYMBOL_ECO );
      auto alice_comment_vote_rewards = asset( alice_comment_payout.amount / 2, SYMBOL_ECO );
      alice_comment_payout -= alice_comment_vote_rewards;
      auto alice_comment_EUSDreward = asset( alice_comment_payout.amount / 2, SYMBOL_ECO ) * exchange_rate;
      auto alice_comment_ECO_fund_for_ESCOR_reward = ( alice_comment_payout - asset( alice_comment_payout.amount / 2, SYMBOL_ECO ) ) * db.get_dynamic_global_properties().get_ESCOR_price();
      unclaimed_payments = alice_comment_vote_rewards;
      alice_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_ECO );
      alice_vote_ECO_fund_for_ESCOR = alice_vote_reward * db.get_dynamic_global_properties().get_ESCOR_price();
      bob_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_ECO );
      bob_vote_ECO_fund_for_ESCOR = bob_vote_reward * db.get_dynamic_global_properties().get_ESCOR_price();
      sam_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "sam" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_ECO );
      sam_vote_ECO_fund_for_ESCOR = sam_vote_reward * db.get_dynamic_global_properties().get_ESCOR_price();
      auto dave_vote_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "dave" ) ).id ) )->weight ) * alice_comment_vote_rewards.amount.value ) / alice_comment_vote_total ), SYMBOL_ECO );
      auto dave_vote_ECO_fund_for_ESCOR = dave_vote_reward * db.get_dynamic_global_properties().get_ESCOR_price();
      unclaimed_payments -= ( alice_vote_reward + bob_vote_reward + sam_vote_reward + dave_vote_reward );

      generate_block();
      auto alice_comment_reward = get_last_operations( 1 )[0].get< comment_reward_operation >();

      BOOST_REQUIRE( ( db.get_dynamic_global_properties().total_reward_fund_ECO + alice_comment_payout + alice_comment_vote_rewards - unclaimed_payments ).amount.value == ECOreward.amount.value );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).total_payout_value.amount.value == ( ( alice_comment_ECO_fund_for_ESCOR_reward * db.get_dynamic_global_properties().get_ESCOR_price() ) + ( alice_comment_EUSDreward * exchange_rate ) ).amount.value );
      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance.amount.value == ( alice_EUSDbalance + alice_comment_EUSDreward ).amount.value );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value == 0 );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value == 0 );
      BOOST_REQUIRE( db.get_account( "alice" ).ESCOR.amount.value == ( alice_ESCOR + alice_vote_ECO_fund_for_ESCOR + alice_comment_ECO_fund_for_ESCOR_reward ).amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).ESCOR.amount.value == ( bob_ESCOR + bob_vote_ECO_fund_for_ESCOR ).amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).ESCOR.amount.value == ( sam_ESCOR + sam_vote_ECO_fund_for_ESCOR ).amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).ESCOR.amount.value == ( dave_ESCOR + dave_vote_ECO_fund_for_ESCOR ).amount.value );
      BOOST_REQUIRE( alice_comment_reward.author == "alice" );
      BOOST_REQUIRE( alice_comment_reward.permlink == "test" );
      BOOST_REQUIRE( alice_comment_reward.payout.amount.value == alice_comment_EUSDreward.amount.value );
      BOOST_REQUIRE( alice_comment_reward.ESCORpayout.amount.value == alice_comment_ECO_fund_for_ESCOR_reward.amount.value );
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

      bob_ESCOR = db.get_account( "bob" ).ESCOR;
      auto bob_EUSD = db.get_account( "bob" ).EUSDbalance;

      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "dave" ) ).id ) ) != vote_idx.end() );
      validate_database();

      generate_block();

      BOOST_REQUIRE( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "dave" ) ).id ) ) == vote_idx.end() );
      BOOST_REQUIRE( bob_ESCOR.amount.value == db.get_account( "bob" ).ESCOR.amount.value );
      BOOST_REQUIRE( bob_EUSD.amount.value == db.get_account( "bob" ).EUSDbalance.amount.value );
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

      price exchange_rate( ASSET( "1.000 TESTS" ), ASSET( "1.000 TBD" ) );
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
      uint128_t ECOreward = gpo.total_reward_fund_ECO.amount.value + ASSET( "2.000 TESTS" ).amount.value;
      uint128_t total_ESCORreward2 = gpo.total_ESCORreward2;

      auto alice_comment = db.get_comment( "alice", string( "test" ) );
      auto bob_comment = db.get_comment( "bob", string( "test" ) );
      auto sam_comment = db.get_comment( "sam", string( "test" ) );
      auto dave_comment = db.get_comment( "dave", string( "test" ) );

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      // Calculate total comment rewards and voting rewards.
      auto alice_comment_reward = ( ( ECOreward * alice_comment.net_ESCORreward.value * alice_comment.net_ESCORreward.value ) / total_ESCORreward2 ).to_uint64();
      total_ESCORreward2 -= uint128_t( alice_comment.net_ESCORreward.value ) * ( alice_comment.net_ESCORreward.value );
      ECOreward -= alice_comment_reward;
      auto alice_comment_vote_rewards = alice_comment_reward / 2;
      alice_comment_reward -= alice_comment_vote_rewards;

      auto alice_vote_alice_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * alice_comment_vote_rewards ) / alice_comment.total_vote_weight ), SYMBOL_ECO );
      auto bob_vote_alice_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "alice", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * alice_comment_vote_rewards ) / alice_comment.total_vote_weight ), SYMBOL_ECO );
      ECOreward += alice_comment_vote_rewards - ( alice_vote_alice_reward + bob_vote_alice_reward ).amount.value;

      auto bob_comment_reward = ( ( ECOreward * bob_comment.net_ESCORreward.value * bob_comment.net_ESCORreward.value ) / total_ESCORreward2 ).to_uint64();
      total_ESCORreward2 -= uint128_t( bob_comment.net_ESCORreward.value ) * bob_comment.net_ESCORreward.value;
      ECOreward -= bob_comment_reward;
      auto bob_comment_vote_rewards = bob_comment_reward / 2;
      bob_comment_reward -= bob_comment_vote_rewards;

      auto alice_vote_bob_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "alice" ) ).id ) )->weight ) * bob_comment_vote_rewards ) / bob_comment.total_vote_weight ), SYMBOL_ECO );
      auto bob_vote_bob_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * bob_comment_vote_rewards ) / bob_comment.total_vote_weight ), SYMBOL_ECO );
      auto sam_vote_bob_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "bob", string( "test" ).id, db.get_account( "sam" ) ).id ) )->weight ) * bob_comment_vote_rewards ) / bob_comment.total_vote_weight ), SYMBOL_ECO );
      ECOreward += bob_comment_vote_rewards - ( alice_vote_bob_reward + bob_vote_bob_reward + sam_vote_bob_reward ).amount.value;

      auto dave_comment_reward = ( ( ECOreward * dave_comment.net_ESCORreward.value * dave_comment.net_ESCORreward.value ) / total_ESCORreward2 ).to_uint64();
      total_ESCORreward2 -= uint128_t( dave_comment.net_ESCORreward.value ) * dave_comment.net_ESCORreward.value;
      ECOreward -= dave_comment_reward;
      auto dave_comment_vote_rewards = dave_comment_reward / 2;
      dave_comment_reward -= dave_comment_vote_rewards;

      auto bob_vote_dave_reward = asset( static_cast< uint64_t >( ( u256( vote_idx.find( std::make_tuple( db.get_comment( "dave", string( "test" ).id, db.get_account( "bob" ) ).id ) )->weight ) * dave_comment_vote_rewards ) / dave_comment.total_vote_weight ), SYMBOL_ECO );
      ECOreward += dave_comment_vote_rewards - bob_vote_dave_reward.amount.value;

      // Calculate rewards paid to parent posts
      auto alice_pays_alice_EUSD = alice_comment_reward / 2;
      auto alice_pays_alice_ESCOR = alice_comment_reward - alice_pays_alice_EUSD;
      auto bob_pays_bob_EUSD = bob_comment_reward / 2;
      auto bob_pays_bob_ESCOR = bob_comment_reward - bob_pays_bob_EUSD;
      auto dave_pays_dave_EUSD = dave_comment_reward / 2;
      auto dave_pays_dave_ESCOR = dave_comment_reward - dave_pays_dave_EUSD;

      auto bob_pays_alice_EUSD = bob_pays_bob_EUSD / 2;
      auto bob_pays_alice_ESCOR = bob_pays_bob_ESCOR / 2;
      bob_pays_bob_EUSD -= bob_pays_alice_EUSD;
      bob_pays_bob_ESCOR -= bob_pays_alice_ESCOR;

      auto dave_pays_sam_EUSD = dave_pays_dave_EUSD / 2;
      auto dave_pays_sam_ESCOR = dave_pays_dave_ESCOR / 2;
      dave_pays_dave_EUSD -= dave_pays_sam_EUSD;
      dave_pays_dave_ESCOR -= dave_pays_sam_ESCOR;
      auto dave_pays_bob_EUSD = dave_pays_sam_EUSD / 2;
      auto dave_pays_bob_ESCOR = dave_pays_sam_ESCOR / 2;
      dave_pays_sam_EUSD -= dave_pays_bob_EUSD;
      dave_pays_sam_ESCOR -= dave_pays_bob_ESCOR;
      auto dave_pays_alice_EUSD = dave_pays_bob_EUSD / 2;
      auto dave_pays_alice_ESCOR = dave_pays_bob_ESCOR / 2;
      dave_pays_bob_EUSD -= dave_pays_alice_EUSD;
      dave_pays_bob_ESCOR -= dave_pays_alice_ESCOR;

      // Calculate total comment payouts
      auto alice_comment_total_payout = db.to_EUSD( asset( alice_pays_alice_EUSD + alice_pays_alice_ESCOR, SYMBOL_ECO ) );
      alice_comment_total_payout += db.to_EUSD( asset( bob_pays_alice_EUSD + bob_pays_alice_ESCOR, SYMBOL_ECO ) );
      alice_comment_total_payout += db.to_EUSD( asset( dave_pays_alice_EUSD + dave_pays_alice_ESCOR, SYMBOL_ECO ) );
      auto bob_comment_total_payout = db.to_EUSD( asset( bob_pays_bob_EUSD + bob_pays_bob_ESCOR, SYMBOL_ECO ) );
      bob_comment_total_payout += db.to_EUSD( asset( dave_pays_bob_EUSD + dave_pays_bob_ESCOR, SYMBOL_ECO ) );
      auto sam_comment_total_payout = db.to_EUSD( asset( dave_pays_sam_EUSD + dave_pays_sam_ESCOR, SYMBOL_ECO ) );
      auto dave_comment_total_payout = db.to_EUSD( asset( dave_pays_dave_EUSD + dave_pays_dave_ESCOR, SYMBOL_ECO ) );

      auto alice_starting_ECO_fund_for_ESCOR = db.get_account( "alice" ).ESCOR;
      auto alice_starting_EUSD = db.get_account( "alice" ).EUSDbalance;
      auto bob_starting_ECO_fund_for_ESCOR = db.get_account( "bob" ).ESCOR;
      auto bob_starting_EUSD = db.get_account( "bob" ).EUSDbalance;
      auto sam_starting_ECO_fund_for_ESCOR = db.get_account( "sam" ).ESCOR;
      auto sam_starting_EUSD = db.get_account( "sam" ).EUSDbalance;
      auto dave_starting_ECO_fund_for_ESCOR = db.get_account( "dave" ).ESCOR;
      auto dave_starting_EUSD = db.get_account( "dave" ).EUSDbalance;

      generate_block();

      gpo = db.get_dynamic_global_properties();

      // Calculate ESCOR rewards from voting.
      auto alice_vote_alice_ECO_fund_for_ESCOR = alice_vote_alice_reward * gpo.get_ESCOR_price();
      auto bob_vote_alice_ECO_fund_for_ESCOR = bob_vote_alice_reward * gpo.get_ESCOR_price();
      auto alice_vote_bob_ECO_fund_for_ESCOR = alice_vote_bob_reward * gpo.get_ESCOR_price();
      auto bob_vote_bob_ECO_fund_for_ESCOR = bob_vote_bob_reward * gpo.get_ESCOR_price();
      auto sam_vote_bob_ECO_fund_for_ESCOR = sam_vote_bob_reward * gpo.get_ESCOR_price();
      auto bob_vote_dave_ECO_fund_for_ESCOR = bob_vote_dave_reward * gpo.get_ESCOR_price();

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
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_alice_EUSD );
      BOOST_REQUIRE( ( com_vop.ESCORpayout * gpo.get_ESCOR_price() ).amount.value == dave_pays_alice_ESCOR );

      com_vop = ops[1].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "bob" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "dave" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_bob_EUSD );
      BOOST_REQUIRE( ( com_vop.ESCORpayout * gpo.get_ESCOR_price() ).amount.value == dave_pays_bob_ESCOR );

      com_vop = ops[2].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "sam" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "dave" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_sam_EUSD );
      BOOST_REQUIRE( ( com_vop.ESCORpayout * gpo.get_ESCOR_price() ).amount.value == dave_pays_sam_ESCOR );

      com_vop = ops[3].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "dave" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "dave" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == dave_pays_dave_EUSD );
      BOOST_REQUIRE( ( com_vop.ESCORpayout * gpo.get_ESCOR_price() ).amount.value == dave_pays_dave_ESCOR );

      cur_vop = ops[4].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "bob" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == bob_vote_dave_ECO_fund_for_ESCOR.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "dave" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      com_vop = ops[5].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "alice" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "bob" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == bob_pays_alice_EUSD );
      BOOST_REQUIRE( ( com_vop.ESCORpayout * gpo.get_ESCOR_price() ).amount.value == bob_pays_alice_ESCOR );

      com_vop = ops[6].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "bob" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "bob" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == bob_pays_bob_EUSD );
      BOOST_REQUIRE( ( com_vop.ESCORpayout * gpo.get_ESCOR_price() ).amount.value == bob_pays_bob_ESCOR );

      cur_vop = ops[7].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "sam" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == sam_vote_bob_ECO_fund_for_ESCOR.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "bob" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      cur_vop = ops[8].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "bob" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == bob_vote_bob_ECO_fund_for_ESCOR.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "bob" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      cur_vop = ops[9].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "alice" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == alice_vote_bob_ECO_fund_for_ESCOR.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "bob" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      com_vop = ops[10].get< comment_reward_operation >();
      BOOST_REQUIRE( com_vop.author == "alice" );
      BOOST_REQUIRE( com_vop.permlink == "test" );
      BOOST_REQUIRE( com_vop.originating_author == "alice" );
      BOOST_REQUIRE( com_vop.originating_permlink == "test" );
      BOOST_REQUIRE( com_vop.payout.amount.value == alice_pays_alice_EUSD );
      BOOST_REQUIRE( ( com_vop.ESCORpayout * gpo.get_ESCOR_price() ).amount.value == alice_pays_alice_ESCOR );

      cur_vop = ops[11].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "bob" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == bob_vote_alice_ECO_fund_for_ESCOR.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "alice" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      cur_vop = ops[12].get< curate_reward_operation >();
      BOOST_REQUIRE( cur_vop.curator == "alice" );
      BOOST_REQUIRE( cur_vop.reward.amount.value == alice_vote_alice_ECO_fund_for_ESCOR.amount.value );
      BOOST_REQUIRE( cur_vop.comment_author == "alice" );
      BOOST_REQUIRE( cur_vop.comment_permlink == "test" );

      BOOST_TEST_MESSAGE( "Checking account balances" );

      auto alice_EUSDtotal = alice_starting_EUSD + asset( alice_pays_alice_EUSD + bob_pays_alice_EUSD + dave_pays_alice_EUSD, SYMBOL_ECO ) * exchange_rate;
      auto alice_totalECO_fund_for_ESCOR = alice_starting_ECO_fund_for_ESCOR + asset( alice_pays_alice_ESCOR + bob_pays_alice_ESCOR + dave_pays_alice_ESCOR + alice_vote_alice_reward.amount + alice_vote_bob_reward.amount, SYMBOL_ECO ) * gpo.get_ESCOR_price();
      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance.amount.value == alice_EUSDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "alice" ).ESCOR.amount.value == alice_totalECO_fund_for_ESCOR.amount.value );

      auto bob_EUSDtotal = bob_starting_EUSD + asset( bob_pays_bob_EUSD + dave_pays_bob_EUSD, SYMBOL_ECO ) * exchange_rate;
      auto bob_totalECO_fund_for_ESCOR = bob_starting_ECO_fund_for_ESCOR + asset( bob_pays_bob_ESCOR + dave_pays_bob_ESCOR + bob_vote_alice_reward.amount + bob_vote_bob_reward.amount + bob_vote_dave_reward.amount, SYMBOL_ECO ) * gpo.get_ESCOR_price();
      BOOST_REQUIRE( db.get_account( "bob" ).EUSDbalance.amount.value == bob_EUSDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).ESCOR.amount.value == bob_totalECO_fund_for_ESCOR.amount.value );

      auto sam_EUSDtotal = sam_starting_EUSD + asset( dave_pays_sam_EUSD, SYMBOL_ECO ) * exchange_rate;
      auto sam_totalECO_fund_for_ESCOR = bob_starting_ECO_fund_for_ESCOR + asset( dave_pays_sam_ESCOR + sam_vote_bob_reward.amount, SYMBOL_ECO ) * gpo.get_ESCOR_price();
      BOOST_REQUIRE( db.get_account( "sam" ).EUSDbalance.amount.value == sam_EUSDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).ESCOR.amount.value == sam_totalECO_fund_for_ESCOR.amount.value );

      auto dave_EUSDtotal = dave_starting_EUSD + asset( dave_pays_dave_EUSD, SYMBOL_ECO ) * exchange_rate;
      auto dave_totalECO_fund_for_ESCOR = dave_starting_ECO_fund_for_ESCOR + asset( dave_pays_dave_ESCOR, SYMBOL_ECO ) * gpo.get_ESCOR_price();
      BOOST_REQUIRE( db.get_account( "dave" ).EUSDbalance.amount.value == dave_EUSDtotal.amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).ESCOR.amount.value == dave_totalECO_fund_for_ESCOR.amount.value );
   }
   FC_LOG_AND_RETHROW()
}
*/


BOOST_AUTO_TEST_CASE( ECO_fund_for_ESCOR_withdrawals )
{
   try
   {
      ACTORS( (alice) )
      fund( "alice", 100000 );
      score( "alice", 100000 );

      const auto& new_alice = db.get_account( "alice" );

      BOOST_TEST_MESSAGE( "Setting up withdrawal" );

      signed_transaction tx;
      withdrawESCOR_operation op;
      op.account = "alice";
      op.ESCOR = asset( new_alice.ESCOR.amount / 2, SYMBOL_TP );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto next_withdrawal = db.head_block_time() + ESCOR_WITHDRAW_INTERVAL_SECONDS;
      asset ESCOR = new_alice.ESCOR;
      asset to_withdraw = op.ESCOR;
      asset original_ECO_fund_for_ESCOR = ESCOR;
      asset withdraw_rate = new_alice.ESCORwithdrawRateInECO;

      BOOST_TEST_MESSAGE( "Generating block up to first withdrawal" );
      generate_blocks( next_withdrawal - ( BLOCK_INTERVAL / 2 ), true);

      BOOST_REQUIRE( db.get_account( "alice" ).ESCOR.amount.value == ESCOR.amount.value );

      BOOST_TEST_MESSAGE( "Generating block to cause withdrawal" );
      generate_block();

      auto fill_op = get_last_operations( 1 )[0].get< fillESCORWithdraw_operation >();
      auto gpo = db.get_dynamic_global_properties();

      BOOST_REQUIRE( db.get_account( "alice" ).ESCOR.amount.value == ( ESCOR - withdraw_rate ).amount.value );
      BOOST_REQUIRE( ( withdraw_rate * gpo.get_ESCOR_price() ).amount.value - db.get_account( "alice" ).balance.amount.value <= 1 ); // Check a range due to differences in the share price
      BOOST_REQUIRE( fill_op.from_account == "alice" );
      BOOST_REQUIRE( fill_op.to_account == "alice" );
      BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
      BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_ESCOR_price() ).amount.value ) <= 1 );
      validate_database();

      BOOST_TEST_MESSAGE( "Generating the rest of the blocks in the withdrawal" );

      ESCOR = db.get_account( "alice" ).ESCOR;
      auto balance = db.get_account( "alice" ).balance;
      auto old_next_ECO_fund_for_ESCOR = db.get_account( "alice" ).nextESCORwithdrawalTime;

      for( int i = 1; i < ECO_fund_for_ESCOR_WITHDRAW_INTERVALS - 1; i++ )
      {
         generate_blocks( db.head_block_time() + ESCOR_WITHDRAW_INTERVAL_SECONDS );

         const auto& alice = db.get_account( "alice" );

         gpo = db.get_dynamic_global_properties();
         fill_op = get_last_operations( 1 )[0].get< fillESCORWithdraw_operation >();

         BOOST_REQUIRE( alice.ESCOR.amount.value == ( ESCOR - withdraw_rate ).amount.value );
         BOOST_REQUIRE( balance.amount.value + ( withdraw_rate * gpo.get_ESCOR_price() ).amount.value - alice.balance.amount.value <= 1 );
         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_ESCOR_price() ).amount.value ) <= 1 );

         if ( i == ECO_fund_for_ESCOR_WITHDRAW_INTERVALS - 1 )
            BOOST_REQUIRE( alice.nextESCORwithdrawalTime == fc::time_point_sec::maximum() );
         else
            BOOST_REQUIRE( alice.nextESCORwithdrawalTime.sec_since_epoch() == ( old_next_ECO_fund_for_ESCOR + ESCOR_WITHDRAW_INTERVAL_SECONDS ).sec_since_epoch() );

         validate_database();

         ESCOR = alice.ESCOR;
         balance = alice.balance;
         old_next_ECO_fund_for_ESCOR = alice.nextESCORwithdrawalTime;
      }

      if (  to_withdraw.amount.value % withdraw_rate.amount.value != 0 )
      {
         BOOST_TEST_MESSAGE( "Generating one more block to take care of remainder" );
         generate_blocks( db.head_block_time() + ESCOR_WITHDRAW_INTERVAL_SECONDS, true );
         fill_op = get_last_operations( 1 )[0].get< fillESCORWithdraw_operation >();
         gpo = db.get_dynamic_global_properties();

         BOOST_REQUIRE( db.get_account( "alice" ).nextESCORwithdrawalTime.sec_since_epoch() == ( old_next_ECO_fund_for_ESCOR + ESCOR_WITHDRAW_INTERVAL_SECONDS ).sec_since_epoch() );
         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_ESCOR_price() ).amount.value ) <= 1 );

         generate_blocks( db.head_block_time() + ESCOR_WITHDRAW_INTERVAL_SECONDS, true );
         gpo = db.get_dynamic_global_properties();
         fill_op = get_last_operations( 1 )[0].get< fillESCORWithdraw_operation >();

         BOOST_REQUIRE( db.get_account( "alice" ).nextESCORwithdrawalTime.sec_since_epoch() == fc::time_point_sec::maximum().sec_since_epoch() );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == to_withdraw.amount.value % withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_ESCOR_price() ).amount.value ) <= 1 );

         validate_database();
      }
      else
      {
         generate_blocks( db.head_block_time() + ESCOR_WITHDRAW_INTERVAL_SECONDS, true );

         BOOST_REQUIRE( db.get_account( "alice" ).nextESCORwithdrawalTime.sec_since_epoch() == fc::time_point_sec::maximum().sec_since_epoch() );

         fill_op = get_last_operations( 1 )[0].get< fillESCORWithdraw_operation >();
         BOOST_REQUIRE( fill_op.from_account == "alice" );
         BOOST_REQUIRE( fill_op.to_account == "alice" );
         BOOST_REQUIRE( fill_op.withdrawn.amount.value == withdraw_rate.amount.value );
         BOOST_REQUIRE( std::abs( ( fill_op.deposited - fill_op.withdrawn * gpo.get_ESCOR_price() ).amount.value ) <= 1 );
      }

      BOOST_REQUIRE( db.get_account( "alice" ).ESCOR.amount.value == ( original_ECO_fund_for_ESCOR - op.ESCOR ).amount.value );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( ECO_fund_for_ESCOR_withdraw_route )
{
   try
   {
      ACTORS( (alice)(bob)(sam) )

      auto original_ECO_fund_for_ESCOR = alice.ESCOR;

      fund( "alice", 1040000 );
      score( "alice", 1040000 );

      auto withdraw_amount = alice.ESCOR - original_ECO_fund_for_ESCOR;

      BOOST_TEST_MESSAGE( "Setup ESCOR withdraw" );
      withdrawESCOR_operation wv;
      wv.account = "alice";
      wv.ESCOR = withdraw_amount;

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( wv );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "Setting up bob destination" );
      setWithdrawESCORasECOroute_operation op;
      op.from_account = "alice";
      op.to_account = "bob";
      op.percent = PERCENT_1 * 50;
      op.autoESCOR = true;
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "Setting up sam destination" );
      op.to_account = "sam";
      op.percent = PERCENT_1 * 30;
      op.autoESCOR = false;
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Setting up first withdraw" );

      auto ESCORwithdrawRateInECO = alice.ESCORwithdrawRateInECO;
      auto old_alice_balance = alice.balance;
      auto old_alice_ECO_fund_for_ESCOR = alice.ESCOR;
      auto old_bob_balance = bob.balance;
      auto old_bob_ECO_fund_for_ESCOR = bob.ESCOR;
      auto old_sam_balance = sam.balance;
      auto old_sam_ECO_fund_for_ESCOR = sam.ESCOR;
      generate_blocks( alice.nextESCORwithdrawalTime, true );

      {
         const auto& alice = db.get_account( "alice" );
         const auto& bob = db.get_account( "bob" );
         const auto& sam = db.get_account( "sam" );

         BOOST_REQUIRE( alice.ESCOR == old_alice_ECO_fund_for_ESCOR - ESCORwithdrawRateInECO );
         BOOST_REQUIRE( alice.balance == old_alice_balance + asset( ( ESCORwithdrawRateInECO.amount * PERCENT_1 * 20 ) / PERCENT_100, SYMBOL_TP ) * db.get_dynamic_global_properties().get_ESCOR_price() );
         BOOST_REQUIRE( bob.ESCOR == old_bob_ECO_fund_for_ESCOR + asset( ( ESCORwithdrawRateInECO.amount * PERCENT_1 * 50 ) / PERCENT_100, SYMBOL_TP ) );
         BOOST_REQUIRE( bob.balance == old_bob_balance );
         BOOST_REQUIRE( sam.ESCOR == old_sam_ECO_fund_for_ESCOR );
         BOOST_REQUIRE( sam.balance ==  old_sam_balance + asset( ( ESCORwithdrawRateInECO.amount * PERCENT_1 * 30 ) / PERCENT_100, SYMBOL_TP ) * db.get_dynamic_global_properties().get_ESCOR_price() );

         old_alice_balance = alice.balance;
         old_alice_ECO_fund_for_ESCOR = alice.ESCOR;
         old_bob_balance = bob.balance;
         old_bob_ECO_fund_for_ESCOR = bob.ESCOR;
         old_sam_balance = sam.balance;
         old_sam_ECO_fund_for_ESCOR = sam.ESCOR;
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

      generate_blocks( db.get_account( "alice" ).nextESCORwithdrawalTime, true );
      {
         const auto& alice = db.get_account( "alice" );
         const auto& bob = db.get_account( "bob" );
         const auto& sam = db.get_account( "sam" );

         BOOST_REQUIRE( alice.ESCOR == old_alice_ECO_fund_for_ESCOR - ESCORwithdrawRateInECO );
         BOOST_REQUIRE( alice.balance == old_alice_balance );
         BOOST_REQUIRE( bob.ESCOR == old_bob_ECO_fund_for_ESCOR + asset( ( ESCORwithdrawRateInECO.amount * PERCENT_1 * 50 ) / PERCENT_100, SYMBOL_TP ) );
         BOOST_REQUIRE( bob.balance == old_bob_balance );
         BOOST_REQUIRE( sam.ESCOR == old_sam_ECO_fund_for_ESCOR );
         BOOST_REQUIRE( sam.balance ==  old_sam_balance + asset( ( ESCORwithdrawRateInECO.amount * PERCENT_1 * 50 ) / PERCENT_100, SYMBOL_TP ) * db.get_dynamic_global_properties().get_ESCOR_price() );
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
         transfer( INIT_MINER_NAME, accounts[i], 10000 );
         witness_create( accounts[i], keys[i], "foo.bar", keys[i].get_public_key(), 1000 );

         ops.push_back( feed_publish_operation() );
         ops[i].publisher = accounts[i];

         txs.push_back( signed_transaction() );
      }

      ops[0].exchange_rate = price( asset( 100000, SYMBOL_ECO ), asset( 1000, SYMBOL_EUSD ) );
      ops[1].exchange_rate = price( asset( 105000, SYMBOL_ECO ), asset( 1000, SYMBOL_EUSD ) );
      ops[2].exchange_rate = price( asset(  98000, SYMBOL_ECO ), asset( 1000, SYMBOL_EUSD ) );
      ops[3].exchange_rate = price( asset(  97000, SYMBOL_ECO ), asset( 1000, SYMBOL_EUSD ) );
      ops[4].exchange_rate = price( asset(  99000, SYMBOL_ECO ), asset( 1000, SYMBOL_EUSD ) );
      ops[5].exchange_rate = price( asset(  97500, SYMBOL_ECO ), asset( 1000, SYMBOL_EUSD ) );
      ops[6].exchange_rate = price( asset( 102000, SYMBOL_ECO ), asset( 1000, SYMBOL_EUSD ) );

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
      BOOST_REQUIRE( feed_history.current_median_history == price( asset( 99000, SYMBOL_ECO), asset( 1000, SYMBOL_EUSD ) ) );
      BOOST_REQUIRE( feed_history.price_history[ 0 ] == price( asset( 99000, SYMBOL_ECO), asset( 1000, SYMBOL_EUSD ) ) );
      validate_database();

      for ( int i = 0; i < 23; i++ )
      {
         BOOST_TEST_MESSAGE( "Updating ops" );

         for( int j = 0; j < 7; j++ )
         {
            txs[j].operations.clear();
            txs[j].signatures.clear();
            ops[j].exchange_rate = price( ops[j].exchange_rate.base, asset( ops[j].exchange_rate.quote.amount + 10, SYMBOL_EUSD ) );
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

BOOST_AUTO_TEST_CASE( convert_delay )
{
   try
   {
      ACTORS( (alice) )
      generate_block();
      score( "alice", ASSET( "10.000 TESTS" ) );
      fund( "alice", ASSET( "25.000 TBD" ) );

      set_price_feed( price( asset::from_string( "1.250 TESTS" ), asset::from_string( "1.000 TBD" ) ) );

      convert_operation op;
      signed_transaction tx;

      auto start_balance = ASSET( "25.000 TBD" );

      BOOST_TEST_MESSAGE( "Setup conversion to TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      op.owner = "alice";
      op.amount = asset( 2000, SYMBOL_EUSD );
      op.requestid = 2;
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Generating Blocks up to conversion block" );
      generate_blocks( db.head_block_time() + CONVERSION_DELAY - fc::seconds( BLOCK_INTERVAL / 2 ), true );

      BOOST_TEST_MESSAGE( "Verify conversion is not applied" );
      const auto& alice_2 = db.get_account( "alice" );
      const auto& convert_request_idx = db.get_index< convert_request_index >().indices().get< by_owner >();
      auto convert_request = convert_request_idx.find( std::make_tuple( "alice", 2 ) );

      BOOST_REQUIRE( convert_request != convert_request_idx.end() );
      BOOST_REQUIRE( alice_2.balance.amount.value == 0 );
      BOOST_REQUIRE( alice_2.EUSDbalance.amount.value == ( start_balance - op.amount ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "Generate one more block" );
      generate_block();

      BOOST_TEST_MESSAGE( "Verify conversion applied" );
      const auto& alice_3 = db.get_account( "alice" );
      auto vop = get_last_operations( 1 )[0].get< fill_convert_request_operation >();

      convert_request = convert_request_idx.find( std::make_tuple( "alice", 2 ) );
      BOOST_REQUIRE( convert_request == convert_request_idx.end() );
      BOOST_REQUIRE( alice_3.balance.amount.value == 2500 );
      BOOST_REQUIRE( alice_3.EUSDbalance.amount.value == ( start_balance - op.amount ).amount.value );
      BOOST_REQUIRE( vop.owner == "alice" );
      BOOST_REQUIRE( vop.requestid == 2 );
      BOOST_REQUIRE( vop.amount_in.amount.value == ASSET( "2.000 TBD" ).amount.value );
      BOOST_REQUIRE( vop.amount_out.amount.value == ASSET( "2.500 TESTS" ).amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( eznode_inflation )
{
   try
   {
   /*
      BOOST_TEST_MESSAGE( "Testing ECO Inflation until the ESCOR start block" );

      auto gpo = db.get_dynamic_global_properties();
      auto virtual_supply = gpo.virtual_supply;
      auto witness_name = db.get_scheduled_witness(1);
      auto old_witness_balance = db.get_account( witness_name ).balance;
      auto old_witness_ESCOR = db.get_account( witness_name ).ESCOR;

      auto new_rewards = std::max( MIN_CONTENT_REWARD, asset( ( CONTENT_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) )
         + std::max( MIN_CURATE_REWARD, asset( ( CURATE_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
      auto witness_pay = std::max( MIN_PRODUCER_REWARD, asset( ( PRODUCER_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
      auto witness_pay_ESCOR = asset( 0, SYMBOL_TP );
      auto new_ESCORvalueInECO = asset( 0, SYMBOL_ECO );
      auto new_ESCOR = gpo.totalESCOR;

      if ( db.get_account( witness_name ).ESCOR.amount.value == 0 )
      {
         new_ESCORvalueInECO += witness_pay;
         new_ESCOR += witness_pay * ( gpo.totalESCOR / gpo.totalECOfundForESCOR );
      }

      auto new_supply = gpo.current_supply + new_rewards + witness_pay + new_ESCORvalueInECO;
      new_rewards += gpo.total_reward_fund_ECO;
      new_ESCORvalueInECO += gpo.totalECOfundForESCOR;

      generate_block();

      gpo = db.get_dynamic_global_properties();

      BOOST_REQUIRE( gpo.current_supply.amount.value == new_supply.amount.value );
      BOOST_REQUIRE( gpo.virtual_supply.amount.value == new_supply.amount.value );
      BOOST_REQUIRE( gpo.total_reward_fund_ECO.amount.value == new_rewards.amount.value );
      BOOST_REQUIRE( gpo.totalECOfundForESCOR.amount.value == new_ESCORvalueInECO.amount.value );
      BOOST_REQUIRE( gpo.totalESCOR.amount.value == new_ESCOR.amount.value );
      BOOST_REQUIRE( db.get_account( witness_name ).balance.amount.value == ( old_witness_balance + witness_pay ).amount.value );

      validate_database();

      while( db.head_block_num() < START_ECO_fund_for_ESCOR_BLOCK - 1)
      {
         virtual_supply = gpo.virtual_supply;
         witness_name = db.get_scheduled_witness(1);
         old_witness_balance = db.get_account( witness_name ).balance;
         old_witness_ESCOR = db.get_account( witness_name ).ESCOR;


         new_rewards = std::max( MIN_CONTENT_REWARD, asset( ( CONTENT_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) )
            + std::max( MIN_CURATE_REWARD, asset( ( CURATE_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
         witness_pay = std::max( MIN_PRODUCER_REWARD, asset( ( PRODUCER_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
         new_ESCORvalueInECO = asset( 0, SYMBOL_ECO );
         new_ESCOR = gpo.totalESCOR;

         if ( db.get_account( witness_name ).ESCOR.amount.value == 0 )
         {
            new_ESCORvalueInECO += witness_pay;
            witness_pay_ESCOR = witness_pay * gpo.get_ESCOR_price();
            new_ESCOR += witness_pay_ESCOR;
            new_supply += witness_pay;
            witness_pay = asset( 0, SYMBOL_ECO );
         }

         new_supply = gpo.current_supply + new_rewards + witness_pay + new_ESCORvalueInECO;
         new_rewards += gpo.total_reward_fund_ECO;
         new_ESCORvalueInECO += gpo.totalECOfundForESCOR;

         generate_block();

         gpo = db.get_dynamic_global_properties();

         BOOST_REQUIRE( gpo.current_supply.amount.value == new_supply.amount.value );
         BOOST_REQUIRE( gpo.virtual_supply.amount.value == new_supply.amount.value );
         BOOST_REQUIRE( gpo.total_reward_fund_ECO.amount.value == new_rewards.amount.value );
         BOOST_REQUIRE( gpo.totalECOfundForESCOR.amount.value == new_ESCORvalueInECO.amount.value );
         BOOST_REQUIRE( gpo.totalESCOR.amount.value == new_ESCOR.amount.value );
         BOOST_REQUIRE( db.get_account( witness_name ).balance.amount.value == ( old_witness_balance + witness_pay ).amount.value );
         BOOST_REQUIRE( db.get_account( witness_name ).ESCOR.amount.value == ( old_witness_ESCOR + witness_pay_ESCOR ).amount.value );

         validate_database();
      }

      BOOST_TEST_MESSAGE( "Testing up to the start block for miner voting" );

      while( db.head_block_num() < START_MINER_VOTING_BLOCK - 1 )
      {
         virtual_supply = gpo.virtual_supply;
         witness_name = db.get_scheduled_witness(1);
         old_witness_balance = db.get_account( witness_name ).balance;

         new_rewards = std::max( MIN_CONTENT_REWARD, asset( ( CONTENT_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) )
            + std::max( MIN_CURATE_REWARD, asset( ( CURATE_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
         witness_pay = std::max( MIN_PRODUCER_REWARD, asset( ( PRODUCER_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
         auto witness_pay_ESCOR = asset( 0, SYMBOL_TP );
         new_ESCORvalueInECO = asset( ( witness_pay + new_rewards ).amount * 9, SYMBOL_ECO );
         new_ESCOR = gpo.totalESCOR;

         if ( db.get_account( witness_name ).ESCOR.amount.value == 0 )
         {
            new_ESCORvalueInECO += witness_pay;
            witness_pay_ESCOR = witness_pay * gpo.get_ESCOR_price();
            new_ESCOR += witness_pay_ESCOR;
            new_supply += witness_pay;
            witness_pay = asset( 0, SYMBOL_ECO );
         }

         new_supply = gpo.current_supply + new_rewards + witness_pay + new_ESCORvalueInECO;
         new_rewards += gpo.total_reward_fund_ECO;
         new_ESCORvalueInECO += gpo.totalECOfundForESCOR;

         generate_block();

         gpo = db.get_dynamic_global_properties();

         BOOST_REQUIRE( gpo.current_supply.amount.value == new_supply.amount.value );
         BOOST_REQUIRE( gpo.virtual_supply.amount.value == new_supply.amount.value );
         BOOST_REQUIRE( gpo.total_reward_fund_ECO.amount.value == new_rewards.amount.value );
         BOOST_REQUIRE( gpo.totalECOfundForESCOR.amount.value == new_ESCORvalueInECO.amount.value );
         BOOST_REQUIRE( gpo.totalESCOR.amount.value == new_ESCOR.amount.value );
         BOOST_REQUIRE( db.get_account( witness_name ).balance.amount.value == ( old_witness_balance + witness_pay ).amount.value );
         BOOST_REQUIRE( db.get_account( witness_name ).ESCOR.amount.value == ( old_witness_ESCOR + witness_pay_ESCOR ).amount.value );

         validate_database();
      }

      for( int i = 0; i < BLOCKS_PER_DAY; i++ )
      {
         virtual_supply = gpo.virtual_supply;
         witness_name = db.get_scheduled_witness(1);
         old_witness_balance = db.get_account( witness_name ).balance;

         new_rewards = std::max( MIN_CONTENT_REWARD, asset( ( CONTENT_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) )
            + std::max( MIN_CURATE_REWARD, asset( ( CURATE_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
         witness_pay = std::max( MIN_PRODUCER_REWARD, asset( ( PRODUCER_APR * gpo.virtual_supply.amount ) / ( BLOCKS_PER_YEAR * 100 ), SYMBOL_ECO ) );
         witness_pay_ESCOR = witness_pay * gpo.get_ESCOR_price();
         new_ESCORvalueInECO = asset( ( witness_pay + new_rewards ).amount * 9, SYMBOL_ECO ) + witness_pay;
         new_ESCOR = gpo.totalESCOR + witness_pay_ESCOR;
         new_supply = gpo.current_supply + new_rewards + new_ESCORvalueInECO;
         new_rewards += gpo.total_reward_fund_ECO;
         new_ESCORvalueInECO += gpo.totalECOfundForESCOR;

         generate_block();

         gpo = db.get_dynamic_global_properties();

         BOOST_REQUIRE( gpo.current_supply.amount.value == new_supply.amount.value );
         BOOST_REQUIRE( gpo.virtual_supply.amount.value == new_supply.amount.value );
         BOOST_REQUIRE( gpo.total_reward_fund_ECO.amount.value == new_rewards.amount.value );
         BOOST_REQUIRE( gpo.totalECOfundForESCOR.amount.value == new_ESCORvalueInECO.amount.value );
         BOOST_REQUIRE( gpo.totalESCOR.amount.value == new_ESCOR.amount.value );
         BOOST_REQUIRE( db.get_account( witness_name ).ESCOR.amount.value == ( old_witness_ESCOR + witness_pay_ESCOR ).amount.value );

         validate_database();
      }

      virtual_supply = gpo.virtual_supply;
      ESCOR = gpo.totalESCOR;
      ESCORvalueInECO = gpo.totalECOfundForESCOR;
      ECOreward = gpo.total_reward_fund_ECO;

      witness_name = db.get_scheduled_witness(1);
      old_witness_ESCOR = db.get_account( witness_name ).ESCOR;

      generate_block();

      gpo = db.get_dynamic_global_properties();

      BOOST_REQUIRE_EQUAL( gpo.totalECOfundForESCOR.amount.value,
         ( ESCORvalueInECO.amount.value
            + ( ( ( uint128_t( virtual_supply.amount.value ) / 10 ) / BLOCKS_PER_YEAR ) * 9 )
            + ( uint128_t( virtual_supply.amount.value ) / 100 / BLOCKS_PER_YEAR ) ).to_uint64() );
      BOOST_REQUIRE_EQUAL( gpo.total_reward_fund_ECO.amount.value,
         ECOreward.amount.value + virtual_supply.amount.value / 10 / BLOCKS_PER_YEAR + virtual_supply.amount.value / 10 / BLOCKS_PER_DAY );
      BOOST_REQUIRE_EQUAL( db.get_account( witness_name ).ESCOR.amount.value,
         old_witness_ESCOR.amount.value + ( asset( ( ( virtual_supply.amount.value / BLOCKS_PER_YEAR ) * PERCENT_1 ) / PERCENT_100, SYMBOL_ECO ) * ( ESCOR / ESCORvalueInECO ) ).amount.value );
      validate_database();
      */
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( EUSD_interest )
{
   try
   {
      ACTORS( (alice)(bob) )
      generate_block();
      score( "alice", ASSET( "10.000 TESTS" ) );
      score( "bob", ASSET( "10.000 TESTS" ) );

      set_price_feed( price( asset::from_string( "1.000 TESTS" ), asset::from_string( "1.000 TBD" ) ) );

      BOOST_TEST_MESSAGE( "Testing interest over smallest interest period" );

      convert_operation op;
      signed_transaction tx;

      fund( "alice", ASSET( "31.903 TBD" ) );

      auto start_time = db.get_account( "alice" ).EUSD_seconds_last_update;
      auto alice_EUSD = db.get_account( "alice" ).EUSDbalance;

      generate_blocks( db.head_block_time() + fc::seconds( EUSD_INTEREST_COMPOUND_INTERVAL_SEC ), true );

      transfer_operation transfer;
      transfer.to = "bob";
      transfer.from = "alice";
      transfer.amount = ASSET( "1.000 TBD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( transfer );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto gpo = db.get_dynamic_global_properties();
      auto interest_op = get_last_operations( 1 )[0].get< interest_operation >();

      BOOST_REQUIRE( gpo.EUSD_interest_rate > 0 );
      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance.amount.value == alice_EUSD.amount.value - ASSET( "1.000 TBD" ).amount.value + ( ( ( ( uint128_t( alice_EUSD.amount.value ) * ( db.head_block_time() - start_time ).to_seconds() ) / SECONDS_PER_YEAR ) * gpo.EUSD_interest_rate ) / PERCENT_100 ).to_uint64() );
      BOOST_REQUIRE( interest_op.owner == "alice" );
      BOOST_REQUIRE( interest_op.interest.amount.value == db.get_account( "alice" ).EUSDbalance.amount.value - ( alice_EUSD.amount.value - ASSET( "1.000 TBD" ).amount.value ) );
      validate_database();

      BOOST_TEST_MESSAGE( "Testing interest under interest period" );

      start_time = db.get_account( "alice" ).EUSD_seconds_last_update;
      alice_EUSD = db.get_account( "alice" ).EUSDbalance;

      generate_blocks( db.head_block_time() + fc::seconds( EUSD_INTEREST_COMPOUND_INTERVAL_SEC / 2 ), true );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance.amount.value == alice_EUSD.amount.value - ASSET( "1.000 TBD" ).amount.value );
      validate_database();

      auto alice_coindays = uint128_t( alice_EUSD.amount.value ) * ( db.head_block_time() - start_time ).to_seconds();
      alice_EUSD = db.get_account( "alice" ).EUSDbalance;
      start_time = db.get_account( "alice" ).EUSD_seconds_last_update;

      BOOST_TEST_MESSAGE( "Testing longer interest period" );

      generate_blocks( db.head_block_time() + fc::seconds( ( EUSD_INTEREST_COMPOUND_INTERVAL_SEC * 7 ) / 3 ), true );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance.amount.value == alice_EUSD.amount.value - ASSET( "1.000 TBD" ).amount.value + ( ( ( ( uint128_t( alice_EUSD.amount.value ) * ( db.head_block_time() - start_time ).to_seconds() + alice_coindays ) / SECONDS_PER_YEAR ) * gpo.EUSD_interest_rate ) / PERCENT_100 ).to_uint64() );
      validate_database();
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( liquidity_rewards )
{
   using std::abs;

   try
   {
      db.liquidity_rewards_enabled = false;

      ACTORS( (alice)(bob)(sam)(dave) )
      generate_block();
      score( "alice", ASSET( "10.000 TESTS" ) );
      score( "bob", ASSET( "10.000 TESTS" ) );
      score( "sam", ASSET( "10.000 TESTS" ) );
      score( "dave", ASSET( "10.000 TESTS" ) );

      BOOST_TEST_MESSAGE( "Rewarding Bob with TESTS" );

      auto exchange_rate = price( ASSET( "1.250 TESTS" ), ASSET( "1.000 TBD" ) );
      set_price_feed( exchange_rate );

      signed_transaction tx;

      fund( "alice", ASSET( "25.522 TBD" ) );
      asset alice_EUSD = db.get_account( "alice" ).EUSDbalance;

      generate_block();

      fund( "alice", alice_EUSD.amount );
      fund( "bob", alice_EUSD.amount );
      fund( "sam", alice_EUSD.amount );
      fund( "dave", alice_EUSD.amount );

      int64_t alice_EUSD_volume = 0;
      int64_t alice_ECO_volume = 0;
      time_point_sec alice_reward_last_update = fc::time_point_sec::min();
      int64_t bob_EUSD_volume = 0;
      int64_t bob_ECO_volume = 0;
      time_point_sec bob_reward_last_update = fc::time_point_sec::min();
      int64_t sam_EUSD_volume = 0;
      int64_t sam_ECO_volume = 0;
      time_point_sec sam_reward_last_update = fc::time_point_sec::min();
      int64_t dave_EUSD_volume = 0;
      int64_t dave_ECO_volume = 0;
      time_point_sec dave_reward_last_update = fc::time_point_sec::min();

      BOOST_TEST_MESSAGE( "Creating Limit Order for ECO that will stay on the books for 30 minutes exactly." );

      limit_order_create_operation op;
      op.owner = "alice";
      op.amount_to_sell = asset( alice_EUSD.amount.value / 20, SYMBOL_EUSD ) ;
      op.min_to_receive = op.amount_to_sell * exchange_rate;
      op.orderid = 1;

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Waiting 10 minutes" );

      generate_blocks( db.head_block_time() + MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      BOOST_TEST_MESSAGE( "Creating Limit Order for EUSD that will be filled immediately." );

      op.owner = "bob";
      op.min_to_receive = op.amount_to_sell;
      op.amount_to_sell = op.min_to_receive * exchange_rate;
      op.fill_or_kill = false;
      op.orderid = 2;

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      alice_ECO_volume += ( asset( alice_EUSD.amount / 20, SYMBOL_EUSD ) * exchange_rate ).amount.value;
      alice_reward_last_update = db.head_block_time();
      bob_ECO_volume -= ( asset( alice_EUSD.amount / 20, SYMBOL_EUSD ) * exchange_rate ).amount.value;
      bob_reward_last_update = db.head_block_time();

      auto ops = get_last_operations( 1 );
      const auto& liquidity_idx = db.get_index< liquidity_reward_balance_index >().indices().get< by_owner >();
      const auto& limit_order_idx = db.get_index< limit_order_index >().indices().get< by_account >();

      auto reward = liquidity_idx.find( db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == alice_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == alice_ECO_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == bob_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == bob_ECO_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      auto fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_EUSD.amount.value / 20, SYMBOL_EUSD ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 2 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == ( asset( alice_EUSD.amount.value / 20, SYMBOL_EUSD ) * exchange_rate ).amount.value );

      BOOST_CHECK( limit_order_idx.find( std::make_tuple( "alice", 1 ) ) == limit_order_idx.end() );
      BOOST_CHECK( limit_order_idx.find( std::make_tuple( "bob", 2 ) ) == limit_order_idx.end() );

      BOOST_TEST_MESSAGE( "Creating Limit Order for EUSD that will stay on the books for 60 minutes." );

      op.owner = "sam";
      op.amount_to_sell = asset( ( alice_EUSD.amount.value / 20 ), SYMBOL_ECO );
      op.min_to_receive = asset( ( alice_EUSD.amount.value / 20 ), SYMBOL_EUSD );
      op.orderid = 3;

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Waiting 10 minutes" );

      generate_blocks( db.head_block_time() + MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      BOOST_TEST_MESSAGE( "Creating Limit Order for EUSD that will stay on the books for 30 minutes." );

      op.owner = "bob";
      op.orderid = 4;
      op.amount_to_sell = asset( ( alice_EUSD.amount.value / 10 ) * 3 - alice_EUSD.amount.value / 20, SYMBOL_ECO );
      op.min_to_receive = asset( ( alice_EUSD.amount.value / 10 ) * 3 - alice_EUSD.amount.value / 20, SYMBOL_EUSD );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Waiting 30 minutes" );

      generate_blocks( db.head_block_time() + MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      BOOST_TEST_MESSAGE( "Filling both limit orders." );

      op.owner = "alice";
      op.orderid = 5;
      op.amount_to_sell = asset( ( alice_EUSD.amount.value / 10 ) * 3, SYMBOL_EUSD );
      op.min_to_receive = asset( ( alice_EUSD.amount.value / 10 ) * 3, SYMBOL_ECO );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      alice_EUSD_volume -= ( alice_EUSD.amount.value / 10 ) * 3;
      alice_reward_last_update = db.head_block_time();
      sam_EUSD_volume += alice_EUSD.amount.value / 20;
      sam_reward_last_update = db.head_block_time();
      bob_EUSD_volume += ( alice_EUSD.amount.value / 10 ) * 3 - ( alice_EUSD.amount.value / 20 );
      bob_reward_last_update = db.head_block_time();
      ops = get_last_operations( 4 );

      fill_order_op = ops[1].get< fill_order_operation >();
      BOOST_REQUIRE( fill_order_op.open_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 4 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( ( alice_EUSD.amount.value / 10 ) * 3 - alice_EUSD.amount.value / 20, SYMBOL_ECO ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 5 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( ( alice_EUSD.amount.value / 10 ) * 3 - alice_EUSD.amount.value / 20, SYMBOL_EUSD ).amount.value );

      fill_order_op = ops[3].get< fill_order_operation >();
      BOOST_REQUIRE( fill_order_op.open_owner == "sam" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 3 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_EUSD.amount.value / 20, SYMBOL_ECO ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 5 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( alice_EUSD.amount.value / 20, SYMBOL_EUSD ).amount.value );

      reward = liquidity_idx.find( db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == alice_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == alice_ECO_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == bob_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == bob_ECO_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      BOOST_TEST_MESSAGE( "Testing a partial fill before minimum time and full fill after minimum time" );

      op.orderid = 6;
      op.amount_to_sell = asset( alice_EUSD.amount.value / 20 * 2, SYMBOL_EUSD );
      op.min_to_receive = asset( alice_EUSD.amount.value / 20 * 2, SYMBOL_ECO );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + fc::seconds( MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10.to_seconds() / 2 ), true );

      op.owner = "bob";
      op.orderid = 7;
      op.amount_to_sell = asset( alice_EUSD.amount.value / 20, SYMBOL_ECO );
      op.min_to_receive = asset( alice_EUSD.amount.value / 20, SYMBOL_EUSD );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + fc::seconds( MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10.to_seconds() / 2 ), true );

      ops = get_last_operations( 3 );
      fill_order_op = ops[2].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 6 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_EUSD.amount.value / 20, SYMBOL_EUSD ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 7 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( alice_EUSD.amount.value / 20, SYMBOL_ECO ).amount.value );

      reward = liquidity_idx.find( db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == alice_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == alice_ECO_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == bob_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == bob_ECO_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      generate_blocks( db.head_block_time() + MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      op.owner = "sam";
      op.orderid = 8;

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      alice_ECO_volume += alice_EUSD.amount.value / 20;
      alice_reward_last_update = db.head_block_time();
      sam_ECO_volume -= alice_EUSD.amount.value / 20;
      sam_reward_last_update = db.head_block_time();

      ops = get_last_operations( 2 );
      fill_order_op = ops[1].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 6 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_EUSD.amount.value / 20, SYMBOL_EUSD ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "sam" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 8 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( alice_EUSD.amount.value / 20, SYMBOL_ECO ).amount.value );

      reward = liquidity_idx.find( db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == alice_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == alice_ECO_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == bob_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == bob_ECO_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      BOOST_TEST_MESSAGE( "Trading to give Alice and Bob positive volumes to receive rewards" );

      transfer_operation transfer;
      transfer.to = "dave";
      transfer.from = "alice";
      transfer.amount = asset( alice_EUSD.amount / 2, SYMBOL_EUSD );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      op.owner = "alice";
      op.amount_to_sell = asset( 8 * ( alice_EUSD.amount.value / 20 ), SYMBOL_ECO );
      op.min_to_receive = asset( op.amount_to_sell.amount, SYMBOL_EUSD );
      op.orderid = 9;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      op.owner = "dave";
      op.amount_to_sell = asset( 7 * ( alice_EUSD.amount.value / 20 ), SYMBOL_EUSD );;
      op.min_to_receive = asset( op.amount_to_sell.amount, SYMBOL_ECO );
      op.orderid = 10;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      alice_EUSD_volume += op.amount_to_sell.amount.value;
      alice_reward_last_update = db.head_block_time();
      dave_EUSD_volume -= op.amount_to_sell.amount.value;
      dave_reward_last_update = db.head_block_time();

      ops = get_last_operations( 1 );
      fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 9 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == 7 * ( alice_EUSD.amount.value / 20 ) );
      BOOST_REQUIRE( fill_order_op.current_owner == "dave" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 10 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == 7 * ( alice_EUSD.amount.value / 20 ) );

      reward = liquidity_idx.find( db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == alice_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == alice_ECO_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == bob_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == bob_ECO_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "dave" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "dave" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == dave_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == dave_ECO_volume );
      BOOST_CHECK( reward->last_update == dave_reward_last_update );*/

      op.owner = "bob";
      op.amount_to_sell.amount = alice_EUSD.amount / 20;
      op.min_to_receive.amount = op.amount_to_sell.amount;
      op.orderid = 11;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      alice_EUSD_volume += op.amount_to_sell.amount.value;
      alice_reward_last_update = db.head_block_time();
      bob_EUSD_volume -= op.amount_to_sell.amount.value;
      bob_reward_last_update = db.head_block_time();

      ops = get_last_operations( 1 );
      fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 9 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == alice_EUSD.amount.value / 20 );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 11 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == alice_EUSD.amount.value / 20 );

      reward = liquidity_idx.find( db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == alice_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == alice_ECO_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == bob_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == bob_ECO_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "dave" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "dave" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == dave_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == dave_ECO_volume );
      BOOST_CHECK( reward->last_update == dave_reward_last_update );*/

      transfer.to = "bob";
      transfer.from = "alice";
      transfer.amount = asset( alice_EUSD.amount / 5, SYMBOL_EUSD );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 12;
      op.amount_to_sell = asset( 3 * ( alice_EUSD.amount / 40 ), SYMBOL_EUSD );
      op.min_to_receive = asset( op.amount_to_sell.amount, SYMBOL_ECO );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      op.owner = "dave";
      op.orderid = 13;
      op.amount_to_sell = op.min_to_receive;
      op.min_to_receive.symbol = SYMBOL_EUSD;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.sign( dave_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      bob_ECO_volume += op.amount_to_sell.amount.value;
      bob_reward_last_update = db.head_block_time();
      dave_ECO_volume -= op.amount_to_sell.amount.value;
      dave_reward_last_update = db.head_block_time();

      ops = get_last_operations( 1 );
      fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 12 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == 3 * ( alice_EUSD.amount.value / 40 ) );
      BOOST_REQUIRE( fill_order_op.current_owner == "dave" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 13 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == 3 * ( alice_EUSD.amount.value / 40 ) );

      reward = liquidity_idx.find( db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "alice" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == alice_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == alice_ECO_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "bob" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == bob_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == bob_ECO_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      reward = liquidity_idx.find( db.get_account( "dave" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db.get_account( "dave" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == dave_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == dave_ECO_volume );
      BOOST_CHECK( reward->last_update == dave_reward_last_update );*/

      auto alice_balance = db.get_account( "alice" ).balance;
      auto bob_balance = db.get_account( "bob" ).balance;
      auto sam_balance = db.get_account( "sam" ).balance;
      auto dave_balance = db.get_account( "dave" ).balance;

      BOOST_TEST_MESSAGE( "Generating Blocks to trigger liquidity rewards" );

      db.liquidity_rewards_enabled = true;
      generate_blocks( LIQUIDITY_REWARD_BLOCKS - ( db.head_block_num() % LIQUIDITY_REWARD_BLOCKS ) - 1 );

      BOOST_REQUIRE( db.head_block_num() % LIQUIDITY_REWARD_BLOCKS == LIQUIDITY_REWARD_BLOCKS - 1 );
      BOOST_REQUIRE( db.get_account( "alice" ).balance.amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).balance.amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).balance.amount.value == sam_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).balance.amount.value == dave_balance.amount.value );

      generate_block();

      //alice_balance += MIN_LIQUIDITY_REWARD;

      BOOST_REQUIRE( db.get_account( "alice" ).balance.amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).balance.amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).balance.amount.value == sam_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).balance.amount.value == dave_balance.amount.value );

      ops = get_last_operations( 1 );

      REQUIRE_THROW( ops[0].get< liquidity_reward_operation>(), fc::exception );
      //BOOST_REQUIRE( ops[0].get< liquidity_reward_operation>().payout.amount.value == MIN_LIQUIDITY_REWARD.amount.value );

      generate_blocks( LIQUIDITY_REWARD_BLOCKS );

      //bob_balance += MIN_LIQUIDITY_REWARD;

      BOOST_REQUIRE( db.get_account( "alice" ).balance.amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "bob" ).balance.amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "sam" ).balance.amount.value == sam_balance.amount.value );
      BOOST_REQUIRE( db.get_account( "dave" ).balance.amount.value == dave_balance.amount.value );

      ops = get_last_operations( 1 );

      REQUIRE_THROW( ops[0].get< liquidity_reward_operation>(), fc::exception );
      //BOOST_REQUIRE( ops[0].get< liquidity_reward_operation>().payout.amount.value == MIN_LIQUIDITY_REWARD.amount.value );

      alice_ECO_volume = 0;
      alice_EUSD_volume = 0;
      bob_ECO_volume = 0;
      bob_EUSD_volume = 0;

      BOOST_TEST_MESSAGE( "Testing liquidity timeout" );

      generate_blocks( sam_reward_last_update + LIQUIDITY_TIMEOUT_SEC - fc::seconds( BLOCK_INTERVAL / 2 ) - MIN_LIQUIDITY_REWARD_PERIOD_SEC , true );

      op.owner = "sam";
      op.orderid = 14;
      op.amount_to_sell = ASSET( "1.000 TESTS" );
      op.min_to_receive = ASSET( "1.000 TBD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + ( BLOCK_INTERVAL / 2 ) + LIQUIDITY_TIMEOUT_SEC, true );

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      /*BOOST_REQUIRE( reward == liquidity_idx.end() );
      BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      generate_block();

      op.owner = "alice";
      op.orderid = 15;
      op.amount_to_sell.symbol = SYMBOL_EUSD;
      op.min_to_receive.symbol = SYMBOL_ECO;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      sam_EUSD_volume = ASSET( "1.000 TBD" ).amount.value;
      sam_ECO_volume = 0;
      sam_reward_last_update = db.head_block_time();

      reward = liquidity_idx.find( db.get_account( "sam" ).id );
      /*BOOST_REQUIRE( reward == liquidity_idx.end() );
      BOOST_REQUIRE( reward->owner == db.get_account( "sam" ).id );
      BOOST_REQUIRE( reward->EUSD_volume == sam_EUSD_volume );
      BOOST_REQUIRE( reward->ECO_volume == sam_ECO_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/
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

      BOOST_REQUIRE( db.get_comment( "alice", string( "test1" ) ).reward_weight == PERCENT_100 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      op.permlink = "test2";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test2" ) ).reward_weight == PERCENT_100 );

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      tx.operations.clear();
      tx.signatures.clear();

      op.permlink = "test3";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test3" ) ).reward_weight == PERCENT_100 );

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      tx.operations.clear();
      tx.signatures.clear();

      op.permlink = "test4";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test4" ) ).reward_weight == PERCENT_100 );

      generate_blocks( db.head_block_time() + MIN_ROOT_COMMENT_INTERVAL + fc::seconds( BLOCK_INTERVAL ), true );

      tx.operations.clear();
      tx.signatures.clear();

      op.permlink = "test5";

      tx.operations.push_back( op );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test5" ) ).reward_weight == PERCENT_100 );
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

      auto exchange_rate = price( ASSET( "1.250 TESTS" ), ASSET( "1.000 TBD" ) );
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

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).last_payout == fc::time_point_sec::min() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time != fc::time_point_sec::min() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time != fc::time_point_sec::maximum() );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time, true );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).last_payout == db.head_block_time() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point_sec::maximum() );

      vote.voter = "sam";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point_sec::maximum() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value == 0 );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).abs_ESCORreward.value == 0 );

      vote.voter = "bob";
      vote.weight = PERCENT_100 * -1;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point_sec::maximum() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value == 0 );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).abs_ESCORreward.value == 0 );

      vote.voter = "dave";
      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( dave_private_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).cashout_time == fc::time_point_sec::maximum() );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).net_ESCORreward.value == 0 );
      BOOST_REQUIRE( db.get_comment( "alice", string( "test" ) ).abs_ESCORreward.value == 0 );

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
BOOST_AUTO_TEST_CASE( EUSD_stability )
{
   try
   {
      resize_shared_mem( 1024 * 1024 * 512 ); // Due to number of blocks in the test, it requires a large file. (64 MB)

      // Using the debug node plugin to manually set account balances to create required market conditions for this test
      auto db_plugin = app.register_plugin< eznode::plugin::debug_node::debug_node_plugin >();
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

      auto exchange_rate = price( ASSET( "1.000 TBD" ), ASSET( "10.000 TESTS" ) );
      set_price_feed( exchange_rate );

      BOOST_REQUIRE( db.get_dynamic_global_properties().EUSD_print_rate == PERCENT_100 );

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

      db_plugin->debug_generate_blocks_until( debug_key, fc::time_point_sec( db.get_comment( comment.author, comment.permlink ).cashout_time.sec_since_epoch() - 2 * BLOCK_INTERVAL ), true, database::skip_witness_signature );

      auto& gpo = db.get_dynamic_global_properties();

      BOOST_TEST_MESSAGE( "Changing sam and gpo to set up market cap conditions" );

      asset EUSDbalance = asset( ( gpo.virtual_supply.amount * ( EUSD_STOP_PERCENT + 30 ) ) / PERCENT_100, SYMBOL_ECO ) * exchange_rate;
      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_account( "sam" ), [&]( account_object& a )
         {
            a.EUSDbalance = EUSDbalance;
         });
      }, database::skip_witness_signature );

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& gpo )
         {
            gpo.current_EUSD_supply = EUSDbalance;
            gpo.virtual_supply = gpo.virtual_supply + EUSDbalance * exchange_rate;
         });
      }, database::skip_witness_signature );

      validate_database();

      db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );

      auto comment_reward = ( gpo.total_reward_fund_ECO.amount + 2000 ) - ( ( gpo.total_reward_fund_ECO.amount + 2000 ) * 25 * PERCENT_1 ) / PERCENT_100 ;
      comment_reward /= 2;
      auto EUSDreward = ( comment_reward * gpo.EUSD_print_rate ) / PERCENT_100;
      auto alice_EUSD = db.get_account( "alice" ).EUSDbalance + db.get_account( "alice" ).EUSDrewardbalance + asset( EUSDreward, SYMBOL_ECO ) * exchange_rate;
      auto alice_ECO = db.get_account( "alice" ).balance + db.get_account( "alice" ).ECOrewardBalance ;

      BOOST_TEST_MESSAGE( "Checking printing EUSD has slowed" );
      BOOST_REQUIRE( db.get_dynamic_global_properties().EUSD_print_rate < PERCENT_100 );

      BOOST_TEST_MESSAGE( "Pay out comment and check rewards are paid as ECO" );
      db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );

      validate_database();

      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance + db.get_account( "alice" ).EUSDrewardbalance == alice_EUSD );
      BOOST_REQUIRE( db.get_account( "alice" ).balance + db.get_account( "alice" ).ECOrewardBalance > alice_ECO );

      BOOST_TEST_MESSAGE( "Letting percent market cap fall to 2% to verify printing of EUSD turns back on" );

      // Get close to 1.5% for printing EUSD to start again, but not all the way
      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_account( "sam" ), [&]( account_object& a )
         {
            a.EUSDbalance = asset( ( 194 * EUSDbalance.amount ) / 500, SYMBOL_EUSD );
         });
      }, database::skip_witness_signature );

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& gpo )
         {
            gpo.current_EUSD_supply = alice_EUSD + asset( ( 194 * EUSDbalance.amount ) / 500, SYMBOL_EUSD );
         });
      }, database::skip_witness_signature );

      db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );
      validate_database();

      BOOST_REQUIRE( db.get_dynamic_global_properties().EUSD_print_rate < PERCENT_100 );

      auto last_print_rate = db.get_dynamic_global_properties().EUSD_print_rate;

      // Keep producing blocks until printing EUSD is back
      while( ( db.get_dynamic_global_properties().current_EUSD_supply * exchange_rate ).amount >= ( db.get_dynamic_global_properties().virtual_supply.amount * EUSD_START_PERCENT ) / PERCENT_100 )
      {
         auto& gpo = db.get_dynamic_global_properties();
         BOOST_REQUIRE( gpo.EUSD_print_rate >= last_print_rate );
         last_print_rate = gpo.EUSD_print_rate;
         db_plugin->debug_generate_blocks( debug_key, 1, database::skip_witness_signature );
         validate_database();
      }

      validate_database();

      BOOST_REQUIRE( db.get_dynamic_global_properties().EUSD_print_rate == PERCENT_100 );
   }
   FC_LOG_AND_RETHROW()
}
#endif

BOOST_AUTO_TEST_CASE( EUSD_price_feed_limit )
{
   try
   {
      ACTORS( (alice) );
      generate_block();
      score( "alice", ASSET( "10.000 TESTS" ) );

      price exchange_rate( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) );
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

      BOOST_TEST_MESSAGE( "Setting EUSD percent to greater than 10% market cap." );

      db.skip_price_feed_limit_check = false;
      const auto& gpo = db.get_dynamic_global_properties();
      auto new_exchange_rate = price( gpo.current_EUSD_supply, asset( ( PERCENT_100 ) * gpo.current_supply.amount ) );
      set_price_feed( new_exchange_rate );
      set_price_feed( new_exchange_rate );

      BOOST_REQUIRE( db.get_feed_history().current_median_history > new_exchange_rate && db.get_feed_history().current_median_history < exchange_rate );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( clear_null_account )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing clearing the null account's balances on block" );

      ACTORS( (alice) );
      generate_block();

      set_price_feed( price( ASSET( "1.000 TESTS" ), ASSET( "1.000 TBD" ) ) );

      fund( "alice", ASSET( "10.000 TESTS" ) );
      fund( "alice", ASSET( "10.000 TBD" ) );

      transfer_operation transfer1;
      transfer1.from = "alice";
      transfer1.to = NULL_ACCOUNT;
      transfer1.amount = ASSET( "1.000 TESTS" );

      transfer_operation transfer2;
      transfer2.from = "alice";
      transfer2.to = NULL_ACCOUNT;
      transfer2.amount = ASSET( "2.000 TBD" );

      transferECOtoESCORfund_operation score;
      score.from = "alice";
      score.to = NULL_ACCOUNT;
      score.amount = ASSET( "3.000 TESTS" );

      transferToSavings_operation save1;
      save1.from = "alice";
      save1.to = NULL_ACCOUNT;
      save1.amount = ASSET( "4.000 TESTS" );

      transferToSavings_operation save2;
      save2.from = "alice";
      save2.to = NULL_ACCOUNT;
      save2.amount = ASSET( "5.000 TBD" );

      BOOST_TEST_MESSAGE( "--- Transferring to NULL Account" );

      signed_transaction tx;
      tx.operations.push_back( transfer1 );
      tx.operations.push_back( transfer2 );
      tx.operations.push_back( score );
      tx.operations.push_back( save1);
      tx.operations.push_back( save2 );
      tx.set_expiration( db.head_block_time() + MAX_TIME_UNTIL_EXPIRATION );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_account( NULL_ACCOUNT ), [&]( account_object& a )
         {
            a.ECOrewardBalance = ASSET( "1.000 TESTS" );
            a.EUSDrewardbalance = ASSET( "1.000 TBD" );
            a.ESCORrewardBalance = ASSET( "1.000000 TP" );
            a.ESCORrewardBalance = ASSET( "1.000 TESTS" );
         });

         db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& gpo )
         {
            gpo.current_supply += ASSET( "2.000 TESTS" );
            gpo.virtual_supply += ASSET( "3.000 TESTS" );
            gpo.current_EUSD_supply += ASSET( "1.000 TBD" );
            gpo.pending_rewarded_ESCOR += ASSET( "1.000000 TP" );
            gpo.pending_rewarded_ESCORvalueInECO += ASSET( "1.000 TESTS" );
         });
      });

      validate_database();

      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).balance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).EUSDbalance == ASSET( "2.000 TBD" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ESCOR > ASSET( "0.000000 TP" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ECOsavingsBalance == ASSET( "4.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).EUSDsavingsBalance == ASSET( "5.000 TBD" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).EUSDrewardbalance == ASSET( "1.000 TBD" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ECOrewardBalance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ESCORrewardBalance == ASSET( "1.000000 TP" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ESCORrewardBalance == ASSET( "1.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "2.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance == ASSET( "3.000 TBD" ) );

      BOOST_TEST_MESSAGE( "--- Generating block to clear balances" );
      generate_block();
      validate_database();

      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).EUSDbalance == ASSET( "0.000 TBD" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ESCOR == ASSET( "0.000000 TP" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ECOsavingsBalance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).EUSDsavingsBalance == ASSET( "0.000 TBD" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).EUSDrewardbalance == ASSET( "0.000 TBD" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ECOrewardBalance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ESCORrewardBalance == ASSET( "0.000000 TP" ) );
      BOOST_REQUIRE( db.get_account( NULL_ACCOUNT ).ESCORrewardBalance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).balance == ASSET( "2.000 TESTS" ) );
      BOOST_REQUIRE( db.get_account( "alice" ).EUSDbalance == ASSET( "3.000 TBD" ) );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
#endif
