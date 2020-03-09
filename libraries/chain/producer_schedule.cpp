#include <node/chain/database.hpp>
#include <node/chain/producer_objects.hpp>
#include <node/chain/producer_schedule.hpp>
#include <node/chain/node_objects.hpp>

#include <node/protocol/config.hpp>

namespace node { namespace chain {

void reset_voting_virtual_schedule_time( database& db )
{
   const producer_schedule_object& pso = db.get_producer_schedule();

   db.modify( pso, [&]( producer_schedule_object& o )
   {
      o.current_voting_virtual_time = fc::uint128();      // reset to 0
   });

   const auto& idx = db.get_index< producer_index >().indices();
   for( const auto& producer : idx )
   {
      db.modify( producer, [&]( producer_object& p )
      {
         p.voting_virtual_position = fc::uint128();
         p.voting_virtual_last_update = pso.current_voting_virtual_time;
         p.voting_virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH / ( p.voting_power.value + 1 );
      });
   }
}

void reset_mining_virtual_schedule_time( database& db )
{
   const producer_schedule_object& pso = db.get_producer_schedule();
   db.modify( pso, [&]( producer_schedule_object& o )
   {
      o.current_mining_virtual_time = fc::uint128(); // reset it 0
   });

   const auto& idx = db.get_index< producer_index >().indices();
   for( const auto& producer : idx )
   {
      db.modify( producer, [&]( producer_object& p )
      {
         p.mining_virtual_position = fc::uint128();
         p.mining_virtual_last_update = pso.current_mining_virtual_time;
         p.mining_virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH / ( p.mining_power.value + 1 );
      });
   }
}

/**
 * Update the median producer properties, according to the producer chain properties
 * median values for each variable.
 */
void update_median_producer_props( database& db )
{
   const producer_schedule_object& pso = db.get_producer_schedule();
   vector< const producer_object* > active;
   active.reserve( pso.num_scheduled_producers );
   size_t offset = active.size()/2;

   for( int i = 0; i < pso.num_scheduled_producers; i++ )
   {
      active.push_back( db.find_producer( pso.current_shuffled_producers[i] ) );      // Fetch producer object pointers.
   }

   const median_chain_property_object& median_props = db.get_median_chain_properties();

   // Sort all properties variables and find median items, placing them into new properties.

   db.modify( median_props, [&]( median_chain_property_object& mcpo )
   {
      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.account_creation_fee.amount < b->props.account_creation_fee.amount;
      });
      mcpo.account_creation_fee = active[ offset ]->props.account_creation_fee;
      
      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.maximum_block_size < b->props.maximum_block_size;
      });
      mcpo.maximum_block_size = active[ offset ]->props.maximum_block_size;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.pow_target_time < b->props.pow_target_time;
      });
      mcpo.pow_target_time = active[ offset ]->props.pow_target_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.pow_decay_time < b->props.pow_decay_time;
      });
      mcpo.pow_decay_time = active[ offset ]->props.pow_decay_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.txn_stake_decay_time < b->props.txn_stake_decay_time;
      });
      mcpo.txn_stake_decay_time = active[ offset ]->props.txn_stake_decay_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.escrow_bond_percent < b->props.escrow_bond_percent;
      });
      mcpo.escrow_bond_percent = active[ offset ]->props.escrow_bond_percent;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.credit_interest_rate < b->props.credit_interest_rate;
      });
      mcpo.credit_interest_rate = active[ offset ]->props.credit_interest_rate;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.credit_open_ratio < b->props.credit_open_ratio;
      });
      mcpo.credit_open_ratio = active[ offset ]->props.credit_open_ratio;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.credit_liquidation_ratio < b->props.credit_liquidation_ratio;
      });
      mcpo.credit_liquidation_ratio = active[ offset ]->props.credit_liquidation_ratio;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.credit_min_interest < b->props.credit_min_interest;
      });
      mcpo.credit_min_interest = active[ offset ]->props.credit_min_interest;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.credit_variable_interest < b->props.credit_variable_interest;
      });
      mcpo.credit_variable_interest = active[ offset ]->props.credit_variable_interest;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.market_max_credit_ratio < b->props.market_max_credit_ratio;
      });
      mcpo.market_max_credit_ratio = active[ offset ]->props.market_max_credit_ratio;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.margin_open_ratio < b->props.margin_open_ratio;
      });
      mcpo.margin_open_ratio = active[ offset ]->props.margin_open_ratio;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.margin_liquidation_ratio < b->props.margin_liquidation_ratio;
      });
      mcpo.margin_liquidation_ratio = active[ offset ]->props.margin_liquidation_ratio;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.maximum_asset_feed_publishers < b->props.maximum_asset_feed_publishers;
      });
      mcpo.maximum_asset_feed_publishers = active[ offset ]->props.maximum_asset_feed_publishers;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.membership_base_price < b->props.membership_base_price;
      });
      mcpo.membership_base_price = active[ offset ]->props.membership_base_price;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.membership_mid_price < b->props.membership_mid_price;
      });
      mcpo.membership_mid_price = active[ offset ]->props.membership_mid_price;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.membership_top_price < b->props.membership_top_price;
      });
      mcpo.membership_top_price = active[ offset ]->props.membership_top_price;

      // Sort the content reward splits by median, and adjust author rewards to be the remainder of the percentages

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.vote_reward_percent < b->props.vote_reward_percent;
      });
      mcpo.vote_reward_percent = active[ offset ]->props.vote_reward_percent;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.view_reward_percent < b->props.view_reward_percent;
      });
      mcpo.view_reward_percent = active[ offset ]->props.view_reward_percent;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.share_reward_percent < b->props.share_reward_percent;
      });
      mcpo.share_reward_percent = active[ offset ]->props.share_reward_percent;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.comment_reward_percent < b->props.comment_reward_percent;
      });
      mcpo.comment_reward_percent = active[ offset ]->props.comment_reward_percent;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.storage_reward_percent < b->props.storage_reward_percent;
      });
      mcpo.storage_reward_percent = active[ offset ]->props.storage_reward_percent;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.moderator_reward_percent < b->props.moderator_reward_percent;
      });
      mcpo.moderator_reward_percent = active[ offset ]->props.moderator_reward_percent;

      uint32_t author_reward_percent = PERCENT_100 - ( mcpo.vote_reward_percent + mcpo.view_reward_percent +
         mcpo.share_reward_percent + mcpo.comment_reward_percent + mcpo.storage_reward_percent + mcpo.moderator_reward_percent );

      mcpo.author_reward_percent = author_reward_percent;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.content_reward_decay_rate < b->props.content_reward_decay_rate;
      });
      mcpo.content_reward_decay_rate = active[ offset ]->props.content_reward_decay_rate;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.content_reward_interval < b->props.content_reward_interval;
      });
      mcpo.content_reward_interval = active[ offset ]->props.content_reward_interval;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.vote_reserve_rate < b->props.vote_reserve_rate;
      });
      mcpo.vote_reserve_rate = active[ offset ]->props.vote_reserve_rate;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.view_reserve_rate < b->props.view_reserve_rate;
      });
      mcpo.view_reserve_rate = active[ offset ]->props.view_reserve_rate;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.share_reserve_rate < b->props.share_reserve_rate;
      });
      mcpo.share_reserve_rate = active[ offset ]->props.share_reserve_rate;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.comment_reserve_rate < b->props.comment_reserve_rate;
      });
      mcpo.comment_reserve_rate = active[ offset ]->props.comment_reserve_rate;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.vote_recharge_time < b->props.vote_recharge_time;
      });
      mcpo.vote_recharge_time = active[ offset ]->props.vote_recharge_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.view_recharge_time < b->props.view_recharge_time;
      });
      mcpo.view_recharge_time = active[ offset ]->props.view_recharge_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.share_recharge_time < b->props.share_recharge_time;
      });
      mcpo.share_recharge_time = active[ offset ]->props.share_recharge_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.comment_recharge_time < b->props.comment_recharge_time;
      });
      mcpo.comment_recharge_time = active[ offset ]->props.comment_recharge_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.curation_auction_decay_time < b->props.curation_auction_decay_time;
      });
      mcpo.curation_auction_decay_time = active[ offset ]->props.curation_auction_decay_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.vote_curation_decay < b->props.vote_curation_decay;
      });
      mcpo.vote_curation_decay = active[ offset ]->props.vote_curation_decay;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.view_curation_decay < b->props.view_curation_decay;
      });
      mcpo.view_curation_decay = active[ offset ]->props.view_curation_decay;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.share_curation_decay < b->props.share_curation_decay;
      });
      mcpo.share_curation_decay = active[ offset ]->props.share_curation_decay;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.comment_curation_decay < b->props.comment_curation_decay;
      });
      mcpo.comment_curation_decay = active[ offset ]->props.comment_curation_decay;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.supernode_decay_time < b->props.supernode_decay_time;
      });
      mcpo.supernode_decay_time = active[ offset ]->props.supernode_decay_time;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.enterprise_vote_percent_required < b->props.enterprise_vote_percent_required;
      });
      mcpo.enterprise_vote_percent_required = active[ offset ]->props.enterprise_vote_percent_required;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.maximum_asset_whitelist_authorities < b->props.maximum_asset_whitelist_authorities;
      });
      mcpo.maximum_asset_whitelist_authorities = active[ offset ]->props.maximum_asset_whitelist_authorities;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.max_stake_intervals < b->props.max_stake_intervals;
      });
      mcpo.max_stake_intervals = active[ offset ]->props.max_stake_intervals;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.max_unstake_intervals < b->props.max_unstake_intervals;
      });
      mcpo.max_unstake_intervals = active[ offset ]->props.max_unstake_intervals;

      std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const producer_object* a, const producer_object* b )
      {
         return a->props.max_exec_budget < b->props.max_exec_budget;
      });
      mcpo.max_exec_budget = active[ offset ]->props.max_exec_budget;
   });
}

