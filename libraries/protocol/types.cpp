#include <node/protocol/config.hpp>
#include <node/protocol/types.hpp>

#include <fc/crypto/base58.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>

namespace node { namespace protocol {

   public_key_type::public_key_type() : 
      key_data() {};

   public_key_type::public_key_type( const fc::ecc::public_key_data& data ) : 
      key_data( data ) {};

   public_key_type::public_key_type( const fc::ecc::public_key& pubkey ) : 
      key_data( pubkey ) {};

   public_key_type::public_key_type( const std::string& base58str )
   {
      std::string prefix( ADDRESS_PREFIX );

      const size_t prefix_len = prefix.size();
      FC_ASSERT( base58str.size() > prefix_len,
         "Public Key string is too short." );
      FC_ASSERT( base58str.substr( 0, prefix_len ) ==  prefix ,
         "Public Key string does not have correct prefix", ("base58str", base58str) );
      auto bin = fc::from_base58( base58str.substr( prefix_len ) );
      auto bin_key = fc::raw::unpack<binary_key>(bin);
      key_data = bin_key.data;
      FC_ASSERT( fc::ripemd160::hash( key_data.data, key_data.size() )._hash[0] == bin_key.check );
   };


   public_key_type::operator fc::ecc::public_key_data() const
   {
      return key_data;
   };

   public_key_type::operator fc::ecc::public_key() const
   {
      return fc::ecc::public_key( key_data );
   };

   public_key_type::operator std::string() const
   {
      binary_key k;
      k.data = key_data;
      k.check = fc::ripemd160::hash( k.data.data, k.data.size() )._hash[0];
      auto data = fc::raw::pack( k );
      return ADDRESS_PREFIX + fc::to_base58( data.data(), data.size() );
   }

   bool operator == ( const public_key_type& p1, const fc::ecc::public_key& p2)
   {
      return p1.key_data == p2.serialize();
   }

   bool operator == ( const public_key_type& p1, const public_key_type& p2)
   {
      return p1.key_data == p2.key_data;
   }

   bool operator != ( const public_key_type& p1, const public_key_type& p2)
   {
      return p1.key_data != p2.key_data;
   }

   // extended_public_key_type

   extended_public_key_type::extended_public_key_type():key_data(){};

   extended_public_key_type::extended_public_key_type( const fc::ecc::extended_key_data& data )
      :key_data( data ){};

   extended_public_key_type::extended_public_key_type( const fc::ecc::extended_public_key& extpubkey )
   {
      key_data = extpubkey.serialize_extended();
   };

   extended_public_key_type::extended_public_key_type( const std::string& base58str )
   {
      std::string prefix( ADDRESS_PREFIX );

      const size_t prefix_len = prefix.size();
      FC_ASSERT( base58str.size() > prefix_len );
      FC_ASSERT( base58str.substr( 0, prefix_len ) ==  prefix , "", ("base58str", base58str) );
      auto bin = fc::from_base58( base58str.substr( prefix_len ) );
      auto bin_key = fc::raw::unpack<binary_key>(bin);
      FC_ASSERT( fc::ripemd160::hash( bin_key.data.data, bin_key.data.size() )._hash[0] == bin_key.check );
      key_data = bin_key.data;
   }

   extended_public_key_type::operator fc::ecc::extended_public_key() const
   {
      return fc::ecc::extended_public_key::deserialize( key_data );
   }

   extended_public_key_type::operator std::string() const
   {
      binary_key k;
      k.data = key_data;
      k.check = fc::ripemd160::hash( k.data.data, k.data.size() )._hash[0];
      auto data = fc::raw::pack( k );
      return ADDRESS_PREFIX + fc::to_base58( data.data(), data.size() );
   }

   bool operator == ( const extended_public_key_type& p1, const fc::ecc::extended_public_key& p2)
   {
      return p1.key_data == p2.serialize_extended();
   }

   bool operator == ( const extended_public_key_type& p1, const extended_public_key_type& p2)
   {
      return p1.key_data == p2.key_data;
   }

   bool operator != ( const extended_public_key_type& p1, const extended_public_key_type& p2)
   {
      return p1.key_data != p2.key_data;
   }

   // extended_private_key_type

   extended_private_key_type::extended_private_key_type():key_data(){};

   extended_private_key_type::extended_private_key_type( const fc::ecc::extended_key_data& data )
      :key_data( data ){};

   extended_private_key_type::extended_private_key_type( const fc::ecc::extended_private_key& extprivkey )
   {
      key_data = extprivkey.serialize_extended();
   };

