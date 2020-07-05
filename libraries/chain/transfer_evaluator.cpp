
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


//=============================//
// === Transfer Evaluators === //
//=============================//


/**
 * Transfers an amount of a liquid asset balance from one account to another.
 */
void transfer_evaluator::do_apply( const transfer_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& from_account = _db.get_account( o.from );
   const account_object& to_account = _db.get_account( o.to );
   FC_ASSERT( to_account.active, 
      "Account: ${s} must be active to receive transfer.",("s", o.to) );
   asset liquid = _db.get_liquid_balance( from_account.name, o.amount.symbol );

   FC_ASSERT( liquid >= o.amount, 
      "Account does not have sufficient funds for transfer." );

   time_point now = _db.head_block_time();
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.to );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.from );

   if( asset_obj.asset_type == asset_property_type::UNIQUE_ASSET )
   {
      FC_ASSERT( o.amount.amount == BLOCKCHAIN_PRECISION,
         "Unique asset must be transferred as a single unit asset." );
      const asset_unique_data_object& unique = _db.get_unique_data( asset_obj.symbol );

      _db.modify( unique, [&]( asset_unique_data_object& audo )
      {
         audo.controlling_owner = o.to;
      });
   }
   else if( asset_obj.asset_type == asset_property_type::STIMULUS_ASSET )
   {
      const asset_stimulus_data_object& stimulus = _db.get_stimulus_data( asset_obj.symbol );
      FC_ASSERT( stimulus.is_redemption( o.to ),
         "Stimulus asset must be transferred to members of the redemption list." );
   }

   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.from, asset_obj ),
      "Transfer is not authorized, due to recipient account's asset permisssions" );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.to, asset_obj ),
      "Transfer is not authorized, due to sender account's asset permisssions" );

   _db.modify( from_account, [&]( account_object& a )
   {
      a.last_transfer_time = now;
   });
   
   vector<string> part; 
   part.reserve(4);
   auto path = o.memo;
   boost::split( part, path, boost::is_any_of("/") );
   if( part.size() > 1 && part[0].size() && part[0][0] == '@' )   // find memo reference to comment, add transfer to payments received.
   {
      auto acnt = part[0].substr(1);
      auto perm = part[1];

      auto comment_ptr = _db.find_comment( acnt, perm );
      if( comment_ptr != nullptr )
      {
         const comment_object& comment = *comment_ptr;
         
         _db.modify( comment, [&]( comment_object& co )
         {
            if( co.payments_received[ o.from ].size() )
            {
               if( co.payments_received[ o.from ][ o.amount.symbol ].amount > 0 )
               {
                  co.payments_received[ o.from ][ o.amount.symbol ] += o.amount;
               }
               else
               {
                  co.payments_received[ o.from ][ o.amount.symbol ] = o.amount;
               }
            }
            else
            {
               co.payments_received[ o.from ][ o.amount.symbol ] = o.amount;
            }
         });
      }
   }

   _db.adjust_liquid_balance( from_account.name, -o.amount );
   _db.adjust_liquid_balance( to_account.name, o.amount );

} FC_CAPTURE_AND_RETHROW( ( o )) }


