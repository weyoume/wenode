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
    * Communities enable posts to be created within a 
    * subsection of the network, generally around a specific topic
    * or group of people. 
    * 
    * Moderators, and Administrators are able to be appointed to 
    * uphold content guidelines and generate a valuable posting and 
    * discussion environment.
    * 
    * Communities can create federation links to share and 
    * duplicate memberships, and create heirarchial or peer
    * structures of communities. 
    */ 
   class community_object : public object< community_object_type, community_object >
   {
      community_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         community_object( Constructor&& c, allocator< Allocator > a ) :
            display_name(a),
            details(a),
            url(a),
            profile_image(a),
            cover_image(a),
            json(a),
            json_private(a),
            pinned_permlink(a)
            {
               c(*this);
            };

         id_type                            id;

         community_name_type                name;                               ///< Name of the community, lowercase letters, numbers and hyphens only.

         account_name_type                  founder;                            ///< The account that created the community, able to add and remove administrators.

         shared_string                      display_name;                       ///< The full name of the community (non-consensus), encrypted with the member key if private community.

         shared_string                      details;                            ///< Describes the community and what it is about and the rules of posting, encrypted with the member key if private community.

         shared_string                      url;                                ///< Community URL link for more details, encrypted with the member key if private community.

         shared_string                      profile_image;                      ///< IPFS Reference of the Icon image of the community, encrypted with the member key if private community.

         shared_string                      cover_image;                        ///< IPFS Reference of the Cover image of the community, encrypted with the member key if private community.

         shared_string                      json;                               ///< Public plaintext JSON information about the community, its topic and rules.

         shared_string                      json_private;                       ///< Private ciphertext JSON information about the community, encrypted with the member key if private community.

         account_name_type                  pinned_author;                      ///< Author of Post pinned to the top of the community's page.

         shared_string                      pinned_permlink;                    ///< Permlink of Post pinned to the top of the community's page, encrypted with the member key if private community.

         flat_set< tag_name_type >          tags;                               ///< Set of tags of the topics within the community to enable discovery.

         community_privacy_type             community_privacy;                  ///< Community privacy level: Open_Public, General_Public, Exclusive_Public, Closed_Public, Open_Private, General_Private, Exclusive_Private, Closed_Private.

         public_key_type                    community_member_key;               ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

         public_key_type                    community_moderator_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

         public_key_type                    community_admin_key;                ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted Admin .

         account_name_type                  interface;                          ///< Account of the interface that broadcasted the transaction.
         
         asset_symbol_type                  reward_currency = SYMBOL_COIN;      ///< The Currency asset used for content rewards in the community.

         asset                              membership_price = asset( 0, SYMBOL_COIN ); ///< Price paid per week by all community members to community founder.

         uint16_t                           max_rating = 9;                     ///< Highest severity rating that posts in the community can have.

         uint32_t                           flags;                              ///< The currently active flags on the community for content settings.

         uint32_t                           permissions;                        ///< The flag permissions that can be activated on the community for content settings.

         uint32_t                           subscriber_count = 0;               ///< Number of accounts that are subscribed to the community.

         uint32_t                           post_count = 0;                     ///< Number of posts created in the community.
         
         uint32_t                           comment_count = 0;                  ///< Number of comments on posts in the community.

         uint32_t                           vote_count = 0;                     ///< Accumulated number of votes received by all posts in the community.

         uint32_t                           view_count = 0;                     ///< Accumulated number of views on posts in the community.

         uint32_t                           share_count = 0;                    ///< Accumulated number of shares on posts in the community.

         time_point                         created;                            ///< Time that the community was created.

         time_point                         last_updated;                       ///< Time that the community's details were last updated.

         time_point                         last_post;                          ///< Time that the user most recently created a comment.

         time_point                         last_root_post;                     ///< Time that the community last created a post.

         bool                               active;                             ///< True if the community is active, false to suspend all interaction.

         share_type membership_amount()const                                    ///< Returns the amount of the membership price of the community.
         {
            return membership_price.amount;
         }

         bool require_member_whitelist()const                                   ///< True if members must be whitelisted by founder.
         { 
            return ( flags & int( community_permission_flags::member_whitelist ) );
         }

         bool require_verified()const                                           ///< True if new members must have an existing verification from an current member to send a join request or be invited
         { 
            return ( flags & int( community_permission_flags::require_verified ) );
         }

         bool enable_messages()const                                            ///< True if Community messages are enabled
         { 
            return !( flags & int( community_permission_flags::disable_messages ) );
         }

         bool enable_text_posts()const                                          ///< True if text posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_text_posts ) );
         }

         bool enable_image_posts()const                                         ///< True if image posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_image_posts ) );
         }

         bool enable_gif_posts()const                                           ///< True if gif posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_gif_posts ) );
         }

         bool enable_video_posts()const                                         ///< True if video posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_video_posts ) );
         }

         bool enable_link_posts()const                                          ///< True if link posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_link_posts ) );
         }

         bool enable_article_posts()const                                       ///< True if article posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_article_posts ) );
         }

         bool enable_audio_posts()const                                         ///< True if audio posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_audio_posts ) );
         }

         bool enable_file_posts()const                                          ///< True if file posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_file_posts ) );
         }

         bool enable_livestream_posts()const                                    ///< True if livestream posts are enabled
         { 
            return !( flags & int( community_permission_flags::disable_livestream_posts ) );
         }
   };


   /**
    * Manages the membership, moderation, and administration lists of communities, 
    * implements community permissioning methods according to the following Table:
    * 
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Permissions         ||   O_PUB   |   G_PUB   |   E_PUB   |   C_PUB   |   O_PRI   |   G_PRI   |   E_PRI   |   C_PRI   |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========| 
    *   |   Read Posts          ||   All     |   All     |   All     |   All     |   Mem     |   Mem     |   Mem     |   Mem     |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========| 
    *   |   Share Posts         ||   All     |   All     |   All     |   Mem     |   None    |   None    |   None    |   None    |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Interact Posts      ||   All     |   All     |   All     |   Mem     |   Mem     |   Mem     |   Mem     |   Mem     |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Create Posts        ||   All     |   All     |   Mem     |   Mem     |   Mem     |   Mem     |   Mem     |   Mem     |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Invite+Accept       ||   Mem     |   Mem     |   Mem     |   Mod     |   Mod     |   Admin   |   Admin   |   Admin   |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Request Join        ||   All     |   All     |   All     |   All     |   All     |   All     |   None    |   None    |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Moderate posts      ||   Mod     |   Mod     |   Mod     |   Mod     |   Mod     |   Mod     |   Mod     |   Mod     |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Update community    ||   Admin   |   Admin   |   Admin   |   Admin   |   Admin   |   Admin   |   Admin   |   Admin   |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Blacklist Accounts  ||   None    |   Mod     |   Mod     |   Mod     |   Admin   |   Admin   |   Admin   |   Admin   |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    * 
    * A standard Practice implemented in interfaces would involve the creation of 8 communities
    * that act as concentric circles of trust within a group. New people would begin by joining
    * the outer public group, and be progressively advanced up the levels, until reaching the
    * Open Private community.
    * 
    * An automated mechanism for making and inviting a team of people to the desired level of community
    * would be created for managing the concentric rings of communities
    * and mirroring the memberships, admin and moderator roles across all lower levels for seperation
    * of communications and posts depending on the community level.
    * 
    * Downstream federated communities recieve all incoming membership and role changes, 
    * and create an automated hierarchial community structure.
    */
   class community_member_object : public object< community_member_object_type, community_member_object >
   {
      community_member_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_member_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                        id;

         community_name_type                            name;                                 ///< Name of the community with permissions set.

         account_name_type                              founder;                              ///< Name of the founding account of the community. Has full permissions.

         community_privacy_type                         community_privacy;                    ///< Community privacy level: Open_Public, General_Public, Exclusive_Public, Closed_Public, Open_Private, General_Private, Exclusive_Private, Closed_Private.

         flat_set< account_name_type >                  subscribers;                          ///< List of accounts that subscribe to the posts made in the community.

         flat_set< account_name_type >                  members;                              ///< List of accounts that are permitted to post in the community. Can invite and accept on public communities.
 
         flat_set< account_name_type >                  moderators;                           ///< Accounts able to filter posts. Can invite and accept on private communities.

         flat_set< account_name_type >                  administrators;                       ///< Accounts able to add and remove moderators and update community details. Can invite and accept on Exclusive communities.

         flat_set< account_name_type >                  blacklist;                            ///< Accounts that are not able to post in this community, or request to join.

         flat_set< community_name_type >                upstream_member_federations;          ///< Communities that this community recieves incoming members from.

         flat_set< community_name_type >                upstream_moderator_federations;       ///< Communities that this community recieves incoming moderators from.

         flat_set< community_name_type >                upstream_admin_federations;           ///< Communities that this community recieves incoming admins from.

         flat_set< community_name_type >                downstream_member_federations;        ///< Communities that Receive incoming members from this community.

         flat_set< community_name_type >                downstream_moderator_federations;     ///< Communities that Receive incoming moderators from this community.

         flat_set< community_name_type >                downstream_admin_federations;         ///< Communities that Receive incoming admins from this community.

         flat_map< account_name_type, share_type >      mod_weight;                           ///< Map of all moderator voting weights for distributing rewards.

         share_type                                     total_mod_weight = 0;                 ///< Total of all moderator weights.

         time_point                                     last_updated;                         ///< Time that the community was last updated.

         /**
          * Adjacency value determines how similar two accounts are by comparing the 
          * accounts, communities and tags that they have in common with eachother. 
          * this value is used for the determination of post recommendations.
          */
         share_type adjacency_value( const community_member_object& m )const
         {
            vector< account_name_type > common_subscribers;
            common_subscribers.reserve( subscribers.size() );
            std::set_intersection( m.subscribers.begin(), m.subscribers.end(), subscribers.begin(), subscribers.end(), common_subscribers.begin() );

            vector< account_name_type > common_members;
            common_members.reserve( members.size() );
            std::set_intersection( m.members.begin(), m.members.end(), members.begin(), members.end(), common_members.begin() );

            vector< account_name_type > common_moderators;
            common_moderators.reserve( moderators.size() );
            std::set_intersection( m.moderators.begin(), m.moderators.end(), moderators.begin(), moderators.end(), common_moderators.begin() );

            vector< account_name_type > common_administrators;
            common_administrators.reserve( administrators.size() );
            std::set_intersection( m.administrators.begin(), m.administrators.end(), administrators.begin(), administrators.end(), common_administrators.begin() );

            share_type result = common_subscribers.size() + 3*common_members.size() + 5*common_moderators.size() + 10*common_administrators.size();
            return result;
         };

         /**
          * Determines Permission to create a new root post in the community.
          */
         bool is_authorized_author( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( community_privacy )
            {
               case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
               case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
               {
                  return true;
               }
               break;
               case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
               case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               {
                  return is_member( account );
               }
               break;
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  return is_moderator( account );
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community privacy: ${t}.",
                     ("t",community_privacy) );
               }
            }
         };

         /**
          * Determines Permission to vote, view, comment or share posts in the community.
          */
         bool is_authorized_interact( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( community_privacy )
            {
               case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
               case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
               {
                  return true;
               }
               break;
               case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  return is_member( account );
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community privacy: ${t}.",
                     ("t",community_privacy) );
               }
            }
         };

         /**
          * Determines Permission to request to join a community.
          */
         bool is_authorized_request( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( community_privacy )
            {
               case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
               case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
               case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               {
                  return true;
               }
               break;
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community privacy: ${t}.",
                     ("t",community_privacy) );
               }
            }
         };

         /**
          * Determines Permission to send invites, accept incoming community join requests.
          */
         bool is_authorized_invite( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( community_privacy )
            {
               case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
               case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
               {
                  return is_member( account );
               }
               break;
               case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               {
                  return is_moderator( account );
               }
               break;
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  return is_administrator( account );
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community privacy: ${t}.",
                     ("t",community_privacy) );
               }
            }
         };

         /**
          * Determines Permission to blacklist an account from the community.
          */
         bool is_authorized_blacklist( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( community_privacy )
            {
               case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
               {
                  return false;
               }
               break;
               case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
               case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
               {
                  return is_moderator( account );
               }
               break;
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  return is_administrator( account );
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community privacy: ${t}.",
                     ("t",community_privacy) );
               }
            }
         };

         bool is_subscriber( const account_name_type& account )const
         {
            return std::find( subscribers.begin(), subscribers.end(), account ) != subscribers.end();
         };

         bool is_member( const account_name_type& account )const
         {
            return std::find( members.begin(), members.end(), account ) != members.end();
         };
      
         bool is_moderator( const account_name_type& account )const
         {
            return std::find( moderators.begin(), moderators.end(), account ) != moderators.end();
         };

         bool is_administrator( const account_name_type& account )const
         {
            return std::find( administrators.begin(), administrators.end(), account ) != administrators.end();
         };

         bool is_blacklisted( const account_name_type& account )const
         {
            return std::find( blacklist.begin(), blacklist.end(), account ) != blacklist.end();
         };
   };


   /**
    * Community Member Keys hold the private decryption key, 
    * encrypted with the secure key of the member of a community.
    * 
    * The member connected into the community can then refer to this 
    * decryption key to unlock private posts
    * created within the private community.
    */
   class community_member_key_object : public object< community_member_key_object_type, community_member_key_object >
   {
      community_member_key_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_member_key_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                          id;                 

         account_name_type                account;                    ///< Account that created the Community key for the new member.

         account_name_type                member;                     ///< Account of the new community member.

         community_name_type              community;                  ///< Community that the key enables access to.

         community_federation_type        community_key_type;         ///< Key encryption level of this key object.

         encrypted_keypair_type           encrypted_community_key;    ///< The community's private key, encrypted with the member's secure public key.
   };


   /**
    * Community moderator votes elect the moderators of a community.
    * 
    * The highest voted moderators get a larger share of the moderation rewards of a community.
    */
   class community_moderator_vote_object : public object< community_moderator_vote_object_type, community_moderator_vote_object >
   {
      community_moderator_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_moderator_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                   id;

         account_name_type         account;         ///< The name of the account that is voting for the moderator.

         community_name_type       community;       ///< Community that the moderator is being voted into.

         account_name_type         moderator;       ///< The name of the moderator being voted for.

         uint16_t                  vote_rank = 1;   ///< The rank of the vote for the community moderator.

         time_point                last_updated;    ///< Time that the vote was last updated.

         time_point                created;         ///< The time the vote was created.
   };


   /**
    * Community Join request enables an account to seek to join a community.
    * 
    * Contains a message from the account to the moderators of the community.
    */
   class community_join_request_object : public object< community_join_request_object_type, community_join_request_object >
   {
      community_join_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_join_request_object( Constructor&& c, allocator< Allocator > a ) :
         message(a)
         {
            c( *this );
         }

         id_type                    id;                 

         account_name_type          account;          ///< Account that created the request.

         community_name_type        community;        ///< Community being requested to join.

         shared_string              message;          ///< Encrypted message to the communities management team, encrypted with community public key.

         time_point                 expiration;       ///< Request expiry time.
   };


   /**
    * Community Join invite enables a community moderator to introduce a new member to a community.
    * 
    * Contains a message from the moderator to the account.
    */
   class community_join_invite_object : public object< community_join_invite_object_type, community_join_invite_object >
   {
      community_join_invite_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_join_invite_object( Constructor&& c, allocator< Allocator > a ) :
         message(a)
         {
            c( *this );
         }

         id_type                    id;                 

         account_name_type          account;                ///< Account that created the invite.

         account_name_type          member;                 ///< Account being invited to join the community membership.

         community_name_type        community;              ///< Community being invited to join.

         shared_string              message;                ///< Encrypted message to the communities management team, encrypted with the members secure public key.

         time_point                 expiration;             ///< Invite expiry time.
   };


   /**
    * Community Federations enable communities to connect together.
    * 
    * Communities share their private decryption keys with eachother securely, 
    * and every holder of the private key of one community is granted access to the other, 
    * enabling content decryption and message decryption.
    * 
    * Federations can be directed or mutual, which can pass memberships from
    * one to another automatically. 
    */
   class community_federation_object : public object< community_federation_object_type, community_federation_object >
   {
      community_federation_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_federation_object( Constructor&& c, allocator< Allocator > a ) :
            message_a(a),
            json_a(a),
            message_b(a),
            json_b(a),
            federation_id(a)
            {
               c( *this );
            }

         id_type                          id;                 

         community_name_type              community_a;                   ///< Community with the lower ID.

         encrypted_keypair_type           encrypted_key_a;               ///< A's private community key, encrypted with the equivalent private community key of Community B.

         shared_string                    message_a;                     ///< Encrypted message from A to the community B membership, encrypted with equivalent community public key.

         shared_string                    json_a;                        ///< Encrypted JSON metadata from A to community B, encrypted with equivalent community public key.

         community_name_type              community_b;                   ///< Community with the higher ID.

         encrypted_keypair_type           encrypted_key_b;               ///< B's private community key, encrypted with the equivalent private community key of Community A.

         shared_string                    message_b;                     ///< Encrypted message from B to the community A membership, encrypted with equivalent community public key.

         shared_string                    json_b;                        ///< Encrypted JSON metadata from B to community A, encrypted with equivalent community public key.

         community_federation_type        federation_type;               ///< Determines the level of Federation between the Communities.

         shared_string                    federation_id;                 ///< Reference uuidv4 for the federation, for local storage of decryption keys.

         bool                             share_accounts_a;              ///< True when community A accepts incoming memberships from B.

         bool                             share_accounts_b;              ///< True when community B accepts incoming memberships from A.

         bool                             approved_a;                    ///< True when community A approves the federation.

         bool                             approved_b;                    ///< True when account B approves the federation.

         time_point                       last_updated;                  ///< Time the connection keys were last updated.

         time_point                       created;                       ///< Time the connection was created.

         bool                             approved()const
         {
            return approved_a && approved_b;
         }
   };


   /**
    * Community Events enables accounts to coordinate an event to gather people together.
    * 
    * Describes the event, and the location that the event is taking place at.
    * Accounts indicate whether or not they are attending.
    * 
    * All members of the Community that the Event is created within are invited to attend.
    * Admins within a community can create and update Events. 
    * Event price amounts are paid to the community founder. 
    */
   class community_event_object : public object< community_event_object_type, community_event_object >
   {
      community_event_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_event_object( Constructor&& c, allocator< Allocator > a ) :
            event_id(a), 
            event_name(a), 
            location(a), 
            details(a), 
            url(a), 
            json(a)
            {
               c( *this );
            }

         id_type                               id;                 

         community_name_type                   community;              ///< Community hosting the event, and all members are being invited to attend.

         shared_string                         event_id;               ///< UUIDv4 referring to the event within the Community. Unique on community/event_id.

         public_key_type                       public_key;             ///< Public key for encrypting the event details. Null if public event.

         shared_string                         event_name;             ///< The Display Name of the event. Encrypted for private events.

         shared_string                         location;               ///< Address location of the event. Encrypted for private events.

         double                                latitude;               ///< Latitude co-ordinates of the event.

         double                                longitude;              ///< Longitude co-ordinates of the event.

         shared_string                         details;                ///< Event details describing the purpose of the event. Encrypted for private events.

         shared_string                         url;                    ///< Reference URL for the event.

         shared_string                         json;                   ///< Additional Event JSON metadata. Encrypted for private events.

         account_name_type                     interface;              ///< Account of the interface that broadcasted the transaction.

         asset                                 event_price;            ///< Amount paid to join the attending list as a ticket holder to the event.

         uint64_t                              interested;             ///< Number of accounts that are interested in the event.

         uint64_t                              attending;              ///< Number of accounts that have confirmed that they will be attending the event and paid event price.

         uint64_t                              not_attending;          ///< Number of accounts that have confirmed that they will not be attending the event.

         time_point                            event_start_time;       ///< Time that the Event will begin.

         time_point                            event_end_time;         ///< Time that the Event will end.

         time_point                            last_updated;           ///< Time that the event was last updated.

         time_point                            created;                ///< Time that the event was created.
   };


   /**
    * Nominates an account as attending a community event
    * 
    * Accounts that nominate an event as Interested see posts relating to the event
    * Accounts that nominate an event as Attending determine that they are going
    * and have paid the event price.
    */
   class community_event_attend_object : public object< community_event_attend_object_type, community_event_attend_object >
   {
      community_event_attend_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_event_attend_object( Constructor&& c, allocator< Allocator > a ) :
            event_id(a),
            message(a), 
            json(a)
            {
               c( *this );
            }

         id_type                               id;

         account_name_type                     attendee;               ///< Account that is attending the event.

         community_name_type                   community;              ///< Community hosting the event, and all members are being invited to attend.

         shared_string                         event_id;               ///< UUIDv4 referring to the event within the Community. Unique on community/event_id.

         public_key_type                       public_key;             ///< Public key for encrypting the event details. Null if public event.

         account_name_type                     interface;              ///< Account of the interface that broadcasted the transaction.

         shared_string                         message;                ///< Encrypted message to the community operating the event.

         shared_string                         json;                   ///< Additional Event Attendance JSON metadata.

         bool                                  interested;             ///< True when the attendee is interested in the event.

         bool                                  attending;              ///< True when the attendee has confirmed that they will be attending the event, and paid the event price.

         time_point                            last_updated;           ///< Time that the attendance was last updated.

         time_point                            created;                ///< Time that the attendance was created.
   };


   struct by_name;
   struct by_founder;
   struct by_last_post;
   struct by_subscriber_count;
   struct by_post_count;
   struct by_vote_count;
   struct by_view_count;
   struct by_membership_price;


   typedef multi_index_container<
      community_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_object, community_id_type, &community_object::id > >,
         ordered_unique< tag< by_name >,
            member< community_object, community_name_type, &community_object::name > >,
         ordered_unique< tag< by_founder >,
            composite_key< community_object,
               member< community_object, account_name_type, &community_object::founder >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< community_id_type > 
            >
         >,
         ordered_unique< tag< by_last_post >,
            composite_key< community_object,
               member< community_object, time_point, &community_object::last_post >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< 
               std::greater< time_point >, 
               std::less< community_id_type > 
            >
         >,
         ordered_unique< tag< by_subscriber_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::subscriber_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< community_id_type > 
            >
         >,
         ordered_unique< tag< by_post_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::post_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< community_id_type > 
            >
         >,
         ordered_unique< tag< by_view_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::view_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< community_id_type > 
            >
         >,
         ordered_unique< tag< by_vote_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::vote_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< 
               std::greater< uint32_t >, 
               std::less< community_id_type > 
            >
         >,
         ordered_unique< tag< by_membership_price >,
            composite_key< community_object,
               const_mem_fun< community_object, share_type, &community_object::membership_amount >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< 
               std::greater< share_type >, 
               std::less< community_id_type > 
            >
         >
      >,
      allocator< community_object >
   > community_index;

   struct by_name;

   typedef multi_index_container<
      community_member_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_member_object, community_member_id_type, &community_member_object::id > >,
         ordered_unique< tag< by_name >,
            member< community_member_object, community_name_type, &community_member_object::name > > 
      >,
      allocator< community_member_object >
   > community_member_index;

   struct by_member_community_type;

   typedef multi_index_container<
      community_member_key_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_member_key_object, community_member_key_id_type, &community_member_key_object::id > >,
         ordered_unique< tag< by_member_community_type >,
            composite_key< community_member_key_object,
               member< community_member_key_object, account_name_type, &community_member_key_object::member >, 
               member< community_member_key_object, community_name_type, &community_member_key_object::community >,
               member< community_member_key_object, community_federation_type, &community_member_key_object::community_key_type >
            >
         >
      >,
      allocator< community_member_key_object >
   > community_member_key_index;


   struct by_community;
   struct by_community_moderator;
   struct by_account;
   struct by_moderator_community_rank;
   struct by_account_community_moderator;
   struct by_account_community_rank;


   typedef multi_index_container<
      community_moderator_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_moderator_vote_object, community_moderator_vote_id_type, &community_moderator_vote_object::id > >,
         ordered_unique< tag< by_community >,
            composite_key< community_moderator_vote_object,
               member< community_moderator_vote_object, community_name_type, &community_moderator_vote_object::community >,
               member< community_moderator_vote_object, community_moderator_vote_id_type, &community_moderator_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< community_moderator_vote_object,
               member< community_moderator_vote_object, account_name_type, &community_moderator_vote_object::account >,
               member< community_moderator_vote_object, community_moderator_vote_id_type, &community_moderator_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_community_rank >,
            composite_key< community_moderator_vote_object,
               member< community_moderator_vote_object, account_name_type, &community_moderator_vote_object::account >,
               member< community_moderator_vote_object, community_name_type, &community_moderator_vote_object::community >,
               member< community_moderator_vote_object, uint16_t, &community_moderator_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_moderator_community_rank >,
            composite_key< community_moderator_vote_object,
               member< community_moderator_vote_object, account_name_type, &community_moderator_vote_object::moderator >,
               member< community_moderator_vote_object, community_name_type, &community_moderator_vote_object::community >,
               member< community_moderator_vote_object, uint16_t, &community_moderator_vote_object::vote_rank >,
               member< community_moderator_vote_object, community_moderator_vote_id_type, &community_moderator_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_community_moderator >,
            composite_key< community_moderator_vote_object,
               member< community_moderator_vote_object, account_name_type, &community_moderator_vote_object::account >,
               member< community_moderator_vote_object, community_name_type, &community_moderator_vote_object::community >,
               member< community_moderator_vote_object, account_name_type, &community_moderator_vote_object::moderator >
            >
         >,
         ordered_unique< tag< by_community_moderator >,
            composite_key< community_moderator_vote_object,
               member< community_moderator_vote_object, community_name_type, &community_moderator_vote_object::community >,
               member< community_moderator_vote_object, account_name_type, &community_moderator_vote_object::moderator >,
               member< community_moderator_vote_object, community_moderator_vote_id_type, &community_moderator_vote_object::id >
            >
         >
      >,
      allocator< community_moderator_vote_object >
   > community_moderator_vote_index;

   struct by_account_community;
   struct by_community_account;
   struct by_expiration;

   typedef multi_index_container<
      community_join_request_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_join_request_object, community_join_request_id_type, &community_join_request_object::id > >,
         ordered_non_unique< tag< by_expiration >,
            member< community_join_request_object, time_point, &community_join_request_object::expiration > >,
         ordered_unique< tag< by_account_community >,
            composite_key< community_join_request_object,
               member< community_join_request_object, account_name_type, &community_join_request_object::account >,
               member< community_join_request_object, community_name_type, &community_join_request_object::community >
            >
         >,
         ordered_unique< tag< by_community_account >,
            composite_key< community_join_request_object,
               member< community_join_request_object, community_name_type, &community_join_request_object::community >,
               member< community_join_request_object, account_name_type, &community_join_request_object::account >
            >
         > 
      >,
      allocator< community_join_request_object >
   > community_join_request_index;

   struct by_member_community;
   struct by_community;
   struct by_account;
   struct by_member;

   typedef multi_index_container<
      community_join_invite_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_join_invite_object, community_join_invite_id_type, &community_join_invite_object::id > >,
         ordered_non_unique< tag< by_expiration >,
            member< community_join_invite_object, time_point, &community_join_invite_object::expiration > >,
         ordered_unique< tag< by_member_community >,
            composite_key< community_join_invite_object,
               member< community_join_invite_object, account_name_type, &community_join_invite_object::member >, 
               member< community_join_invite_object, community_name_type, &community_join_invite_object::community > 
            >
         >,
         ordered_unique< tag< by_community >,
            composite_key< community_join_invite_object,
               member< community_join_invite_object, community_name_type, &community_join_invite_object::community >,
               member< community_join_invite_object, community_join_invite_id_type, &community_join_invite_object::id >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< community_join_invite_object,
               member< community_join_invite_object, account_name_type, &community_join_invite_object::account >,
               member< community_join_invite_object, community_join_invite_id_type, &community_join_invite_object::id >
            >
         >,
         ordered_unique< tag< by_member >,
            composite_key< community_join_invite_object,
               member< community_join_invite_object, account_name_type, &community_join_invite_object::member >,
               member< community_join_invite_object, community_join_invite_id_type, &community_join_invite_object::id >
            >
         >
      >,
      allocator< community_join_invite_object >
   > community_join_invite_index;


   struct by_communities;
   struct by_community_a;
   struct by_community_b;


   typedef multi_index_container<
      community_federation_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_federation_object, community_federation_id_type, &community_federation_object::id > >,
         ordered_unique< tag< by_communities >,
            composite_key< community_federation_object,
               member< community_federation_object, community_name_type, &community_federation_object::community_a >,
               member< community_federation_object, community_name_type, &community_federation_object::community_b >,
               member< community_federation_object, community_federation_type, &community_federation_object::federation_type >
            >,
            composite_key_compare< 
               std::less< community_name_type >, 
               std::less< community_name_type >, 
               std::greater< community_federation_type > 
            >
         >,
         ordered_unique< tag< by_community_a >,
            composite_key< community_federation_object,
               member< community_federation_object, community_name_type, &community_federation_object::community_a >,
               member< community_federation_object, community_federation_type, &community_federation_object::federation_type >,
               member< community_federation_object, community_federation_id_type, &community_federation_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >, 
               std::greater< community_federation_type >,
               std::less< community_federation_id_type >
            >
         >,
         ordered_unique< tag< by_community_b >,
            composite_key< community_federation_object,
               member< community_federation_object, community_name_type, &community_federation_object::community_b >,
               member< community_federation_object, community_federation_type, &community_federation_object::federation_type >,
               member< community_federation_object, community_federation_id_type, &community_federation_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >, 
               std::greater< community_federation_type >,
               std::less< community_federation_id_type >
            >
         >
      >,
      allocator< community_federation_object >
   > community_federation_index;

   

   struct by_start_time;
   struct by_end_time;
   struct by_last_updated;
   struct by_community;
   struct by_event_id;
   struct by_community_time;


   typedef multi_index_container<
      community_event_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_event_object, community_event_id_type, &community_event_object::id > >,
         ordered_non_unique< tag< by_start_time >,
            member< community_event_object, time_point, &community_event_object::event_start_time > 
         >,
         ordered_non_unique< tag< by_end_time >,
            member< community_event_object, time_point, &community_event_object::event_end_time > 
         >,
         ordered_non_unique< tag< by_last_updated >,
            member< community_event_object, time_point, &community_event_object::last_updated > 
         >,
         ordered_unique< tag< by_event_id >,
            composite_key< community_event_object,
               member< community_event_object, community_name_type, &community_event_object::community >,
               member< community_event_object, shared_string, &community_event_object::event_id >
            >,
            composite_key_compare< 
               std::less< community_name_type >, 
               strcmp_less
            >
         >,
         ordered_unique< tag< by_community_time >,
            composite_key< community_event_object,
               member< community_event_object, community_name_type, &community_event_object::community >,
               member< community_event_object, time_point, &community_event_object::event_start_time >,
               member< community_event_object, community_event_id_type, &community_event_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >,
               std::less< time_point >,
               std::less< community_event_id_type >
            >
         >
      >,
      allocator< community_event_object >
   > community_event_index;


   struct by_attending_event_id;
   struct by_attendee;


   typedef multi_index_container<
      community_event_attend_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_event_attend_object, community_event_attend_id_type, &community_event_attend_object::id > >,
         ordered_unique< tag< by_event_id >,
            composite_key< community_event_attend_object,
               member< community_event_attend_object, community_name_type, &community_event_attend_object::community >,
               member< community_event_attend_object, shared_string, &community_event_attend_object::event_id >,
               member< community_event_attend_object, account_name_type, &community_event_attend_object::attendee >
            >,
            composite_key_compare< 
               std::less< community_name_type >,
               strcmp_less,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_attending_event_id >,
            composite_key< community_event_attend_object,
               member< community_event_attend_object, bool, &community_event_attend_object::attending >,
               member< community_event_attend_object, community_name_type, &community_event_attend_object::community >,
               member< community_event_attend_object, shared_string, &community_event_attend_object::event_id >,
               member< community_event_attend_object, account_name_type, &community_event_attend_object::attendee >
            >,
            composite_key_compare<
               std::less< bool >,
               std::less< community_name_type >,
               strcmp_less,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_attendee >,
            composite_key< community_event_attend_object,
               member< community_event_attend_object, account_name_type, &community_event_attend_object::attendee >,
               member< community_event_attend_object, community_event_attend_id_type, &community_event_attend_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >,
               std::less< community_event_attend_id_type >
            >
         >
      >,
      allocator< community_event_attend_object >
   > community_event_attend_index;

} } // node::chain


