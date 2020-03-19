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

BOOST_FIXTURE_TEST_SUITE( account_operation_tests, clean_database_fixture );


   //=======================//
   // === Account Tests === //
   //=======================//


BOOST_AUTO_TEST_CASE( account_create_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT CREATE" );

      account_create_operation create;
      create.registrar = "alice";
      create.new_account_name = "bob";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      BOOST_TEST_MESSAGE( "│   ├── Testing: owner authority" );

      create.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      BOOST_TEST_MESSAGE( "│   ├── Passed: owner authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: active authority" );

      expected.insert( "alice" );
      create.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      BOOST_TEST_MESSAGE( "│   ├── Passed: active authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: posting authority" );

      expected.clear();
      auths.clear();
      create.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      BOOST_TEST_MESSAGE( "│   ├── Passed: posting authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: persona account creation" );

      signed_transaction tx;
      private_key_type priv_key = generate_private_key( "aliceownercorrecthorsebatterystaple" );
      asset init_starting_balance = asset( 100 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      const account_object& init = db.get_account( INIT_ACCOUNT );
      fund( INIT_ACCOUNT, init_starting_balance );
      const dynamic_global_property_object& props = db.get_dynamic_global_properties();

      create.signatory = INIT_ACCOUNT;
      create.registrar = INIT_ACCOUNT;
      create.new_account_name = "alice";
      create.referrer = INIT_ACCOUNT;
      create.proxy = INIT_ACCOUNT;
      create.governance_account = INIT_ACCOUNT;
      create.recovery_account = INIT_ACCOUNT;
      create.reset_account = INIT_ACCOUNT;
      create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      create.owner_auth = authority( 1, priv_key.get_public_key(), 1 );
      create.active_auth = authority( 2, priv_key.get_public_key(), 2 );
      create.posting_auth = authority( 1, priv_key.get_public_key(), 1 );
      create.secure_public_key = string( public_key_type( priv_key.get_public_key() ) );
      create.connection_public_key = string( public_key_type( priv_key.get_public_key() ) );
      create.friend_public_key = string( public_key_type( priv_key.get_public_key() ) );
      create.companion_public_key = string( public_key_type( priv_key.get_public_key() ) );
      create.fee = asset( 10 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( create );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      tx.validate();
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "alice" );
      const account_following_object& acct_follow = db.get_account_following( "alice" );

      BOOST_REQUIRE( acct.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct_follow.account == "alice" );
      BOOST_REQUIRE( acct.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.created == now() );
      BOOST_REQUIRE( acct_auth.owner_auth == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active_auth == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting_auth == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.connection_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.friend_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.companion_public_key == priv_key.get_public_key() );
      BOOST_REQUIRE( acct.id._id == acct_auth.id._id );
      BOOST_REQUIRE( acct.id._id == acct_follow.id._id );
      
      BOOST_REQUIRE( db.get_staked_balance( "alice", SYMBOL_COIN ) == create.fee );

      const asset& init_liquid = db.get_liquid_balance( INIT_ACCOUNT, SYMBOL_COIN );
      
      BOOST_REQUIRE( ( init_starting_balance - create.fee ) == init_liquid );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: persona account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: profile account creation" );

      tx.signatures.clear();
      tx.operations.clear();

      create.new_account_name = "bob";
      tx.operations.push_back( create );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "bob" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "bob" );
      const account_following_object& acct_follow = db.get_account_following( "bob" );

      BOOST_REQUIRE( acct.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.name == "bob" );
      BOOST_REQUIRE( acct_follow.account == "bob" );
      BOOST_REQUIRE( acct.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.created == now() );
      BOOST_REQUIRE( acct_auth.owner_auth == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active_auth == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting_auth == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.connection_public_key == public_key_type( priv_key.get_public_key() ));
      BOOST_REQUIRE( acct.friend_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.companion_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.id._id == acct_auth.id._id );
      BOOST_REQUIRE( acct.id._id == acct_follow.id._id );

      BOOST_REQUIRE( db.get_staked_balance( "bob", SYMBOL_COIN ) == create.fee );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: profile account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: business account creation" );

      account_business_operation business_create;

      business_create.signatory = "ionstudios";
      business_create.account = "ionstudios";
      business_create.business_type = business_structure_type::PUBLIC_BUSINESS;
      business_create.officer_vote_threshold = BLOCKCHAIN_PRECISION * 1000;
      business_create.business_public_key = priv_key.get_public_key();
      business_create.governance_account = INIT_ACCOUNT;
      business_create.init_ceo_account = "alice";
      business_create.validate();


      tx.signatures.clear();
      tx.operations.clear();
      
      tx.operations.push_back( create );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "ionstudios" );
      const account_business_object& acct_bus = db.get_account_business( "ionstudios" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "ionstudios" );
      const account_following_object& acct_follow = db.get_account_following( "ionstudios" );

      BOOST_REQUIRE( acct.registrar == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.name == "ionstudios" );
      BOOST_REQUIRE( acct_follow.account == "ionstudios" );
      BOOST_REQUIRE( acct_bus.account == "ionstudios" );
      BOOST_REQUIRE( acct_bus.executive_board.CHIEF_EXECUTIVE_OFFICER == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.referrer == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.proxy == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recovery_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.reset_account == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.created == now() );
      BOOST_REQUIRE( acct_auth.owner_auth == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active_auth == authority( 2, priv_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct_auth.posting_auth == authority( 1, priv_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct.secure_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.connection_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.friend_public_key == public_key_type( priv_key.get_public_key() ) );
      BOOST_REQUIRE( acct.companion_public_key == public_key_type( priv_key.get_public_key() ) );

      BOOST_REQUIRE( acct.id._id == acct_auth.id._id );
      BOOST_REQUIRE( acct.id._id == acct_follow.id._id );
      BOOST_REQUIRE( acct.id._id == acct_bus.id._id );

      BOOST_REQUIRE( db.get_staked_balance( "ionstudios", SYMBOL_COIN ) == create.fee );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: business account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure of duplicate account creation" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure of duplicate account creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when creator cannot cover fee" );

      tx.signatures.clear();
      tx.operations.clear();

      create.fee = asset( init_liquid + asset( 1 , SYMBOL_COIN ), SYMBOL_COIN );    // Fee slightly greater than liquid balance.

      create.new_account_name = "bob";
      tx.operations.push_back( create );
      tx.sign( init_account_priv_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when creator cannot cover fee" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account fee is insufficient" );

      generate_block();

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get_median_chain_properties(), [&]( median_chain_property_object& mcpo )
         {
            mcpo.account_creation_fee = asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN );    // Fee much too high.
         });
      });

      generate_block();

      tx.clear();
      create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      tx.operations.push_back( create );
      tx.sign( init_account_priv_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account fee is insufficient" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT CREATE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_update_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT UPDATE" );
      
      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account posting authority is invalid" );

      ACTORS( (alice)(bob) );

      account_update_operation update;

      update.account = "alice";
      update.details = "Details.";
      update.json = "{\"json\":\"valid\"}";
      update.posting_auth = authority();
      update.posting_auth.weight_threshold = 1;
      update.posting_auth.add_authorities( "abcdefghijklmnopq", 1 );
      update.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( update );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      try
      {
         db.push_transaction( tx, 0 );

         BOOST_FAIL( "An exception was not thrown for an invalid account name" );
      }
      catch( fc::exception& ) {}

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account posting authority is invalid" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: account update authorities" );

      update.posting_auth = authority();
      update.validate();

      private_key_type active_key = generate_private_key( "aliceactivecorrecthorsebatterystaple" );

      db.modify( db.get< account_authority_object, by_account >( "alice" ), [&]( account_authority_object& a )
      {
         a.active_auth = authority( 1, active_key.get_public_key(), 1 );
      });

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( update );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      db.push_transaction( tx, 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: account update authorities" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no signature" );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invalid signature" );

      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invalid signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when additional irrelevant signature" );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when additional irrelevant signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate signatures" );

      tx.signatures.clear();
      tx.sign( active_key, db.get_chain_id() );
      tx.sign( active_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate signatures" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success on active key" );

      tx.signatures.clear();
      tx.sign( active_key, db.get_chain_id() );    // Single signature with new correct active key
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: success on active key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success on owner key" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "│   ├── Passed: success on owner key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when updating the owner authority with an active key" );

      tx.signatures.clear();
      tx.operations.clear();
      update.owner_auth = authority( 1, active_key.get_public_key(), 1 );    
      tx.operations.push_back( update );
      tx.sign( active_key, db.get_chain_id() );    // Updates owner to active, signed with active instead of owner
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_missing_owner_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when updating the owner authority with an active key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when owner key and active key are present" );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when owner key and active key are present" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when incorrect signature" );

      tx.signatures.clear();
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0), tx_missing_owner_auth );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when incorrect signature" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when duplicate owner keys are present" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when duplicate owner keys are present" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success when updating the owner authority with an owner key" );

      tx.signatures.clear();
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: success when updating the owner authority with an owner key" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update" );

      private_key_type new_private_key = generate_private_key( "aliceownerhunter2" );

      tx.signatures.clear();
      tx.operations.clear();

      update.owner_auth = authority( 1, new_private_key.get_public_key(), 1 );
      update.active_auth = authority( 2, new_private_key.get_public_key(), 2 );
      update.secure_public_key = string( public_key_type( new_private_key.get_public_key() ) );
      update.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      update.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      update.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      update.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";

      tx.operations.push_back( update );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );
      const account_authority_object& acct_auth = db.get< account_authority_object, by_account >( "alice" );

      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct_auth.owner_auth == authority( 1, new_private_key.get_public_key(), 1 ) );
      BOOST_REQUIRE( acct_auth.active_auth == authority( 2, new_private_key.get_public_key(), 2 ) );
      BOOST_REQUIRE( acct.secure_public_key == public_key_type( new_private_key.get_public_key() ) );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when updating a non-existent account" );

      tx.operations.clear();
      tx.signatures.clear();

      update.account = "bob";
      tx.operations.push_back( update );
      tx.sign( new_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception )
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when updating a non-existent account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when account authority does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      update.account = "alice";
      update.posting_auth = authority();
      update.posting_auth.weight_threshold = 1;
      update.posting_auth.add_authorities( "dan", 1 );
      tx.operations.push_back( update );
      tx.sign( new_private_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when account authority does not exist" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT UPDATE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_membership_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT MEMBERSHIP" );
      
      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account membership" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_membership_operation membership;
      membership.signatory = "alice";
      membership.account = "alice";
      membership.membership_type = membership_tier_type::STANDARD_MEMBERSHIP;
      membership.months = 1;
      membership.interface = INIT_ACCOUNT;

      membership.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( membership );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_object& acct = db.get_account( "alice" );

      BOOST_REQUIRE( acct.name == "alice" );
      BOOST_REQUIRE( acct.membership == membership_tier_type::STANDARD_MEMBERSHIP );
      BOOST_REQUIRE( acct.membership_interface == INIT_ACCOUNT );
      BOOST_REQUIRE( acct.recurring_membership == 1 );
      BOOST_REQUIRE( acct.membership_expiration == now() + fc::days(30) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account membership" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when insufficient funds" );

      tx.operations.clear();
      tx.signatures.clear();

      membership.signatory = "bob";
      membership.account = "bob";

      tx.operations.push_back( membership );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when insufficient funds" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBERSHIP" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_vote_executive_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT VOTE EXECUTIVE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account vote executive" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );

      account_vote_executive_operation vote;
      vote.signatory = "alice";
      vote.account = "alice";
      vote.business_account = INIT_ACCOUNT;
      vote.executive_account = INIT_CEO;
      vote.role = executive_role_type::CHIEF_EXECUTIVE_OFFICER;
      vote.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_executive_vote_object& vote = db.get_account_executive_vote( "alice", INIT_ACCOUNT, INIT_CEO );

      BOOST_REQUIRE( vote.account == "alice" );
      BOOST_REQUIRE( vote.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( vote.executive_account ==  INIT_CEO );
      BOOST_REQUIRE( vote.vote_rank == 1 );
      BOOST_REQUIRE( vote.role == executive_role_type::CHIEF_EXECUTIVE_OFFICER );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote executive" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no equity voting power" );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "bob";
      vote.account = "bob";

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no equity voting power" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel vote" );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "alice";
      vote.account = "alice";
      vote.approved = false;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_executive_vote_object* vote_ptr = db.find_account_executive_vote( "alice", INIT_ACCOUNT, INIT_CEO );
      BOOST_REQUIRE( vote_ptr == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel vote" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT VOTE EXECUTIVE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_vote_officer_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT VOTE OFFICER" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account vote officer" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund_stake( "alice", asset( 10000 * BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );

      account_vote_officer_operation vote;

      vote.signatory = "alice";
      vote.account = "alice";
      vote.business_account = INIT_ACCOUNT;
      vote.officer_account = INIT_CEO;

      vote.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_officer_vote_object& vote = db.get_account_officer_vote( "alice", INIT_ACCOUNT, INIT_CEO );

      BOOST_REQUIRE( vote.account == "alice" );
      BOOST_REQUIRE( vote.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( vote.officer_account ==  INIT_CEO );
      BOOST_REQUIRE( vote.vote_rank == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account vote officer" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when no equity voting power" );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "bob";
      vote.account = "bob";

      tx.operations.push_back( vote );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when no equity voting power" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel vote" );

      tx.operations.clear();
      tx.signatures.clear();
      
      vote.signatory = "alice";
      vote.account = "alice";
      vote.approved = false;
      vote.validate();

      tx.operations.push_back( vote );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_officer_vote_object* vote_ptr = db.find_account_officer_vote( "alice", INIT_ACCOUNT, INIT_CEO );
      BOOST_REQUIRE( vote_ptr == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel vote" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT VOTE OFFICER" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_member_request_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT MEMBER REQUEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account member request" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_member_request_operation request;
      request.signatory = "alice";
      request.account = "alice";
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";

      request.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_request_object& req = db.get_account_member_request( "alice", INIT_ACCOUNT );

      BOOST_REQUIRE( req.account == "alice" );
      BOOST_REQUIRE( req.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( req.message == "Hello" );
      BOOST_REQUIRE( req.expiration == now() + CONNECTION_REQUEST_DURATION );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account member request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when request already exists" );

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when request already exists" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel request" );

      request.requested = false;
      request.validate();

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( request );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_request_object* req = db.find_account_member_request( "alice", INIT_ACCOUNT );
      BOOST_REQUIRE( req == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel request" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBER REQUEST" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_member_invite_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT MEMBER INVITE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account member invite" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_member_invite_operation invite;

      invite.signatory = "alice";
      invite.account = "alice";
      invite.business_account = INIT_ACCOUNT;
      invite.member = "bob";
      invite.message = "Hello";
      invite.encrypted_business_key = string( alice_public_owner_key );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      invite.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_invite_object& inv = db.get_account_member_invite( "bob", INIT_ACCOUNT );

      BOOST_REQUIRE( inv.account == "alice" );
      BOOST_REQUIRE( inv.business_account == INIT_ACCOUNT );
      BOOST_REQUIRE( inv.member == "bob" );
      BOOST_REQUIRE( inv.message == "Hello" );
      BOOST_REQUIRE( inv.expiration == now() + CONNECTION_REQUEST_DURATION );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account member invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invite already exists" );

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invite already exists" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel invite" );

      invite.invited = false;
      invite.validate();

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_member_invite_object* req = db.find_account_member_invite( "alice", INIT_ACCOUNT );
      BOOST_REQUIRE( req == nullptr );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel invite" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT MEMBER INVITE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_accept_request_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT ACCEPT REQUEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account accept request" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      account_member_request_operation request;

      request.signatory = "bob";
      request.account = "bob";
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";
      request.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_request_operation accept;

      accept.signatory = "alice";
      accept.account = "alice";
      accept.member = "bob";
      accept.business_account = INIT_ACCOUNT;
      accept.encrypted_business_key = string( alice_public_owner_key );
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when unauthorized request acceptance" );

      tx.operations.clear();
      tx.signatures.clear();

      account_member_request_operation request;

      request.signatory = "candice";
      request.account = "candice";
      request.business_account = INIT_ACCOUNT;
      request.message = "Hello";
      request.validate();

      signed_transaction tx;
      tx.operations.push_back( request );
      tx.sign( candice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_request_operation accept;

      accept.signatory = "dan";     // dan is not an officer, and cannot accept requests
      accept.account = "dan";
      accept.member = "candice";
      accept.business_account = INIT_ACCOUNT;
      accept.encrypted_business_key = string( candice_public_owner_key );
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when unauthorized request acceptance" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT ACCEPTED REQUEST" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_accept_invite_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT ACCEPT INVITE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account accept invite" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
      });

      account_member_invite_operation invite;

      invite.signatory = "alice";
      invite.account = "alice";
      invite.member = "bob";
      invite.business_account = INIT_ACCOUNT;
      invite.message = "Hello";
      invite.encrypted_business_key = string( alice_public_owner_key );
      invite.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( invite );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation accept;
      accept.signatory = "bob";
      accept.account = "bob";
      accept.business_account = INIT_ACCOUNT;
      accept.validate();

      tx.operations.push_back( accept );
      tx.sign( bob_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account accept invite" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when invite does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      account_accept_invite_operation invite;

      invite.signatory = "dan";   // dan has not been invited
      invite.account = "dan";
      invite.business_account = INIT_ACCOUNT;

      invite.validate();

      tx.operations.push_back( invite );
      tx.sign( dan_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when invite does not exist" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT ACCEPT INVITE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_remove_member_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT REMOVE MEMBER" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account remove member" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.officers[ "alice" ] = BLOCKCHAIN_PRECISION * 1000000;    // Add alice to officers
         b.members.insert( "bob" );      // Add bob to members
      });

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );

      account_remove_member_operation remove;

      remove.signatory = "alice";
      remove.account = "alice";
      remove.member = "bob";
      remove.business_account = INIT_ACCOUNT;
      remove.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( remove );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_business_object& bus_acc = db.get_account_business( INIT_ACCOUNT );

      BOOST_REQUIRE( !bus_acc.is_member( "bob" ) );    // Bob has been removed

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account remove member" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when unauthorized remove member" );

      tx.operations.clear();
      tx.signatures.clear();

      db.modify( bus_acc, [&]( account_business_object& b )
      {
         b.members.insert( "bob" );      // Return bob to members
      });

      BOOST_REQUIRE( bus_acc.is_member( "bob" ) );

      account_remove_member_operation remove;

      remove.signatory = "bob";    // Bob is not an officer and cannot remove alice
      remove.account = "bob";
      remove.member = "alice";
      remove.business_account = INIT_ACCOUNT;
      remove.validate();

      tx.operations.push_back( remove );
      tx.sign( bob_private_active_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      validate_database();
   
      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when unauthorized remove member" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT REMOVE MEMBER" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_update_list_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT UPDATE LIST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update list" );
      
      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      account_update_list_operation list;
      list.signatory = "alice";
      list.account = "alice";
      list.listed_account = "bob";
      list.blacklisted = true;
      list.whitelisted = false;
      
      list.validate();

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( list );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_permission_object& acc_perm = db.get_account_permissions( "alice" );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( "bob" ) != acc_perm.blacklisted_accounts.end() );    // Bob has been blacklisted

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update list" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: remove from list" );

      tx.operations.clear();
      tx.signatures.clear();
      
      list.blacklisted = false;
      list.validate();

      tx.operations.push_back( list );
      tx.sign( alice_private_active_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_permission_object& acc_perm = db.get_account_permissions( "alice" );

      BOOST_REQUIRE( acc_perm.blacklisted_accounts.find( "bob" ) == acc_perm.blacklisted_accounts.end() );    // Bob has been removed from blacklist

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: remove from list" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT UPDATE LIST" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_producer_vote_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT PRODUCER VOTE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account producer vote" );
   
      ACTORS( (alice)(bob)(candice)(dan)(elon)(sam) );

      fund( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "alice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "bob", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund( "candice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );
      fund_stake( "candice", asset( BLOCKCHAIN_PRECISION * 1000, SYMBOL_COIN ) );

      private_key_type alice_producer_key = generate_private_key( "aliceproducercorrecthorsebatterystaple" );

      producer_create(
         "alice", 
         alice_private_owner_key, 
         alice_producer_key.get_public_key()
      );

      private_key_type candice_producer_key = generate_private_key( "candiceproducercorrecthorsebatterystaple" );

      producer_create(
         "candice", 
         candice_private_owner_key, 
         candice_producer_key.get_public_key()
      );

      account_producer_vote_operation vote;
      vote.signatory = "bob";
      vote.account = "bob";
      vote.producer = "alice";
      vote.vote_rank = 1;
      vote.approved = true;

      signed_transaction tx;
      tx.set_expiration( db.head_block_time() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const producer_vote_object& vote = db.get_producer_vote( "bob", "alice" );
      const account_object& acc = db.get_account( "bob" ); 
      const producer_object& producer = db.get_producer( "alice" );
      BOOST_REQUIRE( vote.account == "bob" );
      BOOST_REQUIRE( vote.producer == "alice" );
      BOOST_REQUIRE( vote.vote_rank == 1 );
      BOOST_REQUIRE( acc.producer_vote_count == 1 );
      BOOST_REQUIRE( producer.vote_count == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional producer vote" );

      vote.producer = "candice";   // Vote for candice at rank one will move alice to rank two

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const producer_vote_object& vote_a = db.get_producer_vote( "bob", "alice" );
      const producer_vote_object& vote_c = db.get_producer_vote( "bob", "candice" );
      const account_object& acc = db.get_account( "bob" ); 
      const producer_object& producer_a = db.get_producer( "alice" );
      const producer_object& producer_c = db.get_producer( "candice" );
      BOOST_REQUIRE( vote_a.account == "bob" );
      BOOST_REQUIRE( vote_a.producer == "alice" );
      BOOST_REQUIRE( vote_a.vote_rank == 2 );
      BOOST_REQUIRE( vote_c.account == "bob" );
      BOOST_REQUIRE( vote_c.producer == "candice" );
      BOOST_REQUIRE( vote_c.vote_rank == 1 );
      BOOST_REQUIRE( acc.producer_vote_count == 2 );
      BOOST_REQUIRE( producer_a.vote_count == 1 );
      BOOST_REQUIRE( producer_c.vote_count == 1 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: additional producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel producer vote" );

      vote.approved = false;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const producer_vote_object* vote_ptr = db.find_producer_vote( "bob", "candice" );
      const account_object& acc = db.get_account( "bob" );
      const producer_object& producer = db.get_producer( "candice" );
      BOOST_REQUIRE( vote_ptr == nullptr );
      BOOST_REQUIRE( acc.producer_vote_count == 1 );
      BOOST_REQUIRE( producer.vote_count == 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: cancel additional producer vote" );

      vote.producer = "alice";

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      const producer_vote_object* vote_ptr = db.find_producer_vote( "bob", "alice" );
      const account_object& acc = db.get_account( "bob" );
      const producer_object& producer = db.get_producer( "alice" );
      BOOST_REQUIRE( vote_ptr == nullptr );
      BOOST_REQUIRE( acc.producer_vote_count == 0 );
      BOOST_REQUIRE( producer.vote_count == 0 );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: cancel additional producer vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when attempting to revoke a non-existent vote" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when attempting to revoke a non-existent vote" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting for a non-existent account" );

      tx.operations.clear();
      tx.signatures.clear();

      vote.producer = "eve";
      vote.approved = true;

      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting for a non-existent account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when voting for an account that is not a producer" );

      tx.operations.clear();
      tx.signatures.clear();
      vote.producer = "elon";
      tx.operations.push_back( vote );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when voting for an account that is not a producer" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT PRODUCER VOTE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_update_proxy_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT UPDATE PROXY" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account update proxy" );
      
      const dynamic_global_property_object& props = db.get_dynamic_global_properties();
      price equity_price = props.current_median_equity_price;

      ACTORS( (alice)(bob)(candice)(sam)(dan)(elon) )
      fund( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "candice", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "candice", asset( 3000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "sam", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "sam", asset( 5000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "dan", asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "dan", asset( 7000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_update_proxy_operation proxy;

      proxy.signatory = "bob";
      proxy.account = "bob";
      proxy.proxy = "alice";

      signed_transaction tx;
      tx.operations.push_back( proxy );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );    // bob -> alice

      BOOST_REQUIRE( bob.proxy == "alice" );
      BOOST_REQUIRE( bob.proxied.size() == 0 );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *alice.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( alice, equity_price ) == db.get_voting_power( bob, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account update proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: changing proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      proxy.proxy = "sam";
      tx.operations.push_back( proxy );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );     // bob->sam

      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( bob.proxied.size() == 0 );
      BOOST_REQUIRE( alice.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( alice.proxied.size() == 0 );
      BOOST_REQUIRE( sam.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: changing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when changing proxy to existing proxy" );

      REQUIRE_THROW( db.push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when changing proxy to existing proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandparent proxy" );
      
      tx.operations.clear();
      tx.signatures.clear();

      proxy.signatory = "sam";
      proxy.account = "sam";
      proxy.proxy = "dan";
      
      tx.operations.push_back( proxy );
      tx.sign( sam_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );     // bob->sam->dan

      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( *sam.proxied.begin() == "bob" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
      
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandparent proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: adding a grandchild proxy" );

      // alice \
      // bob->  sam->dan

      tx.operations.clear();
      tx.signatures.clear();

      proxy.signatory = "alice";
      proxy.account = "alice";
      proxy.proxy = "sam";
      
      tx.operations.push_back( proxy );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( alice.proxy == "sam" );
      BOOST_REQUIRE( bob.proxy == "sam" );
      BOOST_REQUIRE( sam.proxied.size() == 2 );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( bob, equity_price ) + db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: adding a grandchild proxy" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: removing a grandchild proxy" );

      tx.operations.clear();
      tx.signatures.clear();

      proxy.signatory = "bob";
      proxy.account = "bob";
      proxy.proxy = PROXY_TO_SELF_ACCOUNT;
      
      tx.operations.push_back( proxy );
      tx.sign( bob_private_owner_key, db.get_chain_id() );

      db.push_transaction( tx, 0 );    // alice->sam->dan

      BOOST_REQUIRE( alice.proxy == "sam" );
      BOOST_REQUIRE( bob.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( *sam.proxied.begin() == "alice" );
      BOOST_REQUIRE( db.get_proxied_voting_power( sam, equity_price ) == db.get_voting_power( alice, equity_price ) );
      BOOST_REQUIRE( sam.proxy == "dan" );
      BOOST_REQUIRE( *dan.proxied.begin() == "sam" );
      BOOST_REQUIRE( dan.proxy == PROXY_TO_SELF_ACCOUNT );
      BOOST_REQUIRE( db.get_proxied_voting_power( dan, equity_price ) == db.get_voting_power( sam, equity_price ) );
      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: removing a grandchild proxy" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT UPDATE PROXY" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_recovery_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT RECOVERY SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal request account recovery" );
      
      ACTORS( (alice)(bob)(candice)(sam)(dan) );
      fund( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_create_operation acc_create;       // Creating account bob with alice as recovery account

      acc_create.signatory = "alice";
      acc_create.registrar = "alice";
      acc_create.recovery_account = "alice";
      acc_create.reset_account = "alice";
      acc_create.new_account_name = "bob";
      acc_create.owner_auth = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      acc_create.active_auth = authority( 1, generate_private_key( "bob_active" ).get_public_key(), 1 );
      acc_create.posting_auth = authority( 1, generate_private_key( "bob_posting" ).get_public_key(), 1 );
      acc_create.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_create.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_create.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_create.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      signed_transaction tx;
      tx.operations.push_back( acc_create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      shared_authority bob_auth = db.get< account_authority_object, by_account >( "bob" );
      BOOST_REQUIRE( bob_auth.owner_auth == acc_create.owner_auth );

      // Changing bob's owner authority

      account_update_operation acc_update;

      acc_update.signatory = "bob";
      acc_update.account = "bob";
      acc_update.owner_auth = authority( 1, generate_private_key( "bad_key" ).get_public_key(), 1 );
      acc_update.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_update.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_update.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_update.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_update.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_update.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_update.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_update.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( acc_update );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_auth.owner_auth == acc_update.owner_auth );

      tx.operations.clear();
      tx.signatures.clear();

      request_account_recovery_operation request;
      request.signatory = "alice";
      request.recovery_account = "alice";
      request.account_to_recover = "bob";
      request.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( bob_auth.owner_auth == acc_update.owner_auth );

      generate_blocks( now() + OWNER_UPDATE_LIMIT );

      tx.operations.clear();
      tx.signatures.clear();

      recover_account_operation recover;
      recover.signatory = "bob";
      recover.account_to_recover = "bob";
      recover.new_owner_authority = request.new_owner_authority;
      recover.recent_owner_authority = acc_create.owner_auth;

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );
      shared_authority owner1 = db.get< account_authority_object, by_account >("bob").owner_auth;

      BOOST_REQUIRE( owner1 == recover.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal request account recovery" );

      tx.operations.clear();
      tx.signatures.clear();

      request.new_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when bob does not have new requested authority" );

      generate_blocks( now() + OWNER_UPDATE_LIMIT + BLOCK_INTERVAL );

      recover.new_owner_authority = authority( 1, generate_private_key( "idontknow" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "idontknow" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      shared_authority owner2 = db.get< account_authority_object, by_account >("bob").owner_auth;
      BOOST_REQUIRE( owner2 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when bob does not have new requested authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when bob does not have old authority" );

      recover.recent_owner_authority = authority( 1, generate_private_key( "idontknow" ).get_public_key(), 1 );
      recover.new_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      tx.sign( generate_private_key( "idontknow" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
      shared_authority owner3 = db.get< account_authority_object, by_account >("bob").owner_auth;
      BOOST_REQUIRE( owner3 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when bob does not have old authority" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: using the same old owner auth again for recovery" );

      recover.recent_owner_authority = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      recover.new_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      shared_authority owner4 = db.get< account_authority_object, by_account >("bob").owner_auth;
      BOOST_REQUIRE( owner4 == recover.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: using the same old owner auth again for recovery" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: creating a recovery request that will expire" );

      request.new_owner_authority = authority( 1, generate_private_key( "expire" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& request_idx = db.get_index< account_recovery_request_index >().indices();
      auto req_itr = request_idx.begin();

      BOOST_REQUIRE( req_itr->account_to_recover == "bob" );
      BOOST_REQUIRE( req_itr->new_owner_authority == authority( 1, generate_private_key( "expire" ).get_public_key(), 1 ) );
      BOOST_REQUIRE( req_itr->expiration == now() + ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD );

      time_point expires = req_itr->expiration;
      ++req_itr;
      BOOST_REQUIRE( req_itr == request_idx.end() );

      generate_blocks( time_point( expires - BLOCK_INTERVAL ), true );

      const auto& new_request_idx = db.get_index< account_recovery_request_index >().indices();
      BOOST_REQUIRE( new_request_idx.begin() != new_request_idx.end() );    // Request has not yet expired

      generate_block();

      BOOST_REQUIRE( new_request_idx.begin() == new_request_idx.end() );    // Request has now expired

      recover.new_owner_authority = authority( 1, generate_private_key( "expire" ).get_public_key(), 1 );
      recover.recent_owner_authority = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now() );

      tx.sign( generate_private_key( "expire" ), db.get_chain_id() );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // Recover won't work, because request expired
      shared_authority owner5 = db.get< account_authority_object, by_account >("bob").owner_auth;
      BOOST_REQUIRE( owner5 == authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: creating a recovery request that will expire" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Expiring owner authority history" );

      acc_update.owner_auth = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( acc_update );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now() + ( OWNER_AUTH_RECOVERY_PERIOD - ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD ) );
      generate_block();

      request.new_owner_authority = authority( 1, generate_private_key( "last key" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      recover.new_owner_authority = request.new_owner_authority;
      recover.recent_owner_authority = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "bob_owner" ), db.get_chain_id() );
      tx.sign( generate_private_key( "last key" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // bob_owner key has expired
      shared_authority owner6 = db.get< account_authority_object, by_account >("bob").owner_auth;
      BOOST_REQUIRE( owner6 == authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 ) );

      recover.recent_owner_authority = authority( 1, generate_private_key( "foo bar" ).get_public_key(), 1 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( recover );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( generate_private_key( "foo bar" ), db.get_chain_id() );
      tx.sign( generate_private_key( "last key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );     // foo bar key has not yet expired
      shared_authority owner7 = db.get< account_authority_object, by_account >("bob").owner_auth;
      BOOST_REQUIRE( owner7 == authority( 1, generate_private_key( "last key" ).get_public_key(), 1 ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Expiring owner authority history" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: change recovery account" );

      auto change_recovery_account = [&]( const std::string& account_to_recover, const std::string& new_recovery_account )
      {
         change_recovery_account_operation recovery;
         recovery.account_to_recover = account_to_recover;
         recovery.new_recovery_account = new_recovery_account;

         signed_transaction tx;
         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( alice_private_owner_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      auto recover_account = [&]( const std::string& account_to_recover, const fc::ecc::private_key& new_owner_key, const fc::ecc::private_key& recent_owner_key )
      {
         recover_account_operation recovery;
         recovery.account_to_recover = account_to_recover;
         recovery.new_owner_authority = authority( 1, public_key_type( new_owner_key.get_public_key() ), 1 );
         recovery.recent_owner_authority = authority( 1, public_key_type( recent_owner_key.get_public_key() ), 1 );

         signed_transaction tx;
         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( recent_owner_key, db.get_chain_id() );
         // only Alice -> throw
         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
         tx.signatures.clear();
         tx.sign( new_owner_key, db.get_chain_id() );
         // only Sam -> throw
         REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );
         tx.sign( recent_owner_key, db.get_chain_id() );
         // Alice+Sam -> OK
         db.push_transaction( tx, 0 );
      };

      auto request_account_recovery = [&]( const std::string& recovery_account, const fc::ecc::private_key& recovery_account_key, const std::string& account_to_recover, const public_key_type& new_owner_key )
      {
         request_account_recovery_operation recovery;
         recovery.recovery_account    = recovery_account;
         recovery.account_to_recover  = account_to_recover;
         recovery.new_owner_authority = authority( 1, new_owner_key, 1 );

         signed_transaction tx;
         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( recovery_account_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      auto change_owner = [&]( const std::string& account, const fc::ecc::private_key& old_private_key, const public_key_type& new_public_key )
      {
         account_update_operation recovery;
         recovery.account = account;
         recovery.owner_auth = authority( 1, new_public_key, 1 );

         signed_transaction tx;
         tx.operations.push_back( recovery );
         tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
         tx.sign( old_private_key, db.get_chain_id() );
         db.push_transaction( tx, 0 );
      };

      // if either/both users do not exist, we shouldn't allow it
      REQUIRE_THROW( change_recovery_account( "alice", "nobody" ), fc::exception );
      REQUIRE_THROW( change_recovery_account( "haxer", "sam" ), fc::exception );
      REQUIRE_THROW( change_recovery_account( "haxer", "nobody" ), fc::exception );
      change_recovery_account( "alice", "sam" );

      fc::ecc::private_key alice_priv1 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k1" ) );
      fc::ecc::private_key alice_priv2 = fc::ecc::private_key::regenerate( fc::sha256::hash( "alice_k2" ) );
      public_key_type alice_pub1 = public_key_type( alice_priv1.get_public_key() );

      generate_blocks( now() + OWNER_AUTH_RECOVERY_PERIOD - BLOCK_INTERVAL, true );
      // cannot request account recovery until recovery account is approved
      REQUIRE_THROW( request_account_recovery( "sam", sam_private_owner_key, "alice", alice_pub1 ), fc::exception );
      generate_blocks(1);
      // cannot finish account recovery until requested
      REQUIRE_THROW( recover_account( "alice", alice_priv1, alice_private_owner_key ), fc::exception );
      // do the request
      request_account_recovery( "sam", sam_private_owner_key, "alice", alice_pub1 );
      // can't recover with the current owner key
      REQUIRE_THROW( recover_account( "alice", alice_priv1, alice_private_owner_key ), fc::exception );
      // unless we change it!
      change_owner( "alice", alice_private_owner_key, public_key_type( alice_priv2.get_public_key() ) );
      recover_account( "alice", alice_priv1, alice_private_owner_key );

      BOOST_TEST_MESSAGE( "│   ├── Passed: change_recovery_account_operation" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT RECOVERY SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_reset_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT RESET SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account reset" );

      ACTORS( (alice)(bob)(candice)(dan) );

      fund( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      account_create_operation acc_create;       // Creating account bob with alice as reset account

      acc_create.signatory = "alice";
      acc_create.registrar = "alice";
      acc_create.recovery_account = "alice";
      acc_create.reset_account = "alice";
      acc_create.new_account_name = "bob";
      acc_create.owner_auth = authority( 1, generate_private_key( "bob_owner" ).get_public_key(), 1 );
      acc_create.active_auth = authority( 1, generate_private_key( "bob_active" ).get_public_key(), 1 );
      acc_create.posting_auth = authority( 1, generate_private_key( "bob_posting" ).get_public_key(), 1 );
      acc_create.secure_public_key = string( public_key_type( generate_private_key( "bob_secure" ).get_public_key() ) );
      acc_create.connection_public_key = string( public_key_type( generate_private_key( "bob_connection" ).get_public_key() ) );
      acc_create.friend_public_key = string( public_key_type( generate_private_key( "bob_friend" ).get_public_key() ) );
      acc_create.companion_public_key = string( public_key_type( generate_private_key( "bob_companion" ).get_public_key() ) );
      acc_create.details = "My Details: About 8 Storeys tall, crustacean from the Paleozoic era.";
      acc_create.url = "https://en.wikipedia.org/wiki/Loch_Ness_Monster";
      acc_create.json = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.json_private = "{\"cookie_price\":\"3.50000000 MUSD\"}";
      acc_create.fee = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );

      signed_transaction tx;
      tx.operations.push_back( acc_create );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      shared_authority bob_auth = db.get< account_authority_object, by_account >( "bob" );
      BOOST_REQUIRE( bob_auth.owner_auth == acc_create.owner_auth );

      generate_blocks( now() + ( fc::days( bob.reset_account_delay_days ) - BLOCK_INTERVAL ) );

      reset_account_operation reset;

      reset.signatory = "alice";
      reset.reset_account = "alice";
      reset.account_to_reset = "bob";
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key" ).get_public_key(), 1 );

      tx.operations.push_back( reset );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet

      generate_block();

      db.push_transaction( tx, 0 );    // reset now valid

      BOOST_REQUIRE( bob_auth.owner_auth == reset.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account reset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: set new reset account" );

      tx.operations.clear();
      tx.signatures.clear();

      set_reset_account_operation set;

      set.signatory = "bob";
      set.account = "bob";
      set.new_reset_account = "candice";

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key" ), db.get_chain_id() );
      db.push_transaction( tx, 0 );
      
      BOOST_REQUIRE( bob.reset_account == "candice" );

      BOOST_TEST_MESSAGE( "│   ├── Passed: set new reset account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: additional account reset" );

      tx.operations.clear();
      tx.signatures.clear();

      generate_blocks( now() + ( fc::days( bob.reset_account_delay_days ) - BLOCK_INTERVAL ) );

      reset.signatory = "candice";
      reset.reset_account = "candice";
      reset.account_to_reset = "bob";
      reset.new_owner_authority = authority( 1, generate_private_key( "new_key_1" ).get_public_key(), 1 );

      tx.operations.push_back( reset );
      tx.sign( candice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // reset not valid yet

      generate_block();

      db.push_transaction( tx, 0 );    // reset now valid

      BOOST_REQUIRE( bob_auth.owner_auth == reset.new_owner_authority );

      BOOST_TEST_MESSAGE( "│   ├── Passed: additional account reset" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when setting reset account that does not exist" );

      tx.operations.clear();
      tx.signatures.clear();

      set.new_reset_account = "spudington";

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key_1" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when setting reset account that does not exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when setting reset account to current reset account" );

      tx.operations.clear();
      tx.signatures.clear();

      set.new_reset_account = "candice";

      tx.operations.push_back( set );
      tx.sign( generate_private_key( "new_key_1" ), db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when setting reset account to current reset account" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT RESET SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( decline_voting_rights_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: DECLINE VOTING RIGHTS" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: decline voting rights request" );

      ACTORS( (alice)(bob) );
      generate_block();
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      generate_block();

      decline_voting_rights_operation decline;
      decline.signatory = "alice";
      decline.account = "alice";

      signed_transaction tx;
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< decline_voting_rights_request_index >().indices().get< by_account >();
      auto req_itr = req_idx.find( "alice" );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->effective_date == now() + DECLINE_VOTING_RIGHTS_DURATION );

      BOOST_TEST_MESSAGE( "│   ├── Passed: decline voting rights request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure revoking voting rights with existing request" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure revoking voting rights with existing request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: success cancelling a request" );

      decline.declined = false;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      req_itr = req_idx.find( "alice" );
      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed

      BOOST_TEST_MESSAGE( "│   ├── Passed: success cancelling a request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure cancelling a request that doesn't exist" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );   // Can't cancel again

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure cancelling a request that doesn't exist" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account can vote during waiting period" );

      decline.declined = true;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( decline );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      generate_blocks( now() + DECLINE_VOTING_RIGHTS_DURATION - BLOCK_INTERVAL, true );

      BOOST_REQUIRE( db.get_account( "alice" ).can_vote );   // Alice can still vote

      producer_create( 
         "alice", 
         alice_private_owner_key,
         alice_private_owner_key.get_public_key()
         );

      account_producer_vote_operation producer_vote;

      producer_vote.signatory = "alice";
      producer_vote.account = "alice";
      producer_vote.producer = "alice";
      producer_vote.vote_rank = 1;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      comment_operation comment;

      comment.signatory = "alice";
      comment.author = "alice";
      comment.permlink = "test";
      comment.parent_permlink = "test";
      comment.title = "test";
      comment.body = "test";
      comment.language = "en";
      comment.interface = INIT_ACCOUNT;
      comment.tags.push_back( "test" );
      comment.json = "{\"json\":\"valid\"}";

      comment_options options;

      options.post_type = post_format_type::ARTICLE_POST;
      options.reach = feed_reach_type::TAG_FEED;
      options.rating = 1;

      comment_options options;
      comment.options = options;

      vote_operation vote;

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "alice";
      vote.permlink = "test";
      vote.interface = INIT_ACCOUNT;
      vote.weight = PERCENT_100;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( comment );
      tx.operations.push_back( vote );

      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& comment_idx = db.get_index< comment_index >().indices().get< by_permlink >();
      const auto& comment_vote_idx = db.get_index< comment_vote_index >().indices().get< by_voter_comment >();
      const auto& producer_vote_idx = db.get_index< producer_vote_index >().indices().get< by_producer_account >();

      auto comment_itr = comment_idx.find( boost::make_tuple( "alice", "test" ) );
      auto comment_vote_itr = comment_vote_idx.find( boost::make_tuple( "alice", comment_itr->id ) );
      auto producer_vote_itr = producer_vote_idx.find( boost::make_tuple( "alice", "alice" ) );

      BOOST_REQUIRE( comment_itr->author == "alice" );
      BOOST_REQUIRE( comment_vote_itr->voter == "alice" );
      BOOST_REQUIRE( producer_vote_itr->account == "alice" );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account can vote during waiting period" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: check account cannot vote after request is processed" );

      generate_block();
      BOOST_REQUIRE( !db.get_account( "alice" ).can_vote );   // Alice can no longer vote
      validate_database();

      auto req_itr = req_idx.find( "alice" );
      BOOST_REQUIRE( req_itr == req_idx.end() );   // Request no longer exists

      auto producer_vote_itr = producer_vote_idx.find( boost::make_tuple( "alice", "alice" ) );
      BOOST_REQUIRE( producer_vote_itr == producer_vote_idx.end() );   // Vote has been removed

      tx.operations.clear();
      tx.signatures.clear();

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );

      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      vote.weight = 0;

      tx.operations.clear();
      tx.signatures.clear();
      
      tx.operations.push_back( vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: check account cannot vote after request is processed" );

      BOOST_TEST_MESSAGE( "├── Passed: DECLINE VOTING RIGHTS" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( connection_sequence_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: CONNECTION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: create connection request" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );
      generate_block();
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      generate_block();

      connection_request_operation request;

      request.signatory = "alice";
      request.account = "alice";
      request.requested_account = "bob";
      request.connection_type = connection_tier_type::CONNECTION;
      request.message = "Hello";
      request.requested = true;

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const auto& req_idx = db.get_index< connection_request_index >().indices().get< by_account_req >();
      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      const auto& con_idx = db.get_index< connection_index >().indices().get< by_accounts >();
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->account == "alice" );
      BOOST_REQUIRE( req_itr->requested_account == "bob" );
      BOOST_REQUIRE( req_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( req_itr->expiration == now() + CONNECTION_REQUEST_DURATION );
      BOOST_REQUIRE( con_itr == con_idx.end() );    // Connection not created yet

      BOOST_TEST_MESSAGE( "│   ├── Passed: create connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept connection request" );

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
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: match connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "alice";
      accept.account = "alice";
      accept.requesting_account = "bob";

      tx.operations.push_back( accept );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", connection_tier_type::CONNECTION ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );    // Request is now removed
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: match connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting existing connection type" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting existing connection type" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when requesting friend connection before request duration" );

      generate_blocks( now() + CONNECTION_REQUEST_DURATION - BLOCK_INTERVAL, true );

      request.connection_type = connection_tier_type::FRIEND;

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when requesting friend connection before request duration" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: friend connection request" );

      generate_block();

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( request );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( req_itr != req_idx.end() );
      BOOST_REQUIRE( req_itr->account == "alice" );
      BOOST_REQUIRE( req_itr->requested_account == "bob" );
      BOOST_REQUIRE( req_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( req_itr->expiration == now() + CONNECTION_REQUEST_DURATION );
      BOOST_REQUIRE( con_itr == con_idx.end() );    // Connection not created yet

      BOOST_TEST_MESSAGE( "│   ├── Passed: friend connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: accept friend connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "bob";
      accept.account = "bob";
      accept.requesting_account = "alice";
      accept.connection_type = connection_tier_type::FRIEND;
      accept.connection_id = "eb634e76-f478-49d5-8441-54ae22a4092c";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: accept friend connection request" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: match friend connection request" );

      tx.operations.clear();
      tx.signatures.clear();

      accept.signatory = "alice";
      accept.account = "alice";
      accept.requesting_account = "bob";
      accept.connection_type = connection_tier_type::FRIEND;

      tx.operations.push_back( accept );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      auto req_itr = req_idx.find( boost::make_tuple( "alice", "bob" ) );
      auto con_itr = con_idx.find( boost::make_tuple( "alice", "bob", connection_tier_type::FRIEND ) );

      BOOST_REQUIRE( req_itr == req_idx.end() );
      BOOST_REQUIRE( con_itr->account_a == "alice" );
      BOOST_REQUIRE( con_itr->account_b == "bob" );
      BOOST_REQUIRE( con_itr->connection_type == connection_tier_type::FRIEND );
      BOOST_REQUIRE( con_itr->encrypted_key_a.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->encrypted_key_b.encrypted_private_key == "#supersecretencryptedkeygoeshere" );
      BOOST_REQUIRE( con_itr->connection_id == "eb634e76-f478-49d5-8441-54ae22a4092c" );
      BOOST_REQUIRE( con_itr->created == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: match friend request" );

      BOOST_TEST_MESSAGE( "├── Passed: CONNECTION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( account_follow_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT FOLLOW" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal account follow" );

      ACTORS( (alice)(bob) );

      account_follow_operation follow;

      follow.signatory = "alice";
      follow.follower = "alice";
      follow.following = "bob";
      follow.interface = INIT_ACCOUNT;
      follow.added = true;
      follow.followed = true;

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( follow );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( "alice" );
      const account_following_object& following_b = db.get_account_following( "bob" );

      BOOST_REQUIRE( following_a.is_following( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_follower( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: normal account follow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: mutual following" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = "bob";
      follow.follower = "bob";
      follow.following = "alice";

      tx.operations.push_back( follow );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( following_a.is_follower( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_following( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( following_a.is_mutual( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( following_b.is_mutual( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: mutual following" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when following already followed account" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = "bob";
      follow.follower = "bob";
      follow.following = "alice";

      tx.operations.push_back( follow );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when following already followed account" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: unfollowing" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = "bob";
      follow.follower = "bob";
      follow.following = "alice";
      follow.added = false;
      follow.followed = true;

      tx.operations.push_back( follow );
      tx.sign( bob_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      follow.signatory = "alice";
      follow.follower = "alice";
      follow.following = "bob";
      follow.added = false;
      follow.followed = true;

      tx.operations.push_back( follow );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      BOOST_REQUIRE( !following_a.is_follower( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_following( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( !following_a.is_following( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_follower( account_name_type( "alice" ) ) );
      BOOST_REQUIRE( !following_a.is_mutual( account_name_type( "bob" ) ) );
      BOOST_REQUIRE( !following_b.is_mutual( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: unfollowing" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: filtering" );

      follow.signatory = "alice";
      follow.follower = "alice";
      follow.following = "bob";
      
      follow.added = true;
      follow.followed = false;

      tx.operations.push_back( follow );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( account_name_type( "alice" ) );
      const account_following_object& following_b = db.get_account_following( account_name_type( "bob" ) );

      BOOST_REQUIRE( following_a.is_filtered( account_name_type( "bob" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: filtering" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT FOLLOW" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( tag_follow_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: TAG FOLLOW" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal tag follow" );

      ACTORS( (alice)(bob)(candice)(dan)(elon) );

      tag_follow_operation follow;

      follow.signatory = "alice";
      follow.follower = "alice";
      follow.tag = "test";
      follow.interface = INIT_ACCOUNT;
      follow.added = true;
      follow.followed = true;

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( follow );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( "alice" );
      const tag_following_object& following_t = db.get_tag_following( "test" );
      
      BOOST_REQUIRE( following_a.is_followed_tag( tag_name_type( "test" ) ) );
      BOOST_REQUIRE( following_t.is_follower( account_name_type( "alice" ) ) );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal tag follow" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when following already followed tag" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( follow );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when following already followed tag" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: unfollowing" );

      tx.operations.clear();
      tx.signatures.clear();

      follow.added = false;

      tx.operations.push_back( follow );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const account_following_object& following_a = db.get_account_following( "alice" );
      const tag_following_object& following_t = db.get_tag_following( "test" );
      
      BOOST_REQUIRE( !following_a.is_followed_tag( tag_name_type( "test" ) ) );
      BOOST_REQUIRE( !following_t.is_follower( account_name_type( "alice" ) ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: unfollowing" );

      BOOST_TEST_MESSAGE( "├── Passed: TAG FOLLOW" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( activity_reward_operation_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACTIVITY REWARD" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: normal activity reward claim process" );

      ACTORS( (alice)(bob)(candice)(dan)(elon)(fred)(george)(haz)(isabelle)(jayme)(kathryn) );

      fund_stake( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "alice", alice_private_owner_key, alice_public_owner_key );

      fund_stake( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "bob", bob_private_owner_key, bob_public_owner_key );

      fund_stake( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "candice", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "candice", candice_private_owner_key, candice_public_owner_key );

      fund_stake( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "dan", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "dan", dan_private_owner_key, dan_public_owner_key );

      fund_stake( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "elon", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "elon", elon_private_owner_key, elon_public_owner_key );

      fund_stake( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "fred", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "fred", fred_private_owner_key, fred_public_owner_key );

      fund_stake( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "george", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "george", george_private_owner_key, george_public_owner_key );

      fund_stake( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "haz", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "haz", haz_private_owner_key, haz_public_owner_key );

      fund_stake( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "isabelle", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "isabelle", isabelle_private_owner_key, isabelle_public_owner_key );

      fund_stake( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "jayme", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "jayme", jayme_private_owner_key, jayme_public_owner_key );

      fund_stake( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "kathryn", asset( 1000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );
      producer_create( "kathryn", kathryn_private_owner_key, kathryn_public_owner_key );

      const comment_object& alice_post = comment_create( "alice", alice_private_posting_key, "alicetestpost" );
      const comment_object& bob_post = comment_create( "bob", bob_private_posting_key, "bobtestpost" );
      const comment_object& candice_post = comment_create( "candice", candice_private_posting_key, "candicetestpost" );

      vote_operation vote;

      vote.signatory = "bob";
      vote.voter = "bob";
      vote.author = "alice";
      vote.permlink = "alicetestpost";
      vote.weight = PERCENT_100;
      vote.interface = INIT_ACCOUNT;
      vote.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.operations.push_back( vote );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "alice";
      vote.voter = "alice";
      vote.author = "bob";
      vote.permlink = "bobtestpost";

      tx.operations.push_back( vote );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "candice";
      vote.voter = "candice";
      vote.author = "alice";
      vote.permlink = "alicetestpost";

      tx.operations.push_back( vote );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "dan";
      vote.voter = "dan";
      
      tx.operations.push_back( vote );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      vote.signatory = "elon";
      vote.voter = "elon";

      tx.operations.push_back( vote );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view_operation view;

      view.signatory = "bob";
      view.viewer = "bob";
      view.author = "alice";
      view.permlink = "alicetestpost";
      view.interface = INIT_ACCOUNT;
      view.supernode = INIT_ACCOUNT;
      view.viewed = true;
      view.validate();

      tx.operations.push_back( view );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "alice";
      view.viewer = "alice";
      view.author = "candice";
      view.permlink = "candicetestpost";

      tx.operations.push_back( view );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "candice";
      view.viewer = "candice";
      view.author = "alice";
      view.permlink = "alicetestpost";

      tx.operations.push_back( view );
      tx.sign( candice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "dan";
      view.viewer = "dan";

      tx.operations.push_back( view );
      tx.sign( dan_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      view.signatory = "elon";
      view.viewer = "elon";

      tx.operations.push_back( view );
      tx.sign( elon_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      db.update_comment_metrics();     // calculate median values.

      account_producer_vote_operation producer_vote;

      producer_vote.signatory = "alice";
      producer_vote.account = "alice";
      producer_vote.producer = "bob";
      producer_vote.vote_rank = 1;
      producer_vote.approved = true;
      producer_vote.validate();

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "candice";
      producer_vote.vote_rank = 2;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "dan";
      producer_vote.vote_rank = 3;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "elon";
      producer_vote.vote_rank = 5;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "fred";
      producer_vote.vote_rank = 6;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "george";
      producer_vote.vote_rank = 7;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "haz";
      producer_vote.vote_rank = 8;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "isabelle";
      producer_vote.vote_rank = 9;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "jayme";
      producer_vote.vote_rank = 10;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      producer_vote.producer = "kathryn";
      producer_vote.vote_rank = 11;

      tx.operations.push_back( producer_vote );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      activity_reward_operation activity;

      activity.signatory = "alice";
      activity.account = "alice";
      activity.permlink = "alicetestpost";
      activity.vote_id = bob_post.id._id;
      activity.view_id = candice_post.id._id;
      activity.interface = INIT_ACCOUNT;

      tx.operations.push_back( activity );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      const reward_fund_object& rfo = db.get_reward_fund( SYMBOL_COIN );
      
      BOOST_REQUIRE( alice.last_activity_reward == now() );
      BOOST_REQUIRE( rfo.recent_activity_claims == BLOCKCHAIN_PRECISION );
      
      BOOST_TEST_MESSAGE( "│   ├── Passed: normal activity reward claim process" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: failure when claiming again" );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( activity );
      tx.sign( alice_private_owner_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: failure when claiming again" );

      BOOST_TEST_MESSAGE( "├── Passed: ACTIVITY REWARD" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()