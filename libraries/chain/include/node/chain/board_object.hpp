#pragma once
#include <fc/fixed_string.hpp>

#include <node/protocol/authority.hpp>
#include <node/protocol/node_operations.hpp>

#include <node/chain/node_object_types.hpp>
#include <node/chain/witness_objects.hpp>
#include <node/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace node { namespace chain {

   class board_object : public object< board_object_type, board_object >
   {
      board_object() = delete;

      public:
         template<typename Constructor, typename Allocator>
         board_object( Constructor&& c, allocator< Allocator > a )
         {
            c(*this);
         };

         id_type                            id;

         board_name_type                    name;                               // Name of the board, lowercase letters, numbers and hyphens only.

         account_name_type                  founder;                            // The account that created the board, able to add and remove administrators.

         board_types                        board_type;                         // Type of board, persona, profile or business.

         board_privacy_types                board_privacy;                      // Board privacy level, open, public, private, or exclusive

         public_key_type                    board_public_key;                   // Key used for encrypting and decrypting posts. Private key shared with accepted members.

         shared_string                      json;                               // Public plaintext json information about the board, its topic and rules.

         shared_string                      json_private;                       // Private ciphertext json information about the board.

         comment_id_type                    pinned_comment;                     // Post pinned to the top of the board's page. 

         uint32_t                           subscriber_count = 0;               // number of accounts that are subscribed to the board

         uint32_t                           post_count = 0;                     // number of posts created in the board
         
         uint32_t                           comment_count = 0;                  // number of comments on posts in the board

         uint32_t                           vote_count = 0;                     // accumulated number of votes received by all posts in the board

         uint32_t                           view_count = 0;                     // accumulated number of views on posts in the board 

         uint32_t                           share_count = 0;                    // accumulated number of shares on posts in the board 

         asset                              total_content_rewards = asset(0, SYMBOL_COIN);   // total amount of rewards earned by curators in the board

         time_point                         created;                            // Time that the board was created.

         time_point                         last_board_update;                  // Time that the board's details were last updated.

         time_point                         last_post;                          // Time that the user most recently created a comment.

         time_point                         last_root_post;                     // Time that the board last created a post. 
   };

   /**
    * Manages the membership, moderation, and administration lists of boards, 
    * implements board permissioning methods according to the following Table:
    * 
    *   |=======================++=================+==================+===================+==================|
    *   |   Permissions         ||      OPEN       |      PUBLIC      |      PRIVATE      |     EXCLUSIVE    |
    *   |=======================++=================+==================+===================+==================| 
    *   |   Read+Interact       ||       All       |       All        |      Members      |     Members      |
    *   |=======================++=================+==================+===================+==================|
    *   |   Create Posts        ||       All       |     Members      |      Members      |     Members      |
    *   |=======================++=================+==================+===================+==================|
    *   |   Invite+Accept       ||     Members     |     Members      |       Mods        |     Admins       |
    *   |=======================++=================+==================+===================+==================|
    *   |   Request Join        ||       All       |       All        |       All         |     None         |
    *   |=======================++=================+==================+===================+==================|
    *   |   Remove posts        ||      Mods       |      Mods        |       Mods        |     Mods         |
    *   |=======================++=================+==================+===================+==================|
    *   |   Update board        ||     Admins      |     Admins       |      Admins       |     Admins       |
    *   |=======================++=================+==================+===================+==================|
    *   |   Remove+Blacklist    ||     Mods        |     Mods         |      Admins       |     Admins       |
    *   |=======================++=================+==================+===================+==================|
    * 
    * 
    * 
    */
   class board_member_object : public object< board_member_object_type, board_member_object >
   {
      board_member_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         board_member_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                                    id;

         board_name_type                            name;                        // Name of the board with permissions set.

         account_name_type                          founder;                     // Name of the founding account of the board. Has full permissions.

         board_privacy_types                        board_privacy;               // Privacy setting of board, determines transaction authorization. 

         board_types                                board_type;                  // Type of board, group, event, or store.

         flat_set < account_name_type >             subscribers;                 // List of accounts that subscribe to the posts made in the board.

         flat_set < account_name_type >             members;                     // List of accounts that are permitted to post in the board. Can invite and accept on public boards
 
         flat_set < account_name_type >             moderators;                  // Accounts able to filter posts. Can invite and accept on private boards.

         flat_set < account_name_type >             administrators;              // Accounts able to add and remove moderators and update board details. Can invite and accept on Exclusive boards. 

         flat_set < account_name_type >             blacklist;                   // Accounts that are not able to post in this board, or request to join.

         flat_map < account_name_type, share_type > mod_weight;                  // Map of all moderator voting weights for distributing rewards. 

         share_type                                 total_mod_weight = 0;        // Total of all moderator weights. 

         time_point                                 last_update;                 // Time that the board was last updated.

         /**
          * Adjacency value determines how similar two accounts are by comparing the 
          * accounts, boards and tags that they have in common with eachother. 
          * this value is used for the determination of post recommendations.
          */
         share_type                                 adjacency_value( const board_member_object& m )const
         {
            vector<account_name_type> common_subscribers;
            common_subscribers.reserve( subscribers.size() );
            std::set_intersection( m.subscribers.begin(), m.subscribers.end(), subscribers.begin(), subscribers.end(), common_subscribers.begin());

            vector<account_name_type> common_members;
            common_members.reserve( members.size() );
            std::set_intersection( m.members.begin(), m.members.end(), members.begin(), members.end(), common_members.begin());

            vector<account_name_type> common_moderators;
            common_moderators.reserve( moderators.size() );
            std::set_intersection( m.moderators.begin(), m.moderators.end(), moderators.begin(), moderators.end(), common_moderators.begin());

            vector<account_name_type> common_administrators;
            common_administrators.reserve( administrators.size() );
            std::set_intersection( m.administrators.begin(), m.administrators.end(), administrators.begin(), administrators.end(), common_administrators.begin());

            share_type result = common_subscribers.size() + 3*common_members.size() + 5*common_moderators.size() + 10*common_administrators.size();
            return result;
         };

         
         bool is_authorized_author( const account_name_type& account )const  // Determines Permission to create a new root post in the board
         {
            if( is_blacklisted( account) )
            {
               return false;       // The account is in the board's blacklist
            }
            if( board_privacy == OPEN_BOARD )
            {
               return true;      // The board is open and public, all non-blacklisted accounts can post to it.
            }
            else
            {
               if( is_member( account ) )
               {
                  return true;     // The account is in the membership list
               }
               else
               {
                  return false;     // The account is not in the membership list. 
               } 
            }
         };

         bool is_authorized_interact( const account_name_type& account )const  // Determines Permission to vote, view, comment or share posts in the board
         {
            if( is_blacklisted( account ) )
            {
               return false;          // The account is in the board's blacklist
            }
            if( board_privacy == OPEN_BOARD || board_privacy == PUBLIC_BOARD )  // Open or Public, anyone can interact
            {
               return true;             // The board is open and public, all non-blacklisted accounts can interact with it.
            }
            else                        // Private and Exclusive groups, members can interact.
            {
               if( is_member( account ) )
               {
                  return true;         // The account is in the membership list
               }
               else
               {
                  return false;        // The account is not in the membership list. 
               } 
            }
         };

         bool is_authorized_request( const account_name_type& account )const // Determines Permission to request to join.
         {
            if( is_blacklisted( account ) )
            {
               return false;             // The account is in the board's blacklist
            }
            if( board_privacy == EXCLUSIVE_BOARD )          // Exclusive groups do not allow join requests, invitation only. 
            {
               return false; 
            }
            else
            {
               return true;
            }
         };

         bool is_authorized_invite( const account_name_type& account )const // Determines Permission to send invites, accept join requests
         {
            if( is_blacklisted( account ) )
            {
               return false;       // The account is in the board's blacklist
            }
            if( board_privacy == OPEN_BOARD || board_privacy == PUBLIC_BOARD ) // Public groups, members can invite and accept invites
            {
               if( is_member( account ) )
               {
                  return true; // The account is in the membership list
               }
               else
               {
                  return false; // The account is not in the membership list. 
               }  
            }
            else if( board_privacy == PRIVATE_BOARD ) // Private groups, mods can invite and accept invites
            {
               if( is_moderator( account ) )
               {
                  return true; // The account is in the moderators list
               }
               else
               {
                  return false; // The account is not in the moderators list. 
               }
            }
            else if( board_privacy == EXCLUSIVE_BOARD ) // Exclusive groups, admins can invite and accept invites
            {
               if( is_administrator( account ) )
               {
                  return true; // The account is in the administrators list
               }
               else
               {
                  return false; // The account is not in the administrators list. 
               }
            }
         };

         bool is_authorized_blacklist( const account_name_type& account )const // Determines Permission to blacklist an account from the board. 
         {
            if( is_blacklisted( account ) )
            {
               return false;       // The account is in the board's blacklist
            }
            if( board_privacy == OPEN_BOARD || board_privacy == PUBLIC_BOARD ) // Public groups, moderators can blacklist
            {
               if( is_moderator( account ) )
               {
                  return true; // The account is in the moderators list
               }
               else
               {
                  return false; // The account is not in the moderators list. 
               }  
            }
            else // Private and Exclusive groups, admins can blacklist
            {
               if( is_administrator( account ) )
               {
                  return true; // The account is in the administrators list
               }
               else
               {
                  return false; // The account is not in the administrators list. 
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


   class board_moderator_vote_object : public object< board_moderator_vote_object_type, board_moderator_vote_object >
   {
      board_moderator_vote_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         board_moderator_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                        id;

         account_name_type              account;         // The name of the account that voting for the moderator.

         board_name_type                board;           // Board that the moderator is being voted into.

         account_name_type              moderator;       // The name of the moderator being voted for.

         uint16_t                       vote_rank;       // The rank of the vote for the board moderator
   };

   class board_join_request_object : public object< board_join_request_object_type, board_join_request_object >
   {
      board_join_request_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         board_join_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;                 

         account_name_type          account;                // Account that created the request.

         board_name_type            board;                  // Board being requested to join.

         shared_string              message;                // Encrypted message to the boards management team, encrypted with board public key.

         time_point                 expiration;             // Request expiry time.
   };


   class board_join_invite_object : public object< board_join_invite_object_type, board_join_invite_object >
   {
      board_join_invite_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         board_join_invite_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;                 

         account_name_type          account;                // Account that created the invite.

         account_name_type          member;                 // Account being invited to join the board membership.

         board_name_type            board;                  // Board being invited to join.

         shared_string              message;                // Encrypted message to the boards management team, encrypted with the members secure public key.

         time_point                 expiration;             // Invite expiry time.
   };


   class board_member_key_object : public object< board_member_key_object_type, board_member_key_object >
   {
      board_member_key_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         board_member_key_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                    id;                 

         account_name_type          account;                // Account that created the Board key for the new member.

         account_name_type          member;                 // Account of the new board member.

         board_name_type            board;                  // Board that the key enables access to.

         encrypted_keypair_type     encrypted_board_key;    // Copy of the board's private key, encrypted with the member's secure public key.
   };

   struct by_name;
   struct by_founder;
   struct by_last_post;
   struct by_subscriber_count;
   struct by_post_count;
   struct by_vote_count;
   struct by_view_count;

   typedef multi_index_container<
      board_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< board_object, board_id_type, &board_object::id > >,
         ordered_unique< tag< by_name >,
            member< board_object, board_name_type, &board_object::name > >,
         ordered_unique< tag< by_founder >,
            composite_key< board_object,
               member< board_object, account_name_type, &board_object::founder >,
               member< board_object, board_id_type, &board_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, std::less< board_id_type > >
         >,
         ordered_unique< tag< by_last_post >,
            composite_key< board_object,
               member< board_object, time_point, &board_object::last_post >,
               member< board_object, board_id_type, &board_object::id >
            >,
            composite_key_compare< std::greater< time_point >, std::less< board_id_type > >
         >,
         ordered_unique< tag< by_subscriber_count >,
            composite_key< board_object,
               member< board_object, uint32_t, &board_object::subscriber_count >,
               member< board_object, board_id_type, &board_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< board_id_type > >
         >,
         ordered_unique< tag< by_post_count >,
            composite_key< board_object,
               member< board_object, uint32_t, &board_object::post_count >,
               member< board_object, board_id_type, &board_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< board_id_type > >
         >,
         ordered_unique< tag< by_view_count >,
            composite_key< board_object,
               member< board_object, uint32_t, &board_object::view_count >,
               member< board_object, board_id_type, &board_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< board_id_type > >
         >,
         ordered_unique< tag< by_vote_count >,
            composite_key< board_object,
               member< board_object, uint32_t, &board_object::vote_count >,
               member< board_object, board_id_type, &board_object::id >
            >,
            composite_key_compare< std::greater< uint32_t >, std::less< board_id_type > >
         >
      >,
      allocator< board_object >
   > board_index;

   struct by_name;

   typedef multi_index_container<
      board_member_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< board_member_object, board_member_id_type, &board_member_object::id > >,
         ordered_unique< tag< by_name >,
            member< board_member_object, board_name_type, &board_member_object::name > > 
      >,
      allocator< board_member_object >
   > board_member_index;

   struct by_member_board;

   typedef multi_index_container<
      board_member_key_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< board_member_key_object, board_member_key_id_type, &board_member_key_object::id > >,
         ordered_unique< tag< by_member_board >,
            composite_key< board_member_key_object,
               member< board_member_key_object, account_name_type, &board_member_key_object::member >, 
               member< board_member_key_object, board_name_type, &board_member_key_object::board > 
            >
         >
      >,
      allocator< board_member_key_object >
   > board_member_key_index;

   struct by_board;
   struct by_board_moderator;
   struct by_account;
   struct by_moderator_board;
   struct by_account_board_moderator;
   struct by_account_board_rank;

   typedef multi_index_container<
      board_moderator_vote_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< board_moderator_vote_object, board_moderator_vote_id_type, &board_moderator_vote_object::id > >,
         ordered_unique< tag< by_board >,
            composite_key< board_moderator_vote_object,
               member< board_moderator_vote_object, board_name_type, &board_moderator_vote_object::board >,
               member< board_moderator_vote_object, board_moderator_vote_id_type, &board_moderator_vote_object::id >
            >
         >,
         ordered_unique< tag< by_account_board_rank >,
            composite_key< board_moderator_vote_object,
               member< board_moderator_vote_object, account_name_type, &board_moderator_vote_object::account >,
               member< board_moderator_vote_object, board_name_type, &board_moderator_vote_object::board >,
               member< board_moderator_vote_object, uint16_t, &board_moderator_vote_object::vote_rank >
            >
         >,
         ordered_unique< tag< by_account_board_moderator >,
            composite_key< board_moderator_vote_object,
               member< board_moderator_vote_object, account_name_type, &board_moderator_vote_object::account >,
               member< board_moderator_vote_object, board_name_type, &board_moderator_vote_object::board >,
               member< board_moderator_vote_object, account_name_type, &board_moderator_vote_object::moderator >
            >
         >,
         ordered_unique< tag< by_board_moderator >,
            composite_key< board_moderator_vote_object,
               member< board_moderator_vote_object, board_name_type, &board_moderator_vote_object::board >,
               member< board_moderator_vote_object, account_name_type, &board_moderator_vote_object::moderator >,
               member< board_moderator_vote_object, board_moderator_vote_id_type, &board_moderator_vote_object::id >
            >
         >,
         ordered_unique< tag< by_moderator_board >,
            composite_key< board_moderator_vote_object,
               member< board_moderator_vote_object, account_name_type, &board_moderator_vote_object::moderator >,
               member< board_moderator_vote_object, board_name_type, &board_moderator_vote_object::board >,
               member< board_moderator_vote_object, board_moderator_vote_id_type, &board_moderator_vote_object::id >
            >
         >
      >,
      allocator< board_moderator_vote_object >
   > board_moderator_vote_index;

   struct by_account_board;
   struct by_board_account;
   struct by_expiration;

   typedef multi_index_container<
      board_join_request_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< board_join_request_object, board_join_request_id_type, &board_join_request_object::id > >,
         ordered_non_unique< tag< by_expiration >,
            member< board_join_request_object, time_point, &board_join_request_object::expiration > >,
         ordered_unique< tag< by_account_board >,
            composite_key< board_join_request_object,
               member< board_join_request_object, account_name_type, &board_join_request_object::account >,
               member< board_join_request_object, board_name_type, &board_join_request_object::board >
            >
         >,
         ordered_unique< tag< by_board_account >,
            composite_key< board_join_request_object,
               member< board_join_request_object, board_name_type, &board_join_request_object::board >,
               member< board_join_request_object, account_name_type, &board_join_request_object::account >
            >
         > 
      >,
      allocator< board_join_request_object >
   > board_join_request_index;

   struct by_member_board;
   struct by_board;
   struct by_account;
   struct by_member;

   typedef multi_index_container<
      board_join_invite_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< board_join_invite_object, board_join_invite_id_type, &board_join_invite_object::id > >,
         ordered_non_unique< tag< by_expiration >,
            member< board_join_invite_object, time_point, &board_join_invite_object::expiration > >,
         ordered_unique< tag< by_member_board >,
            composite_key< board_join_invite_object,
               member< board_join_invite_object, account_name_type, &board_join_invite_object::member >, 
               member< board_join_invite_object, board_name_type, &board_join_invite_object::board > 
            >
         >,
         ordered_unique< tag< by_board >,
            composite_key< board_join_invite_object,
               member< board_join_invite_object, board_name_type, &board_join_invite_object::board >,
               member< board_join_invite_object, board_join_invite_id_type, &board_join_invite_object::id >
            >
         >,
         ordered_unique< tag< by_account >,
            composite_key< board_join_invite_object,
               member< board_join_invite_object, account_name_type, &board_join_invite_object::account >,
               member< board_join_invite_object, board_join_invite_id_type, &board_join_invite_object::id >
            >
         >,
         ordered_unique< tag< by_member >,
            composite_key< board_join_invite_object,
               member< board_join_invite_object, account_name_type, &board_join_invite_object::member >,
               member< board_join_invite_object, board_join_invite_id_type, &board_join_invite_object::id >
            >
         >
      >,
      allocator< board_join_invite_object >
   > board_join_invite_index;

} } // node::chain


FC_REFLECT( node::chain::board_object,
         (id)
         (name)
         (founder)
         (board_type)
         (board_public_key)
         (json)
         (json_private)
         (pinned_comment)
         (subscriber_count)
         (post_count)
         (comment_count)
         (vote_count)
         (view_count)
         (share_count)
         (total_content_rewards)
         (created)
         (last_board_update)
         (last_post)
         (last_root_post)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::board_object, node::chain::board_index );

FC_REFLECT( node::chain::board_member_object,
         (id)
         (name)
         (founder)
         (board_privacy)
         (board_type)
         (subscribers)
         (members)
         (moderators)
         (administrators)
         (blacklist)
         (mod_weight)
         (total_mod_weight)
         (last_update)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::board_member_object, node::chain::board_member_index );

FC_REFLECT( node::chain::board_moderator_vote_object,
         (id)
         (account)
         (board)
         (moderator)
         (vote_rank)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::board_moderator_vote_object, node::chain::board_moderator_vote_index );

FC_REFLECT( node::chain::board_join_request_object,
         (id)
         (account)
         (board)
         (message)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::board_join_request_object, node::chain::board_join_request_index );

FC_REFLECT( node::chain::board_join_invite_object,
         (id)
         (account)
         (member)
         (board)
         (message)
         (expiration)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::board_join_invite_object, node::chain::board_join_invite_index );

FC_REFLECT( node::chain::board_member_key_object,
         (id)
         (account)
         (member)
         (board)
         (encrypted_board_key)
         );

CHAINBASE_SET_INDEX_TYPE( node::chain::board_member_key_object, node::chain::board_member_key_index );