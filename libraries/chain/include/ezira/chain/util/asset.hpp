#pragma once

#include <ezira/protocol/asset.hpp>

namespace ezira { namespace chain { namespace util {

using ezira::protocol::asset;
using ezira::protocol::price;

inline asset to_sbd( const price& p, const asset& ezira )
{
   FC_ASSERT( ezira.symbol == EZIRA_SYMBOL );
   if( p.is_null() )
      return asset( 0, SBD_SYMBOL );
   return ezira * p;
}

inline asset to_ezira( const price& p, const asset& sbd )
{
   FC_ASSERT( sbd.symbol == SBD_SYMBOL );
   if( p.is_null() )
      return asset( 0, EZIRA_SYMBOL );
   return sbd * p;
}

} } }
