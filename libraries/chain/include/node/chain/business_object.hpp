#pragma once
#include <fc/fixed_string.hpp>

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <numeric>

namespace node { namespace chain {

   /**
    * A Business Entity that is created to hold the assets of a Business under its username.
    * 
    * Initiates an Equity asset, a Credit Asset, an Account and two Communities with the business object.
    * Ownership authority over all linked objects is held by the business account, and cannot be transferred.
    * Equity Holders each vote for directors.
    * The top voted (Square root of the number of equity holding accounts) directors are selected to vote for the Chief Executive.
    * The Chief Executive can update all executive appointments, salaries and roles.
    */
   class business_object : public object< business_object_type, business_object >
   {
      business_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         business_object( Constructor&& c, allocator< Allocator > a ) :
            business_trading_name(a)
            {
               c(*this);
            }

         id_type                                         id;

         account_name_type                               account;                    ///< Username of the business account, lowercase letters only.

         shared_string                                   business_trading_name;      ///< Trading business name, UPPERCASE letters only.

         asset_symbol_type                               equity_asset;               ///< Equity assets that offer dividends and voting power over the business account's structure.

         uint16_t                                        equity_revenue_share;       ///< Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.

         asset_symbol_type                               credit_asset;               ///< Credit assets that offer interest and buybacks from the business account.

         uint16_t                                        credit_revenue_share;       ///< Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.

         community_name_type                             public_community;           ///< Name of the business public community.

         community_name_type                             private_community;          ///< Name of the business private community. Should be randomized string.

         time_point                                      created;                    ///< Time that business account object was created.

         time_point                                      last_updated;               ///< Time that the business account object was last updated.
   };


   /**
    * Determines the permissioning authority over a Business.
    * 
    * Manages the Chief Executive, Executive accounts, and Directors of a Business account.
    */
   class business_permission_object : public object< business_permission_object_type, business_permission_object >
   {
      business_permission_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         business_permission_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                                         id;

         account_name_type                               account;                    ///< Username of the business account, lowercase letters only.

         flat_set< account_name_type >                   directors;                  ///< Set of all accounts currently elected to vote on behalf of the equity holders.

         flat_set< account_name_type >                   executives;                 ///< Set of all Executive accounts appointed by the Chief Executive Officer.

         account_name_type                               chief_executive;            ///< Account name of the current Chief Executive Officer.

         time_point                                      created;                    ///< Time that business premission object was created.

         time_point                                      last_updated;               ///< Time that the business permission object was last updated.

         bool                                            is_chief_executive( const account_name_type& executive ) const      ///< Finds if a given account name is in the Chief Executive.
         {
            return chief_executive == executive;
         };

         bool                                            is_executive( const account_name_type& executive ) const      ///< Finds if a given account name is in the executives set. 
         {
            return std::find( executives.begin(), executives.end(), executive ) != executives.end();
         };

         bool                                            is_director( const account_name_type& director ) const        ///< Finds if a given account name is in the directors set. 
         {
            return std::find( directors.begin(), directors.end(), director ) != directors.end();
         };
   };


   /**
    * Executive Account for a Business.
    * 
    * Executives can be appointed to the business by the Chief Executive Officer at thier discretion.
    */
   class business_executive_object : public object< business_executive_object_type, business_executive_object >
   {
      business_executive_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         business_executive_object( Constructor&& c, allocator< Allocator > a )
            {
               c(*this);
            }

         id_type                   id;

         account_name_type         executive;               ///< Username of the executive account.

         account_name_type         business;                ///< Name of the business account.

         bool                      active;                  ///< True when the executive is active and can be voted for and appointed.

         bool                      appointed;               ///< True when the executive has been appointed by the Chief Executive for active duty.

         time_point                last_updated;            ///< Time that the business executive last updated.

         time_point                created;                 ///< The time the business executive was created.
   };


   /**
    * Director Vote for an Executive Account as the Chief Executive Officer of a Business.
    */
   class business_executive_vote_object : public object< business_executive_vote_object_type, business_executive_vote_object >
   {
      business_executive_vote_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         business_executive_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                   id;

