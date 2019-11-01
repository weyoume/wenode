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
#include <node/chain/witness_schedule.hpp>
#include <node/witness/witness_objects.hpp>

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
   uint128_t   recent_content_claims = 0;
   asset       content_reward_balance = asset( 0, SYMBOL_COIN );
   share_type  reward_distributed = 0;
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
      if( map_b[ value_a.first ] )
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
share_type database::pay_voters( const comment_object& c, const share_type& max_rewards )
{ try {
   uint128_t total_weight( c.total_vote_weight );
   share_type unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return 0;
   }
   else if( c.total_vote_weight > 0 )
   {
      const auto& vote_idx = get_index<comment_vote_index>().indices().get<by_comment_weight_voter>();
      auto vote_itr = vote_idx.lower_bound( c.id );
      while( vote_itr != vote_idx.end() && vote_itr->comment == c.id )
      {
         uint128_t weight( vote_itr->weight );
         auto claim = ( ( max_rewards.value * weight ) / total_weight ).to_uint64();
         if( claim > 0 )
         {
            unclaimed_rewards -= claim;
            const auto& voter = get_account(vote_itr->voter);
            asset reward = asset( claim, SYMBOL_COIN );
            adjust_reward_balance( voter.name, reward );
            push_virtual_operation( curation_reward_operation( voter.name, reward, c.author, to_string( c.permlink ) ) );

            modify( voter, [&]( account_object& a )
            {
               a.curation_rewards += claim;
            });
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
share_type database::pay_viewers( const comment_object& c, const share_type& max_rewards )
{ try {
   uint128_t total_weight( c.total_view_weight );
   share_type unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return 0;
   }
   else if( c.total_view_weight > 0 )
   {
      const auto& view_idx = get_index<comment_view_index>().indices().get<by_comment_weight_viewer>();
      auto view_itr = view_idx.lower_bound( c.id );
      while( view_itr != view_idx.end() && view_itr->comment == c.id )
      {
         uint128_t weight( view_itr->weight );
         auto claim = ( ( max_rewards.value * weight ) / total_weight ).to_uint64();
         if( claim > 0 )
         {
            unclaimed_rewards -= claim;
            const auto& viewer = get_account(view_itr->viewer);
            asset reward = asset( claim, SYMBOL_COIN );
            adjust_reward_balance( viewer.name, reward );
            push_virtual_operation( curation_reward_operation( viewer.name, reward, c.author, to_string( c.permlink ) ) );

            modify( viewer, [&]( account_object& a )
            {
               a.curation_rewards += claim;
            });
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
share_type database::pay_sharers( const comment_object& c, const share_type& max_rewards )
{ try {
   uint128_t total_weight( c.total_share_weight );
   share_type unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return 0;
   }
   else if( c.total_share_weight > 0 )
   {
      const auto& share_idx = get_index<comment_share_index>().indices().get<by_comment_weight_sharer>();
      auto share_itr = share_idx.lower_bound( c.id );
      while( share_itr != share_idx.end() && share_itr->comment == c.id )
      {
         uint128_t weight( share_itr->weight );
         auto claim = ( ( max_rewards.value * weight ) / total_weight ).to_uint64();
         if( claim > 0 )
         {
            unclaimed_rewards -= claim;
            const auto& sharer = get_account(share_itr->sharer);
            asset reward = asset( claim, SYMBOL_COIN );
            adjust_reward_balance( sharer.name, reward );
            push_virtual_operation( curation_reward_operation( sharer.name, reward, c.author, to_string( c.permlink ) ) );

            modify( sharer, [&]( account_object& a )
            {
               a.curation_rewards += claim;
            });
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
share_type database::pay_commenters( const comment_object& c, const share_type& max_rewards )
{ try {
   uint128_t total_weight( c.total_comment_weight );
   share_type unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return 0;
   }
   else if( c.total_comment_weight > 0 )
   {
      const auto& comment_idx = get_index<comment_index>().indices().get<by_root>();
      auto comment_itr = comment_idx.lower_bound( c.id );
      while( comment_itr != comment_idx.end() && comment_itr->root_comment == c.id )
      {
         uint128_t weight( comment_itr->weight );
         auto claim = ( ( max_rewards.value * weight ) / total_weight ).to_uint64();
         if( claim > 0 )
         {
            unclaimed_rewards -= claim;
            const auto& commenter = get_account(comment_itr->author);
            asset reward = asset( claim, SYMBOL_COIN );
            adjust_reward_balance( commenter.name, reward );
            push_virtual_operation( curation_reward_operation( commenter.name, reward, c.author, to_string( c.permlink ) ) );

            modify( commenter, [&]( account_object& a )
            {
               a.curation_rewards += claim;
            });
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
share_type database::pay_storage( const comment_object& c, const share_type& max_rewards )
{ try {
   uint128_t total_weight( c.total_view_weight );
   share_type unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return 0;
   }
   else if( c.total_view_weight > 0 )
   {
      const auto& view_idx = get_index<comment_view_index>().indices().get<by_comment_weight_viewer>();
      auto view_itr = view_idx.lower_bound( c.id );
      while( view_itr != view_idx.end() && view_itr->comment == c.id )
      {
         uint128_t weight( view_itr->weight );
         auto claim = ( ( max_rewards.value * weight ) / total_weight ).to_uint64();
         if( claim > 0 )
         {
            unclaimed_rewards -= claim;
            const account_object& viewer = get_account( view_itr->viewer );
            const supernode_object& supernode = get_supernode( view_itr->supernode );
            asset reward = asset( claim, SYMBOL_COIN );
            adjust_reward_balance( supernode.account, reward );

            modify( viewer, [&]( account_object& a )
            {
               a.curation_rewards += claim;
            });
         }
         ++view_itr;
      }
   }
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes rewards from a comment to the moderators of the board it was
 * posted to, according to their voting power.
 */
share_type database::pay_moderators( const comment_object& c, const share_type& max_rewards )
{ try {
   const board_member_object& board_member = get_board_member( c.board );
   uint128_t total_weight( board_member.total_mod_weight.value );
   share_type unclaimed_rewards = max_rewards;

   if( !c.allow_curation_rewards )
   {
      return 0;
   }
   else if( c.total_view_weight > 0 )
   {
      for( auto mod : board_member.mod_weight )
      {
         uint128_t weight( mod.second.value );
         auto claim = ( ( max_rewards.value * weight ) / total_weight ).to_uint64();
         if( claim > 0 )
         {
            unclaimed_rewards -= claim;
            const account_object& moderator = get_account( mod.first );
            asset reward = asset( claim, SYMBOL_COIN );
            adjust_reward_balance( moderator.name, reward );

            modify( moderator, [&]( account_object& a )
            {
               a.moderation_rewards += claim;
            });
         }
      }
   }
   return unclaimed_rewards;
} FC_CAPTURE_AND_RETHROW() }



void fill_comment_reward_context_local_state( util::comment_reward_context& ctx, const comment_object& comment )
{
   ctx.reward = comment.net_reward;
   ctx.cashouts_received = comment.cashouts_received;
   ctx.max_reward = comment.max_accepted_payout;
}


/**
 * Distributes Content rewards to comments once per day
 * for the first 30 days after they are created, and 
 * splits rewards between the author and the voters, viewers, 
 * sharers, supernodes, and moderators.
 */
share_type database::distribute_comment_reward( util::comment_reward_context& ctx, const comment_object& comment )
{ try {
   share_type claimed_reward = 0;
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;

   if( comment.net_reward > 0 )
   {
      fill_comment_reward_context_local_state( ctx, comment );
      const auto reward_fund = get_reward_fund();
      ctx.reward_curve = reward_fund.author_reward_curve;
      ctx.content_constant = reward_fund.content_constant;
      const share_type reward = util::get_comment_reward( ctx );
      uint128_t reward_tokens = uint128_t( reward.value );

      if( reward_tokens > 0 )
      {
         share_type voter_tokens = ( ( reward_tokens * comment.vote_reward_percent ) / PERCENT_100 ).to_uint64();
         share_type viewer_tokens = ( ( reward_tokens * comment.view_reward_percent ) / PERCENT_100 ).to_uint64();
         share_type sharer_tokens = ( ( reward_tokens * comment.share_reward_percent ) / PERCENT_100 ).to_uint64();
         share_type commenter_tokens = ( ( reward_tokens * comment.comment_reward_percent ) / PERCENT_100 ).to_uint64();
         share_type storage_tokens = ( ( reward_tokens * comment.storage_reward_percent ) / PERCENT_100 ).to_uint64();
         share_type moderator_tokens = ( ( reward_tokens * comment.moderator_reward_percent ) / PERCENT_100 ).to_uint64();
         share_type total_curation_tokens = voter_tokens + viewer_tokens + sharer_tokens + commenter_tokens + storage_tokens + moderator_tokens;
         share_type author_tokens = reward_tokens - total_curation_tokens;

         author_tokens += pay_voters( comment, voter_tokens );
         author_tokens += pay_viewers( comment, viewer_tokens );
         author_tokens += pay_sharers( comment, sharer_tokens );
         author_tokens += pay_commenters( comment, commenter_tokens );
         author_tokens += pay_storage( comment, storage_tokens );
         author_tokens += pay_moderators( comment, moderator_tokens );

         share_type total_beneficiary = 0;
         claimed_reward = author_tokens + total_curation_tokens;

         for( auto& b : comment.beneficiaries )
         {
            share_type benefactor_tokens = ( author_tokens * b.weight ) / PERCENT_100;
            asset reward_created = asset( benefactor_tokens, SYMBOL_COIN);
            adjust_reward_balance( b.account, reward_created );
            push_virtual_operation( comment_benefactor_reward_operation( b.account, comment.author, to_string( comment.permlink ), reward_created ) );
            total_beneficiary += benefactor_tokens;
         }

         author_tokens -= total_beneficiary;

         const account_object& author = get_account( comment.author );
         asset author_reward = asset( author_tokens, SYMBOL_COIN );
         adjust_reward_balance( author, author_reward );

         adjust_total_payout( comment, asset_to_USD( asset( author_tokens, SYMBOL_COIN ) ), asset_to_USD( asset( total_curation_tokens, SYMBOL_COIN ) ), asset_to_USD( asset( total_beneficiary, SYMBOL_COIN ) ) );

         push_virtual_operation( author_reward_operation( comment.author, to_string( comment.permlink ), author_reward ) );
         push_virtual_operation( comment_reward_operation( comment.author, to_string( comment.permlink ), asset_to_USD( asset( claimed_reward, SYMBOL_COIN ) ) ) );

         modify( comment, [&]( comment_object& c )
         {
            c.author_rewards += author_tokens;
         });

         modify( get_account( comment.author ), [&]( account_object& a )
         {
            a.posting_rewards += author_tokens;
         });

         const board_object* board_ptr = find_board( comment.board );

         if( board_ptr != nullptr )
         {
            modify( *board_ptr, [&]( board_object& b )
            {
               b.total_content_rewards += asset( claimed_reward, SYMBOL_COIN );
            });
         }
      }
   }

   modify( comment, [&]( comment_object& c )
   {
      if( c.cashouts_received < ( props.content_reward_decay_rate.to_seconds() / props.content_reward_interval.to_seconds() ) )
      {
         if( c.net_reward > 0 )   // A payout is only made for positive reward.
         {
            c.cashouts_received++;
            c.last_payout = now;
         }
         c.cashout_time += props.content_reward_interval;
      }
      else
      {
         c.cashout_time = fc::time_point::maximum();
      }
   });

   push_virtual_operation( comment_payout_update_operation( comment.author, to_string( comment.permlink ) ) );
   const auto& vote_idx = get_index< comment_vote_index >().indices().get< by_comment_voter >();
   auto vote_itr = vote_idx.lower_bound( comment.id );
   while( vote_itr != vote_idx.end() && vote_itr->comment == comment.id )
   {
      const comment_vote_object& cur_vote = *vote_itr;
      ++vote_itr;
      if( calculate_discussion_payout_time( comment ) != fc::time_point::maximum() )
      {
         modify( cur_vote, [&]( comment_vote_object& cvo )
         {
            cvo.num_changes = -1;
         });
      }
      else
      {
#ifdef CLEAR_VOTES  // Removes comment vote objects from memory that are no longer required for consensus if flag is enabled.
         remove( cur_vote );
#endif
      }
   }
   return claimed_reward;
} FC_CAPTURE_AND_RETHROW( (comment) ) }


/**
 * Distributes content rewards to content authors and curators
 * when posts reach a cashout time.
 */
void database::process_comment_cashout()
{
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   util::comment_reward_context ctx;
   ctx.current_COIN_USD_price = get_liquidity_pool(SYMBOL_COIN, SYMBOL_USD).day_median_price;
   vector< share_type > reward_distributed;
   auto decay_rate = RECENT_REWARD_DECAY_RATE;
   const reward_fund_object& reward_fund = get_reward_fund();

   // Decay recent reward of post reward fund
   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.recent_content_claims -= ( rfo.recent_content_claims * ( now - rfo.last_update ).to_seconds() ) / decay_rate.to_seconds();
      rfo.last_update = now;
   });

   // Snapshots the reward fund into a seperate object reward fund context to ensure equal execution conditions.
   reward_fund_context rf_ctx;
   rf_ctx.recent_content_claims = reward_fund.recent_content_claims;
   rf_ctx.content_reward_balance = reward_fund.content_reward_balance;

   const auto& cidx = get_index< comment_index >().indices().get< by_cashout_time >();
   const auto& com_by_root = get_index< comment_index >().indices().get< by_root >();
   auto current = cidx.begin();
   
   while( current != cidx.end() && current->cashout_time <= now )
   {
      if( current->net_reward > 0 )
      {
         rf_ctx.recent_content_claims += util::evaluate_reward_curve( uint128_t(current->net_reward), current->cashouts_received, reward_fund.author_reward_curve, decay_rate, reward_fund.content_constant );
      }
      ++current;
   }

   current = cidx.begin();
   
   while( current != cidx.end() && current->cashout_time <= now )
   {
      ctx.total_reward_squared = rf_ctx.recent_content_claims;
      ctx.total_reward_fund = rf_ctx.content_reward_balance;
      rf_ctx.reward_distributed += distribute_comment_reward( ctx, *current );    // Allocates reward balances to accounts from the comment reward context

      ++current;
   }

   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.recent_content_claims = rf_ctx.recent_content_claims;
      rfo.content_reward_balance -= rf_ctx.reward_distributed;
   });
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

   auto now = head_block_time();
   const dynamic_global_property_object& gpo = get_dynamic_global_properties();
   const comment_metrics_object& comment_metrics = get_comment_metrics();
   
   // Initialize comment metrics

   uint32_t recent_post_count = 0;        
   int128_t recent_vote_power = 0;
   int128_t recent_view_power = 0;
   int128_t recent_share_power = 0;
   int128_t recent_comment_power = 0;
   int128_t average_vote_power = 0;
   int128_t average_view_power = 0;
   int128_t average_share_power = 0;
   int128_t average_comment_power = 0;
   int128_t median_vote_power = 0;
   int128_t median_view_power = 0;
   int128_t median_share_power = 0;
   int128_t median_comment_power = 0;
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

   const auto& cidx = get_index<comment_index>().indices().get< by_time >();
   auto current = cidx.lower_bound( true );                                   // Finds first root post in time index
   
   while( current != cidx.end() && (now < (current->created + METRIC_CALC_TIME) ) ) // Iterates over all root posts in last 30 days
   {
      const comment_object& comment = *current;
      comments.push_back( &comment );                          // Add comment pointer to vector
      recent_post_count++;
      recent_vote_power += current->vote_power;
      recent_view_power += current->view_power;
      recent_share_power += current->share_power;
      recent_comment_power += current->comment_power;
      recent_vote_count += current->net_votes;
      recent_view_count += current->view_count;
      recent_share_count += current->share_count;
      recent_comment_count += current->children;
      ++current;
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
      vote_view_ratio = double(recent_view_power) / double( recent_vote_power );
      vote_share_ratio = double(recent_share_power) / double( recent_vote_power);
      vote_comment_ratio = double(recent_comment_power) / double( recent_vote_power );

      // Median count values
      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
      {
         return a->net_votes < b->net_votes;
      });
      median_vote_count = comments[comments.size()/2]->net_votes;

      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
      {
         return a->view_count < b->view_count;
      });
      median_view_count = comments[comments.size()/2]->view_count;

      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
      {
         return a->share_count < b->share_count;
      });
      median_share_count = comments[comments.size()/2]->share_count;

      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
      {
         return a->children < b->children;
      } );
      median_comment_count = comments[comments.size()/2]->children;

      // Median Power values
      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
      {
         return a->vote_power < b->vote_power;
      });
      median_vote_power = comments[comments.size()/2]->vote_power;

      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
      {
         return a->view_power < b->view_power;
      });
      median_view_power = comments[comments.size()/2]->view_power;

      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
      {
         return a->share_power < b->share_power;
      });
      median_share_power = comments[comments.size()/2]->share_power;

      std::sort( comments.begin(), comments.end(), [&](comment_object* a, comment_object* b)
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
 * Additionally adds the new post to the board feed for the board it is created in
 * and the tag feeds for all the tags it contains. 
 */
void database::add_comment_to_feeds( const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const account_following_object& acc_following = get_account_following( comment.author );
   const board_member_object* board_member_ptr = nullptr;
   if( comment.board.size )
   {
      board_member_ptr = find_board_member( comment.board );
   }

   const auto& account_blog_idx = get_index< blog_index >().indices().get< by_comment_board >();
   auto account_blog_itr = account_blog_idx.find( boost::make_tuple( comment_id, comment.board ) );
   if( account_blog_itr == account_blog_idx.end() )       // Comment is not already in the account's blog
   {
      create< blog_object >( [&]( blog_object& b )
      {
         b.account = comment.author;
         b.comment = comment_id;
         b.blog_type = ACCOUNT_BLOG;
         b.blog_time = now; 
         b.shared_by[ comment.author ] = now;
         b.first_shared_by = comment.author;
         b.shares = 1;
      });
   }

   switch( comment.reach )
   {
      case NO_FEED:
      {
         return;               // Do not share to any feeds. Shows only on account blog. 
      }
      case BOARD_FEED:         // Encrypted Board feed variants are only shared to board subscribers, and not account followers or connections. 
      case GROUP_FEED:
      case EVENT_FEED:
      case STORE_FEED:
      case COMPANION_FEED:     // Encrypted posts are only shared to connected accounts of the specified level.
      case FRIEND_FEED:
      case CONNECTION_FEED:
      {
         FC_ASSERT( comment.privacy, "Post should be encrypted at this reach level." );
      }
      case MUTUAL_FEED:        // Public Posts only from here down
      case FOLLOW_FEED:
      case TAG_FEED:           // Tag Feed level posts are shared to tag followers, in addition to account followers. 
      {
         FC_ASSERT( !comment.privacy, "Post should not encrypted at this reach level." );
      }
      default:
      {
         FC_ASSERT( false, "Invalid reach selection. ");
      }
   }

   if( board_member_ptr != nullptr )
   {
      feed_types feed_type = BOARD_FEED;

      switch( board_member_ptr->board_type )
      {
         case BOARD: 
            break;
         case GROUP:
         {
            feed_type = GROUP_FEED;
         }
         break;
         case EVENT:
         {
            feed_type = EVENT_FEED;
         }
         break;
         case STORE:
         {
            feed_type = STORE_FEED;
         }
         break;
         default:
         {
            FC_ASSERT( false, "Invalid board type.");
         }
      }

      const auto& board_blog_idx = get_index< blog_index >().indices().get< by_comment_board >();
      auto board_blog_itr = board_blog_idx.find( boost::make_tuple( comment_id, comment.board ) );
      if( board_blog_itr == board_blog_idx.end() )       // Comment is not already in the board's blog
      {
         create< blog_object >( [&]( blog_object& b )
         {
            b.board = comment.board;
            b.comment = comment_id;
            b.blog_type = BOARD_BLOG;
            b.blog_time = now; 
            b.shared_by[ comment.author ] = now;
            b.first_shared_by = comment.author;
            b.shares = 1;
         });
      }

      for( const account_name_type& account : board_member_ptr->subscribers )    // Add post to board feeds. 
      {
         auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
         if( feed_itr == feed_idx.end() )         // Comment is not already in account's boards feed for the type of board. 
         {
            create< feed_object >( [&]( feed_object& f )
            {
               f.account = account;
               f.comment = comment_id;
               f.feed_type = feed_type;
               f.feed_time = now;
               f.boards[ comment.board ][ comment.author ] = now;
               f.shared_by[ comment.author ] = now;
               f.first_shared_by = comment.author;
               f.shares = 1;
            });
         } 
      }

      if( comment.reach == feed_type )
      { 
         return;
      }
   }

   for( const account_name_type& account : acc_following.companions )    // Add to companion feeds
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, COMPANION_FEED ) );
      if( feed_itr == feed_idx.end() )      // Comment is not already in companion feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = COMPANION_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == COMPANION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.friends ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( comment_id, account, FRIEND_FEED ) );
      if( feed_itr == feed_idx.end() )      // Comment is not already in friend feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = FRIEND_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == FRIEND_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.connections ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, CONNECTION_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in connection feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = CONNECTION_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == CONNECTION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.mutual_followers ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, MUTUAL_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in mutual feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = MUTUAL_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }
   }

   if( comment.reach == MUTUAL_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.followers ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, FOLLOW_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in follow feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = FOLLOW_FEED;
            f.shared_by[ comment.author ] = now;
            f.first_shared_by = comment.author;
            f.shares = 1;
         });
      }   
   }

   if( comment.reach == FOLLOW_FEED )
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
            b.blog_type = TAG_BLOG;
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
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, TAG_FEED ) );
            if( feed_itr == feed_idx.end() )       // Comment is not already in the account's tag feed
            {
               create< feed_object >( [&]( feed_object& f )
               {
                  f.account = account;
                  f.comment = comment_id;
                  f.feed_type = TAG_FEED;
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
   const feed_types& reach, const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const account_following_object& acc_following = get_account_following( sharer );

   switch( reach )
   {
      case COMPANION_FEED:
      case FRIEND_FEED:
      case CONNECTION_FEED:
      case MUTUAL_FEED:
      case FOLLOW_FEED:
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
         b.blog_type = ACCOUNT_BLOG;
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
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, COMPANION_FEED ) );

      if( feed_itr == feed_idx.end() )      // Comment is not already in companion feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = COMPANION_FEED;
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

   if( reach == COMPANION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.friends ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( comment_id, account, FRIEND_FEED ) );

      if( feed_itr == feed_idx.end() )      // Comment is not already in friend feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = FRIEND_FEED;
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

   if( reach == FRIEND_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.connections ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, CONNECTION_FEED ) );

      if( feed_itr == feed_idx.end() )   // Comment is not already in connection feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = CONNECTION_FEED;
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

   if( reach == CONNECTION_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.mutual_followers ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, MUTUAL_FEED ) );

      if( feed_itr == feed_idx.end() )   // Comment is not already in mutual feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = MUTUAL_FEED;
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

   if( reach == MUTUAL_FEED )
   { 
      return;
   }

   for( const account_name_type& account : acc_following.followers ) 
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, FOLLOW_FEED ) );
      if( feed_itr == feed_idx.end() )   // Comment is not already in follow feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.feed_time = now;
            f.comment = comment_id;
            f.feed_type = FOLLOW_FEED;
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
 * the board's subscriber list. Accounts can share posts 
 * with new boards to increase thier reach. 
 */
