//#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <node/chain/node_objects.hpp>
#include <node/chain/database.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/reflect/variant.hpp>

#include "../common/database_fixture.hpp"

#include <cmath>

using namespace node;
using namespace node::chain;
using namespace node::protocol;

BOOST_FIXTURE_TEST_SUITE( serialization_MEC, clean_database_fixture )

BOOST_AUTO_TEST_CASE( account_name_type_test )
{
   auto test = []( const string& data )
   {
      fixed_string<> a(data);
      std::string    b(data);

      auto ap = fc::raw::pack( a );
      auto bp = fc::raw::pack( b );

      FC_ASSERT( ap.size() == bp.size() );
      FC_ASSERT( std::equal( ap.begin(), ap.end(), bp.begin() ) );

      auto sfa = fc::raw::unpack<std::string>( ap );
      auto afs = fc::raw::unpack<fixed_string<>>( bp );
   };

   test( std::string() );
   test( "helloworld" );
   test( "1234567890123456" );

   auto packed_long_string = fc::raw::pack( std::string( "12345678901234567890" ) );
   auto unpacked = fc::raw::unpack<fixed_string<>>( packed_long_string );
   idump( (unpacked) );
}


BOOST_AUTO_TEST_CASE( serialization_raw_test )
{
   try 
   {
      ACTORS( (alice)(bob) );

      transfer_operation op;

      op.signatory = "alice";
      op.from = "alice";
      op.to = "bob";
      op.amount = asset( 1000, SYMBOL_COIN );
      op.memo = "Hello";

      trx.operations.push_back( op );
      auto packed = fc::raw::pack( trx );
      signed_transaction unpacked = fc::raw::unpack<signed_transaction>(packed);
      unpacked.validate();
      BOOST_CHECK( trx.digest() == unpacked.digest() );
   } 
   catch( fc::exception& e ) 
   {
      edump((e.to_detail_string()));
      throw;
   }
}


BOOST_AUTO_TEST_CASE( serialization_json_test )
{
   try 
   {
      ACTORS( (alice)(bob) );

      transfer_operation op;

      op.signatory = "alice";
      op.from = "alice";
      op.to = "bob";
      op.amount = asset( 1000, SYMBOL_COIN );
      op.memo = "Hello";

      fc::variant test( op.amount );
      auto tmp = test.as<asset>();
      BOOST_REQUIRE( tmp == op.amount );

      trx.operations.push_back( op );
      fc::variant packed(trx);
      signed_transaction unpacked = packed.as<signed_transaction>();
      unpacked.validate();
      BOOST_CHECK( trx.digest() == unpacked.digest() );
   } 
   catch (fc::exception& e) 
   {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_CASE( asset_test )
{
   try
   {
      BOOST_TEST_MESSAGE( "Asset Test" );

      BOOST_CHECK_EQUAL( asset().precision, 8 );
      BOOST_CHECK_EQUAL( asset().symbol, SYMBOL_COIN );
      BOOST_CHECK_EQUAL( asset().to_string(), "0.00000000 MEC" );
      
      asset COIN = asset::from_string( "123.45678901 MEC" );
      asset USD = asset::from_string( "654.32100000 MUSD" );
      asset tmp = asset::from_string( "0.00001234 MEC" );
      BOOST_CHECK_EQUAL( tmp.amount.value, 1234 );
      tmp = asset::from_string( "0.56780000 MEC" );
      BOOST_CHECK_EQUAL( tmp.amount.value, 56780000 );

      BOOST_CHECK( std::abs( COIN.to_real() - 123.45678901 ) < 0.0005 );

      BOOST_CHECK_EQUAL( COIN.amount.value, 12345678901 );
      BOOST_CHECK_EQUAL( COIN.precision, 8 );
      BOOST_CHECK_EQUAL( COIN.symbol, "MEC" );
      BOOST_CHECK_EQUAL( COIN.to_string(), "123.45678901 MEC" );
      BOOST_CHECK_EQUAL( COIN.symbol, SYMBOL_COIN );
      BOOST_CHECK_EQUAL( asset( 50, SYMBOL_COIN ).to_string(), "0.00000050 MEC" );
      BOOST_CHECK_EQUAL( asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_COIN ).to_string(), "50.00000000 MEC" );

      BOOST_CHECK( std::abs( USD.to_real() - 654.321 ) < 0.0005 );
      BOOST_CHECK_EQUAL( USD.amount.value, 65432100000 );
      BOOST_CHECK_EQUAL( USD.precision, 8 );
      BOOST_CHECK_EQUAL( USD.symbol, "USD" );
      BOOST_CHECK_EQUAL( USD.to_string(), "654.32100000 USD" );
      BOOST_CHECK_EQUAL( USD.symbol, SYMBOL_USD );
      BOOST_CHECK_EQUAL( asset( 50, SYMBOL_USD ).to_string(), "0.00000050 USD" );
      BOOST_CHECK_EQUAL( asset( 50 * BLOCKCHAIN_PRECISION, SYMBOL_USD ).to_string(), "50.00000000 USD" );

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

      BOOST_CHECK_EQUAL( asset::from_string( "1.00000000 MEC" ).amount.value, 100000000 );
   }
   FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( extended_private_key_type_test )
{
   try
   {
     fc::ecc::extended_private_key key = fc::ecc::extended_private_key( fc::ecc::private_key::generate(),
                                                                       fc::sha256(),
                                                                       0, 0, 0 );
      extended_private_key_type type = extended_private_key_type( key );
      std::string packed = std::string( type );
      extended_private_key_type unpacked = extended_private_key_type( packed );
      BOOST_CHECK( type == unpacked );
   } catch ( const fc::exception& e )
   {
      edump((e.to_detail_string()));
      throw;
   }
}


BOOST_AUTO_TEST_CASE( extended_public_key_type_test )
{
   try
   {
      fc::ecc::extended_public_key key = fc::ecc::extended_public_key( fc::ecc::private_key::generate().get_public_key(),
                                                                       fc::sha256(),
                                                                       0, 0, 0 );
      extended_public_key_type type = extended_public_key_type( key );
      std::string packed = std::string( type );
      extended_public_key_type unpacked = extended_public_key_type( packed );
      BOOST_CHECK( type == unpacked );
   } catch ( const fc::exception& e )
   {
      edump((e.to_detail_string()));
      throw;
   }
}


BOOST_AUTO_TEST_CASE( version_test )
{
   try
   {
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
   }
   FC_LOG_AND_RETHROW();
}


BOOST_AUTO_TEST_CASE( hardfork_version_test )
{
   try
   {
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
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( min_block_size )
{
   signed_block b;
   while( b.producer.length() < MIN_ACCOUNT_NAME_LENGTH )
   {
      b.producer += 'a';
   }
      
   size_t min_size = fc::raw::pack_size( b );
   BOOST_CHECK( min_size == MIN_BLOCK_SIZE );
}

BOOST_AUTO_TEST_SUITE_END()
//#endif