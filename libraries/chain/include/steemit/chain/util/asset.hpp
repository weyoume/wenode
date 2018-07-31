#pragma once

#include <ezira/protocol/asset.hpp>

namespace ezira { namespace chain { namespace util {

using ezira::protocol::asset;
using ezira::protocol::price;

inline asset to_sbd( const price& p, const asset& steem )
{
   FC_ASSERT( steem.symbol == EZIRA_SYMBOL );
   if( p.is_null() )
      return asset( 0, SBD_SYMBOL );
   return steem * p;
}

inline asset to_steem( const price& p, const asset& sbd )
{
   FC_ASSERT( sbd.symbol == SBD_SYMBOL );
   if( p.is_null() )
      return asset( 0, EZIRA_SYMBOL );
   return sbd * p;
}

} } }
