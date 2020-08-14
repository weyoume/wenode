#include <node/protocol/asset.hpp>
#include <boost/rational.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace node { namespace protocol {

   

   /**
    * prec is a power of ten, so for example when working with 7.005 we have fract = 5, prec = 1000. 
    * 
    * So prec+fract=1005 has the correct number of zeros and we can simply trim the leading 1.
    */
   string asset::to_string()const
   {
      int64_t decimal = precision_value();
      string result = fc::to_string( amount.value / decimal );

      if( result.size() > 3 )
      {
         size_t i = result.size();
         while( i > 3 )
         {
            i -= 3;
            result.insert( i, ",");
         }
      }

      if( decimal > 1 )
      {
         auto fract = amount.value % decimal;
         
         result += "." + fc::to_string( decimal + fract ).erase(0,1);
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
         FC_ASSERT( is_valid_symbol( symbol ) );
         size_t symbol_size = symbol.size();
         FC_ASSERT( symbol_size <= MAX_ASSET_SYMBOL_LENGTH );
         result.symbol = symbol;

         return result.amount_from_string( s.substr( 0, space_pos ) );
      }
      FC_CAPTURE_AND_RETHROW( (from) )
   }

   asset asset::multiply_and_round_up( const price& b )const
   {
      const asset& a = *this;
      if( a.symbol == b.base.symbol )
      {
         FC_ASSERT( b.base.amount.value > 0 );
         uint128_t result = ( uint128_t( a.amount.value ) * uint128_t( b.quote.amount.value ) + uint128_t( b.base.amount.value ) - 1 ) / uint128_t( b.base.amount.value );
         FC_ASSERT( result <= share_type::max().value );
         asset r = asset( share_type( result.to_uint64() ), b.quote.symbol );
         FC_ASSERT( is_valid_symbol( b.quote.symbol ) );
         ilog( "Multiply and Round up: Asset: ${a} Price: ${p} Result: ${r}",
            ("a",a.to_string())("p",b.to_string())("r",r.to_string()) );
         return r;
      }
      else if( a.symbol == b.quote.symbol )
      {
         FC_ASSERT( b.quote.amount.value > 0 );
         uint128_t result = ( uint128_t( a.amount.value ) * uint128_t( b.base.amount.value ) + uint128_t( b.quote.amount.value ) - 1 ) / uint128_t( b.quote.amount.value );
         FC_ASSERT( result <= share_type::max().value );
         asset r = asset( share_type( result.to_uint64() ), b.base.symbol );
         FC_ASSERT( is_valid_symbol( b.base.symbol ) );
         ilog( "Multiply and Round up: Asset: ${a} Price: ${p} Result: ${r}",
            ("a",a.to_string())("p",b.to_string())("r",r.to_string()) );
         return r;
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
      ilog( "LHS: ${l}",
         ("l",lhs) );
      if( !lhs.empty() )
      {
         satoshis += fc::safe<int64_t>(std::stoll(lhs));
         satoshis *= scaled_precision;
      }

      if( decimal_found )
      {
         const size_t max_rhs_size = size_t( precision );
         string rhs = amount_string.substr( decimal_pos + 1 );
         ilog( "RHS: ${r}",
            ("r",rhs) );
         FC_ASSERT( rhs.size() <= max_rhs_size );

         while( rhs.size() < max_rhs_size )
         {
            rhs += '0';
         }
         
         if( !rhs.empty() )
         {
            satoshis += std::stoll( rhs );
         }
      }

      FC_ASSERT( satoshis <= share_type::max() );

      if( negative_found )
      {
         satoshis *= -1;
      }

      FC_ASSERT( is_valid_symbol( symbol ) );

      ilog( "Amount from string: ${s} -> ${a}",
         ("s",amount_string)("a",asset(satoshis, symbol)));

      return asset(satoshis, symbol);
   } FC_CAPTURE_AND_RETHROW( (amount_string) ) }

   bool operator == ( const price& a, const price& b )
   {
      FC_ASSERT( is_valid_symbol( a.base.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",a.base.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( b.base.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",b.base.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( a.quote.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",a.quote.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( b.quote.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",b.quote.symbol)("a",a)("b",b));

      if( std::tie( a.base.symbol, a.quote.symbol ) != std::tie( b.base.symbol, b.quote.symbol ) )
      {
         return false;
      }   

      uint128_t amult = uint128_t( b.quote.amount.value ) * uint128_t( a.base.amount.value );
      uint128_t bmult = uint128_t( a.quote.amount.value ) * uint128_t( b.base.amount.value );

      return amult == bmult;
   }

   bool operator < ( const price& a, const price& b )
   {
      FC_ASSERT( is_valid_symbol( a.base.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",a.base.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( b.base.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",b.base.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( a.quote.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",a.quote.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( b.quote.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",b.quote.symbol)("a",a)("b",b));

      if( a.base.symbol < b.base.symbol ) return true;
      if( a.base.symbol > b.base.symbol ) return false;
      if( a.quote.symbol < b.quote.symbol ) return true;
      if( a.quote.symbol > b.quote.symbol ) return false;

      uint128_t amult = uint128_t( b.quote.amount.value ) * uint128_t( a.base.amount.value );
      uint128_t bmult = uint128_t( a.quote.amount.value ) * uint128_t( b.base.amount.value );

      return amult < bmult;
   }

   bool operator <= ( const price& a, const price& b )
   {
      return !(a > b);
   }

   bool operator != ( const price& a, const price& b )
   {
      return !(a == b);
   }

   bool operator > ( const price& a, const price& b )
   {
      FC_ASSERT( is_valid_symbol( a.base.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",a.base.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( b.base.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",b.base.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( a.quote.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",a.quote.symbol)("a",a)("b",b));
      FC_ASSERT( is_valid_symbol( b.quote.symbol ), "Symbol: ${s} of price pair: \n ${a} \n ${b} \n is not a valid symbol",
         ("s",b.quote.symbol)("a",a)("b",b));

      if( a.base.symbol > b.base.symbol ) return true;
      if( a.base.symbol < b.base.symbol ) return false;
      if( a.quote.symbol > b.quote.symbol ) return true;
      if( a.quote.symbol < b.quote.symbol ) return false;

      uint128_t amult = uint128_t( b.quote.amount.value ) * uint128_t( a.base.amount.value );
      uint128_t bmult = uint128_t( a.quote.amount.value ) * uint128_t( b.base.amount.value );

      return amult > bmult;
   }

   bool operator >= ( const price& a, const price& b )
   {
      return !(a < b);
   }

   asset operator * ( const asset& a, const price& b )
   {
      FC_ASSERT( is_valid_symbol( a.symbol ), "Symbol: ${s} is not a valid symbol",("s",a.symbol) );
      FC_ASSERT( is_valid_symbol( b.base.symbol ), "Symbol: ${s} is not a valid symbol",("s",b.base.symbol) );
      FC_ASSERT( is_valid_symbol( b.quote.symbol ), "Symbol: ${s} is not a valid symbol",("s",b.quote.symbol));
      
      uint128_t numerator = 0;
      uint128_t denominator = 0;
      uint128_t result = 0;

      if( a.symbol == b.base.symbol )
      {
         FC_ASSERT( b.base.amount.value > 0 );
         numerator = uint128_t( a.amount.value ) * uint128_t( b.quote.amount.value );
         denominator = uint128_t( b.base.amount.value );
         result = numerator / denominator;
         if( result.hi == 0 )
         {
            share_type r = result.to_uint64();
            return asset( r, b.quote.symbol );
         }
      }
      else if( a.symbol == b.quote.symbol )
      {
         FC_ASSERT( b.quote.amount.value > 0 );
         numerator = uint128_t( a.amount.value ) * uint128_t( b.base.amount.value );
         denominator = uint128_t( b.quote.amount.value );
         result = numerator / denominator;
         if( result.hi == 0 )
         {
            share_type r = result.to_uint64();
            return asset( r, b.base.symbol );
         }
      }

      FC_THROW_EXCEPTION( fc::assert_exception,
         "Invalid asset multiplication: ${a} * ${p} - Numerator: ${n} Denominator: ${d} Result: ${r}", 
         ("a",a)("p",b)("n",numerator)("d",denominator)("r",result) );
   }

   price operator / ( const asset& base, const asset& quote )
   { try {
      FC_ASSERT( is_valid_symbol( base.symbol ), "Symbol: ${s} is not a valid symbol",("s",base.symbol));
      FC_ASSERT( is_valid_symbol( quote.symbol ), "Symbol: ${s} is not a valid symbol",("s",quote.symbol));
      FC_ASSERT( base.symbol != quote.symbol );
      return price{ base, quote };
   } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }

   price price::max( asset_symbol_type base, asset_symbol_type quote ) { return asset( share_type::max(), base ) / asset( 1, quote); }
   price price::min( asset_symbol_type base, asset_symbol_type quote ) { return asset( 1, base ) / asset( share_type::max(), quote); }

   bool price::is_null() const { return *this == price(); }

   void price::validate() const
   { try {
      FC_ASSERT( is_valid_symbol( base.symbol ), "Symbol: ${s} is not a valid symbol",("s",base.symbol));
      FC_ASSERT( is_valid_symbol( quote.symbol ), "Symbol: ${s} is not a valid symbol",("s",quote.symbol));
      FC_ASSERT( base.amount.value > 0,
         "Price is invalid, base is less than 0.");
      FC_ASSERT( quote.amount.value > 0,
         "Price is invalid, quote is less than 0.");
      FC_ASSERT( base.symbol != quote.symbol,
         "Price is invalid, symbols are for the same asset.");
   } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }

   price operator + ( const price& a, const price& b )
   { try {
      FC_ASSERT( a.base.symbol == b.base.symbol,
         "Cannot add prices of different asset pairs");
      FC_ASSERT( a.quote.symbol == b.quote.symbol,
         "Cannot add prices of different asset pairs");
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
      if( shrinked )       // maybe not accurate enough due to rounding, do additional checks here
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
         if( num == max || den == max )         // it's on the edge, we know it's accurate enough
         {
            cp = ncp;
         }  
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

   void option_strike::validate()const
   {
      strike_price.validate();
      expiration_date.validate();
      FC_ASSERT( multiple.value > 0,
         "Option Strike multiple cannot be negative." );
      FC_ASSERT( strike_price.base.amount.value == BLOCKCHAIN_PRECISION.value || strike_price.quote.amount.value == BLOCKCHAIN_PRECISION.value ,
         "Option Strike price must specify an asset with a price unit of 1." );
   }

   string option_strike::to_string()const
   {
      string result;
      if( call )
      {
         result += OPTION_ASSET_CALL;
      }
      else
      {
         result += OPTION_ASSET_PUT;
      }
      result += ".";
      result += strike_price.quote.symbol;
      result += ".";

      int64_t decimal = strike_price.quote.precision_value();
      int64_t round = decimal / 100;
      
      string amount = fc::to_string( strike_price.base.amount.value / decimal );
      if( decimal > 1 )
      {
         int64_t fract = strike_price.base.amount.value % decimal;
         int64_t units = ( decimal + fract ) / round;
         amount += "." + fc::to_string( units ).erase(0,1);
      }

      result += amount;
      result += ".";
      result += strike_price.base.symbol;
      result += ".";
      result += fc::to_string( expiration_date.month );
      result += ".";
      result += fc::to_string( expiration_date.year % 100 );

      return result;
   }

   /**
    * More readable display symbol of an option strike.
    * Space seperated and uses full wording.
    * 
    * QUOTE_SYMBOL STRIKE_PRICE BASE_SYMBOL TYPE OPTION - EXPIRATION: ( EXP_DAY / EXP_MONTH / EXP_YEAR )
    * WYM 3.50000000 MUSD CALL OPTION - EXPIRATION: ( 1 / 1 / 2021 )
    */
   string option_strike::display_symbol()const
   {
      string result;

      result += strike_price.quote.symbol;
      result += " ";
      result += fc::to_string( strike_price.to_real() );
      result += " ";
      result += strike_price.base.symbol;
      result += " ";
      if( call )
      {
         result += "CALL";
      }
      else
      {
         result += "PUT";
      }
      result += " OPTION - EXPIRATION: ( ";
      result += fc::to_string( expiration_date.day );
      result += " / ";
      result += fc::to_string( expiration_date.month );
      result += " / ";
      result += fc::to_string( expiration_date.year );
      result += " )";

      return result;
   }


   /**
    * Generates extended description of an option asset and its underlying assets.
    */
   string option_strike::details( string quote_display, string quote_details, string base_display, string base_details )const
   {
      string result;

      result += quote_display;
      result += " ";
      result += fc::to_string( strike_price.to_real() );
      result += " ";
      result += base_display;
      result += " ";
      if( call )
      {
         result += "CALL";
      }
      else
      {
         result += "PUT";
      }
      result += " OPTION - EXPIRATION: ( ";
      result += fc::to_string( expiration_date.day );
      result += " / ";
      result += fc::to_string( expiration_date.month );
      result += " / ";
      result += fc::to_string( expiration_date.year );
      result += " )";
      result += " Option Assets provides the right, but not the obligation to execute a trade to buy or sell an asset at the strike price on or before the expiration date. ";
      result += " Quote Asset: ";
      result += quote_details;
      result += " Base Asset: ";
      result += base_details;

      return result;
   }

   /**
    * Generates the Option Strike price characteristics of a
    * specified string / asset symbol of the form:
    * TYPE.QUOTE_SYMBOL.STRIKE_PRICE.BASE_SYMBOL.EXP_MONTH.EXP_YEAR
    * CALL.WYM.3.50.MEUSD.1.21
    */
   option_strike option_strike::from_string( const string& from )
   {
      try
      {
         option_strike result;

         std::string str = fc::trim( from );
         std::string delimiter = ".";
         vector< string > tokens;
         size_t prev = 0, pos = 0;
         do
         {
            pos = str.find( delimiter, prev );
            if( pos == string::npos )
            {
               pos = str.length();
            }
            string token = str.substr( prev, pos-prev );
            if( !token.empty() )
            {
               tokens.push_back( token );
            }
            prev = pos + delimiter.length();
         }
         while( pos < str.length() && prev < str.length() );

         if( tokens[ 0 ] == OPTION_ASSET_CALL )
         {
            result.call = true;
         }
         else if( tokens[ 0 ] == OPTION_ASSET_PUT )
         {
            result.call = false;
         }
         else
         {
            FC_ASSERT( false,
               "Option Asset Symbol must specify either CALL or PUT option contract type." );
         }

         asset_symbol_type quote = tokens[ 1 ];

         share_type units = 0;
         share_type scaled_precision = 100;
         share_type asset_precision = BLOCKCHAIN_PRECISION / scaled_precision;

         string lhs = tokens[ 2 ];

         if( !lhs.empty() )
         {
            units += fc::safe< int64_t >( std::stoll( lhs ) ) *= scaled_precision;
         }

         const size_t max_rhs_size = std::to_string( scaled_precision.value ).substr( 1 ).size();
         string rhs = tokens[ 3 ];

         FC_ASSERT( rhs.size() <= max_rhs_size );
         while( rhs.size() < max_rhs_size )
         {
            rhs += '0';
         }
         
         if( !rhs.empty() )
         {
            units += std::stoll( rhs );
         }

         FC_ASSERT( units <= share_type::max(),
            "Asset value is too large." );

         asset_symbol_type base = tokens[ 4 ];

         result.strike_price = price( asset( units * asset_precision, base ), asset( BLOCKCHAIN_PRECISION, quote ) );
         
         const string month = tokens[ 5 ];
         const string year = tokens[ 6 ];

         uint16_t month_n = std::stoll( month );
         uint16_t year_n = std::stoll( year );

         result.expiration_date = date_type( 1, month_n, 2000 + year_n );

         result.validate();
         return result;
      }
      FC_CAPTURE_AND_RETHROW( (from) )
   }


   /**
    * TYPE.QUOTE_SYMBOL.STRIKE_PRICE.BASE_SYMBOL.EXP_MONTH.EXP_YEAR
    */
   asset_symbol_type option_strike::option_symbol()const
   {
      string result;
      if( call )
      {
         result += OPTION_ASSET_CALL;
      }
      else
      {
         result += OPTION_ASSET_PUT;
      }
      result += ".";
      result += strike_price.quote.symbol;
      result += ".";

      int64_t decimal = strike_price.quote.precision_value();
      string amount = fc::to_string( strike_price.base.amount.value / decimal );
      if( decimal > 1 )
      {
         int64_t fract = strike_price.base.amount.value % decimal;
         int64_t round = decimal / 100;
         int64_t units = ( decimal + fract ) / round;
         amount += "." + fc::to_string( units ).erase(0,1);
      }
      result += amount;
      result += ".";
      result += strike_price.base.symbol;
      result += ".";
      result += fc::to_string( expiration_date.month );
      result += ".";
      result += fc::to_string( expiration_date.year % 100 );

      return asset_symbol_type( result );
   }


   time_point option_strike::expiration()const
   {
      return fc::time_point( expiration_date );
   }


   bool operator == ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.expiration_date, a.call, a.strike_price, a.multiple ) == std::tie( b.expiration_date, b.call, b.strike_price, b.multiple );
   }

   bool operator < ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.expiration_date, a.call, a.strike_price, a.multiple ) < std::tie( b.expiration_date, b.call, b.strike_price, b.multiple );
   }

   bool operator <= ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.expiration_date, a.call, a.strike_price, a.multiple ) <= std::tie( b.expiration_date, b.call, b.strike_price, b.multiple );
   }

   bool operator != ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.expiration_date, a.call, a.strike_price, a.multiple ) != std::tie( b.expiration_date, b.call, b.strike_price, b.multiple );
   }

   bool operator > ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.expiration_date, a.call, a.strike_price, a.multiple ) > std::tie( b.expiration_date, b.call, b.strike_price, b.multiple );
   }

   bool operator >= ( const option_strike& a, const option_strike& b )
   {
      return std::tie( a.expiration_date, a.call, a.strike_price, a.multiple ) >= std::tie( b.expiration_date, b.call, b.strike_price, b.multiple );
   }


   // Asset Unit

   bool operator == ( const asset_unit& a, const asset_unit& b )
   {
      return std::tie( a.units, a.balance_type, a.name, a.vesting_time ) == std::tie( b.units, b.balance_type, b.name, b.vesting_time );
   }

   bool operator < ( const asset_unit& a, const asset_unit& b )
   {
      return std::tie( a.units, a.balance_type, a.name, a.vesting_time ) < std::tie( b.units, b.balance_type, b.name, b.vesting_time );
   }

   bool operator <= ( const asset_unit& a, const asset_unit& b )
   {
      return std::tie( a.units, a.balance_type, a.name, a.vesting_time ) <= std::tie( b.units, b.balance_type, b.name, b.vesting_time );
   }

   bool operator != ( const asset_unit& a, const asset_unit& b )
   {
      return std::tie( a.units, a.balance_type, a.name, a.vesting_time ) != std::tie( b.units, b.balance_type, b.name, b.vesting_time );
   }

   bool operator > ( const asset_unit& a, const asset_unit& b )
   {
      return std::tie( a.units, a.balance_type, a.name, a.vesting_time ) > std::tie( b.units, b.balance_type, b.name, b.vesting_time );
   }

   bool operator >= ( const asset_unit& a, const asset_unit& b )
   {
      return std::tie( a.units, a.balance_type, a.name, a.vesting_time ) >= std::tie( b.units, b.balance_type, b.name, b.vesting_time );
   }

} } // node::protocol