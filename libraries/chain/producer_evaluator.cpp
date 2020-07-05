
#include <node/chain/node_evaluator.hpp>
#include <node/chain/database.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <cmath>

#include <node/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
//#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace node { namespace chain {


//==================================//
// === Block Producer Evaluators ===//
//==================================//


void producer_update_evaluator::do_apply( const producer_update_operation& o )
{ try {
   const account_name_type& signed_for = o.owner;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const auto& by_producer_name_idx = _db.get_index< producer_index >().indices().get< by_name >();
   auto producer_itr = by_producer_name_idx.find( o.owner );

   if( producer_itr != by_producer_name_idx.end() )
   {
      _db.modify( *producer_itr, [&]( producer_object& p )
      {
         if ( o.json.size() > 0 )
         {
            from_string( p.json, o.json );
         }
         if ( o.details.size() > 0 )
         {
            from_string( p.details, o.details );
         }
         if ( o.url.size() > 0 )
         {
            from_string( p.url, o.url );
         }
         p.latitude = o.latitude;
         p.longitude = o.longitude;
         p.signing_key = public_key_type( o.block_signing_key );
         p.props = o.props;
         p.active = o.active;
         p.last_updated = now;
      });
   }
   else
   {
      _db.create< producer_object >( [&]( producer_object& p )
      {
         p.owner = o.owner;
         if ( o.json.size() > 0 )
         {
            from_string( p.json, o.json );
         }
         if ( o.details.size() > 0 )
         {
            from_string( p.details, o.details );
         }
         if ( o.url.size() > 0 )
         {
            from_string( p.url, o.url );
         }
         p.latitude = o.latitude;
         p.longitude = o.longitude;
         p.signing_key = public_key_type( o.block_signing_key );
         p.created = now;
         p.last_updated = now;
         p.props = o.props;
         p.active = true;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void proof_of_work_evaluator::do_apply( const proof_of_work_operation& o )
{ try {
   const auto& work = o.work.get< x11_proof_of_work >();
   uint128_t target_pow = _db.pow_difficulty();
   account_name_type miner_account = work.input.miner_account;
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();
   fc::optional< signed_block > prev_block = _db.fetch_block_by_id( work.input.prev_block );

   FC_ASSERT( prev_block.valid(),
      "Proof of Work Prev block not found on node." );
   FC_ASSERT( prev_block->block_num() > props.last_irreversible_block_num,
      "Proof of Work done for block older than last irreversible block number." );
   FC_ASSERT( work.pow_summary < target_pow,
      "Insufficient work difficulty. Work: ${w}, Target: ${t} .", ("w",work.pow_summary)("t", target_pow) );

   uint128_t work_difficulty = ( 1 << 30 ) / work.pow_summary;

   const auto& accounts_by_name = _db.get_index< account_index >().indices().get< by_name >();
   auto acc_itr = accounts_by_name.find( miner_account );

   if( acc_itr == accounts_by_name.end() )
   {
      FC_ASSERT( o.new_owner_key.valid(),
         "New owner key is not valid." );

      public_key_type ok = public_key_type( *o.new_owner_key );

      _db.create< account_object >( [&]( account_object& acc )
      {
         acc.registrar = NULL_ACCOUNT;       // Create brand new account for miner
         acc.name = miner_account;
         acc.recovery_account = "";          // Highest voted producer at time of recovery.
         acc.secure_public_key = ok;
         acc.connection_public_key = ok;
         acc.friend_public_key = ok;
         acc.companion_public_key = ok;
         acc.created = now;
         acc.last_updated = now;
         acc.last_vote_time = now;
         acc.last_view_time = now;
         acc.last_share_time = now;
         acc.last_post = now;
         acc.last_root_post = now;
         acc.last_transfer_time = now;
         acc.last_activity_reward = now;
         acc.last_account_recovery = now;
         acc.mined = true;                     // Mined account creation
      });

      _db.create< account_authority_object >( [&]( account_authority_object& auth )
      {
         auth.account = miner_account;
         auth.owner_auth = authority( 1, ok, 1 );
         auth.active_auth = auth.owner_auth;
         auth.posting_auth = auth.owner_auth;
      });

      _db.create< producer_object >( [&]( producer_object& p )
      {
         p.owner = miner_account;
         p.props = o.props;
         p.signing_key = ok;
         p.mining_count = 0;
         p.mining_power = BLOCKCHAIN_PRECISION;
         p.last_mining_update = now;
      });
   }
   else
   {
      FC_ASSERT( !o.new_owner_key.valid(),
         "Cannot specify an owner key unless creating new mined account." );
      const producer_object* cur_producer = _db.find_producer( miner_account );
      FC_ASSERT( cur_producer != nullptr,
         "Account: ${p} must be active to mine proofs of work.",("p",miner_account) );
      FC_ASSERT( cur_producer->active,
         "Producer: ${p} must be active to mine proofs of work.",("p",miner_account) );
      
      _db.modify( *cur_producer, [&]( producer_object& p )
      {
         p.props = o.props;
      });
   }

   _db.modify( props, [&]( dynamic_global_property_object& p )
   {
      p.total_pow + work_difficulty;
   });

   _db.claim_proof_of_work_reward( miner_account );     // Rewards miner account from mining POW reward pool.
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void verify_block_evaluator::do_apply( const verify_block_operation& o )
{ try {
   const account_name_type& signed_for = o.producer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = _db.head_block_time();
   const producer_object& producer = _db.get_producer( o.producer );
   FC_ASSERT( producer.active, 
      "Account: ${s} must be active to verify blocks.",("s", o.producer) );

   fc::optional< signed_block > prev_block = _db.fetch_block_by_id( o.block_id );
   FC_ASSERT( prev_block.valid(),
      "Proof of Work Prev block not found on node." );
   uint64_t recent_block_num = prev_block->block_num();
   FC_ASSERT( recent_block_num > props.last_irreversible_block_num,
      "Verify Block done for block older than last irreversible block number." );
   FC_ASSERT( recent_block_num > props.last_committed_block_num,
      "Verify Block done for block older than last committed block number." );

   const producer_schedule_object& pso = _db.get_producer_schedule();
   const transaction_id_type& txn_id = _db.get_current_transaction_id();
   vector< account_name_type > top_voting_producers = pso.top_voting_producers;
   vector< account_name_type > top_mining_producers = pso.top_mining_producers;

   FC_ASSERT( std::find( top_voting_producers.begin(), top_voting_producers.end(), o.producer ) != top_voting_producers.end() ||
      std::find( top_mining_producers.begin(), top_mining_producers.end(), o.producer ) != top_mining_producers.end(),
      "Producer must be a top producer or miner to publish a block verification." );

   const auto& valid_idx = _db.get_index< block_validation_index >().indices().get< by_producer_height >();
   auto valid_itr = valid_idx.find( boost::make_tuple( o.producer, recent_block_num ) );
   
   if( valid_itr == valid_idx.end() )     // New verification object at this height.
   {
      _db.create< block_validation_object >( [&]( block_validation_object& bvo )
      {
         bvo.producer = o.producer;
         bvo.block_id = o.block_id;
         bvo.block_height = recent_block_num;
         bvo.created = now;
         bvo.committed = false;
         bvo.verify_txn = txn_id;
         bvo.commit_txn = transaction_id_type();
      });
   }
   else   // Existing verifcation exists, Changing uncommitted block_id.
   {
      const block_validation_object& val = *valid_itr;

      FC_ASSERT( val.block_id != o.block_id,
         "Operation must change an uncommitted block id." );
      FC_ASSERT( val.committed == false,
         "CANNOT ALTER COMMITED VALIDATION. PRODUCER: ${p) BLOCK_ID1: ${a} BLOCK_ID2: ${b} BLOCK HEIGHT: ${h} ATTEMPTED PRODUCER VIOLATION DETECTED.", 
         ("p", o.producer)("a", val.block_id)("b", o.block_id)("h", recent_block_num) );

      _db.modify( val, [&]( block_validation_object& bvo )
      {
         bvo.block_id = o.block_id;
         bvo.verify_txn = txn_id;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void commit_block_evaluator::do_apply( const commit_block_operation& o )
{ try {
   const account_name_type& signed_for = o.producer;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const dynamic_global_property_object& props = _db.get_dynamic_global_properties();
   time_point now = _db.head_block_time();
   const producer_object& producer = _db.get_producer( o.producer );

   fc::optional< signed_block > prev_block = _db.fetch_block_by_id( o.block_id );
   FC_ASSERT( prev_block.valid(),
      "Proof of Work Prev block not found on node." );
   uint64_t recent_block_num = prev_block->block_num();
   FC_ASSERT( recent_block_num > props.last_irreversible_block_num,
      "Verify Block done for block older than last irreversible block number." );
   FC_ASSERT( recent_block_num > props.last_committed_block_num,
      "Verify Block done for block older than last committed block number." );
   
   const producer_schedule_object& pso = _db.get_producer_schedule();
   const auto& valid_idx = _db.get_index< block_validation_index >().indices().get< by_producer_height >();
   auto valid_itr = valid_idx.find( boost::make_tuple( o.producer, recent_block_num ) );

   FC_ASSERT( pso.is_top_producer( o.producer ),
      "Producer must be a top producer or miner to publish a block commit." );
   FC_ASSERT( valid_itr != valid_idx.end(),
      "Please broadcast verify block transaction before commit block transaction." );
   
   const block_validation_object& val = *valid_itr;

   FC_ASSERT( val.block_id == o.block_id, 
      "Different block ID than the verification ID, please update verification." );

   asset stake = _db.get_staked_balance( o.producer, SYMBOL_COIN );

   FC_ASSERT( stake >= o.commitment_stake,
      "Producer has insufficient staked balance to provide a commitment stake for the specified amount." );

   flat_set< account_name_type > verifiers;

   for( transaction_id_type txn : o.verifications )  // Ensure that all included verification transactions are valid.
   {
      annotated_signed_transaction verify_txn = _db.get_transaction( txn );
      FC_ASSERT( verify_txn.operations.size(), 
         "Transaction ID ${t} has no operations.", ("t", txn) );

      for( auto op : verify_txn.operations )
      {
         if( op.which() == operation::tag< verify_block_operation >::value )
         {
            const verify_block_operation& verify_op = op.get< verify_block_operation >();
            verify_op.validate();
            const producer_object& verify_wit = _db.get_producer( verify_op.producer );
            if( verify_op.block_id == o.block_id && 
               pso.is_top_producer( verify_wit.owner ) )
            {
               verifiers.insert( verify_wit.owner );
            }
         }
      }
   }

   FC_ASSERT( verifiers.size() >=( IRREVERSIBLE_THRESHOLD * ( DPOS_VOTING_PRODUCERS + POW_MINING_PRODUCERS ) / PERCENT_100 ),
      "Insufficient Unique Concurrent Valid Verifications for commit transaction. Please await further verifications from block producers." );

   const transaction_id_type& txn_id = _db.get_current_transaction_id();

   _db.modify( val, [&]( block_validation_object& bvo )
   {
      bvo.committed = true;                            // Verification cannot be altered after committed. 
      bvo.commit_time = now;                           // Fastest by commit time are eligible for validation reward.
      bvo.commitment_stake = o.commitment_stake;       // Reward is weighted by stake committed. 
      bvo.verifications = o.verifications;
      bvo.verifiers = verifiers;
      bvo.commit_txn = txn_id;
   });

   _db.modify( producer, [&]( producer_object& p )
   {
      p.last_commit_height = recent_block_num;
      p.last_commit_id = o.block_id;
   });
   
} FC_CAPTURE_AND_RETHROW( ( o ) ) }

/**
 * In the event of block producers signing conflicting commitment transactions
 * to claim validation rewards without commiting to a single chain,
 * any reporting account can declare two contradictory signed transactions
 * from a top producer in which they sign conflicting block ids at the same height.
 */
void producer_violation_evaluator::do_apply( const producer_violation_operation& o )
{ try {
   const account_name_type& signed_for = o.reporter;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_network( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const producer_schedule_object& pso = _db.get_producer_schedule();
   const chain_id_type& chain_id = CHAIN_ID;

   signed_transaction first_trx = fc::raw::unpack< signed_transaction >( o.first_trx );
   signed_transaction second_trx = fc::raw::unpack< signed_transaction >( o.second_trx );
   first_trx.validate();
   second_trx.validate();
   FC_ASSERT( first_trx.operations.size(), 
      "Transaction ID ${t} has no operations.", ("t", first_trx.id() ) );
   FC_ASSERT( second_trx.operations.size(), 
      "Transaction ID ${t} has no operations.", ("t", second_trx.id() ) );

   // Check signatures on transactions to ensure that they are signed by the producers
   
   auto get_active  = [&]( const string& name ) { return authority( _db.get< account_authority_object, by_account >( name ).active_auth ); };
   auto get_owner   = [&]( const string& name ) { return authority( _db.get< account_authority_object, by_account >( name ).owner_auth );  };
   auto get_posting = [&]( const string& name ) { return authority( _db.get< account_authority_object, by_account >( name ).posting_auth );  };

   first_trx.verify_authority( chain_id, get_active, get_owner, get_posting, MAX_SIG_CHECK_DEPTH );
   second_trx.verify_authority( chain_id, get_active, get_owner, get_posting, MAX_SIG_CHECK_DEPTH );
   
   uint64_t first_height = 0;
   uint64_t second_height = 0;
   block_id_type first_block_id = block_id_type();
   block_id_type second_block_id = block_id_type();
   account_name_type first_producer = account_name_type();
   account_name_type second_producer = account_name_type();
   asset first_stake = asset();
   asset second_stake = asset();

   for( operation op : first_trx.operations )
   {
      if( op.which() == operation::tag< commit_block_operation >::value )
      {
         const commit_block_operation& commit_op = op.get< commit_block_operation >();
         commit_op.validate();
         first_block_id = commit_op.block_id;
         first_height = protocol::block_header::num_from_id( commit_op.block_id );
         first_producer = commit_op.producer;
         first_stake = commit_op.commitment_stake;
         break;
      }
   }

   for( operation op : second_trx.operations )
   {
      if( op.which() == operation::tag< commit_block_operation >::value )
      {
         const commit_block_operation& commit_op = op.get< commit_block_operation >();
         commit_op.validate();
         second_block_id = commit_op.block_id;
         second_height = protocol::block_header::num_from_id( commit_op.block_id );
         second_producer = commit_op.producer;
         second_stake = commit_op.commitment_stake;
         break;
      }
   }

   FC_ASSERT( first_height != 0 && second_height != 0 && first_height == second_height,
      "Producer violation claim must include two valid commit block operations at the same height." );
   FC_ASSERT( first_block_id != block_id_type() && second_block_id != block_id_type() && first_block_id != second_block_id,
      "Producer violation claim must include two valid commit block operations with contradictory block IDs." );
   FC_ASSERT( first_producer != account_name_type() && second_producer != account_name_type() && first_producer == second_producer,
      "Producer violation claim must include two valid commit block operations from the same producer." );
   FC_ASSERT( first_stake != asset() && second_stake != asset() &&
      first_stake >= asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ) &&
      second_stake >= asset( BLOCKCHAIN_PRECISION, SYMBOL_COIN ),
      "Producer violation claim must include two valid commit block operations with at least 1 Core asset staked." );
   FC_ASSERT( pso.is_top_producer( first_producer ),
      "Violating producer is not a current top producer." );

   asset available_stake = _db.get_staked_balance( first_producer, SYMBOL_COIN );
   asset commitment_stake = std::max( first_stake, second_stake );
   asset forfeited_stake = std::min( commitment_stake, available_stake );

   const auto& vio_idx = _db.get_index< commit_violation_index >().indices().get< by_producer_height >();
   auto vio_itr = vio_idx.find( boost::make_tuple( first_producer, first_height ) );
   
   FC_ASSERT( vio_itr == vio_idx.end(),
      "Violation by producer: ${p} at block height: ${h} has already been declared and claimed by reporter: ${r}.",
      ("p", first_producer )("h", first_height)("r", vio_itr->reporter) );
   
   _db.create< commit_violation_object >( [&]( commit_violation_object& cvo )
   {
      cvo.reporter = o.reporter;      // Record violation event, ensuring maximum of one claim per producer per height.
      cvo.producer = first_producer;
      cvo.first_trx = first_trx;
      cvo.second_trx = second_trx;
      cvo.block_height = first_height;
      cvo.created = now;
      cvo.forfeited_stake = forfeited_stake;
   });

   _db.adjust_staked_balance( first_producer, -forfeited_stake );    // Transfer forfeited stake amount from producer to reporter.
   _db.adjust_staked_balance( o.reporter, forfeited_stake );
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain