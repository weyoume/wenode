
#include <node/chain/database.hpp>
#include <node/chain/witness_objects.hpp>
#include <node/chain/witness_schedule.hpp>

#include <node/protocol/config.hpp>

namespace node { namespace chain {

void reset_witness_virtual_schedule_time( database& db )
{
   const witness_schedule_object& wso = db.get_witness_schedule();

   db.modify( wso, [&]( witness_schedule_object& o )
   {
      o.current_witness_virtual_time = fc::uint128();      // reset to 0
   });

   const auto& idx = db.get_index< witness_index >().indices();
   for( const auto& witness : idx )
   {
      db.modify( witness, [&]( witness_object& wobj )
      {
         wobj.witness_virtual_position = fc::uint128();
         wobj.witness_virtual_last_update = wso.current_witness_virtual_time;
         wobj.witness_virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH / ( wobj.voting_power.value + 1 );
      });
   }
}

void reset_miner_virtual_schedule_time( database& db )
{
   const witness_schedule_object& wso = db.get_witness_schedule();
   db.modify( wso, [&]( witness_schedule_object& o )
   {
      o.current_miner_virtual_time = fc::uint128(); // reset it 0
   });

   const auto& idx = db.get_index< witness_index >().indices();
   for( const auto& witness : idx )
   {
      db.modify( witness, [&]( witness_object& wobj )
      {
         wobj.miner_virtual_position = fc::uint128();
         wobj.miner_virtual_last_update = wso.current_miner_virtual_time;
         wobj.miner_virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH / ( wobj.mining_power.value + 1 );
      });
   }
}

/**
 * Update the median witness properties, according to the witness chain properties
 * median values for each variable.
 */
void update_median_witness_props( database& db )
{
   const witness_schedule_object& wso = db.get_witness_schedule();
   const dynamic_global_property_object props = db.get_dynamic_global_properties();
   vector< const witness_object* > active; 
   active.reserve( wso.num_scheduled_producers );
   size_t offset = active.size()/2;
   chain_properties new_props;

   for( int i = 0; i < wso.num_scheduled_producers; i++ )
   {
      active.push_back( db.find_witness( wso.current_shuffled_producers[i] ) );  // fetch witness object pointers
   }

   // Sort all properties variables and find median items, placing them into new properties. 
   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.account_creation_fee.amount < b->props.account_creation_fee.amount;
   });
   new_props.account_creation_fee = active[ offset ]->props.account_creation_fee;
   
   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.maximum_block_size < b->props.maximum_block_size;
   });
   new_props.maximum_block_size = active[ offset ]->props.maximum_block_size;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.pow_target_time < b->props.pow_target_time;
   });
   new_props.pow_target_time = active[ offset ]->props.pow_target_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.pow_decay_time < b->props.pow_decay_time;
   });
   new_props.pow_decay_time = active[ offset ]->props.pow_decay_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.txn_stake_decay_time < b->props.txn_stake_decay_time;
   });
   new_props.txn_stake_decay_time = active[ offset ]->props.txn_stake_decay_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.credit_interest_rate < b->props.credit_interest_rate;
   });
   new_props.credit_interest_rate = active[ offset ]->props.credit_interest_rate;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.credit_open_ratio < b->props.credit_open_ratio;
   });
   new_props.credit_open_ratio = active[ offset ]->props.credit_open_ratio;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.credit_liquidation_ratio < b->props.credit_liquidation_ratio;
   });
   new_props.credit_liquidation_ratio = active[ offset ]->props.credit_liquidation_ratio;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.credit_min_interest < b->props.credit_min_interest;
   });
   new_props.credit_min_interest = active[ offset ]->props.credit_min_interest;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.credit_variable_interest < b->props.credit_variable_interest;
   });
   new_props.credit_variable_interest = active[ offset ]->props.credit_variable_interest;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.market_max_credit_ratio < b->props.market_max_credit_ratio;
   });
   new_props.market_max_credit_ratio = active[ offset ]->props.market_max_credit_ratio;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.margin_open_ratio < b->props.margin_open_ratio;
   });
   new_props.margin_open_ratio = active[ offset ]->props.margin_open_ratio;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.margin_liquidation_ratio < b->props.margin_liquidation_ratio;
   });
   new_props.margin_liquidation_ratio = active[ offset ]->props.margin_liquidation_ratio;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.interest_compound_interval < b->props.interest_compound_interval;
   });
   new_props.interest_compound_interval = active[ offset ]->props.interest_compound_interval;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.maximum_asset_feed_publishers < b->props.maximum_asset_feed_publishers;
   });
   new_props.maximum_asset_feed_publishers = active[ offset ]->props.maximum_asset_feed_publishers;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.membership_base_price < b->props.membership_base_price;
   });
   new_props.membership_base_price = active[ offset ]->props.membership_base_price;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.membership_mid_price < b->props.membership_mid_price;
   });
   new_props.membership_mid_price = active[ offset ]->props.membership_mid_price;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.membership_top_price < b->props.membership_top_price;
   });
   new_props.membership_top_price = active[ offset ]->props.membership_top_price;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.content_reward_decay_rate < b->props.content_reward_decay_rate;
   });
   new_props.content_reward_decay_rate = active[ offset ]->props.content_reward_decay_rate;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.content_reward_interval < b->props.content_reward_interval;
   });
   new_props.content_reward_interval = active[ offset ]->props.content_reward_interval;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.vote_reserve_rate < b->props.vote_reserve_rate;
   });
   new_props.vote_reserve_rate = active[ offset ]->props.vote_reserve_rate;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.view_reserve_rate < b->props.view_reserve_rate;
   });
   new_props.view_reserve_rate = active[ offset ]->props.view_reserve_rate;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.share_reserve_rate < b->props.share_reserve_rate;
   });
   new_props.share_reserve_rate = active[ offset ]->props.share_reserve_rate;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.comment_reserve_rate < b->props.comment_reserve_rate;
   });
   new_props.comment_reserve_rate = active[ offset ]->props.comment_reserve_rate;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.vote_recharge_time < b->props.vote_recharge_time;
   });
   new_props.vote_recharge_time = active[ offset ]->props.vote_recharge_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.view_recharge_time < b->props.view_recharge_time;
   });
   new_props.view_recharge_time = active[ offset ]->props.view_recharge_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.share_recharge_time < b->props.share_recharge_time;
   });
   new_props.share_recharge_time = active[ offset ]->props.share_recharge_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.comment_recharge_time < b->props.comment_recharge_time;
   });
   new_props.comment_recharge_time = active[ offset ]->props.comment_recharge_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.curation_auction_decay_time < b->props.curation_auction_decay_time;
   });
   new_props.curation_auction_decay_time = active[ offset ]->props.curation_auction_decay_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.vote_curation_decay < b->props.vote_curation_decay;
   });
   new_props.vote_curation_decay = active[ offset ]->props.vote_curation_decay;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.view_curation_decay < b->props.view_curation_decay;
   });
   new_props.view_curation_decay = active[ offset ]->props.view_curation_decay;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.share_curation_decay < b->props.share_curation_decay;
   });
   new_props.share_curation_decay = active[ offset ]->props.share_curation_decay;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.comment_curation_decay < b->props.comment_curation_decay;
   });
   new_props.comment_curation_decay = active[ offset ]->props.comment_curation_decay;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.supernode_decay_time < b->props.supernode_decay_time;
   });
   new_props.supernode_decay_time = active[ offset ]->props.supernode_decay_time;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.enterprise_vote_percent_required < b->props.enterprise_vote_percent_required;
   });
   new_props.enterprise_vote_percent_required = active[ offset ]->props.enterprise_vote_percent_required;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.executive_role_type_amount < b->props.executive_role_type_amount;
   });
   new_props.executive_role_type_amount = active[ offset ]->props.executive_role_type_amount;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.maximum_asset_whitelist_authorities < b->props.maximum_asset_whitelist_authorities;
   });
   new_props.maximum_asset_whitelist_authorities = active[ offset ]->props.maximum_asset_whitelist_authorities;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.max_stake_intervals < b->props.max_stake_intervals;
   });
   new_props.max_stake_intervals = active[ offset ]->props.max_stake_intervals;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.max_unstake_intervals < b->props.max_unstake_intervals;
   });
   new_props.max_unstake_intervals = active[ offset ]->props.max_unstake_intervals;

   std::nth_element( active.begin(), active.begin() + offset, active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.max_exec_budget < b->props.max_exec_budget;
   });
   new_props.max_exec_budget = active[ offset ]->props.max_exec_budget;

   db.modify( wso, [&]( witness_schedule_object& w )
   {
      w.median_props = new_props;
   });

   db.modify( props, [&]( dynamic_global_property_object& dgpo )
   {
      dgpo.median_props = new_props;
   });
}

