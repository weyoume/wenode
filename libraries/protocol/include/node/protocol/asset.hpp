#pragma once
#include <node/protocol/types.hpp>
#include <node/protocol/config.hpp>
#include <node/protocol/node_operations.hpp>

namespace node { namespace protocol {

   struct price;

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
      asset( share_type amount = 0, asset_symbol_type symbol = SYMBOL_COIN, int16_t precision = BLOCKCHAIN_PRECISION_DIGITS ) : 
      amount(amount),symbol(symbol),precision(precision){}

      share_type           amount;         ///< The amount of asset being represented, using safe int64_t / share_type

      asset_symbol_type    symbol;         ///< The asset type being represented.

      int16_t              precision;      ///< The Asset precision being represented, defaults to BLOCKCHAIN_PRECISION_DIGITS.

      static asset from_string( string from );

      string to_string()const;

      int64_t precision_value()const;

      double to_real()const
      {
         return double( amount.value ) / double( precision );
      }

      asset amount_from_string( string amount_string )const;     ///< Convert a string amount to an asset object with this asset's type

      string amount_to_string( share_type amount )const;          ///< Convert an asset to a textual representation
      
      string amount_to_string( const asset& amount )const           ///< Convert an asset to a textual representation
      { 
         FC_ASSERT( amount.symbol == symbol ); 
         return amount_to_string( amount.amount ); 
      }

      string amount_to_pretty_string(share_type amount)const       ///< Convert an asset to a textual representation with symbol
      { 
         return amount_to_string( amount ) + " " + symbol; 
      }

      string amount_to_pretty_string(const asset &amount)const     ///< Convert an asset to a textual representation with symbol
      { 
         FC_ASSERT( amount.symbol == symbol ); 
         return amount_to_pretty_string( amount.amount ); 
      }

      asset multiply_and_round_up( const price& b )const;

      asset& operator += ( const asset& o )
      {
         FC_ASSERT( symbol == o.symbol );
         amount += o.amount;
         return *this;
      }

      asset& operator -= ( const asset& o )
      {
         FC_ASSERT( symbol == o.symbol );
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
         FC_ASSERT( a.symbol == b.symbol );
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
         return !(a <= b);
      }
      friend bool operator >= ( const asset& a, const asset& b )
      {
         return !(a < b);
      }

      friend asset operator - ( const asset& a, const asset& b )
      {
         FC_ASSERT( a.symbol == b.symbol );
         return asset( a.amount - b.amount, a.symbol );
      }
      friend asset operator + ( const asset& a, const asset& b )
      {
         FC_ASSERT( a.symbol == b.symbol );
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
      price(const asset& base = asset(), const asset& quote = asset())
         : base(base),quote(quote){}

      asset base;    ///< The Asset unit of account of the price.

      asset quote;   ///< The Asset being valued in units of the Base asset. 

      static price max(asset_symbol_type base, asset_symbol_type quote );
      static price min(asset_symbol_type base, asset_symbol_type quote );

      price max()const { return price::max( base.symbol, quote.symbol ); }
      price min()const { return price::min( base.symbol, quote.symbol ); }

      bool is_null()const;
      void validate()const;

      double to_real()const { return double(base.amount.value)/double(quote.amount.value); }

      string to_string()const;
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
      price       settlement_price;                                              ///< Forced settlements will evaluate using this price, defined as BITASSET / COLLATERAL

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
    * Contains the strike price and expiration date of an option asset.
    */
   struct option_strike
   {
      option_strike( price strike_price = price(), date_type expiration_date = date_type() );

      price         strike_price;         ///< Price that the option can be exercised at.

      date_type     expiration_date;      ///< Date at which the option will expire.

      bool is_null()const 
      { 
         return strike_price.is_null() || expiration_date.is_null();
      };

      void validate()const
      { 
         strike_price.validate();
         expiration_date.validate();
      };

      double to_real()const
      { 
         return strike_price.to_real();
      }

      string to_string()const;
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
    * according to the input fund units
    */
   struct asset_unit
   {
      asset_unit( account_name_type name, uint16_t units );

      account_name_type      name;

      uint16_t               units;
   };
   
} } // node::protocol

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
         (expiration_date)
         );

FC_REFLECT( node::protocol::asset_unit,
         (name)
         (units)
         );