#pragma once
#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/protocol/block.hpp>
#include <node/chain/node_object_types.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <numeric>

namespace node { namespace chain {
   
   /**
    * Limit order to buy a specifed asset at a fixed price.
    * 
    * Limit orders can match against other orders on the orderbook
    * when the price is within the limit price stipulated.
    * 
    * Limit orders are executed on a good until cancelled basis, 
    * and expire at the expiration time.
    */
   class limit_order_object : public object< limit_order_object_type, limit_order_object >
   {
      limit_order_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         limit_order_object( Constructor&& c, allocator< Allocator > a ) :
         order_id(a)
         {
            c( *this );
         }

         id_type                id;

         account_name_type      seller;            ///< Selling account name of the trading order.

         shared_string          order_id;          ///< UUIDv4 of the order for each account.

         share_type             for_sale;          ///< asset symbol is sell_price.base.symbol.

         price                  sell_price;        ///< Base price is the asset being sold.

         account_name_type      interface;         ///< The interface account that created the order.

         time_point             created;           ///< Time that the order was created.

         time_point             last_updated;      ///< Time that the order was last modified.

         time_point             expiration;        ///< Expiration time of the order.

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return sell_price.base.symbol < sell_price.quote.symbol ?
               std::make_pair( sell_price.base.symbol, sell_price.quote.symbol ) :
               std::make_pair( sell_price.quote.symbol, sell_price.base.symbol );
         }

         asset                amount_for_sale()const 
         { 
            return asset( for_sale, sell_price.base.symbol );
         }

         asset                amount_to_receive()const 
         { 
            return amount_for_sale() * sell_price;
         }

         asset_symbol_type    sell_asset()const 
         { 
            return sell_price.base.symbol;
         }

         asset_symbol_type    receive_asset()const 
         { 
            return sell_price.quote.symbol;
         }

         double               real_price()const 
         { 
            return sell_price.to_real();
         }
   };


   /**
    * Margin order borrows a debt and holds a trading position.
    * 
    * Margin order holds a balance of collateral and enables long and 
    * short margin positions to make leveraged trades on the 
    * movements of asset prices.
    */
   class margin_order_object : public object< margin_order_object_type, margin_order_object >
   {
      margin_order_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         margin_order_object( Constructor&& c, allocator< Allocator > a ) :
         order_id(a)
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;                       ///< Margin order owners account name

         shared_string              order_id;                    ///< UUIDv4 Unique Identifier of the order for each account.

         price                      sell_price;                  ///< limit exchange price of the borrowed asset being sold for the position asset.

         asset                      collateral;                  ///< Collateral asset used to back the loan value; Returned to credit collateral object when position is closed. 

         asset                      debt;                        ///< Amount of asset borrowed to purchase the position asset. Repaid when the margin order is closed. 

         asset                      debt_balance;                ///< Debt asset that is held by the order when selling debt, or liquidating position.

         asset                      interest;                    ///< Amount of interest accrued on the borrowed asset into the debt value.

         asset                      position;                    ///< Minimum amount of asset to receive as margin position.

         asset                      position_balance;            ///< Amount of asset currently held within the order that has filled.                     

         share_type                 collateralization;           ///< Percentage ratio of ( Collateral + position_balance + debt_balance - debt ) / debt. Position is liquidated when ratio falls below liquidation requirement 

         account_name_type          interface;                   ///< The interface account that created the order.

         time_point                 created;                     ///< Time that the order was created.

         time_point                 last_updated;                ///< Time that the details of the order was last updated.

         time_point                 last_interest_time;          ///< Time that interest was last compounded on the margin order. 

         time_point                 expiration;                  ///< Expiration time of the order.

         asset                      unrealized_value = asset(0, debt.symbol);   ///< Current profit or loss that the position is holding.

         share_type                 last_interest_rate = 0;      ///< The interest rate that was last applied to the order.

         bool                       liquidating = false;         ///< Set to true to place the margin order back into the orderbook and liquidate the position at sell price.

         price                      stop_loss_price = price::min(sell_price.base.symbol, sell_price.quote.symbol);          ///< Price at which the position will be force liquidated if it falls into a net loss.

         price                      take_profit_price = price::max(sell_price.base.symbol, sell_price.quote.symbol);         ///< Price at which the position will be force liquidated if it rises into a net profit.

         price                      limit_stop_loss_price = price::min(sell_price.base.symbol, sell_price.quote.symbol);     ///< Price at which the position will be limit liquidated if it falls into a net loss.

