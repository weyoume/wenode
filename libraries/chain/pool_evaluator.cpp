
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


//=========================//
// === Pool Evaluators === //
//=========================//


void liquidity_pool_create_evaluator::do_apply( const liquidity_pool_create_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const asset_object& first_asset = _db.get_asset( o.first_amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.second_amount.symbol );

   FC_ASSERT( first_asset.asset_type != asset_property_type::LIQUIDITY_POOL_ASSET &&
      second_asset.asset_type != asset_property_type::LIQUIDITY_POOL_ASSET,
      "Cannot make a liquidity pool asset with a liquidity pool asset as a component." );

   asset amount_a;
   asset amount_b;

   if( first_asset.id < second_asset.id )
   {
      amount_a = o.first_amount;
      amount_b = o.second_amount;
   }
   else
   {
      amount_b = o.first_amount;
      amount_a = o.second_amount;
   }

   asset_symbol_type liquidity_asset_symbol = LIQUIDITY_ASSET_PREFIX+string( amount_a.symbol )+"."+string( amount_b.symbol );

   share_type max = std::max( amount_a.amount, amount_b.amount );
   const auto& liq_idx = _db.get_index< asset_liquidity_pool_index >().indices().get< by_asset_pair >();
   auto liq_itr = liq_idx.find( boost::make_tuple( amount_a.symbol, amount_b.symbol ) ); 

   FC_ASSERT( liq_itr == liq_idx.end(), 
      "Asset liquidity pair already exists for asset pair. Please use an exchange or fund operation to trade with it." );
   
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.issuer = o.account;
      a.symbol = liquidity_asset_symbol;
      a.asset_type = asset_property_type::LIQUIDITY_POOL_ASSET;
   });

   _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a ) 
   {
      a.symbol = liquidity_asset_symbol;
      a.total_supply = 0;
      a.liquid_supply = 0;
      a.staked_supply = 0;
      a.reward_supply = 0;
      a.savings_supply = 0;
      a.delegated_supply = 0;
      a.receiving_supply = 0;
      a.pending_supply = 0;
      a.confidential_supply = 0;
   });
      
   _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& alpo )
   {   
      alpo.issuer = o.account;
      alpo.balance_a = amount_a;
      alpo.balance_b = amount_b;
      alpo.symbol_a = amount_a.symbol;
      alpo.symbol_b = amount_b.symbol;
      alpo.hour_median_price = price( alpo.balance_a, alpo.balance_b );
      alpo.day_median_price = price( alpo.balance_a, alpo.balance_b );
      alpo.price_history.push_back( price( alpo.balance_a, alpo.balance_b ) );
      alpo.symbol_liquid = liquidity_asset_symbol;
      alpo.balance_liquid = max;
   });

   _db.adjust_liquid_balance( o.account, asset( max, liquidity_asset_symbol ) );
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void liquidity_pool_exchange_evaluator::do_apply( const liquidity_pool_exchange_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const account_object& account = _db.get_account( o.account );
   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.receive_asset );
   const account_object* int_account_ptr = nullptr;

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }
   
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( first_asset.id < second_asset.id )
   {
      symbol_a = first_asset.symbol;
      symbol_b = second_asset.symbol;
   }
   else
   {
      symbol_b = first_asset.symbol;
      symbol_a = second_asset.symbol;
   }

   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );

   if( o.acquire )
   {
      if( int_account_ptr != nullptr )
      {
         _db.liquid_acquire( o.amount, account, liquidity_pool, *int_account_ptr );
      }
      else
      {
         _db.liquid_acquire( o.amount, account, liquidity_pool );
      }
   }
   else if( o.limit_price.valid() )
   {
      price limit_price = *o.limit_price;
      FC_ASSERT( limit_price < liquidity_pool.base_price( limit_price.base.symbol ), 
         "Limit price must be lower than current liquidity pool exchange price.");
      
      if( int_account_ptr != nullptr )
      {
         _db.liquid_limit_exchange( o.amount, *o.limit_price, account, liquidity_pool, *int_account_ptr );
      }
      else
      {
         _db.liquid_limit_exchange( o.amount, *o.limit_price, account, liquidity_pool );
      }
   }
   else
   {
      if( int_account_ptr != nullptr )
      {
         _db.liquid_exchange( o.amount, account, liquidity_pool, *int_account_ptr );
      }
      else
      {
         _db.liquid_exchange( o.amount, account, liquidity_pool );
      }
   }
 
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void liquidity_pool_fund_evaluator::do_apply( const liquidity_pool_fund_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const account_object& account = _db.get_account( o.account );
   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.pair_asset );

   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( first_asset.id < second_asset.id )
   {
      symbol_a = first_asset.symbol;
      symbol_b = second_asset.symbol;
   }
   else
   {
      symbol_b = first_asset.symbol;
      symbol_a = second_asset.symbol;
   }

   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );

   _db.liquid_fund( o.amount, account, liquidity_pool );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void liquidity_pool_withdraw_evaluator::do_apply( const liquidity_pool_withdraw_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const account_object& account = _db.get_account( o.account );
   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.receive_asset );

   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;

   if( first_asset.id < second_asset.id )
   {
      symbol_a = first_asset.symbol;
      symbol_b = second_asset.symbol;
   }
   else
   {
      symbol_b = first_asset.symbol;
      symbol_a = second_asset.symbol;
   }

   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );

   _db.liquid_withdraw( o.amount, o.receive_asset, account, liquidity_pool );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_collateral_evaluator::do_apply( const credit_pool_collateral_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const account_object& account = _db.get_account( o.account );
   const asset_object& collateral_asset = _db.get_asset( o.amount.symbol );
   const asset& liquid = _db.get_liquid_balance( o.account, o.amount.symbol );
   asset loan_default_balance = account.loan_default_balance;
   time_point now = _db.head_block_time();

   const auto& col_idx = _db.get_index< credit_collateral_index >().indices().get< by_owner_symbol >();
   auto col_itr = col_idx.find( boost::make_tuple( account.name, o.amount.symbol ) );
   asset default_paid = asset( 0, o.amount.symbol );

   if( col_itr == col_idx.end() )    // Collateral object does not exist.
   {
      asset delta = o.amount;
      if ( loan_default_balance.amount > 0 && 
         o.amount.symbol == SYMBOL_CREDIT )
      {
         default_paid = asset( std::min( delta.amount, loan_default_balance.amount ), delta.symbol );
         delta -= default_paid;

         _db.modify( account, [&]( account_object& a )
         {
            a.loan_default_balance -= default_paid;
         });
      }

      FC_ASSERT( liquid >= o.amount,
         "Insufficient liquid balance to collateralize the amount requested." );

      _db.adjust_liquid_balance( o.account, -o.amount );
      _db.adjust_pending_supply( o.amount );

      _db.create< credit_collateral_object >( [&]( credit_collateral_object& cco )
      {
         cco.owner = account.name;
         cco.symbol = collateral_asset.symbol;
         cco.collateral = delta;
         cco.created = now;
         cco.last_updated = now;
      });
   }
   else    // Collateral position exists and is being updated.
   {
      const credit_collateral_object& collateral = *col_itr;
      asset old_collateral = collateral.collateral;
      asset delta = o.amount - old_collateral; 

      if( loan_default_balance.amount > 0 && 
         delta.amount > 0 && 
         o.amount.symbol == SYMBOL_CREDIT )   // If asset is credit and a loan default is owed, pays off default debt. 
      {
         default_paid = asset( std::min( delta.amount, loan_default_balance.amount ), delta.symbol ); 

         _db.modify( account, [&]( account_object& a )
         {
            a.loan_default_balance -= default_paid;
         });
      }

      FC_ASSERT( delta.amount != 0,
         "Operation would not change collateral position in this asset." );
      FC_ASSERT( liquid >= delta,
         "Insufficient liquid balance to increase collateral position to the amount requested." );

      _db.adjust_liquid_balance( o.account, -delta );
      _db.adjust_pending_supply( delta );

      _db.modify( collateral, [&]( credit_collateral_object& cco )
      {
         cco.collateral = o.amount - default_paid;
         cco.last_updated = now;
      });

      if( collateral.collateral.amount == 0 )
      {
         _db.remove( collateral );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_borrow_evaluator::do_apply( const credit_pool_borrow_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const account_object& account = _db.get_account( o.account );
   const asset_object& debt_asset = _db.get_asset( o.amount.symbol );
   const asset_object& collateral_asset = _db.get_asset( o.collateral.symbol );
   const credit_collateral_object& collateral = _db.get_collateral( o.account, o.collateral.symbol );
   const asset_credit_pool_object& pool = _db.get_credit_pool( o.amount.symbol, false );
   const asset& liquid = _db.get_liquid_balance( o.account, o.amount.symbol );
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   asset min_collateral = ( o.amount * median_props.credit_open_ratio ) / PERCENT_100;        // Min collateral equal to 125% of debt value
   asset max_debt = ( o.collateral * PERCENT_100 ) / median_props.credit_liquidation_ratio;   // Max debt before liquidation equal to aprox 90% of collateral value. 
   time_point now = _db.head_block_time();
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

      const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( symbol_a, symbol_b );
      price median_price = liquidity_pool.hour_median_price;  // Get the asset pair's liquidity pool median price for calculations.
      min_collateral = min_collateral * median_price;
      max_debt = max_debt * median_price;
   }

   FC_ASSERT( o.collateral.amount >= min_collateral.amount,
      "Collateral is insufficient to support a loan of this size." );
   FC_ASSERT( debt_asset.asset_type != asset_property_type::CREDIT_POOL_ASSET && debt_asset.asset_type != asset_property_type::LIQUIDITY_POOL_ASSET, 
      "Cannot borrow assets issued by liquidity or credit pools." );
   FC_ASSERT( collateral_asset.asset_type != asset_property_type::CREDIT_POOL_ASSET && collateral_asset.asset_type != asset_property_type::LIQUIDITY_POOL_ASSET, 
      "Cannot collateralize assets issued by liquidity or credit pools." );

   const auto& loan_idx = _db.get_index< credit_loan_index >().indices().get< by_loan_id >();
   auto loan_itr = loan_idx.find( boost::make_tuple( account.name, o.loan_id ) ); 

   if( loan_itr == loan_idx.end() )    // Credit loan object with this ID does not exist.
   {
      FC_ASSERT( account.loan_default_balance.amount == 0,
         "Account has an outstanding loan default balance. Please expend network credit collateral to recoup losses before opening a new loan." );
      FC_ASSERT( _db.credit_check( o.amount, o.collateral, pool ),
         "New loan with provided collateral and debt is not viable with current asset liquidity conditions. Please lower debt." );
      FC_ASSERT( o.amount.amount != 0 && o.collateral.amount != 0,
         "Loan does not exist to close out. Please set non-zero amount and collateral." );
      FC_ASSERT( collateral.collateral.amount >= o.collateral.amount,
         "Insufficient collateral balance in this asset to vest the amount requested in the loan. Please increase collateral." );
      FC_ASSERT( pool.base_balance.amount >= o.amount.amount,
         "Insufficient Available asset to borrow from credit pool. Please lower debt." );

      _db.create< credit_loan_object >( [&]( credit_loan_object& clo )
      {
         clo.owner = account.name;    // Create new loan object for the account
         clo.debt = o.amount;
         clo.interest = asset( 0, o.amount.symbol );
         clo.collateral = o.collateral;
         if( o.amount.symbol != o.collateral.symbol )
         {
            clo.loan_price = price( o.collateral, o.amount);
            clo.liquidation_price = price( o.collateral, max_debt );
         }
         clo.created = now;
         clo.last_updated = now;
         clo.last_interest_time = now;
      });   

      _db.modify( collateral, [&]( credit_collateral_object& cco )
      {
         cco.collateral -= o.collateral;     // Decrement from pledged collateral object balance.
         cco.last_updated = now;
      });
      
      _db.modify( pool, [&]( asset_credit_pool_object& acpo )
      {
         acpo.borrowed_balance += o.amount;    // Transfer borrowed amount from the base balance to the borrowed balance of the credit pool. 
         acpo.base_balance -= o.amount;
      });

      _db.adjust_pending_supply( -o.amount );
      _db.adjust_liquid_balance( o.account, o.amount );     // Withdraw loaned balance from pending supply within credit pool base balance. 
   }
   else    // Loan object exists and is being updated or closed out. 
   {
      const credit_loan_object& loan = *loan_itr;
      asset old_collateral = loan.collateral;
      asset delta_collateral = o.collateral - old_collateral;
      asset old_debt = loan.debt;
      asset delta_debt = o.amount - old_debt;

      FC_ASSERT( delta_collateral.amount != 0 || delta_debt.amount != 0,
         "Operation would not change collateral or debt position in the loan." );
      FC_ASSERT( collateral.collateral.amount >= delta_collateral.amount,
         "Insufficient collateral balance in this asset to vest the amount requested in the loan." );
      FC_ASSERT( liquid.amount >= -delta_debt.amount,
         "Insufficient liquid balance in this asset to repay the amount requested." );

      share_type interest_rate = pool.interest_rate( median_props.credit_min_interest, median_props.credit_variable_interest );     // Calulate pool's interest rate
      share_type interest_accrued = ( loan.debt.amount * interest_rate * ( now - loan.last_interest_time ).count() ) / ( PERCENT_100 * fc::days(365).count() );
      asset interest_asset = asset( interest_accrued, loan.debt.symbol );      // Accrue interest on debt balance

      if( o.amount.amount == 0 || o.collateral.amount == 0 )   // Closing out the loan, ensure both amount and collateral are zero if one is zero. 
      {
         FC_ASSERT( o.amount.amount == 0 && 
            o.collateral.amount == 0,
            "Both collateral and amount must be set to zero to close out loan." );
         asset closing_debt = old_debt + interest_asset;

         _db.adjust_liquid_balance( o.account, -closing_debt );    // Return debt to the pending supply of the credit pool.
         _db.adjust_pending_supply( closing_debt );

         _db.modify( collateral, [&]( credit_collateral_object& cco )
         {
            cco.collateral += old_collateral;     // Return collateral to the account's collateral balance.
            cco.last_updated = now;
         });

         _db.modify( pool, [&]( asset_credit_pool_object& acpo )
         {
            acpo.borrowed_balance -= old_debt;    // Transfer the borrowed balance back to the base balance, with interest. 
            acpo.base_balance += closing_debt;
         });

         _db.remove( loan );
      }
      else      // modifying the loan or repaying partially. 
      {
         FC_ASSERT( delta_debt.amount < 0 || account.loan_default_balance.amount == 0,
            "Account has an outstanding loan default balance. Please expend network credits to recoup losses before increasing debt." );
         FC_ASSERT( _db.credit_check( o.amount, o.collateral, pool ),
            "New loan with provided collateral and debt is not viable with current asset liquidity conditions. Please lower debt." );

         asset new_debt = o.amount + interest_asset;

         _db.modify( collateral, [&]( credit_collateral_object& cco )
         {
            cco.collateral -= delta_collateral;    // Update to new collateral amount
            cco.last_updated = now;
         });

         _db.modify( pool, [&]( asset_credit_pool_object& acpo )
         {
            acpo.borrowed_balance += delta_debt;  // Update borrowed balance
            acpo.base_balance -= delta_debt; 
         });

         _db.modify( loan, [&]( credit_loan_object& clo )
         {
            clo.debt = new_debt;    // Update to new loan parameters
            clo.collateral = o.collateral;
            if( loan.debt.symbol != loan.collateral.symbol )
            {
               clo.loan_price = price( o.collateral, o.amount);
               clo.liquidation_price = price( o.collateral, max_debt );
            }
            clo.last_updated = now;
            clo.last_interest_time = now;
         });

         _db.adjust_liquid_balance( o.account, delta_debt );    // Shift newly borrowed or repaid amount with pending supply. 
         _db.adjust_pending_supply( -delta_debt );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_lend_evaluator::do_apply( const credit_pool_lend_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const account_object& account = _db.get_account( o.account );
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );

   FC_ASSERT( asset_obj.asset_type != asset_property_type::CREDIT_POOL_ASSET,
      "Cannot lend a Credit pool asset, please use withdraw operation to access underlying reserves." );
   FC_ASSERT( asset_obj.asset_type != asset_property_type::LIQUIDITY_POOL_ASSET,
      "Cannot lend a Liquidity pool asset, please use withdraw operation to access underlying reserves." );

   const asset_credit_pool_object& credit_pool = _db.get_credit_pool( asset_obj.symbol, false );

   _db.credit_lend( o.amount, account, credit_pool );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_withdraw_evaluator::do_apply( const credit_pool_withdraw_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
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

   const account_object& account = _db.get_account( o.account );
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );

   FC_ASSERT( asset_obj.asset_type == asset_property_type::CREDIT_POOL_ASSET,
      "Asset must be a credit pool asset to withdraw from it's credit pool." );

   const asset_credit_pool_object& credit_pool = _db.get_credit_pool( asset_obj.symbol, true );

   _db.credit_withdraw( o.amount, account, credit_pool );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain