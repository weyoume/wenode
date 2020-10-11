#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/db_with.hpp>
#include <node/chain/evaluator_registry.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/index.hpp>
#include <node/chain/node_evaluator.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/chain/shared_db_merkle.hpp>
#include <node/chain/operation_notification.hpp>
#include <node/chain/producer_schedule.hpp>

#include <node/chain/util/asset.hpp>
#include <node/chain/util/reward.hpp>
#include <node/chain/util/uint256.hpp>
#include <node/chain/util/reward.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/uint128.hpp>

#include <fc/container/deque.hpp>

#include <fc/io/fstream.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>

namespace node { namespace chain {


const comment_object& database::get_comment( const account_name_type& author, const shared_string& permlink )const
{ try {
   return get< comment_object, by_permlink >( boost::make_tuple( author, permlink ) );
} FC_CAPTURE_AND_RETHROW( (author)(permlink) ) }

const comment_object* database::find_comment( const account_name_type& author, const shared_string& permlink )const
{
   return find< comment_object, by_permlink >( boost::make_tuple( author, permlink ) );
}

const comment_object& database::get_comment( const account_name_type& author, const string& permlink )const
{ try {
   return get< comment_object, by_permlink >( boost::make_tuple( author, permlink ) );
} FC_CAPTURE_AND_RETHROW( (author)(permlink) ) }

const comment_object* database::find_comment( const account_name_type& author, const string& permlink )const
{
   return find< comment_object, by_permlink >( boost::make_tuple( author, permlink ) );
}

const comment_vote_object& database::get_comment_vote( const account_name_type& voter, const comment_id_type& id )const
{ try {
   return get< comment_vote_object, by_voter_comment >( boost::make_tuple( voter, id ) );
} FC_CAPTURE_AND_RETHROW( (voter)(id) ) }

const comment_vote_object* database::find_comment_vote( const account_name_type& voter, const comment_id_type& id )const
{
   return find< comment_vote_object, by_voter_comment >( boost::make_tuple( voter, id ) );
}

const comment_view_object& database::get_comment_view( const account_name_type& viewer, const comment_id_type& id )const
{ try {
   return get< comment_view_object, by_viewer_comment >( boost::make_tuple( viewer, id ) );
} FC_CAPTURE_AND_RETHROW( (viewer)(id) ) }

const comment_view_object* database::find_comment_view( const account_name_type& viewer, const comment_id_type& id )const
{
   return find< comment_view_object, by_viewer_comment >( boost::make_tuple( viewer, id ) );
}

const comment_share_object& database::get_comment_share( const account_name_type& sharer, const comment_id_type& id )const
{ try {
   return get< comment_share_object, by_sharer_comment >( boost::make_tuple( sharer, id ) );
} FC_CAPTURE_AND_RETHROW( (sharer)(id) ) }

const comment_share_object* database::find_comment_share( const account_name_type& sharer, const comment_id_type& id )const
{
   return find< comment_share_object, by_sharer_comment >( boost::make_tuple( sharer, id ) );
}

const comment_moderation_object& database::get_comment_moderation( const account_name_type& moderator, const comment_id_type& id )const
{ try {
   return get< comment_moderation_object, by_moderator_comment >( boost::make_tuple( moderator, id ) );
} FC_CAPTURE_AND_RETHROW( (moderator)(id) ) }

const comment_moderation_object* database::find_comment_moderation( const account_name_type& moderator, const comment_id_type& id )const
{
   return find< comment_moderation_object, by_moderator_comment >( boost::make_tuple( moderator, id ) );
}

const list_object& database::get_list( const account_name_type& creator, const shared_string& list_id )const
{ try {
   return get< list_object, by_list_id >( boost::make_tuple( creator, list_id ) );
} FC_CAPTURE_AND_RETHROW( (creator)(list_id) ) }

const list_object* database::find_list( const account_name_type& creator, const shared_string& list_id )const
{
   return find< list_object, by_list_id >( boost::make_tuple( creator, list_id ) );
}

const list_object& database::get_list( const account_name_type& creator, const string& list_id )const
{ try {
   return get< list_object, by_list_id >( boost::make_tuple( creator, list_id ) );
} FC_CAPTURE_AND_RETHROW( (creator)(list_id) ) }

const list_object* database::find_list( const account_name_type& creator, const string& list_id )const
{
   return find< list_object, by_list_id >( boost::make_tuple( creator, list_id ) );
}

const poll_object& database::get_poll( const account_name_type& creator, const shared_string& poll_id )const
{ try {
   return get< poll_object, by_poll_id >( boost::make_tuple( creator, poll_id ) );
} FC_CAPTURE_AND_RETHROW( (creator)(poll_id) ) }

const poll_object* database::find_poll( const account_name_type& creator, const shared_string& poll_id )const
{
   return find< poll_object, by_poll_id >( boost::make_tuple( creator, poll_id ) );
}

const poll_object& database::get_poll( const account_name_type& creator, const string& poll_id )const
{ try {
   return get< poll_object, by_poll_id >( boost::make_tuple( creator, poll_id ) );
} FC_CAPTURE_AND_RETHROW( (creator)(poll_id) ) }

const poll_object* database::find_poll( const account_name_type& creator, const string& poll_id )const
{
   return find< poll_object, by_poll_id >( boost::make_tuple( creator, poll_id ) );
}

const poll_vote_object& database::get_poll_vote( const account_name_type& voter, const account_name_type& creator, const shared_string& poll_id )const
{ try {
   return get< poll_vote_object, by_voter_creator_poll_id >( boost::make_tuple( voter, creator, poll_id ) );
} FC_CAPTURE_AND_RETHROW( (voter)(creator)(poll_id) ) }

const poll_vote_object* database::find_poll_vote( const account_name_type& voter, const account_name_type& creator, const shared_string& poll_id )const
{
   return find< poll_vote_object, by_voter_creator_poll_id >( boost::make_tuple( voter, creator, poll_id ) );
}

const poll_vote_object& database::get_poll_vote( const account_name_type& voter, const account_name_type& creator, const string& poll_id )const
{ try {
   return get< poll_vote_object, by_voter_creator_poll_id >( boost::make_tuple( voter, creator, poll_id ) );
} FC_CAPTURE_AND_RETHROW( (voter)(creator)(poll_id) ) }

const poll_vote_object* database::find_poll_vote( const account_name_type& voter, const account_name_type& creator, const string& poll_id )const
{
   return find< poll_vote_object, by_voter_creator_poll_id >( boost::make_tuple( voter, creator, poll_id ) );
}

const premium_purchase_object& database::get_premium_purchase( const account_name_type& account, const comment_id_type& id )const
{ try {
   return get< premium_purchase_object, by_account_comment >( boost::make_tuple( account, id ) );
} FC_CAPTURE_AND_RETHROW( (account)(id) ) }

const premium_purchase_object* database::find_premium_purchase( const account_name_type& account, const comment_id_type& id )const
{
   return find< premium_purchase_object, by_account_comment >( boost::make_tuple( account, id ) );
}

const premium_purchase_key_object& database::get_premium_purchase_key( const account_name_type& provider, const account_name_type& account, const comment_id_type& id )const
{ try {
   return get< premium_purchase_key_object, by_provider_account_comment >( boost::make_tuple( provider, account, id ) );
} FC_CAPTURE_AND_RETHROW( (account)(id) ) }

const premium_purchase_key_object* database::find_premium_purchase_key( const account_name_type& provider, const account_name_type& account, const comment_id_type& id )const
{
   return find< premium_purchase_key_object, by_provider_account_comment >( boost::make_tuple( provider, account, id ) );
}

const comment_metrics_object& database::get_comment_metrics()const
{ try {
   return get< comment_metrics_object>();
} FC_CAPTURE_AND_RETHROW() }


struct reward_fund_context
{
   uint128_t       recent_content_claims = 0;

   asset           content_reward_balance;