FC_REFLECT( node::chain::community_object,
         (id)
         (name)
         (founder)
         (display_name)
         (details)
         (url)
         (profile_image)
         (cover_image)
         (json)
         (json_private)
         (pinned_author)
         (pinned_permlink)
         (tags)
         (community_privacy)
         (community_member_key)
         (community_moderator_key)
         (community_admin_key)
         (interface)
         (reward_currency)
         (membership_price)
         (max_rating)
         (flags)
         (permissions)
         (subscriber_count)
         (post_count)
         (comment_count)
         (vote_count)
         (view_count)
         (share_count)
         (created)
         (last_updated)
         (last_post)
         (last_root_post)
         (active)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_object, node::chain::community_index );

FC_REFLECT( node::chain::community_member_object,
         (id)
         (name)
         (founder)
         (community_privacy)
         (subscribers)
         (members)
         (moderators)
         (administrators)
         (blacklist)
         (upstream_member_federations)
         (upstream_moderator_federations)
         (upstream_admin_federations)
         (downstream_member_federations)
         (downstream_moderator_federations)
         (downstream_admin_federations)
         (mod_weight)
         (total_mod_weight)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_member_object, node::chain::community_member_index );

FC_REFLECT( node::chain::community_moderator_vote_object,
         (id)
         (account)
         (community)
         (moderator)
         (vote_rank)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_moderator_vote_object, node::chain::community_moderator_vote_index );

