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
    * A Governance Entity that is used for Protocol Wide content moderation and member resolutions.
    * 
    * Initiaties an Equity Asset, a Credit Asset, an Account and two communites with the Governance Object.
    * 
    * Ownership authority over all linked objects is held by the governance account, and cannot be transferred.
    * 
    * Each member gets one equal vote for a director.
    * 
    * The top voted (Square root of the number of equity holding accounts) directors are selected to vote for the Chief Executive.
    * 
    * The Chief Executive can update all executive appointments, salaries and roles.
    * 
    * Governance Accounts earn a share of the Protocol Fee Revenue paid to the Network from their members.
    * 
    * Governance Accounts also earn a share of the Protocol Currency Rewards earned by thier members.
    * 
    * Interfaces are able to select a list of governance accounts that they support on thier interface.
    */
   class governance_object : public object< governance_object_type, governance_object >
   {
      governance_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         governance_object( Constructor&& c, allocator< Allocator > a ) :
            governance_name(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              account;                    ///< The name of the account that owns the governance account.

         shared_string                  governance_name;            ///< Extended Non-consensus Governance name, UPPERCASE letters only.

         asset_symbol_type              equity_asset;               ///< Equity assets that offer dividends and voting power over the governance account's structure.

         uint16_t                       equity_revenue_share;       ///< Equity asset that the account distributes a percentage of incoming revenue to as dividend payments.

         asset_symbol_type              credit_asset;               ///< Credit assets that offer interest and buybacks from the governance account.

         uint16_t                       credit_revenue_share;       ///< Credit asset that the account uses a percentage of incoming revenue to repurchase at face value.

         community_name_type            public_community;           ///< Name of the governance public community.

         community_name_type            private_community;          ///< Name of the governance private community. Should be randomized string.

         uint32_t                       member_count;               ///< Number of members of the governance Account.

         time_point                     created;                    ///< The time the governance account was created.

         time_point                     last_updated;               ///< The time the governance account was last updated.
   };


   /**
    * Determines the permissioning authority over a governance.
    * 
    * Manages the Chief Executive, Executive accounts, and Directors of a governance account.
    */
   class governance_permission_object : public object< governance_permission_object_type, governance_permission_object >
   {
      governance_permission_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         governance_permission_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                                         id;

         account_name_type                               account;                    ///< Username of the governance account, lowercase letters only.

         flat_set< account_name_type >                   directors;                  ///< Set of all accounts currently elected to vote on behalf of the equity holders.

         flat_set< account_name_type >                   executives;                 ///< Set of all Executive accounts appointed by the Chief Executive Officer.

         account_name_type                               chief_executive;            ///< Account name of the current Chief Executive Officer.

         time_point                                      created;                    ///< Time that governance premission object was created.

         time_point                                      last_updated;               ///< Time that the governance permission object was last updated.

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
    * Executive Account for a governance.
    * 
    * Executives can be appointed to the governance by the Chief Executive Officer at thier discretion.
    */
   class governance_executive_object : public object< governance_executive_object_type, governance_executive_object >
   {
      governance_executive_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         governance_executive_object( Constructor&& c, allocator< Allocator > a )
            {
               c(*this);
            }

         id_type                   id;

         account_name_type         executive;               ///< Username of the executive account.

         account_name_type         governance;              ///< Name of the governance account.

         bool                      active;                  ///< True when the executive is active and can be voted for and appointed.

         bool                      appointed;               ///< True when the executive has been appointed by the Chief Executive for active duty.

         time_point                last_updated;            ///< Time that the governance executive was last updated.

         time_point                created;                 ///< The time the governance executive was created.
   };


   /**
    * Director Vote for an Executive Account as the Chief Executive Officer of a governance.
    */
   class governance_executive_vote_object : public object< governance_executive_vote_object_type, governance_executive_vote_object >
   {
      governance_executive_vote_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         governance_executive_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                   id;

         account_name_type         director;                ///< Username of the account, voting for the executive.

         account_name_type         executive;               ///< Name of the executive account.

         account_name_type         governance;              ///< Name of the referred governance account.

         time_point                last_updated;            ///< Time that the vote was last updated.

         time_point                created;                 ///< The time the vote was created.
   };


   /**
    * Director Account for a governance.
    * 
    * Directors can be appointed to the governance by the equity holders of the governance.
    */
   class governance_director_object : public object< governance_director_object_type, governance_director_object >
   {
      governance_director_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         governance_director_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                   id;

         account_name_type         director;                ///< Username of the director account.

         account_name_type         governance;              ///< Name of the governance account.

         bool                      active;                  ///< True when the director is active and can be voted for and appointed.

         bool                      appointed;               ///< True when the director has been appointed by equity holders for active duty.

         time_point                last_updated;            ///< Time that the governance director last updated.

         time_point                created;                 ///< The time the governance director was created.
   };


   /**
    * Vote for a Director of a Governance Account.
    * 
    * Vote ranks determine the ordering of the vote,
    * and apply higher voting power to higher ranked votes.
    */
   class governance_director_vote_object : public object< governance_director_vote_object_type, governance_director_vote_object >
   {
      governance_director_vote_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         governance_director_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         }

         id_type                    id;

         account_name_type          account;             ///< Username of the account, voting for the director.
         
         account_name_type          director;            ///< Username of the director account.

         account_name_type          governance;          ///< Name of the governance account.

         time_point                 last_updated;        ///< Time that the vote was last updated.

         time_point                 created;             ///< The time the vote was created.
   };


   /**
    * Creates a Member for a Governance account.
    * 
    * Adds the moderation tags from the Governance Account 
    * to the posts browsed by the user.
    * 
    * Provides a share of the network fees to the Governance account
    * as they are paid by the Account.
    * 
    * Governance member objects can only be made for accounts that request to become a member.
    * Governance Accounts can revoke the account's membership at any time, and members can also leave at any time.
    */
   class governance_member_object : public object< governance_member_object_type, governance_member_object >
   {
      governance_member_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         governance_member_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              governance;        ///< The name of the governance account creating the membership.

         account_name_type              account;           ///< The name of the member of the governance account.

         account_name_type              interface;         ///< Account of the interface that most recently updated the membership.

         time_point                     last_updated;      ///< Time the member was last updated.

         time_point                     created;           ///< Time the member was created.
   };


   /**
    * Requests that an account become a Member of a Governance account.
    * 
    * If the account is already a member of a Governance account,
    * the existing membership is recinded when the new membership is accepted. 
    */
   class governance_member_request_object : public object< governance_member_request_object_type, governance_member_request_object >
   {
      governance_member_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         governance_member_request_object( Constructor&& c, allocator< Allocator > a ) :
            message(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              account;           ///< The name of the account creating the request.

         account_name_type              governance;        ///< The name of the governance account to become a member of.

         account_name_type              interface;         ///< Account of the interface that most recently updated the membership.

         shared_string                  message;           ///< Encrypted message to the Governance Account team, encrypted with Governance connection key.

         time_point                     expiration;        ///< Request expiry time.

         time_point                     last_updated;      ///< Time the member request was last updated.

         time_point                     created;           ///< Time the member request was created.
   };


   /**
    * Creates a Resolution for a Governance Account.
    * 
    * Members can then vote for the resolution.
    * 
    * The Resolution is passed if more than 51% of members vote to approve it, 
    * and rejected if more than 51% of members disapprove it.
    */
   class governance_resolution_object : public object< governance_resolution_object_type, governance_resolution_object >
   {
      governance_resolution_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         governance_resolution_object( Constructor&& c, allocator< Allocator > a ) :
            resolution_id(a),
            ammendment_id(a),
            title(a),
            details(a),
            body(a),
            json(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              governance;             ///< The name of the governance account creating the resolution.

         shared_string                  resolution_id;          ///< uuidv4 referring to the resolution.

         shared_string                  ammendment_id;          ///< uuidv4 referring to the ammendment version of the resolution.

         shared_string                  title;                  ///< Title of the resolution to be voted on.

         shared_string                  details;                ///< Short description text of purpose and summary of the resolution to be voted on.

         shared_string                  body;                   ///< Text of the body of the resolution to be voted on.

         shared_string                  json;                   ///< JSON metadata of the resolution.

         account_name_type              interface;              ///< Account of the interface that most recently updated the resolution.

         time_point                     completion_time;        ///< Time the governance resolution voting completes. Fails if not approved before this time.

         time_point                     last_updated;           ///< Time the resolution was last updated.

         time_point                     created;                ///< Time the resolution was created.

         bool                           approved = false;       ///< True when the Resolution has been approved by voters.
   };


   /**
    * Creates a Vote for a Governance Account Resolution.
    * 
    * The Resolution is passed if more than 51% of members vote to approve it, 
    * and rejected if more than 51% of members disapprove it.
    */
   class governance_resolution_vote_object : public object< governance_resolution_vote_object_type, governance_resolution_vote_object >
   {
      governance_resolution_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         governance_resolution_vote_object( Constructor&& c, allocator< Allocator > a ) :
            resolution_id(a),
            ammendment_id(a)
            {
               c( *this );
            }

         id_type                        id;

         account_name_type              account;                ///< The name of the account voting on the resolution.

         account_name_type              governance;             ///< The name of the governance account that created the Resolution.

         shared_string                  resolution_id;          ///< uuidv4 referring to the resolution.

         shared_string                  ammendment_id;          ///< uuidv4 referring to the resolution ammendment, null for the initial resolution.

         account_name_type              interface;              ///< Account of the interface that most recently updated the resolution vote.

         bool                           approved;               ///< True to Approve the resolution.

         time_point                     last_updated;           ///< Time the vote was last updated.

         time_point                     created;                ///< Time the vote was created.
   };



   struct by_name;
   struct by_account;
   struct by_members;
   struct by_member_power;
   struct by_voting_power;
   struct by_vote_count;
   struct by_symbol_type_voting_power;
   struct by_symbol_type_vote_count;


   typedef multi_index_container <
      governance_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< governance_object, governance_id_type, &governance_object::id > >,
         ordered_unique< tag< by_account >,
            member< governance_object, account_name_type, &governance_object::account > >,
         ordered_unique< tag< by_members >,
            composite_key< governance_object,
               member< governance_object, uint32_t, &governance_object::member_count >,
               member< governance_object, governance_id_type, &governance_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< governance_id_type > 
            >
         >
      >,
      allocator< governance_object >
   > governance_index;


   typedef multi_index_container <
      governance_permission_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< governance_permission_object, governance_permission_id_type, &governance_permission_object::id > >,
         ordered_unique< tag< by_account >,
            member< governance_permission_object, account_name_type, &governance_permission_object::account > >
      >,
      allocator< governance_permission_object >
   > governance_permission_index;


   struct by_account_governance;
   struct by_governance_account;
   struct by_governance;
   struct by_director;
   struct by_governance;
   struct by_executive;
   struct by_director_governance;
   struct by_governance_executive;


   typedef multi_index_container <
      governance_executive_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< governance_executive_object, governance_executive_id_type, &governance_executive_object::id >
         >,
         ordered_unique< tag< by_governance >,
            composite_key< governance_executive_object,
               member< governance_executive_object, account_name_type, &governance_executive_object::governance >,
               member< governance_executive_object, governance_executive_id_type, &governance_executive_object::id >
            >
         >,
         ordered_unique< tag< by_executive >,
            composite_key< governance_executive_object,
               member< governance_executive_object, account_name_type, &governance_executive_object::executive >,
               member< governance_executive_object, governance_executive_id_type, &governance_executive_object::id >
            >
         >,
         ordered_unique< tag< by_governance_executive >,
            composite_key< governance_executive_object,
               member< governance_executive_object, account_name_type, &governance_executive_object::governance >,
               member< governance_executive_object, account_name_type, &governance_executive_object::executive >
            >
         >
      >,
      allocator< governance_executive_object >
   > governance_executive_index;



   typedef multi_index_container <
      governance_executive_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< governance_executive_vote_object, governance_executive_vote_id_type, &governance_executive_vote_object::id >
         >,
         ordered_unique< tag< by_director >,
            composite_key< governance_executive_vote_object,
               member< governance_executive_vote_object, account_name_type, &governance_executive_vote_object::director >,
               member< governance_executive_vote_object, governance_executive_vote_id_type, &governance_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_governance >,
            composite_key< governance_executive_vote_object,
               member< governance_executive_vote_object, account_name_type, &governance_executive_vote_object::governance >,
               member< governance_executive_vote_object, governance_executive_vote_id_type, &governance_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_executive >,
            composite_key< governance_executive_vote_object,
               member< governance_executive_vote_object, account_name_type, &governance_executive_vote_object::executive >,
               member< governance_executive_vote_object, governance_executive_vote_id_type, &governance_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_director_governance >,
            composite_key< governance_executive_vote_object,
               member< governance_executive_vote_object, account_name_type, &governance_executive_vote_object::director >,
               member< governance_executive_vote_object, account_name_type, &governance_executive_vote_object::governance >
            >
         >,
         ordered_unique< tag< by_governance_executive >,
            composite_key< governance_executive_vote_object,
               member< governance_executive_vote_object, account_name_type, &governance_executive_vote_object::governance >,
               member< governance_executive_vote_object, account_name_type, &governance_executive_vote_object::executive >,
               member< governance_executive_vote_object, governance_executive_vote_id_type, &governance_executive_vote_object::id >
            >
         >
      >,
      allocator< governance_executive_vote_object >
   > governance_executive_vote_index;


   struct by_director;
   struct by_governance;
   struct by_governance_rank;
   struct by_governance_director;


   typedef multi_index_container <
      governance_director_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< governance_director_object, governance_director_id_type, &governance_director_object::id > 
         >,
         ordered_unique< tag< by_governance >,
            composite_key< governance_director_object,
               member< governance_director_object, account_name_type, &governance_director_object::governance >,
               member< governance_director_object, governance_director_id_type, &governance_director_object::id >
            >
         >,
         ordered_unique< tag< by_director >,
            composite_key< governance_director_object,
               member< governance_director_object, account_name_type, &governance_director_object::director >,
               member< governance_director_object, governance_director_id_type, &governance_director_object::id >
            >
         >,
         ordered_unique< tag< by_governance_director >,
            composite_key< governance_director_object,
               member< governance_director_object, account_name_type, &governance_director_object::governance >,
               member< governance_director_object, account_name_type, &governance_director_object::director >
            >
         >
      >,
      allocator< governance_director_object >
   > governance_director_index;


   typedef multi_index_container <
      governance_director_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< governance_director_vote_object, governance_director_vote_id_type, &governance_director_vote_object::id > 
         >,
         ordered_unique< tag< by_governance >,
            composite_key< governance_director_vote_object,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::governance >,
               member< governance_director_vote_object, governance_director_vote_id_type, &governance_director_vote_object::id >
            >
         >,
         ordered_unique< tag< by_director >,
            composite_key< governance_director_vote_object,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::director >,
               member< governance_director_vote_object, governance_director_vote_id_type, &governance_director_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_governance >,
            composite_key< governance_director_vote_object,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::account >,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::governance >
            >
         >,
         ordered_unique< tag< by_governance_account >,
            composite_key< governance_director_vote_object,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::governance >,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::account > 
            >
         >,
         ordered_unique< tag< by_governance_director >,
            composite_key< governance_director_vote_object,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::governance >,
               member< governance_director_vote_object, account_name_type, &governance_director_vote_object::director >,
               member< governance_director_vote_object, governance_director_vote_id_type, &governance_director_vote_object::id >
            >
         >
      >,
      allocator< governance_director_vote_object >
   > governance_director_vote_index;


   typedef multi_index_container<
      governance_member_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< governance_member_object, governance_member_id_type, &governance_member_object::id > >,
         ordered_unique< tag< by_account >, member< governance_member_object, account_name_type, &governance_member_object::account > >,
         ordered_unique< tag< by_governance_account >,
            composite_key< governance_member_object,
               member< governance_member_object, account_name_type, &governance_member_object::governance >,
               member< governance_member_object, account_name_type, &governance_member_object::account >
            >
         >
      >,
      allocator< governance_member_object >
   > governance_member_index;


   typedef multi_index_container<
      governance_member_request_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< governance_member_request_object, governance_member_request_id_type, &governance_member_request_object::id > >,
         ordered_unique< tag< by_account >, member< governance_member_request_object, account_name_type, &governance_member_request_object::account > >,
         ordered_unique< tag< by_expiration >,
            composite_key< governance_member_request_object,
               member< governance_member_request_object, time_point, &governance_member_request_object::expiration >,
               member< governance_member_request_object, governance_member_request_id_type, &governance_member_request_object::id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< governance_member_request_id_type > 
            >
         >,
         ordered_unique< tag< by_governance_account >,
            composite_key< governance_member_request_object,
               member< governance_member_request_object, account_name_type, &governance_member_request_object::governance >,
               member< governance_member_request_object, account_name_type, &governance_member_request_object::account >
            >
         >
      >,
      allocator< governance_member_request_object >
   > governance_member_request_index;

   struct by_resolution_id;
   struct by_governance_recent;

   typedef multi_index_container<
      governance_resolution_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< governance_resolution_object, governance_resolution_id_type, &governance_resolution_object::id > >,
         ordered_unique< tag< by_resolution_id >,
            composite_key< governance_resolution_object,
               member< governance_resolution_object, account_name_type, &governance_resolution_object::governance >,
               member< governance_resolution_object, shared_string, &governance_resolution_object::resolution_id >,
               member< governance_resolution_object, shared_string, &governance_resolution_object::ammendment_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               strcmp_less,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_governance_recent >,
            composite_key< governance_resolution_object,
               member< governance_resolution_object, account_name_type, &governance_resolution_object::governance >,
               member< governance_resolution_object, time_point, &governance_resolution_object::created >,
               member< governance_resolution_object, governance_resolution_id_type, &governance_resolution_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< time_point >,
               std::less< governance_resolution_id_type >
            >
         >
      >,
      allocator< governance_resolution_object >
   > governance_resolution_index;


   struct by_account_resolution_id;
   struct by_governance_resolution_id;
   struct by_account_recent;


   typedef multi_index_container<
      governance_resolution_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< governance_resolution_vote_object, governance_resolution_vote_id_type, &governance_resolution_vote_object::id > >,
         ordered_unique< tag< by_account_resolution_id >,
            composite_key< governance_resolution_vote_object,
               member< governance_resolution_vote_object, account_name_type, &governance_resolution_vote_object::account >,
               member< governance_resolution_vote_object, account_name_type, &governance_resolution_vote_object::governance >,
               member< governance_resolution_vote_object, shared_string, &governance_resolution_vote_object::resolution_id >,
               member< governance_resolution_vote_object, shared_string, &governance_resolution_vote_object::ammendment_id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >,
               strcmp_less,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_account_recent >,
            composite_key< governance_resolution_vote_object,
               member< governance_resolution_vote_object, account_name_type, &governance_resolution_vote_object::account >,
               member< governance_resolution_vote_object, time_point, &governance_resolution_vote_object::created >,
               member< governance_resolution_vote_object, governance_resolution_vote_id_type, &governance_resolution_vote_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::greater< time_point >,
               std::less< governance_resolution_vote_id_type >
            >
         >,
         ordered_unique< tag< by_governance_resolution_id >,
            composite_key< governance_resolution_vote_object,
               member< governance_resolution_vote_object, account_name_type, &governance_resolution_vote_object::governance >,
               member< governance_resolution_vote_object, shared_string, &governance_resolution_vote_object::resolution_id >,
               member< governance_resolution_vote_object, shared_string, &governance_resolution_vote_object::ammendment_id >,
               member< governance_resolution_vote_object, governance_resolution_vote_id_type, &governance_resolution_vote_object::id >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               strcmp_less,
               strcmp_less,
               std::less< governance_resolution_vote_id_type >
            >
         >
      >,
      allocator< governance_resolution_vote_object >
   > governance_resolution_vote_index;

} }         ///< node::chain

FC_REFLECT( node::chain::governance_object,
         (id)
         (account)
         (governance_name)
         (equity_asset)
         (equity_revenue_share)
         (credit_asset)
         (credit_revenue_share)
         (public_community)
         (private_community)
         (member_count)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_object, node::chain::governance_index );

FC_REFLECT( node::chain::governance_permission_object,
         (id)
         (account)
         (directors)
         (executives)
         (chief_executive)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_permission_object, node::chain::governance_permission_index );

FC_REFLECT( node::chain::governance_executive_object,
         (id)
         (executive)
         (governance)
         (active)
         (appointed)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_executive_object, node::chain::governance_executive_index );

FC_REFLECT( node::chain::governance_executive_vote_object,
         (id)
         (director)
         (executive)
         (governance)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_executive_vote_object, node::chain::governance_executive_vote_index );

FC_REFLECT( node::chain::governance_director_object,
         (id)
         (director)
         (governance)
         (active)
         (appointed)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_director_object, node::chain::governance_director_index );

FC_REFLECT( node::chain::governance_director_vote_object,
         (id)
         (account)
         (director)
         (governance)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_director_vote_object, node::chain::governance_director_vote_index );

FC_REFLECT( node::chain::governance_member_object,
         (id)
         (governance)
         (account)
         (interface)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_member_object, node::chain::governance_member_index );

FC_REFLECT( node::chain::governance_member_request_object,
         (id)
         (account)
         (governance)
         (interface)
         (message)
         (expiration)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_member_request_object, node::chain::governance_member_request_index );

FC_REFLECT( node::chain::governance_resolution_object,
         (id)
         (governance)
         (resolution_id)
         (ammendment_id)
         (title)
         (details)
         (body)
         (json)
         (interface)
         (completion_time)
         (last_updated)
         (created)
         (approved)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_resolution_object, node::chain::governance_resolution_index );

FC_REFLECT( node::chain::governance_resolution_vote_object,
         (id)
         (account)
         (governance)
         (resolution_id)
         (ammendment_id)
         (interface)
         (approved)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::governance_resolution_vote_object, node::chain::governance_resolution_vote_index );