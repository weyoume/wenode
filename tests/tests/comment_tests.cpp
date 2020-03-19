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

BOOST_FIXTURE_TEST_SUITE( comment_operation_tests, clean_database_fixture );


   //================================//
   // === Post and Comment Tests === //
   //================================//



BOOST_AUTO_TEST_CASE( comment_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: COMMENT OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signatures" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( fc::seconds( 60 ).count() / BLOCK_INTERVAL.count() );

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"foo\":\"bar\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = post_format_type::ARTICLE_POST;
      options.reach = feed_reach_type::TAG_FEED;
      options.rating = 1;
      comment.options = options;
      comment.validate();

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( comment );
   
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_posting_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );

      tx.sign( alice_private_posting_key, db.get_chain_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by an additional signature not in the creator's authority" );

      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by an additional signature not in the creator's authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by a signature not in the creator's authority" );

      tx.signatures.clear();
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_posting_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by a signature not in the creator's authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Success posting a root comment" );

      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      BOOST_REQUIRE( alice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( alice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( alice_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( alice_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( alice_comment.ipfs[0] ) == comment.ipfs[0] );
      BOOST_REQUIRE( to_string( alice_comment.magnet[0] ) == comment.magnet[0] );
      BOOST_REQUIRE( alice_comment.community == comment.community );
      BOOST_REQUIRE( alice_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( alice_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( alice_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( alice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( alice_comment.json ) == comment.json );
      BOOST_REQUIRE( alice_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( alice_comment.premium_price == comment.premium_price );

      BOOST_REQUIRE( alice_comment.last_updated == now() );
      BOOST_REQUIRE( alice_comment.created == now() );
      BOOST_REQUIRE( alice_comment.active == now() );

      BOOST_REQUIRE( alice_comment.depth == 0 );
      BOOST_REQUIRE( alice_comment.children == 0 );
      BOOST_REQUIRE( alice_comment.net_votes == 0 );
      BOOST_REQUIRE( alice_comment.view_count == 0 );
      BOOST_REQUIRE( alice_comment.share_count == 0 );

      BOOST_REQUIRE( alice_comment.net_reward == 0 );
      BOOST_REQUIRE( alice_comment.vote_power == 0 );
      BOOST_REQUIRE( alice_comment.view_power == 0 );
      BOOST_REQUIRE( alice_comment.share_power == 0 );
      BOOST_REQUIRE( alice_comment.comment_power == 0 );
      
      BOOST_REQUIRE( alice_comment.cashout_time == fc::time_point( now() + CONTENT_REWARD_INTERVAL ) );
      BOOST_REQUIRE( alice_comment.cashouts_received == 0 );

      BOOST_REQUIRE( alice_comment.total_vote_weight == 0 );
      BOOST_REQUIRE( alice_comment.total_view_weight == 0 );
      BOOST_REQUIRE( alice_comment.total_share_weight == 0 );
      BOOST_REQUIRE( alice_comment.total_comment_weight == 0 );

      BOOST_REQUIRE( alice_comment.total_payout_value.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.curator_payout_value.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.beneficiary_payout_value.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.content_rewards.amount.value == 0 );
      BOOST_REQUIRE( alice_comment.percent_liquid == PERCENT_100 );
      BOOST_REQUIRE( alice_comment.reward == 0 );
      BOOST_REQUIRE( alice_comment.weight == 0 );
      BOOST_REQUIRE( alice_comment.max_weight == 0 );

      BOOST_REQUIRE( alice_comment.author_reward_percent == median_props.author_reward_percent );
      BOOST_REQUIRE( alice_comment.vote_reward_percent == median_props.vote_reward_percent );
      BOOST_REQUIRE( alice_comment.view_reward_percent == median_props.view_reward_percent );
      BOOST_REQUIRE( alice_comment.share_reward_percent == median_props.share_reward_percent );
      BOOST_REQUIRE( alice_comment.comment_reward_percent == median_props.comment_reward_percent );
      BOOST_REQUIRE( alice_comment.storage_reward_percent == median_props.storage_reward_percent );
      BOOST_REQUIRE( alice_comment.moderator_reward_percent == median_props.moderator_reward_percent );

      BOOST_REQUIRE( alice_comment.allow_replies == true );
      BOOST_REQUIRE( alice_comment.allow_votes == true );
      BOOST_REQUIRE( alice_comment.allow_views == true );
      BOOST_REQUIRE( alice_comment.allow_shares == true );
      BOOST_REQUIRE( alice_comment.allow_curation_rewards == true );
      BOOST_REQUIRE( alice_comment.root == true );
      BOOST_REQUIRE( alice_comment.deleted == false );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Success posting a root comment" );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when posting a comment on a non-existent comment" );

      comment.signatory = "bob";
      comment.author = "bob";
      comment.permlink = "ipsum";
      comment.parent_author = "alice";
      comment.parent_permlink = "foobar";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when posting a comment on a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting a comment on previous comment" );

      comment.parent_permlink = "lorem";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& bob_comment = db.get_comment( "bob", string( "ipsum" ) );

      BOOST_REQUIRE( bob_comment.author == comment.author );
      BOOST_REQUIRE( to_string( bob_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( bob_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( bob_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( bob_comment.ipfs[0] ) == comment.ipfs[0] );
      BOOST_REQUIRE( to_string( bob_comment.magnet[0] ) == comment.magnet[0] );
      BOOST_REQUIRE( bob_comment.community == comment.community );
      BOOST_REQUIRE( bob_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( bob_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( bob_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( bob_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( bob_comment.json ) == comment.json );
      BOOST_REQUIRE( bob_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( bob_comment.premium_price == comment.premium_price );

      BOOST_REQUIRE( bob_comment.last_updated == now() );
      BOOST_REQUIRE( bob_comment.created == now() );
      BOOST_REQUIRE( bob_comment.active == now() );

      BOOST_REQUIRE( bob_comment.depth == 1 );
      BOOST_REQUIRE( bob_comment.children == 0 );
      BOOST_REQUIRE( bob_comment.net_votes == 0 );
      BOOST_REQUIRE( bob_comment.view_count == 0 );
      BOOST_REQUIRE( bob_comment.share_count == 0 );

      BOOST_REQUIRE( bob_comment.net_reward == 0 );
      BOOST_REQUIRE( bob_comment.vote_power == 0 );
      BOOST_REQUIRE( bob_comment.view_power == 0 );
      BOOST_REQUIRE( bob_comment.share_power == 0 );
      BOOST_REQUIRE( bob_comment.comment_power == 0 );
      
      BOOST_REQUIRE( bob_comment.cashout_time == fc::time_point( now() + CONTENT_REWARD_INTERVAL ) );
      BOOST_REQUIRE( bob_comment.cashouts_received == 0 );

      BOOST_REQUIRE( bob_comment.total_vote_weight == 0 );
      BOOST_REQUIRE( bob_comment.total_view_weight == 0 );
      BOOST_REQUIRE( bob_comment.total_share_weight == 0 );
      BOOST_REQUIRE( bob_comment.total_comment_weight == 0 );

      BOOST_REQUIRE( bob_comment.total_payout_value.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.curator_payout_value.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.beneficiary_payout_value.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.content_rewards.amount.value == 0 );
      BOOST_REQUIRE( bob_comment.percent_liquid == PERCENT_100 );

      BOOST_REQUIRE( bob_comment.author_reward_percent == median_props.author_reward_percent );
      BOOST_REQUIRE( bob_comment.vote_reward_percent == median_props.vote_reward_percent );
      BOOST_REQUIRE( bob_comment.view_reward_percent == median_props.view_reward_percent );
      BOOST_REQUIRE( bob_comment.share_reward_percent == median_props.share_reward_percent );
      BOOST_REQUIRE( bob_comment.comment_reward_percent == median_props.comment_reward_percent );
      BOOST_REQUIRE( bob_comment.storage_reward_percent == median_props.storage_reward_percent );
      BOOST_REQUIRE( bob_comment.moderator_reward_percent == median_props.moderator_reward_percent );

      BOOST_REQUIRE( bob_comment.allow_replies == true );
      BOOST_REQUIRE( bob_comment.allow_votes == true );
      BOOST_REQUIRE( bob_comment.allow_views == true );
      BOOST_REQUIRE( bob_comment.allow_shares == true );
      BOOST_REQUIRE( bob_comment.allow_curation_rewards == true );
      BOOST_REQUIRE( bob_comment.root == false );
      BOOST_REQUIRE( bob_comment.deleted == false );
      BOOST_REQUIRE( bob_comment.root_comment == alice_comment.id );

      BOOST_REQUIRE( alice_comment.children == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: posting a comment on previous comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting a comment on additional previous comment" );

      comment.signatory = "candice";
      comment.author = "candice";
      comment.permlink = "dolor";
      comment.parent_author = "bob";
      comment.parent_permlink = "ipsum";

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( "candice", string( "dolor" ) );

      BOOST_REQUIRE( candice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( candice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( candice_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( candice_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( candice_comment.ipfs[0] ) == comment.ipfs[0] );
      BOOST_REQUIRE( to_string( candice_comment.magnet[0] ) == comment.magnet[0] );
      BOOST_REQUIRE( candice_comment.community == comment.community );
      BOOST_REQUIRE( candice_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( candice_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( candice_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( candice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( candice_comment.json ) == comment.json );
      BOOST_REQUIRE( candice_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( candice_comment.premium_price == comment.premium_price );

      BOOST_REQUIRE( candice_comment.last_updated == now() );
      BOOST_REQUIRE( candice_comment.created == now() );
      BOOST_REQUIRE( candice_comment.active == now() );

      BOOST_REQUIRE( candice_comment.depth == 2 );
      BOOST_REQUIRE( candice_comment.children == 0 );
      BOOST_REQUIRE( candice_comment.net_votes == 0 );
      BOOST_REQUIRE( candice_comment.view_count == 0 );
      BOOST_REQUIRE( candice_comment.share_count == 0 );

      BOOST_REQUIRE( candice_comment.net_reward == 0 );
      BOOST_REQUIRE( candice_comment.vote_power == 0 );
      BOOST_REQUIRE( candice_comment.view_power == 0 );
      BOOST_REQUIRE( candice_comment.share_power == 0 );
      BOOST_REQUIRE( candice_comment.comment_power == 0 );
      
      BOOST_REQUIRE( candice_comment.cashout_time == fc::time_point( now() + CONTENT_REWARD_INTERVAL ) );
      BOOST_REQUIRE( candice_comment.cashouts_received == 0 );

      BOOST_REQUIRE( candice_comment.total_vote_weight == 0 );
      BOOST_REQUIRE( candice_comment.total_view_weight == 0 );
      BOOST_REQUIRE( candice_comment.total_share_weight == 0 );
      BOOST_REQUIRE( candice_comment.total_comment_weight == 0 );

      BOOST_REQUIRE( candice_comment.total_payout_value.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.curator_payout_value.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.beneficiary_payout_value.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.content_rewards.amount.value == 0 );
      BOOST_REQUIRE( candice_comment.percent_liquid == PERCENT_100 );

      BOOST_REQUIRE( candice_comment.author_reward_percent == median_props.author_reward_percent );
      BOOST_REQUIRE( candice_comment.vote_reward_percent == median_props.vote_reward_percent );
      BOOST_REQUIRE( candice_comment.view_reward_percent == median_props.view_reward_percent );
      BOOST_REQUIRE( candice_comment.share_reward_percent == median_props.share_reward_percent );
      BOOST_REQUIRE( candice_comment.comment_reward_percent == median_props.comment_reward_percent );
      BOOST_REQUIRE( candice_comment.storage_reward_percent == median_props.storage_reward_percent );
      BOOST_REQUIRE( candice_comment.moderator_reward_percent == median_props.moderator_reward_percent );

      BOOST_REQUIRE( candice_comment.allow_replies == true );
      BOOST_REQUIRE( candice_comment.allow_votes == true );
      BOOST_REQUIRE( candice_comment.allow_views == true );
      BOOST_REQUIRE( candice_comment.allow_shares == true );
      BOOST_REQUIRE( candice_comment.allow_curation_rewards == true );
      BOOST_REQUIRE( candice_comment.root == false );
      BOOST_REQUIRE( candice_comment.deleted == false );
      BOOST_REQUIRE( candice_comment.root_comment == alice_comment.id );

      BOOST_REQUIRE( bob_comment.children == 1 );
      BOOST_REQUIRE( alice_comment.children == 2 );

      validate_database();

      generate_blocks( 5 * BLOCKS_PER_MINUTE + 1 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: posting a comment on additional previous comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: modifying a comment" );

      fc::time_point created = candice_comment.created;

      tx.signatures.clear();
      tx.operations.clear();

      comment.title = "foo";
      comment.body = "bar";
      comment.json = "{\"bar\":\"foo\"}";
      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& candice_comment = db.get_comment( "candice", string( "dolor" ) );

      BOOST_REQUIRE( candice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( candice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( candice_comment.parent_author == comment.parent_author );
      BOOST_REQUIRE( to_string( candice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( candice_comment.last_updated == now() );
      BOOST_REQUIRE( candice_comment.created == created );
      BOOST_REQUIRE( candice_comment.cashout_time == candice_comment.created + CONTENT_REWARD_INTERVAL );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: modifying a comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure posting again within time limit" );

      comment.permlink = "sit";
      comment.parent_author = "";
      comment.parent_permlink = "test";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( 5 * BLOCKS_PER_MINUTE );

      comment.permlink = "amet";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      generate_block();
      db.push_transaction( tx, 0 );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure posting again within time limit" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: more than 100% weight on a single route" );

      comment_options options;

      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_100 + 1 ) );

      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: more than 100% weight on a single route" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: more than 100% total weight" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 * 75 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "sam" ), PERCENT_1 * 75 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: more than 100% total weight" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: maximum number of routes" );

      options.beneficiaries.clear();

      for( size_t i = 0; i < 127; i++ )
      {
         options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "foo" + fc::to_string( i ) ), 1 ) );
      }

      std::sort( options.beneficiaries.begin(), options.beneficiaries.end() );
      options.validate();

      BOOST_TEST_MESSAGE( "│   ├── Passed: maximum number of routes" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: one too many routes" );

      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bar" ), 1 ) );
      std::sort( options.beneficiaries.begin(), options.beneficiaries.end() );

      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: one too many routes" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: duplicate accounts" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 * 2 ) );
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: duplicate accounts" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: incorrect account sort order" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( "alice", PERCENT_1 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: incorrect account sort order" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: correct account sort order" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( "alice", PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( "bob", PERCENT_1 ) );
      
      options.validate();

      BOOST_TEST_MESSAGE( "│   ├── Passed: correct account sort order" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: specifying a non-existent benefactor" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "doug" ), PERCENT_1 ) );

      options.validate();
      comment.options = options;

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: specifying a non-existent benefactor" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: setting when comment has been voted on" );

      vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "candice";
      vote.permlink = "dolor";
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;
      vote.validate();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();
      
      options.beneficiaries.clear();

      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), 25 * PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "alice" ), 50 * PERCENT_1 ) );

      options.validate();
      comment.options = options;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: setting when comment has been voted on" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success when altering beneficiaries before voting" );

      comment.signatory = "bob";
      comment.author = "bob";
      comment.permlink = "ipsum";
      comment.parent_author = "alice";
      comment.parent_permlink = "foobar";

      tx.signatures.clear();
      tx.operations.clear();

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success when altering beneficiaries before voting" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when there are already beneficiaries" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "dan" ), 25 * PERCENT_1 ) );
      comment.options = options;

      tx.signatures.clear();
      tx.operations.clear();

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when there are already beneficiaries" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Payout and verify rewards were split properly" );

      tx.signatures.clear();
      tx.operations.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      time_point cashout_time = candice_comment.cashout_time;

      generate_blocks( cashout_time - BLOCK_INTERVAL );

      BOOST_REQUIRE( candice_comment.cashouts_received == 0 );

      asset alice_reward = db.get_reward_balance( "alice", SYMBOL_COIN );
      asset bob_reward = db.get_reward_balance( "bob", SYMBOL_COIN );
      asset candice_reward = db.get_reward_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_reward.amount == 0 );
      BOOST_REQUIRE( bob_reward.amount == 0 );
      BOOST_REQUIRE( candice_reward.amount == 0 );

      generate_block();

      BOOST_REQUIRE( candice_comment.cashouts_received == 1 );

      asset alice_reward = db.get_reward_balance( "alice", SYMBOL_COIN );
      asset bob_reward = db.get_reward_balance( "bob", SYMBOL_COIN );
      asset candice_reward = db.get_reward_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_reward.amount > 0 );
      BOOST_REQUIRE( bob_reward.amount > 0 );
      BOOST_REQUIRE( candice_reward.amount > 0 );

      BOOST_REQUIRE( bob_reward + candice_reward == alice_reward );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Payout and verify rewards were split properly" );

      BOOST_TEST_MESSAGE( "├── Passed: COMMENT OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( message_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: MESSAGE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no connection" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      message_operation message;

      message.signatory = "alice";
      message.sender = "alice";
      message.recipient = "bob";
      message.message = "Hello";
      message.uuid = "6a91e502-1e53-4531-a97a-379ac8a495ff";

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no connection" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: message success with connection" );

      connection_request_operation request;

      request.signatory = "alice";
      request.account = "alice";
      request.requested_account = "bob";
      request.connection_type = connection_tier_type::CONNECTION;
      request.message = "Hello";
      request.requested = true;

      tx.operations.push_back( request );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      connection_accept_operation accept;

      accept.signatory = "bob";
      accept.account = "bob";
      accept.requesting_account = "alice";
      accept.connection_type = connection_tier_type::CONNECTION;
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "alice";
      accept.account = "alice";
      accept.requesting_account = "bob";
      accept.connection_type = connection_tier_type::CONNECTION;
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( message );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 ); 

      const auto& message_idx = db.get_index< message_index >().indices().get< by_sender_uuid >();
      auto message_itr = message_idx.find( std::make_tuple( "alice", "6a91e502-1e53-4531-a97a-379ac8a495ff" ) );

      BOOST_REQUIRE( message_itr != message_idx.end() );
      BOOST_REQUIRE( message_itr->message == message.message );
      BOOST_REQUIRE( message_itr->created == now() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: message success with connection" );

      BOOST_TEST_MESSAGE( "├── Passed: MESSAGE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( vote_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: VOTE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting on a non-existent comment" );
      
      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = post_format_type::ARTICLE_POST;
      options.reach = feed_reach_type::TAG_FEED;
      options.rating = 1;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote_operation vote;

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.author = "alice";
      vote.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting on a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting with an initial weight of 0" );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting with an initial weight of 0" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful vote with PERCENT_100 weight" );

      auto old_voting_power = bob.voting_power;

      vote.weight = PERCENT_100;
      vote.permlink = "lorem";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      auto bob_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "bob" ) );
      int64_t max_vote_denom = median_props.vote_reserve_rate * ( median_props.vote_recharge_time.count() / fc::days(1).count() );

      BOOST_REQUIRE( bob.voting_power == old_voting_power - ( ( old_voting_power + max_vote_denom - 1 ) / max_vote_denom ) );
      BOOST_REQUIRE( bob.last_vote_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_vote_itr != vote_idx.end() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful vote with PERCENT_100 weight" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: negative vote" );

      old_voting_power = candice.voting_power;

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.weight = -1 * PERCENT_100;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "candice" ) );

      BOOST_REQUIRE( candice.voting_power == old_voting_power - ( ( old_voting_power + max_vote_denom - 1 ) / max_vote_denom ) );
      BOOST_REQUIRE( candice.last_vote_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( candice_vote_itr != vote_idx.end() );
      BOOST_REQUIRE( candice_vote_itr->last_updated == now() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: negative vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adjusting vote weight" );

      generate_blocks( now() + MIN_VOTE_INTERVAL );

      vote.weight = PERCENT_1 * 50;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "candice" ) );

      BOOST_REQUIRE( candice_vote_itr->last_updated == now() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adjusting vote weight" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: changing vote to 0 weight" );

      generate_blocks( now() + MIN_VOTE_INTERVAL );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      auto candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, "candice" ) );

      BOOST_REQUIRE( candice_vote_itr->last_updated == now() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );
      BOOST_REQUIRE( candice_vote_itr->reward == 0 );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: changing vote to 0 weight" );

      BOOST_TEST_MESSAGE( "├── Passed: VOTE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( view_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: VIEW OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when viewing a non-existent comment" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& view_idx = db.get_index< comment_view_index >().indices().get< by_comment_viewer >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = post_format_type::ARTICLE_POST;
      options.reach = feed_reach_type::TAG_FEED;
      options.rating = 1;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view_operation view;

      view.signatory = "bob";
      view.viewer = "bob";
      view.author = "alice";
      view.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      view.interface = INIT_ACCOUNT;
      view.supernode = INIT_ACCOUNT;
      view.validate();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when viewing a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful view" );

      auto old_viewing_power = bob.viewing_power;

      view.permlink = "lorem";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      auto bob_view_itr = view_idx.find( std::make_tuple( alice_comment.id, "bob" ) );
      int64_t max_view_denom = median_props.view_reserve_rate * ( median_props.view_recharge_time.count() / fc::days(1).count() );

      BOOST_REQUIRE( bob.viewing_power == old_viewing_power - ( ( old_viewing_power + max_view_denom - 1 ) / max_view_denom ) );
      BOOST_REQUIRE( bob.last_view_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_view_itr != view_idx.end() );
      BOOST_REQUIRE( bob_view_itr->weight > 0 );
      BOOST_REQUIRE( bob_view_itr->reward > 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful view" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing view" );

      generate_blocks( now() + MIN_VIEW_INTERVAL );

      view.viewed = false;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto bob_view_itr = view_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( bob_view_itr == view_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing view" );

      BOOST_TEST_MESSAGE( "├── Passed: VIEW OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( share_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: SHARE OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when sharing a non-existent comment" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      const auto& share_idx = db.get_index< comment_share_index >().indices().get< by_comment_sharer >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = post_format_type::ARTICLE_POST;
      options.reach = feed_reach_type::TAG_FEED;
      options.rating = 1;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      share_operation share;

      share.signatory = "bob";
      share.sharer = "bob";
      share.author = "alice";
      share.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      share.reach = feed_reach_type::FOLLOW_FEED;
      share.interface = INIT_ACCOUNT;

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when sharing a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful share to follow feed" );

      auto old_sharing_power = bob.sharing_power;

      share.permlink = "lorem";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      auto bob_share_itr = share_idx.find( std::make_tuple( alice_comment.id, "bob" ) );
      int64_t max_share_denom = median_props.share_reserve_rate * ( median_props.share_recharge_time.count() / fc::days(1).count() );

      BOOST_REQUIRE( bob.sharing_power == old_sharing_power - ( ( old_sharing_power + max_share_denom - 1 ) / max_share_denom ) );
      BOOST_REQUIRE( bob.last_share_time == now() );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_share_itr != share_idx.end() );
      BOOST_REQUIRE( bob_share_itr->weight > 0 );
      BOOST_REQUIRE( bob_share_itr->reward > 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful share to follow feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing share" );

      generate_blocks( now() + MIN_SHARE_INTERVAL );

      share.shared = false;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto bob_share_itr = share_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( bob_share_itr == share_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing share" );

      BOOST_TEST_MESSAGE( "├── Passed: SHARE OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( moderation_tag_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: MODERATION TAG OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when moderating a non-existent comment" );

      const median_chain_property_object& median_props = db.get_median_chain_properties();

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs.push_back( "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB" );
      comment.magnet.push_back( "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&" );
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{\"json\":\"valid\"}";
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = post_format_type::ARTICLE_POST;
      options.reach = feed_reach_type::TAG_FEED;
      options.rating = 1;
      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      moderation_tag_operation tag;

      tag.signatory = "bob";
      tag.moderator =  "bob";
      tag.author = "alice";
      tag.permlink = "supercalafragilisticexpealadocious";    // Permlink does not exist
      tag.tags.push_back( "nsfw" );
      tag.rating = 9;
      tag.details = "Post is NSFW";
      tag.interface = INIT_ACCOUNT;
      tag.filter = false;
      tag.applied = true;
      tag.validate();

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when moderating a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account is not governance account" );

      tx.operations.clear();
      tx.signatures.clear();

      tag.permlink = "lorem";

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account is not governance account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful moderation tag" );

      account_membership_operation mem;

      mem.signatory = "bob";
      mem.account = "bob";
      mem.membership_type = membership_tier_type::TOP_MEMBERSHIP;
      mem.months = 1;
      mem.interface = INIT_ACCOUNT;
      mem.validate();

      tx.operations.push_back( mem );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      update_governance_operation gov;
      
      gov.signatory = "bob";
      gov.account = "bob";
      gov.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      gov.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      gov.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      gov.validate();

      tx.operations.push_back( gov );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const governance_account_object& governance = db.get_governance_account( "bob" );
      
      BOOST_REQUIRE( governance.active == true );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( "alice", string( "lorem" ) );

      const auto& tag_idx = db.get_index< moderation_tag_index >().indices().get< by_comment_moderator >();
      auto tag_itr = tag_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( tag_itr->details == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );
      BOOST_REQUIRE( tag_itr->created == now() );
   
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful moderation tag" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing moderation tag" );

      generate_blocks( now() + MIN_SHARE_INTERVAL );

      tag.applied = false;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto tag_itr = tag_idx.find( std::make_tuple( alice_comment.id, "bob" ) );

      BOOST_REQUIRE( tag_itr == tag_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing moderation tag" );

      BOOST_TEST_MESSAGE( "├── Passed: MODERATION TAG OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()