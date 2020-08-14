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
using node::protocol::comment_reward_curve;
using fc::uint128_t;

uint8_t find_msb( const uint128_t& u );

uint64_t approx_sqrt( const uint128_t& x );


/**
 * Container of reward pool values 
 * to be passed into comment reward calculations.
 */
struct comment_reward_context
{
   uint128_t             recent_content_claims = 0;                       ///< Sum of the reward curve from posts in the previous decay time, decays linearly.

   asset                 total_reward_fund;                               ///< Amount of Currency asset available for content reward distribution.

   price                 current_COIN_USD_price;                          ///< Price of Currency Asset in USD, used to manage maximum payout values and dust threshold.        
};

/**
 * Determine the current reward curve value for a comment
 * given the parameters contained in the comment's reward curve
 * as included at creating time.
 */
uint128_t evaluate_reward_curve( const comment_object& comment );


} } } // node::chain::util

FC_REFLECT( node::chain::util::comment_reward_context,
         (recent_content_claims)
         (total_reward_fund)
         (current_COIN_USD_price)
         );