void update_witness_schedule( database& db )
{
   if( (db.head_block_num() % TOTAL_PRODUCERS) == 0 ) //  wso.next_shuffle_block_num 
   {
      const witness_schedule_object& wso = db.get_witness_schedule();
      const dynamic_global_property_object& gprops = db.get_dynamic_global_properties();
      fc::uint128 new_witness_virtual_time = wso.current_witness_virtual_time;
      fc::uint128 new_miner_virtual_time = wso.current_miner_virtual_time;

      vector< account_name_type > active_witnesses;
      vector< account_name_type > active_miners;

      uint8_t total_producers = wso.dpos_witness_producers + wso.dpos_witness_additional_producers + wso.pow_miner_producers +  wso.pow_miner_additional_producers;
      uint8_t max_witnesses = wso.dpos_witness_producers + wso.dpos_witness_additional_producers;
      uint8_t max_miners = wso.pow_miner_producers +  wso.pow_miner_additional_producers;

      FC_ASSERT( max_witnesses + max_miners == TOTAL_PRODUCERS,
         "Block production requires max witnesses and miners to add to total producers value." );
      FC_ASSERT( max_witnesses == max_miners,
         "Block production requires equal amounts of miners and witnesses." );

      active_witnesses.reserve( max_witnesses );
      active_miners.reserve( max_miners );

      // Add the highest voted witnesses
      flat_set< witness_id_type > top_witnesses;
      top_witnesses.reserve( wso.dpos_witness_producers );
                 
      const auto& widx = db.get_index< witness_index >().indices().get< by_voting_power >();
      for( auto itr = widx.begin();
         itr != widx.end() && top_witnesses.size() < max_witnesses;
         ++itr )
      {
         if( itr->signing_key == public_key_type() )
            continue;
         top_witnesses.insert( itr->id );
         active_witnesses.push_back( itr->owner );
         db.modify( *itr, [&]( witness_object& wo ) 
         { 
            wo.schedule = witness_object::top_witness; 
         });
      }

      auto num_top_witnesses = top_witnesses.size();

      /// Add miners from the top of the mining queue
      flat_set< witness_id_type > top_miners;
      top_miners.reserve( wso.pow_miner_producers );

      const auto& midx = db.get_index<witness_index>().indices().get<by_mining_power>();
      auto mitr = midx.begin();
      while( mitr != midx.end() && top_miners.size() < max_miners )
      {
         if( top_witnesses.find(mitr->id) == top_witnesses.end() ) // Only consider a miner who is not a top voted witness
         {
            if( !( db.get_witness( mitr->owner ).signing_key == public_key_type() ) ) // Only consider a miner who has a valid block signing key
            {
               top_miners.insert(mitr->id);
               active_miners.push_back(mitr->owner);
               db.modify( *mitr, [&]( witness_object& wo ) 
               { 
                  wo.schedule = witness_object::top_miner; 
               });
            }
         }
         ++mitr;
      }

      vector< account_name_type > top_witness_set = active_witnesses;    // Set of witnesses in the top selection
      vector< account_name_type > top_miner_set = active_miners;         // Set of miners in the top selection. 

      auto num_top_miners = top_miners.size();

      /// Add the additional witnesses in the lead
      flat_set< witness_id_type > additional_witnesses;
      additional_witnesses.reserve( wso.dpos_witness_additional_producers );

      const auto& witness_schedule_idx = db.get_index<witness_index>().indices().get<by_witness_schedule_time>();
      auto aw_itr = witness_schedule_idx.begin(); // Additional Witness iterator
      vector<decltype(aw_itr)> processed_witnesses;

      for( auto witness_count = num_top_witnesses;
         aw_itr != witness_schedule_idx.end() && witness_count < max_witnesses;
         ++aw_itr )
      {
         new_witness_virtual_time = aw_itr->witness_virtual_scheduled_time; /// everyone advances to at least this time
         processed_witnesses.push_back(aw_itr);

         if( aw_itr->signing_key == public_key_type() )
            continue; // skip witnesses without a valid block signing key

         if( top_miners.find(aw_itr->id) == top_miners.end()
            && top_witnesses.find(aw_itr->id) == top_witnesses.end() ) // skip producers already in the selection sets
         {
            additional_witnesses.insert(aw_itr->id);
            active_witnesses.push_back(aw_itr->owner);
            db.modify( *aw_itr, [&]( witness_object& wo ) 
            { 
               wo.schedule = witness_object::additional_witness;
            });

            ++witness_count;
         }
      }

      auto num_additional_witnesses = active_witnesses.size() - num_top_witnesses;

      // Add the additional miners in the lead.
      flat_set< witness_id_type > additional_miners;
      additional_miners.reserve( wso.pow_miner_additional_producers );

      const auto& miner_schedule_idx = db.get_index< witness_index >().indices().get< by_miner_schedule_time >();
      auto am_itr = miner_schedule_idx.begin();     // Additional miner iterator.
      vector<decltype(am_itr)> processed_miners;

      for( auto miner_count = num_top_miners;
         am_itr != miner_schedule_idx.end() && miner_count < max_miners;
         ++am_itr )
      {
         new_miner_virtual_time = am_itr->miner_virtual_scheduled_time;    // everyone advances to at least this time
         processed_miners.push_back( am_itr );

         if( am_itr->signing_key == public_key_type() )
         {
            continue;     // skip miners without a valid block signing key
         } 

         if( top_miners.find( am_itr->id ) == top_miners.end()
            && top_witnesses.find( am_itr->id ) == top_witnesses.end()
            && additional_witnesses.find( am_itr->id ) == additional_witnesses.end() )   // skip producers already in the selection sets
         {
            additional_miners.insert(am_itr->id);
            active_miners.push_back(am_itr->owner);
            db.modify( *am_itr, [&]( witness_object& wo ) 
            { 
               wo.schedule = witness_object::additional_miner; 
            });

            ++miner_count;
         }
      }

      auto num_additional_miners = active_miners.size() - num_top_miners;

      // Update virtual schedule of processed witnesses and miners
      bool reset_witness_virtual_time = false;
      bool reset_miner_virtual_time = false;

      for( auto itr = processed_witnesses.begin(); itr != processed_witnesses.end(); ++itr )
      {
         auto new_witness_virtual_scheduled_time = new_witness_virtual_time + VIRTUAL_SCHEDULE_LAP_LENGTH / ((*itr)->voting_power.value+1);
         if( new_witness_virtual_scheduled_time < new_witness_virtual_time )
         {
            reset_witness_virtual_time = true; // overflow
            break;
         }
         db.modify( *(*itr), [&]( witness_object& wo )
         {
            wo.witness_virtual_position = fc::uint128();
            wo.witness_virtual_last_update = new_witness_virtual_time;
            wo.witness_virtual_scheduled_time = new_witness_virtual_scheduled_time;
         });
      }

      if( reset_witness_virtual_time )
      {
         new_witness_virtual_time = fc::uint128();
         reset_witness_virtual_schedule_time(db);
      }

      for( auto itr = processed_miners.begin(); itr != processed_miners.end(); ++itr )
      {
         auto new_miner_virtual_scheduled_time = new_miner_virtual_time + VIRTUAL_SCHEDULE_LAP_LENGTH / ((*itr)->mining_power.value+1);
         if( new_miner_virtual_scheduled_time < new_miner_virtual_time )
         {
            reset_miner_virtual_time = true; // overflow
            break;
         }
         db.modify( *(*itr), [&]( witness_object& wo )
         {
            wo.miner_virtual_position = fc::uint128();
            wo.miner_virtual_last_update = new_miner_virtual_time;
            wo.miner_virtual_scheduled_time = new_miner_virtual_scheduled_time;
         });
      }

      if( reset_miner_virtual_time )
      {
         new_miner_virtual_time = fc::uint128();
         reset_miner_virtual_schedule_time( db );
      }

      size_t active_producers = active_witnesses.size() + active_miners.size();
      size_t expected_active_producers = max_witnesses + max_miners;

      FC_ASSERT( active_producers == expected_active_producers, 
         "Number of active producers does not equal expected producers",
         ("active_producers", active_producers) ("TOTAL_PRODUCERS",TOTAL_PRODUCERS) ("expected_active_producers", expected_active_producers) );

      FC_ASSERT( num_top_witnesses + num_top_miners + num_additional_witnesses + num_additional_miners == active_producers, 
         "Block production invariants invalid: Producer sum not equal to active producers", 
         ("num_top_witnesses", num_top_witnesses) ("num_top_miners", num_top_miners) ("num_additional_witnesses", num_additional_witnesses)
         ("num_additional_miners", num_additional_miners) ("active_producers", active_producers) );

      auto majority_version = wso.majority_version;

      flat_map< version, uint32_t, std::greater< version > > witness_versions;
      flat_map< std::tuple< hardfork_version, time_point >, uint32_t > hardfork_version_votes;

      for( uint32_t i = 0; i < wso.num_scheduled_producers; i++ )
      {
         auto witness = db.get_witness( wso.current_shuffled_producers[ i ] );
         if( witness_versions.find( witness.running_version ) == witness_versions.end() )
         {
            witness_versions[ witness.running_version ] = 1;
         }
         else
         {
            witness_versions[ witness.running_version ] += 1;
         }
         auto version_vote = std::make_tuple( witness.hardfork_version_vote, witness.hardfork_time_vote );
         if( hardfork_version_votes.find( version_vote ) == hardfork_version_votes.end() )
         {
            hardfork_version_votes[ version_vote ] = 1;
         }
         else
         {
            hardfork_version_votes[ version_vote ] += 1;
         } 
      }

      int witnesses_on_version = 0;
      auto ver_itr = witness_versions.begin();

      // The map should be sorted highest version to smallest, so we iterate until we hit the majority of witnesses on at least this version
      while( ver_itr != witness_versions.end() )
      {
         witnesses_on_version += ver_itr->second;

         if( witnesses_on_version >= wso.hardfork_required_witnesses )
         {
            majority_version = ver_itr->first;
            break;
         }

         ++ver_itr;
      }

      auto hf_itr = hardfork_version_votes.begin();

      while( hf_itr != hardfork_version_votes.end() )
      {
         if( hf_itr->second >= wso.hardfork_required_witnesses )
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

      vector< account_name_type > shuffled_witnesses = db.shuffle_accounts( active_witnesses );     // Shuffle the active witnesses
      vector< account_name_type > shuffled_miners = db.shuffle_accounts( active_miners );     // Shuffle the active miners
      size_t expected_active_producers = std::min( size_t( TOTAL_PRODUCERS ), widx.size()+ midx.size() );

      for( size_t i = shuffled_witnesses.size(); i < max_witnesses; i++ )
      {
         shuffled_witnesses[i] = account_name_type(); // Fills empty positions with empty account name
      }
      
      for( size_t i = shuffled_miners.size(); i < max_miners; i++ )
      {
         shuffled_miners[i] = account_name_type(); // Fills empty positions with empty account name
      }

      size_t min_producers = std::min( shuffled_witnesses.size(), shuffled_miners.size() );

      db.modify( wso, [&]( witness_schedule_object& w )
      {
         for( size_t i = 0; i < min_producers; i++ )
         {
            w.current_shuffled_producers[2 * i] = shuffled_witnesses[i]; // Adds a shuffled witness for every even number position
            w.current_shuffled_producers[2 * i + 1] = shuffled_miners[i]; // Adds a shuffled miner for every odd position. 
         }
         w.top_witnesses = top_witness_set;
         w.top_miners = top_miner_set;

         w.num_scheduled_producers = std::max< uint8_t >( w.current_shuffled_producers.size(), 1 );
         w.current_witness_virtual_time = new_witness_virtual_time;
         w.current_miner_virtual_time = new_miner_virtual_time;
         w.next_shuffle_block_num = db.head_block_num() + w.num_scheduled_producers;
         w.majority_version = majority_version;
      });

      update_median_witness_props( db );
   }
}

} } // node::chain
