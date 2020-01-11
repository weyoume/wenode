#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <node/chain/account_object.hpp>
#include <node/chain/comment_object.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/market_history/market_history_plugin.hpp>

#include "../common/database_fixture.hpp"

using namespace node::chain;
using namespace node::protocol;

BOOST_FIXTURE_TEST_SUITE( market_history, clean_database_fixture )

BOOST_AUTO_TEST_CASE( mh_test )
{
   using namespace node::market_history;

   try
   {
      auto mh_plugin = app.register_plugin< market_history_plugin >();
      boost::program_options::variables_map options;
      mh_plugin->plugin_initialize( options );

      ACTORS( (alice)(bob)(sam) );
      fund( "alice", 1000000 );
      fund( "bob", 1000000 );
      fund( "sam", 1000000 );

      set_price_feed( price( ASSET( "0.500 TESTS" ), ASSET( "1.000 USD" ) ) );

      signed_transaction tx;
      comment_operation comment;
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "foo";
      comment.body = "bar";
      tx.operations.push_back( comment );

      vote_operation vote;
      vote.voter = "alice";
      vote.weight = PERCENT_100;
      vote.author = "alice";
      vote.permlink = "test";
      tx.operations.push_back( vote );

      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.get_comment( "alice", std::string( "test" ) ).cashout_time );

      const auto& duration_idx = db.get_index< market_duration_index >().indices().get< by_duration >();
      const auto& order_hist_idx = db.get_index< order_history_index >().indices().get< by_id >();

      BOOST_REQUIRE( duration_idx.begin() == duration_idx.end() );
      BOOST_REQUIRE( order_hist_idx.begin() == order_hist_idx.end() );
      validate_database();

      tx.operations.clear();
      tx.signatures.clear();

      auto fill_order_a_time = db.head_block_time();
      auto time_a = fc::time_point( ( fill_order_a_time.time_since_epoch() / 15 ) * 15 );

      limit_order_operation op;
      op.owner = "alice";
      op.amount_to_sell = ASSET( "1.000 USD" );
      op.min_to_receive = ASSET( "2.000 TESTS" );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx,  0 );

      tx.operations.clear();
      tx.signatures.clear();

      op.owner = "bob";
      op.amount_to_sell = ASSET( "1.500 TESTS" );
      op.min_to_receive = ASSET( "0.750 USD" );
      tx.operations.push_back( op );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + ( 60 * 90 ) );

      auto fill_order_b_time = db.head_block_time();

      tx.operations.clear();
      tx.signatures.clear();

      op.owner = "sam";
      op.amount_to_sell = ASSET( "1.000 TESTS" );
      op.min_to_receive = ASSET( "0.500 USD" );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( sam_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( db.head_block_time() + 60 );

      auto fill_order_c_time = db.head_block_time();

      tx.operations.clear();
      tx.signatures.clear();

      op.owner = "alice";
      op.amount_to_sell = ASSET( "0.500 USD" );
      op.min_to_receive = ASSET( "0.900 TESTS" );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      op.owner = "bob";
      op.amount_to_sell = ASSET( "0.450 TESTS" );
      op.min_to_receive = ASSET( "0.250 USD" );
      tx.operations.push_back( op );
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      auto duration = duration_idx.begin();

      BOOST_REQUIRE( duration->seconds == 15 );
      BOOST_REQUIRE( duration->open == time_a );
      BOOST_REQUIRE( duration->high_TME == ASSET( "1.500 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "1.500 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.750 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 15 );
      BOOST_REQUIRE( duration->open == time_a + ( 60 * 90 ) );
      BOOST_REQUIRE( duration->high_TME == ASSET( "0.500 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "0.500 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.250 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 15 );
      BOOST_REQUIRE( duration->open == time_a + ( 60 * 90 ) + 60 );
      BOOST_REQUIRE( duration->high_TME == ASSET( "0.450 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "0.450 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "0.950 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.500 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 60 );
      BOOST_REQUIRE( duration->open == time_a );
      BOOST_REQUIRE( duration->high_TME == ASSET( "1.500 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "1.500 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.750 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 60 );
      BOOST_REQUIRE( duration->open == time_a + ( 60 * 90 ) );
      BOOST_REQUIRE( duration->high_TME == ASSET( "0.500 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "0.500 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.250 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 60 );
      BOOST_REQUIRE( duration->open == time_a + ( 60 * 90 ) + 60 );
      BOOST_REQUIRE( duration->high_TME == ASSET( "0.450 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "0.450 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "0.950 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.500 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 300 );
      BOOST_REQUIRE( duration->open == time_a );
      BOOST_REQUIRE( duration->high_TME == ASSET( "1.500 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "1.500 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.750 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 300 );
      BOOST_REQUIRE( duration->open == time_a + ( 60 * 90 ) );
      BOOST_REQUIRE( duration->high_TME == ASSET( "0.450 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "0.450 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "1.450 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.750 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 3600 );
      BOOST_REQUIRE( duration->open == time_a );
      BOOST_REQUIRE( duration->high_TME == ASSET( "1.500 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "1.500 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.750 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 3600 );
      BOOST_REQUIRE( duration->open == time_a + ( 60 * 60 ) );
      BOOST_REQUIRE( duration->high_TME == ASSET( "0.450 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "0.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "0.450 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "1.450 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "0.750 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration->seconds == 86400 );
      BOOST_REQUIRE( duration->open == GENESIS_TIME );
      BOOST_REQUIRE( duration->high_TME == ASSET( "0.450 TESTS " ).amount );
      BOOST_REQUIRE( duration->high_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->low_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->low_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->open_TME == ASSET( "1.500 TESTS" ).amount );
      BOOST_REQUIRE( duration->open_USD == ASSET( "0.750 USD" ).amount );
      BOOST_REQUIRE( duration->close_TME == ASSET( "0.450 TESTS").amount );
      BOOST_REQUIRE( duration->close_USD == ASSET( "0.250 USD" ).amount );
      BOOST_REQUIRE( duration->TME_volume == ASSET( "2.950 TESTS" ).amount );
      BOOST_REQUIRE( duration->USD_volume == ASSET( "1.500 USD" ).amount );
      duration++;

      BOOST_REQUIRE( duration == duration_idx.end() );

      auto order = order_hist_idx.begin();

      BOOST_REQUIRE( order->time == fill_order_a_time );
      BOOST_REQUIRE( order->op.current_owner == "bob" );
      BOOST_REQUIRE( order->op.current_orderid == 0 );
      BOOST_REQUIRE( order->op.current_pays == ASSET( "1.500 TESTS" ) );
      BOOST_REQUIRE( order->op.open_owner == "alice" );
      BOOST_REQUIRE( order->op.open_orderid == 0 );
      BOOST_REQUIRE( order->op.open_pays == ASSET( "0.750 USD" ) );
      order++;

      BOOST_REQUIRE( order->time == fill_order_b_time );
      BOOST_REQUIRE( order->op.current_owner == "sam" );
      BOOST_REQUIRE( order->op.current_orderid == 0 );
      BOOST_REQUIRE( order->op.current_pays == ASSET( "0.500 TESTS" ) );
      BOOST_REQUIRE( order->op.open_owner == "alice" );
      BOOST_REQUIRE( order->op.open_orderid == 0 );
      BOOST_REQUIRE( order->op.open_pays == ASSET( "0.250 USD" ) );
      order++;

      BOOST_REQUIRE( order->time == fill_order_c_time );
      BOOST_REQUIRE( order->op.current_owner == "alice" );
      BOOST_REQUIRE( order->op.current_orderid == 0 );
      BOOST_REQUIRE( order->op.current_pays == ASSET( "0.250 USD" ) );
      BOOST_REQUIRE( order->op.open_owner == "sam" );
      BOOST_REQUIRE( order->op.open_orderid == 0 );
      BOOST_REQUIRE( order->op.open_pays == ASSET( "0.500 TESTS" ) );
      order++;

      BOOST_REQUIRE( order->time == fill_order_c_time );
      BOOST_REQUIRE( order->op.current_owner == "bob" );
      BOOST_REQUIRE( order->op.current_orderid == 0 );
      BOOST_REQUIRE( order->op.current_pays == ASSET( "0.450 TESTS" ) );
      BOOST_REQUIRE( order->op.open_owner == "alice" );
      BOOST_REQUIRE( order->op.open_orderid == 0 );
      BOOST_REQUIRE( order->op.open_pays == ASSET( "0.250 USD" ) );
      order++;

      BOOST_REQUIRE( order == order_hist_idx.end() );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
#endif