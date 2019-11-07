#pragma once

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/node_object_types.hpp>

#include <boost/multi_index/composite_key.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <numeric>

namespace node { namespace chain {
   
   using node::protocol::asset;
   using node::protocol::price;

   class transfer_request_object : public object< transfer_request_object_type, transfer_request_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         transfer_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                  id;

         account_name_type        to;             // Account requesting the transfer.
      
         account_name_type        from;           // Account that is being requested to accept the transfer.
      
         asset                    amount;         // The amount of asset to transfer.

         shared_string            request_id;     // uuidv4 of the request transaction.

         shared_string            memo;           // The memo is plain-text, encryption on the memo is advised. 

         time_point               expiration;     // time that the request expires. 
   };


   class transfer_recurring_object : public object< transfer_recurring_object_type, transfer_recurring_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         transfer_recurring_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                     id;

         account_name_type           from;              // Sending account to transfer asset from.
      
         account_name_type           to;                // Recieving account to transfer asset to.
      
         asset                       amount;            // The amount of asset to transfer for each payment interval.

         shared_string               transfer_id;       // uuidv4 of the request transaction.

         shared_string               memo;              // The memo is plain-text, encryption on the memo is advised.

         time_point                  begin;             // Starting time of the first payment.

         time_point                  end;               // Ending time of the recurring payment. 

         fc::microseconds            interval;          // Microseconds between each transfer event.
   
