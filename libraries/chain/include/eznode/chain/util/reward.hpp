#pragma once

#include <eznode/chain/util/asset.hpp>
#include <eznode/chain/eznode_objects.hpp>

#include <eznode/protocol/asset.hpp>
#include <eznode/protocol/config.hpp>
#include <eznode/protocol/types.hpp>

#include <fc/reflect/reflect.hpp>

#include <fc/uint128.hpp>

namespace eznode { namespace chain { namespace util {

using eznode::protocol::asset;
using eznode::protocol::price;
using eznode::protocol::share_type;

using fc::uint128_t;

struct comment_reward_context
{
   share_type rewardESCOR;
   uint16_t   reward_weight = 0;
   asset      max_EUSD;
   uint128_t  total_reward_ESCOR2;
   asset      total_reward_fund_ECO;
   price      current_ECO_price;
   curve_id   reward_curve = quadratic;
   uint128_t  content_constant = CONTENT_CONSTANT_HF0;
};

uint64_t get_ESCOR_reward( const comment_reward_context& ctx );

inline uint128_t get_content_constant_s()
{
   return CONTENT_CONSTANT_HF0; // looking good for posters
}

uint128_t evaluate_reward_curve( const uint128_t& rewardESCOR, const curve_id& curve = quadratic, const uint128_t& content_constant = CONTENT_CONSTANT_HF0 );

inline bool is_comment_payout_dust( const price& p, uint64_t ECOpayout )
{
   return to_EUSD( p, asset( ECOpayout, SYMBOL_ECO ) ) < MIN_PAYOUT_EUSD;
}

} } } // eznode::chain::util

FC_REFLECT( eznode::chain::util::comment_reward_context,
   (rewardESCOR)
   (reward_weight)
   (max_EUSD)
   (total_reward_ESCOR2)
   (total_reward_fund_ECO)
   (current_ECO_price)
   (reward_curve)
   (content_constant)
   )
