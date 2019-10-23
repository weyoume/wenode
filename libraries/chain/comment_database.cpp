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

using boost::container::flat_set;

struct reward_fund_context
{
   uint128_t   recent_content_claims = 0;
   asset       content_reward_balance = asset( 0, SYMBOL_COIN );
   share_type  reward_distributed = 0;
};

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


/**
 * Distributes rewards from a comment to the voters of that comment, according
 * to their vote weight.
 */
share_type database::pay_voters( const comment_object& c, share_type& max_rewards )
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
share_type database::pay_viewers( const comment_object& c, share_type& max_rewards )
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
share_type database::pay_sharers( const comment_object& c, share_type& max_rewards )
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
share_type database::pay_commenters( const comment_object& c, share_type& max_rewards )
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
share_type database::pay_storage( const comment_object& c, share_type& max_rewards )
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
share_type database::pay_moderators( const comment_object& c, share_type& max_rewards )
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
   time_point now = head_block_time();

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
      }
   }

   modify( comment, [&]( comment_object& c )
   {
      if( c.cashouts_received < ( CONTENT_REWARD_DECAY_RATE.to_seconds() / fc::days(1).to_seconds() ) )
      {
         if( c.net_reward > 0 )   // A payout is only made for positive reward.
         {
            c.cashouts_received++;
            c.last_payout = now;
         }
         c.cashout_time += fc::days(1);
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

void database::process_comment_cashout()
{
   const auto& gpo = get_dynamic_global_properties();
   time_point now = head_block_time();
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
         rf_ctx.recent_content_claims += util::evaluate_reward_curve( current->net_reward, current->cashouts_received, reward_fund.author_reward_curve, decay_rate, reward_fund.content_constant );
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
 * Updates every hour
 */
void database::update_comment_metrics() 
{ try {
   if( (head_block_num() % METRIC_INTERVAL_BLOCKS) != 0 )
      return;

   auto now = head_block_time();
   const dynamic_global_property_object& gpo = get_dynamic_global_properties();
   const comment_metrics_object& cmo = get_comment_metrics();
   
   // Initialize comment metrics

   uint32_t recent_post_count = 0;        
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
   comments.reserve( cmo.recent_post_count * 2 );

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
      vote_view_ratio = recent_view_power / recent_vote_power;
      vote_share_ratio = recent_share_power / recent_vote_power;
      vote_comment_ratio = recent_comment_power / recent_vote_power;

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
 * Adds a shared post to the feeds of each of the accounts in its
 * account following object.
 */
void database::add_comment_to_feeds( const account_name_type& sharer, const comment_id_type& comment_id )
{ try {
   time_point now = head_block_time();
   const auto& feed_idx = get_index< feed_index >().indices().get< by_feed >();
   const auto& comment_idx = get_index< feed_index >().indices().get< by_comment >();
   const account_following_object& acc_following = get_account_following( sharer );

   for( const account_name_type& account : acc_following.followers ) 
   {
      uint32_t next_id = 0;
      auto last_feed = feed_idx.lower_bound( account );
      if( last_feed != feed_idx.end() && last_feed->account == account )
      {
         next_id = last_feed->account_feed_id + 1;
      }
      auto feed_itr = comment_idx.find( boost::make_tuple( comment_id, account ) );
      if( feed_itr == comment_idx.end() )   // Comment is not already in feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.shared_by.push_back( sharer );
            f.first_shared_by = sharer;
            f.first_shared_on = now;
            f.comment = comment_id;
            f.feed_type = FOLLOW_FEED;
            f.shares = 1;
            f.account_feed_id = next_id;
         });
      }
      else
      {
         modify( *feed_itr, [&]( feed_object& f )
         {
            f.shared_by.push_back( sharer );
            f.shares++;
         });
      }    
   } 
   for( const account_name_type& account : acc_following.mutual_followers ) 
   {
      uint32_t next_id = 0;
      auto last_feed = feed_idx.lower_bound( account );

      if( last_feed != feed_idx.end() && last_feed->account == account )
      {
         next_id = last_feed->account_feed_id + 1;
      }

      auto feed_itr = comment_idx.find( boost::make_tuple( comment_id, account ) );

      if( feed_itr == comment_idx.end() )   // Comment is not already in feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.shared_by.push_back( sharer );
            f.first_shared_by = sharer;
            f.first_shared_on = now;
            f.comment = comment_id;
            f.feed_type = MUTUAL_FEED;
            f.shares = 1;
            f.account_feed_id = next_id;
         });
      }
      else
      {
         modify( *feed_itr, [&]( feed_object& f )
         {
            f.shared_by.push_back( sharer );
            f.shares++;
         });
      }
   }
   for( const account_name_type& account : acc_following.connections ) 
   {
      uint32_t next_id = 0;
      auto last_feed = feed_idx.lower_bound( account );

      if( last_feed != feed_idx.end() && last_feed->account == account )
      {
         next_id = last_feed->account_feed_id + 1;
      }

      auto feed_itr = comment_idx.find( boost::make_tuple( comment_id, account ) );

      if( feed_itr == comment_idx.end() )   // Comment is not already in feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.shared_by.push_back( sharer );
            f.first_shared_by = sharer;
            f.first_shared_on = now;
            f.comment = comment_id;
            f.feed_type = CONNECTION_FEED;
            f.shares = 1;
            f.account_feed_id = next_id;
         });
      }
      else
      {
         modify( *feed_itr, [&]( feed_object& f )
         {
            f.shared_by.push_back( sharer );
            f.shares++;
         });
      }
   }
   for( const account_name_type& account : acc_following.friends ) 
   {
      uint32_t next_id = 0;
      auto last_feed = feed_idx.lower_bound( account );

      if( last_feed != feed_idx.end() && last_feed->account == account )
      {
         next_id = last_feed->account_feed_id + 1;
      }

      auto feed_itr = comment_idx.find( boost::make_tuple( comment_id, account ) );

      if( feed_itr == comment_idx.end() )   // Comment is not already in feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.shared_by.push_back( sharer );
            f.first_shared_by = sharer;
            f.first_shared_on = now;
            f.comment = comment_id;
            f.feed_type = FRIEND_FEED;
            f.shares = 1;
            f.account_feed_id = next_id;
         });
      }
      else
      {
         modify( *feed_itr, [&]( feed_object& f )
         {
            f.shared_by.push_back( sharer );
            f.shares++;
         });
      }
   }
   for( const account_name_type& account : acc_following.companions ) 
   {
      uint32_t next_id = 0;
      auto last_feed = feed_idx.lower_bound( account );

      if( last_feed != feed_idx.end() && last_feed->account == account )
      {
         next_id = last_feed->account_feed_id + 1;
      }

      auto feed_itr = comment_idx.find( boost::make_tuple( comment_id, account ) );

      if( feed_itr == comment_idx.end() )   // Comment is not already in feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.shared_by.push_back( sharer );
            f.first_shared_by = sharer;
            f.first_shared_on = now;
            f.comment = comment_id;
            f.feed_type = COMPANION_FEED;
            f.shares = 1;
            f.account_feed_id = next_id;
         });
      }
      else
      {
         modify( *feed_itr, [&]( feed_object& f )
         {
            f.shared_by.push_back( sharer );
            f.shares++;
         });
      }
   }
} FC_CAPTURE_AND_RETHROW( (sharer)(comment_id) ) }



