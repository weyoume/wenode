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
    * Asset liquidity pools contain an open market making balance of two assets.
    *  
    * Liquidity arrays utilize an extended series of Bancor Inspired formulae to provide
    * continuous, asynchronous liquidity to every asset in the network.
    * Any asset may be traded directly for any other asset in the network, 
    * by using the set of core asset liquidity arrays between them to intermediate the exchange.
    * Liquidity arrays charge a pool fee for all exchanges that they process.
    * Limit orders may match against the liquidity pool using a liquid limit exchange, creating a unified
    * orderbook between the outstanding limit orders and call orders, plus the current price of the
    * liquidity pool exchange.
    * 
    * Formulae:
    * 
    * R = Returned Asset
    * I = Input Asset
    * Pa = Price (Average)
    * Pf = Price (Final)
    * Br = Balance (Returned Asset)
    * Bi = Balance (Input Asset)
    * Bs = Balance (Supply Liquidity Pool Asset)
    * Sr = Supply Received
    * Si = Supply Input
    * 
    * 
    * R      = Br * ( 1 - ( Bi / ( I + Bi ) ) )    Amount received for a given input
    * I      = Bi * ( 1 - ( R / Br ) ) - Bi        Input Required for a given return amount   
    * Pa     = ( Br * I ) / ( I * Bi + I^2 )       Average price of an exchange with a given input
    * Pf     = ( Br * Bi ) / ( Bi + I )^2          Final price after an exchange with a given input
    * I(max) = sqrt( ( Br * Bi ) / Pf ) - Bi       Maximum input to reach a given final price.
    * Sr     = Bs * (sqrt( 1 + ( I / Bi ) ) - 1 )  Supply asset obtained for funding an input asset.
    * R      = Br * ( 1 - ( 1 - ( Si / Bs ) )^2 )  Amount Received for withdrawing an input Supply asset.
    * 
    */
   class asset_liquidity_pool_object : public object< asset_liquidity_pool_object_type, asset_liquidity_pool_object >
   {
      asset_liquidity_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_liquidity_pool_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id; 

         asset_symbol_type          symbol_a;                      ///< Ticker symbol string of the asset with the lower ID. Must be core asset if one asset is core.

         asset_symbol_type          symbol_b;                      ///< Ticker symbol string of the asset with the higher ID.

         asset_symbol_type          symbol_liquid;                 ///< Ticker symbol of the pool's liquidity pool asset. 

         asset                      balance_a;                     ///< Balance of Asset A. Must be core asset if one asset is core.

         asset                      balance_b;                     ///< Balance of Asset B.

         asset                      balance_liquid;                ///< Outstanding supply of the liquidity asset for the asset pair.

         price                      hour_median_price;             ///< The median price over the past hour, at 10 minute intervals. Used for collateral calculations. 

         price                      day_median_price;              ///< The median price over the last day, at 10 minute intervals.

         std::deque< price >        price_history;                 ///< Tracks the last 24 hours of median price, one per 10 minutes.

         price                      current_price()const           ///< Liquidity pools current price of Asset A and Asset B. Asset A is the base asset, Asset B is the quote asset.
         {
            return price( balance_a, balance_b );
         }

         price                       base_price( const asset_symbol_type& base )const   ///< Current price with specified asset as base
         {
            FC_ASSERT( base == symbol_a || base == symbol_b,
               "Invalid base asset price requested." );
            if( base == symbol_a )
            {
               return price( balance_a, balance_b);
            }
            else
            {
               return price( balance_b, balance_a );
            }
         }

         price                       base_hour_median_price( const asset_symbol_type& base )const   ///< hourly median price with specified asset as base
         {
            FC_ASSERT( base == symbol_a || base == symbol_b,
               "Invalid base asset price requested." );
            if( base == symbol_a )
            {
               return hour_median_price;
            }
            else
            {
               return ~hour_median_price;
            }
            
         }

         price                       base_day_median_price( const asset_symbol_type& base )const   ///< daily median price with specified asset as base
         {
            FC_ASSERT( base == symbol_a || base == symbol_b,
               "Invalid base asset price requested." );
            if( base == symbol_a )
            {
               return day_median_price;
            }
            else
            {
               return ~day_median_price;
            }
         }

         asset                       asset_balance( const asset_symbol_type& symbol )const
         {
            FC_ASSERT( symbol == symbol_a || symbol == symbol_b,
               "Invalid asset balance requested." );
            if( symbol == symbol_a )
            {
               return balance_a;
            }
            else if( symbol == symbol_b )
            {
               return balance_b;
            }
            else
            {
               return asset( 0, symbol );
            }
         }

         string                      to_string()const
         {
            return symbol_liquid + " - A: " + balance_a.to_string() + " - B: " + balance_b.to_string() + " - L: " + balance_liquid.to_string() +
               " - Current: " + current_price().to_string() + " - Hourly: " + hour_median_price.to_string() + " - Daily: " + day_median_price.to_string();
         }
   };


   /**
    * Holds a reserve of an asset for lending to borrowers.
    * 
    * Funds can be lent to the pool in exchange for the credit pool
    * asset, and earn interest from borrowers when funds are used for
    * margin trading and borrowing orders.
    * 
    * The interest rate charged varies with the ratio of funds 
    * available to the funds currently loaned out. 
    * Interest rates lower when more funds are available in the 
    * @ref base_balance and increase when the @ref borrowed_balance becomes higher. 
    * 
    * Each asset specifies a minimum interest rate,
    * and a variable interest rate that enable 
    * the rate to float accoridng to market credit conditions.
    */
   class asset_credit_pool_object : public object< asset_credit_pool_object_type, asset_credit_pool_object >
   {
      asset_credit_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_credit_pool_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id; 

         asset_symbol_type          base_symbol;            ///< Ticker symbol string of the base asset being lent and borrowed.

         asset_symbol_type          credit_symbol;          ///< Ticker symbol string of the credit asset for use as collateral to borrow the base asset.

         asset                      base_balance;           ///< Balance of the base asset that is available for loans and redemptions.

         asset                      borrowed_balance;       ///< Total amount of base asset currently lent to borrowers, accumulates compounding interest payments.

         asset                      credit_balance;         ///< Balance of the credit asset redeemable for an increasing amount of base asset.

         share_type                 last_interest_rate;     ///< The most recently calculated interest rate when last compounded.

         price                      last_price;             ///< The last price that assets were lent or withdrawn at.

         price                      current_price()const    ///< Credit pool's current price of the credit asset, denominated in the base asset. Increases over time with incoming interest.
         {
            return price( base_balance + borrowed_balance, credit_balance );
         }      

         share_type                 interest_rate( uint16_t min, uint16_t var )const  ///< Current annualized interest rate of borrowing and lending. Updates whenever a loan is taken or repaid.  
         {
            return share_type( std::min( uint64_t( 
               min + var * ( ( borrowed_balance.amount.value + BLOCKCHAIN_PRECISION.value ) / 
               ( base_balance.amount.value + BLOCKCHAIN_PRECISION.value ) ) ), uint64_t( 50 * PERCENT_1 ) ) );
         }

         double                     real_price()const
         {
            return current_price().to_real();
         }

         double                     real_interest_rate()const
         {
            return double(last_interest_rate.value)/double(100);
         }

         string                     to_string()const
         {
            return credit_symbol + " - Base: " + base_balance.to_string() + " - Borrowed: " + borrowed_balance.to_string() + " - Credit: " + credit_balance.to_string() +
               " - Current: " + current_price().to_string() + " - Last Interest Rate: " + fc::to_string( real_interest_rate() ).substr(0,5) + "%";
         }
   };
   

   /**
    * Holds asset collateral balance for lending.
    * 
    * Use to support a credit borrowing order from the asset's credit pool, 
    * or a margin order by substracting collateral from 
    * the object to include in a margin order's collateral.
    */
   class credit_collateral_object : public object< credit_collateral_object_type, credit_collateral_object >
   {
      credit_collateral_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         credit_collateral_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;               ///< Collateral owners account name.

         asset_symbol_type          symbol;              ///< Asset symbol being collateralized. 

         asset                      collateral;          ///< Asset balance that is being locked in for loan backing for loan or margin orders.

         time_point                 created;             ///< Time that collateral was created.

         time_point                 last_updated;        ///< Time that the order was last modified.
   };


   /**
    * Holds assets in collateral for use to support a credit borrowing order.
    * 
    * Borrows funds from the asset's credit pool,
    * or a margin order by substracting collateral from 
    * the object to include in a margin order's collateral.
    */
   class credit_loan_object : public object< credit_loan_object_type, credit_loan_object >
   {
      credit_loan_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         credit_loan_object( Constructor&& c, allocator< Allocator > a ) :
         loan_id(a)
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;                   ///< Collateral owner's account name

         shared_string              loan_id;                 ///< UUIDV4 for the loan to uniquely identify it for reference. 

         asset                      debt;                    ///< Amount of an asset borrowed. Limit of 75% of collateral value. Increases with interest charged.

         asset                      interest;                ///< Total Amount of interest accrued on the loan. 

         asset                      collateral;              ///< Amount of an asset to use as collateral for the loan.             

         price                      liquidation_price;       ///< Collateral / max_debt value. Rises when collateral/debt market price falls.

         asset_symbol_type          symbol_a;                ///< The symbol of asset A in the debt / collateral exchange pair.

         asset_symbol_type          symbol_b;                ///< The symbol of asset B in the debt / collateral exchange pair.

         asset_symbol_type          symbol_liquid;           ///< The symbol of the liquidity pool that underlies the loan object. 

         share_type                 last_interest_rate;      ///< Updates the interest rate of the loan hourly. 

         time_point                 created;                 ///< Time that the loan was taken out.

         time_point                 last_updated;            ///< Time that the loan details was last updated.

         time_point                 last_interest_time;      ///< Time that interest was last compounded. 

         bool                       flash_loan = false;      ///< True when the loan is a flash loan, which has no collateral and must be repaid within same txn.

         asset_symbol_type          debt_asset()const { return debt.symbol; }

         asset_symbol_type          collateral_asset()const { return collateral.symbol; }

         price                      loan_price()const { return collateral / debt; }     ///< Collateral / Debt. Must be higher than liquidation price to remain solvent. 

         price                      liquidation_spread()const { return loan_price() - liquidation_price; }

         double                     real_interest_rate()const
         {
            return double(last_interest_rate.value)/double(100);
         }

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return debt.symbol < collateral.symbol ?
               std::make_pair( debt.symbol, collateral.symbol ) :
               std::make_pair( collateral.symbol, debt.symbol );
         }
   };


   /**
    * Manages the option chain sheet of an asset trading pair.
    * 
    * Each asset pair creates 22 option strike prices each month
    * that can be used to issue option assets. 
    * 
    * Option assets are opened for 12 months in advance of the current month, 
    * and each expiration adds a new month in the year ahead.
    * 
    * Strike prices are determined by rounding to the nearest significant figure
    * and incrementing by 5% intervals up and down the price book.
    * 
    * A total of 264 Option Strikes are open at any given time.
    */
   class asset_option_pool_object : public object< asset_option_pool_object_type, asset_option_pool_object >
   {
      asset_option_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_option_pool_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                id; 

         asset_symbol_type                      base_symbol;       ///< Symbol of the base asset of the trading pair.

         asset_symbol_type                      quote_symbol;      ///< Symbol of the quote asset of the trading pair.
         
         flat_set< asset_symbol_type >          call_symbols;      ///< Symbols of the call options at currently active strikes.

         flat_set< option_strike >              call_strikes;      ///< Available strike price and expirations of call options to buy the quote asset.

         flat_set< asset_symbol_type >          put_symbols;       ///< Symbols of the put options at currently active strikes.

         flat_set< option_strike >              put_strikes;       ///< Available strike price and expirations of put options to sell the quote asset.

         void                                   expire_strike_prices( date_type current_date )
         {
            for( auto call_itr = call_strikes.begin(); call_itr != call_strikes.end(); )
            {
               if( call_itr->expiration_date >= current_date )
               {
                  call_itr = call_strikes.erase( call_itr );
                  call_symbols.erase( call_itr->option_symbol() );
               }
               else
               {
                  ++call_itr;
               }
            }

            for( auto put_itr = put_strikes.begin(); put_itr != put_strikes.end(); )
            {
               if( put_itr->expiration_date >= current_date )
               {
                  put_itr = put_strikes.erase( put_itr );
                  put_symbols.erase( put_itr->option_symbol() );
               }
               else
               {
                  ++put_itr;
               }
            }
         }

         /**
          * Adds a spread of strike prices at a mid_price for a single specified date.
          */
         flat_set< asset_symbol_type >            add_strike_prices( 
               price mid_price, 
               date_type new_date, 
               uint16_t strike_width_percent = OPTION_STRIKE_WIDTH_PERCENT, 
               uint16_t num_strikes = OPTION_NUM_STRIKES )
         {
            FC_ASSERT( strike_width_percent * num_strikes < PERCENT_100,
               "Strike width x num_strikes must be less than 100%, or strike prices will be negative." );

            asset quote_unit( BLOCKCHAIN_PRECISION, quote_symbol );
            asset base_unit = quote_unit * mid_price;
            share_type div = ( base_unit.amount * strike_width_percent ) / PERCENT_100;
            option_strike new_strike;
            flat_set< asset_symbol_type > new_strike_symbols;

            for( int i = -num_strikes; i <= num_strikes; i++ )
            {
               new_strike = option_strike( price( asset( base_unit.amount + i * div, base_symbol ), quote_unit ), true, 100, new_date );
               call_strikes.insert( new_strike );
               call_symbols.insert( new_strike.option_symbol() );
               new_strike_symbols.insert( new_strike.option_symbol() );

               new_strike = option_strike( price( asset( base_unit.amount + i * div, base_symbol ), quote_unit ), false, 100, new_date );
               put_strikes.insert( new_strike );
               put_symbols.insert( new_strike.option_symbol() );
               new_strike_symbols.insert( new_strike.option_symbol() );
            }

            return new_strike_symbols;
         }

         /**
          * Adds a spread of strike prices at a mid_price for a set of specified dates.
          */
         flat_set< asset_symbol_type >            add_strike_prices( 
               price mid_price, 
               flat_set< date_type > new_dates, 
               uint16_t strike_width_percent = OPTION_STRIKE_WIDTH_PERCENT, 
               uint16_t num_strikes = OPTION_NUM_STRIKES )
         {
            FC_ASSERT( strike_width_percent * num_strikes < PERCENT_100,
               "Strike width x num_strikes must be less than 100%, or strike prices will be negative." );

            asset quote_unit( BLOCKCHAIN_PRECISION, quote_symbol );
            asset base_unit = quote_unit * mid_price;
            share_type div = ( base_unit.amount * strike_width_percent ) / PERCENT_100;
            option_strike new_strike;
            flat_set< asset_symbol_type > new_strike_symbols;

            for( date_type d : new_dates )
            {
               for( int i = -num_strikes; i <= num_strikes; i++ )
               {
                  new_strike = option_strike( price( asset( base_unit.amount + i * div, base_symbol ), quote_unit ), true, 100, d );
                  call_strikes.insert( new_strike );
                  call_symbols.insert( new_strike.option_symbol() );
                  new_strike_symbols.insert( new_strike.option_symbol() );

                  new_strike = option_strike( price( asset( base_unit.amount + i * div, base_symbol ), quote_unit ), false, 100, d );
                  put_strikes.insert( new_strike );
                  put_symbols.insert( new_strike.option_symbol() );
                  new_strike_symbols.insert( new_strike.option_symbol() );
               }
            }

            return new_strike_symbols;
         }

         /**
          * Gets a spread of strike prices at a mid_price for a set of specified dates, without adding them.
          */
         flat_set< asset_symbol_type >            get_strike_prices( 
               price mid_price, 
               flat_set< date_type > new_dates, 
               uint16_t strike_width_percent = OPTION_STRIKE_WIDTH_PERCENT, 
               uint16_t num_strikes = OPTION_NUM_STRIKES )const
         {
            FC_ASSERT( strike_width_percent * num_strikes < PERCENT_100,
               "Strike width x num_strikes must be less than 100%, or strike prices will be negative." );

            asset quote_unit( BLOCKCHAIN_PRECISION, quote_symbol );
            asset base_unit = quote_unit * mid_price;
            share_type div = ( base_unit.amount * strike_width_percent ) / PERCENT_100;
            option_strike new_strike;
            flat_set< asset_symbol_type > new_strike_symbols;

            for( date_type d : new_dates )
            {
               for( int i = -num_strikes; i <= num_strikes; i++ )
               {
                  new_strike = option_strike( price( asset( base_unit.amount + i * div, base_symbol ), quote_unit ), true, 100, d );
                  new_strike_symbols.insert( new_strike.option_symbol() );
                  new_strike = option_strike( price( asset( base_unit.amount + i * div, base_symbol ), quote_unit ), false, 100, d );
                  new_strike_symbols.insert( new_strike.option_symbol() );
               }
            }

            return new_strike_symbols;
         }
   };


   /**
    * Manages a prediction market.
    * 
    * Enables a trading market on the likelyhood of
    * a set of public events occuring on or before a specified time
    * period in the future.
    * 
    * Each prediction market has multiple outcome assets,
    * each representing a full unit of the underlying asset 
    * in the event that an outcome occurs.
    * 
    * Funding the prediction pool returns one unit of each outcome
    * asset to the owner, which can be sold.
    * 
    * All funds in the pool are able to be withdrawn after the event
    * has concluded by the holders of the realized outcome asset.
    * 
    * One unit of all outcome assets can be combined and returned to
    * withdraw one unit of the underlying asset. Under normal market conditions, 
    * all asset prices should sum to the value of one unit of the 
    * underlying funding asset.
    * 
    * Prediction markets must include to be valid:
    * - A Public event.
    * - With a series of at least 2 possible outcomes.
    * - That can be reasonably determined on or before a fixed time period.
    * - Where only one outcome can possibly occur.
    * - A public source of information that can be used to verify the outcome.
    * 
    * Each prediction market contains an additional outcome 
    * for invalid or none of the above, in the case that
    * there is a resolution error.
    * 
    * The Security bond is placed by the issuer as a
    * demonstration of confidence in the market, and is lost if the 
    * market is not resolved by the completion time.
    */
   class asset_prediction_pool_object : public object< asset_prediction_pool_object_type, asset_prediction_pool_object >
   {
      asset_prediction_pool_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_prediction_pool_object( Constructor&& c, allocator< Allocator > a ) :
            outcome_details( a ),
            json(a),
            url(a),
            details(a)
            {
               c( *this );
            }

         id_type                                      id;

         asset_symbol_type                            prediction_symbol;        ///< Ticker symbol of the prediction pool primary asset.

         asset_symbol_type                            collateral_symbol;        ///< Ticker symbol of the collateral asset backing the prediction market.

         asset                                        collateral_pool;          ///< Funds accumulated by outcome asset positions for distribution to winning outcome.

         asset                                        prediction_bond_pool;     ///< Security deposit placed by the issuer on the market.

         flat_set< asset_symbol_type >                outcome_assets;           ///< Outcome asset symbols for the market.

         shared_string                                outcome_details;          ///< Description of each outcome and the resolution conditions for each asset. 
         
         shared_string                                json;                     ///< JSON Metadata of the prediction market.

         shared_string                                url;                      ///< Reference URL of the market.

         shared_string                                details;                  ///< Description of the market, how it will be resolved using known public data sources.

         time_point                                   outcome_time;             ///< Time at which the prediction market pool becomes open to resolutions.

         time_point                                   resolution_time;          ///< Time at which the prediction market pool will be resolved.

         void                                         adjust_collateral_pool( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == collateral_symbol );
            collateral_pool += delta;
            FC_ASSERT( collateral_pool.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         void                                         adjust_prediction_bond_pool( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == collateral_symbol );
            prediction_bond_pool += delta;
            FC_ASSERT( prediction_bond_pool.amount >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta ) ) }

         bool                                         is_outcome( const asset_symbol_type& o )const
         {
            return std::find( outcome_assets.begin(), outcome_assets.end(), o ) != outcome_assets.end();
         };
   };


   /**
    * Holds a balance of a Prediction Asset directed to a chosen outcome.
    * 
    * The Amount of Prediction assets used to allocate voting shares
    * towards outcomes determines which outcome wins resolution of the prediction market.
    */
   class asset_prediction_pool_resolution_object : public object< asset_prediction_pool_resolution_object_type, asset_prediction_pool_resolution_object >
   {
      asset_prediction_pool_resolution_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         asset_prediction_pool_resolution_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              account;                       ///< Name of the account which created the prediction market pool.

         asset_symbol_type              prediction_symbol;             ///< Ticker symbol of the prediction pool primary asset.

         asset_symbol_type              resolution_outcome;            ///< Outcome asset symbol for the resolution.

         asset                          amount;                        ///< Amount of Prediction market base asset spent for vote.

         /**
          * Square root of the voting amount, used to calculate the successful outcome.
          */
         share_type                     resolution_votes()const        
         {
            return share_type( int64_t( approx_sqrt( amount.amount.value ) ) );
         }
   };

   struct by_asset_pair;
   struct by_symbol_a;
   struct by_symbol_b;
   struct by_symbol_liquid;

   typedef multi_index_container<
      asset_liquidity_pool_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > >,
         ordered_unique< tag< by_symbol_liquid >, member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_liquid > >,
         ordered_unique< tag< by_asset_pair >,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_a >,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_b > 
            >
         >,
         ordered_unique< tag< by_symbol_a >,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_a >,
               member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > 
            >
         >,
         ordered_unique< tag< by_symbol_b >,
            composite_key< asset_liquidity_pool_object,
               member< asset_liquidity_pool_object, asset_symbol_type, &asset_liquidity_pool_object::symbol_b >,
               member< asset_liquidity_pool_object, asset_liquidity_pool_id_type, &asset_liquidity_pool_object::id > 
            >
         >
      >, 
      allocator< asset_liquidity_pool_object >
   > asset_liquidity_pool_index;

   struct by_base_symbol;
   struct by_credit_symbol;

   typedef multi_index_container<
      asset_credit_pool_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_credit_pool_object, asset_credit_pool_id_type, &asset_credit_pool_object::id > >,
         ordered_unique< tag<by_base_symbol>, member< asset_credit_pool_object, asset_symbol_type, &asset_credit_pool_object::base_symbol > >,
         ordered_unique< tag<by_credit_symbol>, member< asset_credit_pool_object, asset_symbol_type, &asset_credit_pool_object::credit_symbol > >
      >, 
      allocator< asset_credit_pool_object >
   > asset_credit_pool_index;


   struct by_name;
   struct by_symbol;
   struct by_owner_symbol;

   typedef multi_index_container<
      credit_collateral_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< credit_collateral_object, credit_collateral_id_type, &credit_collateral_object::id > >,
         ordered_unique< tag< by_owner_symbol >,
            composite_key< credit_collateral_object,
               member< credit_collateral_object, account_name_type, &credit_collateral_object::owner >,
               member< credit_collateral_object, asset_symbol_type, &credit_collateral_object::symbol >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< asset_symbol_type >
            >
         >,
         ordered_unique< tag< by_owner >,
            composite_key< credit_collateral_object,
               member< credit_collateral_object, account_name_type, &credit_collateral_object::owner >,
               member< credit_collateral_object, credit_collateral_id_type, &credit_collateral_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< credit_collateral_id_type >
            >
         >
      >,
      allocator < credit_collateral_object >
   > credit_collateral_index;

   struct by_loan_id;
   struct by_owner;
   struct by_owner_price;
   struct by_owner_debt;
   struct by_owner_collateral;
   struct by_last_updated; 
   struct by_debt;
   struct by_collateral;
   struct by_liquidation_spread;
   struct by_flash_loan;

   typedef multi_index_container<
      credit_loan_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id > >,
         ordered_unique< tag< by_owner >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag<by_loan_id>,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               member< credit_loan_object, shared_string, &credit_loan_object::loan_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_owner_price >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               const_mem_fun< credit_loan_object, price, &credit_loan_object::loan_price >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< price >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_owner_debt >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< asset_symbol_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_owner_collateral >,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner >,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< asset_symbol_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_liquidation_spread >,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset >,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset >,
               const_mem_fun< credit_loan_object, price, &credit_loan_object::liquidation_spread >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< asset_symbol_type >,
               std::less< price >,
               std::less< credit_loan_id_type >
            >
         >,
         ordered_unique< tag< by_last_updated >,
            composite_key< credit_loan_object,
               member< credit_loan_object, time_point, &credit_loan_object::last_updated >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_debt >,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >, 
               std::less< credit_loan_id_type > 
            >
         >,
         ordered_unique< tag< by_collateral >,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< asset_symbol_type >,
               std::less< credit_loan_id_type >
            >
         >,
         ordered_unique< tag< by_flash_loan >,
            composite_key< credit_loan_object,
               member< credit_loan_object, bool, &credit_loan_object::flash_loan >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::greater< bool >,
               std::less< credit_loan_id_type >
            >
         >
      >, 
      allocator < credit_loan_object >
   > credit_loan_index;

   struct by_quote_symbol;

   typedef multi_index_container<
      asset_option_pool_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< asset_option_pool_object, asset_option_pool_id_type, &asset_option_pool_object::id > >,
         ordered_unique< tag< by_asset_pair >,
            composite_key< asset_option_pool_object,
               member< asset_option_pool_object, asset_symbol_type, &asset_option_pool_object::base_symbol >,
               member< asset_option_pool_object, asset_symbol_type, &asset_option_pool_object::quote_symbol > 
            >
         >,
         ordered_unique< tag<by_base_symbol>,
            composite_key< asset_option_pool_object,
               member< asset_option_pool_object, asset_symbol_type, &asset_option_pool_object::base_symbol >,
               member< asset_option_pool_object, asset_option_pool_id_type, &asset_option_pool_object::id > 
            >
         >,
         ordered_unique< tag<by_quote_symbol>,
            composite_key< asset_option_pool_object,
               member< asset_option_pool_object, asset_symbol_type, &asset_option_pool_object::quote_symbol >,
               member< asset_option_pool_object, asset_option_pool_id_type, &asset_option_pool_object::id > 
            >
         >
      >, 
      allocator< asset_option_pool_object >
   > asset_option_pool_index;


   struct by_prediction_symbol;
   struct by_collateral_symbol;
   struct by_resolution_time;


   typedef multi_index_container<
      asset_prediction_pool_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_prediction_pool_object, asset_prediction_pool_id_type, &asset_prediction_pool_object::id > >,
         ordered_unique< tag< by_prediction_symbol >, member< asset_prediction_pool_object, asset_symbol_type, &asset_prediction_pool_object::prediction_symbol > >,
         ordered_unique< tag< by_collateral_symbol >,
            composite_key< asset_prediction_pool_object,
               member< asset_prediction_pool_object, asset_symbol_type, &asset_prediction_pool_object::collateral_symbol >,
               member< asset_prediction_pool_object, asset_prediction_pool_id_type, &asset_prediction_pool_object::id > 
            >
         >,
         ordered_unique< tag< by_resolution_time >,
            composite_key< asset_prediction_pool_object,
               member< asset_prediction_pool_object, time_point, &asset_prediction_pool_object::resolution_time >,
               member< asset_prediction_pool_object, asset_prediction_pool_id_type, &asset_prediction_pool_object::id >
            >
         >
      >, 
      allocator< asset_prediction_pool_object >
   > asset_prediction_pool_index;


   struct by_outcome_symbol;


   typedef multi_index_container<
      asset_prediction_pool_resolution_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< asset_prediction_pool_resolution_object, asset_prediction_pool_resolution_id_type, &asset_prediction_pool_resolution_object::id > >,
         ordered_unique< tag< by_prediction_symbol >, member< asset_prediction_pool_resolution_object, asset_symbol_type, &asset_prediction_pool_resolution_object::prediction_symbol > >,
         ordered_unique< tag< by_account >,
            composite_key< asset_prediction_pool_resolution_object,
               member< asset_prediction_pool_resolution_object, account_name_type, &asset_prediction_pool_resolution_object::account >,
               member< asset_prediction_pool_resolution_object, asset_symbol_type, &asset_prediction_pool_resolution_object::prediction_symbol >
            >
         >,
         ordered_unique< tag< by_outcome_symbol >,
            composite_key< asset_prediction_pool_resolution_object,
               member< asset_prediction_pool_resolution_object, asset_symbol_type, &asset_prediction_pool_resolution_object::prediction_symbol >,
               member< asset_prediction_pool_resolution_object, asset_symbol_type, &asset_prediction_pool_resolution_object::resolution_outcome >,
               member< asset_prediction_pool_resolution_object, asset_prediction_pool_resolution_id_type, &asset_prediction_pool_resolution_object::id >
            >
         >
      >, 
      allocator< asset_prediction_pool_resolution_object >
   > asset_prediction_pool_resolution_index;

} } // node::chain