   asset           reward_distributed;
};


/**
 * Returns the asset value of the comment reward
 * from a specified comment reward context.
 */
asset database::get_comment_reward( const comment_object& c, const util::comment_reward_context& ctx ) const
{ try {
   FC_ASSERT( c.net_reward.value > 0 );
   FC_ASSERT( ctx.recent_content_claims > 0 );
   FC_ASSERT( ctx.total_reward_fund.amount.value > 0 );

   uint256_t rf = util::to256( uint128_t( ctx.total_reward_fund.amount.value ) );
   uint256_t total_claims = util::to256( ctx.recent_content_claims );
   uint128_t reward_curve = util::evaluate_reward_curve( c );
   uint256_t claim = util::to256( reward_curve );

   uint256_t payout_uint256 = ( rf * claim ) / total_claims;
   FC_ASSERT( payout_uint256 <= util::to256( uint128_t( std::numeric_limits<int64_t>::max() ) ) );
   share_type payout = static_cast< int64_t >( payout_uint256 );
   asset reward_value = asset( payout, ctx.total_reward_fund.symbol );

   if( reward_value * ctx.current_COIN_USD_price < MIN_PAYOUT_USD )
   {
      payout = 0;
   }

   asset max_reward_coin = c.max_accepted_payout * ctx.current_COIN_USD_price;
   payout = std::min( payout, share_type( max_reward_coin.amount.value ) );
   reward_value = asset( payout, ctx.total_reward_fund.symbol );

   FC_ASSERT( reward_value.amount <= ctx.total_reward_fund.amount,
      "Reward Value: ${v} is greater than total reward fund: ${f}",
      ("v",reward_value.to_string())("f",ctx.total_reward_fund.to_string()));

   return reward_value;

} FC_CAPTURE_AND_RETHROW( (c)(ctx) ) }

/**
 * Map_Merge takes two flat_maps and combines the keys into
 * a single flat_map, and if there is a key that has values in both maps
 * chooses the one that satisfies a specified comparator:
 * Comp(a,b) = true;
 */
template< class _KEY_TYPE, class _VALUE_TYPE, class _COMP >
flat_map< _KEY_TYPE, _VALUE_TYPE > map_merge( flat_map<_KEY_TYPE,_VALUE_TYPE> map_a, 
   flat_map<_KEY_TYPE,_VALUE_TYPE> map_b, _COMP Comparator )
{
   flat_map< _KEY_TYPE, _VALUE_TYPE > result_map;
   for( auto value_a : map_a )
   {
      if( map_b.count( value_a.first ) > 0 )
      {
         if( Comparator( value_a.second, map_b[ value_a.first ] ) )
         {
            result_map[ value_a.first ] = value_a.second;
         }
         else
         {
            result_map[ value_a.first ] = map_b[ value_a.first ];
         }
      }
      else
      {
         result_map[ value_a.first ] = value_a.second;
      }
   }
   for( auto value_b : map_b )
   {
      result_map[ value_b.first ] = value_b.second;
   }
   return result_map;
}


/**
 * Distributes rewards from a comment to the voters of that comment, according
 * to their vote weight.
 */
asset database::pay_voters( const comment_object& c, const asset& max_rewards )
{ try {
   uint128_t total_weight( c.total_vote_weight );
   asset unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return asset( 0, max_rewards.symbol );
   }
   else if( c.total_vote_weight > 0 )
   {
      const auto& vote_idx = get_index< comment_vote_index >().indices().get< by_comment_weight_voter >();
      auto vote_itr = vote_idx.lower_bound( c.id );

      while( vote_itr != vote_idx.end() && vote_itr->comment == c.id )
      {
         uint128_t weight( vote_itr->weight );
         uint128_t reward_amount = ( max_rewards.amount.value * weight ) / total_weight;
         asset reward = asset( reward_amount.to_uint64(), max_rewards.symbol );

         if( reward.amount > 0 )
         {
            unclaimed_rewards -= reward;
            const account_object& voter = get_account( vote_itr->voter );
            adjust_reward_balance( voter.name, reward );
            push_virtual_operation( vote_reward_operation( voter.name, c.author, to_string( c.permlink ), reward ) );
         }
         ++vote_itr;
      }
   }
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes rewards from a comment to the viewers of that comment, according
 * to their view weight.
 */
asset database::pay_viewers( const comment_object& c, const asset& max_rewards )
{ try {
   uint128_t total_weight( c.total_view_weight );
   asset unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return asset( 0, max_rewards.symbol );
   }
   else if( c.total_view_weight > 0 )
   {
      const auto& view_idx = get_index< comment_view_index >().indices().get< by_comment_weight_viewer >();
      auto view_itr = view_idx.lower_bound( c.id );
      while( view_itr != view_idx.end() && view_itr->comment == c.id )
      {
         uint128_t weight( view_itr->weight );
         uint128_t reward_amount = ( max_rewards.amount.value * weight ) / total_weight;
         asset reward = asset( int64_t( reward_amount.to_uint64() ), max_rewards.symbol );

         if( reward.amount > 0 )
         {
            unclaimed_rewards -= reward;
            const account_object& viewer = get_account( view_itr->viewer );
            adjust_reward_balance( viewer.name, reward );
            push_virtual_operation( view_reward_operation( viewer.name, c.author, to_string( c.permlink ), reward ) );
         }
         ++view_itr;
      }
   }
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes rewards from a comment to the sharers of that comment, according
 * to their share weight.
 */
asset database::pay_sharers( const comment_object& c, const asset& max_rewards )
{ try {
   uint128_t total_weight( c.total_share_weight );
   asset unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return asset( 0, max_rewards.symbol );
   }
   else if( c.total_share_weight > 0 )
   {
      const auto& share_idx = get_index< comment_share_index >().indices().get< by_comment_weight_sharer >();
      auto share_itr = share_idx.lower_bound( c.id );
      while( share_itr != share_idx.end() && share_itr->comment == c.id )
      {
         uint128_t weight( share_itr->weight );
         uint128_t reward_amount = ( max_rewards.amount.value * weight ) / total_weight;
         asset reward = asset( int64_t( reward_amount.to_uint64() ), max_rewards.symbol );

         if( reward.amount > 0 )
         {
            unclaimed_rewards -= reward;
            const account_object& sharer = get_account( share_itr->sharer );
            adjust_reward_balance( sharer.name, reward );
            push_virtual_operation( share_reward_operation( sharer.name, c.author, to_string( c.permlink ), reward ) );
         }
         ++share_itr;
      }
   }
   
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes rewards from a root post to the authors of comments on it,
 * according to their comment weight.
 */
asset database::pay_commenters( const comment_object& c, const asset& max_rewards )
{ try {
   uint128_t total_weight( c.total_comment_weight );
   asset unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return asset( 0, max_rewards.symbol );
   }
   else if( c.total_comment_weight > 0 )
   {
      const auto& comment_idx = get_index< comment_index >().indices().get< by_root >();
      auto comment_itr = comment_idx.lower_bound( boost::make_tuple( c.id, c.id ) );

      while( comment_itr != comment_idx.end() && 
         comment_itr->root_comment == c.id )
      {
         uint128_t weight( comment_itr->weight );
         uint128_t reward_amount = ( max_rewards.amount.value * weight ) / total_weight;
         asset reward = asset( int64_t( reward_amount.to_uint64() ), max_rewards.symbol );
         const comment_object& comment_obj = *comment_itr;

         if( reward.amount > 0 )
         {
            unclaimed_rewards -= reward;
            adjust_reward_balance( comment_obj.author, reward );
            push_virtual_operation( comment_reward_operation( comment_obj.author, to_string( comment_obj.permlink ), c.author, to_string( c.permlink ), reward ) );
         }
         ++comment_itr;
      }
   }
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes rewards from a comment to the supernodes that provided the files
 * for the viewers of that comment, according to their view weight.
 */
asset database::pay_storage( const comment_object& c, const asset& max_rewards )
{ try {
   uint128_t total_weight( c.total_view_weight );
   asset unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return asset( 0, max_rewards.symbol );
   }
   else if( c.total_view_weight > 0 )
   {
      const auto& view_idx = get_index< comment_view_index >().indices().get< by_comment_weight_viewer >();
      auto view_itr = view_idx.lower_bound( c.id );
      while( view_itr != view_idx.end() && view_itr->comment == c.id )
      {
         uint128_t weight( view_itr->weight );
         uint128_t reward_amount = ( max_rewards.amount.value * weight ) / total_weight;
         asset reward = asset( int64_t( reward_amount.to_uint64() ), max_rewards.symbol );

         if( reward.amount > 0 )
         {
            unclaimed_rewards -= reward;
            const supernode_object& supernode = get_supernode( view_itr->supernode );
            adjust_reward_balance( supernode.account, reward );
            push_virtual_operation( supernode_reward_operation( supernode.account, c.author, to_string( c.permlink ), reward ) );
         }
         ++view_itr;
      }
   }
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes rewards from a comment to the moderators of the community it was
 * posted to, according to their voting power.
 */
asset database::pay_moderators( const comment_object& c, const asset& max_rewards )
{ try {
   const community_member_object& community_member = get_community_member( c.community );
   uint128_t total_weight( community_member.total_mod_weight.value );
   asset unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return asset( 0, max_rewards.symbol );
   }
   else if( c.total_view_weight > 0 )
   {
      for( auto mod : community_member.mod_weight )
      {
         uint128_t weight( mod.second.value );
         uint128_t reward_amount = ( max_rewards.amount.value * weight ) / total_weight;
         asset reward = asset( int64_t( reward_amount.to_uint64() ), max_rewards.symbol );

         if( reward.amount > 0 )
         {
            unclaimed_rewards -= reward;
            const account_object& moderator = get_account( mod.first );
            adjust_reward_balance( moderator.name, reward );
            push_virtual_operation( moderation_reward_operation( moderator.name, c.author, to_string( c.permlink ), reward ) );
         }
      }
   }
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes Content rewards to comments once per day
 * for the first 30 days after they are created, and 
 * splits rewards between the author and the voters, viewers, 
 * sharers, supernodes, and moderators.
 */
asset database::distribute_comment_reward( const comment_object& c, util::comment_reward_context& ctx )
{ try {
   asset claimed_reward = asset( 0, c.reward_currency );
   
   time_point now = head_block_time();

   if( c.net_reward > 0 )
   {
      asset content_reward = get_comment_reward( c, ctx );

      if( content_reward.amount > 0 )
      {
         asset voter_tokens =  ( content_reward * c.reward_curve.vote_reward_percent ) / PERCENT_100;
         asset viewer_tokens = ( content_reward * c.reward_curve.view_reward_percent ) / PERCENT_100;
         asset sharer_tokens = ( content_reward * c.reward_curve.share_reward_percent ) / PERCENT_100;
         asset commenter_tokens = ( content_reward * c.reward_curve.comment_reward_percent ) / PERCENT_100;
         asset storage_tokens = ( content_reward * c.reward_curve.storage_reward_percent ) / PERCENT_100;
         asset moderator_tokens = ( content_reward * c.reward_curve.moderator_reward_percent ) / PERCENT_100;
         asset total_curation_tokens = voter_tokens + viewer_tokens + sharer_tokens + commenter_tokens + storage_tokens + moderator_tokens;
         asset author_reward = content_reward - total_curation_tokens;
         author_reward += pay_voters( c, voter_tokens );
         author_reward += pay_viewers( c, viewer_tokens );
         author_reward += pay_sharers( c, sharer_tokens );
         author_reward += pay_commenters( c, commenter_tokens );
         author_reward += pay_storage( c, storage_tokens );
         author_reward += pay_moderators( c, moderator_tokens );
         asset total_beneficiary = asset( 0, c.reward_currency );
         claimed_reward = content_reward;
         
         for( auto b : c.beneficiaries )
         {
            asset beneficiary_reward = ( author_reward * b.weight ) / PERCENT_100;
            adjust_reward_balance( b.account, beneficiary_reward );
            push_virtual_operation( comment_benefactor_reward_operation( b.account, c.author, to_string( c.permlink ), beneficiary_reward ) );
            total_beneficiary += beneficiary_reward;
         }

         author_reward -= total_beneficiary;
         adjust_reward_balance( c.author, author_reward );
         asset claimed_reward_usd = asset_to_USD( content_reward );
         asset total_curation_usd = asset_to_USD( total_curation_tokens );
         asset total_beneficiary_usd = asset_to_USD( total_beneficiary );
         adjust_total_payout( c, claimed_reward_usd, total_curation_usd, total_beneficiary_usd );
         push_virtual_operation( content_reward_operation( c.author, to_string( c.permlink ), claimed_reward_usd ) );
         push_virtual_operation( author_reward_operation( c.author, to_string( c.permlink ), author_reward ) );

         modify( c, [&]( comment_object& com )
         {
            com.content_rewards += content_reward;
         });
      }
   }

   modify( c, [&]( comment_object& com )
   {
      int16_t max_cashouts = c.reward_curve.reward_interval_amount;
      time_point max_cashout_time = c.created + fc::microseconds( c.reward_curve.reward_duration().count() * 2 );

      if( int16_t( com.cashouts_received ) < max_cashouts && now < max_cashout_time )
      {
         if( com.net_reward > 0 )      // A payout is only made for positive reward.
         {
            com.cashouts_received++;
            com.last_payout = now;
         }
         com.cashout_time += c.reward_curve.reward_interval();      // Bump reward interval to next time.
      }
      else
      {
         com.cashout_time = fc::time_point::maximum();
      }
   });

   push_virtual_operation( comment_payout_update_operation( c.author, to_string( c.permlink ) ) );    // Update comment metrics data

   ilog( "Processed Comment Cashout: Author: ${a} Permlink: ${p} Cashouts: ${c} Next Cashout Time: ${t} Reward: ${r}",
      ("a",c.author)("p",c.permlink)("c",c.cashouts_received)("t",c.cashout_time)("r",claimed_reward.to_string()) );
   
   return claimed_reward;

} FC_CAPTURE_AND_RETHROW() }



/**
 * Distributes content rewards to content authors and curators
 * when posts reach a cashout time.
 */
void database::process_comment_cashout()
{
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = props.time;
   
   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   const auto& comment_idx = get_index< comment_index >().indices().get< by_currency_cashout_time >();
   auto comment_itr = comment_idx.begin();

   while( fund_itr != fund_idx.end() )
   {
      const asset_reward_fund_object& reward_fund = *fund_itr;
      ++fund_itr;

      comment_itr = comment_idx.lower_bound( reward_fund.symbol );

      // Find if there is a comment to cashout

      if( comment_itr != comment_idx.end() && 
         comment_itr->cashout_time <= now && 
         comment_itr->reward_currency == reward_fund.symbol )
      {
         util::comment_reward_context ctx;

         if( reward_fund.symbol == SYMBOL_COIN )
         {
            ctx.current_COIN_USD_price = get_liquidity_pool( SYMBOL_COIN, SYMBOL_USD ).day_median_price;
         }
         else
         {
            ctx.current_COIN_USD_price = get_liquidity_pool( SYMBOL_USD, reward_fund.symbol ).day_median_price;
         }

         modify( reward_fund, [&]( asset_reward_fund_object& rfo )      
         {
            rfo.decay_recent_content_claims( now, median_props );
         });

         reward_fund_context rf_ctx;

         rf_ctx.recent_content_claims = reward_fund.recent_content_claims;
         rf_ctx.content_reward_balance = reward_fund.content_reward_balance;
         rf_ctx.reward_distributed = asset( 0, reward_fund.symbol );

         // Snapshots the reward fund into a seperate object reward fund context to ensure equal execution conditions.
         
         while( comment_itr != comment_idx.end() && 
            comment_itr->cashout_time <= now && 
            comment_itr->reward_currency == reward_fund.symbol )
         {
            const comment_object& comment = *comment_itr;
            ++comment_itr;

            if( comment.net_reward > 0 )
            {
               uint128_t reward_curve = util::evaluate_reward_curve( comment );
               rf_ctx.recent_content_claims += reward_curve;
            }
         }

         comment_itr = comment_idx.lower_bound( reward_fund.symbol );

         ctx.recent_content_claims = rf_ctx.recent_content_claims;
         ctx.total_reward_fund = rf_ctx.content_reward_balance;

         // Allocates reward balances to accounts from the comment reward context

         while( comment_itr != comment_idx.end() && 
            comment_itr->cashout_time <= now &&
            comment_itr->reward_currency == reward_fund.symbol )
         {
            const comment_object& comment = *comment_itr;
            ++comment_itr;
            rf_ctx.reward_distributed += distribute_comment_reward( comment, ctx );
         }

         modify( reward_fund, [&]( asset_reward_fund_object& rfo )
         {
            rfo.recent_content_claims = rf_ctx.recent_content_claims;
            rfo.adjust_content_reward_balance( -rf_ctx.reward_distributed );
         });
      }
   }
}


/**
 * Calculates a full suite of metrics for the votes, views, shares and comments
 * of all posts in the network in the prior 30 days. Used for determining sorting 
 * equalization, content rewards, activty reward eligibility and more.
 * Updates every hour.
 * 
 * Adds a new top ranked post to the featured feed for 
 * network wide distribution.
 */
void database::update_comment_metrics() 
{ try {
   if( (head_block_num() % METRIC_INTERVAL_BLOCKS) != 0 )
      return;

   time_point now = head_block_time();
   const comment_metrics_object& comment_metrics = get_comment_metrics();

   uint128_t recent_post_count = 0;        
   uint128_t recent_vote_power = 0;
   uint128_t recent_view_power = 0;
   uint128_t recent_share_power = 0;
   uint128_t recent_comment_power = 0;
   uint128_t average_vote_power = 0;
   uint128_t average_view_power = 0;
   uint128_t average_share_power = 0;
   uint128_t average_comment_power = 0;
   uint128_t median_vote_power = 0;
   uint128_t median_view_power = 0;
   uint128_t median_share_power = 0;
   uint128_t median_comment_power = 0;
   uint128_t recent_vote_count = 0;
   uint128_t recent_view_count = 0;
   uint128_t recent_share_count = 0;
   uint128_t recent_comment_count = 0;
   uint128_t average_vote_count = 0;
   uint128_t average_view_count = 0;
   uint128_t average_share_count = 0;
   uint128_t average_comment_count = 0;
   uint128_t median_vote_count = 0;
   uint128_t median_view_count = 0;
   uint128_t median_share_count = 0;
   uint128_t median_comment_count = 0;
   double vote_view_ratio = 0;
   double vote_share_ratio = 0;
   double vote_comment_ratio = 0;

   vector< const comment_object* > comments; 
   comments.reserve( comment_metrics.recent_post_count.to_uint64() * 2 );

   const auto& comment_idx = get_index< comment_index >().indices().get< by_time >();
   auto comment_itr = comment_idx.lower_bound( true );                                   // Finds first root post in time index
   
   while( comment_itr != comment_idx.end() &&
      ( now < (comment_itr->created + METRIC_CALC_TIME ) ) )        // Iterates over all root posts in last 30 days
   {
      const comment_object& comment = *comment_itr;
      comments.push_back( &comment );                              // Add comment pointer to vector
      recent_post_count++;
      recent_vote_power += comment_itr->vote_power.value;
      recent_view_power += comment_itr->view_power.value;
      recent_share_power += comment_itr->share_power.value;
      recent_comment_power += comment_itr->comment_power.value;
      recent_vote_count += comment_itr->net_votes;
      recent_view_count += comment_itr->view_count;
      recent_share_count += comment_itr->share_count;
      recent_comment_count += comment_itr->children;
      ++comment_itr;
   }

   if( comments.size() )
   {
      // Average Values
      average_vote_power = recent_vote_power / recent_post_count;
      average_view_power = recent_view_power / recent_post_count;
      average_share_power = recent_share_power / recent_post_count;
      average_comment_power = recent_comment_power / recent_post_count;
      average_vote_count = recent_vote_count / recent_post_count;
      average_view_count = recent_view_count / recent_post_count;
      average_share_count = recent_share_count / recent_post_count;
      average_comment_count = recent_comment_count / recent_post_count;

      // Power Ratios
      vote_view_ratio = double( recent_view_power.to_uint64() ) / double( recent_vote_power.to_uint64() );
      vote_share_ratio = double( recent_share_power.to_uint64() ) / double( recent_vote_power.to_uint64() );
      vote_comment_ratio = double( recent_comment_power.to_uint64() ) / double( recent_vote_power.to_uint64() );

      // Median count values
      
      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->net_votes < b->net_votes;
      });
      median_vote_count = comments[comments.size()/2]->net_votes;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->view_count < b->view_count;
      });
      median_view_count = comments[comments.size()/2]->view_count;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->share_count < b->share_count;
      });
      median_share_count = comments[comments.size()/2]->share_count;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->children < b->children;
      } );
      median_comment_count = comments[comments.size()/2]->children;

      // Median Power values

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->vote_power < b->vote_power;
      });
      median_vote_power = comments[comments.size()/2]->vote_power.value;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->view_power < b->view_power;
      });
      median_view_power = comments[comments.size()/2]->view_power.value;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->share_power < b->share_power;
      });
      median_share_power = comments[comments.size()/2]->share_power.value;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->comment_power < b->comment_power;
      });
      median_comment_power = comments[comments.size()/2]->comment_power.value;

      modify( comment_metrics, [&]( comment_metrics_object& cmo )
      {
         cmo.recent_post_count = recent_post_count;        
         cmo.recent_vote_power = recent_vote_power;
         cmo.recent_view_power = recent_view_power;
         cmo.recent_share_power = recent_share_power;
         cmo.recent_comment_power = recent_comment_power;
         cmo.average_vote_power = average_vote_power;
         cmo.average_view_power = average_view_power;
         cmo.average_share_power = average_share_power;
         cmo.average_comment_power = average_comment_power;
         cmo.median_vote_power = median_vote_power;
         cmo.median_view_power = median_view_power;
         cmo.median_share_power = median_share_power;
         cmo.median_comment_power = median_comment_power;
         cmo.recent_vote_count = recent_vote_count;
         cmo.recent_view_count = recent_view_count;
         cmo.recent_share_count = recent_share_count;
         cmo.recent_comment_count = recent_comment_count;
         cmo.average_vote_count = average_vote_count;
         cmo.average_view_count = average_view_count;
         cmo.average_share_count = average_share_count;
         cmo.average_comment_count = average_comment_count;
         cmo.median_vote_count = median_vote_count;
         cmo.median_view_count = median_view_count;
         cmo.median_share_count = median_share_count;
         cmo.median_comment_count = median_comment_count;
         cmo.vote_view_ratio = vote_view_ratio;
         cmo.vote_share_ratio = vote_share_ratio;
         cmo.vote_comment_ratio = vote_comment_ratio;
         cmo.last_updated = now;
      });

      push_virtual_operation( update_featured_feed_operation( now ) );
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Increments the message counter of all connections between accounts if 
 * they have messaged each other in the last 36 hours once per day.
 */
