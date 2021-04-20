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
    * Accounts hold the state information of every user of the Protocol.
    * 
    * Every account is comprised of a set of signing authorities, and profile information for 
    * managing interaction and transaction signing.
    * 
    * Many Elements in the Account object should be encrypted with the Connection Key of the Account
    * so that they can be selectively decrypted by other accounts that the user connects with.
    */
   class account_object : public object< account_object_type, account_object >
   {
      account_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         account_object( Constructor&& c, allocator< Allocator > a ) : 
            details(a),
            url(a),
            profile_image(a),
            cover_image(a),
            json(a),
            json_private(a),
            first_name(a),
            last_name(a),
            gender(a),
            date_of_birth(a),
            email(a),
            phone(a),
            nationality(a),
            relationship(a),
            political_alignment(a),
            pinned_permlink(a)
            {
               c(*this);
            };

         id_type                          id;

         account_name_type                name;                                  ///< Username of the account, lowercase letter and numbers and hyphens only.

         shared_string                    details;                               ///< The account's Public details string. Profile Biography.

         shared_string                    url;                                   ///< The account's Public personal URL.

         shared_string                    profile_image;                         ///< IPFS Reference of the Public profile image of the account.

         shared_string                    cover_image;                           ///< IPFS Reference of the Public cover image of the account.

         shared_string                    json;                                  ///< The JSON string of additional Public profile information.

         shared_string                    json_private;                          ///< The JSON string of additional encrypted profile information. Encrypted with connection key.

         shared_string                    first_name;                            ///< Encrypted First name of the user. Encrypted with connection key.

         shared_string                    last_name;                             ///< Encrypted Last name of the user. Encrypted with connection key.

         shared_string                    gender;                                ///< Encrypted Gender of the user. Encrypted with connection key.

         shared_string                    date_of_birth;                         ///< Encrypted Date of birth of the user. Format: DD-MM-YYYY. Encrypted with connection key.

         shared_string                    email;                                 ///< Encrypted Email address of the user. Encrypted with connection key.

         shared_string                    phone;                                 ///< Encrypted Phone Number of the user. Encrypted with connection key.

         shared_string                    nationality;                           ///< Encrypted Country of user's residence. Encrypted with connection key.

         shared_string                    relationship;                          ///< Encrypted Relationship status of the account. Encrypted with connection key.

         shared_string                    political_alignment;                   ///< Encrypted Political alignment. Encrypted with connection key.
         
         shared_string                    pinned_permlink;                       ///< Post permlink pinned to the top of the account's profile.

         flat_set< tag_name_type >        interests;                             ///< Set of tags of the interests of the user.
         
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

         uint16_t                         reset_delay_days = 7;                  ///< Days of inactivity required to enable a reset account operation
         
         uint16_t                         referrer_rewards_percentage = 50 * PERCENT_1; ///< The percentage of registrar rewards that are directed to the referrer.
         
         uint32_t                         comment_count = 0;                     ///< total number of comments on other posts created

         uint32_t                         follower_count = 0;                    ///< Total number of followers that the account has

         uint32_t                         following_count = 0;                   ///< Total number of accounts that the account follows

         uint32_t                         post_vote_count = 0;                   ///< Total number of posts voted on

         uint32_t                         post_count = 0;                        ///< Total number of root posts created

         uint16_t                         voting_power = PERCENT_100;            ///< Current voting power of this account, falls after every vote, recovers over time.

         uint16_t                         viewing_power = PERCENT_100;           ///< Current viewing power of this account, falls after every view, recovers over time.

         uint16_t                         sharing_power = PERCENT_100;           ///< Current sharing power of this account, falls after every share, recovers over time.

         uint16_t                         commenting_power = PERCENT_100;        ///< Current commenting power of this account, falls after every comment, recovers over time.

         uint8_t                          savings_withdraw_requests = 0;         ///< Outstanding number of savings withdrawal requests

         uint16_t                         withdraw_routes = 0;                   ///< Number of staked asset withdrawal routes

         share_type                       posting_rewards = 0;                   ///< Rewards in core asset earned from author rewards.

         share_type                       curation_rewards = 0;                  ///< Rewards in core asset earned from voting, shares, views, and commenting
   
         share_type                       moderation_rewards = 0;                ///< Rewards in core asset from moderation rewards. 

         share_type                       total_rewards = 0;                     ///< Rewards in core asset earned from all reward sources.

         share_type                       author_reputation = 0;                 ///< 0 to BLOCKCHAIN_PRECISION rating of the account, based on relative total rewards

         asset                            loan_default_balance = asset(0, SYMBOL_CREDIT);   ///< Amount of credit asset that is owed by the account to the network.

         share_type                       recent_activity_claims = 0;            ///< Value of activity rewards claimed in last 30 days / BLOCKCHAIN_PRECISION

         uint16_t                         producer_vote_count = 0;               ///< Number of producers voted for.

         uint16_t                         officer_vote_count = 0;                ///< Number of network officers that the account has voted for.

         uint16_t                         governance_members = 0;          ///< Number of governance accounts that the account subscribes to.

         uint16_t                         enterprise_vote_count = 0;             ///< Number of Enterprise proposals that the account has voted for. 

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
    * Contains the Transaction signing Authorities of an account.
    */
   class account_authority_object : public object< account_authority_object_type, account_authority_object >
   {
      account_authority_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_authority_object( Constructor&& c, allocator< Allocator > a ) :
            owner_auth( a ), 
            active_auth( a ), 
            posting_auth( a )
            {
               c( *this );
            }

         id_type                  id;

         account_name_type        account;            ///< Name of the account.

         shared_authority         owner_auth;         ///< Used for backup control, can set all other keys.

         shared_authority         active_auth;        ///< Used for all monetary operations, can set active or posting.

         shared_authority         posting_auth;       ///< Used for voting and posting.

         time_point               last_owner_update;  ///< Time that the owner key was last updated.
   };


   /**
    * Permissions contain a set of whitelisted and blacklisted Accounts and Assets.
    */
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
   
         flat_set< account_name_type >            whitelisted_accounts;          ///< List of accounts that are able to interact with this account.

         flat_set< account_name_type >            blacklisted_accounts;          ///< List of accounts that are not able to interact with this account.

         flat_set< asset_symbol_type >            whitelisted_assets;            ///< List of assets that the account has whitelisted to transact with. 

         flat_set< asset_symbol_type >            blacklisted_assets;            ///< List of assets that the account has blacklisted against transactions.

         flat_set< community_name_type >          whitelisted_communities;       ///< List of communities that the account has whitelisted to interact with. 

         flat_set< community_name_type >          blacklisted_communities;       ///< List of communities that the account has blacklisted against interactions.
 
         bool is_authorized_transfer( const account_name_type& name, const asset_object& asset_obj )const          ///< Determines if an asset is authorized for transfer with an accounts permissions object. 
         {
            bool fast_check = !asset_obj.require_balance_whitelist() && !asset_obj.is_transfer_restricted();

            fast_check &= !( whitelisted_assets.size() );
            fast_check &= !( blacklisted_assets.size() );
            fast_check &= !( whitelisted_accounts.size() );
            fast_check &= !( blacklisted_accounts.size() );

            if( fast_check )
            {
               return true; // The asset does not require transfer permission, and the account does not use an asset whitelist or blacklist
            }

            if( asset_obj.is_transfer_restricted() )
            {
               return asset_obj.issuer == name || asset_obj.issuer == account;
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


   /**
    * Manages the Connections between accounts and communities in the Social Graph.
    * 
    * Determines the accounts that are followed and connected with the account, and 
    * the communities that the account subscribes to and is a member of. 
    * Includes the Tags that are followed, and all fo the account, communities and tags
    * that are filtered from display by the account.
    */
   class account_following_object : public object< account_following_object_type, account_following_object >
   {
      account_following_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_following_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                id;

         account_name_type                      account;                                  ///< Name of the account.

         flat_set< account_name_type >          followers;                                ///< Accounts that follow this account.

         flat_set< account_name_type >          following;                                ///< Accounts that this account follows.

         flat_set< account_name_type >          mutual_followers;                         ///< Accounts that are both following and followers of this account.

         flat_set< account_name_type >          connections;                              ///< Accounts that are connections of this account.

         flat_set< account_name_type >          friends;                                  ///< Accounts that are friends of this account.

         flat_set< account_name_type >          companions;                               ///< Accounts that are companions of this account.

         flat_set< community_name_type >        followed_communities;                     ///< Communities that the account subscribes to.

         flat_set< community_name_type >        member_communities;                       ///< Communities that the account is a member within.

         flat_set< community_name_type >        standard_premium_member_communities;      ///< Communities that the account is a standard premium member within.

         flat_set< community_name_type >        mid_premium_member_communities;           ///< Communities that the account is a mid premium member within.

         flat_set< community_name_type >        top_premium_member_communities;           ///< Communities that the account is a top premium member within.

         flat_set< community_name_type >        moderator_communities;                    ///< Communities that the account is a moderator within.

         flat_set< community_name_type >        admin_communities;                        ///< Communities that the account is an admin within.

         flat_set< community_name_type >        founder_communities;                      ///< Communities that the account is a founder of.

         flat_set< tag_name_type >              followed_tags;                            ///< Tags that the account follows.

         flat_set< account_name_type >          filtered;                                 ///< Accounts that this account has filtered. Interfaces should not show posts by these users.

         flat_set< community_name_type >        filtered_communities;                     ///< Communities that this account has filtered. Posts will not display if they are in these communities.

         flat_set< tag_name_type >              filtered_tags;                            ///< Tags that this account has filtered. Posts will not display if they have any of these tags. 

         time_point                             last_updated;                             ///< Last time that the account changed its following sets.

         /**
          * Adjacency value determines how similar two accounts are by comparing the 
          * accounts, communities and tags that they have in common with eachother. 
          * this value is used for the determination of post recommendations.
          */
         share_type                             adjacency_value( const account_following_object& f )const
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

         bool                                   is_connection( const account_name_type& a )const
         {
            return std::find( connections.begin(), connections.end(), a ) != connections.end();
         };

         bool                                   is_friend( const account_name_type& a )const
         {
            return std::find( friends.begin(), friends.end(), a ) != friends.end();
         };

         bool                                   is_companion( const account_name_type& a )const
         {
            return std::find( companions.begin(), companions.end(), a ) != companions.end();
         };

         bool                                   is_follower( const account_name_type& a )const
         {
            return std::find( followers.begin(), followers.end(), a ) != followers.end();
         };

         bool                                   is_following( const account_name_type& a )const
         {
            return std::find( following.begin(), following.end(), a ) != following.end();
         };

         bool                                   is_followed_tag( const tag_name_type& tag )const
         {
            return std::find( followed_tags.begin(), followed_tags.end(), tag ) != followed_tags.end();
         };

         bool                                   is_followed_community( const community_name_type& community )const
         {
            return std::find( followed_communities.begin(), followed_communities.end(), community ) != followed_communities.end();
         };

         bool                                   is_member_community( const community_name_type& community )const
         {
            return std::find( member_communities.begin(), member_communities.end(), community ) != member_communities.end();
         };

         bool                                   is_standard_premium_member_community( const community_name_type& community )const
         {
            return std::find( standard_premium_member_communities.begin(), standard_premium_member_communities.end(), community ) != standard_premium_member_communities.end();
         };

         bool                                   is_mid_premium_member_community( const community_name_type& community )const
         {
            return std::find( mid_premium_member_communities.begin(), mid_premium_member_communities.end(), community ) != mid_premium_member_communities.end();
         };

         bool                                   is_top_premium_member_community( const community_name_type& community )const
         {
            return std::find( top_premium_member_communities.begin(), top_premium_member_communities.end(), community ) != top_premium_member_communities.end();
         };

         bool                                   is_moderator_community( const community_name_type& community )const
         {
            return std::find( moderator_communities.begin(), moderator_communities.end(), community ) != moderator_communities.end();
         };

         bool                                   is_admin_community( const community_name_type& community )const
         {
            return std::find( admin_communities.begin(), admin_communities.end(), community ) != admin_communities.end();
         };

         bool                                   is_founder_community( const community_name_type& community )const
         {
            return std::find( founder_communities.begin(), founder_communities.end(), community ) != founder_communities.end();
         };

         bool                                   is_mutual( const account_name_type& a )const
         {
            return std::find( mutual_followers.begin(), mutual_followers.end(), a ) != mutual_followers.end();
         };

         bool                                   is_filtered( const account_name_type& a )const
         {
            return std::find( filtered.begin(), filtered.end(), a ) != filtered.end();
         };

         bool                                   is_filtered_tag( const tag_name_type& tag )const
         {
            return std::find( filtered_tags.begin(), filtered_tags.end(), tag ) != filtered_tags.end();
         };

         bool                                   is_filtered_community( const community_name_type& community )const
         {
            return std::find( filtered_communities.begin(), filtered_communities.end(), community ) != filtered_communities.end();
         };

         void                                   add_follower( const account_name_type& a )
         {
            if( !is_follower( a ) )
            {
               followers.insert( a );
            }
            if( is_following( a ) )
            {
               mutual_followers.insert( a );
            }
         }

         void                                   remove_follower( const account_name_type& a )
         {
            if( is_follower( a ) )
            {
               if( is_mutual( a ) )
               {
                  mutual_followers.erase( a );
               }
               followers.erase( a );
            }
         }

         void                                   add_following( const account_name_type& a )
         {
            if( !is_following( a ) )
            {
               following.insert( a );
            }
            if( is_follower( a ) )
            {
               mutual_followers.insert( a );
            }
         }

         void                                   add_connection( const account_name_type& a )
         {
            if( !is_connection( a ) )
            {
               connections.insert( a );
            }
         }

         void                                   add_friend( const account_name_type& a )
         {
            if( !is_friend( a ) )
            {
               friends.insert( a );
            }
         }

         void                                   add_companion( const account_name_type& a )
         {
            if( !is_companion( a ) )
            {
               companions.insert( a );
            }
         }

         void                                   add_followed_tag( const tag_name_type& t )
         {
            if( !is_followed_tag( t ) )
            {
               followed_tags.insert( t );
            }
         }

         void                                   add_followed_community( const community_name_type& community )
         {
            if( !is_followed_community( community ) )
            {
               followed_communities.insert( community );
            }
         }

         void                                   add_member_community( const community_name_type& community )
         {
            if( !is_member_community( community ) )
            {
               member_communities.insert( community );
            }
         }

         void                                   add_standard_premium_member_community( const community_name_type& community )
         {
            if( !is_standard_premium_member_community( community ) )
            {
               standard_premium_member_communities.insert( community );
            }
         }

         void                                   add_mid_premium_member_community( const community_name_type& community )
         {
            if( !is_mid_premium_member_community( community ) )
            {
               mid_premium_member_communities.insert( community );
            }
         }

         void                                   add_top_premium_member_community( const community_name_type& community )
         {
            if( !is_top_premium_member_community( community ) )
            {
               top_premium_member_communities.insert( community );
            }
         }

         void                                   add_moderator_community( const community_name_type& community )
         {
            if( !is_moderator_community( community ) )
            {
               moderator_communities.insert( community );
            }
         }

         void                                   add_admin_community( const community_name_type& community )
         {
            if( !is_admin_community( community ) )
            {
               admin_communities.insert( community );
            }
         }

         void                                   add_founder_community( const community_name_type& community )
         {
            if( !is_founder_community( community ) )
            {
               founder_communities.insert( community );
            }
         }

         void                                   remove_following( const account_name_type& a )
         {
            if( is_following( a ) )
            {
               if( is_mutual( a ) )
               {
                  mutual_followers.erase( a );
               }
               following.erase( a );
            }
         }

         void                                   remove_connection( const account_name_type& a )
         {
            if( is_connection( a ) )
            {
               connections.erase( a );
            }
         }

         void                                   remove_friend( const account_name_type& a )
         {
            if( is_friend( a ) )
            {
               friends.erase( a );
            }
         }

         void                                   remove_companion( const account_name_type& a )
         {
            if( is_companion( a ) )
            {
               companions.erase( a );
            }
         }

         void                                   remove_followed_tag( const tag_name_type& t )
         {
            if( is_followed_tag( t ) )
            {
               followed_tags.erase( t );
            }
         }

         void                                   remove_followed_community( const community_name_type& community )
         {
            if( is_followed_community( community ) )
            {
               followed_communities.erase( community );
            }
         }

         void                                   remove_member_community( const community_name_type& community )
         {
            if( is_member_community( community ) )
            {
               member_communities.erase( community );
            }
         }

         void                                   remove_standard_premium_member_community( const community_name_type& community )
         {
            if( is_standard_premium_member_community( community ) )
            {
               standard_premium_member_communities.erase( community );
            }
         }

         void                                   remove_mid_premium_member_community( const community_name_type& community )
         {
            if( is_mid_premium_member_community( community ) )
            {
               mid_premium_member_communities.erase( community );
            }
         }

         void                                   remove_top_premium_member_community( const community_name_type& community )
         {
            if( is_top_premium_member_community( community ) )
            {
               top_premium_member_communities.erase( community );
            }
         }

         void                                   remove_moderator_community( const community_name_type& community )
         {
            if( is_moderator_community( community ) )
            {
               moderator_communities.erase( community );
            }
         }

         void                                   remove_admin_community( const community_name_type& community )
         {
            if( is_admin_community( community ) )
            {
               admin_communities.erase( community );
            }
         }

         void                                   remove_founder_community( const community_name_type& community )
         {
            if( is_founder_community( community ) )
            {
               founder_communities.erase( community );
            }
         }         

         void                                   add_filtered( const account_name_type& a )
         {
            if( !is_filtered( a ) )
            {
               filtered.insert( a );
            }
         }

         void                                   remove_filtered( const account_name_type& a )
         {
            if( is_filtered( a ) )
            {
               filtered.erase( a );
            }
         }

         void                                   add_filtered_tag( const tag_name_type& t )
         {
            if( !is_filtered_tag( t ) )
            {
               filtered_tags.insert( t );
            }
         }

         void                                   remove_filtered_tag( const tag_name_type& t )
         {
            if( is_filtered_tag( t ) )
            {
               filtered_tags.erase( t );
            }
         }

         void                                   add_filtered_community( const community_name_type& community )
         {
            if( !is_filtered_community( community ) )
            {
               filtered_communities.insert( community );
            }
         }

         void                                   remove_filtered_community( const community_name_type& community )
         {
            if( is_filtered_community( community ) )
            {
               filtered_communities.erase( community );
            }
         }

   };


   /**
    * Contains the Set of all accounts that follow a Tag.
    * 
    * Additonally used to calculate the adjacency value of the tag to another tag
    * by finding the amount of common followers of one tag and another tag.
    */
   class account_tag_following_object : public object< account_tag_following_object_type, account_tag_following_object >
   {
      account_tag_following_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_tag_following_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                           id;

         tag_name_type                     tag;              ///< Name of the tag.

         flat_set< account_name_type >     followers;        ///< Accounts that follow this tag.

         time_point                        last_updated;     ///< Last time that the tag changed its following sets.

         share_type                        adjacency_value( const account_tag_following_object& t )const
         {
            vector<account_name_type> common_followers;
            common_followers.reserve( followers.size() );
            std::set_intersection( t.followers.begin(), t.followers.end(), followers.begin(), followers.end(), common_followers.begin());
            share_type result = common_followers.size();
            return result;
         };

         bool                              is_follower( const account_name_type& a ) const
         {
            return std::find( followers.begin(), followers.end(), a ) != followers.end();
         };

         void                              add_follower( const account_name_type& a )
         {
            if( !is_follower( a ) )
            {
               followers.insert( a );
            }
         }

         void                              remove_follower( const account_name_type& a )
         {
            if( is_follower( a ) )
            {
               followers.erase( a );
            }
         }
   };


   /**
    * Connects two accounts together at a specified connection level.
    * 
    * Contains the encrypted private connection, friend, or companion keys 
    * of two accounts to enable the selective decryption of provate content and 
    * profile information.
    */
   class account_connection_object : public object< account_connection_object_type, account_connection_object >
   {
      account_connection_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_connection_object( Constructor&& c, allocator< Allocator > a ) :
            message_a(a),
            json_a(a),
            message_b(a),
            json_b(a),
            connection_id(a)
            {
               c( *this );
            }

         id_type                      id;                 

         account_name_type            account_a;                ///< Account with the lower ID.

         encrypted_keypair_type       encrypted_key_a;          ///< A's private connection key, encrypted with the public secure key of account B.

         shared_string                message_a;                ///< A's Connection encrypted accompanying message for reference.

         shared_string                json_a;                   ///< A's Encrypted JSON metadata.

         account_name_type            account_b;                ///< Account with the greater ID.

         encrypted_keypair_type       encrypted_key_b;          ///< B's private connection key, encrypted with the public secure key of account A.

         shared_string                message_b;                ///< B's Connection encrypted accompanying message for reference.

         shared_string                json_b;                   ///< B's Encrypted JSON metadata.

         connection_tier_type         connection_type;          ///< The connection level shared in this object.

         shared_string                connection_id;            ///< UUIDv4 for the connection, for local storage of decryption key.

         uint32_t                     message_count;            ///< Number of total messages sent between connections.

         uint32_t                     consecutive_days;         ///< Number of consecutive days that the connected accounts have both sent a message.

         time_point                   last_message_time_a;      ///< Time since account A last sent a message.

         time_point                   last_message_time_b;      ///< Time since account B last sent a message.

         bool                         approved_a;               ///< True when account A approves the connection.

         bool                         approved_b;               ///< True when account B approves the connection.

         time_point                   last_updated;             ///< Time the connection keys were last updated.

         time_point                   created;                  ///< Time the connection was created.

         time_point                   last_message_time()const
         {
            return std::min( last_message_time_a, last_message_time_b );
         }

         bool                         approved()const
         {
            return approved_a && approved_b;
         }
   };


   /**
    * Records the previous instances of an account's owner key when updated.
    * 
    * Used to check the validity of an account recovery operation, 
    * which requires the inclusion of a recent previous owner authority.
    */
   class account_authority_history_object : public object< account_authority_history_object_type, account_authority_history_object >
   {
      account_authority_history_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_authority_history_object( Constructor&& c, allocator< Allocator > a ) : 
            previous_owner_authority( shared_authority::allocator_type( a.get_segment_manager() ) )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      account;

         shared_authority       previous_owner_authority;

         time_point             last_valid_time;
   };


   /**
    * Creates a request for an account to be recovered in the event that an owner authority is
    * compromised by an attacker. 
    * 
    * The request must be accepted by the owner of the account, and is created by the listed recovery account.
    * Lasts for 24 hours before expiring
    */
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


   /**
    * Creates a request for a change to the listed recovery account of an account. 
    * 
    * The request has a 30 day delay to ensure that an attacker may not change the
    * recovery account to another malicious account.
    */
   class account_recovery_update_request_object : public object< account_recovery_update_request_object_type, account_recovery_update_request_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         account_recovery_update_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      account_to_recover;

         account_name_type      recovery_account;
         
         time_point             effective_on;
   };


   /**
    * Creates a request for an account to permanently revoke voting rights.
    * 
    * Activates after a delay period of 3 days.
    */
   class account_decline_voting_request_object : public object< account_decline_voting_request_object_type, account_decline_voting_request_object >
   {
      account_decline_voting_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_decline_voting_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                id;

         account_name_type      account;
         
         time_point             effective_date;
   };


   struct by_name;
   struct by_proxy;
   struct by_last_post;
   
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


   struct by_last_owner_update;


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
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< account_authority_id_type > 
            >
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
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_permission_id_type > 
            >
         >
      >,
      allocator< account_permission_object >
   > account_permission_index;

  
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

   

   struct by_name;


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
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_following_id_type > 
            >
         >
      >,
      allocator< account_following_object >
   > account_following_index;


   struct by_tag;


   typedef multi_index_container <
      account_tag_following_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_tag_following_object, account_tag_following_id_type, &account_tag_following_object::id > >,
         ordered_unique< tag< by_tag >,
            member< account_tag_following_object, tag_name_type, &account_tag_following_object::tag > >
      >,
      allocator< account_tag_following_object >
   > account_tag_following_index;

   
   struct by_expiration;
   struct by_accounts;
   struct by_account_a;
   struct by_account_b;
   struct by_last_message_time;


   typedef multi_index_container<
      account_connection_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< account_connection_object, account_connection_id_type, &account_connection_object::id > >,
         ordered_unique< tag< by_accounts >,
            composite_key< account_connection_object,
               member< account_connection_object, account_name_type, &account_connection_object::account_a >,
               member< account_connection_object, account_name_type, &account_connection_object::account_b >,
               member< account_connection_object, connection_tier_type, &account_connection_object::connection_type >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >, 
               std::greater< connection_tier_type > 
            >
         >,
         ordered_unique< tag< by_account_a >,
            composite_key< account_connection_object,
               member< account_connection_object, account_name_type, &account_connection_object::account_a >,
               member< account_connection_object, connection_tier_type, &account_connection_object::connection_type >,
               member< account_connection_object, account_connection_id_type, &account_connection_object::id > 
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< connection_tier_type >, 
               std::less<account_connection_id_type > 
            >
         >,
         ordered_unique< tag< by_account_b >,
            composite_key< account_connection_object,
               member< account_connection_object, account_name_type, &account_connection_object::account_b >,
               member< account_connection_object, connection_tier_type, &account_connection_object::connection_type >,
               member< account_connection_object, account_connection_id_type, &account_connection_object::id > 
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::greater< connection_tier_type >, 
               std::less< account_connection_id_type > 
            >
         >,
         ordered_unique< tag< by_last_message_time >,
            composite_key< account_connection_object,
               const_mem_fun< account_connection_object, time_point, &account_connection_object::last_message_time >,
               member< account_connection_object, account_connection_id_type, &account_connection_object::id > 
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< account_connection_id_type > 
            >
         >
      >,
      allocator< account_connection_object >
   > account_connection_index;


   struct by_account_req;
   struct by_req_account;
   struct by_account;
   struct by_last_valid;


   typedef multi_index_container <
      account_authority_history_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_authority_history_object, account_authority_history_id_type, &account_authority_history_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_authority_history_object,
               member< account_authority_history_object, account_name_type, &account_authority_history_object::account >,
               member< account_authority_history_object, time_point, &account_authority_history_object::last_valid_time >,
               member< account_authority_history_object, account_authority_history_id_type, &account_authority_history_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< time_point >, 
               std::less< account_authority_history_id_type > 
            >
         >
      >,
      allocator< account_authority_history_object >
   > account_authority_history_index;


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
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_recovery_request_id_type > 
            >
         >,
         ordered_unique< tag< by_expiration >,
            composite_key< account_recovery_request_object,
               member< account_recovery_request_object, time_point, &account_recovery_request_object::expiration >,
               member< account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< account_recovery_request_id_type > 
            >
         >
      >,
      allocator< account_recovery_request_object >
   > account_recovery_request_index;


   struct by_effective_date;


   typedef multi_index_container <
      account_recovery_update_request_object,
      indexed_by <
         ordered_unique< tag< by_id >,
            member< account_recovery_update_request_object, account_recovery_update_request_id_type, &account_recovery_update_request_object::id > >,
         ordered_unique< tag< by_account >,
            composite_key< account_recovery_update_request_object,
               member< account_recovery_update_request_object, account_name_type, &account_recovery_update_request_object::account_to_recover >,
               member< account_recovery_update_request_object, account_recovery_update_request_id_type, &account_recovery_update_request_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_recovery_update_request_id_type > 
            >
         >,
         ordered_unique< tag< by_effective_date >,
            composite_key< account_recovery_update_request_object,
               member< account_recovery_update_request_object, time_point, &account_recovery_update_request_object::effective_on >,
               member< account_recovery_update_request_object, account_recovery_update_request_id_type, &account_recovery_update_request_object::id >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< account_recovery_update_request_id_type > 
            >
         >
      >,
      allocator< account_recovery_update_request_object >
   > account_recovery_update_request_index;


   typedef multi_index_container<
      account_decline_voting_request_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< account_decline_voting_request_object, account_decline_voting_request_id_type, &account_decline_voting_request_object::id > >,
         ordered_unique< tag< by_account >,
            member< account_decline_voting_request_object, account_name_type, &account_decline_voting_request_object::account >
         >,
         ordered_unique< tag< by_effective_date >,
            composite_key< account_decline_voting_request_object,
               member< account_decline_voting_request_object, time_point, &account_decline_voting_request_object::effective_date >,
               member< account_decline_voting_request_object, account_name_type, &account_decline_voting_request_object::account >
            >,
            composite_key_compare< 
               std::less< time_point >, 
               std::less< account_name_type > 
            >
         >
      >,
      allocator< account_decline_voting_request_object >
   > account_decline_voting_request_index;


} }   // node::chain