         price                      limit_take_profit_price = price::max(sell_price.base.symbol, sell_price.quote.symbol);   ///< Price at which the position will be limit liquidated if it rises into a net profit.

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return sell_price.base.symbol < sell_price.quote.symbol ?
               std::make_pair( sell_price.base.symbol, sell_price.quote.symbol ) :
               std::make_pair( sell_price.quote.symbol, sell_price.base.symbol );
         }

         asset_symbol_type debt_asset()const
         { 
            return debt.symbol;
         }

         asset_symbol_type position_asset()const 
         { 
            return position.symbol; 
         }

         asset_symbol_type collateral_asset()const
         { 
            return collateral.symbol; 
         } 

         asset                amount_for_sale()const   
         { 
            if( liquidating )
            {
               return position_balance;
            }
            else
            {
               return debt_balance;
            }
         }

         asset                amount_to_receive()const { return amount_for_sale() * sell_price; }

         bool                 filled()const
         { 
            if( liquidating )
            {
               return position_balance.amount == 0;    ///< No position left to sell
            }
            else
            {
               return debt_balance.amount == 0;     ///< No debt left to sell.
            }
         }

         asset_symbol_type    sell_asset()const
         { 
            return sell_price.base.symbol; 
         }

         asset_symbol_type    receive_asset()const
         { 
            return sell_price.quote.symbol; 
         }