/** 
 * Adds a newly created post to the feeds of each of the accounts in
 * the board's membership and subscription lists.
 */
void database::add_comment_to_board( const account_name_type& sharer, const board_object& board, const comment_id_type& comment_id )
{ try {
   const auto& feed_idx = get_index< feed_index >().indices().get< by_feed >();
   const auto& comment_idx = get_index< feed_index >().indices().get< by_comment >();
   const board_member_object& board_member = get_board_member( board.name );
   
   feed_types feed_type = BOARD_FEED;
   bool subscriptions = true;

   switch( board.board_type )
   {
      case BOARD: 
         break;
      case GROUP:
      {
         feed_type = GROUP_FEED;
         break;
      }
      case EVENT:
      {
         feed_type = EVENT_FEED;
         break;
      }
   }
     
   if( subscriptions )
   {
      for( const account_name_type& account : board_member.subscribers ) 
      {
         uint32_t next_id = 0;
         auto last_feed = feed_idx.lower_bound( account );
         if( last_feed != feed_idx.end() && last_feed->account == account )
         {
            next_id = last_feed->account_feed_id + 1;
         }
         auto feed_itr = comment_idx.find( boost::make_tuple( comment_id, account ) );
         if( feed_itr == comment_idx.end() )   // Comment is not already in feed
         {
            create< feed_object >( [&]( feed_object& f )
            {
               f.account = account;
               f.comment = comment_id;
               f.feed_type = feed_type;
               f.account_feed_id = next_id;
            });
         }   
      } 
   }

   for( const account_name_type& account : board_member.members ) 
   {
      uint32_t next_id = 0;
      auto last_feed = feed_idx.lower_bound( account );
      if( last_feed != feed_idx.end() && last_feed->account == account )
      {
         next_id = last_feed->account_feed_id + 1;
      }
      auto feed_itr = comment_idx.find( boost::make_tuple( comment_id, account ) );
      if( feed_itr == comment_idx.end() )   // Comment is not already in feed
      {
         create< feed_object >( [&]( feed_object& f )
         {
            f.account = account;
            f.comment = comment_id;
            f.feed_type = feed_type;
            f.account_feed_id = next_id;
         });
      }   
   }
} FC_CAPTURE_AND_RETHROW( (sharer)(comment_id) ) }

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
   } );
}


void database::adjust_total_payout( const comment_object& cur, const asset& reward_created, const asset& curator_reward_value, const asset& beneficiary_reward_value )
{
   modify( cur, [&]( comment_object& c )
   {
      if( c.total_payout_value.symbol == reward_created.symbol )
         c.total_payout_value += reward_created;
         c.curator_payout_value += curator_reward_value;
         c.beneficiary_payout_value += beneficiary_reward_value;
   } );
}


} } //node::chain