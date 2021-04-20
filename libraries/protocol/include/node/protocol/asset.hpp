#pragma once

#include <node/protocol/types.hpp>
#include <node/protocol/config.hpp>

namespace node { namespace protocol {

   struct asset;
   struct price;
   struct price_feed;
   struct option_strike;
   struct asset_unit;

   /**
    * Valid symbols can contain [A-Z0-9], and '.'
    * They must start with [A, Z]
    * They must end with [A-Z0-9]
    * They can contain a maximum of ten '.'
    */
   inline bool validate_asset_symbol( const string& symbol )
   {
      static const std::locale& loc = std::locale::classic();
      if( symbol.size() < MIN_ASSET_SYMBOL_LENGTH )
         return false;

      if( symbol.size() > MAX_ASSET_SYMBOL_LENGTH )
         return false;

      if( !isalpha( symbol.front(), loc ) )
         return false;

      if( !isalnum( symbol.back(), loc ) )
         return false;

      uint8_t dot_count = 0;
      for( const auto c : symbol )
      {
         if( (isalpha( c, loc ) && isupper( c, loc )) || isdigit( c, loc ) )
            continue;

         if( c == '.' )
         {
            dot_count++;
            if( dot_count > 10 )
            {
               return false;
            }
            continue;
         }

         return false;
      }

      return true;
   }

   /**
    * An object representing an amount of a specified asset.
    * 
    * Enables for integer mathemtics between units of assets, and conversion between 
    * assets and strings specifiying amounts from operations.
    *
    * Each Asset has 100,000,000 integer units, offering 8 decimal places of precision
    * to amounts and balances.
    */
   struct asset
   {
      asset( 
         share_type amount = 0,
         asset_symbol_type symbol = SYMBOL_COIN,
         int16_t precision = BLOCKCHAIN_PRECISION_DIGITS ) : 
         amount(amount),
         symbol(symbol),
         precision(precision){}

      share_type           amount;         ///< The amount of asset being represented, using safe int64_t / share_type

      asset_symbol_type    symbol;         ///< The asset type being represented.

      int16_t              precision;      ///< The Asset precision being represented, defaults to BLOCKCHAIN_PRECISION_DIGITS.

      static asset from_string( string from );

      string to_string()const;

      int64_t precision_value()const
      {
         static int64_t table[] = {
            1, 
            10, 
            100, 
            1000, 
            10000,
            100000, 
            1000000, 
            10000000, 
            100000000ll,
            1000000000ll, 
            10000000000ll,
            100000000000ll, 
            1000000000000ll,
            10000000000000ll, 
            100000000000000ll
            };
         return table[ precision ];
      }

      double to_real()const
      {
         return double( amount.value ) / double( precision_value() );
      }

      asset amount_from_string( string amount_string )const;     ///< Convert a string amount to an asset object with this asset's type

      asset multiply_and_round_up( const price& b )const;

      asset& operator += ( const asset& o )
      {
         FC_ASSERT( symbol == o.symbol,
            "Assets symbols must be the same: ${a} ${b}",
            ("a",*this)("b",o.symbol) );
         amount += o.amount;
         return *this;
      }

      asset& operator -= ( const asset& o )
      {
         FC_ASSERT( symbol == o.symbol,
            "Assets symbols must be the same: ${a} ${b}",
            ("a",*this)("b",o.symbol) );
         amount -= o.amount;
         return *this;
      }
      asset operator -()const { return asset( -amount, symbol ); }