         double               real_price()const
         { 
            return sell_price.to_real(); 
         }
   };


   /**
    * Holds an auction order that exchanges at the daily clearing price.
    * 
    * Auction orders match against other auction orders once per day,
    * if the daily clearing price is above the limit_close_price of the order.
    */
   class auction_order_object : public object< auction_order_object_type, auction_order_object >
   {
      auction_order_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         auction_order_object( Constructor&& c, allocator< Allocator > a ) :
         order_id(a)
         {
            c( *this );
         }

         id_type                  id;

         account_name_type        owner;                    ///< Owner of the Auction order.

         shared_string            order_id;                 ///< uuidv4 of the order for reference.

         asset                    amount_to_sell;           ///< Amount of asset to sell at auction clearing price. Asset is base of price.

         price                    limit_close_price;        ///< The asset pair price to sell the amount at the auction clearing price.

         account_name_type        interface;                ///< Name of the interface that created the transaction.

         time_point               expiration;               ///< Time that the order expires.

         time_point               created;                  ///< Time that the order was created.

         time_point               last_updated;             ///< Time that the order was last modified.

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return limit_close_price.base.symbol < limit_close_price.quote.symbol ?
               std::make_pair( limit_close_price.base.symbol, limit_close_price.quote.symbol ) :
               std::make_pair( limit_close_price.quote.symbol, limit_close_price.base.symbol );
         }

         asset                    amount_for_sale()const
         { 
            return amount_to_sell;
         }

         asset                    amount_to_receive()const
         { 
            return amount_to_sell * limit_close_price;
         }

         asset_symbol_type        sell_asset()const
         { 
            return amount_to_sell.symbol;
         }

         asset_symbol_type        receive_asset()const
         { 
            return limit_close_price.quote.symbol;
         }

         double                   real_price()const
         { 
            return limit_close_price.to_real();
         }
   };


   /**
    * Order that holds a collateralized debt position to issue a market issued asset.
    * 
    * Enables a market issued asset to access the call order as a 
    * repurchase order if the call price falls below the 
    * market collateralization requirement.
    */
   class call_order_object : public object< call_order_object_type, call_order_object >
   {
      call_order_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         call_order_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       borrower;                    ///< Account that is the borrower of the stablecoin asset. 

         asset                   collateral;                  ///< call_price.base.symbol, access via get_collateral

         asset                   debt;                        ///< call_price.quote.symbol, access via get_debt

         price                   call_price;                  ///< Collateral / Debt

         optional< uint16_t >    target_collateral_ratio;     ///< Maximum CR to maintain when selling collateral on margin call

         account_name_type       interface;                   ///< The interface account that created the order

         time_point              created;                     ///< Time that the order was created.

         time_point              last_updated;                ///< Time that the order was last modified.

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            auto tmp = std::make_pair( call_price.base.symbol, call_price.quote.symbol );
            if( tmp.first > tmp.second ) 
            {
               std::swap( tmp.first, tmp.second );
            }
            return tmp;
         }

         double                  real_price()const 
         { 
            return call_price.to_real(); 
         }

         asset amount_to_receive()const { return debt; }

         asset amount_for_sale()const { return collateral; }

         asset_symbol_type debt_type()const { return debt.symbol; }

         asset_symbol_type collateral_type()const { return collateral.symbol; }

         price collateralization()const { return collateral / debt; }

         /**
          *  Calculate maximum quantity of debt to cover to satisfy @ref target_collateral_ratio.
          * 
          * 
            target_CR = max( target_CR, MCR )
            target_CR = new_collateral / ( new_debt / feed_price )
                     = ( collateral - max_amount_to_sell ) * feed_price
                        / ( debt - amount_to_get )
                     = ( collateral - max_amount_to_sell ) * feed_price
                        / ( debt - round_down(max_amount_to_sell * match_price ) )
                     = ( collateral - max_amount_to_sell ) * feed_price
                        / ( debt - (max_amount_to_sell * match_price - x) )
            Note: x is the fraction, 0 <= x < 1
            =>
            max_amount_to_sell = ( (debt + x) * target_CR - collateral * feed_price )
                                 / (target_CR * match_price - feed_price)
                              = ( (debt + x) * tCR / DENOM - collateral * fp_debt_amt / fp_coll_amt )
                                 / ( (tCR / DENOM) * (mp_debt_amt / mp_coll_amt) - fp_debt_amt / fp_coll_amt )
                              = ( (debt + x) * tCR * fp_coll_amt * mp_coll_amt - collateral * fp_debt_amt * DENOM * mp_coll_amt)
                                 / ( tCR * mp_debt_amt * fp_coll_amt - fp_debt_amt * DENOM * mp_coll_amt )
            max_debt_to_cover = max_amount_to_sell * match_price
                              = max_amount_to_sell * mp_debt_amt / mp_coll_amt
                              = ( (debt + x) * tCR * fp_coll_amt * mp_debt_amt - collateral * fp_debt_amt * DENOM * mp_debt_amt)
                              / (tCR * mp_debt_amt * fp_coll_amt - fp_debt_amt * DENOM * mp_coll_amt)
          *
          *  @param match_price the matching price if this call order is margin called
          *  @param feed_price median settlement price of debt asset
          *  @param maintenance_collateral_ratio median maintenance collateral ratio of debt asset
          *  @param maintenance_collateralization maintenance collateralization of debt asset
          *  @return maximum amount of debt that can be called
          */
         share_type get_max_debt_to_cover(
            price match_price,
            price feed_price,
            const uint16_t maintenance_collateral_ratio,
            const optional<price>& maintenance_collateralization = optional<price>()
         )const 
         { try {

            ilog( "Get max debt to cover - match price: ${mp} feed price: ${fp} mcr: ${mcr} mc: ${mc}",
               ("mp",match_price)("fp",feed_price)("mcr",maintenance_collateral_ratio)("mc",maintenance_collateralization) );

            share_type result;
         
            if( feed_price.base.symbol != call_price.base.symbol )
            {
               feed_price = ~feed_price;      // feed_price is in collateral / debt format
            }
               
            FC_ASSERT( feed_price.base.symbol == call_price.base.symbol &&
               feed_price.quote.symbol == call_price.quote.symbol, 
               "Feed and call price must be the same asset pairing." );

            FC_ASSERT( maintenance_collateralization->base.symbol == call_price.base.symbol && 
               maintenance_collateralization->quote.symbol == call_price.quote.symbol );
            
            if( collateralization() > *maintenance_collateralization )
            {
               result = 0;
            }
               
            if( !target_collateral_ratio.valid() ) // target cr is not set
            {
               result = debt.amount;
            }
               
            uint16_t tcr = std::max( *target_collateral_ratio, maintenance_collateral_ratio );    // use mcr if target cr is too small

            price target_collateralization = feed_price * ratio_type( tcr, COLLATERAL_RATIO_DENOM );

            if( match_price.base.symbol != call_price.base.symbol )
            {
               match_price = ~match_price;       // match_price is in collateral / debt format
            }

            FC_ASSERT( match_price.base.symbol == call_price.base.symbol
                     && match_price.quote.symbol == call_price.quote.symbol );

            int256_t mp_debt_amt = match_price.quote.amount.value;
            int256_t mp_coll_amt = match_price.base.amount.value;
            int256_t fp_debt_amt = feed_price.quote.amount.value;
            int256_t fp_coll_amt = feed_price.base.amount.value;
            int256_t numerator = fp_coll_amt * mp_debt_amt * debt.amount.value * tcr - fp_debt_amt * mp_debt_amt * collateral.amount.value * COLLATERAL_RATIO_DENOM;

            if( numerator < 0 ) 
            {
               result = 0;
            }

            int256_t denominator = fp_coll_amt * mp_debt_amt * tcr - fp_debt_amt * mp_coll_amt * COLLATERAL_RATIO_DENOM;
            if( denominator <= 0 )
            {
               result = debt.amount;     // black swan
            } 

            int256_t to_cover_i256 = ( numerator / denominator );
            if( to_cover_i256 >= debt.amount.value )
            { 
               result = debt.amount;
            }

            share_type to_cover_amt = static_cast< int64_t >( to_cover_i256 );
            asset to_pay = asset( to_cover_amt, debt_type() ) * match_price;
            asset to_cover = to_pay * match_price;
            to_pay = to_cover.multiply_and_round_up( match_price );

            if( to_cover.amount >= debt.amount || to_pay.amount >= collateral.amount )
            {
               result = debt.amount;
            }

            FC_ASSERT( to_pay.amount < collateral.amount && to_cover.amount < debt.amount );
            price new_collateralization = ( collateral - to_pay ) / ( debt - to_cover );

            if( new_collateralization > target_collateralization )
            {
               result = to_cover.amount;
            }

            // to_cover is too small due to rounding. deal with the fraction

            numerator += fp_coll_amt * mp_debt_amt * tcr; // plus the fraction
            to_cover_i256 = ( numerator / denominator ) + 1;

            if( to_cover_i256 >= debt.amount.value )
            {
               to_cover_i256 = debt.amount.value;
            }

            to_cover_amt = static_cast< int64_t >( to_cover_i256 );

            asset max_to_pay = ( ( to_cover_amt == debt.amount.value ) ? collateral : asset( to_cover_amt, debt_type() ).multiply_and_round_up( match_price ) );
            if( max_to_pay.amount > collateral.amount )
            { 
               max_to_pay.amount = collateral.amount; 
            }

            asset max_to_cover = ( ( max_to_pay.amount == collateral.amount ) ? debt : ( max_to_pay * match_price ) );

            if( max_to_cover.amount >= debt.amount )
            {
               max_to_pay.amount = collateral.amount;
               max_to_cover.amount = debt.amount;
            }

            if( max_to_pay <= to_pay || max_to_cover <= to_cover ) // strange data. should skip binary search and go on, but doesn't help much
            { 
               result = debt.amount; 
            }

            FC_ASSERT( max_to_pay > to_pay && max_to_cover > to_cover );

            asset min_to_pay = to_pay;
            asset min_to_cover = to_cover;

            // try with binary search to find a good value
            // note: actually binary search can not always provide perfect result here,
            //       due to rounding, collateral ratio is not always increasing while to_pay or to_cover is increasing
            bool max_is_ok = false;
            while( true )
            {
               // get the mean
               if( match_price.base.amount < match_price.quote.amount ) // step of collateral is smaller
               {
                  to_pay.amount = ( min_to_pay.amount + max_to_pay.amount + 1 ) / 2; // should not overflow. round up here
                  if( to_pay.amount == max_to_pay.amount )
                     to_cover.amount = max_to_cover.amount;
                  else
                  {
                     to_cover = to_pay * match_price;
                     if( to_cover.amount >= max_to_cover.amount ) // can be true when max_is_ok is false
                     {
                        to_pay.amount = max_to_pay.amount;
                        to_cover.amount = max_to_cover.amount;
                     }
                     else
                     {
                        to_pay = to_cover.multiply_and_round_up( match_price ); // stabilization, no change or become smaller
                        FC_ASSERT( to_pay.amount < max_to_pay.amount );
                     }
                  }
               }
               else // step of debt is smaller or equal
               {
                  to_cover.amount = ( min_to_cover.amount + max_to_cover.amount ) / 2; // should not overflow. round down here
                  if( to_cover.amount == max_to_cover.amount )
                     to_pay.amount = max_to_pay.amount;
                  else
                  {
                     to_pay = to_cover.multiply_and_round_up( match_price );
                     if( to_pay.amount >= max_to_pay.amount ) // can be true when max_is_ok is false
                     {
                        to_pay.amount = max_to_pay.amount;
                        to_cover.amount = max_to_cover.amount;
                     }
                     else
                     {
                        to_cover = to_pay * match_price; // stabilization, to_cover should have increased
                        if( to_cover.amount >= max_to_cover.amount ) // to be safe
                        {
                           to_pay.amount = max_to_pay.amount;
                           to_cover.amount = max_to_cover.amount;
                        }
                     }
                  }
               }

               // check again to see if we've moved away from the minimums, if not, use the maximums directly
               if( to_pay.amount <= min_to_pay.amount || 
                  to_cover.amount <= min_to_cover.amount || 
                  to_pay.amount > max_to_pay.amount || 
                  to_cover.amount > max_to_cover.amount )
               {
                  to_pay.amount = max_to_pay.amount;
                  to_cover.amount = max_to_cover.amount;
               }

               // check the mean
               if( to_pay.amount == max_to_pay.amount && ( max_is_ok || to_pay.amount == collateral.amount ) )
               {
                  result = to_cover.amount;
               }

               FC_ASSERT( to_pay.amount < collateral.amount && to_cover.amount < debt.amount );

               price new_collateralization = ( collateral - to_pay ) / ( debt - to_cover );

               // Check whether the result is good
               if( new_collateralization > target_collateralization )
               {
                  if( to_pay.amount == max_to_pay.amount )
                  {
                     result = to_cover.amount;
                  }  
                  max_to_pay.amount = to_pay.amount;
                  max_to_cover.amount = to_cover.amount;
                  max_is_ok = true;
               }
               else           // not good
               {
                  if( to_pay.amount == max_to_pay.amount )
                  {
                     break;
                  }
                  min_to_pay.amount = to_pay.amount;
                  min_to_cover.amount = to_cover.amount;
               }
            }

            // be here, max_to_cover is too small due to rounding. search forward
            for( uint64_t d1 = 0, d2 = 1, d3 = 1; ; d1 = d2, d2 = d3, d3 = d1 + d2 ) // 1,1,2,3,5,8,...
            {
               if( match_price.base.amount > match_price.quote.amount ) // step of debt is smaller
               {
                  to_pay.amount += d2;
                  if( to_pay.amount >= collateral.amount )
                  {
                     result = debt.amount;
                  }
                  to_cover = to_pay * match_price;
                  if( to_cover.amount >= debt.amount )
                  {
                     result = debt.amount;
                  }
                  to_pay = to_cover.multiply_and_round_up( match_price ); // stabilization
                  if( to_pay.amount >= collateral.amount )
                  {
                     result = debt.amount;
                  }
               }
               else // step of collateral is smaller or equal
               {
                  to_cover.amount += d2;
                  if( to_cover.amount >= debt.amount )
                  {
                     result = debt.amount;
                  }
                  to_pay = to_cover.multiply_and_round_up( match_price );
                  if( to_pay.amount >= collateral.amount )
                  {
                     result = debt.amount;
                  }
                  to_cover = to_pay * match_price; // stabilization
                  if( to_cover.amount >= debt.amount )
                  {
                     result = debt.amount;
                  }
               }

               FC_ASSERT( to_pay.amount < collateral.amount && to_cover.amount < debt.amount );
               price new_collateralization = ( collateral - to_pay ) / ( debt - to_cover );

               if( new_collateralization > target_collateralization )
               {
                  result = to_cover.amount;
               }
            }

            ilog( "Max debt to cover: ${r}",
               ("r",result));

            return result;
      } FC_CAPTURE_AND_RETHROW() }
   };


   /**
    * Holds an option position that issue new units of option contract assets.
    * 
    * Option assets can be exercised by the holder to execute a 
    * trade at the specified strike price before the expiration date.
    * 
    * Can be closed by repaying the option asset back into the order.
    * 
    * Orders are assigned the trade execution if the options 
    * are exercised in order of creation.
    */
   class option_order_object : public object< option_order_object_type, option_order_object >
   {
      option_order_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         option_order_object( Constructor&& c, allocator< Allocator > a ) :
         order_id(a)
         {
            c( *this );
         }

         id_type                      id;

         account_name_type            owner;                    ///< Owner of the Option order.

         shared_string                order_id;                 ///< uuidv4 of the order for reference.

         asset                        option_position;          ///< Amount of option assets generated by the position. Debt owed by the order.

         asset                        underlying_amount;        ///< Assets to issue options contract assets against.

         asset                        exercise_amount;          ///< Assets that will be received if the options are all excercised.

         option_strike                strike_price;             ///< The asset pair strike price at which the options can be exercised at any time before expiration.

         account_name_type            interface;                ///< Name of the interface that created the transaction.

         time_point                   created;                  ///< Time that the order was created.

         time_point                   last_updated;             ///< Time that the order was last modified.

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            auto tmp = std::make_pair( option_price().base.symbol, option_price().quote.symbol );
            if( tmp.first > tmp.second ) std::swap( tmp.first, tmp.second );
            return tmp;
         }

         asset                        amount_for_sale()const { return underlying_amount; }

         price                        option_price()const { return strike_price.strike_price; }

         asset                        amount_to_receive()const { return exercise_amount; }

         asset_symbol_type            debt_type()const { return option_position.symbol; }

         asset_symbol_type            collateral_type()const { return underlying_amount.symbol; }

         time_point                   expiration()const { return strike_price.expiration(); }

         double                       real_price()const { return option_price().to_real(); }

         bool                         call()const { return strike_price.call; }
   };


   struct by_high_price;
   struct by_low_price;

   struct by_account;
   struct by_expiration;
   struct by_market;
   struct by_symbol;

   
   typedef multi_index_container<
      limit_order_object,
      indexed_by<
         ordered_unique< tag< by_id >, 
            member< limit_order_object, limit_order_id_type, &limit_order_object::id > >,
         ordered_non_unique< tag< by_expiration >, 
            member< limit_order_object, time_point, &limit_order_object::expiration > >,
         ordered_unique< tag< by_high_price >,
            composite_key< limit_order_object,
               member< limit_order_object, price, &limit_order_object::sell_price >,
               member< limit_order_object, limit_order_id_type, &limit_order_object::id >
            >,
            composite_key_compare< 
               std::greater< price >,
               std::less< limit_order_id_type >
            >
         >,
         ordered_unique< tag< by_low_price >,
            composite_key< limit_order_object,
               member< limit_order_object, price, &limit_order_object::sell_price >,
               member< limit_order_object, limit_order_id_type, &limit_order_object::id >
            >,
            composite_key_compare< 
               std::less< price >,
               std::less< limit_order_id_type >
            >
         >,
         ordered_unique< tag< by_market >,
            composite_key< limit_order_object,
               const_mem_fun< limit_order_object, pair< asset_symbol_type, asset_symbol_type >, &limit_order_object::get_market >,
               member< limit_order_object, limit_order_id_type, &limit_order_object::id >
            >,
            composite_key_compare< 
               std::less< pair< asset_symbol_type, asset_symbol_type > >,
               std::less< limit_order_id_type >
            >
         >,
         ordered_unique< tag< by_symbol >,
            composite_key< limit_order_object,
               const_mem_fun< limit_order_object, asset_symbol_type, &limit_order_object::sell_asset >,
               member< limit_order_object, limit_order_id_type, &limit_order_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< limit_order_id_type >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< limit_order_object,
               member< limit_order_object, account_name_type, &limit_order_object::seller >,
               member< limit_order_object, shared_string, &limit_order_object::order_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >
      >,
      allocator< limit_order_object >
   > limit_order_index;

   struct by_debt;
   struct by_collateral;
   struct by_position;
   struct by_collateralization;
   struct by_debt_collateral_position;

   typedef multi_index_container<
      margin_order_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< margin_order_object, margin_order_id_type, &margin_order_object::id > >,
         ordered_non_unique< tag< by_expiration >, member< margin_order_object, time_point, &margin_order_object::expiration > >,
         ordered_unique< tag< by_high_price >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, bool, &margin_order_object::filled >,
               member< margin_order_object, price, &margin_order_object::sell_price >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare<
               std::less< bool >,
               std::greater< price >,
               std::less< margin_order_id_type >
            >
         >,
         ordered_unique< tag< by_low_price >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, bool, &margin_order_object::filled >,
               member< margin_order_object, price, &margin_order_object::sell_price >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare<
               std::less< bool >,
               std::less< price >,
               std::less< margin_order_id_type >
            >
         >,
         ordered_unique< tag< by_market >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, pair< asset_symbol_type, asset_symbol_type >, &margin_order_object::get_market >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare< 
               std::less< pair< asset_symbol_type, asset_symbol_type > >,
               std::less< margin_order_id_type >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< margin_order_object,
               member< margin_order_object, account_name_type, &margin_order_object::owner >,
               member< margin_order_object, shared_string, &margin_order_object::order_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >,
         ordered_unique< tag< by_debt >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::debt_asset >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >, 
               std::less< margin_order_id_type > 
            >
         >,
         ordered_unique< tag< by_position >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::position_asset >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >, 
               std::less< margin_order_id_type > 
            >
         >,
         ordered_unique< tag< by_collateral >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::collateral_asset >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< margin_order_id_type >
            >
         >,
         ordered_unique< tag< by_debt_collateral_position >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::debt_asset >,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::collateral_asset >,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::position_asset >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >, 
               std::less< asset_symbol_type >, 
               std::less< asset_symbol_type >, 
               std::less< margin_order_id_type > 
            >
         >,
         ordered_unique< tag< by_collateralization >,
            composite_key< margin_order_object,
               member< margin_order_object, share_type, &margin_order_object::collateralization >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare< 
               std::less< share_type >, 
               std::less< margin_order_id_type > 
            >
         >
      >,
      allocator< margin_order_object >
   > margin_order_index;

   

   typedef multi_index_container<
      auction_order_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< auction_order_object, auction_order_id_type, &auction_order_object::id > >,
         ordered_non_unique< tag< by_expiration >, 
            member< auction_order_object, time_point, &auction_order_object::expiration > >,
         ordered_non_unique< tag< by_symbol >, 
            const_mem_fun< auction_order_object, asset_symbol_type, &auction_order_object::sell_asset > >,
         ordered_unique< tag< by_high_price >,
            composite_key< auction_order_object,
               member< auction_order_object, price, &auction_order_object::limit_close_price >,
               member< auction_order_object, auction_order_id_type, &auction_order_object::id >
            >,
            composite_key_compare< 
               std::greater< price >, 
               std::less< auction_order_id_type > 
            >
         >,
         ordered_unique< tag< by_low_price >,
            composite_key< auction_order_object,
               member< auction_order_object, price, &auction_order_object::limit_close_price >,
               member< auction_order_object, auction_order_id_type, &auction_order_object::id >
            >,
            composite_key_compare< 
               std::less< price >,
               std::less< auction_order_id_type >
            >
         >,
         ordered_unique< tag< by_market >,
            composite_key< auction_order_object,
               const_mem_fun< auction_order_object, pair< asset_symbol_type, asset_symbol_type >, &auction_order_object::get_market >,
               member< auction_order_object, auction_order_id_type, &auction_order_object::id >
            >,
            composite_key_compare< 
               std::less< pair< asset_symbol_type, asset_symbol_type > >, 
               std::less< auction_order_id_type > 
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< auction_order_object,
               member< auction_order_object, account_name_type, &auction_order_object::owner >,
               member< auction_order_object, shared_string, &auction_order_object::order_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >
      >, 
      allocator < auction_order_object >
   > auction_order_index;


   typedef multi_index_container<
      call_order_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< call_order_object, call_order_id_type, &call_order_object::id > >,
         ordered_unique< tag< by_high_price >,
            composite_key< call_order_object,
               member< call_order_object, price, &call_order_object::call_price >,
               member< call_order_object, call_order_id_type, &call_order_object::id >
            >,
            composite_key_compare< 
               std::greater< price >, 
               std::less< call_order_id_type > 
            >
         >,
         ordered_unique< tag< by_low_price >,
            composite_key< call_order_object,
               member< call_order_object, price, &call_order_object::call_price >,
               member< call_order_object, call_order_id_type, &call_order_object::id >
            >,
            composite_key_compare< 
               std::less< price >, 
               std::less< call_order_id_type > 
            >
         >,
         ordered_unique< tag< by_debt >,
            composite_key< call_order_object,
               const_mem_fun< call_order_object, asset_symbol_type, &call_order_object::debt_type >,
               const_mem_fun< call_order_object, price, &call_order_object::collateralization >,
               member< call_order_object, call_order_id_type, &call_order_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< price >,
               std::less< call_order_id_type >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< call_order_object,
               member< call_order_object, account_name_type, &call_order_object::borrower >,
               const_mem_fun< call_order_object, asset_symbol_type, &call_order_object::debt_type >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< asset_symbol_type >
            >
         >,
         ordered_unique< tag< by_market >,
            composite_key< call_order_object,
               const_mem_fun< call_order_object, pair< asset_symbol_type, asset_symbol_type >, &call_order_object::get_market >,
               member< call_order_object, call_order_id_type, &call_order_object::id >
            >,
            composite_key_compare< 
               std::less< pair< asset_symbol_type, asset_symbol_type > >, 
               std::less< call_order_id_type > 
            >
         >,
         ordered_unique< tag< by_collateral >,
            composite_key< call_order_object,
               const_mem_fun< call_order_object, price, &call_order_object::collateralization >,
               member< call_order_object, call_order_id_type, &call_order_object::id >
            >,
            composite_key_compare< 
               std::less< price >, 
               std::less< call_order_id_type > 
            >
         >
      >, 
      allocator < call_order_object >
   > call_order_index;


   struct by_strike_price;
   struct by_symbol;


   typedef multi_index_container<
      option_order_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< option_order_object, option_order_id_type, &option_order_object::id > >,
         ordered_non_unique< tag< by_expiration >, 
            const_mem_fun< option_order_object, time_point, &option_order_object::expiration > >,
         ordered_unique< tag< by_strike_price >,
            composite_key< option_order_object,
               member< option_order_object, option_strike, &option_order_object::strike_price >,
               member< option_order_object, option_order_id_type, &option_order_object::id >
            >,
            composite_key_compare< 
               std::less< option_strike >, 
               std::less< option_order_id_type > 
            >
         >,
         ordered_unique< tag< by_high_price >,
            composite_key< option_order_object,
               const_mem_fun< option_order_object, price, &option_order_object::option_price >,
               member< option_order_object, option_order_id_type, &option_order_object::id >
            >,
            composite_key_compare< 
               std::greater< price >, 
               std::less< option_order_id_type > 
            >
         >,
         ordered_unique< tag< by_low_price >,
            composite_key< option_order_object,
               const_mem_fun< option_order_object, price, &option_order_object::option_price >,
               member< option_order_object, option_order_id_type, &option_order_object::id >
            >,
            composite_key_compare< 
               std::less< price >,
               std::less< option_order_id_type >
            >
         >,
         ordered_unique< tag< by_symbol >,
            composite_key< option_order_object,
               const_mem_fun< option_order_object, asset_symbol_type, &option_order_object::debt_type >,
               member< option_order_object, option_order_id_type, &option_order_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< option_order_id_type >
            >
         >,
         ordered_unique< tag< by_market >,
            composite_key< option_order_object,
               const_mem_fun< option_order_object, pair< asset_symbol_type, asset_symbol_type >, &option_order_object::get_market >,
               member< option_order_object, option_order_id_type, &option_order_object::id >
            >,
            composite_key_compare< 
               std::less< pair< asset_symbol_type, asset_symbol_type > >,
               std::less< option_order_id_type > 
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< option_order_object,
               member< option_order_object, account_name_type, &option_order_object::owner >,
               member< option_order_object, shared_string, &option_order_object::order_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >
      >, 
      allocator < option_order_object >
   > option_order_index;

} } // node::chain

