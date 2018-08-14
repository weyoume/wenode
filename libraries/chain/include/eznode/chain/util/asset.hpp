#pragma once

#include <eznode/protocol/asset.hpp>

namespace eznode { namespace chain { namespace util {

using eznode::protocol::asset;
using eznode::protocol::price;

inline asset to_EZD( const price& p, const asset& ECO )
{
   FC_ASSERT( ECO.symbol == SYMBOL_ECO );
   if( p.is_null() )
      return asset( 0, SYMBOL_EZD );
   return ECO * p;
}

inline asset to_ECO( const price& p, const asset& EZD )
{
   FC_ASSERT( EZD.symbol == SYMBOL_EZD );
   if( p.is_null() )
      return asset( 0, SYMBOL_ECO );
   return EZD * p;
}

} } }
