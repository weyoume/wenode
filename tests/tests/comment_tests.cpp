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

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( BLOCKS_PER_DAY );

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = INIT_COMMUNITY;
      comment.public_key = string();
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "lorem";
      comment.json = "{ \"valid\": true }";
      comment.latitude = 37.8136;
      comment.longitude = 144.9631;
      comment.comment_price = asset( 0, SYMBOL_COIN );
      comment.reply_price = asset( 0, SYMBOL_COIN );
      comment.premium_price = asset( 0, SYMBOL_COIN );

      comment_options options;

      options.post_type = "article";
      options.reach = "tag";
      options.rating = 1;
      options.max_accepted_payout = MAX_ACCEPTED_PAYOUT;
      comment.options = options;
      comment.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( comment );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );

      tx.sign( alice_private_posting_key, db.get_chain_id() );
      tx.sign( alice_private_posting_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when signed by a signature not in the creator's authority" );

      tx.signatures.clear();
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when signed by a signature not in the creator's authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Success posting a root comment" );

      tx.signatures.clear();
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      generate_block();

      const comment_object& alice_comment = db.get_comment( account_name_type( "alice" ), string( "lorem" ) );

      BOOST_REQUIRE( alice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( alice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( alice_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( alice_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( alice_comment.ipfs ) == comment.ipfs );
      BOOST_REQUIRE( to_string( alice_comment.magnet ) == comment.magnet );
      BOOST_REQUIRE( alice_comment.community == comment.community );
      BOOST_REQUIRE( alice_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( alice_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( alice_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( alice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( alice_comment.json ) == comment.json );
      BOOST_REQUIRE( alice_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( alice_comment.reply_price == comment.reply_price );
      BOOST_REQUIRE( alice_comment.premium_price == comment.premium_price );

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

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when posting a comment on a non-existent comment" );

      comment.signatory = "bob";
      comment.author = "bob";
      comment.permlink = "ipsum";
      comment.parent_author = "alice";
      comment.parent_permlink = "foobar";
      comment.validate();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.signatures.clear();
      tx.operations.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when posting a comment on a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting a comment on previous comment" );

      options.post_type = "article";
      options.reach = "tag";
      options.rating = 1;
      options.max_accepted_payout = MAX_ACCEPTED_PAYOUT;
      comment.options = options;
      
      comment.parent_permlink = "lorem";

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      const comment_object& bob_comment = db.get_comment( account_name_type( "bob" ), string( "ipsum" ) );

      BOOST_REQUIRE( bob_comment.author == comment.author );
      BOOST_REQUIRE( to_string( bob_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( bob_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( bob_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( bob_comment.ipfs ) == comment.ipfs );
      BOOST_REQUIRE( to_string( bob_comment.magnet ) == comment.magnet );
      BOOST_REQUIRE( bob_comment.community == comment.community );
      BOOST_REQUIRE( bob_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( bob_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( bob_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( bob_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( bob_comment.json ) == comment.json );
      BOOST_REQUIRE( bob_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( bob_comment.reply_price == comment.reply_price );
      BOOST_REQUIRE( bob_comment.premium_price == comment.premium_price );

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

      BOOST_TEST_MESSAGE( "│   ├── Passed: posting a comment on previous comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting a comment on additional previous comment" );

      comment.signatory = "candice";
      comment.author = "candice";
      comment.permlink = "dolor";
      comment.parent_author = "bob";
      comment.parent_permlink = "ipsum";
      comment.validate();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      const comment_object& candice_comment = db.get_comment( account_name_type( "candice" ), string( "dolor" ) );

      BOOST_REQUIRE( candice_comment.author == comment.author );
      BOOST_REQUIRE( to_string( candice_comment.permlink ) == comment.permlink );
      BOOST_REQUIRE( to_string( candice_comment.title ) == comment.title );
      BOOST_REQUIRE( to_string( candice_comment.body ) == comment.body );
      BOOST_REQUIRE( to_string( candice_comment.ipfs ) == comment.ipfs );
      BOOST_REQUIRE( to_string( candice_comment.magnet ) == comment.magnet );
      BOOST_REQUIRE( candice_comment.community == comment.community );
      BOOST_REQUIRE( candice_comment.tags[0] == comment.tags[0] );
      BOOST_REQUIRE( candice_comment.interface == comment.interface );
      BOOST_REQUIRE( to_string( candice_comment.language ) == comment.language );
      BOOST_REQUIRE( to_string( candice_comment.parent_permlink ) == comment.parent_permlink );
      BOOST_REQUIRE( to_string( candice_comment.json ) == comment.json );
      BOOST_REQUIRE( candice_comment.comment_price == comment.comment_price );
      BOOST_REQUIRE( candice_comment.reply_price == comment.reply_price );
      BOOST_REQUIRE( candice_comment.premium_price == comment.premium_price );

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

      generate_blocks( 10 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: posting a comment on additional previous comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure posting again within time limit" );

      comment.permlink = "sit";
      comment.parent_author = ROOT_POST_PARENT;
      comment.parent_permlink = "test";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 1 * BLOCKS_PER_MINUTE - 1 );

      comment.permlink = "amet";

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure posting again within time limit" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: more than 100% weight on a single route" );

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
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 * 2 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: duplicate accounts" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: incorrect account sort order" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "alice" ), PERCENT_1 ) );
      
      REQUIRE_THROW( options.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: incorrect account sort order" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: correct account sort order" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "alice" ), PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), PERCENT_1 ) );
      
      options.validate();

      BOOST_TEST_MESSAGE( "│   ├── Passed: correct account sort order" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: specifying a non-existent benefactor" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "alice" ), PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "doug" ), PERCENT_1 ) );

      options.post_type = "article";
      options.reach = "tag";
      options.rating = 1;
      options.max_accepted_payout = MAX_ACCEPTED_PAYOUT;
      comment.options = options;
      
      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

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

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();
      
      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "alice" ), 50 * PERCENT_1 ) );
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "bob" ), 25 * PERCENT_1 ) );
      
      comment.options = options;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: setting when comment has been voted on" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success when altering beneficiaries before voting" );

      comment.signatory = "bob";
      comment.author = "bob";
      comment.permlink = "ipsum";
      comment.parent_author = "alice";
      comment.parent_permlink = "lorem";
      comment.validate();

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.signatures.clear();
      tx.operations.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success when altering beneficiaries before voting" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when there are already beneficiaries" );

      options.beneficiaries.clear();
      options.beneficiaries.push_back( beneficiary_route_type( account_name_type( "candice" ), 25 * PERCENT_1 ) );

      comment.options = options;
      comment.validate();

      tx.operations.push_back( comment );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.signatures.clear();
      tx.operations.clear();

      generate_blocks( 10 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when there are already beneficiaries" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Payout and verify rewards were split properly" );

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "lorem";

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 10 );

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.author = "bob";
      vote.permlink = "ipsum";

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 10 );

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.author = "candice";
      vote.permlink = "dolor";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 10 );

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.author = "alice";
      vote.permlink = "lorem";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 10 );

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "bob";
      vote.permlink = "ipsum";

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( 10 );

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.author = "candice";
      vote.permlink = "dolor";

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_reward = get_reward_balance( "alice", SYMBOL_COIN );
      asset bob_reward = get_reward_balance( "bob", SYMBOL_COIN );
      asset candice_reward = get_reward_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_reward.amount == 0 );
      BOOST_REQUIRE( bob_reward.amount == 0 );
      BOOST_REQUIRE( candice_reward.amount == 0 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 1 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 1 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 1 );

      alice_reward = get_reward_balance( "alice", SYMBOL_COIN );
      bob_reward = get_reward_balance( "bob", SYMBOL_COIN );
      candice_reward = get_reward_balance( "candice", SYMBOL_COIN );

      BOOST_REQUIRE( alice_reward.amount > 0 );
      BOOST_REQUIRE( bob_reward.amount > 0 );
      BOOST_REQUIRE( candice_reward.amount > 0 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 2 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 2 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 2 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 3 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 3 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 3 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 4 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 4 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 4 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 5 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 5 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 5 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 6 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 6 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 6 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 7 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 7 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 7 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 8 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 8 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 8 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 9 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 9 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 9 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 10 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 10 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 10 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 11 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 11 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 11 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 12 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 12 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 12 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 13 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 13 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 13 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 14 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 14 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 14 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 15 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 15 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 15 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 16 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 16 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 16 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 17 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 17 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 17 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 18 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 18 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 18 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 19 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 19 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 19 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 20 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 20 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 20 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 21 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 21 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 21 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 22 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 22 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 22 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 23 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 23 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 23 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 24 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 24 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 24 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 25 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 25 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 25 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 26 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 26 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 26 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 27 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 27 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 27 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 28 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 28 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 28 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 29 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 29 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 29 );

      generate_blocks( now() + fc::hours(24), true );
      generate_block();

      BOOST_REQUIRE( db.get_comment( account_name_type( "alice" ), string( "lorem" ) ).cashouts_received == 30 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "bob" ), string( "ipsum" ) ).cashouts_received == 30 );
      BOOST_REQUIRE( db.get_comment( account_name_type( "candice" ), string( "dolor" ) ).cashouts_received == 30 );

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

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      string alice_private_connection_wif = graphene::utilities::key_to_wif( alice_private_connection_key );
      string bob_private_connection_wif = graphene::utilities::key_to_wif( bob_private_connection_key );

      signed_transaction tx;

      message_operation message;

      message.signatory = "alice";
      message.sender = "alice";
      message.recipient = "bob";
      message.message = "Hello";
      message.uuid = "6a91e502-1e53-4531-a97a-379ac8a495ff";
      message.validate();

      tx.operations.push_back( message );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no connection" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: message success with connection" );

      connection_request_operation request;

      request.signatory = "alice";
      request.account = "alice";
      request.requested_account = "bob";
      request.connection_type = "connection";
      request.message = "Hello";
      request.requested = true;
      request.validate();

      tx.operations.push_back( request );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      connection_accept_operation accept;

      accept.signatory = "bob";
      accept.account = "bob";
      accept.requesting_account = "alice";
      accept.connection_type = "connection";
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = get_encrypted_message( bob_private_secure_key, bob_public_secure_key, alice_public_secure_key, bob_private_connection_wif );
      accept.connected = true;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "alice";
      accept.account = "alice";
      accept.requesting_account = "bob";
      accept.connection_type = "connection";
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, bob_public_secure_key, alice_private_connection_wif );
      accept.connected = true;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( message );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 ); 

      const auto& message_idx = db.get_index< message_index >().indices().get< by_sender_uuid >();
      auto message_itr = message_idx.find( std::make_tuple( account_name_type( "alice" ), string( "6a91e502-1e53-4531-a97a-379ac8a495ff" ) ) );

      BOOST_REQUIRE( message_itr != message_idx.end() );
      BOOST_REQUIRE( to_string( message_itr->message ) == message.message );

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

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      const auto& vote_idx = db.get_index< comment_vote_index >().indices().get< by_comment_voter >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{ \"valid\": true }";
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

      generate_blocks(10);

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting on a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting with an initial weight of 0" );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      generate_blocks(10);

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting with an initial weight of 0" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful vote with PERCENT_100 weight" );

      uint16_t old_voting_power = bob.voting_power;

      vote.weight = PERCENT_100;
      vote.permlink = "lorem";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const comment_object& alice_comment = db.get_comment( account_name_type( "alice" ), string( "lorem" ) );

      auto bob_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, account_name_type( "bob" ) ) );

      int64_t max_vote_denom = median_props.vote_reserve_rate * ( median_props.vote_recharge_time.count() / fc::days(1).count() );

      uint16_t used_power = ( old_voting_power * abs( vote.weight ) ) / PERCENT_100;
      used_power = ( used_power + max_vote_denom - 1 ) / max_vote_denom;

      BOOST_REQUIRE( bob.voting_power == old_voting_power - used_power );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( bob_vote_itr != vote_idx.end() );

      generate_blocks(10);

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

      auto candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, account_name_type( "candice" ) ) );

      used_power = ( old_voting_power * abs( vote.weight ) ) / PERCENT_100;
      used_power = ( used_power + max_vote_denom - 1 ) / max_vote_denom;

      BOOST_REQUIRE( candice.voting_power == old_voting_power - used_power );
      BOOST_REQUIRE( alice_comment.cashout_time == alice_comment.created + CONTENT_REWARD_INTERVAL );
      BOOST_REQUIRE( candice_vote_itr != vote_idx.end() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );

      generate_blocks(10);

      BOOST_TEST_MESSAGE( "│   ├── Passed: negative vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adjusting vote weight" );

      vote.weight = PERCENT_1 * 50;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, account_name_type( "candice" ) ) );

      BOOST_REQUIRE( candice_vote_itr != vote_idx.end() );
      BOOST_REQUIRE( candice_vote_itr->vote_percent == vote.weight );

      BOOST_TEST_MESSAGE( "│   ├── Passed: adjusting vote weight" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: changing vote to 0 weight" );

      generate_blocks(10);

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      candice_vote_itr = vote_idx.find( std::make_tuple( alice_comment.id, account_name_type( "candice" ) ) );

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

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      const auto& view_idx = db.get_index< comment_view_index >().indices().get< by_comment_viewer >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{ \"valid\": true }";
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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      uint16_t old_viewing_power = bob.viewing_power;

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

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when viewing a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful view" );

      view.permlink = "lorem";

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const comment_object& alice_comment = db.get_comment( account_name_type( "alice" ), string( "lorem" ) );
      const comment_view_object& bob_view = db.get_comment_view( account_name_type( "bob" ), alice_comment.id );

      int16_t max_view_denom = int16_t( median_props.view_reserve_rate ) * int16_t( median_props.view_recharge_time.to_seconds() / fc::days(1).to_seconds() );    
      int16_t used_power = ( bob.viewing_power + max_view_denom - 1 ) / max_view_denom;

      BOOST_REQUIRE( bob_view.comment == alice_comment.id );
      BOOST_REQUIRE( bob.viewing_power == old_viewing_power - used_power );

      generate_blocks(10);

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful view" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing view" );

      view.viewed = false;
      
      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto bob_view_itr = view_idx.find( std::make_tuple( alice_comment.id, account_name_type( "bob" ) ) );

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

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      const auto& share_idx = db.get_index< comment_share_index >().indices().get< by_comment_sharer >();

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( "test" );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{ \"valid\": true }";
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
      share.reach = "follow";
      share.interface = INIT_ACCOUNT;

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks(10);

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when sharing a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful share to follow feed" );

      uint16_t old_sharing_power = bob.sharing_power;

      share.permlink = "lorem";

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const comment_object& alice_comment = db.get_comment( account_name_type( "alice" ), string( "lorem" ) );
      const comment_share_object& bob_share = db.get_comment_share( account_name_type( "bob" ), alice_comment.id );

      int16_t max_share_denom = int16_t( median_props.share_reserve_rate ) * int16_t( median_props.share_recharge_time.to_seconds() / fc::days(1).to_seconds() );
      int16_t used_power = ( bob.sharing_power + max_share_denom - 1 ) / max_share_denom;

      BOOST_REQUIRE( bob.sharing_power == old_sharing_power - used_power );
      BOOST_REQUIRE( bob_share.comment == alice_comment.id );
      
      generate_blocks(10);

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful share to follow feed" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing share" );

      share.shared = false;

      tx.operations.push_back( share );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto bob_share_itr = share_idx.find( std::make_tuple( alice_comment.id, account_name_type( "bob" ) ) );

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

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
      comment.json = "{ \"valid\": true }";
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
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      moderation_tag_operation tag;

      tag.signatory = "bob";
      tag.moderator = "bob";
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

      generate_blocks(10);
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when moderating a non-existent comment" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account is not governance account" );

      tx.operations.clear();
      tx.signatures.clear();

      tag.permlink = "lorem";

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks(10);

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account is not governance account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: successful moderation tag" );

      account_membership_operation mem;

      mem.signatory = "bob";
      mem.account = "bob";
      mem.membership_type = "top";
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

      const governance_account_object& governance = db.get_governance_account( account_name_type( "bob" ) );
      
      BOOST_REQUIRE( governance.active == true );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const comment_object& alice_comment = db.get_comment( account_name_type( "alice" ), string( "lorem" ) );

      const auto& tag_idx = db.get_index< moderation_tag_index >().indices().get< by_comment_moderator >();
      auto tag_itr = tag_idx.find( std::make_tuple( alice_comment.id, account_name_type( "bob" ) ) );

      BOOST_REQUIRE( tag_itr != tag_idx.end() );
      BOOST_REQUIRE( to_string( tag_itr->details ) == tag.details );
      BOOST_REQUIRE( tag_itr->tags[0] == tag.tags[0] );
      BOOST_REQUIRE( tag_itr->filter == tag.filter );

      BOOST_TEST_MESSAGE( "│   ├── Passed: successful moderation tag" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing moderation tag" );

      generate_blocks(10);

      tag.applied = false;

      tx.operations.push_back( tag );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tag_itr = tag_idx.find( std::make_tuple( alice_comment.id, account_name_type( "bob" ) ) );

      BOOST_REQUIRE( tag_itr == tag_idx.end() );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing moderation tag" );

      BOOST_TEST_MESSAGE( "├── Passed: MODERATION TAG OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( list_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: LIST OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Creating new List" );

      ACTORS( (alice)(bob) );

      fund_stake( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 10000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "lorem";
      comment.title = "Lorem Ipsum";
      comment.body = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      comment.ipfs = "QmZdqQYUhA6yD1911YnkLYKpc4YVKL3vk6UfKUafRt5BpB";
      comment.magnet = "magnet:?xt=urn:btih:2b415a885a3e2210a6ef1d6c57eba325f20d8bc6&";
      comment.url = "https://www.url.com";
      comment.community = INIT_COMMUNITY;
      comment.tags.push_back( tag_name_type( "test" ) );
      comment.interface = INIT_ACCOUNT;
      comment.language = "en";
      comment.parent_author = "";
      comment.parent_permlink = "ipsum";
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

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( comment );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const comment_object alice_comment = db.get_comment( account_name_type( "alice" ), string( "lorem" ) );

      list_operation list;

      list.signatory = "alice";
      list.creator = "alice";
      list.list_id = "8a5c4916-6008-4d40-a1f2-3d50b44ac535";
      list.name = "Alice's Favourites";
      list.accounts.insert( bob_id._id );
      list.comments.insert( alice_comment.id._id );
      list.validate();

      tx.operations.push_back( list );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      const list_object& alice_list = db.get_list( account_name_type( "alice" ), string( "8a5c4916-6008-4d40-a1f2-3d50b44ac535" ) );

      BOOST_REQUIRE( list.creator == alice_list.creator );
      BOOST_REQUIRE( list.list_id == to_string( alice_list.list_id ) );
      BOOST_REQUIRE( list.name == to_string( alice_list.name ) );
      BOOST_REQUIRE( *list.accounts.begin() == *alice_list.accounts.begin() );
      BOOST_REQUIRE( *list.comments.begin() == *alice_list.comments.begin() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Creating new List" );

      BOOST_TEST_MESSAGE( "├── Passed: LIST OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( poll_operation_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: POLL OPERATION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Creating new Poll" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_liquid( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_blocks( TOTAL_PRODUCERS );

      signed_transaction tx;

      poll_operation poll;

      poll.signatory = "alice";
      poll.creator = "alice";
      poll.poll_id = "2aafe6cf-8d37-4467-a8ce-d55d12a3f492";
      poll.details = "Would you rather fight 100 duck sized horses, or 1 horse sized duck?";
      poll.poll_options = { "100 Duck sized horses", "1 Horse sized duck" };
      poll.completion_time = now() + fc::days(3);
      poll.validate();

      tx.operations.push_back( poll );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const poll_object& alice_poll = db.get_poll( account_name_type( "alice" ), string( "2aafe6cf-8d37-4467-a8ce-d55d12a3f492" ) );

      BOOST_REQUIRE( poll.creator == alice_poll.creator );
      BOOST_REQUIRE( poll.poll_id == to_string( alice_poll.poll_id ) );
      BOOST_REQUIRE( poll.details == to_string( alice_poll.details ) );

      for( size_t i = 0; i < poll.poll_options.size(); i++ )
      {
         BOOST_REQUIRE( poll.poll_options[ i ] == alice_poll.poll_options[ i ] );
      }

      BOOST_TEST_MESSAGE( "│   ├── Passed: Creating new Poll" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Creating Poll vote" );

      poll_vote_operation vote;

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.creator = "alice";
      vote.poll_id = "2aafe6cf-8d37-4467-a8ce-d55d12a3f492";
      vote.poll_option = 1;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      const poll_vote_object& bob_vote = db.get_poll_vote( account_name_type( "bob" ), account_name_type( "alice" ), string( "2aafe6cf-8d37-4467-a8ce-d55d12a3f492" ) );

      BOOST_REQUIRE( vote.creator == bob_vote.creator );
      BOOST_REQUIRE( vote.voter == bob_vote.voter );
      BOOST_REQUIRE( vote.poll_id == to_string( bob_vote.poll_id ) );
      BOOST_REQUIRE( alice_poll.poll_options[ vote.poll_option ] == bob_vote.poll_option );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Creating Poll vote" );

      BOOST_TEST_MESSAGE( "├── Passed: POLL OPERATION" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()