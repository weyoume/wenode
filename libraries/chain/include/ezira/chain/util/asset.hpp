#pragma once

#include <ezira/protocol/asset.hpp>

namespace ezira { namespace chain { namespace util {

using ezira::protocol::asset;
using ezira::protocol::price;

inline asset to_EZD( const price& p, const asset& ezira )
{
   FC_ASSERT( ezira.symbol == SYMBOL );
   if( p.is_null() )
      return asset( 0, EZD_SYMBOL );
   return ezira * p;
}

inline asset to_ezira( const price& p, const asset& EZD )
{
   FC_ASSERT( EZD.symbol == EZD_SYMBOL );
   if( p.is_null() )
      return asset( 0, SYMBOL );
   return EZD * p;
}

} } }