void update_producer_schedule( database& db )
{
   if( (db.head_block_num() % TOTAL_PRODUCERS) == 0 ) //  pso.next_shuffle_block_num 
   {
      const producer_schedule_object& pso = db.get_producer_schedule();
      fc::uint128 new_voting_virtual_time = pso.current_voting_virtual_time;
      fc::uint128 new_mining_virtual_time = pso.current_mining_virtual_time;

      vector< account_name_type > active_voting_producers;
      vector< account_name_type > active_mining_producers;

      uint8_t max_voting_producers = pso.dpos_producers + pso.dpos_additional_producers;
      uint8_t max_mining_producers = pso.pow_producers + pso.pow_additional_producers;

      FC_ASSERT( max_voting_producers + max_mining_producers == TOTAL_PRODUCERS,
         "Block production requires max voting producers and mining producers to add to total producers value." );
      FC_ASSERT( max_voting_producers == max_mining_producers,
         "Block production requires equal amounts of mining producers and voting producers." );

      active_voting_producers.reserve( max_voting_producers );
      active_mining_producers.reserve( max_mining_producers );

      // Add the highest voted voting_producers
      flat_set< producer_id_type > top_voting_producers;
      top_voting_producers.reserve( pso.dpos_producers );
                 
      const auto& voting_power_producer_idx = db.get_index< producer_index >().indices().get< by_voting_power >();
      for( auto voting_power_itr = voting_power_producer_idx.begin();
         voting_power_itr != voting_power_producer_idx.end() && top_voting_producers.size() < max_voting_producers;
         ++voting_power_itr )
      {
         if( voting_power_itr->signing_key == public_key_type() )
            continue;
         top_voting_producers.insert( voting_power_itr->id );
         active_voting_producers.push_back( voting_power_itr->owner );
         db.modify( *voting_power_itr, [&]( producer_object& p ) 
         { 
            p.schedule = producer_object::top_voting_producer; 
         });
      }

      auto num_top_voting_producers = top_voting_producers.size();

      /// Add mining_producers from the top of the mining queue
      flat_set< producer_id_type > top_mining_producers;
      top_mining_producers.reserve( pso.pow_producers );

      const auto& mining_power_producer_idx = db.get_index<producer_index>().indices().get<by_mining_power>();
      auto mining_power_itr = mining_power_producer_idx.begin();
      while( mining_power_itr != mining_power_producer_idx.end() && top_mining_producers.size() < max_mining_producers )
      {
         if( top_voting_producers.find(mining_power_itr->id) == top_voting_producers.end() ) // Only consider a miner who is not a top voting producer
         {
            if( !( db.get_producer( mining_power_itr->owner ).signing_key == public_key_type() ) ) // Only consider a miner who has a valid block signing key
            {
               top_mining_producers.insert(mining_power_itr->id);
               active_mining_producers.push_back(mining_power_itr->owner);
               db.modify( *mining_power_itr, [&]( producer_object& p ) 
               { 
                  p.schedule = producer_object::top_mining_producer; 
               });
            }
         }
         ++mining_power_itr;
      }

      vector< account_name_type > top_voting_producer_set = active_voting_producers;       // Set of voting producers in the top selection
      vector< account_name_type > top_mining_producer_set = active_mining_producers;         // Set of mining_producers in the top selection. 

      auto num_top_mining_producers = top_mining_producers.size();

      /// Add the additional voting_producers in the lead
      flat_set< producer_id_type > additional_voting_producers;
      additional_voting_producers.reserve( pso.dpos_additional_producers );

      const auto& voting_producer_schedule_idx = db.get_index<producer_index>().indices().get<by_voting_schedule_time>();
      auto avp_itr = voting_producer_schedule_idx.begin(); // Additional Voting producer
      vector<decltype(avp_itr)> processed_voting_producers;

      for( auto voting_producer_count = num_top_voting_producers;
         avp_itr != voting_producer_schedule_idx.end() && voting_producer_count < max_voting_producers;
         ++avp_itr )
      {
         new_voting_virtual_time = avp_itr->voting_virtual_scheduled_time; /// everyone advances to at least this time
         processed_voting_producers.push_back(avp_itr);

         if( avp_itr->signing_key == public_key_type() )
            continue; // skip voting_producers without a valid block signing key

         if( top_mining_producers.find(avp_itr->id) == top_mining_producers.end()
            && top_voting_producers.find(avp_itr->id) == top_voting_producers.end() ) // skip producers already in the selection sets
         {
            additional_voting_producers.insert(avp_itr->id);
            active_voting_producers.push_back(avp_itr->owner);
            db.modify( *avp_itr, [&]( producer_object& p ) 
            { 
               p.schedule = producer_object::additional_voting_producer;
            });

            ++voting_producer_count;
         }
      }

      auto num_additional_voting_producers = active_voting_producers.size() - num_top_voting_producers;

      // Add the additional mining_producers in the lead.
      flat_set< producer_id_type > additional_mining_producers;
      additional_mining_producers.reserve( pso.pow_additional_producers );

      const auto& mining_producer_schedule_idx = db.get_index< producer_index >().indices().get< by_mining_schedule_time >();
      auto amp_itr = mining_producer_schedule_idx.begin();     // Additional miner iterator.
      vector<decltype(amp_itr)> processed_mining_producers;

      for( auto mining_producer_count = num_top_mining_producers;
         amp_itr != mining_producer_schedule_idx.end() && mining_producer_count < max_mining_producers;
         ++amp_itr )
      {
         new_mining_virtual_time = amp_itr->mining_virtual_scheduled_time;    // everyone advances to at least this time
         processed_mining_producers.push_back( amp_itr );

         if( amp_itr->signing_key == public_key_type() )
         {
            continue;     // skip mining_producers without a valid block signing key
         } 

         if( top_mining_producers.find( amp_itr->id ) == top_mining_producers.end()
            && top_voting_producers.find( amp_itr->id ) == top_voting_producers.end()
            && additional_voting_producers.find( amp_itr->id ) == additional_voting_producers.end() )   // skip producers already in the selection sets
         {
            additional_mining_producers.insert(amp_itr->id);
            active_mining_producers.push_back(amp_itr->owner);
            db.modify( *amp_itr, [&]( producer_object& p ) 
            { 
               p.schedule = producer_object::additional_mining_producer; 
            });

            ++mining_producer_count;
         }
      }

      auto num_additional_mining_producers = active_mining_producers.size() - num_top_mining_producers;

      // Update virtual schedule of processed voting_producers and mining_producers
      bool reset_voting_virtual_time = false;
      bool reset_mining_virtual_time = false;

      for( auto pvp_itr = processed_voting_producers.begin(); pvp_itr != processed_voting_producers.end(); ++pvp_itr )
      {
         auto new_voting_virtual_scheduled_time = new_voting_virtual_time + VIRTUAL_SCHEDULE_LAP_LENGTH / ((*pvp_itr)->voting_power.value+1);
         if( new_voting_virtual_scheduled_time < new_voting_virtual_time )
         {
            reset_voting_virtual_time = true; // overflow
            break;
         }
         db.modify( *(*pvp_itr), [&]( producer_object& p )
         {
            p.voting_virtual_position = fc::uint128();
            p.voting_virtual_last_update = new_voting_virtual_time;
            p.voting_virtual_scheduled_time = new_voting_virtual_scheduled_time;
         });
      }

      if( reset_voting_virtual_time )
      {
         new_voting_virtual_time = fc::uint128();
         reset_voting_virtual_schedule_time(db);
      }

      for( auto pmp_itr = processed_mining_producers.begin(); pmp_itr != processed_mining_producers.end(); ++pmp_itr )
      {
         auto new_mining_virtual_scheduled_time = new_mining_virtual_time + VIRTUAL_SCHEDULE_LAP_LENGTH / ((*pmp_itr)->mining_power.value+1);
         if( new_mining_virtual_scheduled_time < new_mining_virtual_time )
         {
            reset_mining_virtual_time = true; // overflow
            break;
         }
         db.modify( *(*pmp_itr), [&]( producer_object& p )
         {
            p.mining_virtual_position = fc::uint128();
            p.mining_virtual_last_update = new_mining_virtual_time;
            p.mining_virtual_scheduled_time = new_mining_virtual_scheduled_time;
         });
      }

      if( reset_mining_virtual_time )
      {
         new_mining_virtual_time = fc::uint128();
         reset_mining_virtual_schedule_time( db );
      }

      size_t active_producers = active_voting_producers.size() + active_mining_producers.size();
      size_t expected_active_producers = max_voting_producers + max_mining_producers;

      FC_ASSERT( active_producers == expected_active_producers, 
         "Number of active producers does not equal expected producers",
         ("active_producers", active_producers) ("TOTAL_PRODUCERS",TOTAL_PRODUCERS) ("expected_active_producers", expected_active_producers) );

      FC_ASSERT( num_top_voting_producers + num_top_mining_producers + num_additional_voting_producers + num_additional_mining_producers == active_producers, 
         "Block production invariants invalid: Producer sum not equal to active producers", 
         ("num_top_voting_producers", num_top_voting_producers) ("num_top_mining_producers", num_top_mining_producers) ("num_additional_voting_producers", num_additional_voting_producers)
         ("num_additional_mining_producers", num_additional_mining_producers) ("active_producers", active_producers) );

      auto majority_version = pso.majority_version;

      flat_map< version, uint32_t, std::greater< version > > producer_versions;
      flat_map< std::tuple< hardfork_version, time_point >, uint32_t > hardfork_version_votes;

      for( uint32_t i = 0; i < pso.num_scheduled_producers; i++ )
      {
         auto producer = db.get_producer( pso.current_shuffled_producers[ i ] );
         if( producer_versions.find( producer.running_version ) == producer_versions.end() )
         {
            producer_versions[ producer.running_version ] = 1;
         }
         else
         {
            producer_versions[ producer.running_version ] += 1;
         }
         auto version_vote = std::make_tuple( producer.hardfork_version_vote, producer.hardfork_time_vote );
         if( hardfork_version_votes.find( version_vote ) == hardfork_version_votes.end() )
         {
            hardfork_version_votes[ version_vote ] = 1;
         }
         else
         {
            hardfork_version_votes[ version_vote ] += 1;
         } 
      }

      int voting_producers_on_version = 0;
      auto ver_itr = producer_versions.begin();

      // The map should be sorted highest version to smallest, so we iterate until we hit the majority of voting_producers on at least this version
      while( ver_itr != producer_versions.end() )
      {
         voting_producers_on_version += ver_itr->second;

         if( voting_producers_on_version >= pso.hardfork_required_producers )
         {
            majority_version = ver_itr->first;
            break;
         }

         ++ver_itr;
      }

      auto hf_itr = hardfork_version_votes.begin();

      while( hf_itr != hardfork_version_votes.end() )
      {
         if( hf_itr->second >= pso.hardfork_required_producers )
         {
            const auto& hfp = db.get_hardfork_property_object();
            if( hfp.next_hardfork != std::get<0>( hf_itr->first ) ||
               hfp.next_hardfork_time != std::get<1>( hf_itr->first ) )
            {
               db.modify( hfp, [&]( hardfork_property_object& hpo )
               {
                  hpo.next_hardfork = std::get<0>( hf_itr->first );
                  hpo.next_hardfork_time = std::get<1>( hf_itr->first );
               });
            }
            break;
         }

         ++hf_itr;
      }

      // We no longer have a majority
      if( hf_itr == hardfork_version_votes.end() )
      {
         db.modify( db.get_hardfork_property_object(), [&]( hardfork_property_object& hpo )
         {
            hpo.next_hardfork = hpo.current_hardfork_version;
         });
      }

      vector< account_name_type > shuffled_voting_producers = db.shuffle_accounts( active_voting_producers );     // Shuffle the active voting_producers
      vector< account_name_type > shuffled_mining_producers = db.shuffle_accounts( active_mining_producers );     // Shuffle the active mining_producers
      expected_active_producers = std::min( size_t( TOTAL_PRODUCERS ), voting_power_producer_idx.size()+ mining_power_producer_idx.size() );

      for( size_t i = shuffled_voting_producers.size(); i < max_voting_producers; i++ )
      {
         shuffled_voting_producers[i] = account_name_type(); // Fills empty positions with empty account name
      }
      
      for( size_t i = shuffled_mining_producers.size(); i < max_mining_producers; i++ )
      {
         shuffled_mining_producers[i] = account_name_type(); // Fills empty positions with empty account name
      }

      size_t min_producers = std::min( shuffled_voting_producers.size(), shuffled_mining_producers.size() );

      db.modify( pso, [&]( producer_schedule_object& w )
      {
         for( size_t i = 0; i < min_producers; i++ )
         {
            w.current_shuffled_producers[2 * i] = shuffled_voting_producers[i]; // Adds a shuffled producer for every even number position
            w.current_shuffled_producers[2 * i + 1] = shuffled_mining_producers[i]; // Adds a shuffled miner for every odd position. 
         }
         w.top_voting_producers = top_voting_producer_set;
         w.top_mining_producers = top_mining_producer_set;

         w.num_scheduled_producers = std::max< uint8_t >( w.current_shuffled_producers.size(), 1 );
         w.current_voting_virtual_time = new_voting_virtual_time;
         w.current_mining_virtual_time = new_mining_virtual_time;
         w.next_shuffle_block_num = db.head_block_num() + w.num_scheduled_producers;
         w.majority_version = majority_version;
      });

      const account_authority_object& producer_auth = db.get_account_authority( PRODUCER_ACCOUNT );
      
      // Add top voting_producers to producer authority for producer fed bitassets
      db.modify( producer_auth, [&]( account_authority_object& a )
      {
         a.active.clear();
         a.active.weight_threshold = 1;

         for( account_name_type wit : top_voting_producer_set )
         {
            a.active.add_authority( wit, 1 );  
         }
      });

      update_median_producer_props( db );
   }
}

} } // node::chain
