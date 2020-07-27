
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

void fill_comment_reward_context_local_state( comment_reward_context& ctx, const comment_object& comment )
{
   ctx.reward = comment.net_reward;
   ctx.cashouts_received = comment.cashouts_received;
   ctx.max_reward = comment.max_accepted_payout;
}


/**
 * Determines the value of the comment's reward curve value, based on its
 * net reward value, and past payout details, and the variables of the network.
 */
uint128_t evaluate_reward_curve( 
   const uint128_t& reward,
   const uint32_t cashouts_received,
   const curve_id& curve,
   const fc::microseconds decay_rate,
   const uint128_t& content_constant )
{
   uint128_t result = 0;

   switch( curve )
   {
      case quadratic:
      {
         uint128_t reward_plus_s = reward + content_constant;
         result = reward_plus_s * reward_plus_s - content_constant * content_constant;
      }
      break;
      case quadratic_curation:
      {
         uint128_t two_alpha = content_constant * 2;
         result = uint128_t( reward.lo, 0 ) / ( two_alpha + reward );
      }
      break;
      case linear:
      {
         result = reward;
      }  
      break;
      case square_root:
      {
         result = approx_sqrt( reward );
      } 
      break;
      case convergent_semi_quadratic:     // Returns amount converging to reward to the power of 1.5, linearly decaying over the specified payout days.
      {
         uint128_t r = reward;
         uint128_t s = content_constant;
         int64_t d = cashouts_received;
         int64_t t = decay_rate.to_seconds() / fc::days(1).to_seconds();
         int64_t m = std::max(int64_t(0),t-d);
         uint128_t rs_25 = (r+s)*(r+s)*approx_sqrt(r+s);
         uint128_t s_25 = s*s*approx_sqrt(s);
         
         if( d >= t )
         { 
            result = 0; 
         }
         else
         {
            result = (m*((rs_25-s_25)/(r+4*s)))/t;
         }
      }
      break;
   }

   // ilog( "Reward Curve: ${r} Reward: ${re}", ("r",result)("re",reward));

   return result;
}

} } } // node::chain::util
