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
    * Contains an Asset Balance that is owned by an account.
    * 
    * Account Balances have various component balance values:
    * Liquid: Can be freely spent at any time to other accounts
    * Staked: Cannot be spent, used for voting with the asset. Must be Unstaked in order to spend.
    * Reward: Accumulates newly issued units of the asset, which must be claimed. Split with revenue sharing.
    * Savings: Cannot be spent, must be withdrawn after a 3 day delay for security.
    * Delegated: Staked balance from the owner account that is assigned via a delegation to another account for voting power.
    * Receiving: Staked balance from another account that is assigned via a delegation to this account for voting power.
    */
   class account_balance_object : public object< account_balance_object_type, account_balance_object >
   {
      account_balance_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_balance_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                     id;

         account_name_type           owner;                      ///< Account that owns the Balance.

         asset_symbol_type           symbol;                     ///< Symbol of the asset that the balance corresponds to.

         share_type                  liquid_balance;             ///< Balance that can be freely transferred.

         share_type                  staked_balance;             ///< Balance that cannot be transferred, and is vested in the account for a period of time.

         share_type                  reward_balance;             ///< Balance that is newly issued from the network.

         share_type                  savings_balance;            ///< Balance that cannot be transferred, and must be withdrawn after a delay period.

         share_type                  delegated_balance;          ///< Balance that is delegated to other accounts for voting power.

         share_type                  receiving_balance;          ///< Balance that has been delegated to the account by other delegators.

         share_type                  stake_rate;                 ///< Amount of liquid balance that is being staked from the liquid balance to the staked balance. 

         time_point                  next_stake_time;            ///< time at which the stake rate will be transferred from liquid to staked.

         share_type                  to_stake;                   ///< total amount to stake over the staking period.

         share_type                  total_staked;               ///< total amount that has been staked so far.

         share_type                  unstake_rate;               ///< Amount of staked balance that is being unstaked from the staked balance to the liquid balance.

         time_point                  next_unstake_time;          ///< time at which the unstake rate will be transferred from staked to liquid.

         share_type                  to_unstake;                 ///< total amount to unstake over the withdrawal period.

         share_type                  total_unstaked;             ///< total amount that has been unstaked so far.

         time_point                  last_interest_time;         ///< Last time that interest was compounded.

         asset                       get_liquid_balance()const
         { 
            return asset( liquid_balance, symbol );
         }

         asset                       get_reward_balance()const
         { 
            return asset( reward_balance, symbol );
         }

         asset                       get_staked_balance()const
         { 
            return asset( staked_balance, symbol );
         }

         asset                       get_savings_balance()const
         {
            return asset( savings_balance, symbol );
         }

         asset                       get_delegated_balance()const
         { 
            return asset( delegated_balance, symbol );
         }

         asset                       get_receiving_balance()const
         { 
            return asset( receiving_balance, symbol );
         }

         asset                       get_total_balance()const
         { 
            return asset( ( liquid_balance + reward_balance + staked_balance + savings_balance ), symbol );
         }

         asset                       get_voting_power()const
         { 
            return asset( ( staked_balance - delegated_balance + receiving_balance ), symbol );
         }

         void                        adjust_liquid_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol, 
               "Delta asset: ${s} is not the correct asset for account balance ${b}",("s",delta.symbol)("b",symbol) );
            liquid_balance += delta.amount;
            FC_ASSERT( liquid_balance >= 0,
               "Liquid balance cannot go below 0, balance: ${b}",("b",liquid_balance) );
         } FC_CAPTURE_AND_RETHROW( (delta) ) }

         void                        adjust_staked_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            staked_balance += delta.amount;
            FC_ASSERT( staked_balance >= 0 );
         } FC_CAPTURE_AND_RETHROW( (delta) ) }

         void                        adjust_reward_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            reward_balance += delta.amount;
            FC_ASSERT( reward_balance >= 0 );
         } FC_CAPTURE_AND_RETHROW( (delta) ) }

         void                        adjust_savings_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            savings_balance += delta.amount;
            FC_ASSERT( savings_balance >= 0 );
         } FC_CAPTURE_AND_RETHROW( (delta) ) }

         void                        adjust_delegated_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            delegated_balance += delta.amount;
            FC_ASSERT( delegated_balance >= 0 );
         } FC_CAPTURE_AND_RETHROW( (delta) ) }

         void                        adjust_receiving_balance( const asset& delta )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            receiving_balance += delta.amount;
            FC_ASSERT( receiving_balance >= 0 );
         } FC_CAPTURE_AND_RETHROW( (delta) ) }
   };


   /**
    * Holds an account's vesting balance, which cannot be spent until the vesting time is reached.
    * 
    * Vesting Balances do not confer voting power, and cannot be divested.
    */
   class account_vesting_balance_object : public object< account_vesting_balance_object_type, account_vesting_balance_object >
   {
      account_vesting_balance_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_vesting_balance_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                     id;

         account_name_type           owner;

         asset_symbol_type           symbol;                     ///< Symbol of the asset that the balance corresponds to.

         share_type                  vesting_balance;            ///< Balance that is locked until the vesting time. Cannot be used for voting.

         time_point                  vesting_time;               ///< Time at which the vesting balance will become liquid.

         asset                       get_vesting_balance()const
         { 
            return asset( vesting_balance, symbol );
         }

         void                        adjust_vesting_balance( const asset& delta, const fc::microseconds& delta_time )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            vesting_balance += delta.amount;
            vesting_time += delta_time;
            FC_ASSERT( vesting_balance >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta )( delta_time ) ) }

         void                        create_vesting_balance( const asset& delta, const time_point& time )
         { try {
            FC_ASSERT( delta.symbol == symbol );
            FC_ASSERT( time >= GENESIS_TIME );
            vesting_balance = delta.amount;
            vesting_time = time;
            FC_ASSERT( vesting_balance >= 0 );
         } FC_CAPTURE_AND_RETHROW( ( delta )( time ) ) }
   };


   /**
    * Holds an asset balance that can be spent by a public key.
    * 
    * Acts as a UTXO payment mechanism that splits balances
    * into equal chunks that can be shuffled to provide privacy.
    * 
    * Can be collected and spent into an account to retrieve the balance.
    */
   class confidential_balance_object : public object< confidential_balance_object_type, confidential_balance_object >
   {
      confidential_balance_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         confidential_balance_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                       id;

         authority                     owner;               ///< Owner Authority of the confidential balance.

         transaction_id_type           prev;                ///< Transaction ID of the transaction that created the balance.

         uint16_t                      op_in_trx;           ///< Number of the operation in the creating transaction.

         uint16_t                      index;               ///< Number of the balance created from the origin transaction.

         commitment_type               commitment;          ///< Commitment of the Balance.

         asset_symbol_type             symbol;              ///< Asset symbol of the balance.

         time_point                    created;             ///< Time that the balance was created.

         digest_type                   hash()const          ///< Hash of the balance.
         {
            digest_type::encoder enc;
            fc::raw::pack( enc, owner );
            fc::raw::pack( enc, prev );
            fc::raw::pack( enc, op_in_trx );
            fc::raw::pack( enc, index );
            fc::raw::pack( enc, commitment );
            fc::raw::pack( enc, symbol );
            fc::raw::pack( enc, created );
            return enc.result();
         }

         account_name_type             account_auth()const
         {
            return (*owner.account_auths.begin() ).first;
         }

         public_key_type               key_auth()const
         {
            return (*owner.key_auths.begin() ).first;
         }
   };
   

   /**
    * Holds an account balance that is pending withdrawal from an account's savings balance.
    * 
    * Retrieves the amount into the liquid balance of the To Account.
    */
   class savings_withdraw_object : public object< savings_withdraw_object_type, savings_withdraw_object >
   {
      savings_withdraw_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         savings_withdraw_object( Constructor&& c, allocator< Allocator > a ) :
            memo(a), 
            request_id(a)
            {
               c( *this );
            }

         id_type                 id;

         account_name_type       from;         ///< Account that is withdrawing savings balance

         account_name_type       to;           ///< Account to direct withdrawn assets to

         shared_string           memo;         ///< Reference memo for the transaction

         shared_string           request_id;   ///< uuidv4 reference of the withdrawl instance

         asset                   amount;       ///< Amount to withdraw
         
         time_point              complete;     ///< Time that the withdrawal will complete
   };


   /**
    * A route to send unstaked assets.
    */
   class unstake_asset_route_object : public object< unstake_asset_route_object_type, unstake_asset_route_object >
   {
      unstake_asset_route_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         unstake_asset_route_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       from;                  ///< Account that is unstaking the asset balance

         account_name_type       to;                    ///< Account name that receives the unstaked assets

         asset_symbol_type       symbol;                ///< Asset to be unstaked

         uint16_t                percent = 0;           ///< Percentage of unstaking asset that should be directed to this route.

         bool                    auto_stake = false;    ///< Automatically stake the asset on the receiving account
   };


   /**
    * Holds an amount of staked balance that is assigned to another account for voting power.
    * 
    * The Delegatee gains the voting power of the amount that is delegated.
    * Delegated balances can be used for temporary increases to the voting power of an account,
    * and creating new accounts with a delegated balance for initial voting power.
    * 
    * A market may emerge for renting voting power for curation benefits, or voting benefits.
    */
   class asset_delegation_object : public object< asset_delegation_object_type, asset_delegation_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         asset_delegation_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         asset_delegation_object() {}

         id_type                id;

         account_name_type      delegator;

         account_name_type      delegatee;

         asset                  amount;

         asset_symbol_type      symbol()const
         {
            return amount.symbol;
         }
   };

   /**
    * Determines the time at which a delegated balance should expire.
    * 
    * After expiration, the balance's voting power returns to the 
    * delegator accounts.
    */
   class asset_delegation_expiration_object : public object< asset_delegation_expiration_object_type, asset_delegation_expiration_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         asset_delegation_expiration_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         asset_delegation_expiration_object() {}

         id_type                id;

         account_name_type      delegator;

         account_name_type      delegatee;

         asset                  amount;

         time_point             expiration;

         asset_symbol_type      symbol()const
         {
            return amount.symbol;
         }
   };


   struct by_owner_symbol;
   struct by_owner;
   struct by_symbol_stake;
   struct by_symbol_liquid;
   struct by_symbol;
   struct by_next_stake_time;
   struct by_next_unstake_time;

   
   typedef multi_index_container <
      account_balance_object,
      indexed_by<
         ordered_unique< tag<by_id>, 
            member< account_balance_object, account_balance_id_type, &account_balance_object::id > 
         >,
         ordered_unique< tag<by_owner_symbol>,
            composite_key< account_balance_object,
               member<account_balance_object, account_name_type, &account_balance_object::owner>,
               member<account_balance_object, asset_symbol_type, &account_balance_object::symbol>
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< asset_symbol_type >
            >
         >,
         ordered_unique< tag<by_owner>,
            composite_key< account_balance_object,
               member<account_balance_object, account_name_type, &account_balance_object::owner>,
               member< account_balance_object, account_balance_id_type, &account_balance_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_balance_id_type >
            >
         >,
         ordered_unique< tag<by_symbol>,
            composite_key< account_balance_object,
               member< account_balance_object, asset_symbol_type, &account_balance_object::symbol>,
               member< account_balance_object, account_balance_id_type, &account_balance_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::less< account_balance_id_type >
            >
         >,
         ordered_unique< tag<by_symbol_stake>,
            composite_key< account_balance_object,
               member< account_balance_object, asset_symbol_type, &account_balance_object::symbol>,
               member< account_balance_object, share_type, &account_balance_object::staked_balance >,
               member< account_balance_object, account_balance_id_type, &account_balance_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::greater< share_type >,
               std::less< account_balance_id_type >
            >
         >,
         ordered_unique< tag<by_symbol_liquid>,
            composite_key< account_balance_object,
               member< account_balance_object, asset_symbol_type, &account_balance_object::symbol>,
               member< account_balance_object, share_type, &account_balance_object::liquid_balance >,
               member< account_balance_object, account_balance_id_type, &account_balance_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::greater< share_type >,
               std::less< account_balance_id_type >
            >
         >,
         ordered_unique< tag< by_next_stake_time >,
            composite_key< account_balance_object,
               member< account_balance_object, time_point, &account_balance_object::next_stake_time >,
               member< account_balance_object, account_balance_id_type, &account_balance_object::id >
            >,
            composite_key_compare<
               std::less< time_point >,
               std::less< account_balance_id_type >
            >
         >,
         ordered_unique< tag< by_next_unstake_time >,
            composite_key< account_balance_object,
               member< account_balance_object, time_point, &account_balance_object::next_unstake_time >,
               member< account_balance_object, account_balance_id_type, &account_balance_object::id >
            >,
            composite_key_compare<
               std::less< time_point >,
               std::less< account_balance_id_type >
            >
         >
      >,
      allocator< account_balance_object >
   > account_balance_index;


   struct by_vesting_time;
   struct by_owner_symbol_time;


   typedef multi_index_container <
      account_vesting_balance_object,
      indexed_by<
         ordered_unique< tag< by_id >, 
            member< account_vesting_balance_object, account_vesting_balance_id_type, &account_vesting_balance_object::id > >,
         ordered_unique< tag< by_owner_symbol_time >,
            composite_key< account_vesting_balance_object,
               member< account_vesting_balance_object, account_name_type, &account_vesting_balance_object::owner >,
               member< account_vesting_balance_object, asset_symbol_type, &account_vesting_balance_object::symbol >,
               member< account_vesting_balance_object, time_point, &account_vesting_balance_object::vesting_time >,
               member< account_vesting_balance_object, account_vesting_balance_id_type, &account_vesting_balance_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< asset_symbol_type >,
               std::less< time_point >,
               std::less< account_vesting_balance_id_type >
            >
         >,
         ordered_unique< tag< by_owner >,
            composite_key< account_vesting_balance_object,
               member< account_vesting_balance_object, account_name_type, &account_vesting_balance_object::owner >,
               member< account_vesting_balance_object, account_vesting_balance_id_type, &account_vesting_balance_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_vesting_balance_id_type >
            >
         >,
         ordered_unique< tag< by_symbol >,
            composite_key< account_vesting_balance_object,
               member< account_vesting_balance_object, asset_symbol_type, &account_vesting_balance_object::symbol >,
               member< account_vesting_balance_object, account_vesting_balance_id_type, &account_vesting_balance_object::id >
            >,
            composite_key_compare<
               std::less< asset_symbol_type >,
               std::less< account_vesting_balance_id_type >
            >
         >,
        
         ordered_unique< tag< by_vesting_time >,
            composite_key< account_vesting_balance_object,
               member< account_vesting_balance_object, time_point, &account_vesting_balance_object::vesting_time >,
               member< account_vesting_balance_object, account_vesting_balance_id_type, &account_vesting_balance_object::id >
            >,
            composite_key_compare<
               std::less< time_point >,
               std::less< account_vesting_balance_id_type >
            >
         >
      >,
      allocator< account_vesting_balance_object >
   > account_vesting_balance_index;


   struct by_key;
   struct by_hash;
   struct by_created;
   struct by_commitment;
   struct by_key_auth;
   struct by_account_auth;

   typedef multi_index_container<
      confidential_balance_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< confidential_balance_object, confidential_balance_id_type, &confidential_balance_object::id > >,
         ordered_unique< tag< by_hash >, const_mem_fun< confidential_balance_object, digest_type, &confidential_balance_object::hash > >,
         ordered_non_unique< tag< by_created >, member< confidential_balance_object, time_point, &confidential_balance_object::created > >,
         ordered_unique< tag< by_commitment >, member< confidential_balance_object, commitment_type, &confidential_balance_object::commitment > >,
         ordered_unique< tag< by_key_auth >,
            composite_key< confidential_balance_object,
               const_mem_fun< confidential_balance_object, public_key_type, &confidential_balance_object::key_auth >,
               member< confidential_balance_object, confidential_balance_id_type, &confidential_balance_object::id >
            >,
            composite_key_compare< 
               std::less< public_key_type >,
               std::less< confidential_balance_id_type >
            >
         >,
         ordered_unique< tag< by_account_auth >, 
            composite_key< confidential_balance_object,
               const_mem_fun< confidential_balance_object, account_name_type, &confidential_balance_object::account_auth >,
               member< confidential_balance_object, confidential_balance_id_type, &confidential_balance_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< confidential_balance_id_type >
            >
         >
      >,
      allocator< confidential_balance_object >
   > confidential_balance_index;


   struct by_expiration;
   struct by_request_id;
   struct by_from_account;
   struct by_end;
   struct by_begin;
   struct by_next_transfer;
   struct by_transfer_id;
   struct by_to_account;
   struct by_price;
   struct by_account;
   struct by_owner;
   struct by_conversion_date;
   struct by_volume_weight;
   struct by_withdraw_route;
   struct by_destination;


   typedef multi_index_container<
      unstake_asset_route_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< unstake_asset_route_object, unstake_asset_route_id_type, &unstake_asset_route_object::id > >,
         ordered_unique< tag< by_withdraw_route >,
            composite_key< unstake_asset_route_object,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::from >,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::to >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type > 
            >
         >,
         ordered_unique< tag< by_destination >,
            composite_key< unstake_asset_route_object,
               member< unstake_asset_route_object, account_name_type, &unstake_asset_route_object::to >,
               member< unstake_asset_route_object, unstake_asset_route_id_type, &unstake_asset_route_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< unstake_asset_route_id_type >
            >
         >
      >,
      allocator< unstake_asset_route_object >
   > unstake_asset_route_index;
   
   
   struct by_from_id;
   struct by_to;
   struct by_acceptance_time;
   struct by_dispute_release_time;
   struct by_balance;
   struct by_product_id;
   struct by_last_updated;
   struct by_request_id;
   struct by_to_complete;
   struct by_complete_from_request_id;

   typedef multi_index_container<
      savings_withdraw_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id > >,
         ordered_unique< tag< by_request_id >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, shared_string, &savings_withdraw_object::request_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_to_complete >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::to >,
               member< savings_withdraw_object, time_point,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< time_point >,
               std::less< savings_withdraw_id_type >
            >
         >,
         ordered_unique< tag< by_complete_from_request_id >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, time_point,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, shared_string, &savings_withdraw_object::request_id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< account_name_type >, 
               strcmp_less 
            >
         >
      >,
      allocator< savings_withdraw_object >
   > savings_withdraw_index;

   struct by_account;
   struct by_effective_date;
   struct by_delegator;
   struct by_delegatee;

   typedef multi_index_container <
      asset_delegation_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< asset_delegation_object, asset_delegation_id_type, &asset_delegation_object::id > >,
         ordered_unique< tag< by_delegator >,
            composite_key< asset_delegation_object,
               member< asset_delegation_object, account_name_type, &asset_delegation_object::delegator >,
               member< asset_delegation_object, account_name_type, &asset_delegation_object::delegatee >,
               const_mem_fun< asset_delegation_object, asset_symbol_type, &asset_delegation_object::symbol >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               std::less< asset_symbol_type > 
            >
         >,
         ordered_unique< tag< by_delegatee >,
            composite_key< asset_delegation_object,
               member< asset_delegation_object, account_name_type, &asset_delegation_object::delegatee >,
               member< asset_delegation_object, account_name_type, &asset_delegation_object::delegator >,
               const_mem_fun< asset_delegation_object, asset_symbol_type, &asset_delegation_object::symbol >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               std::less< asset_symbol_type > 
            >
         >
      >,
      allocator< asset_delegation_object >
   > asset_delegation_index;

   struct by_expiration;
   struct by_delegator;
   struct by_delegatee;

   typedef multi_index_container <
      asset_delegation_expiration_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< asset_delegation_expiration_object, asset_delegation_expiration_id_type, &asset_delegation_expiration_object::id > >,
         ordered_unique< tag< by_expiration >,
            composite_key< asset_delegation_expiration_object,
               member< asset_delegation_expiration_object, time_point, &asset_delegation_expiration_object::expiration >,
               member< asset_delegation_expiration_object, asset_delegation_expiration_id_type, &asset_delegation_expiration_object::id >
            >,
            composite_key_compare< 
               std::less< time_point >,
               std::less< asset_delegation_expiration_id_type >
            >
         >,
         ordered_unique< tag< by_delegator >,
            composite_key< asset_delegation_expiration_object,
               member< asset_delegation_expiration_object, account_name_type, &asset_delegation_expiration_object::delegator >,
               member< asset_delegation_expiration_object, time_point, &asset_delegation_expiration_object::expiration >,
               member< asset_delegation_expiration_object, asset_delegation_expiration_id_type, &asset_delegation_expiration_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< time_point >, 
               std::less< asset_delegation_expiration_id_type > 
            >
         >,
         ordered_unique< tag< by_delegatee >,
            composite_key< asset_delegation_expiration_object,
               member< asset_delegation_expiration_object, account_name_type, &asset_delegation_expiration_object::delegatee >,
               member< asset_delegation_expiration_object, time_point, &asset_delegation_expiration_object::expiration >,
               member< asset_delegation_expiration_object, asset_delegation_expiration_id_type, &asset_delegation_expiration_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< time_point >, 
               std::less< asset_delegation_expiration_id_type > 
            >
         >
      >,
      allocator< asset_delegation_expiration_object >
   > asset_delegation_expiration_index;


} } // node::chain


