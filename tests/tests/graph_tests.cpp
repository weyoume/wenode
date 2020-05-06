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

BOOST_FIXTURE_TEST_SUITE( graph_operation_tests, clean_database_fixture );


   //===========================//
   // === Advertising Tests === //
   //===========================//



BOOST_AUTO_TEST_CASE( graph_operation_sequence_test )
{ 
   try 
   {
      BOOST_TEST_MESSAGE( "├── Passed: GRAPH OPERATION SEQUENCE" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Successful graph node property creation" );

      ACTORS( (alice)(bob) );

      fund_stake( "alice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "alice", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      fund_stake( "bob", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_EQUITY ) );
      fund( "bob", asset( 100000*BLOCKCHAIN_PRECISION, SYMBOL_COIN ) );

      signed_transaction tx;

      graph_node_property_operation node_property;

      node_property.signatory = "alice";
      node_property.account = "alice";
      node_property.node_type = "person";
      node_property.graph_privacy = "connection";
      node_property.edge_permission = "connection";
      node_property.details = "Details";
      node_property.url = "www.url.com";
      node_property.json = "{\"json\":\"valid\"}";
      node_property.attributes = { "age", "relationship_status" };
      node_property.interface = INIT_ACCOUNT;
      node_property.validate();

      tx.operations.push_back( node_property );
      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const graph_node_property_object& person_node = db.get_graph_node_property( "person" );

      BOOST_REQUIRE( node_property.account == person_node.account );
      BOOST_REQUIRE( node_property.node_type == person_node.node_type );
      BOOST_REQUIRE( node_property.details == to_string( person_node.details ) );
      BOOST_REQUIRE( node_property.url == to_string( person_node.url ) );
      BOOST_REQUIRE( node_property.json == to_string( person_node.json ) );
      BOOST_REQUIRE( person_node.graph_privacy == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( person_node.edge_permission == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( node_property.interface == person_node.interface );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Successful graph node property creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Successful graph edge property creation" );

      graph_edge_property_operation edge_property;

      edge_property.signatory = "alice";
      edge_property.account = "alice";
      edge_property.edge_type = "family";
      edge_property.graph_privacy = "connection";
      edge_property.from_node_types = { "person" };
      edge_property.to_node_types = { "person" };
      edge_property.details = "Details";
      edge_property.url = "www.url.com";
      edge_property.json = "{\"json\":\"valid\"}";
      edge_property.attributes = { "relationship" };
      edge_property.interface = INIT_ACCOUNT;
      edge_property.validate();

      tx.operations.push_back( edge_property );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const graph_edge_property_object& family_edge = db.get_graph_edge_property( "family" );

      BOOST_REQUIRE( edge_property.account == family_edge.account );
      BOOST_REQUIRE( edge_property.edge_type == family_edge.edge_type );
      BOOST_REQUIRE( family_edge.graph_privacy == connection_tier_type::CONNECTION );
      BOOST_REQUIRE( edge_property.from_node_types[0] == family_edge.from_node_types[0] );
      BOOST_REQUIRE( edge_property.to_node_types[0] == family_edge.to_node_types[0] );
      BOOST_REQUIRE( edge_property.details == to_string( family_edge.details ) );
      BOOST_REQUIRE( edge_property.url == to_string( family_edge.url ) );
      BOOST_REQUIRE( edge_property.json == to_string( family_edge.json ) );
      BOOST_REQUIRE( edge_property.interface == family_edge.interface );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Successful graph edge property creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Sucessful graph node creation" );

      graph_node_operation node;

      node.signatory = "alice";
      node.account = "alice";
      node.node_types = { "person" };
      node.node_id = "939f0aa6-cd4f-4065-83e3-f54ce368bffc";
      node.name = "Alice Person";
      node.details = "Details";
      node.attributes = { "age", "relationship_status" };
      node.attribute_values = { "24", "in_relationship" };
      node.json = "{\"json\":\"valid\"}";
      node.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#{\"json\":\"valid\"}" ) );
      node.node_public_key = string( alice_public_connection_key );
      node.interface = INIT_ACCOUNT;
      node.validate();

      tx.operations.push_back( node );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const graph_node_object& alice_person = db.get_graph_node( "alice", string( "939f0aa6-cd4f-4065-83e3-f54ce368bffc" ) );

      BOOST_REQUIRE( node.account == alice_person.account );

      for( size_t i = 0; i < alice_person.node_types.size(); i++ )
      {
         BOOST_REQUIRE( node.node_types[i] == alice_person.node_types[i] );
      }

      BOOST_REQUIRE( node.node_id == to_string( alice_person.node_id ) );
      BOOST_REQUIRE( node.name == to_string( alice_person.name ) );
      BOOST_REQUIRE( node.details == to_string( alice_person.details ) );

      for( size_t i = 0; i < alice_person.attributes.size(); i++ )
      {
         BOOST_REQUIRE( node.attributes[i] == to_string( alice_person.attributes[i] ) );
         BOOST_REQUIRE( node.attribute_values[i] == to_string( alice_person.attribute_values[i] ) );
      }

      BOOST_REQUIRE( node.json == to_string( alice_person.json ) );
      BOOST_REQUIRE( node.json_private == to_string( alice_person.json_private ) );
      BOOST_REQUIRE( node.node_public_key == string( alice_person.node_public_key ) );
      BOOST_REQUIRE( node.interface == alice_person.interface );
      BOOST_REQUIRE( alice_person.created == now() );
      BOOST_REQUIRE( alice_person.last_updated == now() );

      node.signatory = "bob";
      node.account = "bob";
      node.node_types = { "person" };
      node.node_id = "772a9705-091a-43f6-841c-720d0e0fb9f3";
      node.name = "Bob Person";
      node.details = "Details";
      node.attributes = { "age", "relationship_status" };
      node.attribute_values = { "24", "in_relationship" };
      node.json = "{\"json\":\"valid\"}";
      node.json_private = "{\"json\":\"valid\"}";
      node.node_public_key = string( bob_public_connection_key );
      node.interface = INIT_ACCOUNT;
      node.validate();

      tx.operations.push_back( node );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const graph_node_object& bob_person = db.get_graph_node( "bob", string( "772a9705-091a-43f6-841c-720d0e0fb9f3" ) );

      BOOST_REQUIRE( node.account == bob_person.account );

      for( size_t i = 0; i < bob_person.node_types.size(); i++ )
      {
         BOOST_REQUIRE( node.node_types[i] == bob_person.node_types[i] );
      }

      BOOST_REQUIRE( node.node_id == to_string( bob_person.node_id ) );
      BOOST_REQUIRE( node.name == to_string( bob_person.name ) );
      BOOST_REQUIRE( node.details == to_string( bob_person.details ) );

      for( size_t i = 0; i < bob_person.attributes.size(); i++ )
      {
         BOOST_REQUIRE( node.attributes[i] == to_string( bob_person.attributes[i] ) );
         BOOST_REQUIRE( node.attribute_values[i] == to_string( bob_person.attribute_values[i] ) );
      }

      BOOST_REQUIRE( node.json == to_string( bob_person.json ) );
      BOOST_REQUIRE( node.json_private == to_string( bob_person.json_private ) );
      BOOST_REQUIRE( node.node_public_key == string( bob_person.node_public_key ) );
      BOOST_REQUIRE( node.interface == bob_person.interface );
      BOOST_REQUIRE( bob_person.created == now() );
      BOOST_REQUIRE( bob_person.last_updated == now() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Sucessful graph node creation" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Failure when creating edge without connections" );

      graph_edge_operation edge;

      edge.signatory = "alice";
      edge.account = "alice";
      edge.edge_types = { "family" };
      edge.edge_id = "6a9a6cd7-8fe2-4135-9758-92b0378b7dc6";
      edge.from_node_account = "alice";
      edge.from_node_id = "939f0aa6-cd4f-4065-83e3-f54ce368bffc";
      edge.to_node_account = "bob";
      edge.to_node_id = "772a9705-091a-43f6-841c-720d0e0fb9f3";
      edge.name = "Alice -> Bob Family";
      edge.details = "Alice and Bob are family members";
      edge.attributes = { "relationship" };
      edge.attribute_values = { "wife" };
      edge.json = "{\"json\":\"valid\"}";
      edge.json_private = get_encrypted_message( alice_private_secure_key, alice_public_secure_key, alice_public_connection_key, string( "#{\"json\":\"valid\"}" ) );
      edge.edge_public_key = string( alice_public_connection_key );
      edge.interface = INIT_ACCOUNT;
      edge.validate();

      tx.operations.push_back( edge );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      REQUIRE_THROW( db.push_transaction( tx, 0 ), fc::exception );

      tx.operations.clear();
      tx.signatures.clear();

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Failure when creating edge without connections" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Successful edge creation after connection" );

      connection_request_operation request;

      request.signatory = "alice";
      request.account = "alice";
      request.requested_account = "bob";
      request.connection_type = "connection";
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
      accept.connection_type = "connection";
      accept.connection_id = "adad21ef-3ac5-46be-bd52-14b1c6510628";
      accept.encrypted_key = "#supersecretencryptedkeygoeshere";
      accept.connected = true;

      tx.operations.push_back( accept );
      tx.sign( bob_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      tx.operations.push_back( edge );
      tx.sign( alice_private_posting_key, db.get_chain_id() );
      db.push_transaction( tx, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      const graph_edge_object& alice_family = db.get_graph_edge( "alice", string( "772a9705-091a-43f6-841c-720d0e0fb9f3" ) );

      BOOST_REQUIRE( edge.account == alice_family.account );

      for( size_t i = 0; i < alice_family.edge_types.size(); i++ )
      {
         BOOST_REQUIRE( edge.edge_types[i] == alice_family.edge_types[i] );
      }

      BOOST_REQUIRE( edge.edge_id == to_string( alice_family.edge_id ) );
      BOOST_REQUIRE( alice_person.id == alice_family.from_node );
      BOOST_REQUIRE( bob_person.id == alice_family.to_node );
      BOOST_REQUIRE( edge.name == to_string( alice_family.name ) );
      BOOST_REQUIRE( edge.details == to_string( alice_family.details ) );

      for( size_t i = 0; i < alice_family.attributes.size(); i++ )
      {
         BOOST_REQUIRE( edge.attributes[i] == to_string( alice_family.attributes[i] ) );
         BOOST_REQUIRE( edge.attribute_values[i] == to_string( alice_family.attribute_values[i] ) );
      }

      BOOST_REQUIRE( edge.json == to_string( alice_family.json ) );
      BOOST_REQUIRE( edge.json_private == to_string( alice_family.json_private ) );
      BOOST_REQUIRE( edge.edge_public_key == string( alice_family.edge_public_key ) );
      BOOST_REQUIRE( edge.interface == alice_family.interface );
      BOOST_REQUIRE( alice_family.created == now() );
      BOOST_REQUIRE( alice_family.last_updated == now() );

      validate_database();

      BOOST_TEST_MESSAGE( "│   ├── Passed: Successful edge creation after connection" );

      BOOST_TEST_MESSAGE( "├── Passed: GRAPH OPERATION SEQUENCE" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()