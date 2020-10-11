#pragma once

#include <node/protocol/node_operations.hpp>

#include <fc/utf8.hpp>
#include <fc/crypto/equihash.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/io/json.hpp>
#include <fc/variant.hpp>
#include <regex> 

namespace node { namespace protocol {

   inline void validate_account_name( const string& name )
   {
      FC_ASSERT( is_valid_account_name( name ),
         "Account name of length: ${l} is invalid: ${n} ", 
         ("l", name.size() )("n", fc::json::to_pretty_string( name ) ) );
   };

   inline void validate_community_name( const string& name )
   {
      FC_ASSERT( is_valid_account_name( name ),
         "Community name of length: ${l} is invalid: ${n} ", 
         ("l", name.size() )("n", fc::json::to_pretty_string( name ) ) );
   };

   inline void validate_tag_name( const string& name )
   {
      FC_ASSERT( is_valid_account_name( name ),
         "Tag name of length: ${l} is invalid: ${n} ", 
         ("l", name.size() )("n", fc::json::to_pretty_string( name ) ) );
   };

   inline void validate_permlink( const string& permlink )
   {
      FC_ASSERT( permlink.size() >= MIN_PERMLINK_LENGTH && 
         permlink.size() <= MAX_PERMLINK_LENGTH, 
         "Permlink is not a valid size." );

      for( auto c : permlink )
      {
         switch( c )
         {
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
            case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
            case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': case '0':
            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            case '-':
               break;
            default:
               FC_ASSERT( false, "Invalid permlink character: ${s}", ("s", std::string() + c ) );
         }
      }
   }

   inline void validate_url( const string& url )
   {
      std::regex url_regex( R"((http(s)?:\/\/.)?(www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*))" );
      
      FC_ASSERT( std::regex_match( url, url_regex ),
         "URL is invalid: ${u}", ("u", url ) );
   };

   inline void validate_uuidv4( const string& uuidv4 )
   {
      std::string pattern = "[a-f0-9]{8}-?[a-f0-9]{4}-?4[a-f0-9]{3}-?[89ab][a-f0-9]{3}-?[a-f0-9]{12}";
      std::regex uuidv4_regex( pattern );

      FC_ASSERT( std::regex_match( uuidv4, uuidv4_regex ),
         "UUIDV4 is invalid: ${i}", ("i", uuidv4 ) );
   };

   inline void validate_public_key( const string& key )
   {
      FC_ASSERT( key.size() < MAX_URL_SIZE,
         "Public Key is too long" );
      FC_ASSERT( fc::is_utf8( key ),
         "Public key is not formatted in UTF8." );
      public_key_type pkt = public_key_type( key );
      string result = string( pkt );
      FC_ASSERT( key == result, "Invalid Public Key" );
   };

   /**
    * Retrieves the edit distance between two strings, 
    * the number of additions, substitutions or deletions needed
    * to transfrom string a into string b.
    */
   inline size_t edit_distance( const string& s1, const string& s2 )
   {
      const size_t m(s1.size());
      const size_t n(s2.size());
      
      if( m==0 ) return n;
      if( n==0 ) return m;
      
      vector<size_t> costs;
      costs.reserve( n+1 );
      
      for( size_t k=0; k<=n; k++ )
      {
         costs[k] = k;
      }
      
      size_t i = 0;
      for( std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i )
      {
         costs[0] = i+1;
         size_t corner = i;
      
         size_t j = 0;
         for( std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j )
         {
            size_t upper = costs[j+1];
            if( *it1 == *it2 )
            {
               costs[j+1] = corner;
            }
            else
            {
               size_t t(upper<corner?upper:corner);
               costs[j+1] = (costs[j]<t?costs[j]:t)+1;
            }
      
            corner = upper;
         }
      }
      
      size_t result = costs[n];
      return result;
   };

   /**
    * Returns the Private key generated from a specified seed string.
    */
   inline fc::ecc::private_key generate_private_key( string seed )
   {
      return fc::ecc::private_key::regenerate( fc::sha256::hash( seed ) );
   };

