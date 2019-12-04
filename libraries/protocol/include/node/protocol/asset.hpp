#pragma once
#include <node/protocol/types.hpp>
#include <node/protocol/config.hpp>

namespace node { namespace protocol {

   struct price;

   struct asset
   {
      asset( share_type amount = 0, asset_symbol_type symbol = SYMBOL_COIN, int16_t precision = BLOCKCHAIN_PRECISION_DIGITS ) : 
      amount(amount),symbol(symbol),precision(precision){}

      share_type           amount;
      asset_symbol_type    symbol;
      int16_t              precision;

      /// Convert an asset to a textual representation, i.e. "123.45"
      string amount_to_string(share_type amount)const;

      /// Convert an asset to a textual representation, i.e. "123.45"
      string amount_to_string(const asset& amount)const
      { FC_ASSERT(amount.symbol == symbol); return amount_to_string(amount.amount); }

      static asset from_string( const string& from );
      string       to_string()const;

      int64_t precision_value()const;

      /// Convert a string amount (i.e. "123.45") to an asset object with this asset's type
      /// The string may have a decimal and/or a negative sign.
      asset amount_from_string(string amount_string)const;

      /// Convert an asset to a textual representation, i.e. "123.45"
      string amount_to_string(share_type amount)const;

      /// Convert an asset to a textual representation, i.e. "123.45"
      string amount_to_string(const asset& amount)const
      { FC_ASSERT(amount.symbol == symbol); return amount_to_string(amount.amount); }

      /// Convert an asset to a textual representation with symbol, i.e. "123.45 USD"
      string amount_to_pretty_string(share_type amount)const
      { return amount_to_string(amount) + " " + symbol; }

      /// Convert an asset to a textual representation with symbol, i.e. "123.45 USD"
      string amount_to_pretty_string(const asset &amount)const
      { FC_ASSERT(amount.symbol == symbol); return amount_to_pretty_string(amount.amount); }

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

   struct price
   {
      price(const asset& base = asset(), const asset& quote = asset())
         : base(base),quote(quote){}

      asset base;
      asset quote;

      static price max(asset_symbol_type base, asset_symbol_type quote );
      static price min(asset_symbol_type base, asset_symbol_type quote );

      price max()const { return price::max( base.symbol, quote.symbol ); }
      price min()const { return price::min( base.symbol, quote.symbol ); }

      bool is_null()const;
      void validate()const;

      double to_real()const { return double(base.amount.value)/double(quote.amount.value); }
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

   struct price_feed
   {
      /**
       *  Required maintenance collateral is defined
       *  as a fixed point number with a maximum value of 10.000
       *  and a minimum value of 1.000.  (denominated in GRAPHENE_COLLATERAL_RATIO_DENOM)
       *
       *  A black swan event occurs when value_of_collateral equals
       *  value_of_debt, to avoid a black swan a margin call is
       *  executed when value_of_debt * required_maintenance_collateral
       *  equals value_of_collateral using rate.
       *
       *  Default requirement is $1.75 of collateral per $1 of debt
       *
       *  BlackSwan ---> SQR ---> MCR ----> SP
       */
      ///@{
      /**
       * Forced settlements will evaluate using this price, defined as BITASSET / COLLATERAL
       */
      price settlement_price;

      /// Price at which automatically exchanging this asset for CORE from fee pool occurs (used for paying fees)
      price core_exchange_rate;

      /** Fixed point between 1.000 and 10.000, implied fixed point denominator is GRAPHENE_COLLATERAL_RATIO_DENOM */
      uint16_t maintenance_collateral_ratio = MAINTENANCE_COLLATERAL_RATIO;

      /** Fixed point between 1.000 and 10.000, implied fixed point denominator is GRAPHENE_COLLATERAL_RATIO_DENOM */
      uint16_t maximum_short_squeeze_ratio = MAX_SHORT_SQUEEZE_RATIO;

      /** When selling collateral to pay off debt, the least amount of debt to receive should be
       *  min_usd = max_short_squeeze_price() * collateral
       *
       *  This is provided to ensure that a black swan cannot be trigged due to poor liquidity alone, it
       *  must be confirmed by having the max_short_squeeze_price() move below the black swan price.
       */
      price max_short_squeeze_price()const;
      /// Another implementation of max_short_squeeze_price() before the core-1270 hard fork

      /// Call orders with collateralization (aka collateral/debt) not greater than this value are in margin call territory.
      /// Calculation: ~settlement_price * maintenance_collateral_ratio / GRAPHENE_COLLATERAL_RATIO_DENOM
      price maintenance_collateralization()const;

      friend bool operator == ( const price_feed& a, const price_feed& b )
      {
         return std::tie( a.settlement_price, a.maintenance_collateral_ratio, a.maximum_short_squeeze_ratio ) ==
                std::tie( b.settlement_price, b.maintenance_collateral_ratio, b.maximum_short_squeeze_ratio );
      }

      void validate() const;
      bool is_for( asset_symbol_type symbol ) const;
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
