#pragma once

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/node_object_types.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace node { namespace chain {

   using node::protocol::chain_properties;
   using node::protocol::digest_type;
   using node::protocol::public_key_type;
   using node::protocol::version;
   using node::protocol::hardfork_version;
   using node::protocol::price;
   using node::protocol::asset;
   using node::protocol::asset_symbol_type;

   /**
    * All witnesses with at least 1% net positive approval and
    * at least 2 weeks old are able to participate in block
    * production.
    * These fields are used for the witness scheduling algorithm which uses
    * virtual time to ensure that all witnesses are given proportional time
    * for producing blocks.
    * voting power is used to determine witness speed, and recent_pow is used to determine miner speed.
    * The virtual_scheduled_time is the expected time at which this witness should complete a virtual lap.
    * virtual_scheduled_time = virtual_last_update + (1000*MAXVOTES - virtual_position) / vote
    * 
    * Every time the number of votes changes the virtual_position and virtual_scheduled_time must
    * update. There is a global current virtual_scheduled_time which gets updated every time a witness is scheduled.
    * virtual_position       = virtual_position + votes * (virtual_current_time - virtual_last_update)
    * virtual_last_update    = virtual_current_time
    * votes                  += delta_vote
    * virtual_scheduled_time = virtual_last_update + (1000*MAXVOTES - virtual_position) / vote
    */

   class witness_object : public object< witness_object_type, witness_object >
   {
      witness_object() = delete;

      public:
         enum witness_schedule_type
         {
            top_witness,
            additional_witness,
            top_miner,
            additional_miner,
            none
         };

         template< typename Constructor, typename Allocator >
         witness_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                      id;

         account_name_type            owner;                            // The name of the account that has authority over this witness.

         bool                         active;                           // True if the witness is actively seeking to produce blocks, set false to deactivate the witness and remove from production.

         witness_schedule_type        schedule = none;                  // How the witness was scheduled the last time it was scheduled.

         uint64_t                     last_confirmed_block_num = 0;     // Number of the last block that was successfully produced by this witness. 

         shared_string                details;                          // Witness or miner's details, explaining who they are, machine specs, capabilties.

         shared_string                url;                              // The witnesses or miners URL explaining their details.

         shared_string                json;                             // The witnesses or miners json metadata.

         double                       latitude;                         // Latitude co-ordinates of the witness.

         double                       longitude;                        // Longitude co-ordinates of the witness.

         public_key_type              signing_key;                      // The key used to sign blocks on behalf of this witness or miner.

         time_point                   created;                          // The time the witness was created.

         uint32_t                     last_commit_height;               // Block height that has been most recently committed by the producer

         block_id_type                last_commit_id;                   // Block ID of the height that was most recently committed by the producer. 

         uint32_t                     total_blocks = 0;                 // Accumulated number of blocks produced.

         share_type                   voting_power = 0;                 // The total weighted voting power that supports the witness. 

         uint32_t                     vote_count = 0;                   // The number of accounts that have voted for the witness.

         share_type                   mining_power = 0;                 // The amount of proof of work difficulty accumulated by the miner over the prior 7 days.

         uint32_t                     mining_count = 0;                 // Accumulated number of proofs of work published.

         time_point                   last_mining_update;               // Time that the account last updated its mining power.

         time_point                   last_pow_time;                    // Time that the miner last created a proof of work.

         share_type                   recent_txn_stake_weight = 0;      // Rolling average Amount of transaction stake weight contained that the producer has included in blocks over the prior 7 days.

         time_point                   last_txn_stake_weight_update;     // Time that the recent bandwith and txn stake were last updated.

         uint128_t                    accumulated_activity_stake = 0;   // Recent amount of activity reward stake for the prime witness. 

         uint32_t                     total_missed = 0;                 // Number of blocks missed recently.

         uint64_t                     last_aslot = 0;                   // Last absolute slot that the witness was assigned to produce a block.

         chain_properties             props;                            // The chain properties object that the witness currently proposes for global network variables
         
         uint128_t                    witness_virtual_last_update;

         uint128_t                    witness_virtual_position;

         uint128_t                    witness_virtual_scheduled_time = fc::uint128::max_value();

         uint128_t                    miner_virtual_last_update;

         uint128_t                    miner_virtual_position;

         uint128_t                    miner_virtual_scheduled_time = fc::uint128::max_value();
         
         digest_type                  last_work;

         version                      running_version;  // This field represents the WeYouMe blockchain version the witness is running.

         hardfork_version             hardfork_version_vote;

         time_point                   hardfork_time_vote = GENESIS_TIME;

         void                         witness_object::decay_weights( time_point now, const witness_schedule_object& wso )
         {
            mining_power -= ( ( mining_power * ( now - last_mining_update ).to_seconds() ) / wso.median_props.pow_decay_time.to_seconds() );
            recent_txn_stake_weight -= ( recent_txn_stake_weight * ( now - last_txn_stake_weight_update ).to_seconds() ) / wso.median_props.txn_stake_decay_time.to_seconds();
            last_mining_update = now;
            last_txn_stake_weight_update = now;
         }
   };


   class witness_vote_object : public object< witness_vote_object_type, witness_vote_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         witness_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         witness_vote_object(){}

         id_type                id;

         account_name_type      witness;

         account_name_type      account;

         uint16_t               vote_rank;   // the ordered rank to which the witness is supported, with 1 being the highest voted witness, and increasing for others.
   };


   class witness_schedule_object : public object< witness_schedule_object_type, witness_schedule_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         witness_schedule_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         witness_schedule_object(){}

         id_type                                           id;

         chain_properties                                  median_props;                    // Median of all witness selected blockchain properties.

         uint128_t                                         current_witness_virtual_time;    // Tracks the time used for block producer additional selection

         uint128_t                                         current_miner_virtual_time;      // Tracks the time used for block producer additional selection

         uint32_t                                          next_shuffle_block_num = 1;      //

         fc::array< account_name_type, TOTAL_PRODUCERS >   current_shuffled_producers;

         uint128_t                                         total_witness_voting_power;

         vector< account_name_type >                       top_witnesses;

         vector< account_name_type >                       top_miners;

         uint8_t                                           num_scheduled_producers = 1;

         uint128_t                                         pow_target_difficulty = -1;      //

         uint128_t                                         recent_pow;                      // Rolling average amount of blocks (x prec) mined in the last 7 days.

         time_point                                        last_pow_update;                 // Time that the recent POW was last updated and decayed

         version                                           majority_version;

         uint8_t                                           dpos_witness_producers = DPOS_WITNESS_PRODUCERS;

         uint8_t                                           dpos_witness_additional_producers = DPOS_WITNESS_ADDITONAL;

         uint8_t                                           pow_miner_producers = POW_MINER_PRODUCERS;

         uint8_t                                           pow_miner_additional_producers = POW_MINER_ADDITIONAL;
         
         uint8_t                                           hardfork_required_witnesses = HARDFORK_REQUIRED_WITNESSES;

         bool     is_top_producer( const account_name_type& producer )const    // finds if a given producer name is in the top witnesses or miners set. 
         {
            return (std::find( top_witnesses.begin(), top_witnesses.end(), producer) != top_witnesses.end() ||
            std::find( top_miners.begin(), top_miners.end(), producer) != top_miners.end());
         }

         bool     is_top_witness( const account_name_type& producer )const    // finds if a given producer name is in the top witnesses set. 
         {
            return std::find( top_witnesses.begin(), top_witnesses.end(), producer) != top_witnesses.end();
         }

         bool     is_top_miner( const account_name_type& producer )const    // finds if a given producer name is in the top miners set. 
         {
            return std::find( top_miners.begin(), top_miners.end(), producer) != top_miners.end();
         }

         void       witness_schedule_object::decay_pow( time_point now )
         {
            recent_pow -= ( ( recent_pow * ( now - last_pow_update ).to_seconds() ) / median_props.pow_decay_time.to_seconds() );
            last_pow_update = now;
         }
   };

   /**
    * Block Validation Objects are used by block producers to place a committment
    * stake on recent blocks, before they become irreversible, in order to increase the 
    * speed of achieving consensus finality on block history.
    * Block producers earn an additional share of the block reward when they are in the first 
    * two thirds of producers to commit to a block id at a given height.
    * The reward is distributed to all producers upon the block becoming irreversible
    * to all producers that have commitments at a given height.
    * All producers that include commitments in the block that exceeds two thirds are included
    * in reward distribution.
    * All nodes will use the greater of the last irreversible block, or the last commited block when
    * updating the state of their block logs. 
    * This enables an optimal block finality time of two blocks, with rapid participation of
    * producers to validate and commit to new blocks as they are produced.
    */
   class block_validation_object : public object< block_validation_object_type, block_validation_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         block_validation_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         block_validation_object(){}

         id_type                          id;

         account_name_type                producer;

         block_id_type                    block_id;

         uint32_t                         height;

         time_point                       created; 

         flat_set<transaction_id_type>    verifications;

         flat_set<account_name_type>      verifiers;

         bool                             committed = false;

         time_point                       commit_time = fc::time_point::min();

         asset                            commitment_stake;   // Stake must be at least 1 Core asset.
   };


   class commit_violation_object : public object< commit_violation_object_type, commit_violation_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         commit_violation_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         commit_violation_object(){}

         id_type                          id;

         account_name_type                reporter;

         account_name_type                producer;

         uint32_t                         height;

         signed_transaction               first_trx;

         signed_transaction               second_trx; 

         time_point                       created;

         asset                            forfeited_stake;
   };

   struct by_voting_power;
   struct by_mining_power;
   struct by_name;
   struct by_pow;
   struct by_work;
   struct by_witness_schedule_time;
   struct by_miner_schedule_time;
   struct by_txn_stake_weight;
   struct by_activity_stake;

   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      witness_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< witness_object, witness_id_type, &witness_object::id > >,
         ordered_non_unique< tag< by_work >, member< witness_object, digest_type, &witness_object::last_work > >,
         ordered_unique< tag< by_name >, member< witness_object, account_name_type, &witness_object::owner > >,
         
         ordered_unique< tag< by_voting_power >,
            composite_key< witness_object,
               member< witness_object, share_type, &witness_object::voting_power >,
               member< witness_object, account_name_type, &witness_object::owner >
            >,
            composite_key_compare< std::greater< share_type >, std::less< account_name_type > >
         >,

         ordered_unique< tag< by_mining_power >,
            composite_key< witness_object,
               member< witness_object, share_type, &witness_object::mining_power >,
               member< witness_object, account_name_type, &witness_object::owner >
            >,
            composite_key_compare< std::greater< share_type >, std::less< account_name_type > >
         >,

         ordered_unique< tag< by_activity_stake >,
            composite_key< witness_object,
               member< witness_object, uint128_t, &witness_object::accumulated_activity_stake >,
               member< witness_object, witness_id_type, &witness_object::id >
            >,
            composite_key_compare< std::greater< uint128_t >, std::less< witness_id_type > >
         >,

         ordered_unique< tag< by_txn_stake_weight >,
            composite_key< witness_object,
               member< witness_object, share_type, &witness_object::recent_txn_stake_weight >,
               member< witness_object, witness_id_type, &witness_object::id >
            >,
            composite_key_compare< std::greater< uint128_t >, std::less< witness_id_type > >
         >,

         ordered_unique< tag< by_witness_schedule_time >,
            composite_key< witness_object,
               member< witness_object, uint128_t, &witness_object::witness_virtual_scheduled_time >,
               member< witness_object, witness_id_type, &witness_object::id >
            >
         >,

         ordered_unique< tag< by_miner_schedule_time >,
            composite_key< witness_object,
               member< witness_object, uint128_t, &witness_object::miner_virtual_scheduled_time >,
               member< witness_object, witness_id_type, &witness_object::id >
            >
         >
      >,
      allocator< witness_object >
   > witness_index;

   struct by_account_witness;
   struct by_account_rank_witness;
   struct by_account_rank;
   struct by_witness_account;

   typedef multi_index_container<
      witness_vote_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< witness_vote_object, witness_vote_id_type, &witness_vote_object::id > >,
         ordered_unique< tag<by_account_witness>,
            composite_key< witness_vote_object,
               member<witness_vote_object, account_name_type, &witness_vote_object::account >,
               member<witness_vote_object, account_name_type, &witness_vote_object::witness >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type > 
            >
         >,
         ordered_unique< tag<by_account_rank_witness>,
            composite_key< witness_vote_object,
               member<witness_vote_object, account_name_type, &witness_vote_object::account >,
               member<witness_vote_object, uint16_t, &witness_vote_object::vote_rank >,
               member<witness_vote_object, account_name_type, &witness_vote_object::witness >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< uint16_t >, 
               std::less< account_name_type > 
            >
         >,
         ordered_unique< tag<by_account_rank>,
            composite_key< witness_vote_object,
               member<witness_vote_object, account_name_type, &witness_vote_object::account >,
               member<witness_vote_object, uint16_t, &witness_vote_object::vote_rank >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< uint16_t >
            >
         >,
         ordered_unique< tag<by_witness_account>,
            composite_key< witness_vote_object,
               member<witness_vote_object, account_name_type, &witness_vote_object::witness >,
               member<witness_vote_object, account_name_type, &witness_vote_object::account >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >
            >
         >
      >,
      allocator< witness_vote_object >
   > witness_vote_index;

   typedef multi_index_container<
      witness_schedule_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< witness_schedule_object, witness_schedule_id_type, &witness_schedule_object::id > >
      >,
      allocator< witness_schedule_object >
   > witness_schedule_index;


   struct by_producer_height;
   struct by_height_stake;
   struct by_recent_created;
   struct by_commit_time;
   struct by_producer_block_id;
   
   typedef multi_index_container<
      block_validation_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< block_validation_object, block_validation_id_type, &block_validation_object::id > >,
         ordered_unique< tag<by_producer_height>,
            composite_key< block_validation_object,
               member<block_validation_object, account_name_type, &block_validation_object::producer >,
               member<block_validation_object, uint32_t, &block_validation_object::height >
            >
         >,
         ordered_unique< tag<by_height_stake>,
            composite_key< block_validation_object,
               member<block_validation_object, uint32_t, &block_validation_object::height >,
               member<block_validation_object, asset, &block_validation_object::commitment_stake >
            >,
            composite_key_compare< 
               std::less< uint32_t >, 
               std::greater< asset > 
            >
         >,
         ordered_unique< tag<by_producer_block_id>,
            composite_key< block_validation_object,
               member<block_validation_object, account_name_type, &block_validation_object::producer >,
               member<block_validation_object, block_id_type, &block_validation_object::block_id >
            >
         >,
         ordered_unique< tag<by_recent_created>,
            composite_key< block_validation_object,
               member<block_validation_object, time_point, &block_validation_object::created >,
               member< block_validation_object, block_validation_id_type, &block_validation_object::id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< block_validation_id_type >
            >
         >,
         ordered_unique< tag<by_commit_time>,
            composite_key< block_validation_object,
               member<block_validation_object, time_point, &block_validation_object::commit_time >,
               member< block_validation_object, block_validation_id_type, &block_validation_object::id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< block_validation_id_type > 
            >
         >
      >,
      allocator< block_validation_object >
   > block_validation_index;

   struct by_reporter_height;

   typedef multi_index_container<
      commit_violation_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< commit_violation_object, commit_violation_id_type, &commit_violation_object::id > >,
         ordered_unique< tag<by_reporter_height>,
            composite_key< commit_violation_object,
               member<commit_violation_object, account_name_type, &commit_violation_object::reporter >,
               member<commit_violation_object, uint32_t, &commit_violation_object::height >
            >
         >,
         ordered_unique< tag<by_producer_height>,
            composite_key< commit_violation_object,
               member<commit_violation_object, account_name_type, &commit_violation_object::producer >,
               member<commit_violation_object, uint32_t, &commit_violation_object::height >
            >
         >,
         ordered_unique< tag<by_recent_created>,
            composite_key< commit_violation_object,
               member<commit_violation_object, time_point, &commit_violation_object::created >,
               member<commit_violation_object, commit_violation_id_type, &commit_violation_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< commit_violation_id_type > 
            >
         >
      >,
      allocator< commit_violation_object >
   > commit_violation_index;

} }      // node:chain 

FC_REFLECT_ENUM( node::chain::witness_object::witness_schedule_type, 
         (top_witness)
         (additional_witness)
         (top_miner)
         (additional_miner)
         (none) 
         );

FC_REFLECT( node::chain::witness_object,
         (id)
         (owner)
         (active)
         (schedule)
         (last_confirmed_block_num)
         (details)
         (url)
         (json)
         (latitude)
         (longitude)
         (signing_key)
         (created)
         (last_commit_height)
         (last_commit_id)
         (total_blocks)
         (voting_power)
         (vote_count)
         (mining_power)
         (mining_count)
         (last_mining_update)
         (last_pow_time)
         (recent_txn_stake_weight)
         (last_txn_stake_weight_update)
         (accumulated_activity_stake)
         (total_missed)
         (last_aslot)
         (props)
         (witness_virtual_last_update)
         (witness_virtual_position)
         (witness_virtual_scheduled_time)
         (miner_virtual_last_update)
         (miner_virtual_position)
         (miner_virtual_scheduled_time)
         (last_work)
         (running_version)
         (hardfork_version_vote)
         (hardfork_time_vote)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::witness_object, node::chain::witness_index );

FC_REFLECT( node::chain::witness_vote_object, 
         (id)
         (witness)
         (account)
         (vote_rank)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::witness_vote_object, node::chain::witness_vote_index );

FC_REFLECT( node::chain::witness_schedule_object,
         (id)
         (median_props)
         (current_witness_virtual_time)
         (current_miner_virtual_time)
         (next_shuffle_block_num)
         (current_shuffled_producers)
         (total_witness_voting_power)
         (top_witnesses)
         (top_miners)
         (num_scheduled_producers)
         (pow_target_difficulty)
         (recent_pow)
         (last_pow_update)
         (majority_version)
         (dpos_witness_producers)
         (dpos_witness_additional_producers)
         (pow_miner_producers)
         (pow_miner_additional_producers)
         (hardfork_required_witnesses)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::witness_schedule_object, node::chain::witness_schedule_index );

FC_REFLECT( node::chain::block_validation_object,
         (id)
         (producer)
         (block_id)
         (height)
         (created)
         (verifications)
         (verifiers)
         (committed)
         (commit_time)
         (commitment_stake)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::block_validation_object, node::chain::block_validation_index );

FC_REFLECT( node::chain::commit_violation_object,
         (id)
         (reporter)
         (producer)
         (height)
         (first_trx)
         (second_trx)
         (created)
         (forfeited_stake)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::commit_violation_object, node::chain::commit_violation_index );