   /**
    * Rounds a specifed value N to the nearest n significant figures.
    */
   inline double round_sig_figs( double N, double n ) 
   { 
      int h; 
      double b, d, e, i, j, m, f; 
      b = N; 

      for (i = 0; b >= 1; ++i)
      {
         b = b / 10; 
      }
         
      d = n - i; 
      b = N; 
      b = b * std::pow(10, d); 
      e = b + 0.5; 
      if( (float)e == (float)std::ceil(b) )
      { 
         f = (std::ceil(b)); 
         h = f - 2; 
         if (h % 2 != 0) 
         { 
            e = e - 1; 
         } 
      }
      j = std::floor(e); 
      m = std::pow(10, d); 
      j = j / m;
      return j; 
   }

   inline uint8_t find_msb( const uint128_t& u )
   {
      uint64_t x;
      uint8_t places;
      x      = (u.lo ? u.lo : 1);
      places = (u.hi ?   64 : 0);
      x      = (u.hi ? u.hi : x);
      return uint8_t( boost::multiprecision::detail::find_msb(x) + places );
   }

   inline uint64_t approx_sqrt( const uint128_t& x )
   {
      if( (x.lo == 0) && (x.hi == 0) )
         return 0;

      uint8_t msb_x = find_msb(x);
      uint8_t msb_z = msb_x >> 1;

      uint128_t msb_x_bit = uint128_t(1) << msb_x;
      uint64_t  msb_z_bit = uint64_t (1) << msb_z;

      uint128_t mantissa_mask = msb_x_bit - 1;
      uint128_t mantissa_x = x & mantissa_mask;
      uint64_t mantissa_z_hi = (msb_x & 1) ? msb_z_bit : 0;
      uint64_t mantissa_z_lo = (mantissa_x >> (msb_x - msb_z)).lo;
      uint64_t mantissa_z = (mantissa_z_hi | mantissa_z_lo) >> 1;
      uint64_t result = msb_z_bit | mantissa_z;

      return result;
   }

   /**
    * Generates a public key from a specified account name, authority type 
    * and password using the graphene account authority standard.
    */
   inline public_key_type get_public_key( string name, string type, string password )
   {
      return fc::ecc::private_key::regenerate( fc::sha256::hash( std::string( name + type + password ) ) ).get_public_key();
   }

   /**
    * Generates a private key from a specified account name, authority type 
    * and password using the graphene account authority standard.
    */
   inline private_key_type get_private_key( string name, string type, string password )
   {
      return fc::ecc::private_key::regenerate( fc::sha256::hash( std::string( name + type + password ) ) );
   }

   inline string key_to_wif( const fc::sha256& secret )
   {
      const size_t size_of_data_to_hash = sizeof(secret) + 1;
      const size_t size_of_hash_bytes = 4;
      char data[size_of_data_to_hash + size_of_hash_bytes];
      data[0] = (char)0x80;
      memcpy(&data[1], (char*)&secret, sizeof(secret));
      fc::sha256 digest = fc::sha256::hash(data, size_of_data_to_hash);
      digest = fc::sha256::hash(digest);
      memcpy(data + size_of_data_to_hash, (char*)&digest, size_of_hash_bytes);
      return fc::to_base58(data, sizeof(data));
   }

   inline string key_to_wif( const fc::ecc::private_key& key )
   {
      return key_to_wif( key.get_secret() );
   }

   inline fc::optional< fc::ecc::private_key > wif_to_key( const string& wif_key )
   {
      vector<char> wif_bytes;
      try
      {
         wif_bytes = fc::from_base58(wif_key);
      }
      catch (const fc::parse_error_exception&)
      {
         return fc::optional<fc::ecc::private_key>();
      }
      if( wif_bytes.size() < 5 )
      {
         return fc::optional<fc::ecc::private_key>();
      }
         
      vector<char> key_bytes(wif_bytes.begin() + 1, wif_bytes.end() - 4);
      fc::ecc::private_key key = fc::variant(key_bytes).as<fc::ecc::private_key>();
      fc::sha256 check = fc::sha256::hash(wif_bytes.data(), wif_bytes.size() - 4);
      fc::sha256 check2 = fc::sha256::hash(check);
         
      if( memcmp( (char*)&check, wif_bytes.data() + wif_bytes.size() - 4, 4 ) == 0 || 
            memcmp( (char*)&check2, wif_bytes.data() + wif_bytes.size() - 4, 4 ) == 0 )
         return key;

      return fc::optional<fc::ecc::private_key>();
   }

   
} } ///< node::protocol