FC_REFLECT( node::chain::limit_order_object,
         (id)
         (seller)
         (order_id)
         (for_sale)
         (sell_price)
         (interface)
         (created)
         (last_updated)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::limit_order_object, node::chain::limit_order_index );

FC_REFLECT( node::chain::margin_order_object,
         (id)
         (owner)
         (order_id)
         (sell_price)
         (collateral)
         (debt)
         (debt_balance)
         (interest)
         (position)
         (position_balance)
         (collateralization)
         (interface)
         (created)
         (last_updated)
         (last_interest_time)
         (expiration)
         (unrealized_value)
         (last_interest_rate)
         (liquidating)
         (stop_loss_price)
         (take_profit_price)
         (limit_stop_loss_price)
         (limit_take_profit_price)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::margin_order_object, node::chain::margin_order_index );

FC_REFLECT( node::chain::auction_order_object,
         (id)
         (owner)
         (order_id)
         (amount_to_sell)
         (limit_close_price)
         (interface)
         (expiration)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::auction_order_object, node::chain::auction_order_index );

FC_REFLECT( node::chain::call_order_object,
         (id)
         (borrower)
         (collateral)
         (debt)
         (call_price)
         (target_collateral_ratio)
         (interface)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::call_order_object, node::chain::call_order_index );

FC_REFLECT( node::chain::option_order_object,
         (id)
         (owner)
         (order_id)
         (underlying_amount)
         (option_position)
         (strike_price)
         (interface)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::option_order_object, node::chain::option_order_index );