FC_REFLECT( node::chain::community_join_request_object,
         (id)
         (account)
         (community)
         (message)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_join_request_object, node::chain::community_join_request_index );

FC_REFLECT( node::chain::community_join_invite_object,
         (id)
         (account)
         (member)
         (community)
         (message)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_join_invite_object, node::chain::community_join_invite_index );

FC_REFLECT( node::chain::community_member_key_object,
         (id)
         (account)
         (member)
         (community)
         (community_key_type)
         (encrypted_community_key)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_member_key_object, node::chain::community_member_key_index );

FC_REFLECT( node::chain::community_federation_object,
         (id)
         (community_a)
         (encrypted_key_a)
         (message_a)
         (json_a)
         (community_b)
         (encrypted_key_b)
         (message_b)
         (json_b)
         (federation_type)
         (federation_id)
         (share_accounts_a)
         (share_accounts_b)
         (approved_a)
         (approved_b)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_federation_object, node::chain::community_federation_index );

FC_REFLECT( node::chain::community_event_object,
         (id)
         (community)
         (event_id)
         (public_key)
         (event_name)
         (location)
         (latitude)
         (longitude)
         (details)
         (url)
         (json)
         (interface)
         (event_price)
         (interested)
         (attending)
         (not_attending)
         (event_start_time)
         (event_end_time)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_event_object, node::chain::community_event_index );

FC_REFLECT( node::chain::community_event_attend_object,
         (id)
         (attendee)
         (community)
         (event_id)
         (public_key)
         (interface)
         (message)
         (json)
         (interested)
         (attending)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_event_attend_object, node::chain::community_event_attend_index );