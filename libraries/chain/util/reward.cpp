
#include <node/chain/util/reward.hpp>
#include <node/chain/util/uint256.hpp>

namespace node { namespace chain { namespace util {

uint8_t find_msb( const uint128_t& u )
{
   uint64_t x;
   uint8_t places;
   x      = (u.lo ? u.lo : 1);
   places = (u.hi ?   64 : 0);
   x      = (u.hi ? u.hi : x);
   return uint8_t( boost::multiprecision::detail::find_msb(x) + places );
}

uint64_t approx_sqrt( const uint128_t& x )
{
   if( (x.lo == 0) && (x.hi == 0) )
      return 0;

   uint8_t msb_x = find_msb(x);
   uint8_t msb_z = msb_x >> 1;

   uint128_t msb_x_bit = uint128_t(1) << msb_x;
   uint64_t  msb_z_bit = uint64_t (1) << msb_z;

   uint128_t mantissa_mask = msb_x_bit - 1;
   uint128_t mantissa_x = x & mantissa_mask;
   uint64_t mantissa_z_hi = (msb_x & 1) ? msb_z_bit : 0;
   uint64_t mantissa_z_lo = (mantissa_x >> (msb_x - msb_z)).lo;
   uint64_t mantissa_z = (mantissa_z_hi | mantissa_z_lo) >> 1;
   uint64_t result = msb_z_bit | mantissa_z;

   return result;
}


/**
 * Determines the value of the comment's reward curve value, based on its
 * net reward value, and past payout details, and the variables of the network.
 */
uint128_t evaluate_reward_curve( const comment_object& c )
{
   uint128_t result = 0;
   if( c.net_reward.value <= 0 )
   {
      return result;
   }
   comment_reward_curve curve = c.reward_curve;
   uint128_t r = uint128_t( c.net_reward.value );
   uint128_t s = curve.constant_factor;
   int64_t d = c.cashouts_received;
   int64_t t = curve.reward_interval_amount;
   int64_t m = std::max(int64_t(0),t-d);

   uint128_t rs_3 = (r+s)*(r+s)*(r+s);
   uint128_t s_3 = s*s*s;
   uint128_t quad = ((rs_3-s_3)*uint128_t(curve.quadratic_percent))/uint128_t(PERCENT_100);

   uint128_t rs_25 = (r+s)*(r+s)*approx_sqrt(r+s);
   uint128_t s_25 = s*s*approx_sqrt(s);
   uint128_t semi_quad = ((rs_25-s_25)*uint128_t(curve.semi_quadratic_percent))/uint128_t(PERCENT_100);

   uint128_t rs_2 = (r+s)*(r+s);
   uint128_t s_2 = s*s;
   uint128_t linear = ((rs_2-s_2)*uint128_t(curve.linear_percent))/uint128_t(PERCENT_100);

   uint128_t rs_15 = (r+s)*approx_sqrt(r+s);
   uint128_t s_15 = s*approx_sqrt(s);
   uint128_t sq_rt = ((rs_15-s_15)*uint128_t(curve.sqrt_percent))/uint128_t(PERCENT_100);
   
   if( d < t )
   {
      result = (m*((quad+semi_quad+linear+sq_rt)/(r+4*s)))/t;
   }

   return result;
}

} } } // node::chain::util
