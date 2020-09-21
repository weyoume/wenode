#include <node/protocol/types.hpp>
#include <node/protocol/authority.hpp>
#include <node/protocol/transaction.hpp>
#include <node/chain/database.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>

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
#include <fc/io/json.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>

namespace node { namespace chain {


const producer_object& database::get_producer( const account_name_type& name ) const
{ try {
   return get< producer_object, by_name >( name );
} FC_CAPTURE_AND_RETHROW( (name) ) }

const producer_object* database::find_producer( const account_name_type& name ) const
{
   return find< producer_object, by_name >( name );
}

const producer_vote_object& database::get_producer_vote( const account_name_type& account, const account_name_type& producer )const
{ try {
   return get< producer_vote_object, by_account_producer >( boost::make_tuple( account, producer ) );
} FC_CAPTURE_AND_RETHROW( (account)(producer) ) }

const producer_vote_object* database::find_producer_vote( const account_name_type& account, const account_name_type& producer )const
{
   return find< producer_vote_object, by_account_producer >( boost::make_tuple( account, producer ) );
}

const block_validation_object& database::get_block_validation( const account_name_type& producer, uint64_t height ) const
{ try {
   return get< block_validation_object, by_producer_height >( boost::make_tuple( producer, height) );
} FC_CAPTURE_AND_RETHROW( (producer)(height) ) }

const block_validation_object* database::find_block_validation( const account_name_type& producer, uint64_t height ) const
{
   return find< block_validation_object, by_producer_height >( boost::make_tuple( producer, height) );
}

const producer_schedule_object& database::get_producer_schedule()const
{ try {
   return get< producer_schedule_object, by_id >( 0 );
} FC_CAPTURE_AND_RETHROW() }


x11 database::pow_difficulty()const
{
   return get_producer_schedule().pow_target_difficulty;
}

const median_chain_property_object& database::get_median_chain_properties()const
{ try {
   return get< median_chain_property_object, by_id >( 0 );
} FC_CAPTURE_AND_RETHROW() }


uint32_t database::producer_participation_rate()const
{
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   return uint64_t(PERCENT_100) * props.recent_slots_filled.popcount() / 128;
}


account_name_type database::get_scheduled_producer( uint64_t slot_num )const
{
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const producer_schedule_object& pso = get_producer_schedule();
   uint64_t current_aslot = props.current_aslot + slot_num;
   account_name_type scheduled_producer = pso.current_shuffled_producers[ current_aslot % pso.num_scheduled_producers ];
   return scheduled_producer;
}


fc::time_point database::get_slot_time( uint64_t slot_num )const
{
   if( slot_num == 0 )
   {
      return fc::time_point();
   }

   int64_t interval = BLOCK_INTERVAL.count();

   const dynamic_global_property_object& dgpo = get_dynamic_global_properties();

   if( dgpo.head_block_number == 0 )
   {
      // n.b. first block is at genesis_time plus one block interval
      fc::time_point genesis_time = dgpo.time;
      return genesis_time + fc::microseconds( slot_num * interval );
   }

   // "slot 0" is head_slot_time
   // "slot 1" is head_slot_time

   int64_t head_block_abs_slot = dgpo.time.time_since_epoch().count() / interval;
   fc::time_point head_slot_time = fc::time_point( fc::microseconds( head_block_abs_slot * interval ) );
   fc::time_point slot_time = head_slot_time + fc::microseconds( slot_num * interval );
   return slot_time;
}


uint64_t database::get_slot_at_time( fc::time_point when )const
{
   fc::time_point first_slot_time = get_slot_time( 1 );

   if( when < first_slot_time ) 
   {
      return 0;
   }

   uint64_t slot_number = ( ( when - first_slot_time ).count() / BLOCK_INTERVAL.count() ) + 1;
   return slot_number;
}


void database::update_producer_set()
{ try {
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;

   process_update_producer_set();

} FC_CAPTURE_AND_RETHROW() }


/**
 * Aligns producer votes in order of highest to lowest, with continual ordering.
 */
void database::update_producer_votes( const account_object& account )
{
   const auto& vote_idx = get_index< producer_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name )
   {
      const producer_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( producer_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }

   modify( account, [&]( account_object& a )
   {
      a.producer_vote_count = ( new_vote_rank - 1 );
   });
}

/**
 * Aligns producer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_producer_votes( const account_object& account, const account_name_type& producer, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< producer_vote_index >().indices().get< by_account_rank >();
   auto vote_itr = vote_idx.lower_bound( account.name );

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name )
   {
      const producer_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( producer_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< producer_vote_object >([&]( producer_vote_object& v )
   {
      v.account = account.name;
      v.producer = producer;
      v.vote_rank = input_vote_rank;
   });

   modify( account, [&]( account_object& a )
   {
      a.producer_vote_count = ( new_vote_rank - 1 );
   });
}


void database::process_update_producer_set()
{ try {
   const producer_schedule_object& pso = get_producer_schedule();
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const median_chain_property_object& median_props = get_median_chain_properties();
   const auto& producer_idx = get_index< producer_index >().indices().get< by_voting_power >();
   auto producer_itr = producer_idx.begin();
   uint128_t total_producer_voting_power = 0;
   
   while( producer_itr != producer_idx.end() )
   {
      total_producer_voting_power += update_producer( *producer_itr, pso, props, median_props ).value;
      ++producer_itr;
   }

   modify( pso, [&]( producer_schedule_object& pso )
   {
      pso.total_producer_voting_power = total_producer_voting_power;
   });

   // ilog( "Updated Producer set." );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the voting power and vote count of a producer
 * and returns the total voting power supporting the producer.
 */
share_type database::update_producer( const producer_object& producer, const producer_schedule_object& pso,
   const dynamic_global_property_object& props, const median_chain_property_object& median_props )
{ try {
   const auto& producer_vote_idx = get_index< producer_vote_index >().indices().get< by_producer_account >();
   auto producer_vote_itr = producer_vote_idx.lower_bound( producer.owner );
   price equity_price = props.current_median_equity_price;
   time_point now = head_block_time();
   share_type voting_power = 0;
   uint32_t vote_count = 0;

   while( producer_vote_itr != producer_vote_idx.end() && 
      producer_vote_itr->producer == producer.owner )
   {
      const producer_vote_object& vote = *producer_vote_itr;
      const account_object& voter = get_account( vote.account );
      share_type weight = get_voting_power( producer_vote_itr->account, equity_price );
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.
      voting_power += share_type( weight.value >> vote.vote_rank );
      vote_count++;
      ++producer_vote_itr;
   }

   modify( producer, [&]( producer_object& p )
   {
      p.voting_power = voting_power;
      p.vote_count = vote_count;
      auto delta_pos = p.voting_power.value * (pso.current_voting_virtual_time - p.voting_virtual_last_update);
      p.voting_virtual_position += delta_pos;
      p.voting_virtual_scheduled_time = p.voting_virtual_last_update + (VIRTUAL_SCHEDULE_LAP_LENGTH - p.voting_virtual_position)/(p.voting_power.value+1);
      /** producers with a low number of votes could overflow the time field and end up with a scheduled time in the past */
      if( p.voting_virtual_scheduled_time < pso.current_voting_virtual_time ) 
      {
         p.voting_virtual_scheduled_time = fc::uint128::max_value();
      }
      p.decay_weights( now, median_props );
      p.voting_virtual_last_update = pso.current_voting_virtual_time;
   });

   return voting_power;

} FC_CAPTURE_AND_RETHROW() }



void database::update_signing_producer( const producer_object& signing_producer, const signed_block& new_block )
{ try {
   const dynamic_global_property_object& props = get_dynamic_global_properties();
   uint64_t new_block_aslot = props.current_aslot + get_slot_at_time( new_block.timestamp );

   modify( signing_producer, [&]( producer_object& p )
   {
      p.last_aslot = new_block_aslot;
      p.last_confirmed_block_num = new_block.block_num();
      p.total_blocks++;
   });
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the difficulty required for the network 
 * to track the targeted proof of work production rate.
 */
void database::update_proof_of_work_target()
{ try {
   if( (head_block_num() % POW_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;

   const median_chain_property_object& median_props = get_median_chain_properties();
   const producer_schedule_object& pso = get_producer_schedule();
   time_point now = head_block_time();

   modify( pso, [&]( producer_schedule_object& pso )
   {
      pso.decay_pow( now, median_props );
   });

   uint128_t recent_pow = pso.recent_pow;        // Amount of proofs of work, times block precision, decayed over 7 days
   x11 init = pso.pow_target_difficulty;

   if( recent_pow > 0 )
   {
      ilog( "=======================================================" );
      ilog( "========== Updating Proof of Work Difficulty ==========" );
      ilog( "=======================================================" );

      ilog( "   --> Recent POW:     ${r}",("r",recent_pow));
      uint128_t base = uint128_t::max_value();
      ilog( "   --> Base:           ${b}",("b",base));
      uint128_t dif = std::max( init.to_uint128(), uint128_t( 10 ));
      ilog( "   --> Dif:            ${b}",("b",dif));
      uint128_t coefficient = std::max( base / dif, uint128_t( 10 ));
      ilog( "   --> Coefficient:    ${b}",("b",coefficient));
      uint128_t target_pow = ( BLOCKCHAIN_PRECISION.value * median_props.pow_decay_time.to_seconds() ) / median_props.pow_target_time.to_seconds();
      ilog( "   --> Target POW:     ${b}",("b",target_pow));
      uint128_t mult = std::max( coefficient * recent_pow, target_pow );
      ilog( "   --> Mult:           ${b}",("b",mult));
      uint128_t div = std::max( mult / target_pow, uint128_t( 10 ));
      ilog( "   --> Div:            ${b}",("b",div));
      uint128_t target = base / div;
      ilog( "   --> Target:         ${b}",("b",target));
      x11 pow_target_difficulty = x11( target );
      ilog( "   --> Init:           ${i}",("i",init));
      ilog( "   --> Final:          ${d}",("d",pow_target_difficulty));

      modify( pso, [&]( producer_schedule_object& pso )
      {
         pso.pow_target_difficulty = pow_target_difficulty;
      });
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Provides a producer account with a proof of work mining reward
 * and increments their mining power level for block production selection.
 * 
 * The top mining accounts are selected randomly once per round to 
 * produce a block at their scheduled time.
 */
void database::claim_proof_of_work_reward( const account_name_type& miner )
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = head_block_time();
   const producer_schedule_object& pso = get_producer_schedule();
   const producer_object& producer = get_producer( miner );

   modify( producer, [&]( producer_object& p )
   {
      p.mining_power += BLOCKCHAIN_PRECISION;
      p.mining_count++;
      p.last_pow_time = now;
      p.decay_weights( now, median_props );
   });

   modify( pso, [&]( producer_schedule_object& pso )
   {
      pso.recent_pow += BLOCKCHAIN_PRECISION.value;
      pso.decay_pow( now, median_props );
   });

   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   while( fund_itr != fund_idx.end() )
   {
      const asset_reward_fund_object& reward_fund = *fund_itr;
      asset pow_reward = reward_fund.work_reward_balance;

      modify( reward_fund, [&]( asset_reward_fund_object& r )
      {
         r.adjust_work_reward_balance( -pow_reward );
      });

      adjust_reward_balance( miner, pow_reward );
      adjust_pending_supply( -pow_reward );

      ++fund_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes the transaction stake reward to all block producers according 
 * to the amount of stake weighted transactions included in blocks. 
 * Each transaction included in a block adds the size of the transaction 
 * multipled by the stake weight of its creator.
 */
void database::process_txn_stake_rewards()
{ try {
   if( (head_block_num() % TXN_STAKE_BLOCK_INTERVAL) != 0 )    // Runs once per Day
      return;

   // ilog( "Process Transaction Stake reward" );

   const auto& producer_idx = get_index< producer_index >().indices().get< by_txn_stake_weight >();
   auto producer_itr = producer_idx.begin();
    
   flat_map< account_name_type, uint128_t > stake_map;
   uint128_t total_stake_shares = 0;
   
   while( producer_itr != producer_idx.end() &&
      producer_itr->recent_txn_stake_weight > uint128_t( 0 ) )
   {
      uint128_t stake_shares = producer_itr->recent_txn_stake_weight;         // Get the recent txn stake for each producer

      if( stake_shares > uint128_t( 0 ) )
      {
         total_stake_shares += stake_shares;
         stake_map[ producer_itr->owner ] = stake_shares;
      }
      ++producer_itr;
   }

   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   while( fund_itr != fund_idx.end() && 
      total_stake_shares > uint128_t( 0 ) )
   {
      const asset_reward_fund_object& reward_fund = *fund_itr;
      asset txn_stake_reward = reward_fund.txn_stake_reward_balance;     // Record the opening balance of the transaction stake reward fund
      asset distributed = asset( 0, reward_fund.symbol );

      for( auto b : stake_map )
      {
         uint128_t r_shares = ( uint128_t( txn_stake_reward.amount.value ) * b.second ) / total_stake_shares; 
         asset stake_reward = asset( share_type( int64_t( r_shares.to_uint64() ) ), reward_fund.symbol );
         adjust_reward_balance( b.first, stake_reward );       // Pay transaction stake reward to each block producer proportionally.
         distributed += stake_reward;
   
         // ilog( "Processing Txn Stake Reward for account: ${a} Reward: ${r}", ("a",b.first)("r",stake_reward));
      }

      modify( reward_fund, [&]( asset_reward_fund_object& r )
      {
         r.adjust_txn_stake_reward_balance( -distributed );     // Remove the distributed amount from the reward pool.
      });

      adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.

      ++fund_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Distributes the block reward for validating blocks to producers
 * and miners according to the stake weight of their commitment transactions
 * upon the block becoming irreversible after majority of producers have created
 * a block on top of it.
 * 
 * This enables nodes to have a lower finality time in
 * cases where producers a majority of producers commit to a newly 
 * created block before it becomes irreversible.
 * Nodes will treat the blocks that they commit to as irreversible when
 * greater than two third of producers also commit to the same block.
 */
void database::process_validation_rewards()
{ try {
   // ilog( "Process Validation rewards" );

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   const auto& valid_idx = get_index< block_validation_index >().indices().get< by_height_stake >();
   auto valid_itr = valid_idx.lower_bound( props.last_irreversible_block_num );
    
   flat_map < account_name_type, share_type > validation_map;
   share_type total_validation_shares = 0;

   while( valid_itr != valid_idx.end() &&
      valid_itr->block_height == props.last_irreversible_block_num &&
      valid_itr->commitment_stake.amount >= BLOCKCHAIN_PRECISION ) 
   {
      share_type validation_shares = valid_itr->commitment_stake.amount;       // Get the commitment stake for each producer

      if( validation_shares > 0 )
      {
         total_validation_shares += validation_shares;
         validation_map[ valid_itr->producer ] = validation_shares;
      }
      ++valid_itr;
   }

   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   while( fund_itr != fund_idx.end() && total_validation_shares > 0 )
   {
      const asset_reward_fund_object& reward_fund = *fund_itr;
      asset validation_reward = reward_fund.validation_reward_balance;     // Record the opening balance of the validation reward fund
      asset distributed = asset( 0, reward_fund.symbol );

      for( auto b : validation_map )
      {
         asset validation_reward_split = ( validation_reward * b.second ) / total_validation_shares; 
         adjust_reward_balance( b.first, validation_reward );       // Pay transaction validation reward to each block producer proportionally.
         distributed += validation_reward_split;

         ilog( "Processing Validation Reward for account: ${a} \n ${r} \n",
            ("a",b.first)("r",validation_reward_split ) );
      }

      modify( reward_fund, [&]( asset_reward_fund_object& r )
      {
         r.adjust_validation_reward_balance( -distributed );     // Remove the distributed amount from the reward pool.
      });

      adjust_pending_supply( -distributed );   // Deduct distributed amount from pending supply.

      ++fund_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/** 
 * Rewards producers when they have the current highest accumulated activity stake. 
 * 
 * Each time an account produces an activity reward transaction,
 * they implicitly nominate their highest voted producer to receive a daily vote as their Prime Producer.
 * Award is distributed every eight hours to the leader by activity stake.
 * This incentivizes producers to campaign to achieve prime producer designation with 
 * high stake, active accounts, in a competitive manner. 
 */
void database::process_producer_activity_rewards()
{ try {
   if( (head_block_num() % POA_BLOCK_INTERVAL ) != 0 )    // Runs once per 8 hours.
      return;

   // ilog( "Process Producer Activity Rewards" );
   
   const auto& producer_idx = get_index< producer_index >().indices().get< by_activity_stake >();
   auto producer_itr = producer_idx.begin();

   const auto& fund_idx = get_index< asset_reward_fund_index >().indices().get< by_symbol >();
   auto fund_itr = fund_idx.begin();

   if( producer_itr != producer_idx.end() )    // Get Top producer by activity stake
   {
      const producer_object& prime_producer = *producer_itr;

      modify( prime_producer, [&]( producer_object& p )
      {
         p.accumulated_activity_stake = 0;     // Reset activity stake for top producer.
      });

      while( fund_itr != fund_idx.end() )
      {
         const asset_reward_fund_object& reward_fund = *fund_itr;
         asset poa_reward = reward_fund.producer_activity_reward_balance;    // Record the opening balance of the producer activity reward fund.

         modify( reward_fund, [&]( asset_reward_fund_object& r )
         {
            r.adjust_producer_activity_reward_balance( -poa_reward );        // Remove the distributed amount from the reward pool.
         });

         adjust_reward_balance( prime_producer.owner, poa_reward );          // Pay producer activity reward to the producer with the highest accumulated activity stake.
         adjust_pending_supply( -poa_reward );                               // Deduct distributed amount from pending supply.

         ++fund_itr;
      }
   }
} FC_CAPTURE_AND_RETHROW() }


} } // node::chain