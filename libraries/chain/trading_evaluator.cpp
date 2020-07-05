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


//============================//
// === Trading Evaluators === //
//============================//


void limit_order_evaluator::do_apply( const limit_order_operation& o )
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
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   asset liquid = _db.get_liquid_balance( o.owner, o.amount_to_sell.symbol );
   
   FC_ASSERT( o.expiration > now,
      "Limit order has to expire after head block time." );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   const auto& limit_idx = _db.get_index< limit_order_index >().indices().get< by_account >();
   auto limit_itr = limit_idx.find( std::make_tuple( o.owner, o.order_id ) );

   if( limit_itr == limit_idx.end() )    // Limit order does not already exist
   {
      FC_ASSERT( o.opened,
         "Limit order cannot be closed: No Limit order found with the specified ID: ${id}",
         ("id",o.order_id) );
      FC_ASSERT( liquid >= o.amount_to_sell,
         "Account: ${a} does not have sufficient liquid balance: ${l} for Limit order of amount: ${am}.",
         ("a",o.owner)("l", liquid)("am",o.amount_to_sell));

      _db.adjust_liquid_balance( o.owner, -o.amount_to_sell );
      _db.adjust_pending_supply( o.amount_to_sell );

      const limit_order_object& order = _db.create< limit_order_object >( [&]( limit_order_object& obj )
      {
         obj.seller = o.owner;
         from_string( obj.order_id, o.order_id );
         obj.for_sale = o.amount_to_sell.amount;
         obj.sell_price = o.exchange_rate;
         obj.expiration = o.expiration;
         if( o.interface.size() )
         {
            obj.interface = o.interface;
         }
         obj.created = now;
         obj.last_updated = now;
      });

      bool filled = _db.apply_order( order );

      if( o.fill_or_kill ) 
      {
         FC_ASSERT( filled, 
            "Cancelling order because it was not filled." );
      }
   }
   else
   {
      const limit_order_object& order = *limit_itr;

      if( o.opened )
      {
         asset delta = asset( order.for_sale, order.sell_price.base.symbol ) - o.amount_to_sell;

         FC_ASSERT( liquid >= -delta,
            "Account does not have sufficient funds for limit order." );

         _db.adjust_liquid_balance( o.owner, delta );
         _db.adjust_pending_supply( -delta );

         _db.modify( order, [&]( limit_order_object& obj )
         {
            obj.for_sale = o.amount_to_sell.amount;
            obj.sell_price = o.exchange_rate;
            obj.expiration = o.expiration;
            obj.last_updated = now;
         });

         bool filled = _db.apply_order( order );

         if( o.fill_or_kill ) 
         {
            FC_ASSERT( filled, 
               "Cancelling order because it was not filled." );
         }
      }
      else
      {
         _db.cancel_limit_order( order );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void margin_order_evaluator::do_apply( const margin_order_operation& o )
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
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& owner = _db.get_account( o.owner );
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   time_point now = _db.head_block_time();
   FC_ASSERT( o.expiration > now,
      "Margin order has to expire after head block time." );
   FC_ASSERT( o.exchange_rate.base.symbol == o.amount_to_borrow.symbol,
      "Margin order exchange rate should have debt asset as base of price." );
   FC_ASSERT( owner.loan_default_balance.amount == 0,
      "Account has an outstanding loan default balance. Please expend network credit collateral to recoup losses before opening a new loan." );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   const asset_object& debt_asset = _db.get_asset( o.amount_to_borrow.symbol );
   const asset_object& collateral_asset = _db.get_asset( o.collateral.symbol );
   const asset_object& position_asset = _db.get_asset( o.exchange_rate.quote.symbol );
   const credit_collateral_object& collateral = _db.get_collateral( o.owner, o.collateral.symbol );

   FC_ASSERT( debt_asset.is_credit_enabled(),
      "Cannot borrow debt asset: ${s}.",
      ("s", debt_asset.symbol) );
   FC_ASSERT( collateral_asset.is_credit_enabled(),
      "Cannot collateralize asset: ${s}.",
      ("s", collateral_asset.symbol) );
   FC_ASSERT( position_asset.is_credit_enabled(),
      "Cannot open margin position with asset: ${s}.",
      ("s", position_asset.symbol) );

   const asset_credit_pool_object& pool = _db.get_credit_pool( o.amount_to_borrow.symbol, false );
   asset min_collateral = ( o.amount_to_borrow * median_props.margin_open_ratio ) / PERCENT_100;        // Min margin collateral equal to 20% of debt value.
   share_type collateralization;
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( collateral_asset.symbol != debt_asset.symbol )
   { 
      if( debt_asset.id < collateral_asset.id )
      {
         symbol_a = debt_asset.symbol;
         symbol_b = collateral_asset.symbol;
      }
      else
      {
         symbol_b = debt_asset.symbol;
         symbol_a = collateral_asset.symbol;
      }

      const asset_liquidity_pool_object& debt_collateral_liq_pool = _db.get_liquidity_pool( symbol_a, symbol_b );    
      price median_price = debt_collateral_liq_pool.hour_median_price;     // Get the asset pair's liquidity pool median price for calculations.
      min_collateral = min_collateral * median_price;
      collateralization = ( o.collateral * median_price ).amount * PERCENT_100 / o.amount_to_borrow.amount;
   }
   else
   {
      collateralization = ( o.collateral.amount * PERCENT_100 ) / o.amount_to_borrow.amount;
   }
   
   asset position = o.amount_to_borrow * o.exchange_rate;

   FC_ASSERT( o.collateral.amount >= min_collateral.amount,
      "Collateral is insufficient to support a margin loan of this size. Please increase collateral." );
   FC_ASSERT( _db.margin_check( o.amount_to_borrow, position, o.collateral, pool ),
      "Margin loan with provided collateral, position, and debt is not viable with current asset liquidity conditions. Please lower debt." );

   const auto& margin_idx = _db.get_index< margin_order_index >().indices().get< by_account >();
   auto margin_itr = margin_idx.find( std::make_tuple( o.owner, o.order_id ) );

   if( margin_itr == margin_idx.end() )    // margin order does not already exist
   {
      FC_ASSERT( o.opened,
         "Margin order cannot be closed: No Margin order found with the specified ID." );
      FC_ASSERT( collateral.collateral.amount >= o.collateral.amount, 
         "Insufficient collateral balance in this asset to vest the amount requested in the loan. Please increase collateral.");
      FC_ASSERT( pool.base_balance.amount >= o.amount_to_borrow.amount,
         "Insufficient Available assets to borrow from credit pool. Please lower debt." );
      

      _db.modify( collateral, [&]( credit_collateral_object& cco )
      {
         cco.collateral -= o.collateral;   // Decrement from pledged collateral object balance.
         cco.last_updated = now;
      });

      // Transfer borrowed amount from the base balance to the borrowed balance of the credit pool.
      _db.modify( pool, [&]( asset_credit_pool_object& acpo )
      {
         acpo.borrowed_balance += o.amount_to_borrow;
         acpo.base_balance -= o.amount_to_borrow;
      });

      const margin_order_object& order = _db.create< margin_order_object >( [&]( margin_order_object& moo )
      {
         moo.owner = o.owner;
         from_string( moo.order_id, o.order_id );
         moo.sell_price = o.exchange_rate;
         moo.collateral = o.collateral;
         moo.debt = o.amount_to_borrow;
         moo.debt_balance = o.amount_to_borrow;
         moo.interest = asset( 0, o.amount_to_borrow.symbol );
         moo.position = o.amount_to_borrow * o.exchange_rate;
         moo.position_balance = asset( 0, o.exchange_rate.quote.symbol );
         moo.collateralization = collateralization;
         if( o.interface.size() )
         {
            moo.interface = o.interface;
         }
         moo.expiration = o.expiration;
         if( o.stop_loss_price.valid() )
         {
            moo.stop_loss_price = *o.stop_loss_price;
         }
         if( o.take_profit_price.valid() )
         {
            moo.take_profit_price = *o.take_profit_price;
         }
         if( o.limit_stop_loss_price.valid() )
         {
            moo.limit_stop_loss_price = *o.limit_stop_loss_price;
         }
         if( o.limit_take_profit_price.valid() )
         {
            moo.limit_take_profit_price = *o.limit_take_profit_price;
         }
         moo.liquidating = false;
         moo.created = now;
         moo.last_updated = now;
         moo.last_interest_time = now;
         moo.unrealized_value = asset( 0, debt_asset.symbol );
      });

      bool filled = _db.apply_order( order );

      if( o.fill_or_kill )
      {
         FC_ASSERT( filled,
            "Cancelling order because it was not filled." );
      }
   }
   else      // Editing or closing existing order
   {
      const margin_order_object& order = *margin_itr;

      FC_ASSERT( o.exchange_rate.base.symbol == order.sell_price.base.symbol &&
         o.exchange_rate.quote.symbol == order.sell_price.quote.symbol,
         "Margin exchange rate must maintain the same asset pairing as existing order." );

      if( o.opened )    // Editing margin order
      {
         asset delta_collateral = o.collateral - order.collateral;
         asset delta_borrowed = o.amount_to_borrow - order.debt;

         FC_ASSERT( collateral.collateral >= delta_collateral,
            "Insufficient collateral balance in this asset to vest the amount requested in the loan. Please increase collateral.");
         FC_ASSERT( pool.base_balance >= delta_borrowed,
            "Insufficient Available assets to borrow from credit pool. Please lower debt." );
         FC_ASSERT( order.debt_balance >= -delta_borrowed,
            "Insufficient Margin debt balance to process debt reduction." );
         
         _db.modify( collateral, [&]( credit_collateral_object& cco )
         {
            cco.collateral -= delta_collateral;   // Decrement from pledged collateral object balance.
            cco.last_updated = now;
         });

         _db.modify( pool, [&]( asset_credit_pool_object& acpo )
         {
            acpo.borrowed_balance += delta_borrowed;
            acpo.base_balance -= delta_borrowed;
         });

         _db.modify( order, [&]( margin_order_object& moo )
         {
            moo.sell_price = o.exchange_rate;
            moo.collateral = o.collateral;
            moo.debt = o.amount_to_borrow;
            moo.debt_balance += delta_borrowed;
            moo.position = o.amount_to_borrow * o.exchange_rate;
            moo.collateralization = collateralization;
            moo.expiration = o.expiration;

            if( o.stop_loss_price.valid() )
            {
               moo.stop_loss_price = *o.stop_loss_price;
            }
            if( o.take_profit_price.valid() )
            {
               moo.take_profit_price = *o.take_profit_price;
            }
            if( o.limit_stop_loss_price.valid() )
            {
               moo.limit_stop_loss_price = *o.limit_stop_loss_price;
            }
            if( o.limit_take_profit_price.valid() )
            {
               moo.limit_take_profit_price = *o.limit_take_profit_price;
            }
            moo.last_updated = now;
         });

         bool filled = _db.apply_order( order );

         if( o.fill_or_kill )
         {
            FC_ASSERT( filled,
               "Cancelling order because it was not filled." );
         }
      }
      else     // closing margin order
      {
         if( o.force_close )
         {
            _db.close_margin_order( order );   // Liquidate the remaining order into the liquidity pool at current price and close loan.
         }
         else
         {
            price new_sell_price = ~o.exchange_rate;    // Reverse the exchange rate
            
            _db.modify( order, [&]( margin_order_object& moo )
            {
               moo.liquidating = true;
               moo.sell_price = new_sell_price;   // Invert price and insert into the book as sell position order. 
               moo.last_updated = now;
            });

            bool filled = _db.apply_order( order );

            if( o.fill_or_kill ) 
            {
               FC_ASSERT( filled,
                  "Cancelling close order because it was not filled." );
            }
         }
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void auction_order_evaluator::do_apply( const auction_order_operation& o )
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
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();

   FC_ASSERT( o.expiration > now,
      "Auction order has to expire after head block time." );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   asset liquid = _db.get_liquid_balance( o.owner, o.amount_to_sell.symbol );
   const auto& auction_idx = _db.get_index< auction_order_index >().indices().get< by_account >();
   auto auction_itr = auction_idx.find( std::make_tuple( o.owner, o.order_id ) );

   if( auction_itr == auction_idx.end() )
   {
      FC_ASSERT( o.opened,
         "Auction order cannot be closed: No Auction order found with the specified ID." );
      FC_ASSERT( liquid >= o.amount_to_sell,
         "Account: ${a} does not have sufficient liquid balance: ${l} for Auction order of amount: ${am}.",
         ("a",o.owner)("l", liquid)("am",o.amount_to_sell));

      _db.adjust_liquid_balance( o.owner, -o.amount_to_sell );
      _db.adjust_pending_supply( o.amount_to_sell );

      _db.create< auction_order_object >( [&]( auction_order_object& aoo )
      {
         aoo.owner = o.owner;
         from_string( aoo.order_id, o.order_id );
         aoo.amount_to_sell = o.amount_to_sell;
         aoo.limit_close_price = o.limit_close_price;
         if( o.interface.size() )
         {
            aoo.interface = o.interface;
         }
         aoo.expiration = o.expiration;
         aoo.created = now;
         aoo.last_updated = now;
      });
   }
   else
   {
      const auction_order_object& order = *auction_itr;

      if( o.opened )
      {
         asset delta = order.amount_to_sell - o.amount_to_sell;

         FC_ASSERT( liquid >= -delta,
            "Account does not have sufficient funds for Auction order." );

         _db.adjust_liquid_balance( o.owner, delta );
         _db.adjust_pending_supply( -delta );

         _db.modify( order, [&]( auction_order_object& aoo )
         {
            aoo.amount_to_sell = o.amount_to_sell;
            aoo.limit_close_price = o.limit_close_price;
            aoo.expiration = o.expiration;
            aoo.last_updated = now;
         });
      }
      else
      {
         _db.close_auction_order( order );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void call_order_evaluator::do_apply( const call_order_operation& o )
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
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   time_point now = _db.head_block_time();
   const asset_object& debt_asset = _db.get_asset( o.debt.symbol );
   const asset_dynamic_data_object& debt_dynamic_data = _db.get_dynamic_data( o.debt.symbol );
   const asset_stablecoin_data_object& debt_stablecoin_data = _db.get_stablecoin_data( o.debt.symbol );

   ilog( "Account: ${a} creating call order for stablecoin: ${d}",("a",o.owner)("d",debt_stablecoin_data));

   FC_ASSERT( debt_asset.is_market_issued(),
      "Asset ${sym} is not a collateralized asset.", ("sym", debt_asset.symbol) );
   FC_ASSERT( !debt_stablecoin_data.has_settlement(),
      "Cannot update debt position when the asset has been globally settled." );
   FC_ASSERT( o.collateral.symbol == debt_stablecoin_data.backing_asset,
      "Collateral asset type should be same as backing asset of debt asset." );
   FC_ASSERT( !debt_stablecoin_data.current_feed.settlement_price.is_null(),
      "Cannot borrow asset with no price feed." );

   const auto& call_idx = _db.get_index< call_order_index >().indices().get< by_account >();
   auto call_itr = call_idx.find( std::make_tuple( o.owner, o.debt.symbol ) );

   optional< price > old_collateralization;
   optional< share_type > old_debt;

   asset liquid_collateral = _db.get_liquid_balance( o.owner, o.collateral.symbol );
   asset liquid_debt = _db.get_liquid_balance( o.owner, o.debt.symbol );

   if( call_itr == call_idx.end() )    // creating new debt position
   {
      ilog( "Account: ${a} begin creating new call order -",("a",o.owner));

      FC_ASSERT( debt_dynamic_data.get_total_supply().amount + o.debt.amount <= debt_asset.max_supply,
         "Borrowing this quantity would exceed the asset's Maximum supply." );
      FC_ASSERT( debt_dynamic_data.get_total_supply().amount + o.debt.amount >= 0,
         "This transaction would bring current supply below zero." );
      FC_ASSERT( liquid_collateral >= o.collateral,
         "Account does not have sufficient liquid collateral asset funds for Call order." );

      ilog( "Call Order Details: \n ${de} \n ${ts} \n ${as} \n ${lc}",
         ("de",debt_dynamic_data)("ts", debt_dynamic_data.get_total_supply() )("as",debt_asset)("lc",liquid_collateral) );

      _db.adjust_liquid_balance( o.owner, -o.collateral );
      _db.adjust_pending_supply( o.collateral );
      _db.adjust_liquid_balance( o.owner, o.debt );
      
      const call_order_object& call = _db.create< call_order_object >( [&]( call_order_object& coo )
      {
         coo.borrower = o.owner;
         coo.collateral = o.collateral;
         coo.debt = o.debt;
         coo.call_price = price( asset( 1, o.collateral.symbol ), asset( 1, o.debt.symbol ) );
         coo.target_collateral_ratio = *o.target_collateral_ratio;
         coo.created = now;
         coo.last_updated = now;
      });

      ilog( "Account: ${a} created new call order: ${c}",
         ("a",o.owner)("c",call));
   }
   else        // updating existing debt position
   {
      const call_order_object& call = *call_itr;
      ilog( "Account: ${a} begin editing call order: ${c}",("a",o.owner)("c",call));

      asset delta_collateral = o.collateral - call.collateral;
      asset delta_debt = o.debt - call.debt;

      FC_ASSERT( liquid_collateral >= delta_collateral,
         "Account does not have sufficient liquid collateral asset funds for Call order." );
      FC_ASSERT( liquid_debt >= -delta_debt,
         "Account does not have sufficient liquid debt asset funds for Call order." );

      _db.adjust_liquid_balance( o.owner, -delta_collateral );
      _db.adjust_pending_supply( delta_collateral );
      _db.adjust_liquid_balance( o.owner, delta_debt );
      
      if( o.debt.amount != 0 )
      {
         FC_ASSERT( o.collateral.amount > 0 && o.debt.amount > 0,
            "Both collateral and debt should be positive after updated a debt position if not to close it" );

         old_collateralization = call.collateralization();
         old_debt = call.debt.amount;

         _db.modify( call, [&]( call_order_object& coo )
         {
            coo.collateral = o.collateral;
            coo.debt = o.debt;
            coo.target_collateral_ratio = *o.target_collateral_ratio;
            coo.last_updated = now;
         });

         ilog( "Account: ${a} updated call order: ${c}",("a",o.owner)("c",call));
      }
      else
      {
         FC_ASSERT( o.collateral.amount == 0,
            "Should claim all collateral when closing debt position" );
         ilog( "Removed: ${v}",("v",call));
         _db.remove( call );
      } 
   }

   if( _db.check_call_orders( debt_asset, false, false ) )      // Don't allow black swan, not for new limit order
   {
      const call_order_object* call_obj_ptr = _db.find_call_order( o.owner, o.debt.symbol );
      FC_ASSERT( !call_obj_ptr,
         "Updating call order would trigger a margin call that cannot be fully filled" );
   }
   else
   {
      const call_order_object* call_obj_ptr = _db.find_call_order( o.owner, o.debt.symbol );  // No black swan event has occurred
      FC_ASSERT( call_obj_ptr != nullptr,
         "No margin call was executed and yet the call object was deleted" );

      const call_order_object& call_object = *call_obj_ptr;
   
      FC_ASSERT( ( call_object.collateralization() > debt_stablecoin_data.current_maintenance_collateralization ) || 
         ( old_collateralization.valid() &&
         call_object.debt.amount <= *old_debt && 
         call_object.collateralization() > *old_collateralization ),
         "Can only increase collateral ratio without increasing debt if would trigger a margin call that cannot be fully filled",
         ("old_debt", *old_debt)
         ("new_debt", call_object.debt)
         ("old_collateralization", *old_collateralization)
         ("new_collateralization", call_object.collateralization() )
         );
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void option_order_evaluator::do_apply( const option_order_operation& o )
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
      FC_ASSERT( b.is_authorized_transfer( o.signatory, _db.get_account_permissions( signed_for ) ),
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   FC_ASSERT( o.options_issued.amount % BLOCKCHAIN_PRECISION == 0, 
      "Option assets can only be issued in units of 1." );

   const asset_object& option_asset = _db.get_asset( o.options_issued.symbol );

   FC_ASSERT( option_asset.asset_type == asset_property_type::OPTION_ASSET, 
      "Option asset with symbol: ${s} is not valid.",("s", o.options_issued.symbol ) );
   
   option_strike strike = option_strike::from_string( o.options_issued.symbol );

   time_point now = _db.head_block_time();

   FC_ASSERT( strike.expiration() > now,
      "Option order has to expire after head block time." );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   const auto& option_idx = _db.get_index< option_order_index >().indices().get< by_account >();
   auto option_itr = option_idx.find( std::make_tuple( o.owner, o.order_id ) );
   
   const asset_option_pool_object& option_pool = _db.get_option_pool( strike.strike_price.base.symbol, strike.strike_price.quote.symbol );

   FC_ASSERT( std::find( option_pool.call_strikes.begin(), option_pool.call_strikes.end(), strike ) != option_pool.call_strikes.end() ||
      std::find( option_pool.put_strikes.begin(), option_pool.put_strikes.end(), strike ) != option_pool.put_strikes.end(),
      "Option pool chain sheet does not support the specified option stike ${s}.", ("s", o.options_issued.symbol ) );

   asset underlying_amount;
   asset exercise_amount;

   if( strike.call )       // Call option underlying is quote asset to be bought with base asset.
   {
      underlying_amount = asset( o.options_issued.amount * strike.multiple, strike.strike_price.quote.symbol );
      exercise_amount = underlying_amount * strike.strike_price;
   }
   else                    // Put option underlying is base asset to buy the quote asset.
   {
      exercise_amount = asset( o.options_issued.amount * strike.multiple, strike.strike_price.quote.symbol );
      underlying_amount = exercise_amount * strike.strike_price;
   }
   
   asset liquid_underlying = _db.get_liquid_balance( o.owner, underlying_amount.symbol );
   asset liquid_position = _db.get_liquid_balance( o.owner, o.options_issued.symbol );

   if( option_itr == option_idx.end() )
   {
      FC_ASSERT( o.opened,
         "Option order cannot be closed: No Option order found with the specified ID." );
      FC_ASSERT( liquid_underlying >= underlying_amount,
         "Account does not have sufficient liquid underlying asset funds for Option order." );

      _db.adjust_liquid_balance( o.owner, -underlying_amount );
      _db.adjust_pending_supply( underlying_amount );
      _db.adjust_liquid_balance( o.owner, o.options_issued );

      _db.create< option_order_object >( [&]( option_order_object& ooo )
      {
         ooo.owner = o.owner;
         from_string( ooo.order_id, o.order_id );
         ooo.underlying_amount = underlying_amount;
         ooo.exercise_amount = exercise_amount;
         ooo.option_position = o.options_issued;
         ooo.strike_price = strike;
         if( o.interface.size() )
         {
            ooo.interface = o.interface;
         }
         ooo.created = now;
         ooo.last_updated = now;
      });
   }
   else
   {
      const option_order_object& order = *option_itr;

      asset delta_underlying = underlying_amount - order.underlying_amount;
      asset delta_position = o.options_issued - order.option_position;

      FC_ASSERT( liquid_underlying >= delta_underlying,
         "Account does not have sufficient liquid underlying asset funds for Option order." );
      FC_ASSERT( liquid_position >= -delta_position,
         "Account does not have sufficient liquid option asset funds for Option order." );

      _db.adjust_liquid_balance( o.owner, -delta_underlying );
      _db.adjust_pending_supply( delta_underlying );
      _db.adjust_liquid_balance( o.owner, delta_position );

      if( o.options_issued.amount != 0 )
      {
         _db.modify( order, [&]( option_order_object& ooo )
         {
            ooo.underlying_amount = underlying_amount;
            ooo.exercise_amount = exercise_amount;
            ooo.option_position = o.options_issued;
            ooo.last_updated = now;
         });
      }
      else
      {
         ilog( "Removed: ${v}",("v",order));
         _db.remove( order );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain