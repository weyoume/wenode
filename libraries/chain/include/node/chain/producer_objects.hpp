#pragma once

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/x11.hpp>

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
   using node::protocol::share_type;
   using node::protocol::signed_transaction;
   using node::protocol::x11;

   /**
    * All producers with at least 1% net positive approval and
    * at least 2 weeks old are able to participate in block
    * production.
    * These fields are used for the producer scheduling algorithm which uses
    * virtual time to ensure that all producers are given proportional time
    * for producing blocks.
    * voting power is used to determine voting producer speed, 
    * and mining power is used to determine mining producer speed.
    * The virtual_scheduled_time is the expected time at which this producer should complete a virtual lap.
    * virtual_scheduled_time = virtual_last_update + (1000*MAXVOTES - virtual_position) / vote
    * 
    * Every time the number of votes changes the virtual_position and virtual_scheduled_time must
    * update. There is a global current virtual_scheduled_time which gets updated every time a producer is scheduled.
    * virtual_position       = virtual_position + votes * (virtual_current_time - virtual_last_update)
    * virtual_last_update    = virtual_current_time
    * votes                  += delta_vote
    * virtual_scheduled_time = virtual_last_update + (1000*MAXVOTES - virtual_position) / vote
    */
   class producer_object : public object< producer_object_type, producer_object >
   {
      producer_object() = delete;

      public:
         enum producer_schedule_type
         {
            top_voting_producer,
            additional_voting_producer,
            top_mining_producer,
            additional_mining_producer,
            none
         };

         template< typename Constructor, typename Allocator >
         producer_object( Constructor&& c, allocator< Allocator > a ) :
            details(a), 
            url(a), 
            json(a)
            {
               c( *this );
            }

         id_type                      id;

         account_name_type            owner;                                 ///< The name of the account that has authority over this producer.

         bool                         active = true;                         ///< True if the producer is actively seeking to produce blocks, set false to deactivate the producer and remove from production.

         producer_schedule_type       schedule = none;                       ///< How the producer was scheduled the last time it was scheduled.

         uint64_t                     last_confirmed_block_num = 0;          ///< Number of the last block that was successfully produced by this producer. 

         shared_string                details;                               ///< Producer's details, explaining who they are, machine specs, capabilties.

         shared_string                url;                                   ///< The producer's URL explaining their details.

         shared_string                json;                                  ///< The producer's json metadata.

         double                       latitude;                              ///< Latitude co-ordinates of the producer.

         double                       longitude;                             ///< Longitude co-ordinates of the producer.

         public_key_type              signing_key;                           ///< The key used to sign blocks on behalf of this producer.

         uint64_t                     last_commit_height = 0;                ///< Block height that has been most recently committed by the producer

         block_id_type                last_commit_id = block_id_type();      ///< Block ID of the height that was most recently committed by the producer. 

         uint32_t                     total_blocks = 0;                      ///< Accumulated number of blocks produced.

         share_type                   voting_power = 0;                      ///< The total weighted voting power that supports the producer. 

         uint32_t                     vote_count = 0;                        ///< The number of accounts that have voted for the producer.

         share_type                   mining_power = 0;                      ///< The amount of proof of work difficulty accumulated by the miner over the prior 7 days.

         uint32_t                     mining_count = 0;                      ///< Accumulated number of proofs of work published.

         time_point                   last_mining_update;                    ///< Time that the account last updated its mining power.

         time_point                   last_pow_time;                         ///< Time that the miner last created a proof of work.

         uint128_t                    recent_txn_stake_weight = 0;           ///< Rolling average Amount of transaction stake weight contained that the producer has included in blocks over the prior 7 days.

         time_point                   last_txn_stake_weight_update;          ///< Time that the recent bandwith and txn stake were last updated.

         uint128_t                    accumulated_activity_stake = 0;        ///< Recent amount of activity reward stake for the prime producer. 

         uint32_t                     total_missed;                          ///< Number of blocks missed recently.

         uint64_t                     last_aslot;                            ///< Last absolute slot that the producer was assigned to produce a block.

         chain_properties             props;                                 ///< The chain properties object that the producer currently proposes for global network variables
         
         uint128_t                    voting_virtual_last_update;            ///< Virtual time of last producer update.

         uint128_t                    voting_virtual_position;               ///< Virtual position relative to other producers.

         uint128_t                    voting_virtual_scheduled_time = fc::uint128::max_value();         ///< Expected virtual time of next scheduled block production.

         uint128_t                    mining_virtual_last_update;            ///< Virtual time of last mining update.

         uint128_t                    mining_virtual_position;               ///< Virtual position relative to other miners.

         uint128_t                    mining_virtual_scheduled_time = fc::uint128::max_value();          ///< Expected virtual time of next scheduled block production.

         version                      running_version;                       ///< This field represents the WeYouMe blockchain version the producer is running.

         hardfork_version             hardfork_version_vote;                 ///< The vote for the next hardfork update version.

         time_point                   hardfork_time_vote = GENESIS_TIME;     ///< The time to activate the next hardfork.

         time_point                   created;                               ///< The time the producer was created.

         time_point                   last_updated;                          ///< The time the producer was last updated.

         void                         decay_weights( time_point now, const median_chain_property_object& median_props )
         {
            mining_power -= ( mining_power * ( now - last_mining_update ).to_seconds() ) / median_props.pow_decay_time.to_seconds();
            recent_txn_stake_weight -= ( recent_txn_stake_weight * ( now - last_txn_stake_weight_update ).to_seconds() ) / median_props.txn_stake_decay_time.to_seconds();
            last_mining_update = now;
            last_txn_stake_weight_update = now;
         }
   };


   class producer_vote_object : public object< producer_vote_object_type, producer_vote_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         producer_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         producer_vote_object(){}

         id_type                id;

         account_name_type      producer;             ///< The Producer account voted for.

         account_name_type      account;              ///< The account creating the producer vote.

         uint16_t               vote_rank = 1;        ///< Ordered rank to which the producer is supported.

         time_point             last_updated;         ///< Time the vote was last updated.

         time_point             created;              ///< Time the vote was created.
   };


   class producer_schedule_object : public object< producer_schedule_object_type, producer_schedule_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         producer_schedule_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         producer_schedule_object(){}

         id_type                                           id;

         uint128_t                                         current_voting_virtual_time;               ///< Tracks the time used for block producer additional selection.

         uint128_t                                         current_mining_virtual_time;               ///< Tracks the time used for block producer additional selection.

         uint64_t                                          next_shuffle_block_num = 1;                ///< The block of the next reshuffling of producers.

         vector< account_name_type >                       current_shuffled_producers;                ///< Currently active block producers to be includes in the next production round.

         uint128_t                                         total_producer_voting_power;               ///< Total sum of all voting power that is voting for producers.

         vector< account_name_type >                       top_voting_producers;                      ///< Ordered list of the highest voted producers.

         vector< account_name_type >                       top_mining_producers;                      ///< Ordered list of the highest mining producers.

         uint8_t                                           num_scheduled_producers = TOTAL_PRODUCERS; ///< Amount of producing accounts that are scheduled in each round.

         x11                                               pow_target_difficulty = x11( uint128_t::max_value() / uint128_t( 1000 ) );  ///< Proof of work target, work value must be lower than this value.

         uint128_t                                         recent_pow = INIT_RECENT_POW;              ///< Rolling average amount of blocks (x prec) mined in the last 7 days.

         time_point                                        last_pow_update;                           ///< Time that the recent POW was last updated and decayed.

         version                                           majority_version;

         uint8_t                                           dpos_producers = DPOS_VOTING_PRODUCERS;

         uint8_t                                           dpos_additional_producers = DPOS_VOTING_ADDITIONAL_PRODUCERS;

         uint8_t                                           pow_producers = POW_MINING_PRODUCERS;

         uint8_t                                           pow_additional_producers = POW_MINING_ADDITIONAL_PRODUCERS;
         
         uint8_t                                           hardfork_required_producers = HARDFORK_REQUIRED_PRODUCERS;

         bool                                              is_top_voting_producer( const account_name_type& producer )const            ///< finds if a given producer name is in the top voting producers set. 
         {
            return std::find( top_voting_producers.begin(), top_voting_producers.end(), producer) != top_voting_producers.end();
         }

         bool                                              is_top_mining_producer( const account_name_type& producer )const         ///< finds if a given producer name is in the top mining producers set. 
         {
            return std::find( top_mining_producers.begin(), top_mining_producers.end(), producer) != top_mining_producers.end();
         }

         bool                                              is_top_producer( const account_name_type& producer )const         ///< finds if a given producer name is in the top voting or mining producers set. 
         {
            return is_top_voting_producer( producer ) || is_top_mining_producer( producer );
         }

         void                                              decay_pow( time_point now, const median_chain_property_object& median_props )
         {
            recent_pow -= ( ( recent_pow * ( now - last_pow_update ).to_seconds() ) / median_props.pow_decay_time.to_seconds() );
            last_pow_update = now;
         }
   };


   /**
    * Block Validation Objects are used by block producers
    * to place a committment stake on recent blocks, 
    * before they become irreversible, in order to increase the 
    * speed of achieving consensus finality on block history.
    * 
    * Block producers earn an additional share of the block 
    * reward when they are in the first two thirds of 
    * producers to commit to a block id at a given height.
    * 
    * The reward is distributed to all producers upon the
    * block becoming irreversible to all producers
    * that have commitments at a given height.
    * All producers that include commitments in the 
    * block that exceeds two thirds are included
    * in reward distribution.
    * 
    * All nodes will use the greater of the last irreversible block, 
    * or the last commited block when updating the state of their block logs. 
    * This enables an optimal block finality time of two blocks, 
    * with rapid participation of producers to validate and 
    * commit to new blocks as they are produced.
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

         id_type                               id;

         account_name_type                     producer;             ///< Name of the block producer creating the validation.

         block_id_type                         block_id;             ///< Block ID commited to for the height.

         uint64_t                              block_height;         ///< Height of the block being validated.

         transaction_id_type                   verify_txn;           ///< Transaction ID in which this validation object was verified.

         flat_set< transaction_id_type >       verifications;        ///< Validation transactions from other producers.

         flat_set< account_name_type >         verifiers;            ///< Accounts that have also verfied this block id at this height.

         bool                                  committed = false;    ///< True if the validation has been committed with stake.

         time_point                            commit_time = fc::time_point::min();    ///< Time that the validation was committed.

         asset                                 commitment_stake;     ///< Amount of COIN staked on the validity of the block.

         transaction_id_type                   commit_txn;           ///< Transaction ID in which this validation object was commited.

         time_point                            created;              ///< Time that the validation was created.
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

         account_name_type                reporter;                ///< Name of the account creating the violation.

         account_name_type                producer;                ///< Name of the account that violated a commit transaction

         uint64_t                         block_height;            ///< Height of the block that the violation occured on.

         signed_transaction               first_trx;               ///< First Transaction that conflicts.

         signed_transaction               second_trx;              ///< Second Transaction that conflicts.

         time_point                       created;                 ///< Time that the violation was created.

         asset                            forfeited_stake;         ///< Stake value that was forfeited.
   };

   struct by_voting_power;
   struct by_mining_power;
   struct by_name;
   struct by_pow;
   struct by_voting_schedule_time;
   struct by_mining_schedule_time;
   struct by_txn_stake_weight;
   struct by_activity_stake;

   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      producer_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< producer_object, producer_id_type, &producer_object::id > >,
         ordered_unique< tag< by_name >, member< producer_object, account_name_type, &producer_object::owner > >,
         
         ordered_unique< tag< by_voting_power >,
            composite_key< producer_object,
               member< producer_object, share_type, &producer_object::voting_power >,
               member< producer_object, account_name_type, &producer_object::owner >
            >,
            composite_key_compare< 
               std::greater< share_type >, 
               std::less< account_name_type > 
            >
         >,
         ordered_unique< tag< by_mining_power >,
            composite_key< producer_object,
               member< producer_object, share_type, &producer_object::mining_power >,
               member< producer_object, account_name_type, &producer_object::owner >
            >,
            composite_key_compare< 
               std::greater< share_type >, 
               std::less< account_name_type > 
            >
         >,
         ordered_unique< tag< by_activity_stake >,
            composite_key< producer_object,
               member< producer_object, uint128_t, &producer_object::accumulated_activity_stake >,
               member< producer_object, producer_id_type, &producer_object::id >
            >,
            composite_key_compare< 
               std::greater< uint128_t >, 
               std::less< producer_id_type > 
            >
         >,
         ordered_unique< tag< by_txn_stake_weight >,
            composite_key< producer_object,
               member< producer_object, uint128_t, &producer_object::recent_txn_stake_weight >,
               member< producer_object, producer_id_type, &producer_object::id >
            >,
            composite_key_compare< 
               std::greater< uint128_t >, 
               std::less< producer_id_type > 
            >
         >,
         ordered_unique< tag< by_voting_schedule_time >,
            composite_key< producer_object,
               member< producer_object, uint128_t, &producer_object::voting_virtual_scheduled_time >,
               member< producer_object, producer_id_type, &producer_object::id >
            >
         >,
         ordered_unique< tag< by_mining_schedule_time >,
            composite_key< producer_object,
               member< producer_object, uint128_t, &producer_object::mining_virtual_scheduled_time >,
               member< producer_object, producer_id_type, &producer_object::id >
            >
         >
      >,
      allocator< producer_object >
   > producer_index;


   struct by_account_producer;
   struct by_account_rank;
   struct by_producer_account;


   typedef multi_index_container<
      producer_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< producer_vote_object, producer_vote_id_type, &producer_vote_object::id > >,
         ordered_unique< tag< by_account_producer >,
            composite_key< producer_vote_object,
               member< producer_vote_object, account_name_type, &producer_vote_object::account >,
               member< producer_vote_object, account_name_type, &producer_vote_object::producer >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type > 
            >
         >,
         ordered_unique< tag< by_account_rank >,
            composite_key< producer_vote_object,
               member< producer_vote_object, account_name_type, &producer_vote_object::account >,
               member< producer_vote_object, uint16_t, &producer_vote_object::vote_rank >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< uint16_t >
            >
         >,
         ordered_unique< tag< by_producer_account >,
            composite_key< producer_vote_object,
               member< producer_vote_object, account_name_type, &producer_vote_object::producer >,
               member< producer_vote_object, account_name_type, &producer_vote_object::account >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >
            >
         >
      >,
      allocator< producer_vote_object >
   > producer_vote_index;

   typedef multi_index_container<
      producer_schedule_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< producer_schedule_object, producer_schedule_id_type, &producer_schedule_object::id > >
      >,
      allocator< producer_schedule_object >
   > producer_schedule_index;


   struct by_producer_height;
   struct by_height_stake;
   struct by_recent_created;
   struct by_commit_time;
   struct by_producer_block_id;
   struct by_block_id;
   

   typedef multi_index_container<
      block_validation_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< block_validation_object, block_validation_id_type, &block_validation_object::id > >,
         ordered_unique< tag< by_producer_height >,
            composite_key< block_validation_object,
               member< block_validation_object, account_name_type, &block_validation_object::producer >,
               member< block_validation_object, uint64_t, &block_validation_object::block_height >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::greater< uint64_t >
            >
         >,
         ordered_unique< tag< by_height_stake >,
            composite_key< block_validation_object,
               member< block_validation_object, uint64_t, &block_validation_object::block_height >,
               member< block_validation_object, asset, &block_validation_object::commitment_stake >,
               member< block_validation_object, block_validation_id_type, &block_validation_object::id >
            >,
            composite_key_compare< 
               std::less< uint64_t >,
               std::greater< asset >,
               std::less< block_validation_id_type >
            >
         >,
         ordered_unique< tag< by_producer_block_id >,
            composite_key< block_validation_object,
               member< block_validation_object, account_name_type, &block_validation_object::producer >,
               member< block_validation_object, block_id_type, &block_validation_object::block_id >
            >
         >,
         ordered_unique< tag< by_block_id >,
            composite_key< block_validation_object,
               member< block_validation_object, block_id_type, &block_validation_object::block_id >,
               member< block_validation_object, block_validation_id_type, &block_validation_object::id >
            >,
            composite_key_compare< 
               std::less< block_id_type >, 
               std::less< block_validation_id_type >
            >
         >,
         ordered_unique< tag< by_recent_created >,
            composite_key< block_validation_object,
               member< block_validation_object, time_point, &block_validation_object::created >,
               member< block_validation_object, block_validation_id_type, &block_validation_object::id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< block_validation_id_type >
            >
         >,
         ordered_unique< tag< by_commit_time >,
            composite_key< block_validation_object,
               member< block_validation_object, time_point, &block_validation_object::commit_time >,
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
         ordered_unique< tag< by_id >, member< commit_violation_object, commit_violation_id_type, &commit_violation_object::id > >,
         ordered_unique< tag< by_reporter_height >,
            composite_key< commit_violation_object,
               member< commit_violation_object, account_name_type, &commit_violation_object::reporter >,
               member< commit_violation_object, uint64_t, &commit_violation_object::block_height >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::greater< uint64_t >
            >
         >,
         ordered_unique< tag< by_producer_height >,
            composite_key< commit_violation_object,
               member< commit_violation_object, account_name_type, &commit_violation_object::producer >,
               member< commit_violation_object, uint64_t, &commit_violation_object::block_height >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::greater< uint64_t >
            >
         >,
         ordered_unique< tag< by_recent_created >,
            composite_key< commit_violation_object,
               member< commit_violation_object, time_point, &commit_violation_object::created >,
               member< commit_violation_object, commit_violation_id_type, &commit_violation_object::id >
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

FC_REFLECT_ENUM( node::chain::producer_object::producer_schedule_type, 
         (top_voting_producer)
         (additional_voting_producer)
         (top_mining_producer)
         (additional_mining_producer)
         (none) 
         );

FC_REFLECT( node::chain::producer_object,
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
         (voting_virtual_last_update)
         (voting_virtual_position)
         (voting_virtual_scheduled_time)
         (mining_virtual_last_update)
         (mining_virtual_position)
         (mining_virtual_scheduled_time)
         (running_version)
         (hardfork_version_vote)
         (hardfork_time_vote)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::producer_object, node::chain::producer_index );

FC_REFLECT( node::chain::producer_vote_object, 
         (id)
         (producer)
         (account)
         (vote_rank)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::producer_vote_object, node::chain::producer_vote_index );

FC_REFLECT( node::chain::producer_schedule_object,
         (id)
         (current_voting_virtual_time)
         (current_mining_virtual_time)
         (next_shuffle_block_num)
         (current_shuffled_producers)
         (total_producer_voting_power)
         (top_voting_producers)
         (top_mining_producers)
         (num_scheduled_producers)
         (pow_target_difficulty)
         (recent_pow)
         (last_pow_update)
         (majority_version)
         (dpos_producers)
         (dpos_additional_producers)
         (pow_producers)
         (pow_additional_producers)
         (hardfork_required_producers)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::producer_schedule_object, node::chain::producer_schedule_index );

FC_REFLECT( node::chain::block_validation_object,
         (id)
         (producer)
         (block_id)
         (block_height)
         (verify_txn)
         (verifications)
         (verifiers)
         (committed)
         (commit_time)
         (commitment_stake)
         (commit_txn)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::block_validation_object, node::chain::block_validation_index );

FC_REFLECT( node::chain::commit_violation_object,
         (id)
         (reporter)
         (producer)
         (block_height)
         (first_trx)
         (second_trx)
         (created)
         (forfeited_stake)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::commit_violation_object, node::chain::commit_violation_index );