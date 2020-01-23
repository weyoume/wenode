
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

void fill_comment_reward_context_local_state( util::comment_reward_context& ctx, const comment_object& comment )
{
   ctx.reward = comment.net_reward;
   ctx.cashouts_received = comment.cashouts_received;
   ctx.max_reward = comment.max_accepted_payout;
}

uint128_t get_comment_reward( const comment_reward_context& ctx )
{ try {
   FC_ASSERT( ctx.reward > 0 );
   FC_ASSERT( ctx.recent_content_claims > 0 );

   u256 rf(ctx.total_reward_fund.amount.value);
   u256 total_claims = to256( ctx.recent_content_claims );

   u256 claim = to256( evaluate_reward_curve( 
      ctx.reward.value, 
      ctx.cashouts_received, 
      ctx.reward_curve, 
      ctx.decay_rate, 
      ctx.content_constant ) );

   u256 payout_u256 = ( rf * claim ) / total_claims;

   FC_ASSERT( payout_u256 <= u256( uint64_t( std::numeric_limits<int128_t>::max() ) ) );
   uint128_t payout = static_cast< uint128_t >( payout_u256 );

   if( is_comment_payout_dust( ctx.current_COIN_USD_price, payout ) )
   {
      payout = 0;
   }

   asset max_reward = USD_to_asset( ctx.current_COIN_USD_price, ctx.max_reward );

   payout = std::min( payout, uint128_t( max_reward.amount.value ) );

   return payout;
} FC_CAPTURE_AND_RETHROW( (ctx) ) }


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
         int32_t d = cashouts_received;
         int32_t t = decay_rate.to_seconds() / fc::days(1).to_seconds();
         if( d >= t )
         { 
            result = 0; 
         }
         else
         {
            result = ( std::max( 0, t-d ) * ( ( (r + s) * (r + s) * approx_sqrt(r + s) - s * s * approx_sqrt(s) ) / (r + 4 * s) ) ) / t;
         }
      }
      break;
   }

   

   return result;
}

} } } // node::chain::util
