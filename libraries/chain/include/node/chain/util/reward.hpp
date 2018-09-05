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
   share_type SCOREreward;
   uint16_t   reward_weight = 0;
   asset      max_TSD;
   uint128_t  total_SCOREreward2;
   asset      total_reward_fund_TME;
   price      current_TME_price;
   curve_id   reward_curve = quadratic;
   uint128_t  content_constant = CONTENT_CONSTANT_HF0;
};

uint64_t get_SCORE_reward( const comment_reward_context& ctx );

inline uint128_t get_content_constant_s()
{
   return CONTENT_CONSTANT_HF0; // looking good for posters
}

uint128_t evaluate_reward_curve( const uint128_t& SCOREreward, const curve_id& curve = quadratic, const uint128_t& content_constant = CONTENT_CONSTANT_HF0 );

inline bool is_comment_payout_dust( const price& p, uint64_t TMEpayout )
{
   return to_TSD( p, asset( TMEpayout, SYMBOL_COIN ) ) < MIN_PAYOUT_TSD;
}

} } } // node::chain::util

FC_REFLECT( node::chain::util::comment_reward_context,
   (SCOREreward)
   (reward_weight)
   (max_TSD)
   (total_SCOREreward2)
   (total_reward_fund_TME)
   (current_TME_price)
   (reward_curve)
   (content_constant)
   )
