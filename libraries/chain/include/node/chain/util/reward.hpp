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

using fc::uint128_t;

struct comment_reward_context
{
   share_type          reward;
   asset               max_reward;                                   // Max reward Asset in USD
   uint128_t           total_reward_squared;
   asset               total_reward_fund;
   price               current_COIN_USD_price;
   uint32_t            cashouts_received;                            // Number of days that the comment has received rewards for
   fc::microseconds    cashout_decay = CONTENT_REWARD_DECAY_RATE;    // Days over which the content reward linearly decays
   curve_id            reward_curve = convergent_semi_quadratic;
   uint128_t           content_constant = CONTENT_CONSTANT;
};

uint64_t get_comment_reward( const comment_reward_context& ctx );

inline uint128_t get_content_constant_s()
{
   return CONTENT_CONSTANT;
}

uint128_t evaluate_reward_curve( 
   const uint128_t& reward, 
   const uint32_t cashouts_received, 
   const curve_id& curve = convergent_semi_quadratic, 
   const fc::microseconds cashout_decay = CONTENT_REWARD_DECAY_RATE,
   const uint128_t& content_constant = CONTENT_CONSTANT 
   );

inline bool is_comment_payout_dust( const price& p, share_type reward_payout )
{
   return asset_to_USD( p, asset( reward_payout, SYMBOL_COIN ) ) < MIN_PAYOUT_USD;
}

} } } // node::chain::util

FC_REFLECT( node::chain::util::comment_reward_context,
         (reward)
         (max_reward)
         (total_reward_squared)
         (total_reward_fund)
         (current_COIN_USD_price)
         (cashouts_received)
         (cashout_decay)
         (reward_curve)
         (content_constant)
         );