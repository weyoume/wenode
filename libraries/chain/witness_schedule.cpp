
#include <node/chain/database.hpp>
#include <node/chain/witness_objects.hpp>
#include <node/chain/witness_schedule.hpp>

#include <node/protocol/config.hpp>

namespace node { namespace chain {

void reset_witness_virtual_schedule_time( database& db )
{
   const witness_schedule_object& wso = db.get_witness_schedule();
   db.modify( wso, [&](witness_schedule_object& o )
   {
       o.current_witness_virtual_time = fc::uint128(); // reset it 0
   } );

   const auto& idx = db.get_index<witness_index>().indices();
   for( const auto& witness : idx )
   {
      db.modify( witness, [&]( witness_object& wobj )
      {
         wobj.witness_virtual_position = fc::uint128();
         wobj.witness_virtual_last_update = wso.current_witness_virtual_time;
         wobj.witness_virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH / (wobj.voting_power.value+1);
      } );
   }
}

void reset_miner_virtual_schedule_time( database& db )
{
   const witness_schedule_object& wso = db.get_witness_schedule();
   db.modify( wso, [&](witness_schedule_object& o )
   {
       o.current_miner_virtual_time = fc::uint128(); // reset it 0
   } );

   const auto& idx = db.get_index<witness_index>().indices();
   for( const auto& witness : idx )
   {
      db.modify( witness, [&]( witness_object& wobj )
      {
         wobj.miner_virtual_position = fc::uint128();
         wobj.miner_virtual_last_update = wso.current_miner_virtual_time;
         wobj.miner_virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH / (wobj.mining_power.value+1);
      } );
   }
}

void update_median_witness_props( database& db )
{
   const witness_schedule_object& wso = db.get_witness_schedule();

   /// fetch all witness objects
   vector<const witness_object*> active; active.reserve( wso.num_scheduled_producers );
   for( int i = 0; i < wso.num_scheduled_producers; i++ )
   {
      active.push_back( &db.get_witness( wso.current_shuffled_producers[i] ) );
   }

   /// sort them by account_creation_fee
   std::sort( active.begin(), active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.account_creation_fee.amount < b->props.account_creation_fee.amount;
   } );
   asset median_account_creation_fee = active[active.size()/2]->props.account_creation_fee;

   /// sort them by maximum_block_size
   std::sort( active.begin(), active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.maximum_block_size < b->props.maximum_block_size;
   } );
   uint32_t median_maximum_block_size = active[active.size()/2]->props.maximum_block_size;

   /// sort them by credit_interest_rate
   std::sort( active.begin(), active.end(), [&]( const witness_object* a, const witness_object* b )
   {
      return a->props.credit_interest_rate < b->props.credit_interest_rate;
   } );
   uint16_t median_credit_interest_rate = active[active.size()/2]->props.credit_interest_rate;

   db.modify( wso, [&]( witness_schedule_object& _wso )
   {
      _wso.median_props.account_creation_fee = median_account_creation_fee;
      _wso.median_props.maximum_block_size   = median_maximum_block_size;
      _wso.median_props.credit_interest_rate    = median_credit_interest_rate;
   } );

   db.modify( db.get_dynamic_global_properties(), [&]( dynamic_global_property_object& _dgpo )
   {
      _dgpo.maximum_block_size = median_maximum_block_size;
      _dgpo.credit_interest_rate  = median_credit_interest_rate;
   } );
}

// Shuffle current producer sets
// High performance random generator using 256 bits of internal state. 
/// http://xorshift.di.unimi.it/
vector< account_name_type > shuffle_producers( database& db, vector< account_name_type >& producer_set )  
{
   auto now_hi = uint64_t(db.head_block_time().time_since_epoch().count()) << 32;
   for( uint32_t i = 0; i < producer_set.size(); ++i )
   {
      uint64_t k = now_hi +      uint64_t(i)*26857571057736338717ULL;
      uint64_t l = now_hi >> 1 + uint64_t(i)*95198191871878293511ULL;
      uint64_t m = now_hi >> 2 + uint64_t(i)*58919729024841961988ULL;
      uint64_t n = now_hi >> 3 + uint64_t(i)*27137164109707054410ULL;
      
      k ^= (l >> 7);
      l ^= (m << 9);
      m ^= (n >> 5);
      n ^= (k << 3);

      k*= 14226572565896741612ULL;
      l*= 91985878658736871034ULL;
      m*= 30605588311672529089ULL;
      n*= 43069213742576315243ULL;

      k ^= (l >> 2);
      l ^= (m << 13);
      m ^= (n >> 1);
      n ^= (k << 9);

      k*= 79477756532752495704ULL;
      l*= 94908025588282034792ULL;
      m*= 26941980616458623416ULL;
      n*= 31902236862011382134ULL;

      uint64_t rand = (k ^ l) ^ (m ^ n) ; 
      uint32_t max = producer_set.size() - i;

      uint32_t j = i + rand % max;
      std::swap( producer_set[i], producer_set[j] );
   }
}

