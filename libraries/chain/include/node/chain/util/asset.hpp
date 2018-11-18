#pragma once

#include <node/protocol/asset.hpp>

namespace node { namespace chain { namespace util {

using node::protocol::asset;
using node::protocol::price;

inline asset to_TSD( const price& p, const asset& TME )
{
   FC_ASSERT( TME.symbol == SYMBOL_COIN );
   if( p.is_null() )
      return asset( 0, SYMBOL_USD );
   return TME * p;
}

inline asset to_TME( const price& p, const asset& TSD )
{
   FC_ASSERT( TSD.symbol == SYMBOL_USD );
   if( p.is_null() )
      return asset( 0, SYMBOL_COIN );
   return TSD * p;
}

} } }
