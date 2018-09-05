#pragma once

#include <node/protocol/asset.hpp>

namespace node { namespace chain { namespace util {

using node::protocol::asset;
using node::protocol::price;

inline asset to_TSD( const price& p, const asset& TME )
{
   FC_ASSERT( TME.symbol == SYMBOL_TME );
   if( p.is_null() )
      return asset( 0, SYMBOL_TSD );
   return TME * p;
}

inline asset to_TME( const price& p, const asset& TSD )
{
   FC_ASSERT( TSD.symbol == SYMBOL_TSD );
   if( p.is_null() )
      return asset( 0, SYMBOL_TME );
   return TSD * p;
}

} } }