   extended_private_key_type::extended_private_key_type( const std::string& base58str )
   {
      std::string prefix( ADDRESS_PREFIX );

      const size_t prefix_len = prefix.size();
      FC_ASSERT( base58str.size() > prefix_len );
      FC_ASSERT( base58str.substr( 0, prefix_len ) ==  prefix , "", ("base58str", base58str) );
      auto bin = fc::from_base58( base58str.substr( prefix_len ) );
      auto bin_key = fc::raw::unpack<binary_key>(bin);
      FC_ASSERT( fc::ripemd160::hash( bin_key.data.data, bin_key.data.size() )._hash[0] == bin_key.check );
      key_data = bin_key.data;
   }

   extended_private_key_type::operator fc::ecc::extended_private_key() const
   {
      return fc::ecc::extended_private_key::deserialize( key_data );
   }

   extended_private_key_type::operator std::string() const
   {
      binary_key k;
      k.data = key_data;
      k.check = fc::ripemd160::hash( k.data.data, k.data.size() )._hash[0];
      auto data = fc::raw::pack( k );
      return ADDRESS_PREFIX + fc::to_base58( data.data(), data.size() );
   }

   bool operator == ( const extended_private_key_type& p1, const fc::ecc::extended_public_key& p2)
   {
      return p1.key_data == p2.serialize_extended();
   }

   bool operator == ( const extended_private_key_type& p1, const extended_private_key_type& p2)
   {
      return p1.key_data == p2.key_data;
   }

   bool operator != ( const extended_private_key_type& p1, const extended_private_key_type& p2)
   {
      return p1.key_data != p2.key_data;
   }

   // Encrypted Keypair Type

   encrypted_keypair_type::encrypted_keypair_type() : 
      secure_key(), public_key(), encrypted_private_key() {};

   encrypted_keypair_type::encrypted_keypair_type( const public_key_type& s, const public_key_type& p, const std::string& e ) :
      secure_key( s ), public_key( p ), encrypted_private_key( e ) {};

   bool operator == ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
   {
      return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) == std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
   }