      friend bool operator == ( const asset& a, const asset& b )
      {
         return std::tie( a.symbol, a.amount ) == std::tie( b.symbol, b.amount );
      }
      friend bool operator < ( const asset& a, const asset& b )
      {
         FC_ASSERT( a.symbol == b.symbol,
            "Assets symbols must be the same: ${a} ${b}",
            ("a",a)("b",b));
         return a.amount < b.amount;
      }
      friend bool operator <= ( const asset& a, const asset& b )
      {
         return (a == b) || (a < b);
      }
      friend bool operator != ( const asset& a, const asset& b )
      {
         return !(a == b);
      }
      friend bool operator > ( const asset& a, const asset& b )
      {
         FC_ASSERT( a.symbol == b.symbol,
            "Assets symbols must be the same: ${a} ${b}",
            ("a",a)("b",b));
         return a.amount > b.amount;
      }
      friend bool operator >= ( const asset& a, const asset& b )
      {
         return !(a < b);
      }
      friend asset operator - ( const asset& a, const asset& b )
      {
         FC_ASSERT( a.symbol == b.symbol,
            "Assets symbols must be the same: ${a} ${b}",
            ("a",a)("b",b));
         return asset( a.amount - b.amount, a.symbol );
      }
      friend asset operator + ( const asset& a, const asset& b )
      {
         FC_ASSERT( a.symbol == b.symbol,
            "Assets symbols must be the same: ${a} ${b}",
            ("a",a)("b",b));
         return asset( a.amount + b.amount, a.symbol );
      }
      friend asset operator * ( const asset& a, share_type b )
      {
         return asset( a.amount * b, a.symbol );
      }
      friend asset operator * ( share_type a, const asset& b )
      {
         return asset( a * b.amount, b.symbol );
      }
      friend asset operator / ( const asset& a, share_type b )
      {
         return asset( a.amount / b, a.symbol );
      }
      friend asset operator / ( share_type a, const asset& b )
      {
         return asset( a / b.amount, b.symbol );
      }
   };


   /**
    * Measures an exchange rate equality between two assets, a base, and a quote.
    * 
    * Base asset is the unit of account of the price, and quote is the asset being priced.
    */
   struct price
   {
      price( 
         asset base = asset(), 
         asset quote = asset() ) : 
         base(base),
         quote(quote){}

      asset base;     ///< The Asset unit of account of the price. 10 MEC

      asset quote;   ///< The Asset being valued in units of the Base asset. 20 MUSD

      static price max( asset_symbol_type base, asset_symbol_type quote );
      static price min( asset_symbol_type base, asset_symbol_type quote );

      price max()const { return price::max( base.symbol, quote.symbol ); }
      price min()const { return price::min( base.symbol, quote.symbol ); }

      bool is_null()const;
      void validate()const;

      double to_real()const { return double(base.amount.value)/double(quote.amount.value); }

      string to_string()const { return fc::to_string( to_real() ).substr( 0, 8 ) + " " + quote.symbol + "/" + base.symbol; };
   };

   price operator / ( const asset& base, const asset& quote );
   inline price operator~( const price& p ) { return price{p.quote,p.base}; }   // Inverts the price

   price operator *  ( const price& p, const ratio_type& r );
   price operator /  ( const price& p, const ratio_type& r );

   price operator + ( const price& p, const price& q);
   price operator - ( const price& p, const price& q);

   inline price& operator *=  ( price& p, const ratio_type& r )
   { return p = p * r; }
   inline price& operator /=  ( price& p, const ratio_type& r )
   { return p = p / r; }

   bool  operator <  ( const asset& a, const asset& b );
   bool  operator <= ( const asset& a, const asset& b );
   bool  operator <  ( const price& a, const price& b );
   bool  operator <= ( const price& a, const price& b );
   bool  operator >  ( const price& a, const price& b );
   bool  operator >= ( const price& a, const price& b );
   bool  operator == ( const price& a, const price& b );
   bool  operator != ( const price& a, const price& b );
   asset operator *  ( const asset& a, const price& b );


   /**
    * Price information for a market-issued asset at a time interval.
    * 
    * Required maintenance collateral is defined
    * as a fixed point number with a maximum value of 10.000
    * and a minimum value of 1.000.  (denominated in COLLATERAL_RATIO_DENOM)
    *
    * A black swan event occurs when value_of_collateral equals
    * value_of_debt, to avoid a black swan a margin call is
    * executed when value_of_debt * required_maintenance_collateral
    * equals value_of_collateral using rate.
    *
    * Default requirement is $1.75 of collateral per $1 of debt.
    *
    * BlackSwan ---> SQR ---> MCR ----> SP
    */
   struct price_feed
   {
      price       settlement_price;                                              ///< Forced settlements will evaluate using this price, defined as STABLECOIN / COLLATERAL

      uint16_t    maintenance_collateral_ratio = MAINTENANCE_COLLATERAL_RATIO;   ///< Fixed point between 1.000 and 10.000, implied fixed point denominator is COLLATERAL_RATIO_DENOM

      uint16_t    maximum_short_squeeze_ratio = MAX_SHORT_SQUEEZE_RATIO;         ///< Fixed point between 1.000 and 10.000, implied fixed point denominator is COLLATERAL_RATIO_DENOM 