void database::update_message_counter()
{ try {
   if( (head_block_num() % MESSAGE_COUNT_INTERVAL_BLOCKS) != 0 )
      return;

   // ilog( "Update Message Counter" );

   time_point now = head_block_time();

   const auto& account_connection_idx = get_index< account_connection_index >().indices().get< by_last_message_time >();
   auto connection_itr = account_connection_idx.begin();

   while( connection_itr != account_connection_idx.end() && 
      connection_itr->last_message_time() + fc::days(7) >= now )
   {
      if( connection_itr->last_message_time() + fc::hours(36) >= now )
      {
         modify( *connection_itr, [&]( account_connection_object& co )
         {
            co.consecutive_days++;
         });
      }
      else if( connection_itr->consecutive_days > 0 )
      {
         modify( *connection_itr, [&]( account_connection_object& co )
         {
            co.consecutive_days = 0;
         });
      }

      ++connection_itr;
   }
} FC_CAPTURE_AND_RETHROW() }



/** 
 * Adds a newly created post to the authors following, mutual, connection,
 * friend, and companion feeds according to the post's specified reach.
 * Additionally adds the new post to the community feed for the community it is created in
 * and the tag feeds for all the tags it contains. 
 */
void database::add_comment_to_feeds( const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_account_comment_type >();
   const account_following_object& acc_following = get_account_following( comment.author );
   const community_member_object* community_member_ptr = nullptr;

   if( comment.community != community_name_type() )
   {
      community_member_ptr = find_community_member( comment.community );
   }

   const auto& account_comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment_account >();
   auto account_comment_blog_itr = account_comment_blog_idx.find( boost::make_tuple( comment_id, comment.author ) );
   if( account_comment_blog_itr == account_comment_blog_idx.end() )       // Comment is not already in the account's blog
   {
      create< comment_blog_object >( [&]( comment_blog_object& b )
      {
         b.account = comment.author;
         b.comment = comment_id;
         b.blog_type = blog_reach_type::ACCOUNT_BLOG;
         b.blog_time = now; 
         b.shared_by[ comment.author ] = now;
         b.first_shared_by = comment.author;
         b.shares = 1;
      });
   }

   switch( comment.reach )
   {
      case feed_reach_type::NO_FEED:
      {
         return;                                 // Do not share to any feeds. Shows only on account blog.
      }
      case feed_reach_type::COMMUNITY_FEED:     // Encrypted Community feed variants are only shared to community subscribers, and not account followers or connections. 
      case feed_reach_type::COMPANION_FEED:     // Encrypted posts are only shared to connected accounts of the specified level.
      case feed_reach_type::FRIEND_FEED:
      case feed_reach_type::CONNECTION_FEED:
      {
         FC_ASSERT( comment.is_encrypted(), 
            "Post should be encrypted at this reach level." );
      }
      break;
      case feed_reach_type::MUTUAL_FEED:        // Public Posts only from here down.
      case feed_reach_type::FOLLOW_FEED:
      case feed_reach_type::TAG_FEED:           // Tag Feed level posts are shared to tag followers, in addition to account followers. 
      {
         // No encryption required
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid reach selection: ${r}", ("r", comment.reach ) );
      }
   }

   if( community_member_ptr != nullptr )
   {
      const auto& community_comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment_community >();
      auto community_comment_blog_itr = community_comment_blog_idx.find( boost::make_tuple( comment_id, comment.community ) );
      if( community_comment_blog_itr == community_comment_blog_idx.end() )       // Comment is not already in the community's blog
      {
         create< comment_blog_object >( [&]( comment_blog_object& b )
         {
            b.community = comment.community;
            b.comment = comment_id;
            b.blog_type = blog_reach_type::COMMUNITY_BLOG;
            b.blog_time = now; 
            b.shared_by[ comment.author ] = now;
            b.first_shared_by = comment.author;
            b.shares = 1;
         });
      }

      for( const account_name_type& account : community_member_ptr->subscribers )    // Add post to community feeds. 
      {
         auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMMUNITY_FEED ) );
         if( comment_feed_itr == comment_feed_idx.end() )         // Comment is not already in account's communities feed for the type of community. 
         {
            create< comment_feed_object >( [&]( comment_feed_object& f )
            {
               f.account = account;
               f.comment = comment_id;
               f.feed_type = feed_reach_type::COMMUNITY_FEED;
               f.feed_time = now;
               f.communities[ comment.community ][ comment.author ] = now;
               f.shared_by[ comment.author ] = now;
               f.first_shared_by = comment.author;
               f.shares = 1;
            });
         } 
      }

      if( comment.reach == feed_reach_type::COMMUNITY_FEED )
      { 
         return;
      }
   }

   for( const account_name_type& account : acc_following.companions )    // Add to companion feeds
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMPANION_FEED ) );
      if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in companion feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::COMPANION_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == feed_reach_type::COMPANION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.friends ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FRIEND_FEED ) );
      if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in friend feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::FRIEND_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == feed_reach_type::FRIEND_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.connections ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::CONNECTION_FEED ) );
      if( comment_feed_itr == comment_feed_idx.end() )   // Comment is not already in connection feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::CONNECTION_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == feed_reach_type::CONNECTION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.mutual_followers ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::MUTUAL_FEED ) );
      if( comment_feed_itr == comment_feed_idx.end() )   // Comment is not already in mutual feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::MUTUAL_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == feed_reach_type::MUTUAL_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.followers ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FOLLOW_FEED ) );
      if( comment_feed_itr == comment_feed_idx.end() )   // Comment is not already in follow feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::FOLLOW_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }   
   }

   if( comment.reach == feed_reach_type::FOLLOW_FEED )
   { 
      return;
   }

   for( const tag_name_type& comment_tag : comment.tags )    // Add post to tag feeds. Widest possible distribution. 
   {
      const auto& tag_comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment_tag >();
      auto tag_comment_blog_itr = tag_comment_blog_idx.find( boost::make_tuple( comment_id, comment_tag ) );
      if( tag_comment_blog_itr == tag_comment_blog_idx.end() )       // Comment is not already in the tag's blog
      {
         create< comment_blog_object >( [&]( comment_blog_object& b )
         {
            b.tag = comment_tag;
            b.comment = comment_id;
            b.blog_type = blog_reach_type::TAG_BLOG;
            b.blog_time = now; 
            b.shared_by[ comment.author ] = now;
            b.first_shared_by = comment.author;
            b.shares = 1;
         });
      }

      const account_tag_following_object* tag_ptr = find_account_tag_following( comment_tag );
      if( tag_ptr != nullptr )
      {
         for( const account_name_type& account : tag_ptr->followers )   // For all followers of each tag
         {
            auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::TAG_FEED ) );
            if( comment_feed_itr == comment_feed_idx.end() )       // Comment is not already in the account's tag feed
            {
               create< comment_feed_object >( [&]( comment_feed_object& f )
               {
                  f.account = account;
                  f.comment = comment_id;
                  f.feed_type = feed_reach_type::TAG_FEED;
                  f.feed_time = now; 
                  f.tags[ comment_tag ][ comment.author ] = now;
                  f.shared_by[ comment.author ] = now;
                  f.first_shared_by = comment.author;
                  f.shares = 1;
               });
            }    
         } 
      }
   }
} FC_CAPTURE_AND_RETHROW() }


