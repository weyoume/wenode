#pragma once

#include <node/protocol/types.hpp>
#include <node/protocol/asset.hpp>

namespace node { namespace chain { namespace util {

using node::protocol::asset;
using node::protocol::price;
using node::protocol::asset_symbol_type;

asset asset_to_USD( const price& p, const asset& a )
{
   FC_ASSERT( a.symbol != SYMBOL_USD );
   asset_symbol_type quote_symbol = p.quote.symbol;
   asset_symbol_type base_symbol = p.base.symbol;
   FC_ASSERT( base_symbol == SYMBOL_USD || quote_symbol == SYMBOL_USD );

   if( p.is_null() )
   {
      return asset( 0, SYMBOL_USD );
   }
      
   return a * p;
}

asset USD_to_asset( const price& p, const asset& a )
{
   FC_ASSERT( a.symbol == SYMBOL_USD );
   asset_symbol_type quote_symbol = p.quote.symbol;
   asset_symbol_type base_symbol = p.base.symbol;
   FC_ASSERT( base_symbol == SYMBOL_USD || quote_symbol == SYMBOL_USD );

   if( p.is_null() ) 
   {
      if( base_symbol == SYMBOL_USD) 
      {
         return asset( 0, quote_symbol );
      } 
      else if( quote_symbol == SYMBOL_USD ) 
      {
         return asset( 0, base_symbol );
      }
   }
    
   return a * p;
}

} } }