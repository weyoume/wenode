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

using fc::flat_set;
using fc::flat_map;

struct reward_fund_context
{
   uint128_t       recent_content_claims;

   asset           content_reward_balance;

   asset           reward_distributed;
};

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
         asset reward = asset( int64_t( reward_amount.to_uint64() ), max_rewards.symbol );

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
      auto comment_itr = comment_idx.lower_bound( c.id );
      while( comment_itr != comment_idx.end() && comment_itr->root_comment == c.id )
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
asset database::distribute_comment_reward( util::comment_reward_context& ctx, const comment_object& c )
{ try {
   asset claimed_reward = asset( 0, c.reward_currency );
   time_point now = head_block_time();

   if( c.net_reward > 0 )
   {
      util::fill_comment_reward_context_local_state( ctx, c );
      asset reward_tokens = util::get_comment_reward( ctx );

      if( reward_tokens.amount > 0 )
      {
         asset voter_tokens =  ( reward_tokens * c.vote_reward_percent ) / PERCENT_100;
         asset viewer_tokens = ( reward_tokens * c.view_reward_percent ) / PERCENT_100;
         asset sharer_tokens = ( reward_tokens * c.share_reward_percent ) / PERCENT_100;
         asset commenter_tokens = ( reward_tokens * c.comment_reward_percent ) / PERCENT_100;
         asset storage_tokens = ( reward_tokens * c.storage_reward_percent ) / PERCENT_100;
         asset moderator_tokens = ( reward_tokens * c.moderator_reward_percent ) / PERCENT_100;
         asset total_curation_tokens = voter_tokens + viewer_tokens + sharer_tokens + commenter_tokens + storage_tokens + moderator_tokens;
         asset author_tokens = reward_tokens - total_curation_tokens;

         author_tokens += pay_voters( c, voter_tokens );
         author_tokens += pay_viewers( c, viewer_tokens );
         author_tokens += pay_sharers( c, sharer_tokens );
         author_tokens += pay_commenters( c, commenter_tokens );
         author_tokens += pay_storage( c, storage_tokens );
         author_tokens += pay_moderators( c, moderator_tokens );

         asset total_beneficiary = asset( 0, c.reward_currency );
         claimed_reward = author_tokens + total_curation_tokens;

         for( auto b : c.beneficiaries )
         {
            asset benefactor_tokens = ( author_tokens * b.weight ) / PERCENT_100;
            asset reward_created = benefactor_tokens;
            adjust_reward_balance( b.account, reward_created );
            push_virtual_operation( 
               comment_benefactor_reward_operation( b.account, c.author, to_string( c.permlink ), reward_created ) );
            total_beneficiary += benefactor_tokens;
         }

         author_tokens -= total_beneficiary;
         asset author_reward = author_tokens;
         adjust_reward_balance( c.author, author_reward );

         asset claimed_reward_usd = asset_to_USD( claimed_reward );
         asset total_curation_usd = asset_to_USD( total_curation_tokens );
         asset total_beneficiary_usd = asset_to_USD( total_beneficiary );

         adjust_total_payout( c, claimed_reward_usd, total_curation_usd, total_beneficiary_usd );

         push_virtual_operation( content_reward_operation( c.author, to_string( c.permlink ), claimed_reward_usd ) );
         push_virtual_operation( author_reward_operation( c.author, to_string( c.permlink ), author_reward ) );

         modify( c, [&]( comment_object& com )
         {
            com.content_rewards += claimed_reward;
         });
      }
   }

   modify( c, [&]( comment_object& com )
   {
      if( com.cashouts_received < ( ctx.decay_rate.to_seconds() / ctx.reward_interval.to_seconds() ) )
      {
         if( com.net_reward > 0 )   // A payout is only made for positive reward.
         {
            com.cashouts_received++;
            com.last_payout = now;
         }
         com.cashout_time += ctx.reward_interval;     // Bump reward interval to next time
      }
      else
      {
         com.cashout_time = fc::time_point::maximum();
      }
   });

   push_virtual_operation( comment_payout_update_operation( c.author, to_string( c.permlink ) ) );    // Update comment metrics data
   
   return claimed_reward;
} FC_CAPTURE_AND_RETHROW() }


