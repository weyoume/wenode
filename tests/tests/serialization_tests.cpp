

#include <node/protocol/exceptions.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <boost/test/unit_test.hpp>
#include <fc/crypto/digest.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/reflect/variant.hpp>
#include "../common/database_fixture.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace node;
using namespace node::chain;
using namespace node::protocol;
using std::string;

BOOST_FIXTURE_TEST_SUITE( serialization_tests, clean_database_fixture );



   //=============================//
   // === Serialization Tests === //
   //=============================//


BOOST_AUTO_TEST_CASE( account_name_type_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ACCOUNT NAME TYPE PACKING" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Ensure packing a string is identical to packing account name type" );

      auto test = []( const string& data )
      {
         account_name_type a(data);
         std::string       b(data);

         vector<char> a_packed = fc::raw::pack( a );
         vector<char> b_packed = fc::raw::pack( b );

         BOOST_REQUIRE( a_packed.size() == b_packed.size() );
         BOOST_REQUIRE( std::equal( a_packed.begin(), a_packed.end(), b_packed.begin() ) );

         account_name_type a_unpacked = fc::raw::unpack< account_name_type >( a_packed );
         std::string b_unpacked = fc::raw::unpack< std::string >( b_packed );

         BOOST_REQUIRE( a_unpacked == b_unpacked );
      };

      test( std::string() );
      test( std::string( "helloworld" ) );
      test( std::string( "1234567890" ) );
      test( std::string( "12345678901234567890" ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Ensure packing a string is identical to packing account name type" );

      BOOST_TEST_MESSAGE( "├── Passed: ACCOUNT NAME TYPE PACKING" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( serialization_raw_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: SERIALIZATION RAW TRANSACTION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Serialization of a transfer transaction" );

      ACTORS( (alice)(bob) );

      transfer_operation transfer;

      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( transfer );
      tx.sign( alice_private_active_key, db.get_chain_id() );

      vector<char> packed_tx = fc::raw::pack( tx );

      signed_transaction unpacked_tx = fc::raw::unpack< signed_transaction >( packed_tx );

      unpacked_tx.validate();

      BOOST_REQUIRE( tx.digest() == unpacked_tx.digest() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Serialization of a transfer transaction" );

      BOOST_TEST_MESSAGE( "├── Passed: SERIALIZATION RAW TRANSACTION" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( serialization_json_test )
{
   try 
   {
      BOOST_TEST_MESSAGE( "├── Testing: SERIALIZATION JSON TRANSACTION" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Serialization of a transfer transaction" );

      ACTORS( (alice)(bob) );

      transfer_operation transfer;

      transfer.from = "alice";
      transfer.to = "bob";
      transfer.amount = asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN );
      transfer.memo = "Hello";
      transfer.validate();

      fc::variant test( transfer.amount );
      asset tmp = test.as< asset >();
      BOOST_REQUIRE( tmp == transfer.amount );

      signed_transaction tx;

      tx.set_expiration( now() + fc::seconds( MAX_TIME_UNTIL_EXPIRATION ) );
      tx.set_reference_block( db.head_block_id() );
      tx.operations.push_back( transfer );

      fc::variant packed(tx);

      signed_transaction unpacked = packed.as< signed_transaction >();
      unpacked.validate();

      BOOST_REQUIRE( tx.digest() == unpacked.digest() );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Serialization of a transfer transaction" );

      BOOST_TEST_MESSAGE( "├── Passed: SERIALIZATION JSON TRANSACTION" );
   } 
   FC_LOG_AND_RETHROW()
}



BOOST_AUTO_TEST_CASE( asset_type_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: ASSET TYPE TEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Asset to string conversion" );

      BOOST_REQUIRE( asset().precision == 8 );
      BOOST_REQUIRE( asset().symbol == SYMBOL_COIN );
      BOOST_REQUIRE( asset().to_string() == string( "0.00000000 MEC" ) );
      
      asset COIN = asset::from_string( "123.45678901 MEC" );
      asset USD = asset::from_string( "654.32100000 MUSD" );
      asset tmp = asset::from_string( "0.00001234 MEC" );

      ilog( "Asset Test: ${coin} ${USD} ${tmp}",
         ("coin",COIN)("USD",USD)("tmp",tmp) );

      BOOST_REQUIRE( std::abs( COIN.to_real() - 123.45678901 ) < 0.0005 );
      BOOST_REQUIRE( COIN.amount.value == 12345678901 );
      BOOST_REQUIRE( COIN.precision == 8 );
      BOOST_REQUIRE( COIN.to_string() == string( "123.45678901 MEC" ) );
      BOOST_REQUIRE( COIN.symbol == SYMBOL_COIN );
      
      BOOST_REQUIRE( std::abs( USD.to_real() - 654.321 ) < 0.0005 );
      BOOST_REQUIRE( USD.amount.value == 65432100000 );
      BOOST_REQUIRE( USD.precision == 8 );
      BOOST_REQUIRE( USD.to_string() == string( "654.32100000 MUSD" ) );
      BOOST_REQUIRE( USD.symbol == SYMBOL_USD );
      BOOST_REQUIRE( asset( 50, SYMBOL_USD ).to_string() == string( "0.00000050 MUSD" ) );
      BOOST_REQUIRE( asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_USD ).to_string() == string( "50.00000000 MUSD" ) );

      BOOST_REQUIRE( tmp.amount.value == 1234 );

      tmp = asset::from_string( "0.56780000 MEC" );

      BOOST_REQUIRE( tmp.amount.value == 56780000 );

      BOOST_REQUIRE( asset( 50, SYMBOL_COIN ).to_string() == string( "0.00000050 MEC" ) );
      BOOST_REQUIRE( asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ).to_string() == string( "50.00000000 MEC" ) );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Asset to string conversion" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Asset from string conversion" );

      BOOST_CHECK_THROW( asset::from_string( "1.000000000000000000000000 MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1.000MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1. 333 MEC" ), fc::exception ); // Fails because symbol is '333 MEC', which is too long
      BOOST_CHECK_THROW( asset::from_string( "1 .333 MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1 .333 X" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1 .333" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1 1.1" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "11111111111111111111111111111111111111111111111 MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1.1.1 MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1.abc MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( " MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "MEC" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1.333" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "1.333 " ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "" ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( " " ), fc::exception );
      BOOST_CHECK_THROW( asset::from_string( "  " ), fc::exception );

      BOOST_REQUIRE( asset::from_string( "1.00000000 MEC" ).amount.value == 100000000 );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Asset from string conversion" );

      BOOST_TEST_MESSAGE( "├── Passed: ASSET TYPE TEST" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( extended_private_key_type_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: EXTENDED PRIVATE KEY TYPE TEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Extended private key string conversion" );

      fc::ecc::extended_private_key key = fc::ecc::extended_private_key( fc::ecc::private_key::generate(), fc::sha256(), 0, 0, 0 );

      extended_private_key_type type = extended_private_key_type( key );

      string packed = string( type );

      extended_private_key_type unpacked = extended_private_key_type( packed );

      BOOST_REQUIRE( type == unpacked );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Extended private key string conversion" );

      BOOST_TEST_MESSAGE( "├── Passed: EXTENDED PRIVATE KEY TYPE TEST" );
   } 
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( extended_public_key_type_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: EXTENDED PUBLIC KEY TYPE TEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Extended public key string conversion" );

      fc::ecc::extended_public_key key = fc::ecc::extended_public_key( fc::ecc::private_key::generate().get_public_key(), fc::sha256(), 0, 0, 0 );

      extended_public_key_type type = extended_public_key_type( key );

      string packed = string( type );

      extended_public_key_type unpacked = extended_public_key_type( packed );

      BOOST_REQUIRE( type == unpacked );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Extended public key string conversion" );

      BOOST_TEST_MESSAGE( "├── Passed: EXTENDED PUBLIC KEY TYPE TEST" );
   } 
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( version_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: VERSION TYPE TEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Version variant conversion" );

      BOOST_REQUIRE_EQUAL( string( version( 1, 2, 3) ), "1.2.3" );

      fc::variant ver_str( "3.0.0" );
      version ver;
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == version( 3, 0 , 0 ) );

      ver_str = fc::variant( "0.0.0" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == version() );

      ver_str = fc::variant( "1.0.1" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == version( 1, 0, 1 ) );

      ver_str = fc::variant( "1_0_1" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == version( 1, 0, 1 ) );

      ver_str = fc::variant( "12.34.56" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == version( 12, 34, 56 ) );

      ver_str = fc::variant( "256.0.0" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      ver_str = fc::variant( "0.256.0" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      ver_str = fc::variant( "0.0.65536" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      ver_str = fc::variant( "1.0" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      ver_str = fc::variant( "1.0.0.1" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Version variant conversion" );

      BOOST_TEST_MESSAGE( "├── Passed: VERSION TYPE TEST" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( hardfork_version_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: HARDFORK VERSION TYPE TEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Hardfork Version variant conversion" );

      BOOST_REQUIRE_EQUAL( string( hardfork_version( 1, 2 ) ), "1.2.0" );

      fc::variant ver_str( "3.0.0" );
      hardfork_version ver;
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == hardfork_version( 3, 0 ) );

      ver_str = fc::variant( "0.0.0" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == hardfork_version() );

      ver_str = fc::variant( "1.0.0" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == hardfork_version( 1, 0 ) );

      ver_str = fc::variant( "1_0_0" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == hardfork_version( 1, 0 ) );

      ver_str = fc::variant( "12.34.00" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == hardfork_version( 12, 34 ) );

      ver_str = fc::variant( "256.0.0" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      ver_str = fc::variant( "0.256.0" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      ver_str = fc::variant( "0.0.1" );
      fc::from_variant( ver_str, ver );
      BOOST_REQUIRE( ver == hardfork_version( 0, 0 ) );

      ver_str = fc::variant( "1.0" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      ver_str = fc::variant( "1.0.0.1" );
      REQUIRE_THROW( fc::from_variant( ver_str, ver ), fc::exception );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Hardfork Version variant conversion" );

      BOOST_TEST_MESSAGE( "├── Passed: HARDFORK VERSION TYPE TEST" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( min_block_size )
{
   try
   {
      BOOST_TEST_MESSAGE( "├── Testing: MIN BLOCK SIZE TEST" );

      BOOST_TEST_MESSAGE( "│   ├── Testing: Ensure pack size is min size" );

      signed_block b;
      b.producer = "aaa";
      size_t min_size = fc::raw::pack_size( b );

      ilog( "Min Block size: ${s}",
         ("s",min_size));
         
      BOOST_REQUIRE( min_size == MIN_BLOCK_SIZE );

      BOOST_TEST_MESSAGE( "│   ├── Passed: Ensure pack size is min size" );

      BOOST_TEST_MESSAGE( "├── Passed: MIN BLOCK SIZE TEST" );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()