void update_witness_schedule( database& db )
{
   if( (db.head_block_num() % TOTAL_PRODUCERS) == 0 ) //wso.next_shuffle_block_num 
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

      FC_ASSERT( max_witnesses + max_miners == TOTAL_PRODUCERS, "Block production requires max witnesses and miners to add to total producers value.");
      FC_ASSERT( max_witnesses == max_miners, "Block production requires equal amounts of miners and witnesses");

      active_witnesses.reserve( max_witnesses );
      active_miners.reserve( max_miners );

      /// Add the highest voted witnesses
      flat_set< witness_id_type > top_witnesses;
      top_witnesses.reserve( wso.dpos_witness_producers );
                 
      const auto& widx = db.get_index<witness_index>().indices().get<by_voting_power>();
      for( auto itr = widx.begin();
         itr != widx.end() && top_witnesses.size() < max_witnesses;
         ++itr )
      {
         if( itr->signing_key == public_key_type() )
            continue;
         top_witnesses.insert( itr->id );
         active_witnesses.push_back( itr->owner);
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

      const auto& miner_schedule_idx = db.get_index<witness_index>().indices().get<by_miner_schedule_time>();
      auto am_itr = miner_schedule_idx.begin(); // Additional miner iterator.
      vector<decltype(am_itr)> processed_miners;

      for( auto miner_count = num_top_miners;
         am_itr != miner_schedule_idx.end() && miner_count < max_miners;
         ++am_itr )
      {
         new_miner_virtual_time = am_itr->miner_virtual_scheduled_time; // everyone advances to at least this time
         processed_miners.push_back(am_itr);

         if( am_itr->signing_key == public_key_type() )
            continue; // skip miners without a valid block signing key

         if( top_miners.find(am_itr->id) == top_miners.end()
            && top_witnesses.find(am_itr->id) == top_witnesses.end()
            && additional_witnesses.find(am_itr->id) == additional_witnesses.end() ) // skip producers already in the selection sets
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
            wo.witness_virtual_position        = fc::uint128();
            wo.witness_virtual_last_update     = new_witness_virtual_time;
            wo.witness_virtual_scheduled_time  = new_witness_virtual_scheduled_time;
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
            wo.miner_virtual_position        = fc::uint128();
            wo.miner_virtual_last_update     = new_miner_virtual_time;
            wo.miner_virtual_scheduled_time  = new_miner_virtual_scheduled_time;
         });
      }

      if( reset_miner_virtual_time )
      {
         new_miner_virtual_time = fc::uint128();
         reset_miner_virtual_schedule_time(db);
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
            witness_versions[ witness.running_version ] = 1;
         else
            witness_versions[ witness.running_version ] += 1;

         auto version_vote = std::make_tuple( witness.hardfork_version_vote, witness.hardfork_time_vote );
         if( hardfork_version_votes.find( version_vote ) == hardfork_version_votes.end() )
            hardfork_version_votes[ version_vote ] = 1;
         else
            hardfork_version_votes[ version_vote ] += 1;
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
                  hfp.next_hardfork_time != std::get<1>( hf_itr->first ) ) {

               db.modify( hfp, [&]( hardfork_property_object& hpo )
               {
                  hpo.next_hardfork = std::get<0>( hf_itr->first );
                  hpo.next_hardfork_time = std::get<1>( hf_itr->first );
               } );
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

      vector<account_name_type> shuffled_witnesses = shuffle_producers(db, active_witnesses); // Shuffle the active witnesses
      vector<account_name_type> shuffled_miners = shuffle_producers(db, active_miners); // Shuffle the active miners
      size_t expected_active_producers = std::min( size_t(TOTAL_PRODUCERS), widx.size()+ midx.size() );

      for( size_t i = shuffled_witnesses.size(); i < max_witnesses; i++ )
      {
         shuffled_witnesses[i] = account_name_type(); // Fills empty positions with empty account name
      }
      
      for( size_t i = shuffled_miners.size(); i < max_miners; i++ )
      {
         shuffled_miners[i] = account_name_type(); // Fills empty positions with empty account name
      }

      size_t min_producers = std::min(shuffled_witnesses.size(), shuffled_miners.size() );

      db.modify( wso, [&]( witness_schedule_object& _wso )
      {
         for( size_t i = 0; i < min_producers; i++ )
         {
            _wso.current_shuffled_producers[2 * i] = shuffled_witnesses[i]; // Adds a shuffled witness for every even number position
            _wso.current_shuffled_producers[2 * i + 1] = shuffled_miners[i]; // Adds a shuffled miner for every odd position. 
         }
         _wso.top_witnesses = top_witness_set;
         _wso.top_miners = top_miner_set;

         _wso.num_scheduled_producers = std::max< uint8_t >( _wso.current_shuffled_producers.size(), 1 );
         _wso.current_witness_virtual_time = new_witness_virtual_time;
         _wso.current_miner_virtual_time = new_miner_virtual_time;
         _wso.next_shuffle_block_num = db.head_block_num() + _wso.num_scheduled_producers;
         _wso.majority_version = majority_version;
      } );

      update_median_witness_props(db);
   }
}

} } // node::chain