/** 
 * Adds a shared post to the feeds of each of the accounts in its account following object.
 * Create blog and feed objects for sharer account's followers and connections. 
 */
void database::share_comment_to_feeds( const account_name_type& sharer, 
   const feed_reach_type& reach, const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_account_comment_type >();
   const account_following_object& acc_following = get_account_following( sharer );

   switch( reach )
   {
      case feed_reach_type::COMPANION_FEED:
      case feed_reach_type::FRIEND_FEED:
      case feed_reach_type::CONNECTION_FEED:
      case feed_reach_type::MUTUAL_FEED:
      case feed_reach_type::FOLLOW_FEED:
         break;
      default:
      {
         FC_ASSERT( false, "Invalid reach selection. ${r}", ("r", reach ) );
      }
   }

   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment_account >();
   auto comment_blog_itr = comment_blog_idx.find( boost::make_tuple( comment_id, sharer ) );
   if( comment_blog_itr == comment_blog_idx.end() )       // Comment is not already in the account's blog
   {
      create< comment_blog_object >( [&]( comment_blog_object& b )
      {
         b.account = sharer;
         b.comment = comment_id;
         b.blog_type = blog_reach_type::ACCOUNT_BLOG;
         b.blog_time = now; 
         b.shared_by[ sharer ] = now;
         b.first_shared_by = sharer;
         b.shares = 1;
      });
   }
   else      // Comment has already been shared with the account, bump time and increment shares
   {
      modify( *comment_blog_itr, [&]( comment_blog_object& b )
      {
         b.blog_time = now;
         b.shared_by[ sharer ] = now;
         b.shares++;
      });
   }

   for( const account_name_type& account : acc_following.companions ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMPANION_FEED ) );

      if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in companion feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::COMPANION_FEED;
            f.shared_by[ sharer ] = now;
            f.first_shared_by = sharer;
            f.shares = 1;
         });
      }
      else
      {
         modify( *comment_feed_itr, [&]( comment_feed_object& f )
         {
            f.feed_time = now;                      // Bump share time to now when shared again
            f.shared_by[ sharer ] = now;
            f.shares++;
         });
      }
   }

   if( reach == feed_reach_type::COMPANION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.friends ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FRIEND_FEED ) );

      if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in friend feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::FRIEND_FEED;
            f.shared_by[ sharer ] = now;
            f.first_shared_by = sharer;
            f.shares = 1;
         });
      }
      else
      {
         modify( *comment_feed_itr, [&]( comment_feed_object& f )
         {
            f.feed_time = now;                       // Bump share time to now when shared again
            f.shared_by[ sharer ] = now;
            f.shares++;
         });
      }
   }

   if( reach == feed_reach_type::FRIEND_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.connections ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::CONNECTION_FEED ) );

      if( comment_feed_itr == comment_feed_idx.end() )   // Comment is not already in connection feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::CONNECTION_FEED;
            f.shared_by[ sharer ] = now;
            f.first_shared_by = sharer;
            f.shares = 1;
         });
      }
      else
      {
         modify( *comment_feed_itr, [&]( comment_feed_object& f )
         {
            f.feed_time = now;                   // Bump share time to now when shared again
            f.shared_by[ sharer ] = now;
            f.shares++;
         });
      }
   }

   if( reach == feed_reach_type::CONNECTION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.mutual_followers ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::MUTUAL_FEED ) );

      if( comment_feed_itr == comment_feed_idx.end() )   // Comment is not already in mutual feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::MUTUAL_FEED;
            f.shared_by[ sharer ] = now;
            f.first_shared_by = sharer;
            f.shares = 1;
         });
      }
      else
      {
         modify( *comment_feed_itr, [&]( comment_feed_object& f )
         {
            f.feed_time = now;                 // Bump share time to now when shared again
            f.shared_by[ sharer ] = now;
            f.shares++;
         });
      }
   }

   if( reach == feed_reach_type::MUTUAL_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.followers ) 
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FOLLOW_FEED ) );
      if( comment_feed_itr == comment_feed_idx.end() )   // Comment is not already in follow feed
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = feed_reach_type::FOLLOW_FEED;
            f.shared_by[ sharer ] = now;
            f.first_shared_by = sharer;
            f.shares = 1;
         });
      }
      else
      {
         modify( *comment_feed_itr, [&]( comment_feed_object& f )
         {
            f.feed_time = now;                  // Bump share time to now when shared again
            f.shared_by[ sharer ] = now;
            f.shares++;
         });
      }    
   }
} FC_CAPTURE_AND_RETHROW() }