         time_point                  next_transfer;     // Time of the next transfer.    
   };


   class transfer_recurring_request_object : public object< transfer_recurring_request_object_type, transfer_recurring_request_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         transfer_recurring_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                     id;

         account_name_type           from;              // Sending account to transfer asset from.
      
         account_name_type           to;                // Recieving account to transfer asset to.
      
         asset                       amount;            // The amount of asset to transfer for each payment interval.

         shared_string               request_id;        // uuidv4 of the request transaction.

         shared_string               memo;              // The memo is plain-text, encryption on the memo is advised.

         time_point                  begin;             // Starting time of the first payment.

         time_point                  end;               // Ending time of the recurring payment. 

         fc::microseconds            interval;          // Microseconds between each transfer event.

         time_point                  expiration;        // time that the request expires. 

         asset                       total_payment()
         {
            fc::microseconds time = end - begin;
            share_type payments = time.count / interval.count;
            return amount * payments;
         };
   };

   /**
    *  @brief An offer to sell a amount of a asset at a specified exchange rate by a certain time.
    *  @ingroup object
    *  @ingroup protocol
    *  @ingroup market
    *
    *  This limit_order_objects are indexed by @ref expiration and is automatically deleted on the first block after expiration.
    */
   class limit_order_object : public object< limit_order_object_type, limit_order_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         limit_order_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         limit_order_object(){}

         id_type                id;

         time_point             created;           // Time that the order was created.

         time_point             expiration;        // Expiration time of the order.

         account_name_type      seller;            // Selling account name of the trading order.

         shared_string          order_id;          // UUIDv4 of the order for each account.

         share_type             for_sale;          // asset symbol is sell_price.base.symbol

         price                  sell_price;        // Base price is the asset being sold.

         account_name_type      interface;         // The interface account that created the order

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return sell_price.base.symbol < sell_price.quote.symbol ?
                std::make_pair( sell_price.base.symbol, sell_price.quote.symbol ) :
                std::make_pair( sell_price.quote.symbol, sell_price.base.symbol );
         }

         asset                amount_for_sale()const   { return asset( for_sale, sell_price.base.symbol ); }

         asset                amount_to_receive()const { return amount_for_sale() * sell_price; }

         asset_symbol_type    sell_asset()const    { return sell_price.base.symbol; }

         asset_symbol_type    receive_asset()const { return sell_price.quote.symbol; }

         double               real_price()const { return sell_price.to_real(); }
   };

   /**
    * @class call_order_object
    * @brief tracks debt and call price information
    * There should only be one call_order_object per asset pair per account.
    */
   class call_order_object : public object< call_order_object_type, call_order_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         call_order_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       borrower;

         asset                   collateral;                  // call_price.base.symbol, access via get_collateral

         asset                   debt;                        // call_price.quote.symbol, access via get_debt

         price                   call_price;                  // Collateral / Debt

         optional<uint16_t>      target_collateral_ratio;     // maximum CR to maintain when selling collateral on margin call

         account_name_type       interface;                   // The interface account that created the order

         double                  real_price()const { return call_price.to_real(); }

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            auto tmp = std::make_pair( call_price.base.symbol, call_price.quote.symbol );
            if( tmp.first > tmp.second ) std::swap( tmp.first, tmp.second );
            return tmp;
         }

         asset amount_to_receive()const { return debt; }

         asset amount_for_sale()const { return collateral; }

         asset_symbol_type debt_type()const { return debt.symbol; }

         asset_symbol_type collateral_type()const { return collateral.symbol; }

         price collateralization()const { return collateral / debt; }

         /**
          *  Calculate maximum quantity of debt to cover to satisfy @ref target_collateral_ratio.
          *
          *  @param match_price the matching price if this call order is margin called
          *  @param feed_price median settlement price of debt asset
          *  @param maintenance_collateral_ratio median maintenance collateral ratio of debt asset
          *  @param maintenance_collateralization maintenance collateralization of debt asset,
          *                                       should only be valid after core-1270 hard fork
          *  @return maximum amount of debt that can be called
          */
         share_type get_max_debt_to_cover( price match_price,
                                          price feed_price,
                                          const uint16_t maintenance_collateral_ratio,
                                          const optional<price>& maintenance_collateralization = optional<price>() )const;
   };

   /**
    *  Tracks bitassets scheduled for force settlement at some point in the future.
    *  On the settlement_date the balance will be converted to the collateral asset
    *  and paid to owner and then this object will be deleted.
    */
   class force_settlement_object : public object< force_settlement_object_type, force_settlement_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         force_settlement_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      owner;

         asset                  balance;

         time_point             settlement_date;

         account_name_type      interface;         // The interface account that created the order

         asset_symbol_type      settlement_asset_symbol()const
         { return balance.symbol; }
   };

   /**
    * @class collateral_bid_object
    * @brief bids of collateral for debt after a black swan
    * There should only be one collateral_bid_object per asset per account, and
    * only for smartcoin assets that have a global settlement_price.
    */
   class collateral_bid_object : public object< collateral_bid_object_type, collateral_bid_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         collateral_bid_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type               id;

         account_name_type     bidder;      // Bidding Account name

         price                 inv_swan_price;  // Collateral / Debt

         asset get_additional_collateral()const { return inv_swan_price.base; }

         asset get_debt_covered()const { return inv_swan_price.quote; }

         asset_symbol_type debt_type()const { return inv_swan_price.quote.symbol; } 
   };


   /**
    * Credit collateral object holds assets in collateral for use to support a
    * credit borrowing order from the asset's credit pool, or a margin order by substracting collateral from 
    * the object to include in a margin order's collateral.
    */
   class credit_collateral_object : public object< credit_collateral_object_type, credit_collateral_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         credit_collateral_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;         // Collateral owners account name.

         asset_symbol_type          symbol;        // Asset symbol being collateralized. 

         asset                      collateral;    // Asset balance that is being locked in for loan backing for loan or margin orders.  
   };


   /**
    * Credit collateral object holds assets in collateral for use to support a
    * credit borrowing order from the asset's credit pool, or a margin order by substracting collateral from 
    * the object to include in a margin order's collateral.
    */
   class credit_loan_object : public object< credit_loan_object_type, credit_loan_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         credit_loan_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;                   // Collateral owner's account name

         shared_string              loan_id;                 // UUIDV4 for the loan to uniquely identify it for reference. 

         asset                      debt;                    // Amount of an asset borrowed. Limit of 75% of collateral value. Increases with interest charged.

         asset                      interest;                // Total Amount of interest accrued on the loan. 

         asset                      collateral;              // Amount of an asset to use as collateral for the loan. 

         price                      loan_price;              // Collateral / Debt. Must be higher than liquidation price to remain solvent. 

         price                      liquidation_price;       // Collateral / max_debt value. Rises when collateral/debt market price falls.

         asset_symbol_type          symbol_a;                // The symbol of asset A in the debt / collateral exchange pair.

         asset_symbol_type          symbol_b;                // The symbol of asset B in the debt / collateral exchange pair.

         share_type                 last_interest_rate;      // Updates the interest rate of the loan hourly. 

         time_point                 created;                 // Time that the loan was taken out.

         time_point                 last_updated;            // Time that the loan was last updated, and interest was accrued.

         asset_symbol_type          debt_asset()const { return debt.symbol; } 
         asset_symbol_type          collateral_asset()const { return collateral.symbol; } 
         price                      liquidation_spread()const { return loan_price - liquidation_price; }

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return debt.symbol < collateral.symbol ?
                std::make_pair( debt.symbol, collateral.symbol ) :
                std::make_pair( collateral.symbol, debt.symbol );
         }
   };


   /**
    * Margin order object creates a borrowing balance in a specific asset, then uses that asset to buy 
    * another asset, the position asset.
    */
   class margin_order_object : public object< margin_order_object_type, margin_order_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         margin_order_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;

         account_name_type          owner;                       // Margin order owners account name

         shared_string              order_id;                    // UUIDv4 Unique Identifier of the order for each account.

         price                      sell_price;                  // limit exchange price of the borrowed asset being sold for the position asset.

         asset                      collateral;                  // Collateral asset used to back the loan value; Returned to credit collateral object when position is closed. 

         asset                      debt;                        // Amount of asset borrowed to purchase the position asset. Repaid when the margin order is closed. 

         asset                      debt_balance;                // Debt asset that is held by the order when selling debt, or liquidating position.

         asset                      interest;                    // Amount of interest accrued on the borrowed asset into the debt value.

         asset                      position;                    // Minimum amount of asset to receive as margin position.

         asset                      position_filled;             // Amount of asset currently held within the order that has filled.                     

         share_type                 collateralization;           // Percentage ratio of ( Collateral + position_filled + debt_balance - debt ) / debt. Position is liquidated when ratio falls below liquidation requirement 

         account_name_type          interface;                   // The interface account that created the order.

         time_point                 created;                     // Time that the order was created.

         time_point                 last_updated;                // Time that interest was last compounded on the margin order, and collateralization was last updated. 

         time_point                 expiration;                  // Expiration time of the order.

         asset                      unrealized_value = asset(0, debt.symbol);   // Current profit or loss that the position is holding.

         share_type                 last_interest_rate = 0;      // The interest rate that was last applied to the order.

         bool                       liquidating = false;         // Set to true to place the margin order back into the orderbook and liquidate the position at sell price.

         price                      stop_loss_price = price::min(sell_price.base.symbol, sell_price.quote.symbol);          // Price at which the position will be force liquidated if it falls into a net loss.

         price                      take_profit_price = price::max(sell_price.base.symbol, sell_price.quote.symbol);         // Price at which the position will be force liquidated if it rises into a net profit.

         price                      limit_stop_loss_price = price::min(sell_price.base.symbol, sell_price.quote.symbol);     // Price at which the position will be limit liquidated if it falls into a net loss.

         price                      limit_take_profit_price = price::max(sell_price.base.symbol, sell_price.quote.symbol);   // Price at which the position will be limit liquidated if it rises into a net profit.

         pair< asset_symbol_type, asset_symbol_type > get_market()const
         {
            return sell_price.base.symbol < sell_price.quote.symbol ?
                std::make_pair( sell_price.base.symbol, sell_price.quote.symbol ) :
                std::make_pair( sell_price.quote.symbol, sell_price.base.symbol );
         }

         asset_symbol_type debt_asset()const { return debt.symbol; } 
         asset_symbol_type position_asset()const { return position.symbol; } 
         asset_symbol_type collateral_asset()const { return collateral.symbol; } 

         asset                amount_for_sale()const   
         { 
            if( liquidating )
            {
               return position_filled;
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
               return position_filled.amount == 0;    // No position left to sell
            }
            else
            {
               return debt_balance.amount == 0;     // No debt left to sell.
            }
         }

         asset_symbol_type    sell_asset()const    { return sell_price.base.symbol; }

         asset_symbol_type    receive_asset()const { return sell_price.quote.symbol; }

         double               real_price()const { return sell_price.to_real(); }
   };


   class escrow_object : public object< escrow_object_type, escrow_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         escrow_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         escrow_object(){}

         id_type                id;

         uint32_t               escrow_id = 20;

         account_name_type      from;

         account_name_type      to;

         account_name_type      agent;

         time_point             ratification_deadline;

         time_point             escrow_expiration;

         asset                  balance;

         asset                  pending_fee;
     
         bool                   to_approved = false;

         bool                   agent_approved = false;

         bool                   disputed = false;

         bool                   is_approved()const { return to_approved && agent_approved; }
   };

   class savings_withdraw_object : public object< savings_withdraw_object_type, savings_withdraw_object >
   {
      savings_withdraw_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         savings_withdraw_object( Constructor&& c, allocator< Allocator > a )
            :memo( a )
         {
            c( *this );
         }

         id_type           id;

         account_name_type from;

         account_name_type to;

         shared_string     memo;

         uint32_t          request_id = 0;

         asset             amount;
         
         time_point        complete;
   };

   /**
    *  This object gets updated once per hour, on the hour.
    */
   class feed_history_object  : public object< feed_history_object_type, feed_history_object >
   {
      feed_history_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         feed_history_object( Constructor&& c, allocator< Allocator > a )
            :price_history( a.get_segment_manager() )
         {
            c( *this );
         }

         id_type                                   id;

         price                                     current_median_history;    // the current median of the price history, used as the base for price operations
         
         bip::deque< price, allocator< price > >   price_history;             // tracks this 3.5 days of median_feed one per hour
   };

   /**
    * A route to send unstaked assets.
    */
   class unstake_asset_route_object : public object< unstake_asset_route_object_type, unstake_asset_route_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         unstake_asset_route_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         unstake_asset_route_object(){}

         id_type  id;

         account_name_type   from_account;          // Account that is unstaking the asset balance

         account_name_type   to_account;            // Account name that receives the unstaked assets

         asset_symbol_type   symbol;                // Asset to be unstaked

         uint16_t            percent = 0;           // Percentage of unstaking asset that should be directed to this route.

         bool                auto_stake = false;    // Automatically stake the asset on the receiving account
   };

   class decline_voting_rights_request_object : public object< decline_voting_rights_request_object_type, decline_voting_rights_request_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         decline_voting_rights_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         decline_voting_rights_request_object(){}

         id_type             id;

         account_name_type   account;
         
         time_point          effective_date;
   };

   enum curve_id
   {
      quadratic,                   // Returns the square of the reward, with a constant added
      quadratic_curation,          // Returns an amount converging to linear with reward
      linear,                      // Returns exactly the reward, without using constant
      square_root,                 // returns exactly the square root of reward
      convergent_semi_quadratic    // Returns an amount converging to the reward, to the power of 1.5, which decays over the time period specified
   };

   class reward_fund_object : public object< reward_fund_object_type, reward_fund_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         reward_fund_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         reward_fund_object() {}

         reward_fund_id_type     id;

         asset                   content_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   validation_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   txn_stake_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   work_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   producer_activity_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   supernode_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   power_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   community_fund_balance = asset( 0, SYMBOL_COIN );

         asset                   development_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   marketing_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   advocacy_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   activity_reward_balance = asset( 0, SYMBOL_COIN );

         asset                   premium_partners_fund_balance = asset( 0, SYMBOL_COIN );  // Receives income from memberships, distributed to premium creators. 

         asset                   total_pending_reward_balance = asset( 0, SYMBOL_COIN );

         uint128_t               total_reward_shares = 0;

         uint128_t               recent_content_claims = 0;

         uint128_t               recent_activity_claims = 0;

         uint128_t               content_constant = CONTENT_CONSTANT;

         fc::microseconds        content_reward_decay_rate = CONTENT_REWARD_DECAY_RATE;

         curve_id                author_reward_curve = convergent_semi_quadratic;

         curve_id                curation_reward_curve = convergent_semi_quadratic;

         time_point              last_update;

         void reward_fund_object::adjust_content_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            content_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_validation_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            validation_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_txn_stake_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            txn_stake_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_work_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            work_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_producer_activity_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            activity_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_supernode_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            supernode_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_power_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            power_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_community_fund_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            community_fund_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_development_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            development_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_marketing_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            marketing_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_advocacy_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            advocacy_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_activity_reward_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            activity_reward_balance += delta;
            total_pending_reward_balance += delta;
         }

         void reward_fund_object::adjust_premium_partners_fund_balance(const asset& delta)
         {
            assert(delta.symbol == SYMBOL_COIN);
            premium_partners_fund_balance += delta;
            total_pending_reward_balance += delta;
         }
   };

   struct by_expiration;
   struct by_request_id;
   struct by_from_account;

   typedef multi_index_container<
      transfer_request_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< transfer_request_object, transfer_request_id_type, &transfer_request_object::id > >,
         ordered_non_unique< tag< by_expiration >, member< transfer_request_object, time_point, &transfer_request_object::expiration > >,
         ordered_unique< tag< by_request_id >,
            composite_key< transfer_request_object,
               member< transfer_request_object, account_name_type, &transfer_request_object::to >,
               member< transfer_request_object, shared_string, &transfer_request_object::request_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         >,
         ordered_unique< tag< by_from_account >,
            composite_key< transfer_request_object,
               member< transfer_request_object, account_name_type, &transfer_request_object::from >,
               member< transfer_request_object, transfer_request_id_type, &transfer_request_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< transfer_request_id_type > >
         >
      >,
      allocator< transfer_request_object >
   > transfer_request_index;

   struct by_end;
   struct by_begin;
   struct by_next_transfer;
   struct by_transfer_id;
   struct by_to_account;

   typedef multi_index_container<
      transfer_recurring_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< transfer_recurring_object, transfer_recurring_id_type, &transfer_recurring_object::id > >,
         ordered_non_unique< tag< by_end >, member< transfer_recurring_object, time_point, &transfer_recurring_object::end > >,
         ordered_non_unique< tag< by_begin >, member< transfer_recurring_object, time_point, &transfer_recurring_object::begin > >,
         ordered_non_unique< tag< by_next_transfer >, member< transfer_recurring_object, time_point, &transfer_recurring_object::next_transfer > >,
         ordered_unique< tag< by_transfer_id >,
            composite_key< transfer_recurring_object,
               member< transfer_recurring_object, account_name_type, &transfer_recurring_object::from >,
               member< transfer_recurring_object, shared_string, &transfer_recurring_object::transfer_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         >,
         ordered_unique< tag< by_to_account >,
            composite_key< transfer_recurring_object,
               member< transfer_recurring_object, account_name_type, &transfer_recurring_object::to >,
               member< transfer_recurring_object, transfer_recurring_id_type, &transfer_recurring_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< transfer_recurring_id_type > >
         >
      >,
      allocator< transfer_recurring_object >
   > transfer_recurring_index;

   struct by_from_account;

   typedef multi_index_container<
      transfer_recurring_request_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< transfer_recurring_request_object, transfer_recurring_request_id_type, &transfer_recurring_request_object::id > >,
         ordered_non_unique< tag< by_expiration >, member< transfer_recurring_request_object, time_point, &transfer_recurring_request_object::expiration > >,
         ordered_unique< tag< by_request_id >,
            composite_key< transfer_recurring_request_object,
               member< transfer_recurring_request_object, account_name_type, &transfer_recurring_request_object::to >,
               member< transfer_recurring_request_object, shared_string, &transfer_recurring_request_object::request_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         >,
         ordered_unique< tag< by_from_account >,
            composite_key< transfer_recurring_request_object,
               member< transfer_recurring_request_object, account_name_type, &transfer_recurring_request_object::from >,
               member< transfer_recurring_request_object, transfer_recurring_request_id_type, &transfer_recurring_request_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< transfer_recurring_request_id_type > >
         >
      >,
      allocator< transfer_recurring_request_object >
   > transfer_recurring_request_index;

   struct by_price;
   
   struct by_account;
   typedef multi_index_container<
      limit_order_object,
      indexed_by<
         ordered_unique< tag< by_id >, 
            member< limit_order_object, limit_order_id_type, &limit_order_object::id > >,
         ordered_non_unique< tag< by_expiration >, 
            member< limit_order_object, time_point, &limit_order_object::expiration > >,
         ordered_unique< tag< by_price >,
            composite_key< limit_order_object,
               member< limit_order_object, price, &limit_order_object::sell_price >,
               member< limit_order_object, limit_order_id_type, &limit_order_object::id >
            >,
            composite_key_compare< 
               std::greater< price >, std::less< limit_order_id_type > 
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< limit_order_object,
               member< limit_order_object, account_name_type, &limit_order_object::seller >,
               member< limit_order_object, shared_string, &limit_order_object::order_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, strcmp_less 
            >
         >
      >,
      allocator< limit_order_object >
   > limit_order_index;

   struct by_account;
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
         ordered_unique< tag< by_price >,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, bool, &margin_order_object::filled>,
               member< margin_order_object, price, &margin_order_object::sell_price >,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id >
            >,
            composite_key_compare< std::less< bool >, std::greater< price >, std::less< margin_order_id_type > >
         >,
         ordered_unique< tag< by_account >,
            composite_key< margin_order_object,
               member< margin_order_object, account_name_type, &margin_order_object::owner >,
               member< margin_order_object, shared_string, &margin_order_object::order_id >
            >,
            composite_key_compare< std::less< account_name_type >, strcmp_less >
         >,
         ordered_unique< tag<by_debt>,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::debt_asset>,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id>
            >,
            composite_key_compare< std::less<asset_symbol_type>, std::less<margin_order_id_type> >
         >,
         ordered_unique< tag<by_position>,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::position_asset>,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id>
            >,
            composite_key_compare< std::less<asset_symbol_type>, std::less<margin_order_id_type> >
         >,
         ordered_unique< tag<by_collateral>,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::collateral_asset>,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id>
            >,
            composite_key_compare< std::less<asset_symbol_type>, std::less<margin_order_id_type> >
         >,
         ordered_unique< tag<by_debt_collateral_position>,
            composite_key< margin_order_object,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::debt_asset>,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::collateral_asset>,
               const_mem_fun< margin_order_object, asset_symbol_type, &margin_order_object::position_asset>,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id>
            >,
            composite_key_compare< std::less<asset_symbol_type>, std::less<asset_symbol_type>, std::less<asset_symbol_type>, std::less<margin_order_id_type> >
         >,
         ordered_unique< tag<by_collateralization>,
            composite_key< margin_order_object,
               member< margin_order_object, share_type, &margin_order_object::collateralization>,
               member< margin_order_object, margin_order_id_type, &margin_order_object::id>
            >,
            composite_key_compare< std::less<share_type>, std::less<margin_order_id_type> >
         >
      >,
      allocator< margin_order_object >
   > margin_order_index;

   struct by_owner;
   struct by_conversion_date;

   struct by_owner;
   struct by_volume_weight;

   struct by_withdraw_route;
   struct by_destination;
   typedef multi_index_container<
      unstake_asset_route_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< unstake_asset_route_object, unstake_asset_route_id_type, &unstake_asset_route_object::id > >,
         ordered_unique< tag< by_withdraw_route >,
            composite_key< unstake_asset_route_object,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::from_account >,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::to_account >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_name_type > >
         >,
         ordered_unique< tag< by_destination >,
            composite_key< unstake_asset_route_object,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::to_account >,
               member< unstake_asset_route_object, unstake_asset_route_id_type, &unstake_asset_route_object::id >
            >
         >
      >,
      allocator< unstake_asset_route_object >
   > unstake_asset_route_index;

   struct by_from_id;
   struct by_to;
   struct by_agent;
   struct by_ratification_deadline;
   struct by_USDbalance;
   typedef multi_index_container<
      escrow_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< escrow_object, escrow_id_type, &escrow_object::id > >,
         ordered_unique< tag< by_from_id >,
            composite_key< escrow_object,
               member< escrow_object, account_name_type,  &escrow_object::from >,
               member< escrow_object, uint32_t, &escrow_object::escrow_id >
            >
         >,
         ordered_unique< tag< by_to >,
            composite_key< escrow_object,
               member< escrow_object, account_name_type,  &escrow_object::to >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >
         >,
         ordered_unique< tag< by_agent >,
            composite_key< escrow_object,
               member< escrow_object, account_name_type,  &escrow_object::agent >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >
         >,
         ordered_unique< tag< by_ratification_deadline >,
            composite_key< escrow_object,
               const_mem_fun< escrow_object, bool, &escrow_object::is_approved >,
               member< escrow_object, time_point, &escrow_object::ratification_deadline >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >,
            composite_key_compare< std::less< bool >, std::less< time_point >, std::less< escrow_id_type > >
         >,
         ordered_unique< tag< by_USDbalance >,
            composite_key< escrow_object,
               member< escrow_object, asset, &escrow_object::balance >,
               member< escrow_object, escrow_id_type, &escrow_object::id >
            >,
            composite_key_compare< std::greater< asset >, std::less< escrow_id_type > >
         >
      >,
      allocator< escrow_object >
   > escrow_index;

   struct by_from_rid;
   struct by_to_complete;
   struct by_complete_from_rid;
   typedef multi_index_container<
      savings_withdraw_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id > >,
         ordered_unique< tag< by_from_rid >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, uint32_t, &savings_withdraw_object::request_id >
            >
         >,
         ordered_unique< tag< by_to_complete >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::to >,
               member< savings_withdraw_object, time_point,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id >
            >
         >,
         ordered_unique< tag< by_complete_from_rid >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, time_point,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, uint32_t, &savings_withdraw_object::request_id >
            >
         >
      >,
      allocator< savings_withdraw_object >
   > savings_withdraw_index;

   struct by_account;
   struct by_effective_date;
   typedef multi_index_container<
      decline_voting_rights_request_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< decline_voting_rights_request_object, decline_voting_rights_request_id_type, &decline_voting_rights_request_object::id > >,
         ordered_unique< tag< by_account >,
            member< decline_voting_rights_request_object, account_name_type, &decline_voting_rights_request_object::account >
         >,
         ordered_unique< tag< by_effective_date >,
            composite_key< decline_voting_rights_request_object,
               member< decline_voting_rights_request_object, time_point, &decline_voting_rights_request_object::effective_date >,
               member< decline_voting_rights_request_object, account_name_type, &decline_voting_rights_request_object::account >
            >,
            composite_key_compare< std::less< time_point >, std::less< account_name_type > >
         >
      >,
      allocator< decline_voting_rights_request_object >
   > decline_voting_rights_request_index;

   struct by_name;
   typedef multi_index_container<
      reward_fund_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< reward_fund_object, reward_fund_id_type, &reward_fund_object::id > >
      >,
      allocator< reward_fund_object >
   > reward_fund_index;

   struct by_collateral;
   struct by_debt;
   struct by_account;
   struct by_price;

   typedef multi_index_container<
      call_order_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member< call_order_object, call_order_id_type, &call_order_object::id > >,
         ordered_unique< tag<by_price>,
            composite_key< call_order_object,
               member< call_order_object, price, &call_order_object::call_price>,
               member< call_order_object, call_order_id_type, &call_order_object::id>
            >,
            composite_key_compare< std::less<price>, std::less<call_order_id_type> >
         >,
         ordered_unique< tag<by_debt>,
            composite_key< call_order_object,
               const_mem_fun< call_order_object, asset_symbol_type, &call_order_object::debt_type>,
               const_mem_fun< call_order_object, price, &call_order_object::collateralization >,
               member< call_order_object, call_order_id_type, &call_order_object::id>
            >,
            composite_key_compare< std::less<asset_symbol_type>, std::less<call_order_id_type> >
         >,
         ordered_unique< tag<by_account>,
            composite_key< call_order_object,
               member< call_order_object, account_name_type, &call_order_object::borrower >,
               const_mem_fun< call_order_object, asset_symbol_type, &call_order_object::debt_type>
            >
         >,
         ordered_unique< tag<by_collateral>,
            composite_key< call_order_object,
               const_mem_fun< call_order_object, price, &call_order_object::collateralization >,
               member< call_order_object, call_order_id_type, &call_order_object::id >
            >
         >
      >, 
      allocator < call_order_object >
   > call_order_index;

   struct by_expiration;
   typedef multi_index_container<
      force_settlement_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< force_settlement_object, force_settlement_id_type, &force_settlement_object::id > >,
         ordered_unique< tag<by_account>,
            composite_key< force_settlement_object,
               member<force_settlement_object, account_name_type, &force_settlement_object::owner>,
               member< force_settlement_object, force_settlement_id_type, &force_settlement_object::id >
            >
         >,
         ordered_unique< tag<by_expiration>,
            composite_key< force_settlement_object,
               const_mem_fun<force_settlement_object, asset_symbol_type, &force_settlement_object::settlement_asset_symbol>,
               member<force_settlement_object, time_point, &force_settlement_object::settlement_date>,
               member< force_settlement_object, force_settlement_id_type, &force_settlement_object::id >
            >
         >
      >,
      allocator < force_settlement_object >
   > force_settlement_index;

   typedef multi_index_container<
      collateral_bid_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member< collateral_bid_object, collateral_bid_id_type, &collateral_bid_object::id > >,
         ordered_unique< tag<by_account>,
            composite_key< collateral_bid_object,
               const_mem_fun< collateral_bid_object, asset_symbol_type, &collateral_bid_object::debt_type>,
               member< collateral_bid_object, account_name_type, &collateral_bid_object::bidder>
            >
         >,
         ordered_unique< tag<by_price>,
            composite_key< collateral_bid_object,
               const_mem_fun< collateral_bid_object, asset_symbol_type, &collateral_bid_object::debt_type>,
               member< collateral_bid_object, price, &collateral_bid_object::inv_swan_price >,
               member< collateral_bid_object, collateral_bid_id_type, &collateral_bid_object::id >
            >,
            composite_key_compare< std::less<asset_symbol_type>, std::greater<price>, std::less<collateral_bid_id_type> >
         >
      >,
      allocator < collateral_bid_object >
   > collateral_bid_index;

   struct by_owner_symbol;

   typedef multi_index_container<
      credit_collateral_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member< credit_collateral_object, credit_collateral_id_type, &credit_collateral_object::id > >,
         ordered_unique< tag<by_owner_symbol>,
            composite_key< credit_collateral_object,
               member< credit_collateral_object, account_name_type, &credit_collateral_object::owner>,
               member< credit_collateral_object, asset_symbol_type, &credit_collateral_object::symbol>
            >
         >,
         ordered_unique< tag<by_owner>,
            composite_key< credit_collateral_object,
               member< credit_collateral_object, account_name_type, &credit_collateral_object::owner>,
               member< credit_collateral_object, credit_collateral_id_type, &credit_collateral_object::id >
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

   typedef multi_index_container<
      credit_loan_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id > >,
         ordered_unique< tag<by_owner>,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner>,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less<credit_loan_id_type> 
            >
         >,
         ordered_unique< tag<by_loan_id>,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner>,
               member< credit_loan_object, shared_string, &credit_loan_object::loan_id>
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >,
         ordered_unique< tag<by_owner_price>,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner>,
               member< credit_loan_object, price, &credit_loan_object::loan_price>,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id>
            >,
            composite_key_compare< 
               std::less<account_name_type>, 
               std::less<price>, 
               std::less<credit_loan_id_type> 
            >
         >,
         ordered_unique< tag<by_owner_debt>,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner>,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id>
            >,
            composite_key_compare< 
               std::less<account_name_type>, 
               std::less<asset_symbol_type>, 
               std::less<credit_loan_id_type> 
            >
         >,
         ordered_unique< tag<by_owner_collateral>,
            composite_key< credit_loan_object,
               member< credit_loan_object, account_name_type, &credit_loan_object::owner>,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset >,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id>
            >,
            composite_key_compare< 
               std::less<account_name_type>, 
               std::less<asset_symbol_type>, 
               std::less<credit_loan_id_type> 
            >
         >,
         ordered_unique< tag<by_liquidation_spread>,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset>,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset>,
               const_mem_fun< credit_loan_object, price, &credit_loan_object::liquidation_spread>,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id>
            >,
            composite_key_compare< 
               std::less<asset_symbol_type>, 
               std::less<asset_symbol_type>, 
               std::less<price>, 
               std::less<credit_loan_id_type> 
            >
         >,
         ordered_unique< tag<by_last_updated>,
            composite_key< credit_loan_object,
               member< credit_loan_object, time_point, &credit_loan_object::last_updated>,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id>
            >,
            composite_key_compare< 
               std::greater<time_point>, 
               std::less<credit_loan_id_type> 
            >
         >,
         ordered_unique< tag<by_debt>,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::debt_asset>,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id>
            >,
            composite_key_compare< 
               std::less<asset_symbol_type>, 
               std::less<credit_loan_id_type> 
               >
         >,
         ordered_unique< tag<by_collateral>,
            composite_key< credit_loan_object,
               const_mem_fun< credit_loan_object, asset_symbol_type, &credit_loan_object::collateral_asset>,
               member< credit_loan_object, credit_loan_id_type, &credit_loan_object::id>
            >,
            composite_key_compare< std::less<asset_symbol_type>, std::less<credit_loan_id_type> >
         >
      >, 
      allocator < credit_loan_object >
   > credit_loan_index;

} } // node::chain

