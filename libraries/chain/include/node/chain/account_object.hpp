#pragma once
#include <fc/fixed_string.hpp>

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/chain/witness_objects.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <numeric>

namespace node { namespace chain {

   using node::protocol::authority;

   class account_object : public object< account_object_type, account_object >
   {
      account_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                          id;

         account_name_type                name;                                  // Username of the account, lowercase letter and numbers and hyphens only.

         shared_string                    details;                               // User's account details.

         shared_string                    json;                                  // Public plaintext json information.

         shared_string                    json_private;                          // Private ciphertext json information.

         shared_string                    url;                                   // Account's external reference URL.

         account_types                    account_type;                          // Type of account, persona, profile or business.

         membership_types                 membership;                            // Level of account membership.

         public_key_type                  secure_public_key;                     // Key used for receiving incoming encrypted direct messages and key exchanges.

         public_key_type                  connection_public_key;                 // Key used for encrypting posts for connection level visibility. 

         public_key_type                  friend_public_key;                     // Key used for encrypting posts for friend level visibility.

         public_key_type                  companion_public_key;                  // Key used for encrypting posts for companion level visibility.

         shared_string                    json;                                  // Public plaintext json information.

         shared_string                    json_private;                          // Private ciphertext json information.

         comment_id_type                  pinned_comment;                        // Post pinned to the top of the account's profile. 

         account_name_type                proxy;                                 // Account that votes on behalf of this account

         flat_set< account_name_type>     proxied;                               // Accounts that have set this account to be their proxy voter.

         account_name_type                registrar;                             // The name of the account that created the account;

         account_name_type                referrer;                              // The name of the account that originally referred the account to be created;

         account_name_type                recovery_account = NULL_ACCOUNT;       // Account that can request recovery using a recent owner key if compromised.  

         account_name_type                reset_account = NULL_ACCOUNT;          // Account that has the ability to reset owner authority after specified days of inactivity.

         fc::microseconds                 reset_account_delay = fc::days(7);

         account_name_type                membership_interface = NULL_ACCOUNT;   // Account of the last interface to sell a membership to the account.

         uint16_t                         referrer_rewards_percentage = 50 * PERCENT_1; // The percentage of registrar rewards that are directed to the referrer.
         
         executive_officer_set            officers;                             // Set of officer account names and members and roles in a business account.
         
         uint32_t                         comment_count = 0;

         uint32_t                         follower_count = 0;

         uint32_t                         lifetime_vote_count = 0;

         uint32_t                         post_count = 0;

         uint16_t                         voting_power = PERCENT_100;             // current voting power of this account, falls after every vote, recovers over time.

         uint16_t                         viewing_power = PERCENT_100;             // current viewing power of this account, falls after every view, recovers over time.

         uint8_t                          savings_withdraw_requests = 0;

         uint16_t                         withdraw_routes = 0;

         share_type                       posting_rewards = 0;                      // Rewards in core asset earned from author rewards.

         share_type                       curation_rewards = 0;                     // Rewards in core asset earned from voting, shares, views, and commenting
   
         share_type                       moderation_rewards = 0;                   // Rewards in core asset from moderation rewards. 

         share_type                       total_rewards = 0;                        // Rewards in core asset earned from all reward sources.

         share_type                       author_reputation = 0;                    // 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards

         asset                            loan_default_balance = asset(0, SYMBOL_CREDIT);

         share_type                       recent_activity_claims = 0;

         uint16_t                         witnesses_voted_for = 0;

         uint16_t                         officer_votes = 0;                         // Number of network officers that the account has voted for.

         uint16_t                         executive_board_votes = 0;                 // Number of Executive boards that the account has voted for.

         uint16_t                         governance_subscriptions = 0;              // Number of governance accounts that the account subscribes to.

         uint32_t                         post_bandwidth = 0;

         uint16_t                         recurring_membership = 0;                  // Amount of months membership should be automatically renewed for on expiration

         time_point                       created;                                   // Time that the account was created.

         time_point                       membership_expiration;                     // Time that the account has its current membership subscription until.

         time_point                       last_account_update;                       // Time that the account's details were last updated.

         time_point                       last_vote_time;                            // Time that the account last voted on a comment.

         time_point                       last_view_time;                            // Time that the account last viewed a post.

         time_point                       last_post;                                 // Time that the user most recently created a comment 

         time_point                       last_root_post;                            // Time that the account last created a post.

         time_point                       last_transfer;                             // Time that the account last sent a transfer or created a trading txn. 

         time_point                       last_owner_proved; 

         time_point                       last_active_proved;

         time_point                       last_activity_reward;

         time_point                       last_account_recovery;

         time_point                       last_board_created;

         time_point                       last_asset_created;

         bool                             mined = true;

         bool                             revenue_share = false;

         bool                             can_vote = true;
   };

   /**
    * Manages the membership, officers, and executive team of a business account, 
    * implements signatory permissioning methods according to the following Table:
    * 
    *   |=======================++=================+==================+===================+
    *   |   Permissions         ||      OPEN       |      PUBLIC      |      PRIVATE      |
    *   |=======================++=================+==================+===================| 
    *   |   Interact            ||     Officer     |     Officer      |      CEO/CMO      |
    *   |=======================++=================+==================+===================|
    *   |   Create Posts        ||     Officer     |     Officer      |      CEO/CMO      |
    *   |=======================++=================+==================+===================|
    *   |   Invite+Accept       ||     Officer     |     Executive    |    CEO/COO/CAO    |
    *   |=======================++=================+==================+===================|
    *   |   Request Join        ||     Equity      |     Equity       |       None        |
    *   |=======================++=================+==================+===================|
    *   |   Transfer/Trade      ||     Officer     |     Executive    |      CEO/CFO      |
    *   |=======================++=================+==================+===================|
    *   |   Vote Executives     ||     Equity      |     Equity       |      CEO/COO      |
    *   |=======================++=================+==================+===================|
    *   |   Vote Officers       ||     Equity      |     Executive    |      CEO/COO      |
    *   |=======================++=================+==================+===================|
    *   |   Moderation          ||     Officer     |     Executive    |      CEO/CGO      |
    *   |=======================++=================+==================+===================|
    *   |   Advertising         ||     Officer     |     Executive    |      CEO/CMO      |
    *   |=======================++=================+==================+===================|
    *   |   IFace/SNode/Wit     ||     Officer     |     Executive    |      CEO/CTO      |
    *   |=======================++=================+==================+===================|
    *   |   Network Officer     ||     Officer     |     Executive    |  CEO/CDO/CMO/CAO  |
    *   |=======================++=================+==================+===================|
    *   |   Exec board          ||     Officer     |     Executive    |  CEO/CDO/CMO/CAO  |
    *   |=======================++=================+==================+===================|
    *   |   Remove Officers/BL  ||    Executive    |     Executive    |      CEO/COO      |
    *   |=======================++=================+==================+===================|
    * 
    * 
    */
   class account_business_object : public object< account_business_object_type, account_business_object >
   {
      account_business_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_business_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;                    // Username of the business account, lowercase letters only.

