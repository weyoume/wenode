#pragma once

#include <node/protocol/asset.hpp>

namespace node { namespace chain { namespace util {

using node::protocol::asset;
using node::protocol::price;

inline asset to_EUSD( const price& p, const asset& ECO )
{
   FC_ASSERT( ECO.symbol == SYMBOL_ECO );
   if( p.is_null() )
      return asset( 0, SYMBOL_EUSD );
   return ECO * p;
}

inline asset to_ECO( const price& p, const asset& EUSD )
{
   FC_ASSERT( EUSD.symbol == SYMBOL_EUSD );
   if( p.is_null() )
      return asset( 0, SYMBOL_ECO );
   return EUSD * p;
}

} } }
