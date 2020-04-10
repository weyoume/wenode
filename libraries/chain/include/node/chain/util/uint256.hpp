#pragma once

#include <node/protocol/types.hpp>

#include <fc/uint128.hpp>

namespace node { namespace chain { namespace util {

inline uint256_t to256( const fc::uint128& t )
{
   uint256_t v(t.hi);
   v <<= 64;
   v += t.lo;
   return v;
}

} } }