util::comment_reward_context database::get_comment_reward_context( const reward_fund_object& reward_fund )
{
   util::comment_reward_context ctx;

   ctx.current_COIN_USD_price = get_liquidity_pool( reward_fund.symbol, SYMBOL_USD ).day_median_price;
   ctx.decay_rate = reward_fund.content_reward_decay_rate;
   ctx.reward_interval = reward_fund.content_reward_interval;
   ctx.reward_curve = reward_fund.author_reward_curve;
   ctx.content_constant = reward_fund.content_constant;
   return ctx;
}


/**
 * Distributes content rewards to content authors and curators
 * when posts reach a cashout time.
 */
void database::process_comment_cashout()
{
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   vector< asset > reward_distributed;

   const auto& fund_idx = get_index< reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   while( fund_itr != fund_idx.end() )
   {
      const reward_fund_object& reward_fund = *fund_itr;
      util::comment_reward_context ctx = get_comment_reward_context( reward_fund );

      // Decay recent reward of post reward fund

      modify( reward_fund, [&]( reward_fund_object& rfo )
      {
         rfo.recent_content_claims -= ( rfo.recent_content_claims * ( now - rfo.last_updated ).to_seconds() ) / ctx.decay_rate.to_seconds();
         rfo.last_updated = now;
      });

      // Snapshots the reward fund into a seperate object reward fund context to ensure equal execution conditions.

      reward_fund_context rf_ctx;
      rf_ctx.recent_content_claims = reward_fund.recent_content_claims;
      rf_ctx.content_reward_balance = reward_fund.content_reward_balance;

      const auto& comment_idx = get_index< comment_index >().indices().get< by_currency_cashout_time >();
      auto comment_itr = comment_idx.lower_bound( reward_fund.symbol );
      
      while( comment_itr != comment_idx.end() && 
         comment_itr->cashout_time <= now && 
         comment_itr->reward_currency == reward_fund.symbol )
      {
         if( comment_itr->net_reward > 0 )
         {
            uint128_t net_reward = uint128_t( comment_itr->net_reward.value );
            uint128_t reward_curve = util::evaluate_reward_curve( 
               net_reward, comment_itr->cashouts_received, ctx.reward_curve, ctx.decay_rate, ctx.content_constant );
            rf_ctx.recent_content_claims += reward_curve;
         }
         ++comment_itr;
      }

      comment_itr = comment_idx.lower_bound( reward_fund.symbol );

      ctx.recent_content_claims = rf_ctx.recent_content_claims;
      ctx.total_reward_fund = rf_ctx.content_reward_balance;

      // Allocates reward balances to accounts from the comment reward context

      while( comment_itr != comment_idx.end() && 
         comment_itr->cashout_time <= now &&
         comment_itr->reward_currency == reward_fund.symbol )
      {
         rf_ctx.reward_distributed += distribute_comment_reward( ctx, *comment_itr );
         ++comment_itr;
      }

      modify( reward_fund, [&]( reward_fund_object& rfo )
      {
         rfo.recent_content_claims = rf_ctx.recent_content_claims;
         rfo.content_reward_balance.amount -= rf_ctx.reward_distributed;
      });

      ++fund_itr;
   }
}


/**
 * Calculates a full suite of metrics for the votes, views, shares and comments
 * of all posts in the network in the prior 30 days. Used for determining sorting 
 * equalization, content rewards, activty reward eligibility and more.
 * Updates every hour.
 */