#include <node/chain/network_object.hpp>
#include <node/chain/comment_object.hpp>
#include <node/chain/account_object.hpp>
#include <node/chain/asset_object.hpp>
#include <node/chain/board_object.hpp>
#include <node/chain/ad_object.hpp>

FC_REFLECT_ENUM( node::chain::curve_id,
               (quadratic)
               (quadratic_curation)
               (linear)
               (square_root)
               (convergent_semi_quadratic)
               );

FC_REFLECT( node::chain::limit_order_object,
         (id)
         (created)
         (expiration)
         (seller)
         (orderid)
         (for_sale)
         (sell_price)
         (deferred_fee) 
         (deferred_paid_fee)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::limit_order_object, node::chain::limit_order_index );

FC_REFLECT( node::chain::margin_order_object,
         (id)
         (created)
         (expiration)
         (seller)
         (orderid)
         (for_sale)
         (sell_price)
         (deferred_fee) 
         (deferred_paid_fee)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::margin_order_object, node::chain::margin_order_index );

FC_REFLECT( node::chain::unstake_asset_route_object,
         (id)
         (from_account)
         (to_account)
         (symbol)
         (percent)
         (auto_stake) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::unstake_asset_route_object, node::chain::unstake_asset_route_index );

FC_REFLECT( node::chain::savings_withdraw_object,
         (id)
         (from)
         (to)
         (symbol)
         (memo)
         (request_id)
         (amount)
         (complete)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::savings_withdraw_object, node::chain::savings_withdraw_index );