/** 
 * Adds a post to the feeds of each of the accounts in
 * the community's subscriber list. Accounts can share posts 
 * with new communities to increase their reach. 
 */
void database::share_comment_to_community( const account_name_type& sharer, 
   const community_name_type& community, const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const community_member_object& community_member = get_community_member( community );
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_account_comment_type >();
   feed_reach_type feed_type = feed_reach_type::COMMUNITY_FEED;

   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment_community >();
   auto comment_blog_itr = comment_blog_idx.find( boost::make_tuple( comment_id, community ) );
   if( comment_blog_itr == comment_blog_idx.end() )       // Comment is not already in the communities's blog
   {
      create< comment_blog_object >( [&]( comment_blog_object& b )
      {
         b.community = community;
         b.comment = comment_id;
         b.blog_type = blog_reach_type::COMMUNITY_BLOG;
         b.blog_time = now; 
         b.shared_by[ sharer ] = now;
         b.first_shared_by = sharer;
         b.shares = 1;
      });
   }
   else      // Comment has already been shared with community, bump time and increment shares
   {
      modify( *comment_blog_itr, [&]( comment_blog_object& b )
      {
         b.blog_time = now;
         b.shared_by[ sharer ] = now;
         b.shares++;
      });
   }

   for( const account_name_type& account : community_member.subscribers )
   {
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( comment_feed_itr == comment_feed_idx.end() )         // Comment is not already in account's communities feed for the type of community. 
      {
         create< comment_feed_object >( [&]( comment_feed_object& f )
         {
            f.account = account;
            f.comment = comment_id;
            f.feed_type = feed_type;
            f.feed_time = now; 
            f.communities[ community ][ sharer ] = now;
            f.shared_by[ sharer ] = now;
            f.first_shared_by = sharer;
            f.shares = 1;
         });
      }
      else
      {
         modify( *comment_feed_itr, [&]( comment_feed_object& f )
         {
            f.feed_time = now;                        // Bump share time to now when shared again
            f.shared_by[ sharer ] = now;
            f.communities[ community ][ sharer ] = now;
            f.shares++;
         });
      } 
   } 
} FC_CAPTURE_AND_RETHROW() }