FC_REFLECT( node::chain::account_balance_object,
         (id)
         (owner)
         (symbol)
         (liquid_balance)
         (staked_balance)
         (reward_balance)
         (savings_balance)
         (delegated_balance)
         (receiving_balance)
         (stake_rate)
         (next_stake_time)
         (to_stake)
         (total_staked)
         (unstake_rate)
         (next_unstake_time)
         (to_unstake)
         (total_unstaked)
         (last_interest_time)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_balance_object, node::chain::account_balance_index );

FC_REFLECT( node::chain::account_vesting_balance_object,
         (id)
         (owner)
         (symbol)
         (vesting_balance)
         (vesting_time)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_vesting_balance_object, node::chain::account_vesting_balance_index );

FC_REFLECT( node::chain::confidential_balance_object,
         (id)
         (owner)
         (prev)
         (op_in_trx)
         (index)
         (commitment)
         (symbol)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::confidential_balance_object, node::chain::confidential_balance_index );

FC_REFLECT( node::chain::unstake_asset_route_object,
         (id)
         (from)
         (to)
         (symbol)
         (percent)
         (auto_stake)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::unstake_asset_route_object, node::chain::unstake_asset_route_index );

FC_REFLECT( node::chain::savings_withdraw_object,
         (id)
         (from)
         (to)
         (memo)
         (request_id)
         (amount)
         (complete)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::savings_withdraw_object, node::chain::savings_withdraw_index );

FC_REFLECT( node::chain::asset_delegation_object,
         (id)
         (delegator)
         (delegatee)
         (amount)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_delegation_object, node::chain::asset_delegation_index );

FC_REFLECT( node::chain::asset_delegation_expiration_object,
         (id)
         (delegator)
         (delegatee)
         (amount)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_delegation_expiration_object, node::chain::asset_delegation_expiration_index );