void database::update_comment_metrics() 
{ try {
   if( (head_block_num() % METRIC_INTERVAL_BLOCKS) != 0 )
      return;

   time_point now = head_block_time();
   const comment_metrics_object& comment_metrics = get_comment_metrics();
   
   // Initialize comment metrics

   uint32_t recent_post_count = 0;        
   share_type recent_vote_power = 0;
   share_type recent_view_power = 0;
   share_type recent_share_power = 0;
   share_type recent_comment_power = 0;
   share_type average_vote_power = 0;
   share_type average_view_power = 0;
   share_type average_share_power = 0;
   share_type average_comment_power = 0;
   share_type median_vote_power = 0;
   share_type median_view_power = 0;
   share_type median_share_power = 0;
   share_type median_comment_power = 0;
   uint32_t recent_vote_count = 0;
   uint32_t recent_view_count = 0;
   uint32_t recent_share_count = 0;
   uint32_t recent_comment_count = 0;
   uint32_t average_vote_count = 0;
   uint32_t average_view_count = 0;
   uint32_t average_share_count = 0;
   uint32_t average_comment_count = 0;
   uint32_t median_vote_count = 0;
   uint32_t median_view_count = 0;
   uint32_t median_share_count = 0;
   uint32_t median_comment_count = 0;
   double vote_view_ratio = 0;
   double vote_share_ratio = 0;
   double vote_comment_ratio = 0;

   vector< const comment_object* > comments; 
   comments.reserve( comment_metrics.recent_post_count * 2 );

   const auto& comment_idx = get_index< comment_index >().indices().get< by_time >();
   auto comment_itr = comment_idx.lower_bound( true );                                   // Finds first root post in time index
   
   while( comment_itr != comment_idx.end() && (now < (comment_itr->created + METRIC_CALC_TIME) ) ) // Iterates over all root posts in last 30 days
   {
      const comment_object& comment = *comment_itr;
      comments.push_back( &comment );                          // Add comment pointer to vector
      recent_post_count++;
      recent_vote_power += comment_itr->vote_power;
      recent_view_power += comment_itr->view_power;
      recent_share_power += comment_itr->share_power;
      recent_comment_power += comment_itr->comment_power;
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
      vote_view_ratio = double( recent_view_power.value ) / double( recent_vote_power.value );
      vote_share_ratio = double( recent_share_power.value ) / double( recent_vote_power.value );
      vote_comment_ratio = double( recent_comment_power.value ) / double( recent_vote_power.value );

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
      median_vote_power = comments[comments.size()/2]->vote_power;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->view_power < b->view_power;
      });
      median_view_power = comments[comments.size()/2]->view_power;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->share_power < b->share_power;
      });
      median_share_power = comments[comments.size()/2]->share_power;

      std::sort( comments.begin(), comments.end(), [&]( const comment_object* a, const comment_object* b )
      {
         return a->comment_power < b->comment_power;
      });
      median_comment_power = comments[comments.size()/2]->comment_power;

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
      });
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
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const account_following_object& acc_following = get_account_following( comment.author );
   const community_member_object* community_member_ptr = nullptr;

   if( comment.community != community_name_type() )
   {
      community_member_ptr = find_community_member( comment.community );
   }

   const auto& account_blog_idx = get_index< blog_index >().indices().get< by_comment_account >();
   auto account_blog_itr = account_blog_idx.find( boost::make_tuple( comment_id, comment.author ) );
   if( account_blog_itr == account_blog_idx.end() )       // Comment is not already in the account's blog
   {
      create< blog_object >( [&]( blog_object& b )
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
         return;               // Do not share to any feeds. Shows only on account blog. 
      }
      case feed_reach_type::COMMUNITY_FEED:     // Encrypted Community feed variants are only shared to community subscribers, and not account followers or connections. 
      case feed_reach_type::COMPANION_FEED:     // Encrypted posts are only shared to connected accounts of the specified level.
      case feed_reach_type::FRIEND_FEED:
      case feed_reach_type::CONNECTION_FEED:
      {
         FC_ASSERT( comment.is_encrypted(), 
            "Post should be encrypted at this reach level." );
      }
      case feed_reach_type::MUTUAL_FEED:        // Public Posts only from here down
      case feed_reach_type::FOLLOW_FEED:
      case feed_reach_type::TAG_FEED:           // Tag Feed level posts are shared to tag followers, in addition to account followers. 
      {
         FC_ASSERT( !comment.is_encrypted(), 
            "Post should not encrypted at this reach level." );
      }
      default:
      {
         FC_ASSERT( false, "Invalid reach selection. ");
      }
   }

   if( community_member_ptr != nullptr )
   {
      const auto& community_blog_idx = get_index< blog_index >().indices().get< by_comment_community >();
      auto community_blog_itr = community_blog_idx.find( boost::make_tuple( comment_id, comment.community ) );
      if( community_blog_itr == community_blog_idx.end() )       // Comment is not already in the community's blog
      {
         create< blog_object >( [&]( blog_object& b )
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
         auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMMUNITY_FEED ) );
         if( feed_itr == feed_idx.end() )         // Comment is not already in account's communities feed for the type of community. 
         {
            create< feed_object >( [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMPANION_FEED ) );
      if( feed_itr == feed_idx.end() )      // Comment is not already in companion feed
      {
         create< feed_object >( [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FRIEND_FEED ) );
      if( feed_itr == feed_idx.end() )      // Comment is not already in friend feed
      {
         create< feed_object >( [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::CONNECTION_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in connection feed
      {
         create< feed_object >( [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::MUTUAL_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in mutual feed
      {
         create< feed_object >( [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FOLLOW_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in follow feed
      {
         create< feed_object >( [&]( feed_object& f )
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
      const auto& tag_blog_idx = get_index< blog_index >().indices().get< by_comment_tag >();
      auto tag_blog_itr = tag_blog_idx.find( boost::make_tuple( comment_id, comment_tag ) );
      if( tag_blog_itr == tag_blog_idx.end() )       // Comment is not already in the tag's blog
      {
         create< blog_object >( [&]( blog_object& b )
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

      const tag_following_object* tag_ptr = find_tag_following( comment_tag );
      if( tag_ptr != nullptr )
      {
         for( const account_name_type& account : tag_ptr->followers )   // For all followers of each tag
         {
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::TAG_FEED ) );
            if( feed_itr == feed_idx.end() )       // Comment is not already in the account's tag feed
            {
               create< feed_object >( [&]( feed_object& f )
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
 * Adds a shared post to the feeds of each of the accounts in its
 * account following object.
 */
void database::share_comment_to_feeds( const account_name_type& sharer, 
   const feed_reach_type& reach, const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
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
         FC_ASSERT( false, "Invalid reach selection. ");
      }
   }

   const auto& blog_idx = get_index< blog_index >().indices().get< by_comment_account >();
   auto blog_itr = blog_idx.find( boost::make_tuple( comment_id, sharer ) );
   if( blog_itr == blog_idx.end() )       // Comment is not already in the account's blog
   {
      create< blog_object >( [&]( blog_object& b )
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
      modify( *blog_itr, [&]( blog_object& b )
      {
         b.blog_time = now;
         b.shared_by[ sharer ] = now;
         b.shares++;
      });
   }

   for( const account_name_type& account : acc_following.companions ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMPANION_FEED ) );

      if( feed_itr == feed_idx.end() )      // Comment is not already in companion feed
      {
         create< feed_object >( [&]( feed_object& f )
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
         modify( *feed_itr, [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FRIEND_FEED ) );

      if( feed_itr == feed_idx.end() )      // Comment is not already in friend feed
      {
         create< feed_object >( [&]( feed_object& f )
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
         modify( *feed_itr, [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::CONNECTION_FEED ) );

      if( feed_itr == feed_idx.end() )   // Comment is not already in connection feed
      {
         create< feed_object >( [&]( feed_object& f )
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
         modify( *feed_itr, [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::MUTUAL_FEED ) );

      if( feed_itr == feed_idx.end() )   // Comment is not already in mutual feed
      {
         create< feed_object >( [&]( feed_object& f )
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
         modify( *feed_itr, [&]( feed_object& f )
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FOLLOW_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in follow feed
      {
         create< feed_object >( [&]( feed_object& f )
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
         modify( *feed_itr, [&]( feed_object& f )
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
 * with new communities to increase thier reach. 
 */
void database::share_comment_to_community( const account_name_type& sharer, 
   const community_name_type& community, const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const community_member_object& community_member = get_community_member( community );
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   feed_reach_type feed_type = feed_reach_type::COMMUNITY_FEED;

   const auto& blog_idx = get_index< blog_index >().indices().get< by_comment_community >();
   auto blog_itr = blog_idx.find( boost::make_tuple( comment_id, community ) );
   if( blog_itr == blog_idx.end() )       // Comment is not already in the communities's blog
   {
      create< blog_object >( [&]( blog_object& b )
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
      modify( *blog_itr, [&]( blog_object& b )
      {
         b.blog_time = now;
         b.shared_by[ sharer ] = now;
         b.shares++;
      });
   }

   for( const account_name_type& account : community_member.subscribers )
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( feed_itr == feed_idx.end() )         // Comment is not already in account's communities feed for the type of community. 
      {
         create< feed_object >( [&]( feed_object& f )
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
         modify( *feed_itr, [&]( feed_object& f )
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
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const auto& blog_idx = get_index< blog_index >().indices().get< by_comment_tag >();
   auto blog_itr = blog_idx.find( boost::make_tuple( comment_id, tag ) );
   feed_reach_type feed_type = feed_reach_type::TAG_FEED;

   if( blog_itr == blog_idx.end() )       // Comment is not already in the tag's blog
   {
      create< blog_object >( [&]( blog_object& b )
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
      modify( *blog_itr, [&]( blog_object& b )
      {
         b.blog_time = now;
         b.shared_by[ sharer ] = now;
         b.shares++;
      });
   }
   
   const tag_following_object* tag_ptr = find_tag_following( tag );
   if( tag_ptr != nullptr )
   {
      for( const account_name_type& account : tag_ptr->followers )
      {
         auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
         if( feed_itr == feed_idx.end() )       // Comment is not already in the account's tag feed
         {
            create< feed_object >( [&]( feed_object& f )
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
            modify( *feed_itr , [&]( feed_object& f )
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
   const auto& feed_idx = get_index< feed_index >().indices().get< by_comment >();
   auto feed_itr = feed_idx.lower_bound( comment_id );

   while( feed_itr != feed_idx.end() && feed_itr->comment == comment_id )
   {
      const feed_object& feed = *feed_itr;
      feed_id_type feed_id = feed.id; 
      remove( feed );
      feed_itr = feed_idx.lower_bound( boost::make_tuple( comment_id, feed_id ) );
   }

   const auto& blog_idx = get_index< blog_index >().indices().get< by_comment >();
   auto blog_itr = blog_idx.lower_bound( comment_id );
   auto blog_end = blog_idx.upper_bound( comment_id );

   while( blog_itr != blog_end )
   {
      const blog_object& blog = *blog_itr;
      blog_id_type blog_id = blog.id; 
      remove( blog );
      blog_itr = blog_idx.lower_bound( boost::make_tuple( comment_id, blog_id ) );
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Removes a comment from all account, tag, and community feeds, and blogs that an account shared it to.
 */
void database::remove_shared_comment( const account_name_type& sharer, const comment_object& comment )
{ try {
   const comment_id_type& comment_id = comment.id;
   const auto& feed_idx = get_index< feed_index >().indices().get< by_comment >();
   auto feed_itr = feed_idx.lower_bound( comment_id );

   while( feed_itr != feed_idx.end() && feed_itr->comment == comment_id )
   {
      const feed_object& feed = *feed_itr;
      flat_map< account_name_type, time_point > shared_by = feed.shared_by;
      if( shared_by[ sharer ] != time_point() )
      {
         if( feed.shared_by.size() == 1 )     // Remove if Account was only sharer
         {
            feed_id_type feed_id = feed.id; 
            remove( feed );
            feed_itr = feed_idx.lower_bound( boost::make_tuple( comment_id, feed_id ) );
            continue;
         }
         else     // Remove sharer from stats and decrement shares 
         {
            modify( feed, [&]( feed_object& f )
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
      ++feed_itr;
   }

   const auto& blog_idx = get_index< blog_index >().indices().get< by_comment >();
   auto blog_itr = blog_idx.lower_bound( comment_id );

   while( blog_itr != blog_idx.end() && blog_itr->comment == comment_id )
   {
      const blog_object& blog = *blog_itr;
      flat_map< account_name_type, time_point > shared_by = blog.shared_by;
      if( shared_by[ sharer ] != time_point() )
      {
         if( blog.shared_by.size() == 1 )     // Remove if Account was only sharer
         {
            blog_id_type blog_id = blog.id; 
            remove( blog );
            blog_itr = blog_idx.lower_bound( boost::make_tuple( comment_id, blog_id ) );
            continue;
         }
         else     // Remove sharer from stats and decrement shares 
         {
            modify( blog, [&]( blog_object& f )
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
      ++blog_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Adds or removes Feeds within an account's following 
 * and connection level Feeds for a specified account. 
 */
void database::update_account_in_feed( const account_name_type& account, const account_name_type& followed )
{ try {
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const auto& blog_idx = get_index< blog_index >().indices().get< by_new_account_blog >();
   const account_following_object& acc_following = get_account_following( account );
   auto blog_itr = blog_idx.lower_bound( followed );
   auto blog_end = blog_idx.upper_bound( followed );

   while( blog_itr != blog_end )
   {
      const comment_id_type& comment_id = blog_itr->comment;
      const comment_object& comment = get( comment_id );
      feed_reach_type reach = comment.reach;
      
      switch( reach )
      {
         case feed_reach_type::COMPANION_FEED:
         {
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::COMPANION_FEED ) );
            if( acc_following.is_companion( followed ) )      // Account is new companion
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's companion feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::COMPANION_FEED;
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *feed_itr, [&]( feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer companion, Comment is in account's companion feed, and Account was a sharer
            else if( feed_itr != feed_idx.end() ) 
            {
               const feed_object& feed = *feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( feed_object& f )
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
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FRIEND_FEED ) );
            if( acc_following.is_friend( followed ) )      // Account is new friend
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's friend feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::FRIEND_FEED;
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *feed_itr, [&]( feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer friend, Comment is in account's friend feed, and Account was a sharer
            else if( feed_itr != feed_idx.end() ) 
            {
               const feed_object& feed = *feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( feed_object& f )
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
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::CONNECTION_FEED ) );
            if( acc_following.is_connection( followed ) )      // Account is new connection
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's connection feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::CONNECTION_FEED;
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *feed_itr, [&]( feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer connection, Comment is in account's connection feed, and Account was a sharer
            else if( feed_itr != feed_idx.end() ) 
            {
               const feed_object& feed = *feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( feed_object& f )
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
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::MUTUAL_FEED ) );
            if( acc_following.is_mutual( followed ) )      // Account is new mutual
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's mutual feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::MUTUAL_FEED;
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *feed_itr, [&]( feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer mutual, Comment is in account's mutual feed, and Account was a sharer
            else if( feed_itr != feed_idx.end() ) 
            {
               const feed_object& feed = *feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( feed_object& f )
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
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_reach_type::FOLLOW_FEED ) );
            if( acc_following.is_following( followed ) )      // Account is new follow
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's follow feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = feed_reach_type::FOLLOW_FEED;
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares = 1;
                  });
               }
               else
               {
                  modify( *feed_itr, [&]( feed_object& f )
                  {
                     f.feed_time = std::max( f.feed_time, blog_itr->blog_time );     // Bump feed time if blog is later
                     f.shared_by[ followed ] = blog_itr->blog_time;
                     f.shares++;
                  });
               }
            } // Account is no longer follow, Comment is in account's follow feed, and Account was a sharer
            else if( feed_itr != feed_idx.end() ) 
            {
               const feed_object& feed = *feed_itr;
               flat_map< account_name_type, time_point> shared_by = feed.shared_by;
               if( feed.shares == 1 && feed.first_shared_by == followed ) // Account was only sharer
               {
                  remove( feed );
               }
               else if( shared_by[ followed ] != time_point() )    // remove account from sharing metrics
               {
                  modify( feed, [&]( feed_object& f )
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

      ++blog_itr;
   }

} FC_CAPTURE_AND_RETHROW() }


/**
 * Adds or removes Feeds within an account's Community Feed variants for a 
 * specified community. 
 */
void database::update_community_in_feed( const account_name_type& account, const community_name_type& community )
{ try {
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const auto& blog_idx = get_index< blog_index >().indices().get< by_new_community_blog >();
   const account_following_object& acc_following = get_account_following( account );
   auto blog_itr = blog_idx.lower_bound( community );
   auto blog_end = blog_idx.upper_bound( community );
   feed_reach_type feed_type = feed_reach_type::COMMUNITY_FEED;

   while( blog_itr != blog_end )
   {
      const comment_id_type& comment_id = blog_itr->comment;
      
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( acc_following.is_followed_community( community ) )
      {
         if( feed_itr == feed_idx.end() )      // Comment is not already in account's community feed 
         {
            create< feed_object >( [&]( feed_object& f )
            {
               f.account = account;
               f.feed_time = blog_itr->blog_time;
               f.comment = comment_id;
               f.feed_type = feed_type;
               f.shared_by = blog_itr->shared_by;
               f.first_shared_by = blog_itr->first_shared_by;
               f.shares = blog_itr->shares;
            });
         }
         else
         {
            modify( *feed_itr, [&]( feed_object& f )
            {
               f.feed_time = std::max( f.feed_time, blog_itr->blog_time );     // Bump feed time if blog is later
               f.shared_by = map_merge( f.shared_by, blog_itr->shared_by, 
               [&]( time_point a, time_point b )
               {
                  return a > b;    // Take the latest time point of all sharing accounts
               });
               f.shares = f.shared_by.size();
            });
         }
      } // Account is no longer following community, Comment is in account's community feed
      else if( feed_itr != feed_idx.end() ) 
      {
         const feed_object& feed = *feed_itr;
         flat_map< community_name_type, flat_map< account_name_type, time_point > > communities = feed.communities;
         if( feed_itr->communities.size() == 1 )      // This community was the only community it was shared with.
         {
            remove( feed );        // Remove the entire feed
         }
         else if( communities[ community ].size() != 0 )    // remove community from sharing metrics
         {
            modify( feed, [&]( feed_object& f )
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
      ++blog_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Adds or removes Feeds within an account's Tag Feed for a 
 * specified tag. 
 */
void database::update_tag_in_feed( const account_name_type& account, const tag_name_type& tag )
{ try {
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const auto& blog_idx = get_index< blog_index >().indices().get< by_new_tag_blog >();
   auto blog_itr = blog_idx.lower_bound( tag );
   auto blog_end = blog_idx.upper_bound( tag );
   const account_following_object& acc_following = get_account_following( account );
   feed_reach_type feed_type = feed_reach_type::COMMUNITY_FEED;

   while( blog_itr != blog_end )
   {
      const comment_id_type& comment_id = blog_itr->comment;
      
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( acc_following.is_followed_tag( tag ) )
      {
         if( feed_itr == feed_idx.end() )      // Comment is not already in account's tag feed 
         {
            create< feed_object >( [&]( feed_object& f )
            {
               f.account = account;
               f.feed_time = blog_itr->blog_time;
               f.comment = comment_id;
               f.feed_type = feed_type;
               f.shared_by = blog_itr->shared_by;
               f.first_shared_by = blog_itr->first_shared_by;
               f.shares = blog_itr->shares;
            });
         }
         else
         {
            modify( *feed_itr, [&]( feed_object& f )
            {
               f.feed_time = std::max( f.feed_time, blog_itr->blog_time );     // Bump feed time if blog is later
               f.shared_by = map_merge( f.shared_by, blog_itr->shared_by, [&]( time_point a, time_point b )
                  {
                     return a > b;    // Take the latest time point of all sharing accounts
                  });
               f.shares = f.shared_by.size();
            });
         }
      } // Account is no longer following tag, Comment is in account's tag feed
      else if( feed_itr != feed_idx.end() ) 
      {
         const feed_object& feed = *feed_itr;
         flat_map< tag_name_type, flat_map< account_name_type, time_point > > tags = feed.tags;
         if( feed_itr->tags.size() == 1 )      // This Tag was the only tag it was shared with.
         {
            remove( feed );    // Remove the entire feed
         }
         else if( tags[ tag ].size() != 0 )     // remove tag from sharing metrics
         {
            modify( feed, [&]( feed_object& f )
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
      ++blog_itr;
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


} } //node::chain