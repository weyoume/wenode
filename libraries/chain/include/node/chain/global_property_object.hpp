#pragma once
#include <fc/uint128.hpp>

#include <node/chain/node_object_types.hpp>

#include <node/protocol/asset.hpp>

namespace node { namespace chain {

   using node::protocol::asset;
   using node::protocol::price;

   /**
    * @class dynamic_global_property_object
    * @brief Maintains global state information
    * @ingroup object
    * @ingroup implementation
    *
    * This is an implementation detail. The values here are calculated during normal chain operations and reflect the
    * current values of global blockchain properties.
    */
   class dynamic_global_property_object : public object< dynamic_global_property_object_type, dynamic_global_property_object>
   {
      public:
         template< typename Constructor, typename Allocator >
         dynamic_global_property_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         dynamic_global_property_object(){}

         id_type           id;

         uint32_t          head_block_number = 0;
         block_id_type     head_block_id;
         time_point_sec    time;
         account_name_type current_witness;


         /**
          *  The total POW accumulated, aka the sum of num_pow_witness at the time new POW is added
          */
         uint64_t total_pow = -1;

         /**
          * The current count of how many pending POW witnesses there are, determines the difficulty
          * of doing pow
          */
         uint32_t num_pow_witnesses = 0;

         asset       virtual_supply             = asset( 0, SYMBOL_COIN );
         asset       current_supply             = asset( 0, SYMBOL_COIN );
         asset       confidential_supply        = asset( 0, SYMBOL_COIN ); ///< total asset held in confidential balances
         asset       current_TSD_supply         = asset( 0, SYMBOL_USD );
         asset       confidential_TSD_supply    = asset( 0, SYMBOL_USD ); ///< total asset held in confidential balances
         asset       totalTMEfundForSCORE   = asset( 0, SYMBOL_COIN );
         asset       totalSCORE       = asset( 0, SYMBOL_SCORE );
         asset       total_reward_fund_TME    = asset( 0, SYMBOL_COIN );
         fc::uint128 totalSCOREreward2; ///< the running total of REWARD^2
         asset       pending_rewarded_SCORE = asset( 0, SYMBOL_SCORE );
         asset       pending_rewarded_SCOREvalueInTME = asset( 0, SYMBOL_COIN );

         price       get_SCORE_price() const
         {
            if ( totalTMEfundForSCORE.amount == 0 || totalSCORE.amount == 0 )
               return price ( asset( 1000, SYMBOL_COIN ), asset( 1000000, SYMBOL_SCORE ) );

            return price( totalSCORE, totalTMEfundForSCORE );
         }

         price get_SCOREreward_price() const
         {
            return price( totalSCORE + pending_rewarded_SCORE,
               totalTMEfundForSCORE + pending_rewarded_SCOREvalueInTME );
         }

         /**
          *  This property defines the interest rate that TSD deposits receive.
          */
         uint16_t TSD_interest_rate = 0;

         uint16_t TSD_print_rate = PERCENT_100;

         /**
          *  Maximum block size is decided by the set of active witnesses which change every round.
          *  Each witness posts what they think the maximum size should be as part of their witness
          *  properties, the median size is chosen to be the maximum block size for the round.
          *
          *  @note the minimum value for maximum_block_size is defined by the protocol to prevent the
          *  network from getting stuck by witnesses attempting to set this too low.
          */
         uint32_t     maximum_block_size = 0;

         /**
          * The current absolute slot number.  Equal to the total
          * number of slots since genesis.  Also equal to the total
          * number of missed slots plus head_block_number.
          */
         uint64_t      current_aslot = 0;

         /**
          * used to compute witness participation.
          */
         fc::uint128_t recent_slots_filled;
         uint8_t       participation_count = 0; ///< Divide by 128 to compute participation percentage

         uint32_t last_irreversible_block_num = 0;

         /**
          * The number of votes regenerated per day.  Any user voting slower than this rate will be
          * "wasting" voting power through spillover; any user voting faster than this rate will have
          * their votes reduced.
          */
         uint32_t vote_power_reserve_rate = 40;
   };

   typedef multi_index_container<
      dynamic_global_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< dynamic_global_property_object, dynamic_global_property_object::id_type, &dynamic_global_property_object::id > >
      >,
      allocator< dynamic_global_property_object >
   > dynamic_global_property_index;

} } // node::chain

FC_REFLECT( node::chain::dynamic_global_property_object,
             (id)
             (head_block_number)
             (head_block_id)
             (time)
             (current_witness)
             (total_pow)
             (num_pow_witnesses)
             (virtual_supply)
             (current_supply)
             (confidential_supply)
             (current_TSD_supply)
             (confidential_TSD_supply)
             (totalTMEfundForSCORE)
             (totalSCORE)
             (total_reward_fund_TME)
             (totalSCOREreward2)
             (pending_rewarded_SCORE)
             (pending_rewarded_SCOREvalueInTME)
             (TSD_interest_rate)
             (TSD_print_rate)
             (maximum_block_size)
             (current_aslot)
             (recent_slots_filled)
             (participation_count)
             (last_irreversible_block_num)
             (vote_power_reserve_rate)
          )
CHAINBASE_SET_INDEX_TYPE( node::chain::dynamic_global_property_object, node::chain::dynamic_global_property_index )
