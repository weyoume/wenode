#pragma once

#include <ezira/chain/util/asset.hpp>
#include <ezira/chain/ezira_objects.hpp>

#include <ezira/protocol/asset.hpp>
#include <ezira/protocol/config.hpp>
#include <ezira/protocol/types.hpp>

#include <fc/reflect/reflect.hpp>

#include <fc/uint128.hpp>

namespace ezira { namespace chain { namespace util {

using ezira::protocol::asset;
using ezira::protocol::price;
using ezira::protocol::share_type;

using fc::uint128_t;

struct comment_reward_context
{
   share_type rshares;
   uint16_t   reward_weight = 0;
   asset      max_EZD;
   uint128_t  total_reward_shares2;
   asset      total_reward_fund_ezira;
   price      current_ezira_price;
   curve_id   reward_curve = quadratic;
   uint128_t  content_constant = CONTENT_CONSTANT_HF0;
};

uint64_t get_rshare_reward( const comment_reward_context& ctx );

inline uint128_t get_content_constant_s()
{
   return CONTENT_CONSTANT_HF0; // looking good for posters
}

uint128_t evaluate_reward_curve( const uint128_t& rshares, const curve_id& curve = quadratic, const uint128_t& content_constant = CONTENT_CONSTANT_HF0 );

inline bool is_comment_payout_dust( const price& p, uint64_t ezira_payout )
{
   return to_EZD( p, asset( ezira_payout, SYMBOL_EZIRA ) ) < MIN_PAYOUT_EZD;
}

} } } // ezira::chain::util

FC_REFLECT( ezira::chain::util::comment_reward_context,
   (rshares)
   (reward_weight)
   (max_EZD)
   (total_reward_shares2)
   (total_reward_fund_ezira)
   (current_ezira_price)
   (reward_curve)
   (content_constant)
   )