void transfer_request_evaluator::do_apply( const transfer_request_operation& o )
{ try {
   const account_name_type& signed_for = o.to;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& from_account = _db.get_account( o.from );
   FC_ASSERT( from_account.active, 
      "Account: ${s} must be active to receive transfer request.",("s", o.from) );
   time_point now = _db.head_block_time();
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.to );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.from );

   if( asset_obj.asset_type == asset_property_type::UNIQUE_ASSET )
   {
      FC_ASSERT( o.amount.amount == BLOCKCHAIN_PRECISION, 
         "Unique asset must be transferred as a single unit asset." );
   }

   asset from_liquid = _db.get_liquid_balance( o.from, o.amount.symbol );

   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.from, asset_obj ),
      "Transfer is not authorized, due to recipient account's asset permisssions." );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.to, asset_obj ),
      "Transfer is not authorized, due to sender account's asset permisssions." );
   FC_ASSERT( from_liquid.amount >= o.amount.amount,
         "Account: ${a} does not have sufficient funds: ${b} for transfer of amount: ${m}.",
         ("a",o.from)("b",from_liquid)("m",o.amount) );

   const auto& req_idx = _db.get_index< transfer_request_index >().indices().get< by_request_id >();
   auto req_itr = req_idx.find( boost::make_tuple( o.to, o.request_id ) );

   if( req_itr == req_idx.end() )    // Transfer request does not exist, creating new request.
   {
      FC_ASSERT( o.requested, 
         "Transfer request does not exist to cancel, set requested to true." );

      _db.create< transfer_request_object >( [&]( transfer_request_object& tro )
      {
         tro.to = o.to;
         tro.from = o.from;
         from_string( tro.memo, o.memo );
         from_string( tro.request_id, o.request_id );
         tro.amount = o.amount;
         tro.expiration = now + TRANSFER_REQUEST_DURATION;
      });
   }
   else
   {
      if( o.requested )
      {
         _db.modify( *req_itr, [&]( transfer_request_object& tro )
         {
            from_string( tro.memo, o.memo );
            tro.amount = o.amount;
         });
      }
      else
      {
         ilog( "Removed: ${v}",("v",*req_itr));
         _db.remove( *req_itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void transfer_accept_evaluator::do_apply( const transfer_accept_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& from_account = _db.get_account( o.from );
   const account_object& to_account = _db.get_account( o.to );

   FC_ASSERT( to_account.active, 
      "Account: ${s} must be active to receive transfer.",("s", o.to) );

   const transfer_request_object& request = _db.get_transfer_request( o.to, o.request_id );
   const asset_object& asset_obj = _db.get_asset( request.amount.symbol );
   time_point now = _db.head_block_time();
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.to );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.from );

   if( asset_obj.asset_type == asset_property_type::UNIQUE_ASSET )
   {
      FC_ASSERT( request.amount.amount == BLOCKCHAIN_PRECISION,
         "Unique asset must be transferred as a single unit asset." );
   }

   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.from, asset_obj ),
      "Transfer is not authorized, due to recipient account's asset permisssions" );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.to, asset_obj ),
      "Transfer is not authorized, due to sender account's asset permisssions" );
   FC_ASSERT( now < request.expiration,
      "Request has expired." );

   if( o.accepted )
   {
      FC_ASSERT( _db.get_liquid_balance( from_account.name, request.amount.symbol ) >= request.amount,
         "Account does not have sufficient funds for transfer." );
      _db.adjust_liquid_balance( request.from, -request.amount );
      _db.adjust_liquid_balance( request.to, request.amount );

      _db.modify( from_account, [&]( account_object& a )
      {
         a.last_transfer_time = now;
      });
   }
   else
   {
      ilog( "Removed: ${v}",("v",request));
      _db.remove( request );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void transfer_recurring_evaluator::do_apply( const transfer_recurring_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& from_account = _db.get_account( o.from );
   const account_object& to_account = _db.get_account( o.to );
   FC_ASSERT( to_account.active, 
      "Account: ${s} must be active to receive transfer.",("s", o.to) );
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );
   time_point now = _db.head_block_time();
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.to );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.from );
   asset from_liquid = _db.get_liquid_balance( o.from, o.amount.symbol );

   FC_ASSERT( from_liquid >= o.amount,
      "Account: ${f} does not have sufficient funds for transfer amount.",("f", o.from) );
   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.from, asset_obj ),
      "Transfer is not authorized, due to recipient account's asset permisssions" );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.to, asset_obj ),
      "Transfer is not authorized, due to sender account's asset permisssions" );
   FC_ASSERT( !( o.extensible && o.fill_or_kill ),
      "Recurring transfer cannot be both extensible and fill or kill." );
   
   const auto& recurring_idx = _db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
   auto recurring_itr = recurring_idx.find( boost::make_tuple( from_account.name, o.transfer_id ) );

   if( recurring_itr == recurring_idx.end() )    // Recurring transfer does not exist, creating new recurring transfer.
   {
      FC_ASSERT( o.active,
         "Recurring transfer does not exist to cancel, set active to true." );
      FC_ASSERT( o.begin > now,
         "Begin time must be in the future." );

      _db.create< transfer_recurring_object >( [&]( transfer_recurring_object& tro )
      {
         tro.to = to_account.name;
         tro.from = from_account.name;
         from_string( tro.memo, o.memo );
         from_string( tro.transfer_id, o.transfer_id );
         tro.amount = o.amount;
         tro.begin = o.begin;
         tro.payments = o.payments;
         tro.payments_remaining = o.payments;
         tro.end = o.begin + fc::microseconds( o.interval.count() * ( o.payments - 1 ) );
         tro.interval = o.interval;
         tro.next_transfer = o.begin;
         tro.extensible = o.extensible;
         tro.fill_or_kill = o.fill_or_kill;
      });
   }
   else
   {
      if( o.active )
      {
         int32_t prev_remaining = recurring_itr->payments_remaining;
         int32_t delta_remaining = o.payments - prev_remaining;
         int32_t new_remaining = prev_remaining += delta_remaining;
         time_point next_transfer = recurring_itr->next_transfer;
         time_point init_begin = recurring_itr->begin;
         time_point new_end = next_transfer + fc::microseconds( o.interval.count() * ( new_remaining - 1 ) );

         FC_ASSERT( new_end > now,
            "Cannot change payment schedule to result in a completion time in the past." );

         if( init_begin < now )
         {
            FC_ASSERT( o.begin == init_begin,
               "Cannot change payment begin time after payment has already begun." );
         }

         _db.modify( *recurring_itr, [&]( transfer_recurring_object& tro )
         {
            tro.interval = o.interval;
            from_string( tro.memo, o.memo );
            tro.amount = o.amount;
            tro.payments = o.payments;
            tro.payments_remaining = new_remaining;
            tro.begin = o.begin;
            tro.end = new_end;
            tro.extensible = o.extensible;
            tro.fill_or_kill = o.fill_or_kill;
         });
      }
      else
      {
         ilog( "Removed: ${v}",("v",*recurring_itr));
         _db.remove( *recurring_itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void transfer_recurring_request_evaluator::do_apply( const transfer_recurring_request_operation& o )
{ try {
   const account_name_type& signed_for = o.to;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& from_account = _db.get_account( o.from );
   FC_ASSERT( from_account.active, 
      "Account: ${s} must be active to receive transfer request.",("s", o.from) );
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );
   time_point now = _db.head_block_time();
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.to );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.from );
   asset from_liquid = _db.get_liquid_balance( o.from, o.amount.symbol );

   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.from, asset_obj ),
      "Transfer is not authorized, due to recipient account's asset permisssions" );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.to, asset_obj ),
      "Transfer is not authorized, due to sender account's asset permisssions" );
   FC_ASSERT( from_liquid >= o.amount,
      "Account does not have sufficient funds for first transfer." );
   FC_ASSERT( !( o.extensible && o.fill_or_kill ),
      "Recurring transfer cannot be both extensible and fill or kill." );

   const auto& req_idx = _db.get_index< transfer_recurring_request_index >().indices().get< by_request_id >();
   auto req_itr = req_idx.find( boost::make_tuple( o.to, o.request_id ) );

   if( req_itr == req_idx.end() )   // Recurring transfer request does not exist, creating new recurring transfer request.
   {
      FC_ASSERT( o.requested,
         "Recurring transfer request does not exist to cancel, set active to true." );
      FC_ASSERT( o.begin > now,
         "Begin time must be in the future." );
      FC_ASSERT( o.expiration <= o.begin,
         "Expiration time must be before or equal to begin time." );

      _db.create< transfer_recurring_request_object >( [&]( transfer_recurring_request_object& trro )
      {
         trro.to = o.to;
         trro.from = o.from;
         from_string( trro.memo, o.memo );
         from_string( trro.request_id, o.request_id );
         trro.amount = o.amount;
         trro.begin = o.begin;
         trro.end = o.begin + fc::microseconds( o.interval.count() * ( o.payments - 1 ) );
         trro.interval = o.interval;
         trro.payments = o.payments;
         trro.expiration = o.expiration;
         trro.extensible = o.extensible;
         trro.fill_or_kill = o.fill_or_kill;
      });
   }
   else
   {
      if( o.requested )
      {
         _db.modify( *req_itr, [&]( transfer_recurring_request_object& trro )
         {
            from_string( trro.memo, o.memo );
            trro.amount = o.amount;
            trro.payments = o.payments;
            trro.begin = o.begin;
            trro.end = o.begin + fc::microseconds( o.interval.count() * ( o.payments - 1 ) );
            trro.interval = o.interval;
            trro.extensible = o.extensible;
            trro.fill_or_kill = o.fill_or_kill;
         });
      }
      else
      {
         ilog( "Removed: ${v}",("v",*req_itr));
         _db.remove( *req_itr );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void transfer_recurring_accept_evaluator::do_apply( const transfer_recurring_accept_operation& o )
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& to_account = _db.get_account( o.to );
   FC_ASSERT( to_account.active, 
      "Account: ${s} must be active to receive transfer.",("s", o.to) );
   const account_object& from_account = _db.get_account( o.from );
   const transfer_recurring_request_object& request = _db.get_transfer_recurring_request( to_account.name, o.request_id );
   const asset_object& asset_obj = _db.get_asset( request.amount.symbol );
   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.to );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.from );
   asset from_liquid = _db.get_liquid_balance( request.from, request.amount.symbol );

   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.from, asset_obj ),
      "Transfer is not authorized, due to recipient account's asset permisssions" );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.to, asset_obj ),
      "Transfer is not authorized, due to sender account's asset permisssions" );
   FC_ASSERT( from_liquid >= request.amount,
      "Account does not have sufficient funds for first transfer." );

   const auto& recurring_idx = _db.get_index< transfer_recurring_index >().indices().get< by_transfer_id >();
   auto recurring_itr = recurring_idx.find( boost::make_tuple( from_account.name, o.request_id ) );   // Request ID is used as new transfer ID when accepted. 

   FC_ASSERT( recurring_itr == recurring_idx.end(),
      "Recurring transfer with this ID already exists, cannot accept again." );   // Recurring transfer does not exist.
   
   if( o.accepted )   // Accepting transfer by creating new transfer.
   {
      _db.create< transfer_recurring_object >( [&]( transfer_recurring_object& tro )
      {
         tro.to = request.to;
         tro.from = request.from;
         tro.memo = request.memo;
         tro.transfer_id = request.request_id;  // Request id becomes new transfer ID
         tro.amount = request.amount;
         tro.payments = request.payments;
         tro.payments_remaining = request.payments;
         tro.begin = request.begin;
         tro.end = request.end;
         tro.interval = request.interval;
         tro.next_transfer = request.begin;
         tro.extensible = request.extensible;
         tro.fill_or_kill = request.fill_or_kill;
      });
   }
   else    // Rejecting recurring transfer request.
   {
      ilog( "Removed: ${v}",("v",request));
      _db.remove( request );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void transfer_confidential_evaluator::do_apply( const transfer_confidential_operation& o ) 
{ try {
   const auto& confidential_idx = _db.get_index< confidential_balance_index >().indices().get< by_commitment >();
   auto confidential_itr = confidential_idx.begin();

   const asset_object& asset_obj = _db.get_asset( o.fee.symbol );
   transaction_id_type txid = _db.get_current_transaction_id();
   uint16_t op_in_trx = _db.get_current_op_in_trx();
   time_point now = _db.head_block_time();

   FC_ASSERT( asset_obj.enable_confidential(), 
      "Asset does not enable confidential transfers." );
   FC_ASSERT( !asset_obj.is_transfer_restricted(),
      "Asset is transfer restricted." );
   FC_ASSERT( !asset_obj.require_balance_whitelist(),
      "Asset requires a whitelist to hold balance." );

   for( const auto& out : o.outputs )
   {
      out.owner.validate();
      for( const auto& a : out.owner.account_auths )
      {
         const account_object& acc = _db.get_account( a.first );
         FC_ASSERT( acc.active,
            "Account must be active to be balance account authority." );
      }
   }

   for( const auto& in : o.inputs )
   {
      confidential_itr = confidential_idx.find( in.commitment );
      FC_ASSERT( confidential_itr != confidential_idx.end(),
         "Input Commitment is not found", ("commitment",in.commitment) );
      FC_ASSERT( confidential_itr->symbol == o.fee.symbol,
         "Confidential Balance must be the same asset as fee asset." );
      FC_ASSERT( confidential_itr->owner == in.owner,
         "Confidential Balance owner is not the same as input balance." );
   }

   for( const auto& in : o.inputs )
   {
      confidential_itr = confidential_idx.find( in.commitment );
      FC_ASSERT( confidential_itr != confidential_idx.end(), 
         "Input Commitment is not found", ("commitment",in.commitment) );
      ilog( "Removed: ${v}",("v",*confidential_itr));
      _db.remove( *confidential_itr );
   }

   for( size_t i = 0; i < o.outputs.size(); i++ )
   {
      confidential_output out = o.outputs[ i ];
      uint16_t index = i;
      
      _db.create< confidential_balance_object >( [&]( confidential_balance_object& cbo )
      {
         cbo.owner = out.owner;
         cbo.commitment = out.commitment;
         cbo.prev = txid;
         cbo.op_in_trx = op_in_trx;
         cbo.index = index;
         cbo.created = now;
      });
   }

   _db.pay_network_fees( o.fee );
   _db.adjust_confidential_supply( -o.fee );

} FC_CAPTURE_AND_RETHROW( (o) ) }


void transfer_to_confidential_evaluator::do_apply( const transfer_to_confidential_operation& o ) 
{ try {
   const account_name_type& signed_for = o.from;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );
   transaction_id_type txid = _db.get_current_transaction_id();
   uint16_t op_in_trx = _db.get_current_op_in_trx();
   time_point now = _db.head_block_time();

   FC_ASSERT( asset_obj.enable_confidential(), 
      "Asset does not enable confidential transfers." );
   FC_ASSERT( !asset_obj.is_transfer_restricted(),
      "Asset is transfer restricted." );
   FC_ASSERT( !asset_obj.require_balance_whitelist(),
      "Asset requires a whitelist to hold balance." );

   for( confidential_output out : o.outputs )
   {
      out.owner.validate();
      for( const auto& a : out.owner.account_auths )
      {
         const account_object& acc = _db.get_account( a.first );
         FC_ASSERT( acc.active,
            "Account must be active to be balance account authority." );
      }
   }

   asset from_liquid = _db.get_liquid_balance( o.from, asset_obj.symbol );
   FC_ASSERT( from_liquid >= ( o.amount + o.fee ),
      "Account does not have sufficient liquid balance for confidential transfer." );

   _db.adjust_liquid_balance( o.from, -( o.amount + o.fee ) );
   _db.pay_network_fees( o.fee );
   _db.adjust_confidential_supply( o.amount );

   for( size_t i = 0; i < o.outputs.size(); i++ )
   {
      confidential_output out = o.outputs[ i ];
      uint16_t index = i;
      
      _db.create< confidential_balance_object >( [&]( confidential_balance_object& cbo )
      {
         cbo.owner = out.owner;
         cbo.commitment = out.commitment;
         cbo.prev = txid;
         cbo.op_in_trx = op_in_trx;
         cbo.index = index;
         cbo.created = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( (o) ) }


void transfer_from_confidential_evaluator::do_apply( const transfer_from_confidential_operation& o )
{ try {
   const auto& confidential_idx = _db.get_index< confidential_balance_index >().indices().get< by_commitment >();
   auto confidential_itr = confidential_idx.begin();

   for( const auto& in : o.inputs )
   {
      confidential_itr = confidential_idx.find( in.commitment );

      FC_ASSERT( confidential_itr != confidential_idx.end(), 
         "Confidential balance not found." );
      FC_ASSERT( confidential_itr->symbol == o.amount.symbol,
         "Confidential balance asset is not the same as payment symbol." );
      FC_ASSERT( confidential_itr->symbol == o.fee.symbol,
         "Confidential Balance must be the same asset as fee asset." );
      FC_ASSERT( confidential_itr->owner == in.owner,
         "Confidential Balance owner is not the same as input balance." );
   }

   _db.adjust_confidential_supply( -( o.amount + o.fee ) );
   _db.pay_network_fees( o.fee );
   _db.adjust_liquid_balance( o.to, o.amount );
   
   for( const auto& in : o.inputs )
   {
      confidential_itr = confidential_idx.find( in.commitment );
      FC_ASSERT( confidential_itr != confidential_idx.end() );
      ilog( "Removed: ${v}",("v",*confidential_itr));
      _db.remove( *confidential_itr );
   }

} FC_CAPTURE_AND_RETHROW( (o) ) }

} } // node::chain