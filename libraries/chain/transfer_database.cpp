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


const transfer_request_object& database::get_transfer_request( const account_name_type& name, const shared_string& request_id )const
{ try {
   return get< transfer_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(request_id) ) }

const transfer_request_object* database::find_transfer_request( const account_name_type& name, const shared_string& request_id )const
{
   return find< transfer_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
}

const transfer_request_object& database::get_transfer_request( const account_name_type& name, const string& request_id )const
{ try {
   return get< transfer_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(request_id) ) }

const transfer_request_object* database::find_transfer_request( const account_name_type& name, const string& request_id )const
{
   return find< transfer_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
}

const transfer_recurring_object& database::get_transfer_recurring( const account_name_type& name, const shared_string& transfer_id )const
{ try {
   return get< transfer_recurring_object, by_transfer_id >( boost::make_tuple( name, transfer_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(transfer_id) ) }

const transfer_recurring_object* database::find_transfer_recurring( const account_name_type& name, const shared_string& transfer_id )const
{
   return find< transfer_recurring_object, by_transfer_id >( boost::make_tuple( name, transfer_id ) );
}

const transfer_recurring_object& database::get_transfer_recurring( const account_name_type& name, const string& transfer_id )const
{ try {
   return get< transfer_recurring_object, by_transfer_id >( boost::make_tuple( name, transfer_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(transfer_id) ) }

const transfer_recurring_object* database::find_transfer_recurring( const account_name_type& name, const string& transfer_id )const
{
   return find< transfer_recurring_object, by_transfer_id >( boost::make_tuple( name, transfer_id ) );
}

const transfer_recurring_request_object& database::get_transfer_recurring_request( const account_name_type& name, const shared_string& request_id )const
{ try {
   return get< transfer_recurring_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(request_id) ) }

const transfer_recurring_request_object* database::find_transfer_recurring_request( const account_name_type& name, const shared_string& request_id )const
{
   return find< transfer_recurring_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
}

const transfer_recurring_request_object& database::get_transfer_recurring_request( const account_name_type& name, const string& request_id )const
{ try {
   return get< transfer_recurring_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
} FC_CAPTURE_AND_RETHROW( (name)(request_id) ) }

const transfer_recurring_request_object* database::find_transfer_recurring_request( const account_name_type& name, const string& request_id )const
{
   return find< transfer_recurring_request_object, by_request_id >( boost::make_tuple( name, request_id ) );
}


void database::process_recurring_transfers()
{
   // ilog( "Process Recurring Transfers" );

   const dynamic_global_property_object& props = get_dynamic_global_properties();
   time_point now = props.time;
   const auto& transfer_idx = get_index< transfer_recurring_index >().indices().get< by_next_transfer >();
   auto transfer_itr = transfer_idx.begin();

   while( transfer_itr != transfer_idx.end() &&
      transfer_itr->next_transfer <= now )
   {
      const transfer_recurring_object& transfer = *transfer_itr;
      ++transfer_itr;
      asset liquid = get_liquid_balance( transfer.from, transfer.amount.symbol );

      if( liquid >= transfer.amount )    // Account has sufficient funds to pay
      {
         adjust_liquid_balance( transfer.from, -transfer.amount );
         adjust_liquid_balance( transfer.to, transfer.amount );

         modify( transfer, [&]( transfer_recurring_object& tro )
         {
            tro.next_transfer += tro.interval;
            tro.payments_remaining -= 1;
         });

         ilog( "Processed Recurring Transfer: \n ${t} \n",
            ("t",transfer));

         if( transfer.payments_remaining == 0 )
         {
            ilog( "Removed: ${v}",("v",transfer));
            remove( transfer );
         }
      }
      else     // Account cannot make the payment
      {
         if( transfer.fill_or_kill )     // Fill or kill causes transfer to be cancelled if payment cannot be made.
         {
            ilog( "Removed: ${v}",("v",transfer));
            remove( transfer );
         }
         else if( transfer.extensible )   // Extensible recurring transfer is extended if a payment is missed.
         {
            modify( transfer, [&]( transfer_recurring_object& tro )
            {
               tro.next_transfer += tro.interval;
               tro.end += tro.interval;
            });
            
            ilog( "Processed Recurring Transfer: \n ${t} \n",
               ("t",transfer));
         }
         else if( transfer.payments_remaining > 1 )    // Payments are remaining, not extensible, so payment is not extended
         {
            modify( transfer, [&]( transfer_recurring_object& tro )
            {
               tro.next_transfer += tro.interval;
               tro.payments_remaining -= 1;
            });

            ilog( "Processed Recurring Transfer: \n ${t} \n",
               ("t",transfer));
         }
         else     // No payments remaining
         {
            ilog( "Removed: ${v}",("v",transfer));
            remove( transfer );
         }
      }
   }
}

} } // node::chain