         account_name_type         director;                ///< Username of the account, voting for the executive.

         account_name_type         executive;               ///< Name of the executive account.

         account_name_type         business;                ///< Name of the referred business account.

         time_point                last_updated;            ///< Time that the vote was last updated.

         time_point                created;                 ///< The time the vote was created.
   };


   /**
    * Director Account for a business.
    * 
    * Directors can be appointed to the business by the equity holders of the business.
    */
   class business_director_object : public object< business_director_object_type, business_director_object >
   {
      business_director_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         business_director_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                   id;

         account_name_type         director;                ///< Username of the director account.

         account_name_type         business;                ///< Name of the business account.

         bool                      active;                  ///< True when the director is active and can be voted for and appointed.

         bool                      appointed;               ///< True when the director has been appointed by equity holders for active duty.

         time_point                last_updated;            ///< Time that the business director last updated.

         time_point                created;                 ///< The time the business director was created.
   };


   /**
    * Vote for a Director of a Business Account.
    * 
    * Vote ranks determine the ordering of the vote,
    * and apply higher voting power to higher ranked votes.
    */
   class business_director_vote_object : public object< business_director_vote_object_type, business_director_vote_object >
   {
      business_director_vote_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         business_director_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                    id;

         account_name_type          account;             ///< Username of the account, voting for the director.
         
         account_name_type          director;            ///< Username of the director account.

         account_name_type          business;            ///< Name of the business account.

         uint16_t                   vote_rank = 1;       ///< The rank of the officer vote.

         time_point                 last_updated;        ///< Time that the vote was last updated.

         time_point                 created;             ///< The time the vote was created.
   };


   struct by_account;

   typedef multi_index_container <
      business_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< business_object, business_id_type, &business_object::id > 
         >,
         ordered_unique< tag< by_account >,
            member< business_object, account_name_type, &business_object::account >
         >
      >,
      allocator< business_object >
   > business_index;


   typedef multi_index_container <
      business_permission_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< business_permission_object, business_permission_id_type, &business_permission_object::id > 
         >,
         ordered_unique< tag< by_account >,
            member< business_permission_object, account_name_type, &business_permission_object::account >
         >
      >,
      allocator< business_permission_object >
   > business_permission_index;


   struct by_director;
   struct by_business;
   struct by_executive;
   struct by_director_business;


   typedef multi_index_container <
      business_executive_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< business_executive_vote_object, business_executive_vote_id_type, &business_executive_vote_object::id >
         >,
         ordered_unique< tag< by_director >,
            composite_key< business_executive_vote_object,
               member< business_executive_vote_object, account_name_type, &business_executive_vote_object::director >,
               member< business_executive_vote_object, business_executive_vote_id_type, &business_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_business >,
            composite_key< business_executive_vote_object,
               member< business_executive_vote_object, account_name_type, &business_executive_vote_object::business >,
               member< business_executive_vote_object, business_executive_vote_id_type, &business_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_executive >,
            composite_key< business_executive_vote_object,
               member< business_executive_vote_object, account_name_type, &business_executive_vote_object::executive >,
               member< business_executive_vote_object, business_executive_vote_id_type, &business_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_director_business >,
            composite_key< business_executive_vote_object,
               member< business_executive_vote_object, account_name_type, &business_executive_vote_object::director >,
               member< business_executive_vote_object, account_name_type, &business_executive_vote_object::business >
            >
         >
      >,
      allocator< business_executive_vote_object >
   > business_executive_vote_index;


   struct by_business_executive;


   typedef multi_index_container <
      business_executive_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< business_executive_object, business_executive_id_type, &business_executive_object::id >
         >,
         ordered_unique< tag< by_business >,
            composite_key< business_executive_object,
               member< business_executive_object, account_name_type, &business_executive_object::business >,
               member< business_executive_object, business_executive_id_type, &business_executive_object::id >
            >
         >,
         ordered_unique< tag< by_executive >,
            composite_key< business_executive_object,
               member< business_executive_object, account_name_type, &business_executive_object::executive >,
               member< business_executive_object, business_executive_id_type, &business_executive_object::id >
            >
         >,
         ordered_unique< tag< by_business_executive >,
            composite_key< business_executive_object,
               member< business_executive_object, account_name_type, &business_executive_object::business >,
               member< business_executive_object, account_name_type, &business_executive_object::executive >
            >
         >
      >,
      allocator< business_executive_object >
   > business_executive_index;