FC_REFLECT( node::chain::account_object,
         (id)
         (name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (first_name)
         (last_name)
         (gender)
         (date_of_birth)
         (email)
         (phone)
         (nationality)
         (relationship)
         (political_alignment)
         (pinned_permlink)
         (interests)
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
         (reset_delay_days)
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
         (governance_members)
         (enterprise_vote_count)
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

FC_REFLECT( node::chain::account_permission_object,
         (id)
         (account)
         (whitelisted_accounts)
         (blacklisted_accounts)
         (whitelisted_assets)
         (blacklisted_assets)
         (whitelisted_communities)
         (blacklisted_communities)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_permission_object, node::chain::account_permission_index );

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
         (member_communities)
         (standard_premium_member_communities)
         (mid_premium_member_communities)
         (top_premium_member_communities)
         (moderator_communities)
         (admin_communities)
         (founder_communities)
         (followed_tags)
         (filtered)
         (filtered_communities)
         (filtered_tags)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_following_object, node::chain::account_following_index );

FC_REFLECT( node::chain::account_tag_following_object,
         (id)
         (tag)
         (followers)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_tag_following_object, node::chain::account_tag_following_index );

FC_REFLECT( node::chain::account_connection_object,
         (id)
         (account_a)
         (encrypted_key_a)
         (message_a)
         (json_a)
         (account_b)
         (encrypted_key_b)
         (message_b)
         (json_b)
         (connection_type)
         (connection_id)
         (message_count)
         (consecutive_days)
         (last_message_time_a)
         (last_message_time_b)
         (approved_a)
         (approved_b)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_connection_object, node::chain::account_connection_index );

FC_REFLECT( node::chain::account_authority_history_object,
         (id)
         (account)
         (previous_owner_authority)
         (last_valid_time)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_authority_history_object, node::chain::account_authority_history_index );

FC_REFLECT( node::chain::account_recovery_request_object,
         (id)
         (account_to_recover)
         (new_owner_authority)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_recovery_request_object, node::chain::account_recovery_request_index );

FC_REFLECT( node::chain::account_recovery_update_request_object,
         (id)
         (account_to_recover)
         (recovery_account)
         (effective_on)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::account_recovery_update_request_object, node::chain::account_recovery_update_request_index );

FC_REFLECT( node::chain::account_decline_voting_request_object,
         (id)
         (account)
         (effective_date) 
         );
            
CHAINBASE_SET_INDEX_TYPE( node::chain::account_decline_voting_request_object, node::chain::account_decline_voting_request_index );