   bool operator < ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
   {
      return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) < std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
   }

   bool operator > ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
   {
      return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) > std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
   }

   bool operator != ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
   {
      return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) != std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
   }

   bool operator <= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
   {
      return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) <= std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
   }
   
   bool operator >= ( const encrypted_keypair_type& p1, const encrypted_keypair_type& p2 )
   {
      return std::tie( p1.secure_key, p1.public_key, p1.encrypted_private_key ) >= std::tie( p2.secure_key, p2.public_key, p2.encrypted_private_key );
   }

   // Date Type

   date_type::date_type() : 
      day(), month(), year() {};

   date_type::date_type( uint16_t d, uint16_t m, uint16_t y ) : 
      day(d), month(m), year(y) {};

   /**
    * time_point constructor of date_type
    */
   date_type::date_type( time_point time )
   {
      time_t unix_time = time.sec_since_epoch();
      struct tm * ptm;
      ptm = gmtime( &unix_time );

      day = ptm->tm_mday;
      month = ptm->tm_mon + 1;
      year = ptm->tm_year + 1900;
      validate();
   };

   bool operator == ( const date_type& date1, const date_type& date2 )
   {
      return std::tie( date1.year, date1.month, date1.day ) == std::tie( date2.year, date2.month, date2.day );
   }

   bool operator < ( const date_type& date1, const date_type& date2 )
   {
      return std::tie( date1.year, date1.month, date1.day ) < std::tie( date2.year, date2.month, date2.day );
   }

   bool operator > ( const date_type& date1, const date_type& date2 )
   {
      return std::tie( date1.year, date1.month, date1.day ) > std::tie( date2.year, date2.month, date2.day );
   }

   bool operator != ( const date_type& date1, const date_type& date2 )
   {
      return std::tie( date1.year, date1.month, date1.day ) != std::tie( date2.year, date2.month, date2.day );
   }

   bool operator <= ( const date_type& date1, const date_type& date2 )
   {
      return std::tie( date1.year, date1.month, date1.day ) <= std::tie( date2.year, date2.month, date2.day );
   }
   
   bool operator >= ( const date_type& date1, const date_type& date2 )
   {
      return std::tie( date1.year, date1.month, date1.day ) >= std::tie( date2.year, date2.month, date2.day );
   }

   string date_type::to_string()const
   {
      validate();
      string sday = fc::to_string( day );
      string smonth = fc::to_string( month );
      string syear = fc::to_string( year );

      return sday + "-" + smonth + "-" + syear;
   }

   bool date_type::is_null()const
   { 
      return *this == date_type();
   }

   date_type::operator fc::time_point()const
   {
      time_t unix_time = GENESIS_TIME.sec_since_epoch();
      struct tm * ptm;
      ptm = gmtime( &unix_time );

      ptm->tm_mday = day;
      ptm->tm_mon = month - 1;
      ptm->tm_year = year - 1900;

      time_t secs = mktime( ptm );
      validate();
      return fc::time_point( fc::seconds( secs ) );
   }

   /**
    * Ensures the date type matches a valid calander day after 1st Jan 1970.
    */
   void date_type::validate()const
   { try {
      uint16_t month_days;

      switch( month )
      {
         case 1:   // January
         {
            month_days = 31;
         }
         break;
         case 2:   // February
         {
            if( year % 4 == 0 && year % 100 != 0 )
            {
               month_days = 29;  // Leap Year
            }
            else
            {
               month_days = 28;
            }
         }
         break;
         case 3:   // March
         {
            month_days = 31;
         }
         break;
         case 4:  // April
         {
            month_days = 30;
         }
         break;
         case 5:  // May
         {
            month_days = 31;
         }
         break;
         case 6:  // June
         {
            month_days = 30;
         }
         break;
         case 7:  // July
         {
            month_days = 31;
         }
         break;
         case 8:  // August
         {
            month_days = 31;
         }
         break;
         case 9:  // September
         {
            month_days = 30;
         }
         break;
         case 10:  // October
         {
            month_days = 31;
         }
         break;
         case 11:  // November
         {
            month_days = 30;
         }
         break;
         case 12:  // December
         {
            month_days = 31;
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid Month value.");
         }
      }
      FC_ASSERT( day <= month_days, "Invalid Day value.");
      FC_ASSERT( year >= 1970, "Invalid Day value.");
      
   } FC_CAPTURE_AND_RETHROW( (day)(month)(year) ) }

   /**
    * Advance the date type to the next calander day.
    */
   void date_type::next()
   { try {
      uint16_t month_days;

      switch( month )
      {
         case 1:   // January
         {
            month_days = 31;
         }
         break;
         case 2:  // February
         {
            if( year % 4 == 0 && year % 100 != 0 )
            {
               month_days = 29;  // Leap Year
            }
            else
            {
               month_days = 28;
            }
         }
         break;
         case 3:  // March
         {
            month_days = 31;
         }
         break;
         case 4:  // April
         {
            month_days = 30;
         }
         break;
         case 5:  // May
         {
            month_days = 31;
         }
         break;
         case 6:  // June
         {
            month_days = 30;
         }
         break;
         case 7:  // July
         {
            month_days = 31;
         }
         break;
         case 8:  // August
         {
            month_days = 31;
         }
         break;
         case 9:  // September
         {
            month_days = 30;
         }
         break;
         case 10:  // October
         {
            month_days = 31;
         }
         break;
         case 11:  // November
         {
            month_days = 30;
         }
         break;
         case 12:  // December
         {
            month_days = 31;
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid Month value.");
         }
      }
      
      if( day < month_days )
      {
         day++;
      }
      else
      {
         day = 1;

         if( month < 12 )
         {
            month++;
         }
         else
         {
            month = 1;
            year++;
         }
      }

      FC_ASSERT( day <= month_days, "Invalid Day value.");
      FC_ASSERT( year >= 1970, "Invalid Day value.");

   } FC_CAPTURE_AND_RETHROW( (day)(month)(year) ) }


} } // node::protocol

namespace fc
{
   using namespace std;
   void to_variant( const node::protocol::public_key_type& var,  fc::variant& vo )
   {
      vo = std::string( var );
   }

   void from_variant( const fc::variant& var,  node::protocol::public_key_type& vo )
   {
      vo = node::protocol::public_key_type( var.as_string() );
   }

   void to_variant( const node::protocol::extended_public_key_type& var, fc::variant& vo )
   {
      vo = std::string( var );
   }

   void from_variant( const fc::variant& var, node::protocol::extended_public_key_type& vo )
   {
      vo = node::protocol::extended_public_key_type( var.as_string() );
   }

   void to_variant( const node::protocol::extended_private_key_type& var, fc::variant& vo )
   {
      vo = std::string( var );
   }

   void from_variant( const fc::variant& var, node::protocol::extended_private_key_type& vo )
   {
      vo = node::protocol::extended_private_key_type( var.as_string() );
   }
} // fc
