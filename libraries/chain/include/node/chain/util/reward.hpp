#pragma once

#include <node/chain/util/asset.hpp>
#include <node/chain/node_objects.hpp>

#include <node/protocol/asset.hpp>
#include <node/protocol/config.hpp>
#include <node/protocol/types.hpp>

#include <fc/reflect/reflect.hpp>

#include <fc/uint128.hpp>

namespace node { namespace chain { namespace util {

using node::protocol::asset;
using node::protocol::price;
using node::protocol::share_type;
using node::protocol::asset_symbol_type;
using fc::uint128_t;

uint8_t find_msb( const uint128_t& u );

uint64_t approx_sqrt( const uint128_t& x );


/**
 * Container of network and comment values 
 * to be passed into comment reward calculations.
 */
struct comment_reward_context
{
   share_type          reward;                         ///< Net reward from a given comment.

   asset               max_reward;                     ///< Max reward Asset in USD.

   uint128_t           recent_content_claims;          ///< Sum of the reward curve from posts in the previous decay time, decays linearly.

   asset               total_reward_fund;              ///< Amount of Coin available for content reward distribution.

   price               current_COIN_USD_price;         ///< Price of Coin in USD, used to manage maximum payout values and dust threshold.

   uint32_t            cashouts_received;              ///< Number of days that the comment has received rewards for previously.

   fc::microseconds    decay_rate;                     ///< Days over which the content reward claims linearly decays.

   fc::microseconds    reward_interval;                ///< Time over which each content reward payout occur.

   curve_id            reward_curve;                   ///< Formula selection for calculating the value of reward shares from a comment.

   uint128_t           content_constant;               ///< Constant added to reward value for reward curve calculation.
};

void                    fill_comment_reward_context_local_state( comment_reward_context& ctx, const comment_object& comment );

inline uint128_t        get_content_constant_s()
{
   return CONTENT_CONSTANT;
}

uint128_t evaluate_reward_curve( 
   const uint128_t& reward, 
   const uint32_t cashouts_received, 
   const curve_id& curve, 
   const fc::microseconds decay_rate,
   const uint128_t& content_constant );



} } } // node::chain::util

FC_REFLECT( node::chain::util::comment_reward_context,
         (reward)
         (max_reward)
         (recent_content_claims)
         (total_reward_fund)
         (current_COIN_USD_price)
         (cashouts_received)
         (decay_rate)
         (reward_curve)
         (content_constant)
         );