/** 
 * Adds a newly created post to the feeds of each of the accounts 
 * that follow the tags specified in the comment. 
 */
void database::share_comment_to_tag( const account_name_type& sharer, 
   const tag_name_type& tag, const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_account_comment_type >();
   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment_tag >();
   auto comment_blog_itr = comment_blog_idx.find( boost::make_tuple( comment_id, tag ) );
   feed_reach_type feed_type = feed_reach_type::TAG_FEED;

   if( comment_blog_itr == comment_blog_idx.end() )       // Comment is not already in the tag's blog
   {
      create< comment_blog_object >( [&]( comment_blog_object& b )
      {
         b.tag = tag;
         b.comment = comment_id;
         b.blog_type = blog_reach_type::TAG_BLOG;
         b.blog_time = now; 
         b.shared_by[ sharer ] = now;
         b.first_shared_by = sharer;
         b.shares = 1;
      });
   }
   else
   {
      modify( *comment_blog_itr, [&]( comment_blog_object& b )
      {
         b.blog_time = now;
         b.shared_by[ sharer ] = now;
         b.shares++;
      });
   }
   
   const account_tag_following_object* tag_ptr = find_account_tag_following( tag );
   if( tag_ptr != nullptr )
   {
      for( const account_name_type& account : tag_ptr->followers )
      {
         auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
         if( comment_feed_itr == comment_feed_idx.end() )       // Comment is not already in the account's tag feed
         {
            create< comment_feed_object >( [&]( comment_feed_object& f )
            {
               f.account = account;
               f.comment = comment_id;
               f.feed_type = feed_type;
               f.feed_time = now; 
               f.tags[ tag ][ sharer ] = now ;
               f.shared_by[ sharer ] = now;
               f.first_shared_by = sharer;
               f.shares = 1;
            });
         }
         else
         {
            modify( *comment_feed_itr , [&]( comment_feed_object& f )
            {
               f.feed_time = now;                     // Bump share time to now when shared again
               f.shared_by[ sharer ] = now;
               f.tags[ tag ][ sharer ] = now;
               f.shares++;
            });
         }    
      } 
   }
} FC_CAPTURE_AND_RETHROW() }

/**
 * Removes a comment from all feeds and blogs for all accounts, tags, and communities. 
 */