   struct by_director;
   struct by_business;
   struct by_business_rank;
   struct by_business_director;


   typedef multi_index_container <
      business_director_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< business_director_object, business_director_id_type, &business_director_object::id > 
         >,
         ordered_unique< tag< by_business >,
            composite_key< business_director_object,
               member< business_director_object, account_name_type, &business_director_object::business >,
               member< business_director_object, business_director_id_type, &business_director_object::id >
            >
         >,
         ordered_unique< tag< by_director >,
            composite_key< business_director_object,
               member< business_director_object, account_name_type, &business_director_object::director >,
               member< business_director_object, business_director_id_type, &business_director_object::id >
            >
         >,
         ordered_unique< tag< by_business_director >,
            composite_key< business_director_object,
               member< business_director_object, account_name_type, &business_director_object::business >,
               member< business_director_object, account_name_type, &business_director_object::director >
            >
         >
      >,
      allocator< business_director_object >
   > business_director_index;


   struct by_account_business_director;
   struct by_account_business_rank;


   typedef multi_index_container <
      business_director_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< business_director_vote_object, business_director_vote_id_type, &business_director_vote_object::id > 
         >,
         ordered_unique< tag< by_business >,
            composite_key< business_director_vote_object,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::business >,
               member< business_director_vote_object, business_director_vote_id_type, &business_director_vote_object::id >
            >
         >,
         ordered_unique< tag< by_director >,
            composite_key< business_director_vote_object,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::director >,
               member< business_director_vote_object, business_director_vote_id_type, &business_director_vote_object::id >
            >
         >,
         ordered_unique< tag< by_business_rank >,
            composite_key< business_director_vote_object,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::business >,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::account >,
               member< business_director_vote_object, uint16_t, &business_director_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_business_director >,
            composite_key< business_director_vote_object,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::account >,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::business >,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::director >
            >
         >,
         ordered_unique< tag< by_business_director >,
            composite_key< business_director_vote_object,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::business >,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::director >,
               member< business_director_vote_object, business_director_vote_id_type, &business_director_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_business_rank >,
            composite_key< business_director_vote_object,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::account >,
               member< business_director_vote_object, account_name_type, &business_director_vote_object::business >,
               member< business_director_vote_object, uint16_t, &business_director_vote_object::vote_rank >
            >
         >
      >,
      allocator< business_director_vote_object >
   > business_director_vote_index;


} }   // node::chain

FC_REFLECT( node::chain::business_object,
         (id)
         (account)
         (business_trading_name)
         (equity_asset)
         (equity_revenue_share)
         (credit_asset)
         (credit_revenue_share)
         (public_community)
         (private_community)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::business_object, node::chain::business_index );

FC_REFLECT( node::chain::business_permission_object,
         (id)
         (account)
         (directors)
         (executives)
         (chief_executive)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::business_permission_object, node::chain::business_permission_index );

FC_REFLECT( node::chain::business_executive_object,
         (id)
         (executive)
         (business)
         (active)
         (appointed)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::business_executive_object, node::chain::business_executive_index );

FC_REFLECT( node::chain::business_executive_vote_object,
         (id)
         (director)
         (business)
         (executive)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::business_executive_vote_object, node::chain::business_executive_vote_index );

FC_REFLECT( node::chain::business_director_object,
         (id)
         (director)
         (business)
         (active)
         (appointed)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::business_director_object, node::chain::business_director_index );

FC_REFLECT( node::chain::business_director_vote_object,
         (id)
         (account)
         (director)
         (business)
         (vote_rank)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::business_director_vote_object, node::chain::business_director_vote_index );