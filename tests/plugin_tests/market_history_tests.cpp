#include <boost/test/unit_test.hpp>
#include <node/chain/database.hpp>
#include <node/market_history/market_history_plugin.hpp>
#include "../common/database_fixture.hpp"

using namespace node;
using namespace node::chain;
using namespace node::protocol;
using namespace node::market_history;

BOOST_FIXTURE_TEST_SUITE( market_history_tests, clean_database_fixture )

BOOST_AUTO_TEST_CASE( market_history_test )
{
   try
   {
      auto mh_plugin = app.register_plugin< market_history_plugin >();
      boost::program_options::variables_map options;
      mh_plugin->plugin_initialize( options );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );

      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      
      signed_transaction tx;

      generate_blocks( BLOCKS_PER_DAY );

      const auto& duration_idx = db.get_index< market_duration_index >().indices().get< by_duration >();
      const auto& order_hist_idx = db.get_index< order_history_index >().indices().get< by_id >();

      BOOST_REQUIRE( duration_idx.begin() == duration_idx.end() );
      BOOST_REQUIRE( order_hist_idx.begin() == order_hist_idx.end() );

      validate_database();

      auto fill_order_a_time = now();
      fc::time_point time_a = fc::time_point( fc::seconds( ( fill_order_a_time.sec_since_epoch() / 60 ) * 60 ) );

      limit_order_operation limit;

      limit.owner = "alice";
      limit.order_id = "9f42232f-2c75-46d5-90cd-15a41b737d2c";
      limit.amount_to_sell = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      limit.exchange_rate = price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );
      limit.interface = INIT_ACCOUNT;
      limit.expiration = now() + fc::days( 30 );
      limit.opened = true;
      limit.fill_or_kill = false;
      limit.validate();

      tx.operations.push_back( limit );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx,  0 );

      tx.operations.clear();
      tx.signatures.clear();

      limit.owner = "bob";
      limit.order_id = "af8a566e-1394-40ce-9e37-d21ab64273f1";
      limit.amount_to_sell = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );
      limit.exchange_rate = price( asset( 1000, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );

      tx.operations.push_back( limit );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 3 * BLOCKS_PER_DAY );

      auto fill_order_b_time = now();

      limit.owner = "candice";
      limit.order_id = "004a1e0a-5a1c-4898-9315-40f3a098ca24";
      limit.amount_to_sell = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      limit.exchange_rate = price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) );

      tx.operations.push_back( limit );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      limit.owner = "dan";
      limit.order_id = "49d29c23-6493-4111-a4bc-14d9a1d4d0fb";
      limit.amount_to_sell = asset( 105000000, SYMBOL_USD );
      limit.exchange_rate = price( asset( 1050, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );

      tx.operations.push_back( limit );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 12 * BLOCKS_PER_HOUR );

      auto fill_order_c_time = now();

      limit.owner = "alice";
      limit.order_id = "bc4cfa8f-76ff-494f-b98e-7e0eb5c4bd76";
      limit.amount_to_sell = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      limit.exchange_rate = price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) );

      tx.operations.push_back( limit );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx,  0 );

      tx.operations.clear();
      tx.signatures.clear();

      limit.owner = "bob";
      limit.order_id = "1e1fe2e6-bc4a-4a9e-98cd-156eb652250b";
      limit.amount_to_sell = asset( 95000000, SYMBOL_USD );
      limit.exchange_rate = price( asset( 950, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );

      tx.operations.push_back( limit );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 30 * BLOCKS_PER_MINUTE );

      auto fill_order_d_time = now();

      limit.owner = "candice";
      limit.order_id = "5d006a9f-1359-419f-a2ab-d3fe4a596009";
      limit.amount_to_sell = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      limit.exchange_rate = price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) );

      tx.operations.push_back( limit );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      limit.owner = "dan";
      limit.order_id = "a2320964-55f0-461a-b281-238cb1103bcb";
      limit.amount_to_sell = asset( BLOCKCHAIN_PRECISION, SYMBOL_USD );
      limit.exchange_rate = price( asset( 1000, SYMBOL_USD ), asset( 1000, SYMBOL_COIN ) );

      tx.operations.push_back( limit );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( dan_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      auto duration = duration_idx.begin();

      BOOST_REQUIRE( duration->seconds == 60 );
      BOOST_REQUIRE( duration->open_time == time_a );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 60 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 60 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) + fc::hours(12) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 60 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) + fc::hours(12) + fc::minutes(30) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 300 );
      BOOST_REQUIRE( duration->open_time == time_a );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 300 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 300 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) + fc::hours(12) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 300 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) + fc::hours(12) + fc::minutes(30) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 3600 );
      BOOST_REQUIRE( duration->open_time == time_a );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 3600 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 3600 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) + fc::hours(12) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 86400 );
      BOOST_REQUIRE( duration->open_time == time_a );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 86400 );
      BOOST_REQUIRE( duration->open_time == time_a + fc::days(3) );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration->seconds == 604800 );
      BOOST_REQUIRE( duration->open_time == time_a );
      BOOST_REQUIRE( duration->symbol_a == SYMBOL_COIN );
      BOOST_REQUIRE( duration->symbol_b == SYMBOL_USD );
      BOOST_REQUIRE( duration->open_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->high_price == price( asset( 1000, SYMBOL_COIN ), asset( 1050, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->low_price == price( asset( 1000, SYMBOL_COIN ), asset( 950, SYMBOL_USD ) ) );
      BOOST_REQUIRE( duration->close_price == price( asset( 1000, SYMBOL_COIN ), asset( 1000, SYMBOL_USD ) ) );
      duration++;

      BOOST_REQUIRE( duration == duration_idx.end() );

      auto order = order_hist_idx.begin();

      BOOST_REQUIRE( order->time == fill_order_a_time );
      BOOST_REQUIRE( order->op.current_owner == "bob" );
      BOOST_REQUIRE( order->op.current_order_id == "af8a566e-1394-40ce-9e37-d21ab64273f1" );
      BOOST_REQUIRE( order->op.current_pays == asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      BOOST_REQUIRE( order->op.open_owner == "alice" );
      BOOST_REQUIRE( order->op.open_order_id == "9f42232f-2c75-46d5-90cd-15a41b737d2c" );
      BOOST_REQUIRE( order->op.open_pays == asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      order++;

      BOOST_REQUIRE( order->time == fill_order_b_time );
      BOOST_REQUIRE( order->op.current_owner == "dan" );
      BOOST_REQUIRE( order->op.current_order_id == "49d29c23-6493-4111-a4bc-14d9a1d4d0fb" );
      BOOST_REQUIRE( order->op.current_pays == asset( 105000000, SYMBOL_USD ) );
      BOOST_REQUIRE( order->op.open_owner == "candice" );
      BOOST_REQUIRE( order->op.open_order_id == "004a1e0a-5a1c-4898-9315-40f3a098ca24" );
      BOOST_REQUIRE( order->op.open_pays == asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      order++;

      BOOST_REQUIRE( order->time == fill_order_c_time );
      BOOST_REQUIRE( order->op.current_owner == "bob" );
      BOOST_REQUIRE( order->op.current_order_id == "1e1fe2e6-bc4a-4a9e-98cd-156eb652250b" );
      BOOST_REQUIRE( order->op.current_pays == asset( 95000000, SYMBOL_USD ) );
      BOOST_REQUIRE( order->op.open_owner == "alice" );
      BOOST_REQUIRE( order->op.open_order_id == "bc4cfa8f-76ff-494f-b98e-7e0eb5c4bd76" );
      BOOST_REQUIRE( order->op.open_pays == asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      order++;

      BOOST_REQUIRE( order->time == fill_order_d_time );
      BOOST_REQUIRE( order->op.current_owner == "dan" );
      BOOST_REQUIRE( order->op.current_order_id == "a2320964-55f0-461a-b281-238cb1103bcb" );
      BOOST_REQUIRE( order->op.current_pays == asset( BLOCKCHAIN_PRECISION, SYMBOL_USD ) );
      BOOST_REQUIRE( order->op.open_owner == "candice" );
      BOOST_REQUIRE( order->op.open_order_id == "5d006a9f-1359-419f-a2ab-d3fe4a596009" );
      BOOST_REQUIRE( order->op.open_pays == asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      order++;

      BOOST_REQUIRE( order == order_hist_idx.end() );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()