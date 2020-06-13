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
    */ 
   class community_object : public object< community_object_type, community_object >
   {
      community_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         community_object( Constructor&& c, allocator< Allocator > a ) :
         details(a),
         url(a),
         json(a),
         json_private(a),
         pinned_permlink(a)
         {
            c(*this);
         };

         id_type                            id;

         community_name_type                name;                               ///< Name of the community, lowercase letters, numbers and hyphens only.

         account_name_type                  founder;                            ///< The account that created the community, able to add and remove administrators.

         community_privacy_type             community_privacy;                  ///< Community privacy level, open, public, private, or exclusive

         public_key_type                    community_public_key;               ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

         shared_string                      details;                            ///< Describes the community and what it is about and the rules of posting.

         shared_string                      url;                                ///< Community URL link for more details.

         shared_string                      json;                               ///< Public plaintext json information about the community, its topic and rules.

         shared_string                      json_private;                       ///< Private ciphertext json information about the community.

         account_name_type                  pinned_author;                      ///< Author of Post pinned to the top of the community's page.

         shared_string                      pinned_permlink;                    ///< Permlink of Post pinned to the top of the community's page.
         
         uint16_t                           max_rating = 9;                     ///< Highest severity rating that posts in the community can have.

         uint32_t                           flags = 0;                          ///< The currently active flags on the community for content settings.

         uint32_t                           permissions;                        ///< The flag permissions that can be activated on the community for content settings. 

         uint32_t                           subscriber_count = 0;               ///< number of accounts that are subscribed to the community.

         uint32_t                           post_count = 0;                     ///< number of posts created in the community.
         
         uint32_t                           comment_count = 0;                  ///< number of comments on posts in the community.

         uint32_t                           vote_count = 0;                     ///< accumulated number of votes received by all posts in the community.

         uint32_t                           view_count = 0;                     ///< accumulated number of views on posts in the community.

         uint32_t                           share_count = 0;                    ///< accumulated number of shares on posts in the community.

         asset_symbol_type                  reward_currency = SYMBOL_COIN;      ///< The Currency asset used for content rewards in the community. 

         time_point                         created;                            ///< Time that the community was created.

         time_point                         last_updated;                       ///< Time that the community's details were last updated.

         time_point                         last_post;                          ///< Time that the user most recently created a comment.

         time_point                         last_root_post;                     ///< Time that the community last created a post.

         bool                               active;                             ///< True if the community is active, false to suspend all interaction.

         bool require_member_whitelist()const                                   ///< True if members must be whitelisted by founder.
         { 
            return ( flags & int( community_permission_flags::member_whitelist ) );
         }

         bool require_profile()const                                            ///< True if members must be have profile data.
         { 
            return ( flags & int( community_permission_flags::require_profile ) );
         }

         bool require_verified()const                                           ///< True if new members must be have verification from an existing member
         { 
            return ( flags & int( community_permission_flags::require_verified ) );
         }

         bool enable_messages()const                                            ///< True if Image Posts are enabled
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
    *   |   Read Posts          ||   All     |   All     |   All     |   All     |   Mems    |   Mems    |   Mems    |   Mems    |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Interact Posts      ||   All     |   All     |   All     |   Mems    |   Mems    |   Mems    |   Mems    |   Mems    |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Create Posts        ||   All     |   All     |   Mems    |   Mems    |   Mems    |   Mems    |   Mems    |   Mods    |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Invite+Accept       ||   Mems    |   Mems    |   Mems    |   Mods    |   Mods    |   Mods    |   Mods    |   Admins  |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Request Join        ||   All     |   All     |   All     |   All     |   All     |   All     |   None    |   None    |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Moderate posts      ||   Mods    |   Mods    |   Mods    |   Mods    |   Mods    |   Mods    |   Mods    |   Mods    |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Update community    ||   Admins  |   Admins  |   Admins  |   Admins  |   Admins  |   Admins  |   Admins  |   Admins  |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    *   |   Blacklist Accounts  ||   None    |   Mods    |   Mods    |   Mods    |   Admins  |   Admins  |   Admins  |   Admins  |
    *   |=======================++===========+===========+===========+===========+===========+===========+===========+===========|
    * 
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

         community_name_type                            name;                        ///< Name of the community with permissions set.

         account_name_type                              founder;                     ///< Name of the founding account of the community. Has full permissions.

         community_privacy_type                         community_privacy;           ///< Privacy setting of community, determines transaction authorization. 

         flat_set< account_name_type >                  subscribers;                 ///< List of accounts that subscribe to the posts made in the community.

         flat_set< account_name_type >                  members;                     ///< List of accounts that are permitted to post in the community. Can invite and accept on public communities
 
         flat_set< account_name_type >                  moderators;                  ///< Accounts able to filter posts. Can invite and accept on private communities.

         flat_set< account_name_type >                  administrators;              ///< Accounts able to add and remove moderators and update community details. Can invite and accept on Exclusive communities. 

         flat_set< account_name_type >                  blacklist;                   ///< Accounts that are not able to post in this community, or request to join.

         flat_map< account_name_type, share_type >      mod_weight;                  ///< Map of all moderator voting weights for distributing rewards. 

         share_type                                     total_mod_weight = 0;        ///< Total of all moderator weights. 

         time_point                                     last_updated;                 ///< Time that the community was last updated.

         /**
          * Adjacency value determines how similar two accounts are by comparing the 
          * accounts, communities and tags that they have in common with eachother. 
          * this value is used for the determination of post recommendations.
          */
         share_type                                 adjacency_value( const community_member_object& m )const
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
            if( is_blacklisted( account) )
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
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  if( is_member( account ) )
                  {
                     return true;
                  }
                  else
                  {
                     return false;
                  }
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
                  if( is_member( account ) )
                  {
                     return true;
                  }
                  else
                  {
                     return false;
                  } 
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
               {
                  return false; 
               }
               break;
               case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
               case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  return true;
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
                  if( is_member( account ) )
                  {
                     return true;
                  }
                  else
                  {
                     return false;
                  }  
               }
               break;
               case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               {
                  if( is_moderator( account ) )
                  {
                     return true;
                  }
                  else
                  {
                     return false;
                  }
               }
               break;
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  if( is_administrator( account ) )
                  {
                     return true;
                  }
                  else
                  {
                     return false;
                  }
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
                  if( is_moderator( account ) )
                  {
                     return true;
                  }
                  else
                  {
                     return false;
                  }
               }
               break;
               case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
               case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
               case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
               case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
               {
                  if( is_administrator( account ) )
                  {
                     return true;
                  }
                  else
                  {
                     return false;
                  }
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

         void                              add_subscriber( const account_name_type& account )
         {
            if( !is_subscriber( account )  )
            {
               subscribers.insert( account );
            }
         }

         void                              remove_subscriber( const account_name_type& account )
         {
            if( is_subscriber( account ) )
            {
               subscribers.erase( account );
            }
         }  

         void                              add_member( const account_name_type& account )
         {
            if( !is_member( account )  )
            {
               members.insert( account );
            }
         }

         void                              remove_member( const account_name_type& account )
         {
            if( is_member( account ) )
            {
               members.erase( account );
            }
         }

         void                              add_moderator( const account_name_type& account )
         {
            if( !is_moderator( account )  )
            {
               moderators.insert( account );
            }
         }

         void                              remove_moderator( const account_name_type& account )
         {
            if( is_moderator( account ) )
            {
               moderators.erase( account );
            }
         }
   };


   class community_member_key_object : public object< community_member_key_object_type, community_member_key_object >
   {
      community_member_key_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_member_key_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;                 

         account_name_type          account;                    ///< Account that created the Community key for the new member.

         account_name_type          member;                     ///< Account of the new community member.

         community_name_type        community;                  ///< Community that the key enables access to.

         encrypted_keypair_type     encrypted_community_key;    ///< Copy of the community's private key, encrypted with the member's secure public key.
   };


   class community_moderator_vote_object : public object< community_moderator_vote_object_type, community_moderator_vote_object >
   {
      community_moderator_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_moderator_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              account;         ///< The name of the account that is voting for the moderator.

         community_name_type            community;       ///< Community that the moderator is being voted into.

         account_name_type              moderator;       ///< The name of the moderator being voted for.

         uint16_t                       vote_rank = 1;   ///< The rank of the vote for the community moderator.
   };

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


   class community_event_object : public object< community_event_object_type, community_event_object >
   {
      community_event_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_event_object( Constructor&& c, allocator< Allocator > a ) :
         event_name(a), location(a), details(a), url(a), json(a)
         {
            c( *this );
         }

         id_type                               id;                 

         account_name_type                     account;                ///< Account that created the event.

         community_name_type                   community;              ///< Community being invited to join.

         shared_string                         event_name;             ///< The Name of the event.

         shared_string                         location;               ///< Address location of the event.

         double                                latitude;               ///< Latitude co-ordinates of the event.

         double                                longitude;              ///< Longitude co-ordinates of the event.

         shared_string                         details;                ///< Event details describing the purpose of the event.

         shared_string                         url;                    ///< Reference URL for the event.

         shared_string                         json;                   ///< Additional Event JSON data.

         flat_set< account_name_type >         interested;             ///< Members that are interested in the event.

         flat_set< account_name_type >         attending;              ///< Members that have confirmed that they will be attending the event.

         flat_set< account_name_type >         not_attending;          ///< Members that have confirmed that they will not be attending the event.

         time_point                            event_start_time;       ///< Time that the Event will begin.

         time_point                            event_end_time;         ///< Time that the event will end.

         time_point                            last_updated;           ///< Time that the event was last updated.

         time_point                            created;                ///< Time that the event was created.

         bool                                  is_interested( const account_name_type& account )const
         {
            return std::find( interested.begin(), interested.end(), account ) != interested.end();
         };

         bool                                  is_attending( const account_name_type& account )const
         {
            return std::find( attending.begin(), attending.end(), account ) != attending.end();
         };

         bool                                  is_not_attending( const account_name_type& account )const
         {
            return std::find( not_attending.begin(), not_attending.end(), account ) != not_attending.end();
         };
   };

   struct by_name;
   struct by_founder;
   struct by_last_post;
   struct by_subscriber_count;
   struct by_post_count;
   struct by_vote_count;
   struct by_view_count;

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
            composite_key_compare< std::less< account_name_type >, std::less< community_id_type > >
         >,
         ordered_unique< tag< by_last_post >,
            composite_key< community_object,
               member< community_object, time_point, &community_object::last_post >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< std::greater< time_point >, std::less< community_id_type > >
         >,
         ordered_unique< tag< by_subscriber_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::subscriber_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< community_id_type > >
         >,
         ordered_unique< tag< by_post_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::post_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< community_id_type > >
         >,
         ordered_unique< tag< by_view_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::view_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< community_id_type > >
         >,
         ordered_unique< tag< by_vote_count >,
            composite_key< community_object,
               member< community_object, uint32_t, &community_object::vote_count >,
               member< community_object, community_id_type, &community_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< community_id_type > >
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

   struct by_member_community;

   typedef multi_index_container<
      community_member_key_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_member_key_object, community_member_key_id_type, &community_member_key_object::id > >,
         ordered_unique< tag< by_member_community >,
            composite_key< community_member_key_object,
               member< community_member_key_object, account_name_type, &community_member_key_object::member >, 
               member< community_member_key_object, community_name_type, &community_member_key_object::community > 
            >
         >
      >,
      allocator< community_member_key_object >
   > community_member_key_index;

   struct by_community;
   struct by_community_moderator;
   struct by_account;
   struct by_moderator_community;
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
         >,
         ordered_unique< tag< by_moderator_community >,
            composite_key< community_moderator_vote_object,
               member< community_moderator_vote_object, account_name_type, &community_moderator_vote_object::moderator >,
               member< community_moderator_vote_object, community_name_type, &community_moderator_vote_object::community >,
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
   

   struct by_start_time;
   struct by_end_time;
   struct by_last_updated;
   struct by_community;


   typedef multi_index_container<
      community_event_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_event_object, community_event_id_type, &community_event_object::id > >,
         ordered_unique< tag< by_community >,
            member< community_event_object, community_name_type, &community_event_object::community > >,
         ordered_non_unique< tag< by_start_time >,
            member< community_event_object, time_point, &community_event_object::event_start_time > >,
         ordered_non_unique< tag< by_end_time >,
            member< community_event_object, time_point, &community_event_object::event_end_time > >,
         ordered_non_unique< tag< by_last_updated >,
            member< community_event_object, time_point, &community_event_object::last_updated > >
      >,
      allocator< community_event_object >
   > community_event_index;

} } // node::chain


FC_REFLECT( node::chain::community_object,
         (id)
         (name)
         (founder)
         (community_privacy)
         (community_public_key)
         (json)
         (json_private)
         (pinned_author)
         (pinned_permlink)
         (subscriber_count)
         (post_count)
         (comment_count)
         (vote_count)
         (view_count)
         (share_count)
         (reward_currency)
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
         (encrypted_community_key)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_member_key_object, node::chain::community_member_key_index );       

FC_REFLECT( node::chain::community_event_object,
         (id)
         (account)
         (community)
         (event_name)
         (location)
         (latitude)
         (longitude)
         (details)
         (url)
         (json)
         (interested)
         (attending)
         (not_attending)
         (event_start_time)
         (event_end_time)
         (created)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_event_object, node::chain::community_event_index );

         