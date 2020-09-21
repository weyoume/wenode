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


const asset_liquidity_pool_object& database::get_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const
{ try {
   return get< asset_liquidity_pool_object, by_asset_pair >( boost::make_tuple( symbol_a, symbol_b ) );
} FC_CAPTURE_AND_RETHROW( (symbol_a)(symbol_b) ) }

const asset_liquidity_pool_object* database::find_liquidity_pool( const asset_symbol_type& symbol_a, const asset_symbol_type& symbol_b )const
{
   return find< asset_liquidity_pool_object, by_asset_pair >( boost::make_tuple( symbol_a, symbol_b ) );
}

const asset_liquidity_pool_object& database::get_liquidity_pool( const asset_symbol_type& symbol )const
{ try {
   return get< asset_liquidity_pool_object, by_symbol_liquid >( symbol );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_liquidity_pool_object* database::find_liquidity_pool( const asset_symbol_type& symbol )const
{
   return find< asset_liquidity_pool_object, by_symbol_liquid >( symbol );
}

const asset_credit_pool_object& database::get_credit_pool( const asset_symbol_type& symbol, bool credit_asset )const
{ try {
   if( credit_asset )
   {
      return get< asset_credit_pool_object, by_credit_symbol >( symbol );
   }
   else
   {
      return get< asset_credit_pool_object, by_base_symbol >( symbol );
   }
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_credit_pool_object* database::find_credit_pool( const asset_symbol_type& symbol, bool credit_asset )const
{
   if( credit_asset )
   {
      return find< asset_credit_pool_object, by_credit_symbol >( symbol );
   }
   else
   {
      return find< asset_credit_pool_object, by_base_symbol >( symbol );
   }
}

const credit_collateral_object& database::get_collateral( const account_name_type& owner, const asset_symbol_type& symbol  )const
{ try {
   return get< credit_collateral_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
} FC_CAPTURE_AND_RETHROW( (owner)(symbol) ) }

const credit_collateral_object* database::find_collateral( const account_name_type& owner, const asset_symbol_type& symbol )const
{
   return find< credit_collateral_object, by_owner_symbol >( boost::make_tuple( owner, symbol ) );
}

const credit_loan_object& database::get_loan( const account_name_type& owner, const shared_string& loan_id  )const
{ try {
   return get< credit_loan_object, by_loan_id >( boost::make_tuple( owner, loan_id ) );
} FC_CAPTURE_AND_RETHROW( (owner)(loan_id) ) }

const credit_loan_object* database::find_loan( const account_name_type& owner, const shared_string& loan_id )const
{
   return find< credit_loan_object, by_loan_id >( boost::make_tuple( owner, loan_id ) );
}

const credit_loan_object& database::get_loan( const account_name_type& owner, const string& loan_id  )const
{ try {
   return get< credit_loan_object, by_loan_id >( boost::make_tuple( owner, loan_id ) );
} FC_CAPTURE_AND_RETHROW( (owner)(loan_id) ) }

const credit_loan_object* database::find_loan( const account_name_type& owner, const string& loan_id )const
{
   return find< credit_loan_object, by_loan_id >( boost::make_tuple( owner, loan_id ) );
}

const asset_option_pool_object& database::get_option_pool( const asset_symbol_type& base_symbol, const asset_symbol_type& quote_symbol )const
{ try {
   return get< asset_option_pool_object, by_asset_pair >( boost::make_tuple( base_symbol, quote_symbol ) );
} FC_CAPTURE_AND_RETHROW( (base_symbol)(quote_symbol) ) }

const asset_option_pool_object* database::find_option_pool( const asset_symbol_type& base_symbol, const asset_symbol_type& quote_symbol )const
{
   return find< asset_option_pool_object, by_asset_pair >( boost::make_tuple( base_symbol, quote_symbol ) );
}

const asset_option_pool_object& database::get_option_pool( const asset_symbol_type& symbol )const
{ try {
   return get< asset_option_pool_object, by_asset_pair >( boost::make_tuple( SYMBOL_COIN, symbol ) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_option_pool_object* database::find_option_pool( const asset_symbol_type& symbol )const
{
   return find< asset_option_pool_object, by_asset_pair >( boost::make_tuple( SYMBOL_COIN, symbol ) );
}

const asset_prediction_pool_object& database::get_prediction_pool( const asset_symbol_type& symbol )const
{ try {
   return get< asset_prediction_pool_object, by_prediction_symbol >( symbol );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_prediction_pool_object* database::find_prediction_pool( const asset_symbol_type& symbol )const
{
   return find< asset_prediction_pool_object, by_prediction_symbol >( symbol );
}

const asset_prediction_pool_resolution_object& database::get_prediction_pool_resolution( const account_name_type& name, const asset_symbol_type& symbol )const
{ try {
   return get< asset_prediction_pool_resolution_object, by_account >( boost::make_tuple( name, symbol ) );
} FC_CAPTURE_AND_RETHROW( (symbol) ) }

const asset_prediction_pool_resolution_object* database::find_prediction_pool_resolution( const account_name_type& name, const asset_symbol_type& symbol )const
{
   return find< asset_prediction_pool_resolution_object, by_account >( boost::make_tuple( name, symbol ) );
}

/**
 * Adds an asset into a liquidity pool
 * and receives the pool's liquidity pool asset,
 * which earns a portion of fees from trading through the pool.
 */
void database::liquid_fund( const asset& input, const account_object& account, const asset_liquidity_pool_object& pool )
{ try {
   asset liquid = get_liquid_balance( account.name, input.symbol );
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( liquid >= input, 
      "Account: ${a} does not have enough liquid balance to fund requested amount: ${i}.",
      ("a", account.name)("i", input.to_string()) );
   FC_ASSERT( liquid.symbol == pool.symbol_a || liquid.symbol == pool.symbol_b, 
      "Invalid symbol input to liquidity pool: ${s}.",("s", input.symbol) );
   

   uint128_t pr = BLOCKCHAIN_PRECISION.value;
   uint128_t pr_sq = pr * pr;
   uint128_t sup = pool.balance_liquid.amount.value;
   uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
   uint128_t in = input.amount.value;

   uint128_t return_amount = ( sup * ( uint128_t( approx_sqrt( pr_sq + ( ( pr_sq * in ) / ib ) ) ) - pr  ) ) / pr;
   share_type ra = int64_t( return_amount.to_uint64() );
   asset return_asset = asset( ra, pool.symbol_liquid );
   
   adjust_liquid_balance( account.name, -input );

   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( input.symbol == p.symbol_a )
      {
         p.balance_a += input;
      }
      else if( input.symbol == p.symbol_b )
      {
         p.balance_b += input;
      }
      p.balance_liquid += return_asset;
   });

   adjust_liquid_balance( account.name, return_asset );

   ilog( "Funded liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input.to_string())("o",return_asset.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }


/**
 * Withdraws a pool's liquidity asset for some of its underlying assets,
 * lowering the total supply of the pool's liquidity asset
 */
void database::liquid_withdraw( const asset& input, const asset_symbol_type& receive, 
   const account_object& account, const asset_liquidity_pool_object& pool )
{ try {
   asset liquid = get_liquid_balance( account.name, input.symbol );
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( liquid >= input, 
      "Account: ${a} does not have enough liquid balance to withdraw requested amount: ${i}.",
      ("a",account.name)("i",input.to_string()) );

   uint128_t pr = BLOCKCHAIN_PRECISION.value;
   uint128_t pr_sq = pr * pr;
   uint128_t sup = pool.balance_liquid.amount.value;
   uint128_t rb = pool.asset_balance( receive ).amount.value;
   uint128_t in = input.amount.value;

   uint128_t var = pr - ( ( in * pr ) / sup );
   uint128_t return_amount = ( rb * ( pr_sq - ( var * var ) ) ) / pr_sq;

   share_type ra = int64_t( return_amount.to_uint64() );
   asset return_asset = asset( ra, receive );
   
   adjust_liquid_balance( account.name, -input );

   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      p.balance_liquid -= input;
      if( receive == p.symbol_a )
      {
         p.balance_a -= return_asset;
      }
      else if( receive == p.symbol_b )
      {
         p.balance_b -= return_asset ;
      }
   });

   adjust_liquid_balance( account.name, return_asset );

   ilog( "Withdraw liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input.to_string())("o",return_asset.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }

/**
 * Exchanges an asset for any other asset in the network
 * by using the core asset as a liquidity pathway.
 */
asset database::liquid_exchange( const asset& input, const asset_symbol_type& receive, 
   bool execute = true, bool apply_fees = true )
{ try {
   FC_ASSERT( input.symbol != receive,
      "Assets must have different symbols to exchange.");
   FC_ASSERT( input.amount > 0 );

   asset coin_input;

   if( input.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& input_pool = get_liquidity_pool( SYMBOL_COIN, input.symbol );

      uint128_t pr = BLOCKCHAIN_PRECISION.value;
      uint128_t ib = input_pool.asset_balance( input.symbol ).amount.value;
      uint128_t rb = input_pool.asset_balance( SYMBOL_COIN ).amount.value;
      uint128_t in = input.amount.value;

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;

      share_type ra = int64_t( return_amount.to_uint64() );
      asset total_fees = asset( ( ra * TRADING_FEE_PERCENT ) / PERCENT_100,  SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;
   
      asset return_asset = asset( int64_t( return_amount.to_uint64() ), SYMBOL_COIN );

      if( apply_fees )
      {
         return_asset -= total_fees;
      }
      
      if( execute )
      {
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
         modify( input_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a -= return_asset;
            p.balance_b += input ; 
         });

         ilog( "Exchanged liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
            ("i",input.to_string())("o",return_asset.to_string())("p",input_pool.to_string()));
      }

      coin_input = return_asset;
   }
   else
   {
      coin_input = input;
   }

   if( receive != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& receive_pool = get_liquidity_pool( SYMBOL_COIN, receive );

      asset total_fees = asset( ( coin_input.amount.value * TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;

      if( apply_fees )
      {
         coin_input -= total_fees;
      }

      uint128_t in = coin_input.amount.value;
      uint128_t pr = BLOCKCHAIN_PRECISION.value;
      uint128_t ib = receive_pool.asset_balance( SYMBOL_COIN ).amount.value;
      uint128_t rb = receive_pool.asset_balance( receive ).amount.value;

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;

      share_type ra = int64_t( return_amount.to_uint64() );
      asset return_asset = asset( ra, receive );
      
      if( execute )
      {
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
         modify( receive_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a += coin_input;
            p.balance_b -= return_asset ; 
         });

         ilog( "Exchanged liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
            ("i",coin_input.to_string())("o",return_asset.to_string())("p",receive_pool.to_string()));
      }
      return return_asset;
   }
   else
   {
      return coin_input;
   }

   
} FC_CAPTURE_AND_RETHROW( (input)(receive) ) }


void database::liquid_exchange( const asset& input, const account_object& account, 
   const asset_liquidity_pool_object& pool, const account_object& int_account )
{ try {
   asset liquid = get_liquid_balance( account.name, input.symbol );
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( liquid >= input, 
      "Account: ${a} does not have enough liquid balance to fund requested amount: ${i}.",
      ("a", account.name)("i", input.to_string()) );

   asset total_fees;
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   uint128_t pr = BLOCKCHAIN_PRECISION.value;
   uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
   uint128_t rb = pool.asset_balance( rec ).amount.value;
   uint128_t in = input.amount.value;

   if( input.symbol == SYMBOL_COIN )
   {
      total_fees = asset( ( input.amount.value * TRADING_FEE_PERCENT ) / PERCENT_100,  SYMBOL_COIN );
      in -= total_fees.amount.value;
   }

   uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;

   share_type ra = int64_t( return_amount.to_uint64() );

   if( input.symbol != SYMBOL_COIN )
   {
      total_fees = asset( ( ra * TRADING_FEE_PERCENT ) / PERCENT_100, rec );
      ra -= total_fees.amount;
   }
   
   asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset pool_fees = total_fees - network_fees - interface_fees;
   
   asset return_asset = asset( ra, rec );
   
   adjust_liquid_balance( account.name, -input );
   pay_network_fees( account, network_fees );
   pay_fee_share( int_account, interface_fees, true );

   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( input.symbol == p.symbol_a )
      {
         p.balance_a += input; 
         p.balance_b -= return_asset;
      }
      else if( input.symbol == p.symbol_b )
      {
         p.balance_b += input; 
         p.balance_a -= return_asset;
      }
      if( pool_fees.symbol == p.symbol_a )
      {
         p.balance_a += pool_fees;
      }
      else if( pool_fees.symbol == p.symbol_b )
      {
         p.balance_b += pool_fees;
      }
   });

   adjust_liquid_balance( account.name, return_asset );

   ilog( "Exchanged liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input.to_string())("o",return_asset.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool)(int_account) ) }


void database::liquid_exchange( const asset& input, const account_object& account, 
   const asset_liquidity_pool_object& pool )
{ try {
   asset liquid = get_liquid_balance( account.name, input.symbol );
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( liquid >= input, 
      "Account: ${a} does not have enough liquid balance to fund requested amount: ${i}.",
      ("a", account.name)("i", input.to_string()) );

   asset total_fees;
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   uint128_t pr = BLOCKCHAIN_PRECISION.value;
   uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
   uint128_t rb = pool.asset_balance( rec ).amount.value;
   uint128_t in = input.amount.value;

   if( input.symbol == SYMBOL_COIN )
   {
      total_fees = asset( ( input.amount.value * TRADING_FEE_PERCENT ) / PERCENT_100,  SYMBOL_COIN );
      in -= total_fees.amount.value;
   }

   uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;

   share_type ra = int64_t( return_amount.to_uint64() );

   if( input.symbol != SYMBOL_COIN )
   {
      total_fees = asset( ( ra * TRADING_FEE_PERCENT ) / PERCENT_100,  rec );
      ra -= total_fees.amount;
   }
   
   asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset pool_fees = total_fees - network_fees - interface_fees;
   
   asset return_asset = asset( ra, rec );
   
   adjust_liquid_balance( account.name, -input );
   pay_network_fees( account, network_fees + interface_fees );

   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( input.symbol == p.symbol_a )
      {
         p.balance_a += input; 
         p.balance_b -= return_asset;
      }
      else if( input.symbol == p.symbol_b )
      {
         p.balance_b += input; 
         p.balance_a -= return_asset;
      }
      if( pool_fees.symbol == p.symbol_a )
      {
         p.balance_a += pool_fees;
      }
      else if( pool_fees.symbol == p.symbol_b )
      {
         p.balance_b += pool_fees;
      }
   });

   adjust_liquid_balance( account.name, return_asset );

   ilog( "Exchanged liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input.to_string())("o",return_asset.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }


asset database::liquid_acquire( const asset& receive, const asset_symbol_type& input, 
   bool execute = true, bool apply_fees = true )
{ try {
   FC_ASSERT( receive.amount > 0 );
   FC_ASSERT( receive.symbol != input,
      "Assets must have different symbols to acquire." );

   asset coin_asset = asset( 0, SYMBOL_COIN );

   if( receive.symbol != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& acquire_pool = get_liquidity_pool( SYMBOL_COIN, receive.symbol );

      uint128_t pr = BLOCKCHAIN_PRECISION.value;
      uint128_t pr_sq = pr * pr;
      uint128_t ib = acquire_pool.asset_balance( SYMBOL_COIN ).amount.value;
      uint128_t rb = acquire_pool.asset_balance( receive.symbol ).amount.value;
      uint128_t re = receive.amount.value;

      uint128_t input_coin = ( ( ( pr_sq * ib ) / ( pr - ( ( pr * re ) / rb ) ) ) - ( pr * ib ) ) / pr;

      share_type ic = int64_t( input_coin.to_uint64() );

      asset total_fees = asset( ( ic * TRADING_FEE_PERCENT ) / PERCENT_100,  SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;
      
      coin_asset = asset( ic, SYMBOL_COIN );
      
      if( execute )
      {
         modify( acquire_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a += coin_asset;
            p.balance_b -= receive; 
         });
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
      }

      if( apply_fees )
      {
         coin_asset += total_fees;
      }
      if( execute )
      {
         ilog( "Acquired liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
            ("i",coin_asset.to_string())("o",receive.to_string())("p",acquire_pool.to_string()));
      }
   }
   else
   {
      coin_asset = receive;
   }

   if( input != SYMBOL_COIN )
   {
      const asset_liquidity_pool_object& receive_pool = get_liquidity_pool( SYMBOL_COIN, input );

      asset total_fees = asset( ( coin_asset.amount.value * TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset network_fees = asset( ( total_fees.amount * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100, SYMBOL_COIN );
      asset pool_fees = total_fees - network_fees;

      if( apply_fees )
      {
         coin_asset += total_fees;
      }

      uint128_t in = coin_asset.amount.value;
      uint128_t pr = BLOCKCHAIN_PRECISION.value;
      uint128_t ib = receive_pool.asset_balance( input ).amount.value;
      uint128_t rb = receive_pool.asset_balance( SYMBOL_COIN ).amount.value;

      uint128_t input_amount = ( rb * ( pr - ( ( pr * ib ) / ( in + ib ) ) ) ) / pr;

      share_type ia = int64_t( input_amount.to_uint64() );
      
      asset input_asset = asset( ia, input );
      
      if( execute )
      {
         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }
         modify( receive_pool, [&]( asset_liquidity_pool_object& p )
         {
            if( apply_fees )
            {
               p.balance_a += pool_fees; 
            }
            p.balance_a -= coin_asset;
            p.balance_b += input_asset; 
         });
         ilog( "Acquired liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
            ("i",input_asset.to_string())("o",coin_asset.to_string())("p",receive_pool.to_string()));
      }
      return input_asset;
   }
   else
   {
      return coin_asset;
   }

} FC_CAPTURE_AND_RETHROW( (receive)(input) ) }


void database::liquid_acquire( const asset& receive, const account_object& account, 
   const asset_liquidity_pool_object& pool, const account_object& int_account )
{ try {
   FC_ASSERT( receive.amount > 0 );
   FC_ASSERT( receive.symbol == pool.symbol_a || receive.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset total_fees;
   asset_symbol_type in = pool.base_price( receive.symbol ).quote.symbol;
   uint128_t pr = BLOCKCHAIN_PRECISION.value;
   uint128_t pr_sq = pr * pr;
   uint128_t ib = pool.asset_balance( in ).amount.value;
   uint128_t rb = pool.asset_balance( receive.symbol ).amount.value;
   uint128_t re = receive.amount.value;

   if( receive.symbol == SYMBOL_COIN )
   {
      total_fees = ( receive * TRADING_FEE_PERCENT ) / PERCENT_100;
      re += total_fees.amount.value;
   }

   uint128_t input_amount = ( ( ( pr_sq * ib ) / ( pr - ( ( pr * re ) / rb ) ) ) - ( pr * ib ) ) / pr;
   share_type ia = int64_t( input_amount.to_uint64() );
   asset input_asset = asset( ia, in );
   
   if( receive.symbol != SYMBOL_COIN )
   {
      total_fees = ( input_asset * TRADING_FEE_PERCENT ) / PERCENT_100;
      input_asset += total_fees;
   }

   asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset pool_fees = total_fees - network_fees - interface_fees;

   asset liquid = get_liquid_balance( account.name, input_asset.symbol );

   FC_ASSERT( liquid >= input_asset,
      "Account: ${a} has Insufficient liquid Balance to acquire requested amount: ${am}.",
      ("a",account.name)("am",receive.to_string()));

   adjust_liquid_balance( account.name, -input_asset );

   pay_network_fees( account, network_fees );
   pay_fee_share( int_account, interface_fees, true );
   
   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( receive.symbol == p.symbol_a )
      {
         p.balance_a -= receive;
         p.balance_b += input_asset;
      }
      else if( receive.symbol == p.symbol_a )
      {
         p.balance_b -= receive;
         p.balance_a += input_asset;
      }
      if( pool_fees.symbol == p.symbol_a )
      {
         p.balance_a += pool_fees;
      }
      else if( pool_fees.symbol == p.symbol_b )
      {
         p.balance_b += pool_fees;
      }
   });

   adjust_liquid_balance( account.name, receive );

   ilog( "Acquired liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input_asset.to_string())("o",receive.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (receive)(account)(pool) ) }


void database::liquid_acquire( const asset& receive, const account_object& account, 
   const asset_liquidity_pool_object& pool )
{ try {
   FC_ASSERT( receive.amount > 0 );
   FC_ASSERT( receive.symbol == pool.symbol_a || receive.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset total_fees;
   asset_symbol_type in = pool.base_price( receive.symbol ).quote.symbol;
   uint128_t pr = BLOCKCHAIN_PRECISION.value;
   uint128_t pr_sq = pr * pr;
   uint128_t ib = pool.asset_balance( in ).amount.value;
   uint128_t rb = pool.asset_balance( receive.symbol ).amount.value;
   uint128_t re = receive.amount.value;

   if( receive.symbol == SYMBOL_COIN )
   {
      total_fees = ( receive * TRADING_FEE_PERCENT ) / PERCENT_100;
      re += total_fees.amount.value;
   }

   uint128_t input_amount = ( ( ( pr_sq * ib ) / ( pr - ( ( pr * re ) / rb ) ) ) - ( pr * ib ) ) / pr;
   share_type ia = int64_t( input_amount.to_uint64() );
   asset input_asset = asset( ia, in );
   
   if( receive.symbol != SYMBOL_COIN )
   {
      total_fees = ( input_asset * TRADING_FEE_PERCENT ) / PERCENT_100;
      input_asset += total_fees;
   }

   asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
   asset pool_fees = total_fees - network_fees - interface_fees;

   asset liquid = get_liquid_balance( account.name, input_asset.symbol );

   FC_ASSERT( liquid >= input_asset,
      "Account: ${a} has Insufficient liquid Balance to acquire requested amount: ${am}.",
      ("a",account.name)("am",receive.to_string()));

   adjust_liquid_balance( account.name , -input_asset );

   pay_network_fees( account, network_fees + interface_fees );
   
   modify( pool, [&]( asset_liquidity_pool_object& p )
   {
      if( receive.symbol == p.symbol_a )
      {
         p.balance_a -= receive;
         p.balance_b += input_asset;
      }
      else if( receive.symbol == p.symbol_a )
      {
         p.balance_b -= receive;
         p.balance_a += input_asset;
      }
      if( pool_fees.symbol == p.symbol_a )
      {
         p.balance_a += pool_fees;
      }
      else if( pool_fees.symbol == p.symbol_b )
      {
         p.balance_b += pool_fees;
      }
   });

   adjust_liquid_balance( account.name, receive );

   ilog( "Acquired liquidity Pool: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input_asset.to_string())("o",receive.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (receive)(account)(pool) ) }


/**
 * Sells an input asset into an asset liquidity pool, up to the lower of a specified amount, or
 * an amount that would cause the sale price to fall below a specified limit price.
 */
pair< asset, asset > database::liquid_limit_exchange( const asset& input, const price& limit_price, 
   const asset_liquidity_pool_object& pool, bool execute, bool apply_fees )
{ try {
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( input.symbol == pool.symbol_a || input.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset total_fees;
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   price current = pool.base_price( limit_price.base.symbol );
   price lim;
   if( limit_price.base.symbol == input.symbol )
   {
      lim = limit_price;
   }
   else if( limit_price.quote.symbol == input.symbol)
   {
      lim = ~limit_price;
   }

   if( current > limit_price )
   {
      uint128_t pr = BLOCKCHAIN_PRECISION.value;
      uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
      uint128_t rb = pool.asset_balance( rec ).amount.value;
      uint128_t in = input.amount.value;

      uint128_t product = ( ib * rb * lim.base.amount.value ) / lim.quote.amount.value;
      uint128_t limit_amount = approx_sqrt( product ) - in;

      FC_ASSERT( limit_amount >= 0, 
         "Negative limit amount, limit price above current price.");
      
      uint128_t lim_in = std::min( in, limit_amount );
      share_type lim_in_share = int64_t( lim_in.to_uint64() );
      asset input_asset = asset( lim_in_share, input.symbol );

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( lim_in + ib ) ) ) ) / pr;
      share_type ra = int64_t( return_amount.to_uint64() );
      
      if( apply_fees )
      {
         total_fees = asset( ( ra * TRADING_FEE_PERCENT ) / PERCENT_100,  rec );
         ra -= total_fees.amount;
      }

      asset return_asset = asset( ra, rec );
      asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset pool_fees = total_fees - network_fees;
      
      if( execute )
      {
         modify( pool, [&]( asset_liquidity_pool_object& p )
         {
            if( input.symbol == p.symbol_a )
            {
               p.balance_a += input_asset; 
               p.balance_b -= return_asset;
            }
            else if( input.symbol == p.symbol_b )
            {
               p.balance_b += input_asset; 
               p.balance_a -= return_asset;
            }
            if( apply_fees )
            {
               if( pool_fees.symbol == p.symbol_a )
               {
                  p.balance_a += pool_fees;
               }
               else if( pool_fees.symbol == p.symbol_b )
               {
                  p.balance_b += pool_fees;
               }
            }
         });

         if( apply_fees )
         {
            pay_network_fees( network_fees );
         }

         ilog( "Liquid Limit Exchange: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
            ("i",input_asset.to_string())("o",return_asset.to_string())("p",pool.to_string()));
      }

      return std::make_pair( input_asset, return_asset );
   }
   else
   {
     return std::make_pair( asset(0, input.symbol), asset(0, rec) );
   }
} FC_CAPTURE_AND_RETHROW( (input)(limit_price)(pool) ) }


/**
 * Sells an input asset into an asset liquidity pool, up to the lower of a specified amount, or
 * an amount that would cause the sale price to fall below a specified limit price.
 */
void database::liquid_limit_exchange( const asset& input, const price& limit_price, const account_object& account, 
   const asset_liquidity_pool_object& pool, const account_object& int_account )
{ try {
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( input.symbol == pool.symbol_a || input.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   price current = pool.base_price( limit_price.base.symbol );
   price lim;
   if( limit_price.base.symbol == input.symbol )
   {
      lim = limit_price;
   }
   else if( limit_price.quote.symbol == input.symbol )
   {
      lim = ~limit_price;
   }

   if( current > limit_price )
   {
      uint128_t pr = BLOCKCHAIN_PRECISION.value;
      uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
      uint128_t rb = pool.asset_balance( rec ).amount.value;
      uint128_t in = input.amount.value;

      uint128_t product = ( ib * rb * lim.base.amount.value ) / lim.quote.amount.value;
      uint128_t limit_amount = approx_sqrt( product ) - in;

      FC_ASSERT( limit_amount >= 0, 
         "Negative limit amount, limit price above current price.");
      
      uint128_t lim_in = std::min( in, limit_amount );
      share_type lim_in_share = int64_t( lim_in.to_uint64() );
      asset input_asset = asset( lim_in_share, input.symbol );

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( lim_in + ib ) ) ) ) / pr;
      share_type ra = int64_t( return_amount.to_uint64() );
      
      asset total_fees = asset( ( ra * TRADING_FEE_PERCENT ) / PERCENT_100,  rec );
      ra -= total_fees.amount;
      
      asset return_asset = asset( ra, rec );
      asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset pool_fees = total_fees - network_fees - interface_fees;

      asset liquid = get_liquid_balance( account.name, input_asset.symbol );

      FC_ASSERT( liquid >= input_asset,
      "Account: ${a} has Insufficient liquid Balance to acquire requested amount: ${am}.",
      ("a",account.name)("am",return_asset.to_string()));

      adjust_liquid_balance( account.name , -input_asset );
      
      modify( pool, [&]( asset_liquidity_pool_object& p )
      {
         if( input.symbol == p.symbol_a )
         {
            p.balance_a += input_asset; 
            p.balance_b -= return_asset;
         }
         else if( input.symbol == p.symbol_b )
         {
            p.balance_b += input_asset; 
            p.balance_a -= return_asset;
         }
         
         if( pool_fees.symbol == p.symbol_a )
         {
            p.balance_a += pool_fees;
         }
         else if( pool_fees.symbol == p.symbol_b )
         {
            p.balance_b += pool_fees;
         }
      });

      pay_network_fees( account, network_fees );
      pay_fee_share( int_account, interface_fees, true );

      adjust_liquid_balance( account.name, return_asset );    

      ilog( "Liquid Limit Exchange: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
         ("i",input_asset.to_string())("o",return_asset.to_string())("p",pool.to_string()));
   }
} FC_CAPTURE_AND_RETHROW( (input)(limit_price)(account)(pool) ) }


void database::liquid_limit_exchange( const asset& input, const price& limit_price, const account_object& account, 
   const asset_liquidity_pool_object& pool )
{ try {
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( input.symbol == pool.symbol_a || input.symbol == pool.symbol_b,
      "Invalid pool requested for acquisition.");
   asset_symbol_type rec = pool.base_price( input.symbol ).quote.symbol;
   price current = pool.base_price( limit_price.base.symbol );
   price lim;
   if( limit_price.base.symbol == input.symbol )
   {
      lim = limit_price;
   }
   else if( limit_price.quote.symbol == input.symbol )
   {
      lim = ~limit_price;
   }

   if( current > limit_price )
   {
      uint128_t pr = BLOCKCHAIN_PRECISION.value;
      uint128_t ib = pool.asset_balance( input.symbol ).amount.value;
      uint128_t rb = pool.asset_balance( rec ).amount.value;
      uint128_t in = input.amount.value;

      uint128_t product = ( ib * rb * lim.base.amount.value ) / lim.quote.amount.value;
      uint128_t limit_amount = approx_sqrt( product ) - in;

      FC_ASSERT( limit_amount >= 0, 
         "Negative limit amount, limit price above current price.");
      
      uint128_t lim_in = std::min( in, limit_amount );
      share_type lim_in_share = int64_t( lim_in.to_uint64() );
      asset input_asset = asset( lim_in_share, input.symbol );

      uint128_t return_amount = ( rb * ( pr - ( ( pr * ib ) / ( lim_in + ib ) ) ) ) / pr;
      share_type ra = int64_t( return_amount.to_uint64() );
      
      asset total_fees = asset( ( ra * TRADING_FEE_PERCENT ) / PERCENT_100,  rec );
      ra -= total_fees.amount;
      
      asset return_asset = asset( ra, rec );
      asset network_fees = ( total_fees * NETWORK_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset interface_fees = ( total_fees * TAKER_TRADING_FEE_PERCENT ) / PERCENT_100;
      asset pool_fees = total_fees - network_fees - interface_fees;

      asset liquid = get_liquid_balance( account.name, input_asset.symbol );

      FC_ASSERT( liquid >= input_asset,
      "Account: ${a} has Insufficient liquid Balance to acquire requested amount: ${am}.",
      ("a",account.name)("am",return_asset.to_string()));

      adjust_liquid_balance( account.name , -input_asset );
      
      modify( pool, [&]( asset_liquidity_pool_object& p )
      {
         if( input.symbol == p.symbol_a )
         {
            p.balance_a += input_asset; 
            p.balance_b -= return_asset;
         }
         else if( input.symbol == p.symbol_b )
         {
            p.balance_b += input_asset; 
            p.balance_a -= return_asset;
         }
         
         if( pool_fees.symbol == p.symbol_a )
         {
            p.balance_a += pool_fees;
         }
         else if( pool_fees.symbol == p.symbol_b )
         {
            p.balance_b += pool_fees;
         }
      });

      pay_network_fees( account, network_fees + interface_fees );

      adjust_liquid_balance( account.name, return_asset );

      ilog( "Liquid Limit Exchange: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
         ("i",input_asset.to_string())("o",return_asset.to_string())("p",pool.to_string()));
   }
} FC_CAPTURE_AND_RETHROW( (input)(limit_price)(account)(pool) ) }




void database::update_median_liquidity()
{ try {
   if( (head_block_num() % MEDIAN_LIQUIDITY_INTERVAL_BLOCKS) != 0 )
      return;

   // ilog( "Update Median Liquidity" );

   const auto& liq_idx = get_index< asset_liquidity_pool_index >().indices().get< by_asset_pair>();
   auto liq_itr = liq_idx.begin();

   size_t day_history_window = 1 + fc::days(1).to_seconds() / MEDIAN_LIQUIDITY_INTERVAL.to_seconds();
   size_t hour_history_window = 1 + fc::hours(1).to_seconds() / MEDIAN_LIQUIDITY_INTERVAL.to_seconds();

   while( liq_itr != liq_idx.end() )
   {
      const asset_liquidity_pool_object& pool = *liq_itr;
      ++liq_itr;
      vector< price > day; 
      vector< price > hour;
      day.reserve( day_history_window );
      hour.reserve( hour_history_window );

      modify( pool, [&]( asset_liquidity_pool_object& p )
      {
         p.price_history.push_back( pool.current_price() );
         
         while( p.price_history.size() > day_history_window )
         {
            p.price_history.pop_front();       // Maintain one day worth of price history
         }

         for( auto i : p.price_history )
         {
            day.push_back( i );
         }

         size_t offset = day.size()/2;

         std::nth_element( day.begin(), day.begin()+offset, day.end(),
         []( price a, price b )
         {
            return a < b;
         });

         p.day_median_price = day[ offset ];    // Set day median price to the median of all prices in the last day, at 10 min intervals

         auto hour_itr = p.price_history.rbegin();

         while( hour_itr != p.price_history.rend() && hour.size() < hour_history_window )
         {
            hour.push_back( *hour_itr );
            ++hour_itr;
         }

         offset = hour.size()/2;

         std::nth_element( hour.begin(), hour.begin()+offset, hour.end(),
         []( price a, price b )
         {
            return a < b;
         });

         p.hour_median_price = hour[ offset ];   // Set hour median price to the median of all prices in the last hour, at 10 min intervals

      });
   } 
} FC_CAPTURE_AND_RETHROW() }


/**
 * Adds new capital reserves to an asset's credit liquidity pool.
 * 
 * Returns depositors the credit pool asset which earns a share of
 * incoming interest when withdrawn.
 */
void database::credit_lend( const asset& input, const account_object& account, const asset_credit_pool_object& pool )
{ try {
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( input.symbol == pool.base_symbol,
      "Incorrect pool for input asset" );

   price credit_price = pool.current_price();
   asset borrowed = input * credit_price;
   asset liquid = get_liquid_balance( account.name, input.symbol );

   FC_ASSERT( liquid >= input,
      "Account: ${a} has Insufficient liquid Balance to lend amount to credit pool: ${am}.",
      ("a",account.name)("am",input));
   
   adjust_liquid_balance( account.name, -input );
   adjust_pending_supply( input );

   modify( pool, [&]( asset_credit_pool_object& acpo )
   {
      acpo.base_balance += input;
      acpo.credit_balance += borrowed;
      acpo.last_price = credit_price;
   });

   adjust_liquid_balance( account.name, borrowed );

   ilog( "Credit Lend: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input.to_string())("o",borrowed.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }


/**
 * Withdraws capital reserves from an asset's credit liquidity pool.
 * 
 * Exchanges a credit pool asset for its underlying reserve asset 
 * at the current exchange rate.
 */
void database::credit_withdraw( const asset& input, const account_object& account, const asset_credit_pool_object& pool )
{ try {
   FC_ASSERT( input.amount > 0 );
   FC_ASSERT( input.symbol == pool.credit_symbol,
      "Incorrect pool for input asset" );
   asset liquid = get_liquid_balance( account.name, input.symbol );
   price credit_price = pool.current_price();
   asset withdrawn = input * credit_price;

   FC_ASSERT( liquid >= input,
      "Account: ${a} has Insufficient liquid Balance to withdraw amount to credit pool: ${am}.",
      ("a",account.name)("am",input));
   FC_ASSERT( pool.base_balance >= withdrawn,
      "Credit pool: ${p} does not have sufficient available base balance, please wait for outstanding loans to be repaid.",
      ("p",pool));

   adjust_liquid_balance( account.name, -input );
   adjust_pending_supply( input );

   modify( pool, [&]( asset_credit_pool_object& acpo )
   {
      acpo.base_balance -= withdrawn;
      acpo.credit_balance -= input;
      acpo.last_price = credit_price;
   });

   adjust_liquid_balance( account.name, withdrawn );

   ilog( "Credit Withdraw: \n Input: ${i} \n Output: ${o} \n Pool: ${p} \n",
      ("i",input.to_string())("o",withdrawn.to_string())("p",pool.to_string()));

} FC_CAPTURE_AND_RETHROW( (input)(account)(pool) ) }


/**
 * Checks whether a proposed credit loan has sufficient liquidity.
 * 
 * Confirms that the credit asset has sufficient liquidity to the core asset,
 * and that the debt asset has greater outstanding debt
 * than market_max_credit_ratio (50%) of the amount that the liquidity pool 
 * has available in exchange for the core asset.
 * 
 * Credit Check Objective:
 * 
 * Ensure that the credit loan system is fully solvent and can be liquidated with only liquidity pool reserves.
 * 
 * 1 - Prevent Debt asset from becoming too depressed in the event of a liquidation.
 * 2 - Prevent Collateral asset from becoming too depressed in the event of a liquidation.
 * 3 - Ensure sufficient pool balances to support a full liquidation of an order 10 times the requested size.
 * 4 - Ensure that no assets accumulate margin debt in excess of the total available Coin liquidity for the debt.
 * 5 - Ensure sufficient liquidity for Coin in the credit asset liquidity pool.
 */
bool database::credit_check( const asset& debt, const asset& collateral, const asset_credit_pool_object& credit_pool )
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   const asset_liquidity_pool_object& credit_asset_pool = get_liquidity_pool( SYMBOL_COIN, SYMBOL_CREDIT );  //  Credit : Coin Liquidity pool
   asset collateral_coin = collateral;
   asset debt_coin = debt;
   asset debt_outstanding = credit_pool.borrowed_balance;

   FC_ASSERT( debt.amount > 0 );
   FC_ASSERT( collateral.amount > 0 );
   FC_ASSERT( debt.symbol == credit_pool.base_symbol,
      "Incorrect credit pool for requested debt asset." );

   if( collateral.symbol != SYMBOL_COIN )
   {
      collateral_coin = liquid_exchange( 10 * collateral, SYMBOL_COIN, false, false);
   }
   else
   {
      collateral_coin = 10 * collateral;
   }

   if( debt.symbol != SYMBOL_COIN )     // Coin cost of acquiring 10 times debt amount
   {
      const asset_liquidity_pool_object& debt_pool = get_liquidity_pool( SYMBOL_COIN, debt.symbol );   // Debt : Coin Liquidity pool
      asset debt_balance = debt_pool.asset_balance( debt.symbol );

      if( debt_balance >= 10 * debt )   
      {
         debt_coin = liquid_acquire( 10 * debt, SYMBOL_COIN, false, false );
      }
      else       // Pool does not have enough debt asset 
      {
         ilog( "Credit Check failed: Pool does not have enough debt asset: Required: ${r} Actual: ${a}",
            ("r",(10*debt).to_string())("a",debt_balance.to_string()));
         return false;
      }

      asset debt_limit = ( debt_pool.asset_balance( debt.symbol ) * median_props.market_max_credit_ratio ) / PERCENT_100;
      
      if( debt_outstanding > debt_limit )
      {
         // If too much debt is outstanding on the specified debt asset, compared with available liquidity to Coin
         // Prevent margin liquidations from running out of available debt asset liquidity
         ilog( "Credit Check failed: Insufficient debt asset liquidity: Max Debt limit: ${l} Actual: ${a}",
            ("l",debt_limit.to_string())("a",debt_outstanding.to_string()) );
         return false;
      }
   }
   else
   {
      debt_coin = 10 * debt;
   }

   if( credit_asset_pool.asset_balance( SYMBOL_COIN ) >= debt_coin )  // Not enough coin to cover cost of debt with credit 
   {
      if( collateral_coin >= debt_coin )    // Order 10 times requested would be insolvent due to illiquidity
      {
         return true;     // Requested margin order passes all credit checks 
      }
      else        
      {
         ilog( "Credit Check failed: Insufficient collateral asset liquidity: Required: ${r} Actual: ${a}",
            ("r",debt_coin.to_string())("a",collateral_coin.to_string()));
         return false;
      }
   }
   else
   {
      ilog( "Credit Check failed: Insufficient credit asset liquidity: Coin balance required: ${r} Actual: ${a}",
         ("r",debt_coin.to_string())("a",credit_asset_pool.asset_balance( SYMBOL_COIN ).to_string() ) );
      return false;
   }

} FC_CAPTURE_AND_RETHROW( (debt)(collateral)(credit_pool) ) }


/**
 * Checks whether a proposed margin position has sufficient liquidity.
 * 
 * Confirms that the credit asset has sufficient liquidity to the core asset,
 * and that the debt asset has greater outstanding debt
 * than market_max_credit_ratio (50%) of the amount that the liquidity pool 
 * has available in exchange for the core asset.
 * 
 * Margin Check Objective:
 * 
 * Ensure that the margin order system is fully solvent and can be liquidated with only liquidity pool reserves.
 * 
 * 1 - Prevent Position asset from becoming too squeezed in the event of a liquidation.
 * 2 - Prevent Debt asset from becoming too depressed in the event of a liquidation.
 * 3 - Prevent Collateral asset from becoming too depressed in the event of a liquidation.
 * 4 - Ensure sufficient pool balances to support a full liquidation of an order 10 times the requested size.
 * 5 - Ensure that no assets accumulate margin debt in excess of the total available Coin liquidity for the debt.
 * 6 - Ensure sufficient liquidity for Coin in the credit asset liquidity pool.
 * 
 * @todo Enable margin positions in liquidity and credit pool assets by checking 
 * liquidity of underlying assets after redemptions.
 * 
 * @todo Enhance checks to prevent an arbitrary asset from being issued,
 * lent to its pool, then borrowed and deliberately defaulted on by manipulating the
 * price of the debt or collateral asset, which purchases the issued asset with credit
 * and captures the network credit default acquisition privately.
 * 
 * @todo Dual credit pool system with a high risk pool, and a low risk pool.
 * Losses from loan and margin defaults are covered by the high risk pool.
 * Low risk pool is fully backed by credit asset issuance to cover defaults.
 * The majority of incoming interest revenue is added to the high risk pool.
 */
bool database::margin_check( const asset& debt, const asset& position, const asset& collateral, const asset_credit_pool_object& credit_pool )
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   const asset_liquidity_pool_object& credit_asset_pool = get_liquidity_pool( SYMBOL_COIN, SYMBOL_CREDIT );    //  Credit : Coin Liquidity pool
   asset collateral_coin = collateral;
   asset position_coin = position;
   asset debt_coin = debt;
   asset debt_outstanding = credit_pool.borrowed_balance;

   FC_ASSERT( debt.amount > 0 );
   FC_ASSERT( position.amount > 0 );
   FC_ASSERT( collateral.amount > 0 );
   FC_ASSERT( debt.symbol == credit_pool.base_symbol,
      "Incorrect credit pool for requested debt asset." );

   if( collateral.symbol != SYMBOL_COIN )    // Coin derived from sale of 10 times collateral amount
   {
      collateral_coin = liquid_exchange( 10 * collateral, SYMBOL_COIN, false, false );
   }
   else
   {
      collateral_coin = 10 * collateral;
   }

   if( position.symbol != SYMBOL_COIN )      // Coin derived from sale of 10 times position amount
   {
      position_coin = liquid_exchange( 10 * position, SYMBOL_COIN, false, false );
   }
   else
   {
      position_coin = 10 * position;
   }
   
   if( debt.symbol != SYMBOL_COIN )     // Coin cost of acquiring 10 times debt amount
   {
      const asset_liquidity_pool_object& debt_pool = get_liquidity_pool( SYMBOL_COIN, debt.symbol );   // Debt : Coin Liquidity pool
      asset debt_balance = debt_pool.asset_balance( debt.symbol );

      if( debt_balance >= 10 * debt )
      {
         debt_coin = liquid_acquire( 10 * debt, SYMBOL_COIN, false, false );
      }
      else       // Pool does not have enough debt asset 
      {
         ilog( "Margin Check failed: Pool does not have enough debt asset: Required: ${r} Actual: ${a}",
            ("r",(10*debt).to_string())("a",debt_balance.to_string()));
         return false;
      }

      asset debt_limit = ( debt_balance * median_props.market_max_credit_ratio ) / PERCENT_100;
      
      if( debt_outstanding > debt_limit )
      {
         // If too much debt is outstanding on the specified debt asset, compared with available liquidity to Coin
         // Prevent margin liquidations from running out of available debt asset liquidity
         ilog( "Margin Check failed: Insufficient debt asset liquidity: Max Debt limit: ${l} Actual: ${a}",
            ("l",debt_limit.to_string())("a",debt_outstanding.to_string()));
         return false;
      }
   }
   else
   {
      debt_coin = 10 * debt;
   }

   if( credit_asset_pool.asset_balance( SYMBOL_COIN ) >= debt_coin )  // Not enough coin to cover cost of debt with credit 
   {
      if( ( collateral_coin + position_coin ) >= debt_coin )    // Order 10 times requested would be insolvent due to illiquidity
      {
         ilog( "Margin Check successful." );
         return true;     // Requested margin order passes all credit checks 
      }
      else        
      {
         ilog( "Margin Check failed: Insufficient collateral and position asset liquidity: Required: ${r} Actual: ${a}",
            ("r",debt_coin.to_string())("a",(collateral_coin + position_coin).to_string() ));
         return false;
      }
   }
   else
   {
      ilog( "Margin Check failed: Insufficient credit asset liquidity: Coin balance required: ${r} Actual: ${a}",
         ("r",debt_coin.to_string())("a",credit_asset_pool.asset_balance( SYMBOL_COIN ).to_string()) );
      return false;
   }
} FC_CAPTURE_AND_RETHROW( (debt)(position)(collateral)(credit_pool) ) }



/**
 * Updates the state of all credit loans.
 * 
 * Compounds interest on all credit loans, 
 * checks collateralization ratios, and 
 * liquidates under collateralized loans in 
 * response to price changes.
 */
void database::process_credit_updates()
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = head_block_time();

   const auto& loan_idx = get_index< credit_loan_index >().indices().get< by_liquidation_spread >();
   auto loan_itr = loan_idx.begin();

   // ilog( "Process Credit Updates" );

   while( loan_itr != loan_idx.end() )
   {
      const asset_object& debt_asset = get_asset( loan_itr->debt_asset() );
      const asset_credit_pool_object& credit_pool = get_credit_pool( loan_itr->debt_asset(), false );
      uint16_t fixed = median_props.credit_min_interest;
      uint16_t variable = median_props.credit_variable_interest;
      share_type interest_rate = credit_pool.interest_rate( fixed, variable );
      asset total_interest = asset( 0, debt_asset.symbol );

      while( loan_itr != loan_idx.end() && 
         loan_itr->debt_asset() == debt_asset.symbol )
      {
         const asset_object& collateral_asset = get_asset( loan_itr->collateral_asset() );
         const asset_liquidity_pool_object& pool = get_liquidity_pool( loan_itr->symbol_liquid );
         price col_debt_price = pool.base_hour_median_price( loan_itr->collateral_asset() );

         while( loan_itr != loan_idx.end() &&
            loan_itr->debt_asset() == debt_asset.symbol &&
            loan_itr->collateral_asset() == collateral_asset.symbol )
         {
            const credit_loan_object& loan = *loan_itr;
            ++loan_itr;

            int64_t interest_seconds = ( now - loan.last_interest_time ).to_seconds();
            if( interest_seconds >= INTEREST_MIN_INTERVAL.to_seconds() )    // Check once every 60 seconds
            {
               uint128_t interest_amount = uint128_t( loan.debt.amount.value ) * uint128_t( interest_rate.value ) * uint128_t( interest_seconds );
               interest_amount /= uint128_t( fc::days(365).to_seconds() * PERCENT_100 );

               asset interest = asset( interest_amount.to_uint64(), debt_asset.symbol );
               asset max_debt = ( ( loan.collateral * col_debt_price ) * median_props.credit_liquidation_ratio ) / PERCENT_100;
               price liquidation_price = price( loan.collateral, max_debt );

               modify( loan, [&]( credit_loan_object& c )
               {
                  if( interest_amount > uint128_t( INTEREST_MIN_AMOUNT ) )
                  {
                     c.debt += interest;
                     c.interest += interest;
                     c.last_interest_rate = interest_rate;
                     c.last_interest_time = now;
                  }
                  c.liquidation_price = liquidation_price;
               });

               if( interest_amount > uint128_t( INTEREST_MIN_AMOUNT ) )    // Ensure interest is above dust to prevent lossy rounding
               {
                  total_interest += interest;
               }

               ilog( "Credit Loan Interest paid: ${i} Total Interest: ${t} Last Interest Rate: ${r}",
                  ("i",interest.to_string())("t",loan.interest.to_string())("r",fc::to_string( loan.real_interest_rate() ).substr(0,5) + "%") );

               if( loan.loan_price() < loan.liquidation_price )    // If loan falls below liquidation price
               {
                  liquidate_credit_loan( loan );                   // Liquidate it at current price
               }
            }
         }
      }

      modify( credit_pool, [&]( asset_credit_pool_object& c )
      {
         c.last_interest_rate = interest_rate;
         c.borrowed_balance += total_interest;
      });
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Checks the Credit loan index for any unresolved flash loans 
 * that have not been repaid in the same transaction.
 */
void database::check_flash_loans()
{ try {
   const auto& flash_idx = get_index< credit_loan_index >().indices().get< by_flash_loan >();
   auto flash_itr = flash_idx.lower_bound( true );

   if( flash_itr != flash_idx.end() )
   {
      const credit_loan_object& flash_loan = *flash_itr;
      
      FC_ASSERT( !flash_loan.flash_loan,
         "Transaction does not repay flash loan: ${l}.",
         ("l",flash_loan));
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the state of all margin orders.
 * 
 * Compounds interest on all margin orders, checks collateralization
 * ratios for all orders, and liquidates them if they are under collateralized.
 * Places orders into the book into liquidation mode 
 * if they reach their specified limit stop or take profit price.
 */
void database::process_margin_updates()
{ try {
   const median_chain_property_object& median_props = get_median_chain_properties();
   time_point now = head_block_time();
   const auto& margin_idx = get_index< margin_order_index >().indices().get< by_debt_collateral_position >();
   auto margin_itr = margin_idx.begin();

   while( margin_itr != margin_idx.end() )
   {
      const asset_object& debt_asset = get_asset( margin_itr->debt_asset() );
      const asset_credit_pool_object& credit_pool = get_credit_pool( margin_itr->debt_asset(), false );
      uint16_t fixed = median_props.credit_min_interest;
      uint16_t variable = median_props.credit_variable_interest;
      share_type interest_rate = credit_pool.interest_rate( fixed, variable );
      asset total_interest = asset( 0, debt_asset.symbol );
      price col_debt_price;
      price pos_debt_price;

      while( margin_itr != margin_idx.end() && 
         margin_itr->debt_asset() == debt_asset.symbol )
      {
         const asset_object& collateral_asset = get_asset( margin_itr->collateral_asset() );

         if( collateral_asset.symbol != debt_asset.symbol )
         {
            asset_symbol_type symbol_a;
            asset_symbol_type symbol_b;
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

            const asset_liquidity_pool_object& col_debt_pool = get_liquidity_pool( symbol_a, symbol_b );
            col_debt_price = col_debt_pool.base_hour_median_price( debt_asset.symbol );
         }

         while( margin_itr != margin_idx.end() &&
            margin_itr->debt_asset() == debt_asset.symbol &&
            margin_itr->collateral_asset() == collateral_asset.symbol )
         {
            const asset_object& position_asset = get_asset( margin_itr->position_asset() );

            asset_symbol_type symbol_a;
            asset_symbol_type symbol_b;
            if( debt_asset.id < position_asset.id )
            {
               symbol_a = debt_asset.symbol;
               symbol_b = position_asset.symbol;
            }
            else
            {
               symbol_b = debt_asset.symbol;
               symbol_a = position_asset.symbol;
            }

            const asset_liquidity_pool_object& pos_debt_pool = get_liquidity_pool( symbol_a, symbol_b );
            pos_debt_price = pos_debt_pool.base_hour_median_price( debt_asset.symbol );

            while( margin_itr != margin_idx.end() &&
               margin_itr->debt_asset() == debt_asset.symbol &&
               margin_itr->collateral_asset() == collateral_asset.symbol &&
               margin_itr->position_asset() == position_asset.symbol )
            {
               const margin_order_object& margin = *margin_itr;
               ++margin_itr;
               
               asset collateral_debt_value;

               if( margin.collateral_asset() != margin.debt_asset() )
               {
                  collateral_debt_value = margin.collateral * col_debt_price;
               }
               else
               {
                  collateral_debt_value = margin.collateral;
               }

               asset position_debt_value = margin.position_balance * pos_debt_price;
               asset equity = margin.debt_balance + position_debt_value + collateral_debt_value;
               asset unrealized_value = margin.debt_balance + position_debt_value - margin.debt;
               share_type collateralization = ( ( equity - margin.debt ).amount * share_type( PERCENT_100 ) ) / margin.debt.amount;
               int64_t interest_seconds = ( now - margin.last_interest_time ).to_seconds();

               if( interest_seconds >= INTEREST_MIN_INTERVAL.to_seconds() )    // Check once every hour
               {
                  uint128_t interest_amount = uint128_t( margin.debt.amount.value ) * uint128_t( interest_rate.value ) * uint128_t( interest_seconds );
                  interest_amount /= uint128_t( fc::days(365).to_seconds() * PERCENT_100 );
                  asset interest = asset( interest_amount.to_uint64(), debt_asset.symbol );

                  if( interest.amount > INTEREST_MIN_AMOUNT )      // Ensure interest is above dust to prevent lossy rounding
                  {
                     total_interest += interest;
                  }
                  
                  modify( margin, [&]( margin_order_object& m )
                  {
                     if( interest.amount > INTEREST_MIN_AMOUNT )
                     {
                        m.debt += interest;      // Increment interest onto margin loan
                        m.interest += interest;
                        m.last_interest_time = now;
                        m.last_interest_rate = interest_rate;
                     }

                     m.collateralization = collateralization;
                     m.unrealized_value = unrealized_value;
                  });

                  ilog( "Margin Order Interest Paid: ${i} Total Interest: ${t} Interest Rate: ${r}",
                     ("i",interest.to_string())("t",margin.interest.to_string())("r",fc::to_string( margin.real_interest_rate() ).substr(0,5) + "%"));
               }

               if( margin.collateralization < median_props.margin_liquidation_ratio ||
                     pos_debt_price <= margin.stop_loss_price ||
                     pos_debt_price >= margin.take_profit_price )
               {
                  close_margin_order( margin );          // If margin value falls below collateralization threshold, or stop prices are reached
               }
               else if( pos_debt_price <= margin.limit_stop_loss_price && !margin.liquidating )
               {
                  modify( margin, [&]( margin_order_object& m )
                  {
                     m.liquidating = true;
                     m.last_updated = now;
                     m.sell_price = ~m.limit_stop_loss_price;       // If price falls below limit stop loss, reverse order and sell at limit price
                  });
                  apply_order( margin );
               }
               else if( pos_debt_price >= margin.limit_take_profit_price && !margin.liquidating )
               {
                  modify( margin, [&]( margin_order_object& m )
                  {
                     m.liquidating = true;
                     m.last_updated = now;
                     m.sell_price = ~m.limit_take_profit_price;      // If price rises above take profit, reverse order and sell at limit price
                  });
                  apply_order( margin );
               }
            }     // Same Position, Collateral, and Debt
         }        // Same Collateral and Debt
      }           // Same Debt

      modify( credit_pool, [&]( asset_credit_pool_object& c )
      {
         c.last_interest_rate = interest_rate;
         c.borrowed_balance += total_interest;
      });
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Deleverages a loan and repays the debt.
 * 
 * When a loan has gone under its collateralization requirements, 
 * by selling the collateral to the liquidity arrays.
 */
void database::liquidate_credit_loan( const credit_loan_object& loan )
{ try {
   asset debt_liquidated = liquid_exchange( loan.collateral, loan.debt_asset(), true, true );
   const asset_credit_pool_object& credit_pool = get_credit_pool( loan.debt_asset(), false );

   if( loan.debt.amount > debt_liquidated.amount )
   {
      asset deficit = loan.debt - debt_liquidated;
      asset default_credit = network_credit_acquisition( deficit, true );
      debt_liquidated = loan.debt;
      const account_object& owner = get_account( loan.owner );

      modify( owner, [&]( account_object& a )
      {
         a.loan_default_balance += default_credit;
      });

      ilog( "Account loan default balance: ${b}",
         ("b",owner.loan_default_balance.to_string()));
   }

   modify( credit_pool, [&]( asset_credit_pool_object& c )
   {
      c.borrowed_balance -= loan.debt;
      c.base_balance += debt_liquidated;
   });

   ilog( "Removed: ${v}",("v",loan));
   remove( loan );

} FC_CAPTURE_AND_RETHROW() }



/**
 * Operates an interation of option stike price activation and expiration.
 * 
 * Expires all outstanding option orders, balances and resets option pool strike prices.
 * Adds in a new month of option assets by shifting the option strike forward by one year.
 */
void database::process_option_pools()
{ try {
   if( (head_block_num() % OPTION_INTERVAL_BLOCKS ) != 0 )
   { 
      return; 
   }

   time_point now = head_block_time();
   const auto& option_order_idx = get_index< option_order_index >().indices().get< by_expiration >();
   auto option_order_itr = option_order_idx.begin();

   asset_symbol_type option_symbol;
   option_strike strike;
   price current_price;

   while( option_order_itr != option_order_idx.end() &&
      now >= option_order_itr->expiration() )
   {
      const option_order_object& order = *option_order_itr;
      ++option_order_itr;

      option_symbol = order.debt_type();
      strike = option_strike::from_string( option_symbol );
      const asset_option_pool_object& option_pool = get_option_pool( strike.strike_price.base.symbol, strike.strike_price.quote.symbol );
      const asset_liquidity_pool_object& liquidity_pool = get_liquidity_pool( strike.strike_price.base.symbol, strike.strike_price.quote.symbol );
      current_price = liquidity_pool.base_day_median_price( strike.strike_price.base.symbol );
      flat_set< asset_symbol_type > new_strikes;
      const asset_object& base_asset = get_asset( strike.strike_price.base.symbol );
      const asset_object& quote_asset = get_asset( strike.strike_price.quote.symbol );

      modify( option_pool, [&]( asset_option_pool_object& aopo )
      {
         aopo.expire_strike_prices( strike.expiration_date );
         date_type new_date = date_type( 1, strike.expiration_date.month, strike.expiration_date.year + 1 );
         new_strikes = aopo.add_strike_prices( current_price, new_date );
      });

      for( asset_symbol_type s : new_strikes )     // Create the new asset objects for the option.
      {
         create< asset_object >( [&]( asset_object& a )
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

         create< asset_dynamic_data_object >( [&]( asset_dynamic_data_object& a )
         {
            a.symbol = s;
         });
      }
      clear_asset_balances( option_symbol );      // Clear all balances and order positions of the option.
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Close all Prediction Pools which have reached their resolution time.
 * 
 * Holders of prediction assets receive the balance in the prediction pool
 * in proportion to the amount held.
 * 
 * Prediction resolvers additionally receive a share of the 
 * Bond pool when they resolve to the most voted outcome.
 */
void database::process_prediction_pools()
{ try {
   time_point now = head_block_time();
   const auto& prediction_pool_idx = get_index< asset_prediction_pool_index >().indices().get< by_resolution_time >();
   auto prediction_pool_itr = prediction_pool_idx.begin();
   
   while( prediction_pool_itr != prediction_pool_idx.end() &&
      now >= prediction_pool_itr->resolution_time )
   {
      const asset_prediction_pool_object& prediction_pool = *prediction_pool_itr;
      ++prediction_pool_itr;
      close_prediction_pool( prediction_pool );
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Resolves a prediction pool.
 * 
 * - Finds the winning outcome according to quadratic voting for resolutions.
 * - Pays the holders of the successful outcome asset.
 * - Pays the voters of the successful outcome.
 * - Wipes the blances of all holders of all outcome assets and prediction assets.
 */
void database::close_prediction_pool( const asset_prediction_pool_object& pool )
{ try {
   const auto& balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   const auto& resolution_idx = get_index< asset_prediction_pool_resolution_index >().indices().get< by_outcome_symbol >();
   auto balance_itr = balance_idx.begin();
   auto resolution_itr = resolution_idx.lower_bound( pool.prediction_symbol );

   asset_symbol_type top_outcome;
   uint128_t top_outcome_votes = 0;
   uint128_t top_outcome_shares = 0;
   flat_map< asset_symbol_type, uint128_t > resolution_votes;
   flat_map< asset_symbol_type, uint128_t > resolution_shares;

   for( asset_symbol_type s : pool.outcome_assets )
   {
      resolution_votes[ s ] = 0;
      resolution_shares[ s ] = 0;
   }

   // Determine the top outcome by accumulating resolution votes.

   while( resolution_itr != resolution_idx.end() &&
      resolution_itr->prediction_symbol == pool.prediction_symbol )
   {
      const asset_prediction_pool_resolution_object& resolution = *resolution_itr;
      ++resolution_itr;

      resolution_votes[ resolution.resolution_outcome ] += uint128_t( resolution.resolution_votes().value );
      resolution_shares[ resolution.resolution_outcome ] += uint128_t( resolution.amount.amount.value );

      if( resolution_votes[ resolution.resolution_outcome ] > top_outcome_votes )
      {
         top_outcome_votes = resolution_votes[ resolution.resolution_outcome ];
         top_outcome_shares = resolution_shares[ resolution.resolution_outcome ];
         top_outcome = resolution.resolution_outcome;
      }
   }

   asset prediction_bond_remaining = pool.prediction_bond_pool;
   asset collateral_remaining = pool.collateral_pool;
   resolution_itr = resolution_idx.lower_bound( boost::make_tuple( pool.prediction_symbol, top_outcome ) );
   asset pool_split = asset( 0, pool.collateral_symbol );

   // Distribute funds from the prediction bond pool to the resolvers of the top outcome.

   while( resolution_itr != resolution_idx.end() &&
      resolution_itr->prediction_symbol == pool.prediction_symbol &&
      resolution_itr->resolution_outcome == top_outcome &&
      prediction_bond_remaining.amount > 0 )
   {
      const asset_prediction_pool_resolution_object& resolution = *resolution_itr;
      ++resolution_itr;
      uint128_t split_amount = ( uint128_t( resolution.amount.amount.value ) * uint128_t( pool.prediction_bond_pool.amount.value ) ) / top_outcome_shares;
      pool_split = asset( share_type( split_amount.to_uint64() ), pool.collateral_symbol );
      
      if( pool_split > prediction_bond_remaining )
      {
         pool_split = prediction_bond_remaining;
      }

      ilog( "Account: ${a} received resolving amount: ${am}",
         ("a",resolution.account)("am",pool_split.to_string()));

      adjust_liquid_balance( resolution.account, pool_split );
      prediction_bond_remaining -= pool_split;
   }

   balance_itr = balance_idx.lower_bound( top_outcome );

   // Distribute funds from the collateral pool to the holders of the succesful outcome asset.

   while( balance_itr != balance_idx.end() &&
      balance_itr->symbol == top_outcome &&
      collateral_remaining.amount > 0 )
   {
      const account_balance_object& balance = *balance_itr;
      ++balance_itr;

      pool_split = asset( balance.get_total_balance().amount, pool.collateral_symbol );
      
      if( pool_split > collateral_remaining )
      {
         pool_split = collateral_remaining;
      }

      ilog( "Account: ${a} received prediction outcome amount: ${am}",
         ("a",balance.owner)("am",pool_split.to_string()));

      adjust_liquid_balance( balance.owner, pool_split );
      collateral_remaining -= pool_split;
   }

   // Clear all ballances and orders in the prediction pool base asset and outcome assets.

   clear_asset_balances( pool.prediction_symbol );    

   for( asset_symbol_type a : pool.outcome_assets )
   {
      clear_asset_balances( a );
   }

   ilog( "Removed: ${v}",("v",pool.prediction_symbol));
   remove( pool );
   
} FC_CAPTURE_AND_RETHROW() }


} } // node::chain