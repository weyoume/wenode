
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
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const asset_object& first_asset = _db.get_asset( o.first_amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.second_amount.symbol );

   FC_ASSERT( first_asset.is_liquid_enabled() &&
      second_asset.is_liquid_enabled(),
      "Cannot make a liquidity pool asset with specifed asset pair." );

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

   asset liquid_a = _db.get_liquid_balance( o.account, amount_a.symbol );
   asset liquid_b = _db.get_liquid_balance( o.account, amount_b.symbol );

   FC_ASSERT( liquid_a.amount >= amount_a.amount && 
      liquid_b.amount >= amount_b.amount, 
      "Insufficient Liquid Balance to create liquidity pool with initial asset amounts." );

   _db.adjust_liquid_balance( o.account, -amount_a );
   _db.adjust_liquid_balance( o.account, -amount_b );

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
   });
      
   const asset_liquidity_pool_object& pool = _db.create< asset_liquidity_pool_object >( [&]( asset_liquidity_pool_object& alpo )
   {   
      alpo.symbol_a = amount_a.symbol;
      alpo.symbol_b = amount_b.symbol;
      alpo.symbol_liquid = liquidity_asset_symbol;
      alpo.balance_a = amount_a;
      alpo.balance_b = amount_b;
      alpo.hour_median_price = price( alpo.balance_a, alpo.balance_b );
      alpo.day_median_price = price( alpo.balance_a, alpo.balance_b );
      alpo.price_history.push_back( price( alpo.balance_a, alpo.balance_b ) );
      alpo.balance_liquid = asset( max, liquidity_asset_symbol );
   });

   _db.adjust_liquid_balance( o.account, asset( max, liquidity_asset_symbol ) );

   ilog( "Account: ${a} Created Liquidity Pool: \n ${p} \n",
      ("a",o.account)("p",pool.to_string()));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void liquidity_pool_exchange_evaluator::do_apply( const liquidity_pool_exchange_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.receive_asset );
   asset liquid = _db.get_liquid_balance( o.account, o.amount.symbol );
   const account_object* int_account_ptr = nullptr;

   FC_ASSERT( first_asset.is_liquid_enabled() &&
      second_asset.is_liquid_enabled(),
      "Cannot Exchange specifed asset pair as assets are not liquid enabled" );

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
   else
   {
      FC_ASSERT( liquid >= o.amount, 
         "Account: ${a} has insufficient liquid balance to exchange amount: ${am}",
         ("a",o.account)("ac",o.amount.to_string()));

      if( o.limit_price.valid() )
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
   }

   ilog( "Account: ${a} exchanged: ${am} for asset: ${s} with Liquidity Pool: \n ${p} \n",
      ("a",o.account)("am",o.amount.to_string())("s",o.receive_asset)("p",liquidity_pool.to_string()));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void liquidity_pool_fund_evaluator::do_apply( const liquidity_pool_fund_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_object& first_asset = _db.get_asset( o.amount.symbol );
   const asset_object& second_asset = _db.get_asset( o.pair_asset );

   FC_ASSERT( first_asset.is_liquid_enabled() &&
      second_asset.is_liquid_enabled(),
      "Cannot Exchange specifed asset pair as assets are not liquid enabled" );

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

   ilog( "Account: ${a} funded: ${am} into Liquidity Pool: \n ${p} \n",
      ("a",o.account)("am",o.amount.to_string())("p",liquidity_pool.to_string()));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void liquidity_pool_withdraw_evaluator::do_apply( const liquidity_pool_withdraw_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( o.amount.symbol );

   _db.liquid_withdraw( o.amount, o.receive_asset, account, liquidity_pool );

   ilog( "Account: ${a} withdrew: ${am} from Liquidity Pool: \n ${p} \n",
      ("a",o.account)("am",o.amount.to_string())("p",liquidity_pool.to_string()));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_collateral_evaluator::do_apply( const credit_pool_collateral_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_object& collateral_asset = _db.get_asset( o.amount.symbol );
   const asset& liquid = _db.get_liquid_balance( o.account, o.amount.symbol );
   asset loan_default_balance = account.loan_default_balance;
   time_point now = _db.head_block_time();

   FC_ASSERT( collateral_asset.is_credit_enabled(),
      "Cannot make a collateral position with credit disabled asset." );

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

      const credit_collateral_object& collateral = _db.create< credit_collateral_object >( [&]( credit_collateral_object& cco )
      {
         cco.owner = account.name;
         cco.symbol = collateral_asset.symbol;
         cco.collateral = delta;
         cco.created = now;
         cco.last_updated = now;
      });

      ilog( "Account: ${a} created new collateral: \n ${p} \n",
         ("a",o.account)("p",collateral));
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

      ilog( "Account: ${a} updated Collateral: \n ${c} \n",
         ("a",o.account)("c",collateral));

      if( collateral.collateral.amount == 0 )
      {
         ilog( "Removed: ${v}",("v",collateral));
         _db.remove( collateral );
      } 
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_borrow_evaluator::do_apply( const credit_pool_borrow_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_object& debt_asset = _db.get_asset( o.amount.symbol );
   const asset_object& collateral_asset = _db.get_asset( o.collateral.symbol );

   FC_ASSERT( debt_asset.is_credit_enabled(),
      "Cannot borrow debt asset: ${s}.",
      ("s",debt_asset.symbol));
   FC_ASSERT( collateral_asset.is_credit_enabled(), 
      "Cannot collateralize asset: ${s}.",
      ("s",collateral_asset.symbol));

   if( o.flash_loan )
   {
      FC_ASSERT( o.collateral.amount == 0,
         "Flash loan is instantaneously repaid within same transaction and does not require collateral." );
   }

   const credit_collateral_object& collateral = _db.get_collateral( o.account, o.collateral.symbol );
   const asset_credit_pool_object& pool = _db.get_credit_pool( o.amount.symbol, false );
   const asset& liquid = _db.get_liquid_balance( o.account, o.amount.symbol );
   const median_chain_property_object& median_props = _db.get_median_chain_properties();
   asset min_collateral = ( o.amount * median_props.credit_open_ratio ) / PERCENT_100;        // Min collateral equal to 125% of debt value
   asset max_debt = ( o.collateral * PERCENT_100 ) / median_props.credit_liquidation_ratio;   // Max debt before liquidation equal to aprox 90% of collateral value. 
   time_point now = _db.head_block_time();
   asset_symbol_type symbol_a;
   asset_symbol_type symbol_b;
   asset_symbol_type symbol_liquid;
   
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
   symbol_liquid = liquidity_pool.symbol_liquid;
   price median_price = liquidity_pool.hour_median_price;  // Get the asset pair's liquidity pool median price for calculations.
   min_collateral = min_collateral * median_price;
   max_debt = max_debt * median_price;
   
   FC_ASSERT( o.collateral.amount >= min_collateral.amount || o.flash_loan,
      "Collateral amount is insufficient: ${a} Required: ${r}.",
      ("a",o.collateral.to_string())("r",min_collateral.to_string()) );
   
   const auto& loan_idx = _db.get_index< credit_loan_index >().indices().get< by_loan_id >();
   auto loan_itr = loan_idx.find( boost::make_tuple( o.account, o.loan_id ) );

   if( loan_itr == loan_idx.end() )    // Credit loan object with this ID does not exist.
   {
      FC_ASSERT( account.loan_default_balance.amount == 0,
         "Account has an outstanding loan default balance. Please expend network credit collateral to recoup losses before opening a new loan." );
      FC_ASSERT( _db.credit_check( o.amount, o.collateral, pool ) || o.flash_loan,
         "New loan with provided collateral and debt is not viable with current asset liquidity conditions. Please lower debt." );
      FC_ASSERT( o.amount.amount != 0,
         "Loan does not exist to close out. Please set non-zero amount." );
      FC_ASSERT( o.collateral.amount != 0 || o.flash_loan,
         "Loan does not exist to close out. Please set non-zero collateral." );
      FC_ASSERT( collateral.collateral >= o.collateral || o.flash_loan,
         "Insufficient collateral balance. Required: ${r} Actual: ${a}",
         ("r",o.collateral.to_string())("a",collateral.collateral.to_string()) );
      FC_ASSERT( pool.base_balance >= o.amount,
         "Insufficient Base Balance to borrow from credit pool. Required: ${r} Actual: ${a}",
         ("r",o.amount.to_string())("a",pool.base_balance.to_string()) );

      const credit_loan_object& loan = _db.create< credit_loan_object >( [&]( credit_loan_object& clo )
      {
         clo.owner = o.account;    // Create new loan object for the account
         from_string( clo.loan_id, o.loan_id );
         clo.debt = o.amount;
         clo.interest = asset( 0, o.amount.symbol );
         clo.collateral = o.collateral;
         clo.liquidation_price = price( o.collateral, max_debt );
         clo.symbol_a = symbol_a;
         clo.symbol_b = symbol_b;
         clo.symbol_liquid = symbol_liquid;
         clo.created = now;
         clo.last_updated = now;
         clo.last_interest_time = now;
         clo.flash_loan = o.flash_loan;
      });

      ilog( "Account: ${a} created credit Loan: \n ${l} \n",
         ("a",o.account)("l",loan));

      if( !o.flash_loan )
      {
         _db.modify( collateral, [&]( credit_collateral_object& cco )
         {
            cco.collateral -= o.collateral;     // Decrement from pledged collateral object balance.
            cco.last_updated = now;
         });
      }
      
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

      FC_ASSERT( delta_collateral.amount != 0 || 
         delta_debt.amount != 0,
         "Operation would not change collateral or debt position in the loan." );
      FC_ASSERT( collateral.collateral.amount >= delta_collateral.amount || o.flash_loan,
         "Insufficient collateral balance in this asset to vest the amount requested in the loan." );
      FC_ASSERT( liquid.amount >= -delta_debt.amount,
         "Insufficient liquid balance in this asset to repay the amount requested." );
      FC_ASSERT( loan.flash_loan == o.flash_loan,
         "Flash Loan status cannot be altered. Loan must be repaid within same Transaction with 1 day of accrued interest." );

      share_type interest_rate = pool.interest_rate( median_props.credit_min_interest, median_props.credit_variable_interest );
      uint128_t interest_seconds = ( now - loan.last_interest_time ).to_seconds();

      if( o.flash_loan )
      {
         interest_seconds = fc::days(1).to_seconds();    // Flash loan pays one day worth of interest
      }

      uint128_t interest_amount = uint128_t( loan.debt.amount.value ) * uint128_t( interest_rate.value ) * uint128_t( interest_seconds );
      interest_amount /= uint128_t( fc::days(365).to_seconds() * PERCENT_100 );

      asset interest_asset = asset( interest_amount.to_uint64(), loan.debt.symbol );      // Accrue interest on debt balance
      asset interest_fees = ( loan.interest * INTEREST_FEE_PERCENT ) / PERCENT_100;
      asset closing_debt = old_debt + interest_asset;
      
      if( o.amount.amount == 0 || o.collateral.amount == 0 )         // Closing out the loan, ensure both amount and collateral are zero if one is zero. 
      {
         FC_ASSERT( o.amount.amount == 0 && 
            o.collateral.amount == 0,
            "Both collateral and amount must be set to zero to close out loan." );

         _db.adjust_liquid_balance( o.account, -closing_debt );       // Return debt to the pending supply of the credit pool.
         _db.adjust_pending_supply( closing_debt );
         _db.pay_network_fees( interest_fees );                       // Pay network fees on interest accrued in loan lifetime.

         if( !o.flash_loan )
         {
            _db.modify( collateral, [&]( credit_collateral_object& cco )
            {
               cco.collateral += old_collateral;        // Return collateral to the account's collateral balance.
               cco.last_updated = now;
            });
         }

         _db.modify( pool, [&]( asset_credit_pool_object& acpo )
         {
            acpo.borrowed_balance -= old_debt;       // Transfer the borrowed balance back to the base balance, less interest fees. 
            acpo.base_balance += ( closing_debt - interest_fees );
         });

         ilog( "Closing Loan: \n ${l} \n Credit Pool: \n ${p} \n",
            ("l",loan)("p",pool.to_string()));

         _db.remove( loan );
      }
      else      // modifying the loan or repaying partially. 
      {
         FC_ASSERT( delta_debt.amount < 0 || 
            account.loan_default_balance.amount == 0,
            "Account has an outstanding loan default balance. Please expend network credits to recoup losses before increasing debt." );
         FC_ASSERT( _db.credit_check( o.amount, o.collateral, pool ) || o.flash_loan,
            "New loan with provided collateral and debt is not viable with current asset liquidity conditions. Please lower debt." );

         asset new_debt = o.amount + interest_asset;

         if( !o.flash_loan )
         {
            _db.modify( collateral, [&]( credit_collateral_object& cco )
            {
               cco.collateral -= delta_collateral;         // Update to new collateral amount.
               cco.last_updated = now;
            });
         }

         _db.modify( pool, [&]( asset_credit_pool_object& acpo )
         {
            acpo.borrowed_balance += delta_debt;       // Update borrowed balance.
            acpo.base_balance -= delta_debt; 
         });

         _db.modify( loan, [&]( credit_loan_object& clo )
         {
            clo.debt = new_debt;                     // Update to new loan parameters.
            clo.collateral = o.collateral;
            clo.liquidation_price = price( o.collateral, max_debt );
            clo.last_updated = now;
            clo.last_interest_time = now;
         });

         _db.adjust_liquid_balance( o.account, delta_debt );        // Shift newly borrowed or repaid amount with pending supply. 
         _db.adjust_pending_supply( -delta_debt );

         ilog( "Account: ${a} updated credit Loan: \n ${l} \n Credit Pool: \n ${p} \n",
            ("a",o.account)("l",loan)("p",pool.to_string()));
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_lend_evaluator::do_apply( const credit_pool_lend_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );

   FC_ASSERT( asset_obj.is_credit_enabled(),
      "Cannot lend asset: ${s}.",
      ("s",asset_obj.symbol) );

   const asset_credit_pool_object& credit_pool = _db.get_credit_pool( asset_obj.symbol, false );

   _db.credit_lend( o.amount, account, credit_pool );

   ilog( "Account: ${a} lent amount: ${am} to credit pool: \n ${l} \n",
      ("a",o.account)("am",o.amount.to_string())("l",credit_pool));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void credit_pool_withdraw_evaluator::do_apply( const credit_pool_withdraw_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_object& asset_obj = _db.get_asset( o.amount.symbol );

   FC_ASSERT( asset_obj.asset_type == asset_property_type::CREDIT_POOL_ASSET,
      "Asset must be a credit pool asset to withdraw from it's credit pool." );

   const asset_credit_pool_object& credit_pool = _db.get_credit_pool( asset_obj.symbol, true );

   _db.credit_withdraw( o.amount, account, credit_pool );

   ilog( "Account: ${a} withdrew amount: ${am} from credit pool: \n ${l} \n",
      ("a",o.account)("am",o.amount.to_string())("l",credit_pool));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void option_pool_create_evaluator::do_apply( const option_pool_create_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const asset_object& first_asset = _db.get_asset( o.first_asset );
   const asset_object& second_asset = _db.get_asset( o.second_asset );

   time_point now = _db.head_block_time();
   date_type today = date_type( now );

   FC_ASSERT( first_asset.is_option_enabled() &&
      second_asset.is_option_enabled(),
      "Cannot make an option pool using this asset pair." );

   asset_symbol_type base_symbol;
   asset_symbol_type quote_symbol;

   if( first_asset.id < second_asset.id )
   {
      base_symbol = o.first_asset;
      quote_symbol = o.second_asset;
   }
   else
   {
      quote_symbol = o.first_asset;
      base_symbol = o.second_asset;
   }

   const asset_object& base_asset = _db.get_asset( base_symbol );
   const asset_object& quote_asset = _db.get_asset( quote_symbol );

   const auto& pool_idx = _db.get_index< asset_option_pool_index >().indices().get< by_asset_pair >();
   auto pool_itr = pool_idx.find( boost::make_tuple( base_symbol, quote_symbol ) );

   FC_ASSERT( pool_itr == pool_idx.end(), 
      "Asset option pool pair already exists for this asset pair. Use the Option Order operation to issue option assets." );

   const asset_liquidity_pool_object& liquidity_pool = _db.get_liquidity_pool( base_symbol, quote_symbol );
   price current_price = liquidity_pool.base_day_median_price( base_symbol );
   flat_set< asset_symbol_type > new_strikes;
   flat_set< date_type > new_dates;
   date_type next_date = today;

   for( int i = 0; i < 12; i++ )      // compile the next 12 months of expiration dates
   {
      if( next_date.month != 12 )
      {
         next_date = date_type( 1, next_date.month + 1, next_date.year );
      }
      else
      {
         next_date = date_type( 1, 1, next_date.year + 1 );
      }
      new_dates.insert( next_date );
   }
   
   const asset_option_pool_object& pool = _db.create< asset_option_pool_object >( [&]( asset_option_pool_object& aopo )
   {   
      aopo.base_symbol = base_symbol;
      aopo.quote_symbol = quote_symbol;
      new_strikes = aopo.add_strike_prices( current_price, new_dates );
   });

   for( asset_symbol_type s : new_strikes )     // Create the new asset objects for the option.
   {
      _db.create< asset_object >( [&]( asset_object& a )
      {
         option_strike strike = option_strike::from_string(s);

         a.symbol = s;
         a.asset_type = asset_property_type::OPTION_ASSET;
         a.issuer = NULL_ACCOUNT;
         from_string( a.display_symbol, strike.display_symbol() );

         from_string( 
            a.details, 
            strike.details( 
               to_string( quote_asset.display_symbol ), 
               to_string( quote_asset.details ), 
               to_string( base_asset.display_symbol ), 
               to_string( base_asset.details ) ) );
         
         from_string( a.json, "" );
         from_string( a.url, "" );
         a.max_supply = MAX_ASSET_SUPPLY;
         a.stake_intervals = 0;
         a.unstake_intervals = 0;
         a.market_fee_percent = 0;
         a.market_fee_share_percent = 0;
         a.issuer_permissions = 0;
         a.flags = 0;
         a.created = now;
         a.last_updated = now;
      });

      _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
      {
         a.symbol = s;
      });
   }

   ilog( "Account: ${a} created new option pool: \n ${p} \n",
      ("a",o.account)("p",pool));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void prediction_pool_create_evaluator::do_apply( const prediction_pool_create_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );

   const asset_object& collateral_asset = _db.get_asset( o.collateral_symbol );
   time_point now = _db.head_block_time();
   
   FC_ASSERT( collateral_asset.is_credit_enabled(), 
      "Cannot make a prediction pool using the collateral asset: ${s}.",
      ("s",o.collateral_symbol) );

   const asset_object* asset_ptr = _db.find_asset( o.prediction_symbol );
   FC_ASSERT( asset_ptr == nullptr, 
      "Asset already exists with the specified prediction symbol: ${s}.",
      ("s",o.prediction_symbol) );

   asset liquid = _db.get_liquid_balance( o.account, o.prediction_bond.symbol );

   FC_ASSERT( liquid >= o.prediction_bond, 
      "Insufficient liquid balance for prediction bond." );
   FC_ASSERT( o.outcome_time >= now + fc::days(7),
      "Outcome time must be at least 7 days in the future." );

   const auto& pool_idx = _db.get_index< asset_prediction_pool_index >().indices().get< by_prediction_symbol >();
   auto pool_itr = pool_idx.find( o.prediction_symbol );

   FC_ASSERT( pool_itr == pool_idx.end(), 
      "As pool pair already exists for this asset pair. Use the Option Order operation to issue option assets." );

   // Add Invalid outcome asset 

   vector< asset_symbol_type > outcome_assets;
   
   for( asset_symbol_type a : o.outcome_assets )
   {
      asset_symbol_type outcome_symbol = string( o.prediction_symbol )+"."+string( a );
      outcome_assets.push_back( outcome_symbol );
   }

   asset_symbol_type invalid_symbol = string( o.prediction_symbol )+"."+INVALID_OUTCOME_SYMBOL;
   outcome_assets.push_back( invalid_symbol );
   
   _db.create< asset_object >( [&]( asset_object& a )
   {
      a.symbol = o.prediction_symbol;
      a.asset_type = asset_property_type::PREDICTION_ASSET;
      a.issuer = o.account;
      from_string( a.display_symbol, o.display_symbol );
      from_string( a.details, o.details );
      from_string( a.json, o.json );
      from_string( a.url, o.url );
      a.max_supply = MAX_ASSET_SUPPLY;
      a.stake_intervals = 0;
      a.unstake_intervals = 0;
      a.market_fee_percent = 0;
      a.market_fee_share_percent = 0;
      a.issuer_permissions = 0;
      a.flags = 0;
      a.created = now;
      a.last_updated = now;
   });

   _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
   {
      a.symbol = o.prediction_symbol;
   });

   _db.adjust_liquid_balance( o.account, -o.prediction_bond );
   _db.adjust_liquid_balance( o.account, asset( o.prediction_bond.amount, o.prediction_symbol ) );
   _db.adjust_pending_supply( o.prediction_bond );

   _db.create< asset_prediction_pool_object >( [&]( asset_prediction_pool_object& appo )
   {   
      appo.prediction_symbol = o.prediction_symbol;
      appo.collateral_symbol = o.collateral_symbol;
      appo.collateral_pool = asset( 0, o.collateral_symbol );

      for( auto out : outcome_assets )
      {
         appo.outcome_assets.insert( out );
      }

      from_string( appo.outcome_details, o.outcome_details );
      from_string( appo.json, o.json );
      from_string( appo.url, o.url );
      from_string( appo.details, o.details );
      appo.outcome_time = o.outcome_time;
      appo.resolution_time = o.outcome_time + fc::days(7);
      appo.prediction_bond_pool = o.prediction_bond;
   });

   for( size_t i = 0; i < outcome_assets.size(); i++ )
   {
      asset_symbol_type s = outcome_assets[ i ];

      _db.create< asset_object >( [&]( asset_object& a )
      {
         a.symbol = s;
         a.asset_type = asset_property_type::OPTION_ASSET;
         a.issuer = o.account;
         from_string( a.display_symbol, o.display_symbol );
         from_string( a.details, o.outcome_details );
         from_string( a.json, o.json );
         from_string( a.url, o.url );
         a.max_supply = MAX_ASSET_SUPPLY;
         a.stake_intervals = 0;
         a.unstake_intervals = 0;
         a.market_fee_percent = 0;
         a.market_fee_share_percent = 0;
         a.issuer_permissions = 0;
         a.flags = 0;
         a.created = now;
         a.last_updated = now;
      });

      _db.create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
      {
         a.symbol = s;
      });

      ilog( "Created Prediction pool outcome asset: ${p}",("p",s ));
   }

   ilog( "Created Prediction pool: ${p}",("p",o.prediction_symbol ));
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void prediction_pool_exchange_evaluator::do_apply( const prediction_pool_exchange_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_object& prediction_asset = _db.get_asset( o.prediction_asset );
   const asset_object& collateral_asset = _db.get_asset( o.amount.symbol );
   const asset_prediction_pool_object& prediction_pool = _db.get_prediction_pool( o.prediction_asset );

   time_point now = _db.head_block_time();
   FC_ASSERT( now <= prediction_pool.outcome_time,
      "Cannot exchange with a prediction market pool after the outcome time. Please await market resolution." );

   if( o.exchange_base )
   {
      asset base_amount = asset( o.amount.amount, prediction_asset.symbol );
      asset liquid_collateral = _db.get_liquid_balance( o.account, collateral_asset.symbol );
      asset liquid_base = _db.get_liquid_balance( o.account, o.prediction_asset );
      
      if( o.withdraw )
      {
         FC_ASSERT( liquid_base >= base_amount, 
            "Account: ${a} does not have enough liquid balance to exchange requested amount: ${i}.",
            ("a", account.name)("i", base_amount.to_string()) );

         _db.adjust_liquid_balance( o.account, o.amount );
         _db.adjust_liquid_balance( o.account, -base_amount );
         _db.adjust_pending_supply( -o.amount );

         _db.modify( prediction_pool, [&]( asset_prediction_pool_object& appo )
         {
            appo.adjust_prediction_bond_pool( -o.amount );
         });
      }
      else
      {
         FC_ASSERT( liquid_collateral >= o.amount, 
            "Account: ${a} does not have enough liquid balance to exchange requested amount: ${i}.",
            ("a", account.name)("i", o.amount.to_string()) );

         _db.adjust_liquid_balance( o.account, -o.amount );
         _db.adjust_liquid_balance( o.account, base_amount );
         _db.adjust_pending_supply( o.amount );

         _db.modify( prediction_pool, [&]( asset_prediction_pool_object& appo )
         {
            appo.adjust_prediction_bond_pool( o.amount );
         });
      }
   }
   else
   {
      asset liquid_collateral = _db.get_liquid_balance( o.account, o.amount.symbol );

      if( o.withdraw )
      {
         asset outcome_amount;
         asset liquid_outcome;

         for( asset_symbol_type outcome : prediction_pool.outcome_assets )
         {
            liquid_outcome = _db.get_liquid_balance( o.account, outcome );
            FC_ASSERT( liquid_outcome.amount >= o.amount.amount,
               "Account: ${a} does not have enough liquid balance to exchange requested amount: ${i}.",
               ("a", account.name)("i", o.amount.to_string()) );
         }

         for( asset_symbol_type outcome : prediction_pool.outcome_assets )
         {
            outcome_amount = asset( o.amount.amount, outcome );
            _db.adjust_liquid_balance( o.account, -outcome_amount );
         }

         _db.adjust_liquid_balance( o.account, o.amount );
         _db.adjust_pending_supply( -o.amount );

         _db.modify( prediction_pool, [&]( asset_prediction_pool_object& appo )
         {
            appo.adjust_collateral_pool( -o.amount );
         });
      }
      else
      {
         FC_ASSERT( liquid_collateral >= o.amount,
            "Account: ${a} does not have enough liquid balance to exchange requested amount: ${i}.",
            ("a", account.name)("i", o.amount.to_string()) );
         
         _db.adjust_liquid_balance( o.account, -o.amount );
         _db.adjust_pending_supply( o.amount );

         asset outcome_amount;

         for( asset_symbol_type outcome : prediction_pool.outcome_assets )
         {
            outcome_amount = asset( o.amount.amount, outcome );
            _db.adjust_liquid_balance( o.account, outcome_amount );
         }

         _db.modify( prediction_pool, [&]( asset_prediction_pool_object& appo )
         {
            appo.adjust_collateral_pool( o.amount );
         });
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void prediction_pool_resolve_evaluator::do_apply( const prediction_pool_resolve_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account) );
   const asset_object& prediction_asset = _db.get_asset( o.amount.symbol );
   const asset_object& outcome_asset = _db.get_asset( o.resolution_outcome );
   const asset_prediction_pool_object& prediction_pool = _db.get_prediction_pool( o.amount.symbol );
   asset liquid = _db.get_liquid_balance( o.account, o.amount.symbol );
   time_point now = _db.head_block_time();

   FC_ASSERT( prediction_pool.is_outcome( outcome_asset.symbol ),
      "Resolution outcome must be a valid outcome within the prediction market." );
   FC_ASSERT( now >= prediction_pool.outcome_time,
      "Cannot Resolve market before outcome time." );
   
   const auto& resolution_idx = _db.get_index< asset_prediction_pool_resolution_index >().indices().get< by_account >();
   auto resolution_itr = resolution_idx.find( boost::make_tuple( o.account, prediction_asset.symbol ) );

   if( resolution_itr == resolution_idx.end() )
   {
      FC_ASSERT( liquid >= o.amount,
      "Account: ${a} does not have enough liquid balance to exchange requested amount: ${i}.",
      ("a", account.name)("i", o.amount.to_string()) );

      _db.adjust_liquid_balance( o.account, -o.amount );
      _db.adjust_pending_supply( o.amount );

      const asset_prediction_pool_resolution_object& resolution = _db.create< asset_prediction_pool_resolution_object >( [&]( asset_prediction_pool_resolution_object& a )
      {
         a.account = o.account;
         a.resolution_outcome = o.resolution_outcome;
         a.prediction_symbol = o.amount.symbol;
         a.amount = o.amount;
      });

      ilog( "Account: ${a} created new prediction resolution: \n ${r} \n",
         ("a",o.account)("r",resolution));
   }
   else
   {
      const asset_prediction_pool_resolution_object& resolution = *resolution_itr;

      asset delta_amount = o.amount - resolution.amount;

      FC_ASSERT( delta_amount.amount > 0,
         "Must only increase amount after resolving prediction outcome." );
      FC_ASSERT( o.resolution_outcome == resolution.resolution_outcome,
         "Cannot change outcome selection resolving prediction outcome." );
      FC_ASSERT( liquid >= delta_amount,
         "Account: ${a} does not have enough liquid balance to exchange requested amount: ${i}.",
         ("a", account.name)("i", delta_amount.to_string() ) );

      _db.adjust_liquid_balance( o.account, -delta_amount );
      _db.adjust_pending_supply( delta_amount );

      _db.modify( resolution, [&]( asset_prediction_pool_resolution_object& appro )
      {
         appro.amount = o.amount;
      });

      ilog( "Account: ${a} updated prediction resolution: \n ${r} \n",
         ("a",o.account)("r",resolution));
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain