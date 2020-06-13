#pragma once
#include <fc/fixed_string.hpp>

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>
#include <node/chain/node_object_types.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <numeric>

namespace node { namespace chain {

   class account_object : public object< account_object_type, account_object >
   {
      account_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_object( Constructor&& c, allocator< Allocator > a ) : 
         details(a), 
         url(a),
         image(a),
         json(a), 
         json_private(a),
         first_name(a), 
         last_name(a), 
         gender(a), 
         date_of_birth(a), 
         email(a), 
         phone(a), 
         nationality(a),
         pinned_permlink(a)
         {
            c(*this);
         };

         id_type                          id;

         account_name_type                name;                                  ///< Username of the account, lowercase letter and numbers and hyphens only.

         shared_string                    details;                               ///< The account's Public details string.

         shared_string                    url;                                   ///< The account's Public personal URL.

         shared_string                    image;                                 ///< IPFS Reference of the Public profile image of the account.

         shared_string                    json;                                  ///< The JSON string of additional Public profile information.

         shared_string                    json_private;                          ///< The JSON string of additional encrypted profile information. Encrypted with connection key.

         shared_string                    first_name;                            ///< Encrypted First name of the user. Encrypted with connection key.

         shared_string                    last_name;                             ///< Encrypted Last name of the user. Encrypted with connection key.

         shared_string                    gender;                                ///< Encrypted Gender of the user. Encrypted with connection key.

         shared_string                    date_of_birth;                         ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.

         shared_string                    email;                                 ///< Encrypted Email address of the user. Encrypted with connection key.

         shared_string                    phone;                                 ///< Encrypted Phone Number of the user. Encrypted with connection key.

         shared_string                    nationality;                           ///< Encrypted Country of user's residence. Encrypted with connection key.

         shared_string                    pinned_permlink;                       ///< Post permlink pinned to the top of the account's profile. 
         
         membership_tier_type             membership;                            ///< Level of account membership.

         public_key_type                  secure_public_key;                     ///< Key used for receiving incoming encrypted direct messages and key exchanges.

         public_key_type                  connection_public_key;                 ///< Key used for encrypting posts for connection level visibility. 

         public_key_type                  friend_public_key;                     ///< Key used for encrypting posts for friend level visibility.

         public_key_type                  companion_public_key;                  ///< Key used for encrypting posts for companion level visibility.

         account_name_type                proxy = PROXY_TO_SELF_ACCOUNT;         ///< Account that votes on behalf of this account.

         flat_set< account_name_type >    proxied;                               ///< Accounts that have set this account to be their proxy voter.

         account_name_type                registrar;                             ///< The name of the account that created the account.

         account_name_type                referrer;                              ///< The name of the account that originally referred the account to be created.

         account_name_type                recovery_account = NULL_ACCOUNT;       ///< Account that can request recovery using a recent owner key if compromised.  

         account_name_type                reset_account = NULL_ACCOUNT;          ///< Account that has the ability to reset owner authority after specified days of inactivity.

         account_name_type                membership_interface = NULL_ACCOUNT;   ///< Account of the last interface to sell a membership to the account.

         uint16_t                         reset_account_delay_days = 7;          ///< Days of inactivity required to enable a reset account operation
         
         uint16_t                         referrer_rewards_percentage = 50 * PERCENT_1; ///< The percentage of registrar rewards that are directed to the referrer.
         
         uint32_t                         comment_count = 0;                     ///< total number of comments on other posts created

         uint32_t                         follower_count = 0;                    ///< Total number of followers that the account has

         uint32_t                         following_count = 0;                   ///< Total number of accounts that the account follows

         uint32_t                         post_vote_count = 0;                   ///< Total number of posts voted on

         uint32_t                         post_count = 0;                        ///< Total number of root posts created

         uint16_t                         voting_power = PERCENT_100;            ///< current voting power of this account, falls after every vote, recovers over time.

         uint16_t                         viewing_power = PERCENT_100;           ///< current viewing power of this account, falls after every view, recovers over time.

         uint16_t                         sharing_power = PERCENT_100;           ///< current sharing power of this account, falls after every share, recovers over time.

         uint16_t                         commenting_power = PERCENT_100;        ///< current commenting power of this account, falls after every comment, recovers over time.

         uint8_t                          savings_withdraw_requests = 0;         ///< Outstanding number of savings withdrawal requests

         uint16_t                         withdraw_routes = 0;                   ///< Number of staked asset withdrawal routes

         share_type                       posting_rewards = 0;                   ///< Rewards in core asset earned from author rewards.

         share_type                       curation_rewards = 0;                  ///< Rewards in core asset earned from voting, shares, views, and commenting
   
         share_type                       moderation_rewards = 0;                ///< Rewards in core asset from moderation rewards. 

         share_type                       total_rewards = 0;                     ///< Rewards in core asset earned from all reward sources.

         share_type                       author_reputation = 0;                 ///< 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards

         asset                            loan_default_balance = asset(0, SYMBOL_CREDIT);

         share_type                       recent_activity_claims = 0;            ///< Value of activity rewards claimed in last 30 days / BLOCKCHAIN_PRECISION

         uint16_t                         producer_vote_count = 0;               ///< Number of producers voted for.

         uint16_t                         officer_vote_count = 0;                ///< Number of network officers that the account has voted for.

         uint16_t                         executive_board_vote_count = 0;        ///< Number of Executive boards that the account has voted for.

         uint16_t                         governance_subscriptions = 0;          ///< Number of governance accounts that the account subscribes to.

         uint16_t                         enterprise_approval_count = 0;         ///< Number of Enterprise proposals that the account has voted for. 

         uint16_t                         recurring_membership = 0;              ///< Amount of months membership should be automatically renewed for on expiration

         time_point                       created;                               ///< Time that the account was created.

         time_point                       membership_expiration;                 ///< Time that the account has its current membership subscription until.

         time_point                       last_updated;                          ///< Time that the account's details were last updated.

         time_point                       last_vote_time;                        ///< Time that the account last voted on a comment.

         time_point                       last_view_time;                        ///< Time that the account last viewed a post.

         time_point                       last_share_time;                       ///< Time that the account last shared a post.

         time_point                       last_post;                             ///< Time that the user most recently created a comment 

         time_point                       last_root_post;                        ///< Time that the account last created a post.

         time_point                       last_transfer_time;                    ///< Time that the account last sent a transfer or created a trading txn. 

         time_point                       last_activity_reward;                  ///< Time that the account last claimed an activity reward. 

         time_point                       last_account_recovery;                 ///< Time that the account was last recovered.

         time_point                       last_community_created;                ///< Time that the account last created a community.

         time_point                       last_asset_created;                    ///< Time that the account last created an asset.

         bool                             mined;                                 ///< True if the account was mined by a mining producer.

         bool                             revenue_share;                         ///< True if the account is sharing revenue.

         bool                             can_vote;                              ///< True if the account can vote, false if voting has been declined.

         bool                             active;                                ///< True when the account is active, false to deactivate account.
   };


   /**
    * Describes the process and details of the verification of an account by another account.
    * 
    * Accounts must have a profile object before they can begin the verification process.
    * 
    * The verifier account proves that they have 
    * access to the profile data of the verified account
    * by signing an image of both people in the same picture, 
    * holding a hand writen note containing both account names
    * and a recent head_block_id of the blockchain using the private key
    * corresponding to the verified account's profile public key.
    */
   class account_verification_object : public object< account_verification_object_type, account_verification_object >
   {
      account_verification_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_verification_object( Constructor&& c, allocator< Allocator > a ) :
         shared_image(a)
         {
            c(*this);
         };

         id_type                   id;

         account_name_type         verifier_account;              ///< Name of the Account with the profile.

         account_name_type         verified_account;              ///< Name of the account being verifed.

         shared_string             shared_image;                  ///< IPFS reference to an image containing both people and the current.

         time_point                created;                       ///< Time of verification.

         time_point                last_updated;                  ///< Time that the verifcation was last updated. 
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

         account_name_type                        account;                       ///< Name of the account with permissions set.
   
         flat_set< account_name_type >            whitelisted_accounts;          ///< List of accounts that are able to send transfers to this account.

         flat_set< account_name_type >            blacklisted_accounts;          ///< List of accounts that are not able to receive transfers from this account.

         flat_set< asset_symbol_type >            whitelisted_assets;            ///< List of assets that the account has whitelisted to receieve transfers of. 

         flat_set< asset_symbol_type >            blacklisted_assets;            ///< List of assets that the account has blacklisted against incoming transfers.
 
         bool is_authorized_transfer( const account_name_type& name, const asset_object& asset_obj )const          ///< Determines if an asset is authorized for transfer with an accounts permissions object. 
         {
            bool fast_check = !( asset_obj.flags & int( asset_issuer_permission_flags::balance_whitelist ) );
            fast_check &= !( whitelisted_assets.size() );
            fast_check &= !( blacklisted_assets.size() );
            fast_check &= !( whitelisted_accounts.size() );
            fast_check &= !( blacklisted_accounts.size() );

            if( fast_check )
            {
               return true; // The asset does not require transfer permission, and the account does not use an asset whitelist or blacklist
            }

            if( blacklisted_accounts.size() )
            {
               if( blacklisted_accounts.find( name ) != blacklisted_accounts.end() )
               {
                  return false;  // The account is in the blacklist of the account
               }
            }

            if( whitelisted_accounts.size() )
            {
               if( whitelisted_accounts.find( name ) == whitelisted_accounts.end() )
               {
                  return false;  // The account is not in the whitelist of the account
               }
            }

            if( blacklisted_assets.size() )
            {
               if( blacklisted_assets.find( asset_obj.symbol ) != blacklisted_assets.end() )
               {
                  return false; // The asset is in the account's blacklist
               }
            }

            if( whitelisted_assets.size() )
            {
               if( whitelisted_assets.find( asset_obj.symbol ) == whitelisted_assets.end() )
               {
                  return false; // The asset is not in the account's whitelist
               } 
            }

            if( asset_obj.blacklist_authorities.size() )
            {
               if( asset_obj.blacklist_authorities.find( name ) != asset_obj.blacklist_authorities.end() )
               {
                  return false; // The account is in the asset's blacklist
               }
            }

            if( asset_obj.whitelist_authorities.size() )
            {
               if( asset_obj.whitelist_authorities.find( name ) == asset_obj.whitelist_authorities.end() )
               {
                  return false; // The account is not in the asset's whitelist
               }
            }

            return true;
         };

         bool is_authorized_transfer( const account_name_type& name )const 
         {
            if( blacklisted_accounts.size() )
            {
               if( blacklisted_accounts.find( name ) != blacklisted_accounts.end() )
               {
                  return false;  // The account is in the blacklist of the account
               }
            }

            if( whitelisted_accounts.size() )
            {
               if( whitelisted_accounts.find( name ) == whitelisted_accounts.end() )
               {
                  return false;  // The account is not in the whitelist of the account
               }
            }

            return true;
         };
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

         id_type                                         id;

         account_name_type                               account;                    ///< Username of the business account, lowercase letters only.

         business_structure_type                         business_type;              ///< Type of business account, controls authorizations for transactions of different types.

         public_key_type                                 business_public_key;        ///< Public key of the business account for internal message encryption.

         executive_officer_set                           executive_board;            ///< Set of highest voted executive accounts for each role.

         flat_map< account_name_type, pair< executive_role_type, share_type > >  executive_votes;   ///< Set of all executive names.

         flat_set< account_name_type  >                  executives;                 ///< Set of all executive names.

         flat_map< account_name_type, share_type >       officer_votes;              ///< Set of all officers in the business, and their supporting voting power.

         flat_set< account_name_type >                   officers;                   ///< Set of all officers in the business, and their supporting voting power.

         flat_set< account_name_type >                   members;                    ///< Set of all members of the business.

         share_type                                      officer_vote_threshold;     ///< Amount of voting power required for an officer to be active.

         flat_set< asset_symbol_type >                   equity_assets;              ///< Set of equity assets that offer dividends and voting power over the business account's structure.

         flat_set< asset_symbol_type >                   credit_assets;              ///< Set of credit assets that offer interest and buybacks from the business account.

         flat_map< asset_symbol_type, uint16_t >         equity_revenue_shares;      ///< Holds a map of all equity assets that the account shares incoming revenue with, and percentages.

         flat_map< asset_symbol_type, uint16_t >         credit_revenue_shares;      ///< Holds a map of all equity assets that the account shares incoming revenue with, and percentages.

         bool                                            active;                     ///< True when the business account is active, false to deactivate business account.

         time_point                                      created;                    ///< Time that business account object was created.

         time_point                                      last_updated;               ///< Time that the business account object was last updated.

