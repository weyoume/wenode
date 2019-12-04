#pragma once
#include <fc/uint128.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/protocol/types.hpp>
#include <node/protocol/config.hpp>
#include <node/protocol/asset.hpp>
#include <node/protocol/chain_properties.hpp>

namespace node { namespace chain {

   using node::protocol::asset;
   using node::protocol::price;
   using node::protocol::chain_properties;

   /**
    * The values here are calculated during normal chain operations and reflect the
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

         id_type                id;

         uint32_t               head_block_number = 0;                                    // Number of the current highest block in the network that the node has processed. 

         block_id_type          head_block_id;                                            // The block id of the most recently produced block.

         uint32_t               last_irreversible_block_num = 0;                          // The number of the last block that has been produced on top of by at least #IRREVERSIBLE_THRESHOLD of block producers.

         block_id_type          last_irreversible_block_id;                               // The block id of the last irreversible block.

         uint32_t               last_committed_block_num = 0;                             // The number of the last block that has been committed by #IRREVERSIBLE_THRESHOLD of block producers.

         block_id_type          last_committed_block_id;                                  // The block id of the last irreversible block.
  
         account_name_type      current_producer;                                         // The account name of the current block producing witness or miner. 

         time_point             time;                                                     // Current blockchain time in microseconds;

         time_point             next_maintenance_time;                                    // Time in microseconds of the next maintenance period.
         
         chain_properties       median_props;                                             // Median properties object as desginated by the active block producers.               

         asset                  accumulated_network_revenue = asset(0, SYMBOL_COIN);      // Counter for the total of all core assets burned as network revenue.        

         price                  current_median_equity_price;                              // The current price of Equity Asset in Coin asset.

         price                  current_median_usd_price;                                 // The current price of the USd asst in the Coin asset.

         uint128_t              total_voting_power;                                       // Current outstanding supply of voting power in both equity and staked coin balances.

         uint128_t              total_pow = 0;                                            // The total POW accumulated

         uint64_t               current_aslot = 0;                                        // The current absolute slot number. Equal to the total number of slots since genesis.

         fc::uint128_t          recent_slots_filled;                                      // parameter used to compute witness participation.

         uint8_t                participation_count = 0;                                  // Divide by 128 to compute participation percentage

         uint32_t               dynamic_flags = 0;

         enum dynamic_flag_bits
         {
            /**
             * If maintenance_flag is set, then the head block is a
             * maintenance block.  This means
             * get_time_slot(1) - head_block_time() will have a gap
             * due to maintenance duration.
             *
             * This flag answers the question, "Was maintenance
             * performed in the last call to apply_block()?"
             */
            maintenance_flag = 0x01
         };
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
         (last_irreversible_block_num)
         (last_irreversible_block_id)
         (last_committed_block_num)
         (last_committed_block_id)
         (current_producer)
         (time)
         (next_maintenance_time)
         (median_props)
         (accumulated_network_revenue)
         (current_median_equity_price)
         (current_median_usd_price)
         (total_voting_power)
         (total_pow)
         (current_aslot)
         (recent_slots_filled)
         (participation_count)
         (dynamic_flags)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::dynamic_global_property_object, node::chain::dynamic_global_property_index );