         business_types                          business_type;              // Type of business account, controls authorizations for transactions of different types.

         executive_officer_set                   executive_board;            // Set of highest voted executive accounts for each role.

         flat_map<account_name_type, pair< executive_types, share_type > >   executives;   // Set of all executive names.    

         flat_map<account_name_type, share_type> officers;                   // Set of all officers in the business, and their supporting voting power.

         flat_set<account_name_type>             members;                    // Set of all members of the business.

         share_type                              officer_vote_threshold;     // Amount of voting power required for an officer to be active. 

         flat_set<asset_symbol_type>             equity_assets;              // Set of equity assets that offer dividends and voting power over the business account's structure

         flat_set<asset_symbol_type>             credit_assets;              // Set of credit assets that offer interest and buybacks from the business account

         flat_map<asset_symbol_type, uint16_t>   equity_revenue_shares;      // Holds a map of all equity assets that the account shares incoming revenue with, and percentages.

         flat_map<asset_symbol_type, uint16_t>   credit_revenue_shares;      // Holds a map of all equity assets that the account shares incoming revenue with, and percentages.

         bool is_authorized_request( const account_name_type& account, const account_permission_object& obj )const      // Determines Permission to request to join.
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the board's blacklist
               }   
            }
            if( business_type == PRIVATE_BUSINESS ) // Exclusive groups do not allow join requests, invitation only. 
            {
               return false; 
            }
            else
            {
               return true;
            }
         };

         bool is_authorized_invite( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to send invites, accept join requests
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the board's blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS ) // Public groups, officers can invite.
            {
               if( officers.find( account ) != officers.end() )
               {
                  return true; // The account is in the officers list.
               }
               else
               {
                  return false; // The account is not in the officers list. 
               }  
            }
            else if( business_type == PUBLIC_BUSINESS ) // Public business, executives can invite
            {
               if( executives.find( account ) != executives.end() )
               {
                  return true; // The account is in the executives list.
               }
               else
               {
                  return false; // The account is not in the executives list. 
               }
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO and COO can invite
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account || executive_board.CHIEF_ADVOCACY_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool is_authorized_blacklist( const account_name_type& account, const account_permission_object& obj )const // Determines Permission to blacklist an account from the board. 
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the businesses blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS || business_type == PUBLIC_BUSINESS )    // Open and public business, executives can blacklist
            {
               if( executives.find( account ) != executives.end() )
               {
                  return true; // The account is in the executives list.
               }
               else
               {
                  return false; // The account is not in the executives list. 
               }  
            }
            else if( business_type == PRIVATE_BUSINESS )    // Private business, CEO can blacklist
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account )
               {
                  return true; // The account is the CEO.
               }
               else
               {
                  return false; // The account is not the CEO. 
               }
            }
         };

         bool is_authorized_transfer( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to send invites, accept join requests
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the board's blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS ) // Public groups, officers can invite.
            {
               if( officers.find( account ) != officers.end() )
               {
                  return true; // The account is in the officers list.
               }
               else
               {
                  return false; // The account is not in the officers list. 
               }  
            }
            else if( business_type == PUBLIC_BUSINESS ) // Public business, executives can transfer
            {
               if( executives.find( account ) != executives.end() )
               {
                  return true; // The account is in the executives list.
               }
               else
               {
                  return false; // The account is not in the executives list. 
               }
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO and CFO can transfer
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_FINANCIAL_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool is_authorized_general( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to make general transactions 
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the board's blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS ) // Public groups, officers authorized
            {
               if( officers.find( account ) != officers.end() )
               {
                  return true; // The account is in the officers list.
               }
               else
               {
                  return false; // The account is not in the officers list. 
               }  
            }
            else if( business_type == PUBLIC_BUSINESS ) // Public business, executives authorized
            {
               if( executives.find( account ) != executives.end() )
               {
                  return true; // The account is in the executives list.
               }
               else
               {
                  return false; // The account is not in the executives list. 
               }
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO and CFO can authorized
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool is_authorized_content( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to create content and interactions, and manage boards
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the board's blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS || business_type == PUBLIC_BUSINESS ) // Open and public business, officers can post
            {
               if( officers.find( account ) != officers.end() )
               {
                  return true; // The account is in the officers list.
               }
               else
               {
                  return false; // The account is not in the officers list. 
               }  
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO, CMO and CAO can post
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_MARKETING_OFFICER == account || executive_board.CHIEF_ADVOCACY_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool is_authorized_vote_executive( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to vote for executives
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the board's blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS  || business_type == PUBLIC_BUSINESS ) // Public and open business, all equity holders can vote for executives.
            {
               return true; // The account is in the officers list.
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO and COO can vote for executives
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool is_authorized_vote_officer( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to vote for executives
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS )    // Open business, all equity holders can vote for officers.
            {
               return true; 
            }
            else if( business_type == PUBLIC_BUSINESS ) // Public business, executives can vote for officers.
            {
               if( executives.find( account ) != executives.end() )
               {
                  return true; // The account is in the executives list.
               }
               else
               {
                  return false; // The account is not in the executives list. 
               }
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO and COO can vote for officers
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool is_authorized_governance( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to update governance and create moderation tags
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS )    // Open business, all officers can adjust governance
            {
               if( officers.find( account ) != officers.end() )
               {
                  return true; // The account is in the officers list.
               }
               else
               {
                  return false; // The account is not in the officers list. 
               }
            }
            else if( business_type == PUBLIC_BUSINESS ) // Public business, executives can adjust governance
            {
               if( executives.find( account ) != executives.end() )
               {
                  return true; // The account is in the executives list.
               }
               else
               {
                  return false; // The account is not in the executives list. 
               }
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO and CGO
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_GOVERNANCE_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool is_authorized_network( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to alter interface, supernode, network officers, witnesses, exec board
         {
            if( obj.blacklisted_accounts.size )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the blacklist
               }   
            }
            if( business_type == OPEN_BUSINESS )    // Open business, all equity holders can create network operations
            {
               if( officers.find( account ) != officers.end() )
               {
                  return true; // The account is in the officers list.
               }
               else
               {
                  return false; // The account is not in the officers list. 
               }
            }
            else if( business_type == PUBLIC_BUSINESS ) // Public business, executives can create network operations
            {
               if( executives.find( account ) != executives.end() )
               {
                  return true; // The account is in the executives list.
               }
               else
               {
                  return false; // The account is not in the executives list. 
               }
            }
            else if( business_type == PRIVATE_BUSINESS ) // Private business, CEO and CDO, CMO, CTO, CDO, and CAO can create network operations
            {
               if( executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_DEVELOPMENT_OFFICER == account || 
                  executive_board.CHIEF_MARKETING_OFFICER == account || executive_board.CHIEF_ADVOCACY_OFFICER == account ||
                  executive_board.CHIEF_TECHNOLOGY_OFFICER == account || executive_board.CHIEF_DESIGN_OFFICER == account )
               {
                  return true; // The account is the relevant executive.
               }
               else
               {
                  return false; // The account is not authorized. 
               }
            }
         };

         bool                                    is_chief( const account_name_type& executive )    // finds if a given account name is in the CEO or COO. 
         {
            return executive_board.CHIEF_EXECUTIVE_OFFICER == executive || executive_board.CHIEF_OPERATING_OFFICER == executive;
         };

         bool                                    is_executive( const account_name_type& executive )    // finds if a given account name is in the executives set. 
         {
            return std::find( executives.begin(), executives.end(), executive ) != executives.end();
         };

         bool                                    is_officer( const account_name_type& officer )    // finds if a given account name is in the officers set. 
         {
            return std::find( officers.begin(), officers.end(), officer ) != officers.end();
         };

         bool                                    is_member( const account_name_type& member )    // finds if a given account name is in the members set. 
         {
            return std::find( members.begin(), members.end(), member ) != members.end();
         };
   };

   class account_executive_vote_object : public object< account_executive_vote_object_type, account_executive_vote_object >
   {
      account_executive_vote_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_executive_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;               // Username of the account, voting for the executive
         
         account_name_type                       business_account;      // Name of the referred business account.

         account_name_type                       executive_account;     // Name of the executive account 

         executive_types                         role;                  // Role voted in favor of.

         uint16_t                                vote_rank;             // The rank of the executive vote.
   };

   class account_officer_vote_object : public object< account_officer_vote_object_type, account_officer_vote_object >
   {
      account_officer_vote_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_officer_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;             // Username of the account, voting for the officer
         
         account_name_type                       business_account;    // Name of the referred business account.

         account_name_type                       officer_account;     // Name of the officer account.

         uint16_t                                vote_rank;           // The rank of the officer vote.
   };

   class account_member_request_object : public object< account_member_request_object_type, account_member_request_object >
   {
      account_member_request_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_member_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;               // Username of the account requesting membership in the business account.
         
         account_name_type                       business_account;      // Name of the business account.

         shared_string                           message;               // Encrypted message to the business management team, encrypted with .

         time_point                              expiration;            // time that the request expires.
   };

   class account_member_invite_object : public object< account_member_invite_object_type, account_member_invite_object >
   {
      account_member_invite_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_member_invite_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;               // Username of the account creating the invitation to join the business account.
         
         account_name_type                       business_account;      // Name of the business account.

         account_name_type                       member;                // Name of the newly invited account.

         shared_string                           message;               // Encrypted message to the member, encrypted with the members secure public key.

         time_point                              expiration;            // time that the invitation expires.
   };

   class account_member_key_object : public object< account_member_key_object_type, account_member_key_object >
   {
      account_member_key_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_member_key_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;                    // Username of the account that created the key for the new member.

         account_name_type                       member;                     // Username of the newly added member, for which the key was created.
         
         account_name_type                       business_account;           // Name of the business account that the key is for.

         shared_string                           encrypted_business_key;     // Copy of the business account's private communications key, encrypted with the member's secure key . 
   };


   class account_permission_object : public object< account_permission_object_type, account_permission_object >
   {
      account_permission_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_permission_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                  id;

         account_name_type                        account;                       // Name of the account with permissions set.
   
         flat_set<account_name_type>              whitelisted_accounts;          // List of accounts that are able to send transfers to this account.

         flat_set<account_name_type>              blacklisted_accounts;          // List of accounts that are not able to recieve transfers from this account.
         
         flat_set<account_name_type>              whitelisting_accounts;         // List of accounts that have whitelisted this account.

         flat_set<account_name_type>              blacklisting_accounts;         // List of accounts that have blacklisted this account. 

         flat_set<asset_symbol_type>              whitelisted_assets;            // List of assets that the account has whitelisted to receieve transfers of. 

         flat_set<asset_symbol_type>              blacklisted_assets;            // List of assets that the account has blacklisted against incoming transfers.

         flat_set<asset_symbol_type>              whitelisting_assets;           // List of assets that the account has been whitelisted to receieve transfers of.

         flat_set<asset_symbol_type>              blacklisting_assets;           // List of assets that the account has been blacklisted against receieving transfers.
 
         bool is_authorized_asset( const asset_object& asset_obj)const           // Determines if an asset is authorized for transfer with an accounts permissions object. 
         {
            bool fast_check = !(asset_obj.options.flags & white_list);
            fast_check &= !(whitelisted_assets.size);
            fast_check &= !(blacklisted_assets.size);

            if( fast_check )
               return true; // The asset does not require transfer permission, and the account does not use an asset whitelist or blacklist

            if( whitelisted_assets.size )
            {
               if( whitelisted_assets.find( asset_obj.symbol ) == whitelisted_assets.end() )
                  return false; // The asset is not in the account's whitelist
            }

            if( blacklisted_assets.size )
            {
               if( blacklisted_assets.find( asset_obj.symbol ) != blacklisted_assets.end() )
                  return false; // The asset is in the account's blacklist
            }

            if( blacklisting_assets.size )
            {
               if( blacklisting_assets.find( asset_obj.symbol ) != blacklisting_assets.end() ) 
               {
                  return false; // The account is in the asset's blacklist
               }   
            }

            if( asset_obj.options.whitelist_authorities.size() == 0 ) 
            {
               return true; // The asset has an empty whitelist. 
            }
               
            if( whitelisting_assets.size )
            {
               if( whitelisting_assets.find( asset_obj.symbol ) != whitelisting_assets.end() )
                  return true; // The asset uses a whitelist, and this account is in the whitelist.
            }

            return false; // Account uses a whitelist and the user is not in the whitelist. 
         };
   };


   class account_balance_object : public object< account_balance_object_type, account_balance_object >
   {
      account_balance_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_balance_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         account_name_type       owner;

         asset_symbol_type       symbol;

         share_type              liquid_balance;             // Balance that can be freely transferred.

         share_type              staked_balance;             // Balance that cannot be transferred, and is vested in the account for a period of time.

         share_type              reward_balance;             // Balance that is newly issued from the network.

         share_type              savings_balance;            // Balance that cannot be transferred, and must be withdrawn after a delay period. 

         share_type              delegated_balance;          // Balance that is delegated to other accounts for voting power.

         share_type              receiving_balance;          // Balance that has been delegated to the account by other delegators. 

         share_type              total_balance;              // The total of all balances

         share_type              stake_rate;                 // Amount of liquid balance that is being staked from the liquid balance to the staked balance.  

         time_point              next_stake_time;            // time at which the stake rate will be transferred from liquid to staked. 

         share_type              to_stake;                   // total amount to stake over the staking period. 

         share_type              total_staked;               // total amount that has been staked so far. 

         share_type              unstake_rate;               // Amount of staked balance that is being unstaked from the staked balance to the liquid balance.  

         time_point              next_unstake_time;          // time at which the unstake rate will be transferred from staked to liquid. 

         share_type              to_unstake;                 // total amount to unstake over the withdrawal period. 

         share_type              total_unstaked;             // total amount that has been unstaked so far. 

         time_point              last_interest_time;         // Last time that interest was compounded.

         bool                    maintenance_flag = false;   // Whether need to process this balance object in maintenance interval

         asset get_liquid_balance()const { return asset(liquid_balance, symbol); }

         asset get_reward_balance()const { return asset(reward_balance, symbol); }

         asset get_staked_balance()const { return asset(staked_balance, symbol); }

         asset get_savings_balance()const { return asset(savings_balance, symbol); }

         asset get_delegated_balance()const { return asset(delegated_balance, symbol); }

         asset get_receiving_balance()const { return asset(receiving_balance, symbol); }

         asset get_total_balance()const { return asset(total_balance, symbol); }

         asset get_voting_power()const { return asset((staked_balance - delegated_balance + receiving_balance), symbol); }

         void account_balance_object::adjust_liquid_balance(const asset& delta)
         {
            assert(delta.symbol == symbol);
            liquid_balance += delta.amount;
            total_balance += delta.amount;
            if( delta.symbol == SYMBOL_COIN ) // CORE asset
               maintenance_flag = true;
         }

         void account_balance_object::adjust_reward_balance(const asset& delta)
         {
            assert(delta.symbol == symbol);
            reward_balance += delta.amount;
            total_balance += delta.amount;
            if( delta.symbol == SYMBOL_COIN ) // CORE asset
               maintenance_flag = true;
         }

         void account_balance_object::adjust_staked_balance(const asset& delta)
         {
            assert(delta.symbol == symbol);
            staked_balance += delta.amount;
            total_balance += delta.amount;
            if( delta.symbol == SYMBOL_COIN ) // CORE asset
               maintenance_flag = true;
         }

         void account_balance_object::adjust_savings_balance(const asset& delta)
         {
            assert(delta.symbol == symbol);
            savings_balance += delta.amount;
            total_balance += delta.amount;
            if( delta.symbol == SYMBOL_COIN ) // CORE asset
               maintenance_flag = true;
         }

         void account_balance_object::adjust_delegated_balance(const asset& delta)
         {
            assert(delta.symbol == symbol);
            delegated_balance += delta.amount;
            if( delta.symbol == SYMBOL_COIN ) // CORE asset
               maintenance_flag = true;
         }

         void account_balance_object::adjust_receiving_balance(const asset& delta)
         {
            assert(delta.symbol == symbol);
            receiving_balance += delta.amount;
            if( delta.symbol == SYMBOL_COIN ) // CORE asset
               maintenance_flag = true;
         }
   };


   class account_authority_object : public object< account_authority_object_type, account_authority_object >
   {
      account_authority_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_authority_object( Constructor&& c, allocator< Allocator > a )
            : owner( a ), active( a ), posting( a )
         {
            c( *this );
         }

         id_type                  id;

         account_name_type        account;            // Name of the account

         shared_authority         owner;              // used for backup control, can set owner or active

         shared_authority         active;             // used for all monetary operations, can set active or posting

         shared_authority         posting;            // used for voting and posting

         time_point               last_owner_update;  // Time that the owner key was last updated.
   };


   class account_following_object : public object< account_following_object_type, account_following_object >
   {
      account_following_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_following_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                           id;

         account_name_type                 account;              // Name of the account.

         flat_set< account_name_type >     followers;            // Accounts that follow this account.

         flat_set< account_name_type >     following;            // Accounts that this account follows.

         flat_set< account_name_type >     mutual_followers;     // Accounts that are both following and followers of this account.

         flat_set< account_name_type >     connections;          // Account that are connections of this account.

         flat_set< account_name_type >     friends;              // Accounts that are friends of this account.

         flat_set< account_name_type >     companions;           // Accounts that are companions of this account. 

         flat_set< board_name_type >       followed_boards;      // Boards that the account subscribes to. 

         flat_set< tag_name_type >         followed_tags;        // Tags that the account follows. 

         flat_set< account_name_type >     filtered;             // Accounts that this account has filtered. Interfaces should not show posts by these users.

         flat_set< board_name_type >       filtered_boards;      // Boards that this account has filtered. Posts will not display if they are in these boards.

         flat_set< tag_name_type >         filtered_tags;        // Tags that this account has filtered. Posts will not display if they have any of these tags. 

         time_point                        last_update;          // Last time that the account changed its following sets.

         bool                              is_connection( const account_name_type& account )const
         {
            return std::find( connections.begin(), connections.end(), account ) != connections.end();
         };

         bool                              is_friend( const account_name_type& account )const
         {
            return std::find( friends.begin(), friends.end(), account ) != friends.end();
         };

         bool                              is_companion( const account_name_type& account )const
         {
            return std::find( companions.begin(), companions.end(), account ) != companions.end();
         };

         bool                              is_follower( const account_name_type& account )const
         {
            return std::find( followers.begin(), followers.end(), account ) != followers.end();
         };

         bool                              is_following( const account_name_type& account )const
         {
            return std::find( following.begin(), following.end(), account ) != following.end();
         };

         bool                              is_following( const tag_name_type& tag )const
         {
            return std::find( followed_tags.begin(), followed_tags.end(), tag ) != followed_tags.end();
         };

         bool                              is_following( const board_name_type& board )const
         {
            return std::find( followed_boards.begin(), followed_boards.end(), board ) != followed_boards.end();
         };

         bool                              is_mutual( const account_name_type& account )const
         {
            return std::find( mutual_followers.begin(), mutual_followers.end(), account ) != mutual_followers.end();
         };

         bool                              is_filtered( const account_name_type& account )const
         {
            return std::find( filtered.begin(), filtered.end(), account ) != filtered.end();
         };

         bool                              is_filtered( const tag_name_type& tag )const
         {
            return std::find( filtered_tags.begin(), filtered_tags.end(), tag ) != filtered_tags.end();
         };

         bool                              is_filtered( const board_name_type& board )const
         {
            return std::find( filtered_boards.begin(), filtered_boards.end(), board ) != filtered_boards.end();
         };

         void                              add_follower( const account_name_type& account )
         {
            if( !is_follower( account ) )
            {
               followers.insert( account );
            }
            if( is_following( account ) )
            {
               mutual_followers.insert( account );
            }
         }

         void                              remove_follower( const account_name_type& account )
         {
            if( is_follower( account ) )
            {
               if( is_mutual( account ) )
               {
                  mutual_followers.erase( account );
               }
               followers.erase( account );
            }
         }

         void                              add_following( const account_name_type& account )
         {
            if( !is_following( account ) )
            {
               following.insert( account );
            }
            if( is_follower( account ) )
            {
               mutual_followers.insert( account );
            }
         }

         void                              add_following( const tag_name_type& tag )
         {
            if( !is_following( tag ) )
            {
               followed_tags.insert( tag );
            }
         }

         void                              add_following( const board_name_type& board )
         {
            if( !is_following( board ) )
            {
               followed_boards.insert( board );
            }
         }

         void                              remove_following( const tag_name_type& tag )
         {
            if( is_following( tag ) )
            {
               followed_tags.erase( tag );
            }
         }

         void                              remove_following( const board_name_type& board )
         {
            if( is_following( board ) )
            {
               followed_boards.erase( board );
            }
         }

         void                              remove_following( const account_name_type& account )
         {
            if( is_following( account ) )
            {
               if( is_mutual( account ) )
               {
                  mutual_followers.erase( account );
               }
               following.erase( account );
            }
         }

         void                              add_filtered( const account_name_type& account )
         {
            if( !is_filtered( account ) )
            {
               filtered.insert( account );
            }
         }

         void                              remove_filtered( const account_name_type& account )
         {
            if( is_filtered( account ) )
            {
               filtered.erase( account );
            }
         }

         void                              add_filtered( const tag_name_type& tag )
         {
            if( !is_filtered( tag ) )
            {
               filtered_tags.insert( tag );
            }
         }

         void                              remove_filtered( const tag_name_type& tag )
         {
            if( is_filtered( tag ) )
            {
               filtered_tags.erase( tag );
            }
         }

         void                              add_filtered( const const board_name_type& board )
         {
            if( !is_filtered( board ) )
            {
               filtered_boards.insert( board );
            }
         }

         void                              remove_filtered( const board_name_type& board )
         {
            if( is_filtered( board ) )
            {
               filtered_boards.erase( board );
            }
         }

   };


   class tag_following_object : public object< tag_following_object_type, tag_following_object >
   {
      tag_following_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         tag_following_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                           id;

         tag_name_type                     tag;                  // Name of the account.

         flat_set< account_name_type >     followers;            // Accounts that follow this account. 

         time_point                        last_update;          // Last time that the tag changed its following sets.

         bool                              is_follower( const account_name_type& account )
         {
            return std::find( followers.begin(), followers.end(), account ) != followers.end();
         };

         void                              add_follower( const account_name_type& account )
         {
            if( !is_follower( account ) )
            {
               followers.insert( account );
            }
         }

         void                              remove_follower( const account_name_type& account )
         {
            if( is_follower( account ) )
            {
               followers.erase( account );
            }
         }
   };


   class connection_request_object : public object< connection_request_object_type, connection_request_object >
   {
      connection_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         connection_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;                 

         account_id_type        account;            // Account that created the request

         account_id_type        requested_account;  

         connection_types       connection_type;

         shared_string          message;

         time_point             expiration;

   };


   class connection_object : public object< connection_object_type, connection_object >
   {
      connection_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         connection_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;                 

         account_name_type      account_a;                // Account with the lower ID.

         shared_string          encrypted_key_a;          // A's private connection key, encrypted with the public secure key of account B.

         account_name_type      account_b;                // Account with the greater ID.

         shared_string          encrypted_key_b;          // B's private connection key, encrypted with the public secure key of account A.

         connection_types       connection_type;          // The connection level shared in this object

         shared_string          connection_id;            // Unique uuidv4 for the connection, for local storage of decryption key.

         uint32_t               connection_strength = 0;  // Number of total messages sent between connections

         uint32_t               consecutive_days = 0;     // Number of consecutive days that the connected accounts have both sent a message.

         time_point             last_message_time_a;      // Time since the account A last sent a message

         time_point             last_message_time_b;      // Time since the account B last sent a message

         time_point             created;                  // Time the connection was created. 

         time_point             last_message_time()const
         {
            return std::min( last_message_time_a, last_message_time_b );
         }
   };


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

         time_point             min_delegation_time;
   };


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

         asset                  amount;

         time_point             expiration;
   };


   class owner_authority_history_object : public object< owner_authority_history_object_type, owner_authority_history_object >
   {
      owner_authority_history_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         owner_authority_history_object( Constructor&& c, allocator< Allocator > a )
            :previous_owner_authority( shared_authority::allocator_type( a.get_segment_manager() ) )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      account;

         shared_authority       previous_owner_authority;

         time_point             last_valid_time;
   };


   class account_recovery_request_object : public object< account_recovery_request_object_type, account_recovery_request_object >
   {
      account_recovery_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_recovery_request_object( Constructor&& c, allocator< Allocator > a )
            :new_owner_authority( shared_authority::allocator_type( a.get_segment_manager() ) )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      account_to_recover;

         shared_authority       new_owner_authority;

         time_point             expiration;
   };


   class change_recovery_account_request_object : public object< change_recovery_account_request_object_type, change_recovery_account_request_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         change_recovery_account_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      account_to_recover;

         account_name_type      recovery_account;
         
         time_point             effective_on;
   };


   struct by_name;
   struct by_proxy;
   struct by_last_post;
   struct by_next_stake_time;
   struct by_next_unstake_time;
   struct by_post_count;
   struct by_vote_count;
   struct by_follower_count;
   struct by_subscribers;
   struct by_subscriber_power;
   struct by_membership_expiration;
   struct by_total_rewards;

   typedef multi_index_container<
      account_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< account_object, account_id_type, &account_object::id > >,
         ordered_unique< tag< by_name >,
            member< account_object, account_name_type, &account_object::name > >,
         ordered_non_unique< tag< by_membership_expiration >,
            member< account_object, time_point, &account_object::membership_expiration > >,
         ordered_unique< tag< by_proxy >,
            composite_key< account_object,
               member< account_object, account_name_type, &account_object::proxy >,
               member< account_object, account_id_type, &account_object::id >
            >
         >,
         ordered_unique< tag< by_total_rewards >,
            composite_key< account_object,
               member< account_object, share_type, &account_object::total_rewards >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< 
               std::greater< share_type >, 
               std::less< account_id_type > 
            >
         >,
         ordered_unique< tag< by_last_post >,
            composite_key< account_object,
               member< account_object, time_point, &account_object::last_post >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< account_id_type > 
            >
         >,
         ordered_unique< tag< by_post_count >,
            composite_key< account_object,
               member< account_object, uint32_t, &account_object::post_count >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< account_id_type > 
            >
         >,
         ordered_unique< tag< by_follower_count >,
            composite_key< account_object,
               member< account_object, uint32_t, &account_object::follower_count >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< account_id_type > 
            >
         >,
         ordered_unique< tag< by_vote_count >,
            composite_key< account_object,
               member< account_object, uint32_t, &account_object::lifetime_vote_count >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< account_id_type > 
            >
         >
      >,
      allocator< account_object >
   > account_index;

   struct by_account;
   struct by_last_valid;

   typedef multi_index_container <
      owner_authority_history_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< owner_authority_history_object, owner_authority_history_id_type, &owner_authority_history_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< owner_authority_history_object,
               member< owner_authority_history_object, account_name_type, &owner_authority_history_object::account >,
               member< owner_authority_history_object, time_point, &owner_authority_history_object::last_valid_time >,
               member< owner_authority_history_object, owner_authority_history_id_type, &owner_authority_history_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< time_point >, 
               std::less< owner_authority_history_id_type > >
         >
      >,
      allocator< owner_authority_history_object >
   > owner_authority_history_index;


   typedef multi_index_container <
      account_business_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_business_object, account_business_id_type, &account_business_object::id > 
         >,
         ordered_unique< tag< by_account >,
            member< account_business_object, account_name_type, &account_business_object::account >
         >
      >,
      allocator< account_business_object >
   > account_business_index;


   struct by_business;
   struct by_account_business_executive;
   struct by_account_business_officer;
   struct by_business_role_executive;
   struct by_account_business_role_rank;
   struct by_account_business_role_executive;
   struct by_business_account_role_rank;

   typedef multi_index_container <
      account_executive_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_executive_vote_object, account_executive_vote_id_type, &account_executive_vote_object::id >
         >,
         ordered_unique< tag< by_business >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, account_executive_vote_id_type, &account_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_business_role_executive >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, executive_types, &account_executive_vote_object::role >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::executive_account >,
               member< account_executive_vote_object, account_executive_vote_id_type, &account_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_business_executive >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::account >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::executive_account >,
               member< account_executive_vote_object, account_executive_vote_id_type, &account_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_business_role_rank >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::account >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, executive_types, &account_executive_vote_object::role >,
               member< account_executive_vote_object, uint16_t, &account_executive_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_business_role_rank >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::account >,
               member< account_executive_vote_object, executive_types, &account_executive_vote_object::role >,
               member< account_executive_vote_object, uint16_t, &account_executive_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_business_role_executive >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::account >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, executive_types, &account_executive_vote_object::role >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::executive_account >
            >
         >
      >,
      allocator< account_executive_vote_object >
   > account_executive_vote_index;

   struct by_account_business_officer;
   struct by_account_business_rank;
   struct by_business_officer;
   struct by_business_account_rank;
   
   typedef multi_index_container <
      account_officer_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_officer_vote_object, account_officer_vote_id_type, &account_officer_vote_object::id > 
         >,
         ordered_unique< tag< by_business >,
            composite_key< account_executive_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, account_officer_vote_id_type, &account_officer_vote_object::id >
            >
         >,
         ordered_unique< tag< by_business_account_rank >,
            composite_key< account_executive_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::account >,
               member< account_officer_vote_object, uint16_t, &account_officer_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_business_officer >,
            composite_key< account_executive_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::officer_account >
            >
         >,
         ordered_unique< tag< by_business_officer >,
            composite_key< account_executive_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::officer_account >,
               member< account_officer_vote_object, account_officer_vote_id_type, &account_officer_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_business_rank >,
            composite_key< account_officer_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, uint16_t, &account_officer_vote_object::vote_rank >
            >
         >
      >,
      allocator< account_officer_vote_object >
   > account_officer_vote_index;

   struct by_account_business;
   struct by_member_business;
   struct by_expiration;

   typedef multi_index_container <
      account_member_request_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_member_request_object, account_member_request_id_type, &account_member_request_object::id > 
         >,
         ordered_non_unique< tag< by_expiration >,
            member< account_member_request_object, time_point, &account_member_request_object::expiration > 
         >,
         ordered_unique< tag< by_account_business >,
            composite_key< account_member_request_object,
               member< account_member_request_object, account_name_type, &account_member_request_object::account >,
               member< account_member_request_object, account_name_type, &account_member_request_object::business_account >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >
            >
         >
      >,
      allocator< account_member_request_object >
   > account_member_request_index;


   typedef multi_index_container <
      account_member_invite_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_member_invite_object, account_member_invite_id_type, &account_member_invite_object::id >
         >,
         ordered_non_unique< tag< by_expiration >,
            member< account_member_invite_object, time_point, &account_member_invite_object::expiration > 
         >,
         ordered_unique< tag< by_account_business >,
            composite_key< account_member_invite_object,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::account >,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::business_account >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_member_business >,
            composite_key< account_member_invite_object,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::member >,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::business_account >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< account_name_type >
            >
         >
      >,
      allocator< account_member_invite_object >
   > account_member_invite_index;



   typedef multi_index_container <
      account_member_key_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_member_key_object, account_member_key_id_type, &account_member_key_object::id > 
         >,
         ordered_unique< tag< by_account >,
            member< account_member_key_object, account_name_type, &account_member_key_object::account > 
         >
      >,
      allocator< account_member_key_object >
   > account_member_key_index;


   struct by_last_owner_update;
   struct by_owner_symbol;
   struct by_maintenance_flag;
   struct by_symbol_stake;
   struct by_symbol_liquid;
   struct by_symbol;
   
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
         >,
         ordered_non_unique< tag<by_maintenance_flag>,
            member< account_balance_object, bool, &account_balance_object::maintenance_flag > 
         >
      >,
      allocator< account_balance_object >
   > account_balance_index;

   struct by_name;

   typedef multi_index_container <
      account_authority_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_authority_object, account_authority_id_type, &account_authority_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_authority_object,
               member< account_authority_object, account_name_type, &account_authority_object::account >,
               member< account_authority_object, account_authority_id_type, &account_authority_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_authority_id_type > >
         >,
         ordered_unique< tag< by_last_owner_update >,
            composite_key< account_authority_object,
               member< account_authority_object, time_point, &account_authority_object::last_owner_update >,
               member< account_authority_object, account_authority_id_type, &account_authority_object::id >
            >,
            composite_key_compare< std::greater< time_point >, std::less< account_authority_id_type > >
         >
      >,
      allocator< account_authority_object >
   > account_authority_index;

   typedef multi_index_container <
      account_permission_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_permission_object, account_permission_id_type, &account_permission_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_permission_object,
               member< account_permission_object, account_name_type, &account_permission_object::account >,
               member< account_permission_object, account_permission_id_type, &account_permission_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_permission_id_type > >
         >
      >,
      allocator< account_permission_object >
   > account_permission_index;

   typedef multi_index_container <
      account_following_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_following_object, account_following_id_type, &account_following_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_following_object,
               member< account_following_object, account_name_type, &account_following_object::account >,
               member< account_following_object, account_following_id_type, &account_following_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_following_id_type > >
         >
      >,
      allocator< account_following_object >
   > account_following_index;

   struct by_tag;

   typedef multi_index_container <
      tag_following_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< tag_following_object, tag_following_id_type, &tag_following_object::id > >,
         ordered_unique< tag< by_tag >,
            member< tag_following_object, tag_name_type, &tag_following_object::tag >,
         >
      >,
      allocator< tag_following_object >
   > tag_following_index;

   struct by_delegation;

   typedef multi_index_container <
      asset_delegation_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< asset_delegation_object, asset_delegation_id_type, &asset_delegation_object::id > >,
         ordered_unique< tag< by_delegation >,
            composite_key< asset_delegation_object,
               member< asset_delegation_object, account_name_type, &asset_delegation_object::delegator >,
               member< asset_delegation_object, account_name_type, &asset_delegation_object::delegatee >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_name_type > >
         >
      >,
      allocator< asset_delegation_object >
   > asset_delegation_index;

   struct by_expiration;
   struct by_account_expiration;

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
            composite_key_compare< std::less< time_point >, std::less< asset_delegation_expiration_id_type > >
         >,
         ordered_unique< tag< by_account_expiration >,
            composite_key< asset_delegation_expiration_object,
               member< asset_delegation_expiration_object, account_name_type, &asset_delegation_expiration_object::delegator >,
               member< asset_delegation_expiration_object, time_point, &asset_delegation_expiration_object::expiration >,
               member< asset_delegation_expiration_object, asset_delegation_expiration_id_type, &asset_delegation_expiration_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< time_point >, std::less< asset_delegation_expiration_id_type > >
         >
      >,
      allocator< asset_delegation_expiration_object >
   > asset_delegation_expiration_index;

   struct by_expiration;

   typedef multi_index_container <
      account_recovery_request_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_recovery_request_object,
               member< account_recovery_request_object, account_name_type, &account_recovery_request_object::account_to_recover >,
               member< account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_recovery_request_id_type > >
         >,
         ordered_unique< tag< by_expiration >,
            composite_key< account_recovery_request_object,
               member< account_recovery_request_object, time_point, &account_recovery_request_object::expiration >,
               member< account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id >
            >,
            composite_key_compare< std::less< time_point >, std::less< account_recovery_request_id_type > >
         >
      >,
      allocator< account_recovery_request_object >
   > account_recovery_request_index;

   struct by_effective_date;

   typedef multi_index_container <
      change_recovery_account_request_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< change_recovery_account_request_object, change_recovery_account_request_id_type, &change_recovery_account_request_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< change_recovery_account_request_object,
               member< change_recovery_account_request_object, account_name_type, &change_recovery_account_request_object::account_to_recover >,
               member< change_recovery_account_request_object, change_recovery_account_request_id_type, &change_recovery_account_request_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< change_recovery_account_request_id_type > >
         >,
         ordered_unique< tag< by_effective_date >,
            composite_key< change_recovery_account_request_object,
               member< change_recovery_account_request_object, time_point, &change_recovery_account_request_object::effective_on >,
               member< change_recovery_account_request_object, change_recovery_account_request_id_type, &change_recovery_account_request_object::id >
            >,
            composite_key_compare< std::less< time_point >, std::less< change_recovery_account_request_id_type > >
         >
      >,
      allocator< change_recovery_account_request_object >
   > change_recovery_account_request_index;

   struct by_account_req;

   typedef multi_index_container<
      connection_request_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< connection_request_object, connection_request_id_type, &connection_request_object::id > >,
         ordered_unique< tag<by_account_req>,
            composite_key< connection_request_object,
               member<connection_request_object, account_id_type, &connection_request_object::account >,
               member<connection_request_object, account_id_type, &connection_request_object::requested_account >
            >,
            composite_key_compare< std::less< account_id_type >, std::less< account_id_type > >
         >,
         ordered_unique< tag< by_expiration >,
            composite_key< connection_request_object,
               member< connection_request_object, time_point, &connection_request_object::expiration >,
               member< connection_request_object, connection_request_id_type, &connection_request_object::id >
            >,
            composite_key_compare< std::less< time_point >, std::less< connection_request_id_type > >
         >
      >,
      allocator< connection_request_object >
   > connection_request_index;

   struct by_accounts;
   struct by_account_a;
   struct by_account_b;

   typedef multi_index_container<
      connection_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< connection_object, connection_id_type, &connection_object::id > >,
         ordered_unique< tag<by_accounts>,
            composite_key< connection_object,
               member<connection_object, account_name_type, &connection_object::account_a >,
               member<connection_object, account_name_type, &connection_object::account_b >,
               member<connection_object, connection_types, &connection_object::connection_type >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< account_name_type >, std::greater< connection_types > >
         >,
         ordered_non_unique< tag<by_account_a>,
            composite_key< connection_object,
               member<connection_object, account_name_type, &connection_object::account_a >,
               member<connection_object, connection_types, &connection_object::connection_type >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< connection_types > >
         >,
         ordered_non_unique< tag<by_account_b>,
            composite_key< connection_object,
               member<connection_object, account_name_type, &connection_object::account_b >,
               member<connection_object, connection_types, &connection_object::connection_type >
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< connection_types > >
         >
      >,
      allocator< connection_object >
   > connection_index;

} }

FC_REFLECT( node::chain::account_object,
         (id)
         (name)
         (secure_public_key)
         (json)
         (json_private)
         (proxy)
         (last_account_update)
         (created)
         (mined)
         (last_owner_proved)
         (last_active_proved)
         (recovery_account)
         (last_account_recovery)
         (reset_account)
         (comment_count)
         (lifetime_vote_count)
         (post_count)
         (can_vote)
         (voting_power)
         (last_vote_time)
         (savings_withdraw_requests)
         (curation_rewards)
         (posting_rewards)
         (proxied_voting_power)
         (witnesses_voted_for)
         (last_post)
         (last_root_post)
         (post_bandwidth)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_object, node::chain::account_index );

FC_REFLECT( node::chain::account_authority_object,
         (id)
         (account)
         (owner)
         (active)
         (posting)
         (last_owner_update)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_authority_object, node::chain::account_authority_index )

FC_REFLECT( node::chain::asset_delegation_object,
         (id)
         (delegator)
         (delegatee)
         (asset)
         (min_delegation_time) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_delegation_object, node::chain::asset_delegation_index );

FC_REFLECT( node::chain::asset_delegation_expiration_object,
         (id)
         (delegator)
         (asset)
         (expiration) 
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::asset_delegation_expiration_object, node::chain::asset_delegation_expiration_index );

FC_REFLECT( node::chain::owner_authority_history_object,
         (id)
         (account)
         (previous_owner_authority)
         (last_valid_time)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::owner_authority_history_object, node::chain::owner_authority_history_index )

FC_REFLECT( node::chain::account_recovery_request_object,
         (id)
         (account_to_recover)
         (new_owner_authority)
         (expires)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_recovery_request_object, node::chain::account_recovery_request_index )

FC_REFLECT( node::chain::change_recovery_account_request_object,
         (id)
         (account_to_recover)
         (recovery_account)
         (effective_on)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::change_recovery_account_request_object, node::chain::change_recovery_account_request_index );

FC_REFLECT( node::chain::account_balance_object,
         (id)
         (owner)
         (symbol)
         (total_balance)
         (liquid_balance)
         (reward_balance)
         (staked_balance)
         (savings_balance)
         (delegated_balance)
         (received_balance)
         (maintenance_flag)
          );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_balance_object, node::chain::account_balance_index );

FC_REFLECT( node::chain::account_permission_object,
         (id)
         (account)
         (whitelisted_accounts)
         (blacklisted_accounts)
         (whitelisting_accounts)
         (blacklisting_accounts)
         (whitelisted_assets)
         (blacklisted_assets)
         (whitelisting_assets)
         (blacklisting_assets)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_permission_object, node::chain::account_permission_index );