void database::clear_comment_feeds( const comment_object& comment )
{ try {
   const comment_id_type& comment_id = comment.id;
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_comment >();
   auto comment_feed_itr = comment_feed_idx.lower_bound( comment_id );

   while( comment_feed_itr != comment_feed_idx.end() && comment_feed_itr->comment == comment_id )
   {
      const comment_feed_object& feed = *comment_feed_itr;
      comment_feed_id_type comment_feed_id = feed.id; 
      remove( feed );
      comment_feed_itr = comment_feed_idx.lower_bound( boost::make_tuple( comment_id, comment_feed_id ) );
   }

   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment >();
   auto comment_blog_itr = comment_blog_idx.lower_bound( comment_id );
   auto blog_end = comment_blog_idx.upper_bound( comment_id );

   while( comment_blog_itr != blog_end )
   {
      const comment_blog_object& blog = *comment_blog_itr;
      comment_blog_id_type comment_blog_id = blog.id; 
      remove( blog );
      comment_blog_itr = comment_blog_idx.lower_bound( boost::make_tuple( comment_id, comment_blog_id ) );
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Removes a comment from all account, tag, and community feeds, and blogs that an account shared it to.
 */
void database::remove_shared_comment( const account_name_type& sharer, const comment_object& comment )
{ try {
   const comment_id_type& comment_id = comment.id;
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_comment >();
   auto comment_feed_itr = comment_feed_idx.lower_bound( comment_id );

   while( comment_feed_itr != comment_feed_idx.end() && comment_feed_itr->comment == comment_id )
   {
      const comment_feed_object& feed = *comment_feed_itr;
      flat_map< account_name_type, time_point > shared_by = feed.shared_by;
      if( shared_by[ sharer ] != time_point() )
      {
         if( feed.shared_by.size() == 1 )     // Remove if Account was only sharer
         {
            comment_feed_id_type comment_feed_id = feed.id; 
            remove( feed );
            comment_feed_itr = comment_feed_idx.lower_bound( boost::make_tuple( comment_id, comment_feed_id ) );
            continue;
         }
         else     // Remove sharer from stats and decrement shares 
         {
            modify( feed, [&]( comment_feed_object& f )
            {
               for( auto tag : f.tags )
               {
                  f.tags[ tag.first ].erase( sharer );
               }
               for( auto community : f.communities )
               {
                  f.communities[ community.first ].erase( sharer );
               }
               f.shared_by.erase( sharer );
               vector< pair< account_name_type, time_point > > shared_by_copy;
               for( auto time : f.shared_by )
               {
                  shared_by_copy.push_back( time );
               }
               std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
               [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
               {
                  return a.second < b.second;
               });

               f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
               f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first; // Earliest time is new first sharer
               f.shares--;
            });
         }
      }
      ++comment_feed_itr;
   }

   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_comment >();
   auto comment_blog_itr = comment_blog_idx.lower_bound( comment_id );

   while( comment_blog_itr != comment_blog_idx.end() && comment_blog_itr->comment == comment_id )
   {
      const comment_blog_object& blog = *comment_blog_itr;
      flat_map< account_name_type, time_point > shared_by = blog.shared_by;
      if( shared_by[ sharer ] != time_point() )
      {
         if( blog.shared_by.size() == 1 )     // Remove if Account was only sharer
         {
            comment_blog_id_type comment_blog_id = blog.id; 
            remove( blog );
            comment_blog_itr = comment_blog_idx.lower_bound( boost::make_tuple( comment_id, comment_blog_id ) );
            continue;
         }
         else     // Remove sharer from stats and decrement shares 
         {
            modify( blog, [&]( comment_blog_object& f )
            {
               f.shared_by.erase( sharer );
               vector< pair < account_name_type, time_point > > shared_by_copy;
               for( auto time : f.shared_by )
               {
                  shared_by_copy.push_back( time );
               }
               std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
               [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
               {
                  return a.second < b.second;
               });

               f.blog_time = shared_by_copy[0].second;       // blog time is the new latest time after removing account. 
               f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;      // Earliest time is new first sharer.
               f.shares--;
            }); 
         }
      }
      ++comment_blog_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Adds or removes Feeds within an account's following 
 * and connection level Feeds for a specified account. 
 */
void database::update_account_in_feed( const account_name_type& account, const account_name_type& followed )
{ try {
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_account_comment_type >();
   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_new_account_blog >();
   const account_following_object& acc_following = get_account_following( account );
   auto comment_blog_itr = comment_blog_idx.lower_bound( followed );
   auto blog_end = comment_blog_idx.upper_bound( followed );

   while( comment_blog_itr != blog_end )
   {
      const comment_id_type& comment_id = comment_blog_itr->comment;
      const comment_object& comment = get( comment_id );
      feed_reach_type reach = comment.reach;
      
      switch( reach )
      {
         case feed_reach_type::COMPANION_FEED:
         {
            auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMPANION_FEED ) );
            if( acc_following.is_companion( followed ) )      // Account is new companion
            {
               if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in account's companion feed 
               {
                  create< comment_feed_object >( [&]( comment_feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = comment_blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::COMPANION_FEED;
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *comment_feed_itr, [&]( comment_feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, comment_blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer companion, Comment is in account's companion feed, and Account was a sharer
            else if( comment_feed_itr != comment_feed_idx.end() ) 
            {
               const comment_feed_object& feed = *comment_feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( comment_feed_object& f )
                  {
                     f.shared_by.erase( followed );
                     vector< pair < account_name_type, time_point > > shared_by_copy;
                     for( auto time : f.shared_by )
                     {
                        shared_by_copy.push_back( time );
                     }
                     std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
                     [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
                     {
                        return a.second < b.second;
                     });

                     f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
                     f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;
                     f.shares--;
                  });
               }  
            }
         }
         break;
         case feed_reach_type::FRIEND_FEED:
         {
            auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FRIEND_FEED ) );
            if( acc_following.is_friend( followed ) )      // Account is new friend
            {
               if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in account's friend feed 
               {
                  create< comment_feed_object >( [&]( comment_feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = comment_blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::FRIEND_FEED;
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *comment_feed_itr, [&]( comment_feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, comment_blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer friend, Comment is in account's friend feed, and Account was a sharer
            else if( comment_feed_itr != comment_feed_idx.end() ) 
            {
               const comment_feed_object& feed = *comment_feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( comment_feed_object& f )
                  {
                     f.shared_by.erase( followed );
                     vector< pair < account_name_type, time_point > > shared_by_copy;
                     for( auto time : f.shared_by )
                     {
                        shared_by_copy.push_back( time );
                     }
                     std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
                     [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
                     {
                        return a.second < b.second;
                     });

                     f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
                     f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;
                     f.shares--;
                  });
               }  
            }
         }
         break;
         case feed_reach_type::CONNECTION_FEED:
         {
            auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::CONNECTION_FEED ) );
            if( acc_following.is_connection( followed ) )      // Account is new connection
            {
               if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in account's connection feed 
               {
                  create< comment_feed_object >( [&]( comment_feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = comment_blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::CONNECTION_FEED;
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *comment_feed_itr, [&]( comment_feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, comment_blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer connection, Comment is in account's connection feed, and Account was a sharer
            else if( comment_feed_itr != comment_feed_idx.end() ) 
            {
               const comment_feed_object& feed = *comment_feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( comment_feed_object& f )
                  {
                     f.shared_by.erase( followed );
                     vector< pair < account_name_type, time_point > > shared_by_copy;
                     for( auto time : f.shared_by )
                     {
                        shared_by_copy.push_back( time );
                     }
                     std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
                     [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
                     {
                        return a.second < b.second;
                     });

                     f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
                     f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;
                     f.shares--;
                  });
               }  
            }
         }
         break;
         case feed_reach_type::MUTUAL_FEED:
         {
            auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::MUTUAL_FEED ) );
            if( acc_following.is_mutual( followed ) )      // Account is new mutual
            {
               if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in account's mutual feed 
               {
                  create< comment_feed_object >( [&]( comment_feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = comment_blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::MUTUAL_FEED;
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *comment_feed_itr, [&]( comment_feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, comment_blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer mutual, Comment is in account's mutual feed, and Account was a sharer
            else if( comment_feed_itr != comment_feed_idx.end() ) 
            {
               const comment_feed_object& feed = *comment_feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( comment_feed_object& f )
                  {
                     f.shared_by.erase( followed );
                     vector< pair < account_name_type, time_point > > shared_by_copy;
                     for( auto time : f.shared_by )
                     {
                        shared_by_copy.push_back( time );
                     }
                     std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
                     [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
                     {
                        return a.second < b.second;
                     });

                     f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
                     f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;
                     f.shares--;
                  });
               }  
            }
         }
         break;
         case feed_reach_type::FOLLOW_FEED:
         {
            auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FOLLOW_FEED ) );
            if( acc_following.is_following( followed ) )      // Account is new follow
            {
               if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in account's follow feed 
               {
                  create< comment_feed_object >( [&]( comment_feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = comment_blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::FOLLOW_FEED;
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *comment_feed_itr, [&]( comment_feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, comment_blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = comment_blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer follow, Comment is in account's follow feed, and Account was a sharer
            else if( comment_feed_itr != comment_feed_idx.end() ) 
            {
               const comment_feed_object& feed = *comment_feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( comment_feed_object& f )
                  {
                     f.shared_by.erase( followed );
                     vector< pair < account_name_type, time_point > > shared_by_copy;
                     for( auto time : f.shared_by )
                     {
                        shared_by_copy.push_back( time );
                     }
                     std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
                     [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b )
                     {
                        return a.second < b.second;
                     });

                     f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
                     f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;
                     f.shares--;
                  });
               }  
            }
         }
         break;
         default:
         break;
      }

      ++comment_blog_itr;
   }

} FC_CAPTURE_AND_RETHROW() }


/**
 * Adds or removes Feeds within an account's Community Feed variants for a 
 * specified community. 
 */
void database::update_community_in_feed( const account_name_type& account, const community_name_type& community )
{ try {
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_account_comment_type >();
   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_new_community_blog >();
   const account_following_object& acc_following = get_account_following( account );
   auto comment_blog_itr = comment_blog_idx.lower_bound( community );
   auto blog_end = comment_blog_idx.upper_bound( community );
   feed_reach_type feed_type = feed_reach_type::COMMUNITY_FEED;

   while( comment_blog_itr != blog_end )
   {
      const comment_id_type& comment_id = comment_blog_itr->comment;
      
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( acc_following.is_followed_community( community ) )
      {
         if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in account's community feed 
         {
            create< comment_feed_object >( [&]( comment_feed_object& f )
            {
               f.account = account;
               f.feed_time = comment_blog_itr->blog_time;
               f.comment = comment_id;
               f.feed_type = feed_type;
               f.shared_by = comment_blog_itr->shared_by;
               f.first_shared_by = comment_blog_itr->first_shared_by;
               f.shares = comment_blog_itr->shares;
            });
         }
         else
         {
            modify( *comment_feed_itr, [&]( comment_feed_object& f )
            {
               f.feed_time = std::max( f.feed_time, comment_blog_itr->blog_time );     // Bump feed time if blog is later
               f.shared_by = map_merge( f.shared_by, comment_blog_itr->shared_by, 
               [&]( time_point a, time_point b )
               {
                  return a > b;    // Take the latest time point of all sharing accounts
               });
               f.shares = f.shared_by.size();
            });
         }
      } // Account is no longer following community, Comment is in account's community feed
      else if( comment_feed_itr != comment_feed_idx.end() ) 
      {
         const comment_feed_object& feed = *comment_feed_itr;
         flat_map< community_name_type, flat_map< account_name_type, time_point > > communities = feed.communities;
         if( comment_feed_itr->communities.size() == 1 )      // This community was the only community it was shared with.
         {
            remove( feed );        // Remove the entire feed
         }
         else if( communities[ community ].size() != 0 )    // remove community from sharing metrics
         {
            modify( feed, [&]( comment_feed_object& f )
            {
               for( auto community_value : f.communities[ community ] )
               {
                  f.shared_by.erase( community_value.first );   // Remove all shares from this community
               }
               f.communities.erase( community );
               vector< pair < account_name_type, time_point > > shared_by_copy;
               for( auto time : f.shared_by )
               {
                  shared_by_copy.push_back( time );
               }
               std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
               [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
               {
                  return a.second < b.second;
               });

               f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
               f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;
               f.shares--;
            });
         }  
      }
      ++comment_blog_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Adds or removes Feeds within an account's Tag Feed for a 
 * specified tag. 
 */
void database::update_tag_in_feed( const account_name_type& account, const tag_name_type& tag )
{ try {
   const auto& comment_feed_idx = get_index< comment_feed_index >().indices().get< by_account_comment_type >();
   const auto& comment_blog_idx = get_index< comment_blog_index >().indices().get< by_new_tag_blog >();
   auto comment_blog_itr = comment_blog_idx.lower_bound( tag );
   auto blog_end = comment_blog_idx.upper_bound( tag );
   const account_following_object& acc_following = get_account_following( account );
   feed_reach_type feed_type = feed_reach_type::COMMUNITY_FEED;

   while( comment_blog_itr != blog_end )
   {
      const comment_id_type& comment_id = comment_blog_itr->comment;
      
      auto comment_feed_itr = comment_feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( acc_following.is_followed_tag( tag ) )
      {
         if( comment_feed_itr == comment_feed_idx.end() )      // Comment is not already in account's tag feed 
         {
            create< comment_feed_object >( [&]( comment_feed_object& f )
            {
               f.account = account;
               f.feed_time = comment_blog_itr->blog_time;
               f.comment = comment_id;
               f.feed_type = feed_type;
               f.shared_by = comment_blog_itr->shared_by;
               f.first_shared_by = comment_blog_itr->first_shared_by;
               f.shares = comment_blog_itr->shares;
            });
         }
         else
         {
            modify( *comment_feed_itr, [&]( comment_feed_object& f )
            {
               f.feed_time = std::max( f.feed_time, comment_blog_itr->blog_time );     // Bump feed time if blog is later
               f.shared_by = map_merge( f.shared_by, comment_blog_itr->shared_by, [&]( time_point a, time_point b )
                  {
                     return a > b;    // Take the latest time point of all sharing accounts
                  });
               f.shares = f.shared_by.size();
            });
         }
      } // Account is no longer following tag, Comment is in account's tag feed
      else if( comment_feed_itr != comment_feed_idx.end() ) 
      {
         const comment_feed_object& feed = *comment_feed_itr;
         flat_map< tag_name_type, flat_map< account_name_type, time_point > > tags = feed.tags;
         if( comment_feed_itr->tags.size() == 1 )      // This Tag was the only tag it was shared with.
         {
            remove( feed );    // Remove the entire feed
         }
         else if( tags[ tag ].size() != 0 )     // remove tag from sharing metrics
         {
            modify( feed, [&]( comment_feed_object& f )
            {
               for( auto tag_value : f.tags[ tag ] )
               {
                  f.shared_by.erase( tag_value.first );   // Remove all shares from this tag
               }
               f.tags.erase( tag );
               vector< pair < account_name_type, time_point > > shared_by_copy;
               for( auto time : f.shared_by )
               {
                  shared_by_copy.push_back( time );
               }
               std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
               [&]( pair < account_name_type, time_point > a, pair < account_name_type, time_point > b)
               {
                  return a.second < b.second;
               });

               f.feed_time = shared_by_copy[0].second;  // Feed time is the new latest time after removing account. 
               f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first;
               f.shares--;
            });
         }  
      }
      ++comment_blog_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


void database::adjust_total_payout( const comment_object& cur, const asset& reward_created, const asset& curator_reward_value, const asset& beneficiary_reward_value )
{
   modify( cur, [&]( comment_object& c )
   {
      if( c.total_payout_value.symbol == reward_created.symbol )
      {
         c.total_payout_value += reward_created;
         c.curator_payout_value += curator_reward_value;
         c.beneficiary_payout_value += beneficiary_reward_value;
      }
   });
}


void database::deliver_premium_purchase( const premium_purchase_object& purchase, const account_name_type& interface, const account_name_type& supernode )
{ try {
   FC_ASSERT( purchase.released, 
      "Purchase must have been released before delivering" );

   const comment_object& comment = get( purchase.comment );

   asset purchase_amount = purchase.premium_price;
   asset total_fees = ( purchase_amount * PREMIUM_FEE_PERCENT ) / PERCENT_100;
   asset supernode_share = ( total_fees * NODE_PREMIUM_FEE_PERCENT ) / PERCENT_100;
   asset interface_share = ( total_fees * INTERFACE_PREMIUM_FEE_PERCENT ) / PERCENT_100;
   asset network_fee = ( total_fees * NETWORK_PREMIUM_FEE_PERCENT ) / PERCENT_100;

   const account_object& author_account = get_account( comment.author );
   const account_object& supernode_account = get_account( supernode );
   const account_object& interface_account = get_account( interface );
   
   asset supernode_paid = pay_fee_share( supernode_account, supernode_share, true );
   asset interface_paid = pay_fee_share( interface_account, interface_share, true );
   pay_network_fees( author_account, network_fee );

   asset fees_paid = supernode_paid + interface_paid + network_fee;
   asset premium_paid = purchase_amount - fees_paid;

   adjust_reward_balance( comment.author, premium_paid );
   adjust_pending_supply( -purchase_amount );

   ilog( "Account: ${a} paid advertising delivery: ${v}",
      ("a",purchase.account)("v",purchase_amount.to_string()));

   remove( purchase );

} FC_CAPTURE_AND_RETHROW() }


void database::cancel_premium_purchase( const premium_purchase_object& purchase )
{ try {
   asset purchase_amount = purchase.premium_price;
   asset network_fee = ( purchase_amount * PREMIUM_FEE_PERCENT ) / PERCENT_100;
   const account_object& purchaser_account = get_account( purchase.account );
   pay_network_fees( purchaser_account, network_fee );
   asset premium_refunded = purchase_amount - network_fee;
   adjust_liquid_balance( purchase.account, premium_refunded );
   adjust_pending_supply( -purchase_amount );

   ilog( "Account: ${a} cancelled premium purchase: ${v}",
      ("a",purchase.account)("v",purchase_amount.to_string()));

   remove( purchase );

} FC_CAPTURE_AND_RETHROW() }


} } //node::chain