         bool is_authorized_request( const account_name_type& account, const account_permission_object& obj )const      ///< Determines Permission to request to join.
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return true;
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_invite( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to send invites, accept join requests
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               {
                  return officers.find( account ) != officers.end();
               }
               break;
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return executives.find( account ) != executives.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account || executive_board.CHIEF_ADVOCACY_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_blacklist( const account_name_type& account, const account_permission_object& obj )const // Determines Permission to blacklist an account from the community. 
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return executives.find( account ) != executives.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_transfer( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to send invites, accept join requests
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               {
                  return officers.find( account ) != officers.end();
               }
               break;
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return executives.find( account ) != executives.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_FINANCIAL_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_general( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to make general transactions 
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               {
                  return officers.find( account ) != officers.end();
               }
               break;
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return executives.find( account ) != executives.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_content( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to create content and interactions, and manage communities
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return officers.find( account ) != officers.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_MARKETING_OFFICER == account || executive_board.CHIEF_ADVOCACY_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_vote_executive( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to vote for executives
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return true;
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_vote_officer( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to vote for executives
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false;
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               {
                  return true;
               }
               break;
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return executives.find( account ) != executives.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_OPERATING_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_governance( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to update governance and create moderation tags
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the blacklist
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               {
                  return officers.find( account ) != officers.end();
               }
               break;
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return executives.find( account ) != executives.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_GOVERNANCE_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool is_authorized_network( const account_name_type& account, const account_permission_object& obj )const     // Determines Permission to alter interface, supernode, network officers, producers, executive board
         {
            if( obj.blacklisted_accounts.size() )
            {
               if( obj.blacklisted_accounts.find( account ) != obj.blacklisted_accounts.end() )
               {
                  return false; // The account is in the blacklist
               }   
            }

            switch( business_type )
            {
               case business_structure_type::OPEN_BUSINESS:
               {
                  return officers.find( account ) != officers.end();
               }
               break;
               case business_structure_type::PUBLIC_BUSINESS:
               {
                  return executives.find( account ) != executives.end();
               }
               break;
               case business_structure_type::PRIVATE_BUSINESS:
               {
                  return executive_board.CHIEF_EXECUTIVE_OFFICER == account || executive_board.CHIEF_DEVELOPMENT_OFFICER == account || 
                  executive_board.CHIEF_MARKETING_OFFICER == account || executive_board.CHIEF_ADVOCACY_OFFICER == account ||
                  executive_board.CHIEF_TECHNOLOGY_OFFICER == account || executive_board.CHIEF_DESIGN_OFFICER == account;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid Business Type." );
               }
            }
         };

         bool                                    is_chief( const account_name_type& executive ) const       // finds if a given account name is in the CEO or COO. 
         {
            return executive_board.CHIEF_EXECUTIVE_OFFICER == executive || executive_board.CHIEF_OPERATING_OFFICER == executive;
         };

         bool                                    is_executive( const account_name_type& executive ) const   // finds if a given account name is in the executives set. 
         {
            return std::find( executives.begin(), executives.end(), executive ) != executives.end();
         };

         bool                                    is_officer( const account_name_type& officer ) const    // finds if a given account name is in the officers set. 
         {
            return std::find( officers.begin(), officers.end(), officer ) != officers.end();
         };

         bool                                    is_member( const account_name_type& member ) const   // finds if a given account name is in the members set. 
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

         id_type                   id;

         account_name_type         account;               ///< Username of the account, voting for the executive.
         
         account_name_type         business_account;      ///< Name of the referred business account.

         account_name_type         executive_account;     ///< Name of the executive account.

         executive_role_type       role;                  ///< Role voted in favor of.

         uint16_t                  vote_rank = 1;         ///< The rank of the executive vote.
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

         account_name_type                       account;             ///< Username of the account, voting for the officer.
         
         account_name_type                       business_account;    ///< Name of the referred business account.

         account_name_type                       officer_account;     ///< Name of the officer account.

         uint16_t                                vote_rank = 1;       ///< The rank of the officer vote.
   };

   class account_member_request_object : public object< account_member_request_object_type, account_member_request_object >
   {
      account_member_request_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_member_request_object( Constructor&& c, allocator< Allocator > a ) :
         message(a)
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;               ///< Username of the account requesting membership in the business account.
         
         account_name_type                       business_account;      ///< Name of the business account.

         shared_string                           message;               ///< Encrypted message to the business management team, encrypted with .

         time_point                              expiration;            ///< time that the request expires.
   };

   class account_member_invite_object : public object< account_member_invite_object_type, account_member_invite_object >
   {
      account_member_invite_object() = delete;

      public:
         template< typename Constructor, typename Allocator>
         account_member_invite_object( Constructor&& c, allocator< Allocator > a ) :
         message(a)
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;               ///< Username of the account creating the invitation to join the business account.
         
         account_name_type                       business_account;      ///< Name of the business account.

         account_name_type                       member;                ///< Name of the newly invited account.

         shared_string                           message;               ///< Encrypted message to the member, encrypted with the members secure public key.

         time_point                              expiration;            ///< time that the invitation expires.
   };

   class account_member_key_object : public object< account_member_key_object_type, account_member_key_object >
   {
      account_member_key_object() = delete;

      public:
         template< typename Constructor, typename Allocator>
         account_member_key_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                                 id;

         account_name_type                       account;                    ///< Username of the account that created the key for the new member.

         account_name_type                       member;                     ///< Username of the newly added member, for which the key was created.
         
         account_name_type                       business_account;           ///< Name of the business account that the key is for.

         encrypted_keypair_type                  encrypted_business_key;     ///< Copy of the business account's private communications key, encrypted with the member's secure key . 
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

         id_type                     id;

         account_name_type           owner;

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


   class account_authority_object : public object< account_authority_object_type, account_authority_object >
   {
      account_authority_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_authority_object( Constructor&& c, allocator< Allocator > a )
            : owner_auth( a ), active_auth( a ), posting_auth( a )
         {
            c( *this );
         }

         id_type                  id;

         account_name_type        account;            ///< Name of the account

         shared_authority         owner_auth;         ///< used for backup control, can set all other keys

         shared_authority         active_auth;        ///< used for all monetary operations, can set active or posting

         shared_authority         posting_auth;       ///< used for voting and posting

         time_point               last_owner_update;  ///< Time that the owner key was last updated.
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

         account_name_type                 account;              ///< Name of the account.

         flat_set< account_name_type >     followers;            ///< Accounts that follow this account.

         flat_set< account_name_type >     following;            ///< Accounts that this account follows.

         flat_set< account_name_type >     mutual_followers;     ///< Accounts that are both following and followers of this account.

         flat_set< account_name_type >     connections;          ///< Accounts that are connections of this account.

         flat_set< account_name_type >     friends;              ///< Accounts that are friends of this account.

         flat_set< account_name_type >     companions;           ///< Accounts that are companions of this account. 

         flat_set< community_name_type >   followed_communities; ///< Communities that the account subscribes to. 

         flat_set< tag_name_type >         followed_tags;        ///< Tags that the account follows. 

         flat_set< account_name_type >     filtered;             ///< Accounts that this account has filtered. Interfaces should not show posts by these users.

         flat_set< community_name_type >   filtered_communities; ///< Communities that this account has filtered. Posts will not display if they are in these communities.

         flat_set< tag_name_type >         filtered_tags;        ///< Tags that this account has filtered. Posts will not display if they have any of these tags. 

         time_point                        last_updated;          ///< Last time that the account changed its following sets.

         /**
          * Adjacency value determines how similar two accounts are by comparing the 
          * accounts, communities and tags that they have in common with eachother. 
          * this value is used for the determination of post recommendations.
          */
         share_type                        adjacency_value( const account_following_object& f )const
         {
            vector< account_name_type > common_followers;
            common_followers.reserve( followers.size() );
            std::set_intersection( f.followers.begin(), f.followers.end(), followers.begin(), followers.end(), common_followers.begin());

            vector< account_name_type > common_following;
            common_following.reserve( following.size() );
            std::set_intersection( f.following.begin(), f.following.end(), following.begin(), following.end(), common_following.begin());

            vector< account_name_type > common_mutual_followers;
            common_mutual_followers.reserve( mutual_followers.size() );
            std::set_intersection( f.mutual_followers.begin(), f.mutual_followers.end(), mutual_followers.begin(), mutual_followers.end(), common_mutual_followers.begin());

            vector< account_name_type > common_connections;
            common_connections.reserve( connections.size() );
            std::set_intersection( f.connections.begin(), f.connections.end(), connections.begin(), connections.end(), common_connections.begin());

            vector< account_name_type > common_friends;
            common_friends.reserve( friends.size() );
            std::set_intersection( f.friends.begin(), f.friends.end(), friends.begin(), friends.end(), common_friends.begin());

            vector< account_name_type > common_companions;
            common_companions.reserve( companions.size() );
            std::set_intersection( f.companions.begin(), f.companions.end(), companions.begin(), companions.end(), common_companions.begin());

            vector< account_name_type > common_followed_communities;
            common_followed_communities.reserve( followed_communities.size() );
            std::set_intersection( f.followed_communities.begin(), f.followed_communities.end(), followed_communities.begin(), followed_communities.end(), common_followed_communities.begin());

            vector< account_name_type > common_followed_tags;
            common_followed_tags.reserve( followed_tags.size() );
            std::set_intersection( f.followed_tags.begin(), f.followed_tags.end(), followed_tags.begin(), followed_tags.end(), common_followed_tags.begin());

            share_type result = common_followers.size() + common_following.size() + 2*common_mutual_followers.size() + 3*common_connections.size() +
            5*common_friends.size() + 10*common_companions.size() + common_followed_communities.size() + common_followed_tags.size();
            return result;
         };

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

         bool                              is_followed_tag( const tag_name_type& tag )const
         {
            return std::find( followed_tags.begin(), followed_tags.end(), tag ) != followed_tags.end();
         };

         bool                              is_followed_community( const community_name_type& community )const
         {
            return std::find( followed_communities.begin(), followed_communities.end(), community ) != followed_communities.end();
         };

         bool                              is_mutual( const account_name_type& account )const
         {
            return std::find( mutual_followers.begin(), mutual_followers.end(), account ) != mutual_followers.end();
         };

         bool                              is_filtered( const account_name_type& account )const
         {
            return std::find( filtered.begin(), filtered.end(), account ) != filtered.end();
         };

         bool                              is_filtered_tag( const tag_name_type& tag )const
         {
            return std::find( filtered_tags.begin(), filtered_tags.end(), tag ) != filtered_tags.end();
         };

         bool                              is_filtered_community( const community_name_type& community )const
         {
            return std::find( filtered_communities.begin(), filtered_communities.end(), community ) != filtered_communities.end();
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

         void                              add_followed_tag( const tag_name_type& t )
         {
            if( !is_followed_tag( t ) )
            {
               followed_tags.insert( t );
            }
         }

         void                              add_followed_community( const community_name_type& community )
         {
            if( !is_followed_community( community ) )
            {
               followed_communities.insert( community );
            }
         }

         void                              remove_followed_tag( const tag_name_type& t )
         {
            if( is_followed_tag( t ) )
            {
               followed_tags.erase( t );
            }
         }

         void                              remove_followed_community( const community_name_type& community )
         {
            if( is_followed_community( community ) )
            {
               followed_communities.erase( community );
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

         void                              add_filtered_tag( const tag_name_type& t )
         {
            if( !is_filtered_tag( t ) )
            {
               filtered_tags.insert( t );
            }
         }

         void                              remove_filtered_tag( const tag_name_type& t )
         {
            if( is_filtered_tag( t ) )
            {
               filtered_tags.erase( t );
            }
         }

         void                              add_filtered_community( const community_name_type& community )
         {
            if( !is_filtered_community( community ) )
            {
               filtered_communities.insert( community );
            }
         }

         void                              remove_filtered_community( const community_name_type& community )
         {
            if( is_filtered_community( community ) )
            {
               filtered_communities.erase( community );
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

         tag_name_type                     tag;              ///< Name of the tag.

         flat_set< account_name_type >     followers;        ///< Accounts that follow this tag. 

         time_point                        last_updated;     ///< Last time that the tag changed its following sets.

         share_type                        adjacency_value( const tag_following_object& t )const
         {
            vector<account_name_type> common_followers;
            common_followers.reserve( followers.size() );
            std::set_intersection( t.followers.begin(), t.followers.end(), followers.begin(), followers.end(), common_followers.begin());
            share_type result = common_followers.size();
            return result;
         };

         bool                              is_follower( const account_name_type& account ) const
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
         connection_request_object( Constructor&& c, allocator< Allocator > a ) :
         message(a)
         {
            c( *this );
         }

         id_type                id;                 

         account_name_type      account;               ///< Account that created the request.

         account_name_type      requested_account;     ///< Account that the request is being made to.

         connection_tier_type   connection_type;       ///< Connection level of the request.

         shared_string          message;               ///< Connection request accompanying message for reference. 

         time_point             expiration;            ///< Time that the request expires at. 
   };


   class connection_object : public object< connection_object_type, connection_object >
   {
      connection_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         connection_object( Constructor&& c, allocator< Allocator > a ) :
         connection_id(a)
         {
            c( *this );
         }

         id_type                      id;                 

         account_name_type            account_a;                ///< Account with the lower ID.

         encrypted_keypair_type       encrypted_key_a;          ///< A's private connection key, encrypted with the public secure key of account B.

         account_name_type            account_b;                ///< Account with the greater ID.

         encrypted_keypair_type       encrypted_key_b;          ///< B's private connection key, encrypted with the public secure key of account A.

         connection_tier_type         connection_type;          ///< The connection level shared in this object.

         shared_string                connection_id;            ///< Unique uuidv4 for the connection, for local storage of decryption key.

         uint32_t                     message_count;            ///< Number of total messages sent between connections.

         uint32_t                     consecutive_days;         ///< Number of consecutive days that the connected accounts have both sent a message.

         time_point                   last_message_time_a;      ///< Time since the account A last sent a message.

         time_point                   last_message_time_b;      ///< Time since the account B last sent a message.

         time_point                   last_updated;             ///< Time the connection keys were last updated.

         time_point                   created;                  ///< Time the connection was created.

         time_point                   last_message_time()const
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

         asset_symbol_type      symbol()const
         {
            return amount.symbol;
         }
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

         account_name_type      delegatee;

         asset                  amount;

         time_point             expiration;

         asset_symbol_type      symbol()const
         {
            return amount.symbol;
         }
   };


   class owner_authority_history_object : public object< owner_authority_history_object_type, owner_authority_history_object >
   {
      owner_authority_history_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         owner_authority_history_object( Constructor&& c, allocator< Allocator > a ) : 
            previous_owner_authority( shared_authority::allocator_type( a.get_segment_manager() ) )
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
         account_recovery_request_object( Constructor&& c, allocator< Allocator > a ) : 
            new_owner_authority( shared_authority::allocator_type( a.get_segment_manager() ) )
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
   struct by_following_count;
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
         ordered_unique< tag< by_following_count >,
            composite_key< account_object,
               member< account_object, uint32_t, &account_object::following_count >,
               member< account_object, account_id_type, &account_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< account_id_type > 
            >
         >,
         ordered_unique< tag< by_vote_count >,
            composite_key< account_object,
               member< account_object, uint32_t, &account_object::post_vote_count >,
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
               std::less< owner_authority_history_id_type > 
            >
         >
      >,
      allocator< owner_authority_history_object >
   > owner_authority_history_index;


   struct by_verifier_verified;
   struct by_verified_verifier;


   typedef multi_index_container<
      account_verification_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< account_verification_object, account_verification_id_type, &account_verification_object::id > 
         >,
         ordered_unique< tag< by_verified_verifier >,
            composite_key< account_verification_object,
               member< account_verification_object, account_name_type, &account_verification_object::verified_account >,
               member< account_verification_object, account_name_type, &account_verification_object::verifier_account >
            >
         >,
         ordered_unique< tag< by_verifier_verified >,
            composite_key< account_verification_object,
               member< account_verification_object, account_name_type, &account_verification_object::verifier_account >,
               member< account_verification_object, account_name_type, &account_verification_object::verified_account >
            >
         >
      >,
      allocator< account_verification_object >
   > account_verification_index;


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
   struct by_executive;

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
         ordered_unique< tag< by_executive >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::executive_account >,
               member< account_executive_vote_object, account_executive_vote_id_type, &account_executive_vote_object::id >
            >
         >,
         ordered_unique< tag< by_business_role_executive >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, executive_role_type, &account_executive_vote_object::role >,
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
               member< account_executive_vote_object, executive_role_type, &account_executive_vote_object::role >,
               member< account_executive_vote_object, uint16_t, &account_executive_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_business_account_role_rank >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::account >,
               member< account_executive_vote_object, executive_role_type, &account_executive_vote_object::role >,
               member< account_executive_vote_object, uint16_t, &account_executive_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_business_role_executive >,
            composite_key< account_executive_vote_object,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::account >,
               member< account_executive_vote_object, account_name_type, &account_executive_vote_object::business_account >,
               member< account_executive_vote_object, executive_role_type, &account_executive_vote_object::role >,
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
   struct by_officer;
   
   typedef multi_index_container <
      account_officer_vote_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_officer_vote_object, account_officer_vote_id_type, &account_officer_vote_object::id > 
         >,
         ordered_unique< tag< by_business >,
            composite_key< account_officer_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, account_officer_vote_id_type, &account_officer_vote_object::id >
            >
         >,
         ordered_unique< tag< by_officer >,
            composite_key< account_officer_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::officer_account >,
               member< account_officer_vote_object, account_officer_vote_id_type, &account_officer_vote_object::id >
            >
         >,
         ordered_unique< tag< by_business_account_rank >,
            composite_key< account_officer_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::account >,
               member< account_officer_vote_object, uint16_t, &account_officer_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_business_officer >,
            composite_key< account_officer_vote_object,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::business_account >,
               member< account_officer_vote_object, account_name_type, &account_officer_vote_object::officer_account >
            >
         >,
         ordered_unique< tag< by_business_officer >,
            composite_key< account_officer_vote_object,
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
   struct by_business_account;
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
            >
         >,
         ordered_unique< tag< by_business_account >,
            composite_key< account_member_request_object,
               member< account_member_request_object, account_name_type, &account_member_request_object::business_account >,
               member< account_member_request_object, account_name_type, &account_member_request_object::account >
            >
         >
      >,
      allocator< account_member_request_object >
   > account_member_request_index;

   struct by_member;
   struct by_business;
   struct by_account;


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
            >
         >,
         ordered_unique< tag< by_member_business >,
            composite_key< account_member_invite_object,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::member >,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::business_account >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< account_member_invite_object,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::account >,
               member< account_member_invite_object, account_member_invite_id_type, &account_member_invite_object::id >
            >
         >,
         ordered_unique< tag< by_member >,
            composite_key< account_member_invite_object,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::member >,
               member< account_member_invite_object, account_member_invite_id_type, &account_member_invite_object::id >
            >
         >,
         ordered_unique< tag< by_business >,
            composite_key< account_member_invite_object,
               member< account_member_invite_object, account_name_type, &account_member_invite_object::business_account >,
               member< account_member_invite_object, account_member_invite_id_type, &account_member_invite_object::id >
            >
         >
      >,
      allocator< account_member_invite_object >
   > account_member_invite_index;

   struct by_business_member;
   struct by_member_business;

   typedef multi_index_container <
      account_member_key_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_member_key_object, account_member_key_id_type, &account_member_key_object::id >
         >,
         ordered_unique< tag< by_member_business >,
            composite_key< account_member_key_object,
               member< account_member_key_object, account_name_type, &account_member_key_object::member >,
               member< account_member_key_object, account_name_type, &account_member_key_object::business_account >,
               member< account_member_key_object, account_member_key_id_type, &account_member_key_object::id >
            >
         >,
         ordered_unique< tag< by_business_member >,
            composite_key< account_member_key_object,
               member< account_member_key_object, account_name_type, &account_member_key_object::business_account >,
               member< account_member_key_object, account_name_type, &account_member_key_object::member >,
               member< account_member_key_object, account_member_key_id_type, &account_member_key_object::id >
            >
         >
      >,
      allocator< account_member_key_object >
   > account_member_key_index;


   struct by_last_owner_update;
   struct by_owner_symbol;
   struct by_owner;
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
         >
      >,
      allocator< account_balance_object >
   > account_balance_index;


   struct by_vesting_time;
   struct by_owner_symbol_time;

   typedef multi_index_container <
      account_vesting_balance_object,
      indexed_by<
         ordered_unique< tag<by_id>, 
            member< account_vesting_balance_object, account_vesting_balance_id_type, &account_vesting_balance_object::id > 
         >,
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
            member< tag_following_object, tag_name_type, &tag_following_object::tag > >
      >,
      allocator< tag_following_object >
   > tag_following_index;

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
   struct by_req_account;

   typedef multi_index_container<
      connection_request_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< connection_request_object, connection_request_id_type, &connection_request_object::id > >,
         ordered_unique< tag<by_account_req>,
            composite_key< connection_request_object,
               member<connection_request_object, account_name_type, &connection_request_object::account >,
               member<connection_request_object, account_name_type, &connection_request_object::requested_account >
            >
         >,
         ordered_unique< tag<by_req_account>,
            composite_key< connection_request_object,
               member<connection_request_object, account_name_type, &connection_request_object::requested_account >,
               member<connection_request_object, account_name_type, &connection_request_object::account >
            >
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
   struct by_last_message_time;

   typedef multi_index_container<
      connection_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< connection_object, connection_id_type, &connection_object::id > >,
         ordered_unique< tag< by_accounts >,
            composite_key< connection_object,
               member< connection_object, account_name_type, &connection_object::account_a >,
               member< connection_object, account_name_type, &connection_object::account_b >,
               member< connection_object, connection_tier_type, &connection_object::connection_type >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               std::greater< connection_tier_type > 
            >
         >,
         ordered_unique< tag< by_account_a >,
            composite_key< connection_object,
               member< connection_object, account_name_type, &connection_object::account_a >,
               member< connection_object, connection_tier_type, &connection_object::connection_type >,
               member< connection_object, connection_id_type, &connection_object::id > 
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< connection_tier_type >, 
               std::less<connection_id_type > 
            >
         >,
         ordered_unique< tag< by_account_b >,
            composite_key< connection_object,
               member< connection_object, account_name_type, &connection_object::account_b >,
               member< connection_object, connection_tier_type, &connection_object::connection_type >,
               member< connection_object, connection_id_type, &connection_object::id > 
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< connection_tier_type >, 
               std::less< connection_id_type > 
            >
         >,
         ordered_unique< tag< by_last_message_time >,
            composite_key< connection_object,
               const_mem_fun< connection_object, time_point, &connection_object::last_message_time >,
               member< connection_object, connection_id_type, &connection_object::id > 
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< connection_id_type > 
            >
         >
      >,
      allocator< connection_object >
   > connection_index;

} }   // node::chain

FC_REFLECT( node::chain::account_object,
         (id)
         (name)
         (details)
         (json)
         (json_private)
         (url)
         (image)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (pinned_permlink)
         (membership)
         (secure_public_key)
         (connection_public_key)
         (friend_public_key)
         (companion_public_key)
         (proxy)
         (proxied)
         (registrar)
         (referrer)
         (recovery_account)
         (reset_account)
         (membership_interface)
         (reset_account_delay_days)
         (referrer_rewards_percentage)
         (comment_count)
         (follower_count)
         (following_count)
         (post_vote_count)
         (post_count)
         (voting_power)
         (viewing_power)
         (sharing_power)
         (commenting_power)
         (savings_withdraw_requests)
         (withdraw_routes)
         (posting_rewards)
         (curation_rewards)
         (moderation_rewards)
         (total_rewards)
         (author_reputation)
         (loan_default_balance)
         (recent_activity_claims)
         (producer_vote_count)
         (officer_vote_count)
         (executive_board_vote_count)
         (governance_subscriptions)
         (enterprise_approval_count)
         (recurring_membership)
         (created)
         (membership_expiration)
         (last_updated)
         (last_vote_time)
         (last_view_time)
         (last_share_time)
         (last_post)
         (last_root_post)
         (last_transfer_time)
         (last_activity_reward)
         (last_account_recovery)
         (last_community_created)
         (last_asset_created)
         (mined)
         (revenue_share)
         (can_vote)
         (active)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_object, node::chain::account_index );

FC_REFLECT( node::chain::account_verification_object,
         (id)
         (verifier_account)
         (verified_account)
         (shared_image)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_verification_object, node::chain::account_verification_index );

FC_REFLECT( node::chain::account_business_object,
         (id)
         (account)
         (business_type)
         (business_public_key)
         (executive_board)
         (executive_votes)
         (executives)
         (officer_votes)
         (officers)
         (members)
         (officer_vote_threshold)
         (equity_assets)
         (credit_assets)
         (equity_revenue_shares)
         (credit_revenue_shares)
         (active)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_business_object, node::chain::account_business_index );

FC_REFLECT( node::chain::account_executive_vote_object,
         (id)
         (account)
         (business_account)
         (executive_account)
         (role)
         (vote_rank)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_executive_vote_object, node::chain::account_executive_vote_index );

FC_REFLECT( node::chain::account_officer_vote_object,
         (id)
         (account)
         (business_account)
         (officer_account)
         (vote_rank)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_officer_vote_object, node::chain::account_officer_vote_index );

FC_REFLECT( node::chain::account_member_request_object,
         (id)
         (account)
         (business_account)
         (message)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_member_request_object, node::chain::account_member_request_index );

FC_REFLECT( node::chain::account_member_invite_object,
         (id)
         (account)
         (business_account)
         (member)
         (message)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_member_invite_object, node::chain::account_member_invite_index );

FC_REFLECT( node::chain::account_member_key_object,
         (id)
         (account)
         (member)
         (business_account)
         (encrypted_business_key)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_member_key_object, node::chain::account_member_key_index );

FC_REFLECT( node::chain::account_permission_object,
         (id)
         (account)
         (whitelisted_accounts)
         (blacklisted_accounts)
         (whitelisted_assets)
         (blacklisted_assets)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_permission_object, node::chain::account_permission_index );

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

FC_REFLECT( node::chain::account_authority_object,
         (id)
         (account)
         (owner_auth)
         (active_auth)
         (posting_auth)
         (last_owner_update)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_authority_object, node::chain::account_authority_index );

FC_REFLECT( node::chain::account_following_object,
         (id)
         (account)
         (followers)
         (following)
         (mutual_followers)
         (connections)
         (friends)
         (companions)
         (followed_communities)
         (followed_tags)
         (filtered)
         (filtered_communities)
         (filtered_tags)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_following_object, node::chain::account_following_index );

FC_REFLECT( node::chain::tag_following_object,
         (id)
         (tag)
         (followers)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::tag_following_object, node::chain::tag_following_index );

FC_REFLECT( node::chain::connection_request_object,
         (id)
         (account)
         (requested_account)
         (connection_type)
         (message)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::connection_request_object, node::chain::connection_request_index );

FC_REFLECT( node::chain::connection_object,
         (id)
         (account_a)
         (encrypted_key_a)
         (account_b)
         (encrypted_key_b)
         (connection_type)
         (connection_id)
         (message_count)
         (consecutive_days)
         (last_message_time_a)
         (last_message_time_b)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::connection_object, node::chain::connection_index );

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
         (amount)
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
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_recovery_request_object, node::chain::account_recovery_request_index )

FC_REFLECT( node::chain::change_recovery_account_request_object,
         (id)
         (account_to_recover)
         (recovery_account)
         (effective_on)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::change_recovery_account_request_object, node::chain::change_recovery_account_request_index );