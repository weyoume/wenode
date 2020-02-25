#include <node/protocol/asset.hpp>
#include <boost/rational.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace node { namespace protocol {
   typedef boost::multiprecision::int128_t  int128_t;

   int64_t asset::precision_value()const
   {
      static int64_t table[] = {
         1, 10, 100, 1000, 10000,
         100000, 1000000, 10000000, 100000000ll,
         1000000000ll, 10000000000ll,
         100000000000ll, 1000000000000ll,
         10000000000000ll, 100000000000000ll
         };
      return table[ precision ];
   }

   /**
    * prec is a power of ten, so for example when working with
    * 7.005 we have fract = 5, prec = 1000.  So prec+fract=1005
    * has the correct number of zeros and we can simply trim the
    * leading 1.
    */
   string asset::to_string()const
   {
      int64_t decimal = precision_value();
      string result = fc::to_string(amount.value / decimal);
      if( decimal > 1 )
      {
         auto fract = amount.value % decimal;
         
         result += "." + fc::to_string(decimal + fract).erase(0,1);
      }
      return result + " " + symbol;
   }

   /**
    * Returns an asset derived from an input string.
    * String must have a space between decimal amount and symbol
    * and a dot point seperating the integer part from the
    * fractional part of the amount.
    */
   asset asset::from_string( string from )
   {
      try
      {
         string s = fc::trim( from );
         asset result;
         auto space_pos = s.find( " " );
         auto dot_pos = s.find( "." );

         FC_ASSERT( space_pos != std::string::npos );
         FC_ASSERT( dot_pos != std::string::npos );
         FC_ASSERT( space_pos > dot_pos );
         
         string symbol = s.substr( space_pos + 1 );
         is_valid_symbol( symbol );
         size_t symbol_size = symbol.size();
         FC_ASSERT( symbol_size <= MAX_ASSET_SYMBOL_LENGTH );
         result.symbol = symbol;

         return result.amount_from_string( s.substr( 0, space_pos - 1 ) );
      }
      FC_CAPTURE_AND_RETHROW( (from) )
   }

   asset asset::multiply_and_round_up( const price& b )const
   {
      const asset& a = *this;
      if( a.symbol == b.base.symbol )
      {
         FC_ASSERT( b.base.amount.value > 0 );
         uint128_t result = (uint128_t(a.amount.value) * b.quote.amount.value + b.base.amount.value - 1)/b.base.amount.value;
         FC_ASSERT( result <= share_type::max());
         return asset( result.to_uint64(), b.quote.symbol );
      }
      else if( a.symbol == b.quote.symbol )
      {
         FC_ASSERT( b.quote.amount.value > 0 );
         uint128_t result = (uint128_t(a.amount.value) * b.base.amount.value + b.quote.amount.value - 1)/b.quote.amount.value;
         FC_ASSERT( result <= share_type::max());
         return asset( result.to_uint64(), b.base.symbol );
      }
      FC_THROW_EXCEPTION( fc::assert_exception, "invalid asset::multiply_and_round_up(price)", ("asset",a)("price",b) );
   }

   asset asset::amount_from_string( string amount_string )const
   { try {
      bool negative_found = false;
      bool decimal_found = false;
      for( const char c : amount_string )
      {
         if( isdigit( c ) )
         {
            continue;
         }

         if( c == '-' && !negative_found )
         {
            negative_found = true;
            continue;
         }

         if( c == '.' && !decimal_found )
         {
            decimal_found = true;
            continue;
         }

         FC_THROW( (amount_string) );
      }

      share_type satoshis = 0;
      share_type scaled_precision = asset::precision_value();

      const auto decimal_pos = amount_string.find( '.' );
      const string lhs = amount_string.substr( negative_found, decimal_pos );

      if( !lhs.empty() )
      {
         satoshis += fc::safe<int64_t>(std::stoll(lhs)) *= scaled_precision;
      }

      if( decimal_found )
      {
         const size_t max_rhs_size = std::to_string( scaled_precision.value ).substr( 1 ).size();
         string rhs = amount_string.substr( decimal_pos + 1 );
         FC_ASSERT( rhs.size() <= max_rhs_size );
         while( rhs.size() < max_rhs_size )
            rhs += '0';

         if( !rhs.empty() )
            satoshis += std::stoll( rhs );
      }

      FC_ASSERT( satoshis <= share_type::max() );

      if( negative_found )
         satoshis *= -1;

      return asset(satoshis, symbol);
   } FC_CAPTURE_AND_RETHROW( (amount_string) ) }

   bool operator == ( const price& a, const price& b )
   {
      if( std::tie( a.base.symbol, a.quote.symbol ) != std::tie( b.base.symbol, b.quote.symbol ) )
            return false;

      const auto amult = share_type( b.quote.amount.value ) * a.base.amount.value;
      const auto bmult = share_type( a.quote.amount.value ) * b.base.amount.value;

      return amult == bmult;
   }

   bool operator < ( const price& a, const price& b )
   {
      if( a.base.symbol < b.base.symbol ) return true;
      if( a.base.symbol > b.base.symbol ) return false;
      if( a.quote.symbol < b.quote.symbol ) return true;
      if( a.quote.symbol > b.quote.symbol ) return false;

      const auto amult = share_type( b.quote.amount.value ) * a.base.amount.value;
      const auto bmult = share_type( a.quote.amount.value ) * b.base.amount.value;

      return amult < bmult;
   }

   bool operator <= ( const price& a, const price& b )
   {
      return (a == b) || (a < b);
   }

   bool operator != ( const price& a, const price& b )
   {
      return !(a == b);
   }

   bool operator > ( const price& a, const price& b )
   {
      return !(a <= b);
   }

   bool operator >= ( const price& a, const price& b )
   {
      return !(a < b);
   }

   asset operator * ( const asset& a, const price& b )
   {
      if( a.symbol == b.base.symbol )
      {
         FC_ASSERT( b.base.amount.value > 0 );
         uint128_t result = (uint128_t(a.amount.value) * b.quote.amount.value)/b.base.amount.value;
         FC_ASSERT( result.hi == 0 );
         return asset( result, b.quote.symbol );
      }
      else if( a.symbol == b.quote.symbol )
      {
         FC_ASSERT( b.quote.amount.value > 0 );
         uint128_t result = (uint128_t(a.amount.value) * b.base.amount.value)/b.quote.amount.value;
         FC_ASSERT( result.hi == 0 );
         return asset( result, b.base.symbol );
      }
      FC_THROW_EXCEPTION( fc::assert_exception, "invalid asset * price", ("asset",a)("price",b) );
   }

   price operator / ( const asset& base, const asset& quote )
   { try {
      FC_ASSERT( base.symbol != quote.symbol );
      return price{ base, quote };
   } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }

   price price::max( asset_symbol_type base, asset_symbol_type quote ) { return asset( share_type::max(), base ) / asset( 1, quote); }
   price price::min( asset_symbol_type base, asset_symbol_type quote ) { return asset( 1, base ) / asset( share_type::max(), quote); }

   bool price::is_null() const { return *this == price(); }

   void price::validate() const
   { try {
      FC_ASSERT( base.amount.value > 0,
         "Price is invalid, base is less than 0.");
      FC_ASSERT( quote.amount.value > 0,
         "Price is invalid, quote is less than 0.");
      FC_ASSERT( base.symbol != quote.symbol,
         "Price is invalid, symbols are for the same asset.");
   } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }

   price operator / ( const asset& base, const asset& quote )
   { try {
      FC_ASSERT( base.symbol != quote.symbol );
      return price{base,quote};
   } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }

   price operator + ( const price& a, const price& b )
   { try {
      FC_ASSERT( a.base.symbol == b.base.symbol, "Cannot add prices of different asset pairs");
      FC_ASSERT( a.quote.symbol == b.quote.symbol, "Cannot add prices of different asset pairs");
      a.validate();
      b.validate();

      if( a.base.amount.value == 0 )
      {
         return b;
      }
      if( b.base.amount.value == 0 )
      {
         return a;
      }

      boost::rational<int128_t> a_rational( a.base.amount.value, a.quote.amount.value );
      boost::rational<int128_t> b_rational( b.base.amount.value, b.quote.amount.value );
      boost::rational<int128_t> result = a_rational + b_rational;

      price p = asset( result.numerator().convert_to<int64_t>(), a.base.symbol ) / asset( result.denominator().convert_to<int64_t>(), a.quote.symbol );

      return p;
   } FC_CAPTURE_AND_RETHROW( (a)(b) ) }

   price operator - ( const price& a, const price& b )
   { try {
      FC_ASSERT( a.base.symbol == b.base.symbol, "Cannot subtract prices of different asset pairs");
      FC_ASSERT( a.quote.symbol == b.quote.symbol, "Cannot substract prices of different asset pairs");
      a.validate();
      b.validate();

      if(a == b )
      {
         return asset( 0, a.base.symbol ) / asset( 0, a.quote.symbol );   // Returns null price if prices are equal
      }
      if( a.base.amount.value == 0 )
      {
         return b;
      }
      if( b.base.amount.value == 0 )
      {
         return a;
      }

      boost::rational<int128_t> a_rational( a.base.amount.value, a.quote.amount.value );
      boost::rational<int128_t> b_rational( b.base.amount.value, b.quote.amount.value );
      boost::rational<int128_t> result = a_rational - b_rational;

      return asset( result.numerator().convert_to<int64_t>(), a.base.symbol ) / asset( result.denominator().convert_to<int64_t>(), a.quote.symbol );
   } FC_CAPTURE_AND_RETHROW( (a)(b) ) }

   price operator *  ( const price& p, const ratio_type& r )
   { try {
      p.validate();

      FC_ASSERT( r.numerator() > 0 && r.denominator() > 0 );

      if( r.numerator() == r.denominator() ) return p;

      boost::rational<int128_t> p128( p.base.amount.value, p.quote.amount.value );
      boost::rational<int128_t> r128( r.numerator(), r.denominator() );
      auto cp = p128 * r128;
      auto ocp = cp;

      bool shrinked = false;
      bool using_max = false;
      int128_t max( share_type::max().value );
      while( cp.numerator() > max || cp.denominator() > max )
      {
         if( cp.numerator() == 1 )
         {
            cp = boost::rational<int128_t>( 1, max );
            using_max = true;
            break;
         }
         else if( cp.denominator() == 1 )
         {
            cp = boost::rational<int128_t>( max, 1 );
            using_max = true;
            break;
         }
         else
         {
            cp = boost::rational<int128_t>( cp.numerator() >> 1, cp.denominator() >> 1 );
            shrinked = true;
         }
      }
      if( shrinked ) // maybe not accurate enough due to rounding, do additional checks here
      {
         int128_t num = ocp.numerator();
         int128_t den = ocp.denominator();
         if( num > den )
         {
            num /= den;
            if( num > max )
               num = max;
            den = 1;
         }
         else
         {
            den /= num;
            if( den > max )
               den = max;
            num = 1;
         }
         boost::rational<int128_t> ncp( num, den );
         if( num == max || den == max ) // it's on the edge, we know it's accurate enough
            cp = ncp;
         else
         {
            // from the accurate ocp, now we have ncp and cp. use the one which is closer to ocp.
            auto diff1 = abs( ncp - ocp );
            auto diff2 = abs( cp - ocp );
            if( diff1 < diff2 )
            {
               cp = ncp;
            }
         }
      }

      price np = asset( cp.numerator().convert_to<int64_t>(), p.base.symbol )
               / asset( cp.denominator().convert_to<int64_t>(), p.quote.symbol );

      if( shrinked || using_max )
      {
         if( ( r.numerator() > r.denominator() && np < p )
               || ( r.numerator() < r.denominator() && np > p ) )
            // even with an accurate result, if p is out of valid range, return it
            np = p;
      }

      np.validate();
      return np;
   } FC_CAPTURE_AND_RETHROW( (p)(r.numerator())(r.denominator()) ) }

   price operator /  ( const price& p, const ratio_type& r )
   { try {
      return p * ratio_type( r.denominator(), r.numerator() );
   } FC_CAPTURE_AND_RETHROW( (p)(r.numerator())(r.denominator()) ) }

   void price_feed::validate() const
   { try {
      if( !settlement_price.is_null() )
         settlement_price.validate();
      FC_ASSERT( maximum_short_squeeze_ratio >= MIN_COLLATERAL_RATIO );
      FC_ASSERT( maximum_short_squeeze_ratio <= MAX_COLLATERAL_RATIO );
      FC_ASSERT( maintenance_collateral_ratio >= MIN_COLLATERAL_RATIO );
      FC_ASSERT( maintenance_collateral_ratio <= MAX_COLLATERAL_RATIO );
      
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   bool price_feed::is_for( asset_symbol_type symbol ) const
   {
      try
      {
         if( !settlement_price.is_null() )
         {
            return (settlement_price.base.symbol == symbol);
         }
         else
         {
            return true; // (null, null) is valid for any feed
         }
      }
      FC_CAPTURE_AND_RETHROW( (*this) )
   }

   // settlement price is in debt/collateral
   price price_feed::max_short_squeeze_price()const
   {
      return settlement_price * ratio_type( COLLATERAL_RATIO_DENOM, maximum_short_squeeze_ratio );
   }

   price price_feed::maintenance_collateralization()const
   {
      if( settlement_price.is_null() )
      {
         return price();
      }  
      return ~settlement_price * ratio_type( maintenance_collateral_ratio, COLLATERAL_RATIO_DENOM );
   }

   // Option Strike

   string option_strike::to_string()const
   {
      return strike_price.quote.symbol + "-" + fc::to_string( strike_price.to_real() ) + "-" + strike_price.base.symbol + "-" + expiration_date.to_string();
   }

   bool operator == ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.strike_price, a.expiration_date ) == std::tie( b.strike_price, b.expiration_date );
   }

   bool operator < ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.strike_price, a.expiration_date ) < std::tie( b.strike_price, b.expiration_date );
   }

   bool operator <= ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.strike_price, a.expiration_date ) <= std::tie( b.strike_price, b.expiration_date );
   }

   bool operator != ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.strike_price, a.expiration_date ) != std::tie( b.strike_price, b.expiration_date );
   }

   bool operator > ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.strike_price, a.expiration_date ) > std::tie( b.strike_price, b.expiration_date );
   }

   bool operator >= ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.strike_price, a.expiration_date ) >= std::tie( b.strike_price, b.expiration_date );
   }

} } // node::protocol