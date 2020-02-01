//#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <node/protocol/exceptions.hpp>

#include <node/chain/block_summary_object.hpp>
#include <node/chain/database.hpp>
//#include <node/chain/hardfork.hpp>
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

BOOST_FIXTURE_TEST_SUITE( processing_tests, clean_database_fixture )



   //==========================//
   // === Processing Tests === //
   //==========================//



BOOST_AUTO_TEST_CASE( comment_processing_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMENT PROCESSING" );
      
      BOOST_TEST_MESSAGE( "│   ├── Testing: Comment payout received in equal amounts for equal voting power" );

      ACTORS( (alice)(bob)(dan)(ulysses)(vivian)(wendy) );

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

      authors.emplace_back( "alice", alice_private_posting_key );
      authors.emplace_back( "bob", bob_private_posting_key, ASSET( "0.00000000 USD" ) );
      authors.emplace_back( "dan", dan_private_posting_key );
      voters.emplace_back( "ulysses", ulysses_private_posting_key, "alice");
      voters.emplace_back( "vivian", vivian_private_posting_key, "bob"  );
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

         comment_operation com;

         com.signatory = author.name;
         com.author = author.name;
         com.permlink = "mypost";
         com.parent_author = ROOT_POST_PARENT;
         com.parent_permlink = "test";
         com.title = "Hello from "+author.name;
         com.body = "Hello, my name is "+author.name;
         
         if( author.max_accepted_payout.valid() )
         {
            comment_options copt;

            copt.max_accepted_payout = *( author.max_accepted_payout );
            com.options = copt;
         }

         tx.operations.push_back( com );

         tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( author.private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      }

      generate_blocks(1);

      // voters all vote in the same block with the same stake
      for( const auto& voter : voters )
      {
         signed_transaction tx;

         vote_operation vote;

         vote.signatory = voter.name;
         vote.voter = voter.name;
         vote.author = voter.favorite_author;
         vote.permlink = "mypost";
         vote.weight = PERCENT_100;
         tx.operations.push_back( vote );
         tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( voter.private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      }

      generate_blocks(10);

      generate_blocks( db.get_comment( "alice", string( "mypost" ) ).cashout_time, true );

      for( const auto& author : authors )
      {
         const account_object& a = db.get_account( author.name );
         const account_balance_object& bal = db.get_account_balance( author.name, SYMBOL_COIN );
         ilog( "${n} : ${r}", ("n", author.name)("r", bal.reward_balance) );
      }
      for( const auto& voter : voters )
      {
         const account_object& a = db.get_account( voter.name );
         const account_balance_object& bal = db.get_account_balance( voter.name, SYMBOL_COIN );
         ilog( "${n} : ${r}", ("n", voter.name)("r", bal.reward_balance) );
      }
      
      const account_balance_object& alice_account_balance = db.get_account_balance( "alice", SYMBOL_COIN );
      const account_balance_object& bob_account_balance   = db.get_account_balance( "bob", SYMBOL_COIN );
      const account_balance_object& dan_account_balance  = db.get_account_balance( "dan", SYMBOL_COIN );

      BOOST_CHECK( alice_account_balance.reward_balance.value > 0);
      BOOST_CHECK( bob_account_balance.reward_balance.value == 0 );
      BOOST_CHECK( dan_account_balance.get_reward_balance() == alice_account_balance.get_reward_balance() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Comment payout received in equal amounts for equal voting power" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMENT PAYOUT PROCESS" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_payout_dust_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMENT DUST THRESHOLD" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: comment payout dust threshold" );

      ACTORS( (alice)(bob) );
      generate_block();

      fund( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 100000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_block();
      validate_database();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";

      vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.weight = 81 * PERCENT_1;

      
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      comment.author = "bob";
      vote.voter = "bob";
      vote.author = "bob";
      vote.weight = 59 * PERCENT_1;

      tx.clear();
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time );

      // If comments are paid out independent of order, then the last satoshi of TME cannot be divided among them
      const reward_fund_object rf = db.get_reward_fund();

      BOOST_REQUIRE( rf.content_reward_balance.amount == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: comment payout dust threshold" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMENT DUST THRESHOLD" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( recent_content_claims_decay_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CONTENT CLAIMS DECAY" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Two comments with overlapping decay from reward fund" );

      ACTORS( (alice)(bob) )
      generate_block();

      comment_operation comment;

      vote_operation vote;

      signed_transaction tx;

      comment.signatory = "alice";
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
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const reward_fund_object& rf = db.get_reward_fund();
      const median_chain_property_object& median_props = db.get_median_chain_properties();

      const comment_object& alice_comment = db.get_comment( "alice", string( "test" ) );

      uint128_t alice_reward_curve = util::evaluate_reward_curve(
         uint128_t( alice_comment.net_reward ),
         alice_comment.cashouts_received,
         rf.author_reward_curve,
         median_props.content_reward_decay_rate,
         rf.content_constant );

      generate_blocks( 5 );

      comment.author = "bob";
      vote.voter = "bob";
      vote.author = "bob";
      tx.clear();
      tx.operations.push_back( comment );
      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "alice", string( "test" ) ).cashout_time );

      BOOST_REQUIRE( rf.recent_content_claims == alice_reward_curve );
      validate_database();

      const comment_object& bob_comment = db.get_comment( "bob", string( "test" ) );
      
      auto bob_cashout_time = bob_comment.cashout_time;

      auto bob_reward_curve = util::evaluate_reward_curve( 
         uint128_t( bob_comment.net_reward ),
         bob_comment.cashouts_received,
         rf.author_reward_curve,
         median_props.content_reward_decay_rate,
         rf.content_constant );

      generate_block();

      while( db.head_block_time() < bob_cashout_time )
      {
         alice_reward_curve -= ( alice_reward_curve * BLOCK_INTERVAL.to_seconds() ) / RECENT_REWARD_DECAY_RATE.to_seconds();
         const auto& post_rf = db.get_reward_fund();

         BOOST_REQUIRE( post_rf.recent_content_claims == alice_reward_curve );

         generate_block();
      }

      alice_reward_curve -= ( alice_reward_curve * BLOCK_INTERVAL.to_seconds() ) / RECENT_REWARD_DECAY_RATE.to_seconds();
      const auto& post_rf = db.get_reward_fund();

      BOOST_REQUIRE( post_rf.recent_content_claims == alice_reward_curve + bob_reward_curve );
      validate_database();
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: Two comments with overlapping decay from reward fund" );

      BOOST_TEST_MESSAGE( "├── Passed: CONTENT CLAIMS DECAY" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( comment_payout_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMENT PAYOUT" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Reward fund correctly allocates content rewards for post payouts" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 7500 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 7500 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "candice", asset( 8000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 8000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "dan", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_author = "";
      comment.parent_permlink = "test";
      comment.title = "foo";
      comment.body = "bar";

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

      const reward_fund_object& rf = db.get_reward_fund();
      const comment_object& alice_comment = db.get_comment( "alice", string( "test" ) );
      const comment_object& bob_comment = db.get_comment( "bob", string( "test" ) );

      generate_blocks( alice_comment.cashout_time - BLOCK_INTERVAL, true );

      uint128_t recent_content_claims = rf.recent_content_claims;

      asset reward = rf.content_reward_balance;

      BOOST_REQUIRE( recent_content_claims == 0 );

      generate_block();

      util::comment_reward_context ctx = db.get_comment_reward_context( rf );

      util::fill_comment_reward_context_local_state( ctx, alice_comment );
      uint128_t alice_reward = util::get_comment_reward( ctx );

      util::fill_comment_reward_context_local_state( ctx, bob_comment );
      uint128_t bob_reward = util::get_comment_reward( ctx );

      asset alice_comment_payout = asset( alice_reward.to_uint64(), SYMBOL_COIN );
      asset bob_comment_payout = asset( bob_reward.to_uint64(), SYMBOL_COIN );

      BOOST_REQUIRE( rf.content_reward_balance == reward - alice_comment_payout );

      recent_content_claims = rf.recent_content_claims;
      reward = rf.content_reward_balance;

      generate_block();

      util::comment_reward_context ctx = db.get_comment_reward_context( rf );

      util::fill_comment_reward_context_local_state( ctx, alice_comment );
      uint128_t alice_reward = util::get_comment_reward( ctx );

      util::fill_comment_reward_context_local_state( ctx, bob_comment );
      uint128_t bob_reward = util::get_comment_reward( ctx );

      asset alice_comment_payout = asset( alice_reward.to_uint64(), SYMBOL_COIN );
      asset bob_comment_payout = asset( bob_reward.to_uint64(), SYMBOL_COIN );

      BOOST_REQUIRE( rf.content_reward_balance == reward - bob_comment_payout );

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

      const comment_object& candice_comment = db.get_comment( "candice", string( "test" ) );
      const comment_object& dan_comment = db.get_comment( "dan", string( "test" ) );

      reward = rf.content_reward_balance;

      generate_blocks( candice_comment.cashout_time, true );
      
      util::comment_reward_context ctx = db.get_comment_reward_context( rf );

      util::fill_comment_reward_context_local_state( ctx, candice_comment );
      uint128_t candice_reward = util::get_comment_reward( ctx );

      util::fill_comment_reward_context_local_state( ctx, dan_comment );
      uint128_t dan_reward = util::get_comment_reward( ctx );

      asset candice_comment_payout = asset( candice_reward.to_uint64(), SYMBOL_COIN );
      asset dan_comment_payout = asset( dan_reward.to_uint64(), SYMBOL_COIN );
   
      BOOST_REQUIRE( rf.content_reward_balance == reward - candice_comment_payout - dan_comment_payout );

      BOOST_REQUIRE( alice_comment.total_payout_value == db.asset_to_USD( alice_comment_payout ) );
      BOOST_REQUIRE( bob_comment.total_payout_value == db.asset_to_USD( bob_comment_payout ) );
      BOOST_REQUIRE( candice_comment.total_payout_value == db.asset_to_USD( candice_comment_payout ) );
      BOOST_REQUIRE( dan_comment.total_payout_value == db.asset_to_USD( dan_comment_payout ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Nested Comments Payout" );
      
      BOOST_TEST_MESSAGE( "├── PASSED: COMMENT PAYOUT" );
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

      ACTORS( (alice0)(alice1)(alice2)(alice3)(alice4)(alice5)(alice6) )

      generate_blocks( fc::seconds( 30 ).count() / BLOCK_INTERVAL.count() );

      vector< string > accounts;

      accounts.push_back( "alice0" );
      accounts.push_back( "alice1" );
      accounts.push_back( "alice2" );
      accounts.push_back( "alice3" );
      accounts.push_back( "alice4" );
      accounts.push_back( "alice5" );
      accounts.push_back( "alice6" );

      vector< private_key_type > keys;
      keys.push_back( alice0_private_active_key );
      keys.push_back( alice1_private_active_key );
      keys.push_back( alice2_private_active_key );
      keys.push_back( alice3_private_active_key );
      keys.push_back( alice4_private_active_key );
      keys.push_back( alice5_private_active_key );
      keys.push_back( alice6_private_active_key );

      vector< asset_publish_feed_operation > ops;
      vector< signed_transaction > txs;

      // Upgrade accounts to producers

      for( int i = 0; i < 7; i++ )
      {
         fund( accounts[i], asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         fund_stake( accounts[i], asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
         producer_create( accounts[i], keys[i], keys[i].get_public_key() );
         ops.push_back( asset_publish_feed_operation() );
         ops[i].publisher = accounts[i];
         ops[i].symbol = SYMBOL_USD;

         txs.push_back( signed_transaction() );
      }

      ops[0].feed.settlement_price = price( asset( 500, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      ops[1].feed.settlement_price = price( asset( 550, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      ops[2].feed.settlement_price = price( asset( 600, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      ops[3].feed.settlement_price = price( asset( 650, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );   // Median price
      ops[4].feed.settlement_price = price( asset( 700, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      ops[5].feed.settlement_price = price( asset( 750, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );
      ops[6].feed.settlement_price = price( asset( 800, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );

      for( int i = 0; i < 7; i++ )
      {
         txs[i].set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         txs[i].operations.push_back( ops[i] );
         txs[i].sign( keys[i], db.get_chain_id() );
         db.push_transaction( txs[i], 0 );
      }

      generate_blocks( BLOCKS_PER_HOUR ); // Jump forward 1 hour

      const asset_bitasset_data_object& bitasset = db.get_bitasset_data( SYMBOL_USD );

      BOOST_REQUIRE( bitasset.current_feed.settlement_price == price( asset( 650, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) ) );

      BOOST_REQUIRE( bitasset.feeds.size() == 7 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Selection of median price feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Updating median price feed" );

      for ( int i = 0; i < 23; i++ )
      {
         for( int j = 0; j < 7; j++ )
         {
            txs[j].operations.clear();
            txs[j].signatures.clear();

            ops[j].feed.settlement_price = price( 
               asset( ops[j].feed.settlement_price.base.amount + 10, SYMBOL_USD ), 
               asset( ops[j].feed.settlement_price.quote.amount, SYMBOL_COIN ) );

            txs[j].set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
            txs[j].operations.push_back( ops[j] );
            txs[j].sign( keys[j], db.get_chain_id() );
            db.push_transaction( txs[j], 0 );
         }

         generate_blocks( BLOCKS_PER_HOUR  ); // Jump forward 1 hour

         BOOST_REQUIRE( bitasset.current_feed.settlement_price == ops[4].feed.settlement_price );

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

      ACTORS( (alice) );

      generate_block();

      fund( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );
      fund_savings( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_CREDIT ) );

      time_point now = db.head_block_time();

      signed_transaction tx;

      asset init_liquid_balance = db.get_liquid_balance( "alice", SYMBOL_CREDIT );
      asset init_staked_balance = db.get_staked_balance( "alice", SYMBOL_CREDIT );
      asset init_savings_balance = db.get_savings_balance( "alice", SYMBOL_CREDIT );

      const asset_credit_data_object& credit = db.get_credit_data( SYMBOL_CREDIT );
      asset_symbol_type cs = credit.symbol;
      const asset_dynamic_data_object& dyn_data = db.get_dynamic_data( cs );
      price buyback = credit.buyback_price;
      price market = db.get_liquidity_pool( credit.buyback_asset, credit.symbol ).base_hour_median_price( credit.buyback_asset );
      
      asset unit = asset( BLOCKCHAIN_PRECISION, credit.symbol );
      share_type range = credit.var_interest_range;     // Percentage range that caps the price divergence between market and buyback
      share_type pr = PERCENT_100;
      share_type hpr = PERCENT_100 / 2;

      share_type mar = ( market * unit ).amount;    // Market price of the credit asset
      share_type buy = ( buyback * unit ).amount;   // Buyback price of the credit asset

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
//#endif