FC_REFLECT( node::chain::asset_liquidity_pool_object,
         (id)
         (symbol_a)
         (symbol_b)
         (symbol_liquid)
         (balance_a)
         (balance_b)
         (balance_liquid)
         (hour_median_price)
         (day_median_price)
         (price_history)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_liquidity_pool_object, node::chain::asset_liquidity_pool_index );

FC_REFLECT( node::chain::asset_credit_pool_object,
         (id)
         (base_symbol)
         (credit_symbol)
         (base_balance)
         (borrowed_balance)
         (credit_balance)
         (last_interest_rate)
         (last_price)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_credit_pool_object, node::chain::asset_credit_pool_index );

FC_REFLECT( node::chain::credit_collateral_object,
         (id)
         (owner)
         (symbol)
         (collateral)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::credit_collateral_object, node::chain::credit_collateral_index );   

FC_REFLECT( node::chain::credit_loan_object,
         (id)
         (owner)
         (loan_id)
         (debt)
         (interest)
         (collateral)
         (liquidation_price)
         (symbol_a)
         (symbol_b)
         (symbol_liquid)
         (last_interest_rate)
         (created)
         (last_updated)
         (last_interest_time)
         (flash_loan)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::credit_loan_object, node::chain::credit_loan_index );

FC_REFLECT( node::chain::asset_option_pool_object,
         (id)
         (base_symbol)
         (quote_symbol)
         (call_symbols)
         (call_strikes)
         (put_symbols)
         (put_strikes)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_option_pool_object, node::chain::asset_option_pool_index );

FC_REFLECT( node::chain::asset_prediction_pool_object,
         (id)
         (prediction_symbol)
         (collateral_symbol)
         (collateral_pool)
         (prediction_bond_pool)
         (outcome_assets)
         (outcome_details)
         (json)
         (url)
         (details)
         (outcome_time)
         (resolution_time)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_prediction_pool_object, node::chain::asset_prediction_pool_index );

FC_REFLECT( node::chain::asset_prediction_pool_resolution_object,
         (id)
         (account)
         (prediction_symbol)
         (resolution_outcome)
         (amount)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_prediction_pool_resolution_object, node::chain::asset_prediction_pool_resolution_index );