FC_REFLECT( node::chain::escrow_object,
         (id)
         (escrow_id)
         (from)
         (to)
         (agent)
         (ratification_deadline)
         (escrow_expiration)
         (balance)
         (pending_fee)
         (to_approved)
         (agent_approved)
         (disputed) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::escrow_object, node::chain::escrow_index );

FC_REFLECT( node::chain::decline_voting_rights_request_object,
         (id)
         (account)
         (effective_date) 
         );
            
CHAINBASE_SET_INDEX_TYPE( node::chain::decline_voting_rights_request_object, node::chain::decline_voting_rights_request_index );

FC_REFLECT( node::chain::reward_fund_object,
         (id)
         (content_reward_balance)
         (equity_reward_balance)
         (validation_reward_balance) 
         (txn_stake_reward_balance) 
         (work_reward_balance)
         (producer_activity_reward_balance) 
         (supernode_reward_balance)
         (power_reward_balance)
         (community_fund_balance)
         (development_reward_balance)
         (marketing_reward_balance)
         (advocacy_reward_balance)
         (activity_reward_balance)
         (total_pending_reward_balance)
         (recent_content_claims)
         (last_update)
         (content_constant)
         (author_reward_curve)
         (curation_reward_curve)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::reward_fund_object, node::chain::reward_fund_index );

FC_REFLECT( node::chain::call_order_object,
         (id)
         (borrower)
         (collateral)  
         (debt)       
         (call_price)  
         (target_collateral_ratio) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::call_order_object, node::chain::call_order_index );    

FC_REFLECT( node::chain::force_settlement_object, 
         (id)
         (owner)
         (balance)
         (settlement_date)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::force_settlement_object, node::chain::force_settlement_index );

FC_REFLECT( node::chain::collateral_bid_object, 
         (id)
         (bidder)
         (inv_swan_price)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::collateral_bid_object, node::chain::collateral_bid_index );

FC_REFLECT( node::chain::credit_collateral_object,
         (id)
         (owner)
         (symbol)
         (collateral)  
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::credit_collateral_object, node::chain::credit_collateral_index );   

FC_REFLECT( node::chain::credit_loan_object,
         (id)
         (owner)
         (loan_id)
         (debt)
         (interest)
         (collateral)
         (loan_price)
         (liquidation_price)
         (symbol_a)
         (symbol_b)
         (last_interest_rate)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::credit_loan_object, node::chain::credit_loan_index );