void database::share_comment_to_board( const account_name_type& sharer, 
   const board_name_type& board, const comment_object& comment )
{ try {
   time_point now = head_block_time();
   const comment_id_type& comment_id = comment.id;
   const board_member_object& board_member = get_board_member( board );
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   feed_types feed_type = BOARD_FEED;

   switch( board_member.board_type )
   {
      case BOARD: 
         break;
      case GROUP:
      {
         feed_type = GROUP_FEED;
      }
      break;
      case EVENT:
      {
         feed_type = EVENT_FEED;
      }
      break;
      case STORE:
      {
         feed_type = STORE_FEED;
      }
      break;
   }

   const auto& blog_idx = get_index< blog_index >().indices().get< by_comment_board >();
   auto blog_itr = blog_idx.find( boost::make_tuple( comment_id, board ) );
   if( blog_itr == blog_idx.end() )       // Comment is not already in the boards's blog
   {
      create< blog_object >( [&]( blog_object& b )
      {
         b.board = board;
         b.comment = comment_id;
         b.blog_type = BOARD_BLOG;
         b.blog_time = now; 
         b.shared_by[ sharer ] = now;
         b.first_shared_by = sharer;
         b.shares = 1;
      });
   }
   else      // Comment has already been shared with board, bump time and increment shares
   {
      modify( *blog_itr, [&]( blog_object& b )
      {
         b.blog_time = now;
         b.shared_by[ sharer ] = now;
         b.shares++;
      });
   }

   for( const account_name_type& account : board_member.subscribers )
   {
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( feed_itr == feed_idx.end() )         // Comment is not already in account's boards feed for the type of board. 
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.comment = comment_id;
            f.feed_type = feed_type;
            f.feed_time = now; 
            f.boards[ board ][ sharer ] = now;
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
            f.boards[ board ][ sharer ] = now;
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

   if( blog_itr == blog_idx.end() )       // Comment is not already in the tag's blog
   {
      create< blog_object >( [&]( blog_object& b )
      {
         b.tag = tag;
         b.comment = comment_id;
         b.blog_type = TAG_BLOG;
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
         auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, TAG_FEED ) );
         if( feed_itr == feed_idx.end() )       // Comment is not already in the account's tag feed
         {
            create< feed_object >( [&]( feed_object& f )
            {
               f.account = account;
               f.comment = comment_id;
               f.feed_type = TAG_FEED;
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
 * Removes a comment from all feeds and blogs for all accounts, tags, and boards. 
 */
void database::clear_comment_feeds( const comment_object& comment )
{ try {
   time_point now = head_block_time();
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
 * Removes a comment from all account, tag, and board feeds, and blogs that an account shared it to. 
 */
void database::remove_shared_comment( const account_name_type& sharer, const comment_object& comment )
{ try {
   time_point now = head_block_time();
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
               for( auto board : f.boards )
               {
                  f.boards[ board.first ].erase( sharer );
               }
               f.shared_by.erase( sharer );
               vector< pair< account_name_type, time_point > > shared_by_copy;
               for( auto time : f.shared_by )
               {
                  shared_by_copy.push_back( time );
               }
               std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
               [&]( auto a, auto b)
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
               [&]( auto a, auto b)
               {
                  return a.second < b.second;
               });

               f.blog_time = shared_by_copy[0].second;  // blog time is the new latest time after removing account. 
               f.first_shared_by = shared_by_copy[ shared_by_copy.size()-1 ].first; // Earliest time is new first sharer
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
   time_point now = head_block_time();
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const auto& blog_idx = get_index< blog_index >().indices().get< by_new_account_blog >();
   const account_following_object& acc_following = get_account_following( account );
   auto blog_itr = blog_idx.lower_bound( followed );
   auto blog_end = blog_idx.upper_bound( followed );

   while( blog_itr != blog_end )
   {
      const comment_id_type& comment_id = blog_itr->comment;
      const comment_object& comment = get( comment_id );
      feed_types reach = comment.reach;
      
      switch( reach )
      {
         case COMPANION_FEED:
         {
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, COMPANION_FEED ) );
            if( acc_following.is_companion( followed ) )      // Account is new companion
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's companion feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = COMPANION_FEED;
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
                     [&]( auto a, auto b)
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
         case FRIEND_FEED:
         {
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, FRIEND_FEED ) );
            if( acc_following.is_friend( followed ) )      // Account is new friend
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's friend feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = FRIEND_FEED;
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
                     [&]( auto a, auto b)
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
         case CONNECTION_FEED:
         {
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, CONNECTION_FEED ) );
            if( acc_following.is_connection( followed ) )      // Account is new connection
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's connection feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = CONNECTION_FEED;
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
                     [&]( auto a, auto b)
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
         case MUTUAL_FEED:
         {
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, MUTUAL_FEED ) );
            if( acc_following.is_mutual( followed ) )      // Account is new mutual
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's mutual feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = MUTUAL_FEED;
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
                     [&]( auto a, auto b)
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
         case FOLLOW_FEED:
         {
            auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, FOLLOW_FEED ) );
            if( acc_following.is_following( followed ) )      // Account is new follow
            {
               if( feed_itr == feed_idx.end() )      // Comment is not already in account's follow feed 
               {
                  create< feed_object >( [&]( feed_object& f )
                  {
                     f.account = account;
                     f.feed_time = blog_itr->blog_time;
                     f.comment = comment_id;
                     f.feed_type = FOLLOW_FEED;
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
                     [&]( auto a, auto b)
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
 * Adds or removes Feeds within an account's Board Feed variants for a 
 * specified board. 
 */
void database::update_board_in_feed( const account_name_type& account, const board_name_type& board )
{ try {
   const auto& feed_idx = get_index< feed_index >().indices().get< by_account_comment_type >();
   const auto& blog_idx = get_index< blog_index >().indices().get< by_new_board_blog >();
   const account_following_object& acc_following = get_account_following( account );
   auto blog_itr = blog_idx.lower_bound( board );
   auto blog_end = blog_idx.upper_bound( board );
   const board_member_object& board_member = get_board_member( board );
   feed_types feed_type = BOARD_FEED;

   switch( board_member.board_type )
   {
      case BOARD: 
         break;
      case GROUP:
      {
         feed_type = GROUP_FEED;
      }
      break;
      case EVENT:
      {
         feed_type = EVENT_FEED;
      }
      break;
      case STORE:
      {
         feed_type = STORE_FEED;
      }
      break;
   }

   while( blog_itr != blog_end )
   {
      const comment_id_type& comment_id = blog_itr->comment;
      const comment_object& comment = get( comment_id );
      
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, feed_type ) );
      if( acc_following.is_following( board ) )
      {
         if( feed_itr == feed_idx.end() )      // Comment is not already in account's board feed 
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
               f.shared_by = map_merge( f.shared_by, blog_itr->shared_by, [&](auto a, auto b)
                  {
                     return a > b;    // Take the latest time point of all sharing accounts
                  });
               f.shares = f.shared_by.size();
            });
         }
      } // Account is no longer following board, Comment is in account's board feed
      else if( feed_itr != feed_idx.end() ) 
      {
         const feed_object& feed = *feed_itr;
         flat_map< board_name_type, flat_map< account_name_type, time_point > > boards = feed.boards;
         if( feed_itr->boards.size() == 1 )      // This board was the only board it was shared with.
         {
            remove( feed );        // Remove the entire feed
         }
         else if( boards[ board ].size() != 0 )    // remove board from sharing metrics
         {
            modify( feed, [&]( feed_object& f )
            {
               for( auto board_value : f.boards[ board ] )
               {
                  f.shared_by.erase( board_value.first );   // Remove all shares from this board
               }
               f.boards.erase( board );
               vector< pair < account_name_type, time_point > > shared_by_copy;
               for( auto time : f.shared_by )
               {
                  shared_by_copy.push_back( time );
               }
               std::sort( shared_by_copy.begin(), shared_by_copy.end(), 
               [&]( auto a, auto b)
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

   while( blog_itr != blog_end )
   {
      const comment_id_type& comment_id = blog_itr->comment;
      const comment_object& comment = get( comment_id );
      
      auto feed_itr = feed_idx.find( boost::make_tuple( account, comment_id, TAG_FEED ) );
      if( acc_following.is_following( tag ) )
      {
         if( feed_itr == feed_idx.end() )      // Comment is not already in account's tag feed 
         {
            create< feed_object >( [&]( feed_object& f )
            {
               f.account = account;
               f.feed_time = blog_itr->blog_time;
               f.comment = comment_id;
               f.feed_type = TAG_FEED;
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
               f.shared_by = map_merge( f.shared_by, blog_itr->shared_by, [&](auto a, auto b)
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
               [&]( auto a, auto b)
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
 * This method updates total_reward_squared on DGPO, and children_reward_squared on comments, when a comment's reward_squared changes
 * from old_reward_squared to new_reward_squared.  Maintaining invariants that children_reward_squared is the sum of all descendants' reward_squared,
 * and dgpo.total_reward_squared is the total number of reward_squared outstanding.
 */
void database::adjust_reward_shares( const comment_object& c, uint128_t old_reward_shares, uint128_t new_reward_shares )
{
   const auto& reward_fund = get_reward_fund();
   modify( reward_fund, [&]( reward_fund_object& rfo )
   {
      rfo.total_reward_shares -= old_reward_shares;
      rfo.total_reward_shares += new_reward_shares;
   });
}


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