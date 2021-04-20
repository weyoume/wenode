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

         flat_set< tag_name_type >          tags;                               ///< Set of recommended tags of the topics within the community to enable discovery.

         public_key_type                    community_member_key;               ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted members.

         public_key_type                    community_moderator_key;            ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted moderators.

         public_key_type                    community_admin_key;                ///< Key used for encrypting and decrypting posts and messages. Private key shared with accepted admins.

         public_key_type                    community_secure_key;               ///< Key used for encrypting and decrypting posts and messages. Private key held only by the community founder.

         public_key_type                    community_standard_premium_key;     ///< Key used for encrypting and decrypting posts and messages. Private key shared with standard premium members.

         public_key_type                    community_mid_premium_key;          ///< Key used for encrypting and decrypting posts and messages. Private key shared with mid premium members.

         public_key_type                    community_top_premium_key;          ///< Key used for encrypting and decrypting posts and messages. Private key shared with top premium members.

         account_name_type                  interface;                          ///< Account of the interface that enabled the creation of the community. Gets a share of membership payments.

         asset_symbol_type                  reward_currency = SYMBOL_COIN;      ///< The Currency asset used for content rewards in the community.

         asset                              standard_membership_price = MEMBERSHIP_FEE_BASE;      ///< Price paid per month by community standard members to community founder.

         asset                              mid_membership_price = MEMBERSHIP_FEE_MID;            ///< Price paid per month by all mid level community members to community founder.

         asset                              top_membership_price = MEMBERSHIP_FEE_TOP;            ///< Price paid per month by all top level community members to community founder.

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

         bool enable_events()const                                              ///< True if events are enabled
         { 
            return !( flags & int( community_permission_flags::disable_events ) );
         }

         bool enable_polls()const                                               ///< True if polls are enabled
         { 
            return !( flags & int( community_permission_flags::disable_polls ) );
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

         bool enable_directives()const                                          ///< True if directives are enabled
         { 
            return !( flags & int( community_permission_flags::disable_directives ) );
         }
   };


   /**
    * Manages the membership, moderation, and administration lists of communities, 
    * implements community permissioning methods that determine which accounts can 
    * take community actions.
    * 
    * A standard Practice implemented in interfaces would create three communities
    * that act as concentric circles of trust within a group.
    * New accounts would begin by joining the main public group, 
    * then be advanced into the outer private community, 
    * then into the inner core Private community.
    * 
    * Downstream federated communities recieve all incoming membership and role changes,
    * and create an automated hierarchial community structure.
    */
   class community_permission_object : public object< community_permission_object_type, community_permission_object >
   {
      community_permission_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_permission_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                        id;

         community_name_type                            name;                                 ///< Name of the community with permissions set.

         account_name_type                              founder;                              ///< Name of the founding account of the community. Has full permissions.

         bool                                           private_community = false;            ///< True when the community is private, and all posts must be encrypted.

         bool                                           channel = false;                      ///< True when a community is a channel, and only accepts channel posts from admins.

         community_permission_type                      author_permission;                    ///< Determines which accounts can create root posts.

         community_permission_type                      reply_permission;                     ///< Determines which accounts can create replies to root posts.

         community_permission_type                      vote_permission;                      ///< Determines which accounts can create comment votes on posts and comments.

         community_permission_type                      view_permission;                      ///< Determines which accounts can create comment views on posts and comments.

         community_permission_type                      share_permission;                     ///< Determines which accounts can create comment shares on posts and comments.

         community_permission_type                      message_permission;                   ///< Determines which accounts can create direct messages in the community.
         
         community_permission_type                      poll_permission;                      ///< Determines which accounts can create polls in the community.

         community_permission_type                      event_permission;                     ///< Determines which accounts can create events in the community.

         community_permission_type                      directive_permission;                 ///< Determines which accounts can create directives and directive votes in the community.

         community_permission_type                      add_permission;                       ///< Determines which accounts can add new members, and accept membership requests.

         community_permission_type                      request_permission;                   ///< Determines which accounts can request to join the community.

         community_permission_type                      remove_permission;                    ///< Determines which accounts can remove and blacklist.

         flat_set< account_name_type >                  subscribers;                          ///< List of accounts that subscribe to the posts made in the community.

         flat_set< account_name_type >                  members;                              ///< List of accounts that are members of the community.

         flat_set< account_name_type >                  standard_premium_members;             ///< List of accounts that have paid the standard premium membership of the community.

         flat_set< account_name_type >                  mid_premium_members;                  ///< List of accounts that have paid (at least) the mid premium membership of the community.

         flat_set< account_name_type >                  top_premium_members;                  ///< List of accounts that have paid (at least) the top premium membership of the community.
 
         flat_set< account_name_type >                  moderators;                           ///< Accounts that are moderators of the community.

         flat_set< account_name_type >                  administrators;                       ///< Accounts that are administrators of the community.

         flat_set< account_name_type >                  blacklist;                            ///< Accounts that are not able to interact with the community.

         flat_set< community_name_type >                upstream_member_federations;          ///< Communities that this community recieves incoming members from.

         flat_set< community_name_type >                upstream_moderator_federations;       ///< Communities that this community recieves incoming moderators from.

         flat_set< community_name_type >                upstream_admin_federations;           ///< Communities that this community recieves incoming admins from.

         flat_set< community_name_type >                downstream_member_federations;        ///< Communities that Receive incoming members from this community.

         flat_set< community_name_type >                downstream_moderator_federations;     ///< Communities that Receive incoming moderators from this community.

         flat_set< community_name_type >                downstream_admin_federations;         ///< Communities that Receive incoming admins from this community.

         flat_map< account_name_type, share_type >      vote_weight;                          ///< Map of all moderator voting weights for distributing rewards.

         share_type                                     total_vote_weight = 0;                ///< Total of all moderator weights.

         flat_set< account_name_type >                  verifiers;                            ///< Accounts that are considered ground truth sources of verification authority, wth degree 0.

         uint64_t                                       min_verification_count = 0;           ///< Minimum number of incoming verification transaction to be considered verified by this community.

         uint64_t                                       max_verification_distance = 0;        ///< Maximum number of degrees of seperation from a verfier to be considered verified by this community.

         time_point                                     last_updated;                         ///< Time that the community was last updated.

         /**
          * Adjacency value determines how similar two accounts are by comparing the 
          * accounts, communities and tags that they have in common with eachother. 
          * this value is used for the determination of post recommendations.
          */
         share_type adjacency_value( const community_permission_object& m )const
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
          * Determines Permission to create new root posts in the community.
          */
         bool is_authorized_author( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( author_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",author_permission) );
               }
            }
         };

         /**
          * Determines Permission to reply to posts in the community.
          */
         bool is_authorized_reply( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( reply_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",reply_permission) );
               }
            }
         };

         /**
          * Determines Permission to vote on posts in the community.
          */
         bool is_authorized_vote( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( vote_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",vote_permission) );
               }
            }
         };

         /**
          * Determines Permission to view posts in the community.
          */
         bool is_authorized_view( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( view_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",view_permission) );
               }
            }
         };

         /**
          * Determines Permission to share posts in the community.
          */
         bool is_authorized_share( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( share_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",share_permission) );
               }
            }
         };

         /**
          * Determines Permission to create community messages.
          */
         bool is_authorized_message( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( message_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",message_permission) );
               }
            }
         };

         /**
          * Determines Permission to create polls within the community.
          */
         bool is_authorized_poll( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( poll_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",poll_permission) );
               }
            }
         };

         /**
          * Determines Permission to create events in the community.
          */
         bool is_authorized_event( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( event_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",event_permission) );
               }
            }
         };

         /**
          * Determines Permission to create directives and vote on directives in the community.
          */
         bool is_authorized_directive( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( directive_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",directive_permission) );
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

            switch( request_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",request_permission) );
               }
            }
         };
         
         /**
          * Determines Permission to invite and accept new members.
          */
         bool is_authorized_add( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( add_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",add_permission) );
               }
            }
         };

         /**
          * Determines Permission to remove or blacklist an account from the community.
          */
         bool is_authorized_remove( const account_name_type& account )const
         {
            if( is_blacklisted( account ) )
            {
               return false;
            }

            switch( remove_permission )
            {
               case community_permission_type::ALL_PERMISSION:
               {
                  return true;
               }
               break;
               case community_permission_type::MEMBER_PERMISSION:
               {
                  return is_member( account );
               }
               break;
               case community_permission_type::STANDARD_PREMIUM_PERMISSION:
               {
                  return is_standard_premium_member( account );
               }
               break;
               case community_permission_type::MID_PREMIUM_PERMISSION:
               {
                  return is_mid_premium_member( account );
               }
               break;
               case community_permission_type::TOP_PREMIUM_PERMISSION:
               {
                  return is_top_premium_member( account );
               }
               break;
               case community_permission_type::MODERATOR_PERMISSION:
               {
                  return is_moderator( account );
               }
               break;
               case community_permission_type::ADMIN_PERMISSION:
               {
                  return is_administrator( account );
               }
               break;
               case community_permission_type::FOUNDER_PERMISSION:
               {
                  return (founder == account);
               }
               break;
               case community_permission_type::NONE_PERMISSION:
               {
                  return false;
               }
               break;
               default:
               {
                  FC_ASSERT( false, "Invalid community permission: ${t}.",
                     ("t",remove_permission) );
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

         bool is_standard_premium_member( const account_name_type& account )const
         {
            return std::find( standard_premium_members.begin(), standard_premium_members.end(), account ) != standard_premium_members.end();
         };

         bool is_mid_premium_member( const account_name_type& account )const
         {
            return std::find( mid_premium_members.begin(), mid_premium_members.end(), account ) != mid_premium_members.end();
         };

         bool is_top_premium_member( const account_name_type& account )const
         {
            return std::find( top_premium_members.begin(), top_premium_members.end(), account ) != top_premium_members.end();
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

         void                                   add_subscriber( const account_name_type& a )
         {
            if( !is_subscriber( a ) )
            {
               subscribers.insert( a );
            }
         }

         void                                   add_member( const account_name_type& a )
         {
            if( !is_member( a ) )
            {
               members.insert( a );
            }
         }

         void                                   add_standard_premium_member( const account_name_type& a )
         {
            if( !is_standard_premium_member( a ) )
            {
               standard_premium_members.insert( a );
            }
         }

         void                                   add_mid_premium_member( const account_name_type& a )
         {
            if( !is_mid_premium_member( a ) )
            {
               mid_premium_members.insert( a );
            }
         }

         void                                   add_top_premium_member( const account_name_type& a )
         {
            if( !is_top_premium_member( a ) )
            {
               top_premium_members.insert( a );
            }
         }

         void                                   add_moderator( const account_name_type& a )
         {
            if( !is_moderator( a ) )
            {
               moderators.insert( a );
            }
         }

         void                                   add_administrator( const account_name_type& a )
         {
            if( !is_administrator( a ) )
            {
               administrators.insert( a );
            }
         }

         void                                   add_blacklist( const account_name_type& a )
         {
            if( !is_blacklisted( a ) )
            {
               blacklist.insert( a );
            }
         }

         void                                   remove_subscriber( const account_name_type& a )
         {
            if( is_subscriber( a ) )
            {
               subscribers.erase( a );
            }
         }

         void                                   remove_member( const account_name_type& a )
         {
            if( is_member( a ) )
            {
               members.erase( a );
            }
         }

         void                                   remove_standard_premium_member( const account_name_type& a )
         {
            if( is_standard_premium_member( a ) )
            {
               standard_premium_members.erase( a );
            }
         }

         void                                   remove_mid_premium_member( const account_name_type& a )
         {
            if( is_mid_premium_member( a ) )
            {
               mid_premium_members.erase( a );
            }
         }

         void                                   remove_top_premium_member( const account_name_type& a )
         {
            if( is_top_premium_member( a ) )
            {
               top_premium_members.erase( a );
            }
         }

         void                                   remove_moderator( const account_name_type& a )
         {
            if( is_moderator( a ) )
            {
               moderators.erase( a );
            }
         }

         void                                   remove_administrator( const account_name_type& a )
         {
            if( is_administrator( a ) )
            {
               administrators.erase( a );
            }
         }

         void                                   remove_blacklist( const account_name_type& a )
         {
            if( is_blacklisted( a ) )
            {
               blacklist.erase( a );
            }
         }
   };


   /**
    * Community Member Keys hold the private decryption key, 
    * encrypted with the secure key of the member of a community.
    * 
    * The member connected into the community can then refer to this 
    * decryption key to unlock private and premium posts created within the community.
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

         id_type                          id;                 

         account_name_type                account;                               ///< Account that added the member as a community member, and provided the access key.

         account_name_type                member;                                ///< Account of the admitted community member.

         community_name_type              community;                             ///< Community that the member is granted access permission to.

         community_permission_type        member_type;                           ///< Membership and Key encryption access and permission level.

         account_name_type                interface;                             ///< Account of the interface that most recently updated the membership key.

         encrypted_keypair_type           encrypted_community_key;               ///< The community's private key, encrypted with the member's secure public key.

         time_point                       expiration;                            ///< Time that the membership and key will be expired, and a new key must be generated.

         time_point                       last_updated;                          ///< Time that the membership was last updated.

         time_point                       created;                               ///< The time the membership was created.
   };


   /**
    * Community Member Requests enable an account to seek permission to join a community.
    * 
    * Contains a message from the account to the moderators of the community.
    * Requests can be accepted by existing members of the community with the creation of a community member object in response. 
    */
   class community_member_request_object : public object< community_member_request_object_type, community_member_request_object >
   {
      community_member_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_member_request_object( Constructor&& c, allocator< Allocator > a ) :
            message(a)
            {
               c( *this );
            }

         id_type                         id;                 

         account_name_type               account;          ///< Account that created the request.

         community_name_type             community;        ///< Community being requested to join.

         account_name_type               interface;        ///< Account of the interface that most recently updated the membership key.

         community_permission_type       member_type;      ///< Membership and Key encryption access and permission level.

         shared_string                   message;          ///< Encrypted message to the communities management team, encrypted with community public key.

         time_point                      expiration;       ///< Request expiry time.

         time_point                      last_updated;     ///< Time that the request was last updated.

         time_point                      created;          ///< The time the request was created.
   };


   /**
    * Community member votes place voting support on members of a community for reward distribution.
    * 
    * The highest voted members get a larger share of the moderation rewards of a community.
    */
   class community_member_vote_object : public object< community_member_vote_object_type, community_member_vote_object >
   {
      community_member_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_member_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                   id;

         account_name_type         account;         ///< The name of the account that is voting for the moderator.

         community_name_type       community;       ///< Community that the moderator is being voted into.

         account_name_type         member;          ///< The name of the moderator being voted for.

         uint16_t                  vote_rank = 1;   ///< The rank of the vote for the community moderator.

         time_point                last_updated;    ///< Time that the vote was last updated.

         time_point                created;         ///< The time the vote was created.
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


   /**
    * A Community directive that contains action instructions and deliverables for community members.
    * 
    * Community Directives enable four simultaneous collective 
    * decision making and leadership flows to run in parallel.
    * Each Directive can have a subdirective tree, 
    * indicating multiple stages of action progress completion.
    * 
    * Command: [Single Elected Commander]
    * 
    * A single commander is voted for by the entire community,
    * which makes broadcast directives for all community members.
    * 
    * Delegate: [Elected Representatives]
    * 
    * Each account votes for delegates within the community,
    * and accepts the directives of the highest voted approved delegate from thier selection.
    * 
    * Consensus: [Elected Directives]
    * 
    * Each account votes for directives within the community,
    * highest voted directive is applied to all community members as the consensus selection.
    * 
    * Emergent: [Individual Selection]
    * 
    * Each account votes for directives within the community,
    * and accepts the highest voted approved directive from thier selection.
    * 
    * UI Convention for directives would display them in a structure of increasing specificity:
    * 
    * 0 Depth is a board
    * 1 Depth is a list, within a board
    * 2 Depth is a card, within a list
    * 3 depth is a task, within a card
    */
   class community_directive_object : public object< community_directive_object_type, community_directive_object >
   {
      community_directive_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_directive_object( Constructor&& c, allocator< Allocator > a ) :
            directive_id(a),
            parent_directive_id(a),
            details(a),
            cover_image(a),
            ipfs(a),
            json(a)
            {
               c( *this );
            }

         id_type                               id;

         account_name_type                     account;                    ///< Account that created the directive.

         shared_string                         directive_id;               ///< UUIDv4 referring to the directive. Unique on account/directive_id.

         account_name_type                     parent_account;             ///< Account that created the directive.

         shared_string                         parent_directive_id;        ///< UUIDv4 referring to the directive. Unique on account/directive_id.

         community_name_type                   community;                  ///< Community that the directive is given to.

         public_key_type                       public_key;                 ///< Public key for encrypting the directive details. Null if public directive.

         account_name_type                     interface;                  ///< Account of the interface that broadcasted the transaction.

         shared_string                         details;                    ///< Text details of the directive. Should explain the action items.

         shared_string                         cover_image;                ///< IPFS image for display of this directive in the interface. 

         shared_string                         ipfs;                       ///< IPFS file reference for the directive. Images or other files can be attatched.

         shared_string                         json;                       ///< Additional Directive JSON metadata.

         id_type                               root_directive;             ///< Directive that this is a subdirective of.

         uint16_t                              depth = 0;                  ///< The number of subdirectives deep this directive is. 0 for root directive.

         int32_t                               net_votes = 0;              ///< The amount of approval votes, minus disapprovals on the post.

         time_point                            directive_start_time;       ///< Time that the Directive will begin.

         time_point                            directive_end_time;         ///< Time that the Directive must be completed by.

         time_point                            last_updated;               ///< Time that the directive was last updated.

         time_point                            created;                    ///< Time that the directive was created.

         bool                                  root = true;                ///< True if the directive is a root directive.

         bool                                  completed = false;          ///< True when the directive has been completed.

         bool                                  member_active = false;      ///< True when the directive is designated as currently outgoing.
   };


   /**
    * Votes to approve or disapprove a directive made by a member of a community.
    * 
    * Used for Consensus directive selection, and for directive feedback.
    */
   class community_directive_vote_object : public object< community_directive_vote_object_type, community_directive_vote_object >
   {
      community_directive_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_directive_vote_object( Constructor&& c, allocator< Allocator > a ) :
            details(a),
            json(a)
            {
               c( *this );
            }

         id_type                               id;

         account_name_type                     voter;                      ///< Account creating the directive vote.

         community_directive_id_type           directive;                  ///< The ID of the directive being voted on.

         community_name_type                   community;                  ///< Community that the directive is contained within.

         public_key_type                       public_key;                 ///< Public key for encrypting the directive vote details. Null if public directive vote.

         account_name_type                     interface;                  ///< Account of the interface that broadcasted the transaction.

         shared_string                         details;                    ///< Text details of the directive vote. Should contain directive feedback.

         shared_string                         json;                       ///< Additional Directive JSON metadata.

         bool                                  approve = true;             ///< True when the directive is approved, false when it is opposed. 

         time_point                            last_updated;               ///< Time that the vote was last updated.

         time_point                            created;                    ///< Time that the vote was created.
   };


   /**
    * Determines the State of an account's incoming and outgoing directives.
    * 
    * All members of a community have a directive member object created upoon joining,
    * which manages the directives that they are currently interacting with from the
    * four decision making flows.
    */
   class community_directive_member_object : public object< community_directive_member_object_type, community_directive_member_object >
   {
      community_directive_member_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_directive_member_object( Constructor&& c, allocator< Allocator > a ) :
            details(a), 
            json(a),
            command_directive_id(a),
            delegate_directive_id(a),
            consensus_directive_id(a),
            emergent_directive_id(a)
            {
               c( *this );
            }

         id_type                               id;

         account_name_type                     account;                             ///< Account recieving and creating directives within a community. 

         community_name_type                   community;                           ///< Community that the directives assignment is contained within.

         public_key_type                       public_key;                          ///< Public key for encrypting the directive member details. Null if public directive member.

         account_name_type                     interface;                           ///< Account of the interface that broadcasted the transaction.

         shared_string                         details;                             ///< Text details of the directive member. Should elaborate interests and prorities for voting selection.

         shared_string                         json;                                ///< Additional Directive Member JSON metadata.

         shared_string                         command_directive_id;                ///< The Current outgoing directive as community command co-ordinator.

         shared_string                         delegate_directive_id;               ///< The Current outgoing directive to all hierachy subordinate members.

         shared_string                         consensus_directive_id;              ///< The Current outgoing directive for community consensus selection.

         shared_string                         emergent_directive_id;               ///< The Current outgoing emergent directive for selection by other members.

         int32_t                               net_votes = 0;                       ///< The amount of approval votes.

         bool                                  active;                              ///< True when the account is active for directive distribution.

         time_point                            last_updated;                        ///< Time that the member was last updated.

         time_point                            created;                             ///< Time that the member was created.
   };


   /**
    * Votes to approve or disapprove a directive made by a member of a community.
    * 
    * Used for Consensus directive selection, and for directive feedback.
    */
   class community_directive_member_vote_object : public object< community_directive_member_vote_object_type, community_directive_member_vote_object >
   {
      community_directive_member_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         community_directive_member_vote_object( Constructor&& c, allocator< Allocator > a ) :
            details(a),
            json(a)
            {
               c( *this );
            }

         id_type                               id;

         account_name_type                     voter;                      ///< Account creating the directive member vote.

         account_name_type                     member;                     ///< Account being voted on.

         community_name_type                   community;                  ///< Community that the directive member vote is contained within.

         public_key_type                       public_key;                 ///< Public key for encrypting the directive member vote details. Null if public directive member vote.

         account_name_type                     interface;                  ///< Account of the interface that broadcasted the transaction.

         shared_string                         details;                    ///< Text details of the directive member vote. Should contain directive member feedback.

         shared_string                         json;                       ///< Additional Directive member vote JSON metadata.

         bool                                  approve = true;             ///< True when the directive member is approved, false when they is opposed.

         time_point                            last_updated;               ///< Time that the vote was last updated.

         time_point                            created;                    ///< Time that the vote was created.
   };



   struct by_name;
   struct by_founder;
   struct by_last_post;
   struct by_subscriber_count;
   struct by_post_count;
   struct by_vote_count;
   struct by_view_count;
   struct by_standard_membership_price;
   struct by_mid_membership_price;
   struct by_top_membership_price;


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
         >
      >,
      allocator< community_object >
   > community_index;

   struct by_name;

   typedef multi_index_container<
      community_permission_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_permission_object, community_permission_id_type, &community_permission_object::id > >,
         ordered_unique< tag< by_name >,
            member< community_permission_object, community_name_type, &community_permission_object::name > > 
      >,
      allocator< community_permission_object >
   > community_permission_index;


   struct by_member_community_type;
   struct by_community_type_member;
   struct by_expiration;


   typedef multi_index_container<
      community_member_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_member_object, community_member_id_type, &community_member_object::id > >,
         ordered_non_unique< tag< by_expiration >,
            member< community_member_object, time_point, &community_member_object::expiration > >,
         ordered_unique< tag< by_member_community_type >,
            composite_key< community_member_object,
               member< community_member_object, account_name_type, &community_member_object::member >, 
               member< community_member_object, community_name_type, &community_member_object::community >,
               member< community_member_object, community_permission_type, &community_member_object::member_type >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< community_name_type >,
               std::less< community_permission_type >
            >
         >,
         ordered_unique< tag< by_community_type_member >,
            composite_key< community_member_object,
               member< community_member_object, community_name_type, &community_member_object::community >,
               member< community_member_object, community_permission_type, &community_member_object::member_type >,
               member< community_member_object, account_name_type, &community_member_object::member >
            >,
            composite_key_compare<
               std::less< community_name_type >,
               std::less< community_permission_type >,
               std::less< account_name_type >
            >
         >
      >,
      allocator< community_member_object >
   > community_member_index;


   struct by_account_community_type;
   struct by_community_type_account;
   

   typedef multi_index_container<
      community_member_request_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_member_request_object, community_member_request_id_type, &community_member_request_object::id > >,
         ordered_non_unique< tag< by_expiration >,
            member< community_member_request_object, time_point, &community_member_request_object::expiration > >,
         ordered_unique< tag< by_account_community_type >,
            composite_key< community_member_request_object,
               member< community_member_request_object, account_name_type, &community_member_request_object::account >,
               member< community_member_request_object, account_name_type, &community_member_request_object::community >,
               member< community_member_request_object, community_permission_type, &community_member_request_object::member_type >
            >,
            composite_key_compare< 
               std::less< account_name_type >, 
               std::less< account_name_type >,
               std::less< community_permission_type > 
            >
         >,
         ordered_unique< tag< by_community_type_account >,
            composite_key< community_member_request_object,
               member< community_member_request_object, community_name_type, &community_member_request_object::community >,
               member< community_member_request_object, community_permission_type, &community_member_request_object::member_type >,
               member< community_member_request_object, account_name_type, &community_member_request_object::account >
            >,
            composite_key_compare< 
               std::less< community_name_type >, 
               std::less< community_permission_type >,
               std::less< account_name_type > 
            >
         > 
      >,
      allocator< community_member_request_object >
   > community_member_request_index;


   struct by_community;
   struct by_community_member;
   struct by_account;
   struct by_member_community_rank;
   struct by_account_community_member;
   struct by_account_community_rank;
   struct by_community_member_account_rank;


   typedef multi_index_container<
      community_member_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_member_vote_object, community_member_vote_id_type, &community_member_vote_object::id > >,
         ordered_unique< tag< by_account_community_rank >,
            composite_key< community_member_vote_object,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::account >,
               member< community_member_vote_object, community_name_type, &community_member_vote_object::community >,
               member< community_member_vote_object, uint16_t, &community_member_vote_object::vote_rank >,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::member >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< community_name_type >,
               std::less< uint16_t >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_account_community_member >,
            composite_key< community_member_vote_object,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::account >,
               member< community_member_vote_object, community_name_type, &community_member_vote_object::community >,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::member >,
               member< community_member_vote_object, uint16_t, &community_member_vote_object::vote_rank >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< community_name_type >,
               std::less< account_name_type >,
               std::less< uint16_t >
            >
         >,
         ordered_unique< tag< by_member_community_rank >,
            composite_key< community_member_vote_object,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::member >,
               member< community_member_vote_object, community_name_type, &community_member_vote_object::community >,
               member< community_member_vote_object, uint16_t, &community_member_vote_object::vote_rank >,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::account >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< community_name_type >,
               std::less< uint16_t >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_community_member_account_rank >,
            composite_key< community_member_vote_object,
               member< community_member_vote_object, community_name_type, &community_member_vote_object::community >,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::member >,
               member< community_member_vote_object, account_name_type, &community_member_vote_object::account >,
               member< community_member_vote_object, uint16_t, &community_member_vote_object::vote_rank >
            >,
            composite_key_compare<
               std::less< community_name_type >,
               std::less< account_name_type >,
               std::less< account_name_type >,
               std::less< uint16_t >
            >
         >
      >,
      allocator< community_member_vote_object >
   > community_member_vote_index;


   struct by_member_community;
   struct by_community;
   struct by_account;
   struct by_member;
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


   struct by_directive_id;
   struct by_parent_directive;
   struct by_root_directive;
   struct by_community_votes;


   typedef multi_index_container<
      community_directive_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_directive_object, community_directive_id_type, &community_directive_object::id > >,
         ordered_unique< tag< by_directive_id >,
            composite_key< community_directive_object,
               member< community_directive_object, account_name_type, &community_directive_object::account >,
               member< community_directive_object, shared_string, &community_directive_object::directive_id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               strcmp_less
            >
         >,
         ordered_unique< tag< by_parent_directive >,
            composite_key< community_directive_object,
               member< community_directive_object, account_name_type, &community_directive_object::parent_account >,
               member< community_directive_object, shared_string, &community_directive_object::parent_directive_id >,
               member< community_directive_object, community_directive_id_type, &community_directive_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               strcmp_less,
               std::less< community_directive_id_type >
            >
         >,
         ordered_unique< tag< by_root_directive >,
            composite_key< community_directive_object,
               member< community_directive_object, community_directive_id_type, &community_directive_object::root_directive >,
               member< community_directive_object, community_directive_id_type, &community_directive_object::id >
            >,
            composite_key_compare< 
               std::less< community_directive_id_type >,
               std::less< community_directive_id_type >
            >
         >,
         ordered_unique< tag< by_community_votes >,
            composite_key< community_directive_object,
               member< community_directive_object, community_name_type, &community_directive_object::community >,
               member< community_directive_object, bool, &community_directive_object::member_active >,
               member< community_directive_object, int32_t, &community_directive_object::net_votes >,
               member< community_directive_object, community_directive_id_type, &community_directive_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >,
               std::greater< bool >,
               std::greater< int32_t >,
               std::less< community_directive_id_type >
            >
         >
      >,
      allocator< community_directive_object >
   > community_directive_index;


   struct by_voter_community_directive;
   struct by_directive_voter;
   struct by_voter_recent;
   struct by_directive_approve;


   typedef multi_index_container<
      community_directive_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_directive_vote_object, community_directive_vote_id_type, &community_directive_vote_object::id > >,
         ordered_unique< tag< by_voter_community_directive >,
            composite_key< community_directive_vote_object,
               member< community_directive_vote_object, account_name_type, &community_directive_vote_object::voter >,
               member< community_directive_vote_object, community_name_type, &community_directive_vote_object::community >,
               member< community_directive_vote_object, community_directive_id_type, &community_directive_vote_object::directive >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< community_name_type >,
               std::less< community_directive_id_type >
            >
         >,
         ordered_unique< tag< by_directive_voter >,
            composite_key< community_directive_vote_object,
               member< community_directive_vote_object, community_directive_id_type, &community_directive_vote_object::directive >,
               member< community_directive_vote_object, account_name_type, &community_directive_vote_object::voter >
            >,
            composite_key_compare< 
               std::less< community_directive_id_type >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_directive_approve >,
            composite_key< community_directive_vote_object,
               member< community_directive_vote_object, community_directive_id_type, &community_directive_vote_object::directive >,
               member< community_directive_vote_object, bool, &community_directive_vote_object::approve >,
               member< community_directive_vote_object, community_directive_vote_id_type, &community_directive_vote_object::id >
            >,
            composite_key_compare< 
               std::less< community_directive_id_type >,
               std::less< bool >,
               std::less< community_directive_vote_id_type >
            >
         >,
         ordered_unique< tag< by_voter_recent >,
            composite_key< community_directive_vote_object,
               member< community_directive_vote_object, account_name_type, &community_directive_vote_object::voter >,
               member< community_directive_vote_object, community_directive_vote_id_type, &community_directive_vote_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< community_directive_vote_id_type >
            >
         >
      >,
      allocator< community_directive_vote_object >
   > community_directive_vote_index;


   struct by_member_community;
   struct by_community_member;
   struct by_community_votes;


   typedef multi_index_container<
      community_directive_member_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_directive_member_object, community_directive_member_id_type, &community_directive_member_object::id > >,
         ordered_unique< tag< by_member_community >,
            composite_key< community_directive_member_object,
               member< community_directive_member_object, account_name_type, &community_directive_member_object::account >,
               member< community_directive_member_object, community_name_type, &community_directive_member_object::community >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< community_name_type >
            >
         >,
         ordered_unique< tag< by_community_member >,
            composite_key< community_directive_member_object,
               member< community_directive_member_object, community_name_type, &community_directive_member_object::community >,
               member< community_directive_member_object, account_name_type, &community_directive_member_object::account >
            >,
            composite_key_compare< 
               std::less< community_name_type >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_community_votes >,
            composite_key< community_directive_member_object,
               member< community_directive_member_object, community_name_type, &community_directive_member_object::community >,
               member< community_directive_member_object, bool, &community_directive_member_object::active >,
               member< community_directive_member_object, int32_t, &community_directive_member_object::net_votes >,
               member< community_directive_member_object, community_directive_member_id_type, &community_directive_member_object::id >
            >,
            composite_key_compare< 
               std::less< community_name_type >,
               std::greater< bool >,
               std::greater< int32_t >,
               std::less< community_directive_member_id_type >
            >
         >
      >,
      allocator< community_directive_member_object >
   > community_directive_member_index;


   struct by_voter_community_member;
   struct by_community_voter_member;
   struct by_member_community_voter;
   struct by_voter_recent;
   struct by_member_recent;
   
   
   typedef multi_index_container<
      community_directive_member_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< community_directive_member_vote_object, community_directive_member_vote_id_type, &community_directive_member_vote_object::id > >,
         ordered_unique< tag< by_voter_community_member >,
            composite_key< community_directive_member_vote_object,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::voter >,
               member< community_directive_member_vote_object, community_name_type, &community_directive_member_vote_object::community >,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::member >
               
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::less< community_name_type >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_community_voter_member >,
            composite_key< community_directive_member_vote_object,
               member< community_directive_member_vote_object, community_name_type, &community_directive_member_vote_object::community >,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::voter >,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::member >
            >,
            composite_key_compare<
               std::less< community_name_type >,
               std::less< account_name_type >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_member_community_voter >,
            composite_key< community_directive_member_vote_object,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::member >,
               member< community_directive_member_vote_object, community_name_type, &community_directive_member_vote_object::community >,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::voter >
            >,
            composite_key_compare<
               std::less< account_name_type >,
               std::less< community_name_type >,
               std::less< account_name_type >
            >
         >,
         ordered_unique< tag< by_voter_recent >,
            composite_key< community_directive_member_vote_object,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::voter >,
               member< community_directive_member_vote_object, community_directive_member_vote_id_type, &community_directive_member_vote_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< community_directive_member_vote_id_type >
            >
         >,
         ordered_unique< tag< by_member_recent >,
            composite_key< community_directive_member_vote_object,
               member< community_directive_member_vote_object, account_name_type, &community_directive_member_vote_object::member >,
               member< community_directive_member_vote_object, community_directive_member_vote_id_type, &community_directive_member_vote_object::id >
            >,
            composite_key_compare< 
               std::less< account_name_type >,
               std::greater< community_directive_member_vote_id_type >
            >
         >
      >,
      allocator< community_directive_member_vote_object >
   > community_directive_member_vote_index;


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
         (community_member_key)
         (community_moderator_key)
         (community_admin_key)
         (community_secure_key)
         (community_standard_premium_key)
         (community_mid_premium_key)
         (community_top_premium_key)
         (interface)
         (reward_currency)
         (standard_membership_price)
         (mid_membership_price)
         (top_membership_price)
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

FC_REFLECT( node::chain::community_permission_object,
         (id)
         (name)
         (founder)
         (private_community)
         (channel)
         (author_permission)
         (reply_permission)
         (vote_permission)
         (view_permission)
         (share_permission)
         (message_permission)
         (poll_permission)
         (event_permission)
         (directive_permission)
         (add_permission)
         (request_permission)
         (remove_permission)
         (subscribers)
         (members)
         (standard_premium_members)
         (mid_premium_members)
         (top_premium_members)
         (moderators)
         (administrators)
         (blacklist)
         (upstream_member_federations)
         (upstream_moderator_federations)
         (upstream_admin_federations)
         (downstream_member_federations)
         (downstream_moderator_federations)
         (downstream_admin_federations)
         (vote_weight)
         (total_vote_weight)
         (verifiers)
         (min_verification_count)
         (max_verification_distance)
         (last_updated)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_permission_object, node::chain::community_permission_index );

FC_REFLECT( node::chain::community_member_object,
         (id)
         (account)
         (member)
         (community)
         (member_type)
         (interface)
         (encrypted_community_key)
         (expiration)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_member_object, node::chain::community_member_index );

FC_REFLECT( node::chain::community_member_request_object,
         (id)
         (account)
         (community)
         (interface)
         (member_type)
         (message)
         (expiration)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_member_request_object, node::chain::community_member_request_index );

FC_REFLECT( node::chain::community_member_vote_object,
         (id)
         (account)
         (community)
         (member)
         (vote_rank)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_member_vote_object, node::chain::community_member_vote_index );

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

FC_REFLECT( node::chain::community_directive_object,
         (id)
         (account)
         (directive_id)
         (parent_account)
         (parent_directive_id)
         (community)
         (public_key)
         (interface)
         (details)
         (cover_image)
         (ipfs)
         (json)
         (root_directive)
         (depth)
         (net_votes)
         (directive_start_time)
         (directive_end_time)
         (last_updated)
         (created)
         (root)
         (completed)
         (member_active)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_directive_object, node::chain::community_directive_index );

FC_REFLECT( node::chain::community_directive_vote_object,
         (id)
         (voter)
         (directive)
         (community)
         (public_key)
         (interface)
         (details)
         (json)
         (approve)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_directive_vote_object, node::chain::community_directive_vote_index );

FC_REFLECT( node::chain::community_directive_member_object,
         (id)
         (account)
         (community)
         (public_key)
         (interface)
         (details)
         (json)
         (command_directive_id)
         (delegate_directive_id)
         (consensus_directive_id)
         (emergent_directive_id)
         (net_votes)
         (active)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_directive_member_object, node::chain::community_directive_member_index );

FC_REFLECT( node::chain::community_directive_member_vote_object,
         (id)
         (voter)
         (member)
         (community)
         (public_key)
         (interface)
         (details)
         (json)
         (approve)
         (last_updated)
         (created)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::community_directive_member_vote_object, node::chain::community_directive_member_vote_index );