      /** 
       * When selling collateral to pay off debt, the least amount of debt to receive should be
       * min_usd = max_short_squeeze_price() * collateral
       * This is provided to ensure that a black swan cannot be trigged due to poor liquidity alone, it
       * must be confirmed by having the max_short_squeeze_price() move below the black swan price.
       */
      price max_short_squeeze_price()const;

      /// Call orders with collateralization (aka collateral/debt) not greater than this value are in margin call territory.
      /// Calculation: ~settlement_price * maintenance_collateral_ratio / COLLATERAL_RATIO_DENOM
      price maintenance_collateralization()const;

      friend bool operator == ( const price_feed& a, const price_feed& b )
      {
         return std::tie( a.settlement_price, a.maintenance_collateral_ratio, a.maximum_short_squeeze_ratio ) ==
                std::tie( b.settlement_price, b.maintenance_collateral_ratio, b.maximum_short_squeeze_ratio );
      }

      void validate() const;
      bool is_for( asset_symbol_type symbol ) const;
   };


   /**
    * Contains the strike price, call/put directionality and expiration date of an option asset.
    */
   struct option_strike
   {
      option_strike( 
         price strike_price = price(), 
         bool call = true,
         share_type multiple = OPTION_ASSET_MULTIPLE,
         date_type expiration_date = date_type() ) : 
         strike_price(strike_price), 
         call(call),
         multiple(multiple),
         expiration_date(expiration_date){}

      price              strike_price;         ///< Price that the option can be exercised at.

      bool               call;                 ///< True for call option, false for put option.

      share_type         multiple;             ///< Amount of underlying asset the option corresponds to.

      date_type          expiration_date;      ///< Date at which the option will expire.

      bool is_null()const 
      { 
         return strike_price.is_null() || expiration_date.is_null();
      };

      void validate()const;

      double to_real()const
      { 
         return strike_price.to_real();
      }

      string to_string()const;

      string details( string quote_display, string quote_details, string base_display, string base_details )const;

      string display_symbol()const;

      asset_symbol_type option_symbol()const;

      static option_strike from_string( const string& from );

      time_point expiration()const;
   };

   bool  operator <  ( const option_strike& a, const option_strike& b );
   bool  operator <= ( const option_strike& a, const option_strike& b );
   bool  operator >  ( const option_strike& a, const option_strike& b );
   bool  operator >= ( const option_strike& a, const option_strike& b );
   bool  operator == ( const option_strike& a, const option_strike& b );
   bool  operator != ( const option_strike& a, const option_strike& b );

   /**
    * Dividing units in an asset distribution process.
    * For every unit of asset contributed, the proceeds are divided
    * according to the input fund units.
    * 
    * Use the name "sender" to specify the sender of the funds balance.
    */
   struct asset_unit
   {
      asset_unit( 
         account_name_type name = account_name_type(), 
         share_type units = 0,
         string balance_type = account_balance_values[ 0 ],
         time_point vesting_time = GENESIS_TIME ) : 
         name(name),
         units(units),
         balance_type(balance_type),
         vesting_time(vesting_time){}

      account_name_type          name = ASSET_UNIT_SENDER;

      share_type                 units = BLOCKCHAIN_PRECISION;

      string                     balance_type = account_balance_values[ 0 ];

      time_point                 vesting_time = GENESIS_TIME;
   };

   bool  operator <  ( const asset_unit& a, const asset_unit& b );
   bool  operator <= ( const asset_unit& a, const asset_unit& b );
   bool  operator >  ( const asset_unit& a, const asset_unit& b );
   bool  operator >= ( const asset_unit& a, const asset_unit& b );
   bool  operator == ( const asset_unit& a, const asset_unit& b );
   bool  operator != ( const asset_unit& a, const asset_unit& b );
   
} } // node::protocol

#define GRAPHENE_PRICE_FEED_FIELDS (settlement_price)(maintenance_collateral_ratio)(maximum_short_squeeze_ratio)

FC_REFLECT( node::protocol::asset,
         (amount)
         (symbol)
         (precision)
         );

FC_REFLECT( node::protocol::price,
         (base)
         (quote) 
         );

FC_REFLECT( node::protocol::price_feed,
         (settlement_price)
         (maintenance_collateral_ratio)
         (maximum_short_squeeze_ratio)
         );

FC_REFLECT( node::protocol::option_strike,
         (strike_price)
         (call)
         (multiple)
         (expiration_date)
         );

FC_REFLECT( node::protocol::asset_unit,
         (name)
         (units)
         (balance_type)
         (vesting_time)
         );