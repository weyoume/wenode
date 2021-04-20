
#include <node/chain/node_evaluator.hpp>
#include <node/chain/database.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/block_summary_object.hpp>
#include <cmath>

#include <node/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
//#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace node { namespace chain {


//==============================//
// === Community Evaluators === //
//==============================//


void community_create_evaluator::do_apply( const community_create_operation& o )
{ try {
   _db.check_namespace( o.name );
   const account_object& founder = _db.get_account( o.founder );
   FC_ASSERT( founder.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.founder));

   time_point now = _db.head_block_time();
   FC_ASSERT( now >= founder.last_community_created + MIN_COMMUNITY_CREATE_INTERVAL,
      "Founder: ${f} can only create one community per day, try again after: ${t}.",
      ("f",o.founder)("t", founder.last_community_created + MIN_COMMUNITY_CREATE_INTERVAL) );
   
   const asset_object& reward_currency = _db.get_asset( o.reward_currency );
    FC_ASSERT( reward_currency.asset_type == asset_property_type::CURRENCY_ASSET,
      "Reward currency asset must be either a currency type asset." );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   community_permission_type author_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type reply_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type vote_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type view_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type share_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type message_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type poll_permission = community_permission_type::ADMIN_PERMISSION;
   community_permission_type event_permission = community_permission_type::ADMIN_PERMISSION;
   community_permission_type directive_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type add_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type request_permission = community_permission_type::ALL_PERMISSION;
   community_permission_type remove_permission = community_permission_type::ADMIN_PERMISSION;

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.author_permission == community_permission_values[ i ] )
      {
         author_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.reply_permission == community_permission_values[ i ] )
      {
         reply_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.vote_permission == community_permission_values[ i ] )
      {
         vote_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.view_permission == community_permission_values[ i ] )
      {
         view_permission = community_permission_type( i );
         break;
      }
   }
   
   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.share_permission == community_permission_values[ i ] )
      {
         share_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.message_permission == community_permission_values[ i ] )
      {
         message_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.poll_permission == community_permission_values[ i ] )
      {
         poll_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.event_permission == community_permission_values[ i ] )
      {
         event_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.directive_permission == community_permission_values[ i ] )
      {
         directive_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.add_permission == community_permission_values[ i ] )
      {
         add_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.request_permission == community_permission_values[ i ] )
      {
         request_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.remove_permission == community_permission_values[ i ] )
      {
         remove_permission = community_permission_type( i );
         break;
      }
   }

   const community_object& new_community = _db.create< community_object >( [&]( community_object& co )
   {
      co.name = o.name;
      co.founder = o.founder;
      
      from_string( co.display_name, o.display_name );
      from_string( co.details, o.details );
      from_string( co.url, o.url );
      from_string( co.profile_image, o.profile_image );
      from_string( co.cover_image, o.cover_image );
      from_string( co.json, o.json );
      from_string( co.json_private, o.json_private );

      for( auto t : o.tags )
      {
         co.tags.insert( t );
      }

      co.community_member_key = public_key_type( o.community_member_key );
      co.community_moderator_key = public_key_type( o.community_moderator_key );
      co.community_admin_key = public_key_type( o.community_admin_key );
      co.community_secure_key = public_key_type( o.community_secure_key );
      co.community_standard_premium_key = public_key_type( o.community_standard_premium_key );
      co.community_mid_premium_key = public_key_type( o.community_mid_premium_key );
      co.community_top_premium_key = public_key_type( o.community_top_premium_key );

      co.interface = o.interface;
      co.max_rating = o.max_rating;
      co.flags = o.flags;
      co.permissions = o.permissions;
      co.reward_currency = o.reward_currency;
      co.standard_membership_price = o.standard_membership_price;
      co.mid_membership_price = o.mid_membership_price;
      co.top_membership_price = o.top_membership_price;
      co.created = now;
      co.last_updated = now;
      co.last_post = now;
      co.last_root_post = now;
      co.active = true;
   });

   const community_permission_object& new_community_permission = _db.create< community_permission_object >( [&]( community_permission_object& cmo )
   {
      cmo.name = o.name;
      cmo.founder = o.founder;

      cmo.add_subscriber( o.founder );
      cmo.add_member( o.founder );
      cmo.add_standard_premium_member( o.founder );
      cmo.add_mid_premium_member( o.founder );
      cmo.add_top_premium_member( o.founder );
      cmo.add_moderator( o.founder );
      cmo.add_administrator( o.founder );

      cmo.private_community = o.private_community;
      cmo.channel = o.channel;
      cmo.author_permission = author_permission;
      cmo.reply_permission = reply_permission;
      cmo.vote_permission = vote_permission;
      cmo.view_permission = view_permission;
      cmo.share_permission = share_permission;
      cmo.message_permission = message_permission;
      cmo.poll_permission = poll_permission;
      cmo.event_permission = event_permission;
      cmo.directive_permission = directive_permission;
      cmo.add_permission = add_permission;
      cmo.request_permission = request_permission;
      cmo.remove_permission = remove_permission;

      for( auto v : o.verifiers )
      {
         cmo.verifiers.insert( v );
      }

      cmo.min_verification_count = o.min_verification_count;
      cmo.max_verification_distance = o.max_verification_distance;
      cmo.last_updated = now;
   });

   _db.create< community_member_vote_object >( [&]( community_member_vote_object& v )
   {
      v.member = o.founder;
      v.account = o.founder;
      v.community = o.name;
      v.vote_rank = 1;
      v.last_updated = now;
      v.created = now;
   });

   _db.modify( founder, [&]( account_object& a )
   {
      a.last_community_created = now;
   });

   _db.update_community_moderators( o.name );

   ilog( "Founder: ${f} created new Community: \n ${c} \n ${m} \n",
      ("f",o.founder)("c",new_community)("m",new_community_permission) );

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_update_evaluator::do_apply( const community_update_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.account) );
   const community_object& community = _db.get_community( o.community );

   // New permissions must be subset of old permissions.
   
   FC_ASSERT(!( o.permissions & ~community.permissions ), 
      "Cannot reinstate previously revoked community permissions.");
   
   // Changed flags must be subset of old permissions.

   FC_ASSERT(!(( o.flags ^ community.flags ) & ~community.permissions ),
      "Flag change is not possible within community permissions." );
   
   time_point now = _db.head_block_time();

   FC_ASSERT( now >= ( community.last_updated + MIN_COMMUNITY_UPDATE_INTERVAL ),
      "Communities can only be updated once per 10 minutes." );

   const community_permission_object& community_permission = _db.get_community_permission( o.community );

   FC_ASSERT( community_permission.is_administrator( o.account ),
      "Only administrators of the community can update it.");

   community_permission_type author_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type reply_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type vote_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type view_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type share_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type message_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type poll_permission = community_permission_type::ADMIN_PERMISSION;
   community_permission_type event_permission = community_permission_type::ADMIN_PERMISSION;
   community_permission_type directive_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type add_permission = community_permission_type::MEMBER_PERMISSION;
   community_permission_type request_permission = community_permission_type::ALL_PERMISSION;
   community_permission_type remove_permission = community_permission_type::ADMIN_PERMISSION;

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.author_permission == community_permission_values[ i ] )
      {
         author_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.reply_permission == community_permission_values[ i ] )
      {
         reply_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.vote_permission == community_permission_values[ i ] )
      {
         vote_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.view_permission == community_permission_values[ i ] )
      {
         view_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.share_permission == community_permission_values[ i ] )
      {
         share_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.message_permission == community_permission_values[ i ] )
      {
         message_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.poll_permission == community_permission_values[ i ] )
      {
         poll_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.event_permission == community_permission_values[ i ] )
      {
         event_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.directive_permission == community_permission_values[ i ] )
      {
         directive_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.add_permission == community_permission_values[ i ] )
      {
         add_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.request_permission == community_permission_values[ i ] )
      {
         request_permission = community_permission_type( i );
         break;
      }
   }

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.remove_permission == community_permission_values[ i ] )
      {
         remove_permission = community_permission_type( i );
         break;
      }
   }

   if( o.pinned_author.size() > 0 || o.pinned_permlink.size() > 0 )
   {
      const comment_object& pinned_post = _db.get_comment( o.pinned_author, o.pinned_permlink );

      FC_ASSERT( pinned_post.root,
         "Pinned post must be a root comment." );
      FC_ASSERT( pinned_post.community == o.community,
         "Pinned post must be contained within the community." );
   }

   _db.modify( community, [&]( community_object& co )
   {
      if( o.display_name.size() > 0 )
      {
         from_string( co.display_name, o.display_name );
      }
      if( o.details.size() > 0 )
      {
         from_string( co.details, o.details );
      }
      if( o.url.size() > 0 )
      {
         from_string( co.url, o.url );
      }
      if( o.profile_image.size() > 0 )
      {
         from_string( co.profile_image, o.profile_image );
      }
      if( o.cover_image.size() > 0 )
      {
         from_string( co.cover_image, o.cover_image );
      }
      if( o.json.size() > 0 )
      {
         from_string( co.json, o.json );
      }
      if( o.json_private.size() > 0 )
      {
         from_string( co.json_private, o.json_private );
      }
      if( o.pinned_author.size() > 0 )
      {
         co.pinned_author = o.pinned_author;
      }
      if( o.pinned_permlink.size() > 0 )
      {
         from_string( co.pinned_permlink, o.pinned_permlink );
      }

      for( auto t : o.tags )
      {
         co.tags.insert( t );
      }

      if( o.community_member_key.size() > 0 )
      {
         co.community_member_key = public_key_type( o.community_member_key );
      }
      if( o.community_moderator_key.size() > 0 )
      {
         co.community_moderator_key = public_key_type( o.community_moderator_key );
      }
      if( o.community_admin_key.size() > 0 )
      {
         co.community_admin_key = public_key_type( o.community_admin_key );
      }
      if( o.community_secure_key.size() > 0 )
      {
         co.community_secure_key = public_key_type( o.community_secure_key );
      }
      if( o.community_standard_premium_key.size() > 0 )
      {
         co.community_standard_premium_key = public_key_type( o.community_standard_premium_key );
      }
      if( o.community_mid_premium_key.size() > 0 )
      {
         co.community_mid_premium_key = public_key_type( o.community_mid_premium_key );
      }
      if( o.community_top_premium_key.size() > 0 )
      {
         co.community_top_premium_key = public_key_type( o.community_top_premium_key );
      }
      
      if( co.max_rating != o.max_rating && o.max_rating != 0 )
      {
         co.max_rating = o.max_rating;
      }
      if( co.flags != o.flags )
      {
         co.flags = o.flags;
      }
      if( co.permissions != o.permissions )
      {
         co.permissions = o.permissions;
      }
      
      co.last_updated = now;
      co.active = o.active;
   });

   _db.modify( community_permission, [&]( community_permission_object& cmo )
   {
      cmo.private_community = o.private_community;
      cmo.channel = o.channel;
      cmo.author_permission = author_permission;
      cmo.reply_permission = reply_permission;
      cmo.vote_permission = vote_permission;
      cmo.view_permission = view_permission;
      cmo.share_permission = share_permission;
      cmo.message_permission = message_permission;
      cmo.poll_permission = poll_permission;
      cmo.event_permission = event_permission;
      cmo.directive_permission = directive_permission;
      cmo.add_permission = add_permission;
      cmo.request_permission = request_permission;
      cmo.remove_permission = remove_permission;

      for( auto v : o.verifiers )
      {
         cmo.verifiers.insert( v );
      }

      cmo.min_verification_count = o.min_verification_count;
      cmo.max_verification_distance = o.max_verification_distance;
      cmo.last_updated = now;
   });

   ilog( "Account: ${a} Updated community: \n ${c} \n ${p} \n",
      ("a",o.account)("c",community)("p",community_permission));

} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_member_evaluator::do_apply( const community_member_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));

   const account_object& member = _db.get_account( o.member );
   const account_following_object& account_following = _db.get_account_following( o.member );

   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active,
      "Community: ${s} must be active to create member requests.",
      ("s", o.community) );

   const community_permission_object& community_permission = _db.get_community_permission( o.community );

   ilog( "Account: ${a} Adding Community Member: ${m} of type: ${t} \n ${p} \n",
      ("a",o.account)("m",o.member)("t",o.member_type)("p",community_permission));

   time_point now = _db.head_block_time();

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   community_permission_type member_type = community_permission_type::MEMBER_PERMISSION;

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.member_type == community_permission_values[ i ] )
      {
         member_type = community_permission_type( i );
         break;
      }
   }

   const auto& member_idx = _db.get_index< community_member_index >().indices().get< by_member_community_type >();
   auto member_itr = member_idx.find( boost::make_tuple( o.member, o.community, member_type ) );

   const auto& req_idx = _db.get_index< community_member_request_index >().indices().get< by_account_community_type >();
   auto req_itr = req_idx.find( boost::make_tuple( o.member, o.community, member_type ) );

   if( o.accepted )
   {
      FC_ASSERT( community.active,
         "Community: ${c} must be active to add new members or edit memberships.",
         ("c",o.community));
      
      switch( member_type )
      {
         case community_permission_type::ALL_PERMISSION:
         {
            FC_ASSERT( false, "Cannot set community membership level to ALL" );
         }
         break;
         case community_permission_type::MEMBER_PERMISSION:
         {
            FC_ASSERT( !community_permission.is_member( o.member ),
               "Account: ${a} is already a member of the community: ${c}.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_authorized_add( o.account ),
               "Account: ${a} is not authorized to add members to the community: ${c}.",
               ("a",o.account)("c",o.community));
         
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.add_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.add_member_community( community.name );
               afo.last_updated = now;
            });

            for( auto down : community_permission.downstream_member_federations )
            {
               const community_permission_object& down_community_permission = _db.get_community_permission( down );

               _db.modify( down_community_permission, [&]( community_permission_object& cpo )
               {
                  cpo.add_member( o.member );
                  cpo.last_updated = now;
               });

               _db.modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_member_community( down );
                  afo.last_updated = now;
               });
            }

            if( member_itr == member_idx.end() )     // Creating new membership
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               FC_ASSERT( _db.check_community_verification( o.member, community_permission, 0 ),
                  "Account: ${a} is not sufficiently verified within community: ${c}.",
                  ("a",o.member)("c",o.community));

               const community_member_object& community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
               {
                  cmo.account = o.account;
                  cmo.member = o.member;
                  cmo.community = o.community;
                  cmo.member_type = member_type;
                  cmo.interface = o.interface;
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
                  cmo.created = now;
               });

               ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
            else
            {
               const community_member_object& community_member = *member_itr;

               FC_ASSERT( o.expiration >= ( community_member.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( community_member, [&]( community_member_object& cmo )
               {
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
               });

               ilog( "Account: ${a} Edited Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::STANDARD_PREMIUM_PERMISSION:
         {
            FC_ASSERT( req_itr != req_idx.end(),
               "Standard Premium membership must be requested by the member to approve payment." );
            FC_ASSERT( community_permission.is_member( o.member ),
               "Account: ${a} must be a member before they can upgrade to standard premium membership of Community: ${c}.",
               ("a", o.member)("c", o.community));
            FC_ASSERT( !community_permission.is_standard_premium_member( o.member ),
               "Account: ${a} is already a standard premium member of the community: ${c}.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_standard_premium_member( o.account ),
               "Account: ${a} is not authorized to add standard premium members to the community: ${c}.",
               ("a",o.account)("c",o.community));

            const community_member_request_object& request = *req_itr;
         
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.add_standard_premium_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.add_standard_premium_member_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr == member_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               asset liquid = _db.get_liquid_balance( o.member, community.standard_membership_price.symbol );

               FC_ASSERT( liquid >= community.standard_membership_price, 
                  "Account: ${a} has insufficient liquid balance to pay membership fee for community: ${c}.",
                  ("a",o.account)("c",o.community));

               const community_member_object& community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
               {
                  cmo.account = o.account;
                  cmo.member = o.member;
                  cmo.community = o.community;
                  cmo.member_type = member_type;
                  cmo.interface = o.interface;
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_standard_premium_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
                  cmo.created = now;
               });

               _db.process_community_premium_membership( community_member );

               ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
            else
            {
               const community_member_object& community_member = *member_itr;

               FC_ASSERT( o.expiration >= ( community_member.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( community_member, [&]( community_member_object& cmo )
               {
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_standard_premium_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
               });

               ilog( "Account: ${a} Edited Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
 
            _db.remove( request );
         }
         break;
         case community_permission_type::MID_PREMIUM_PERMISSION:
         {
            FC_ASSERT( req_itr != req_idx.end(), 
               "Mid Premium membership must be requested by the member to approve payment." );
            FC_ASSERT( community_permission.is_member( o.member ),
               "Account: ${a} must already be at least a member of the community: ${c} to upgrade to mid premium.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_standard_premium_member( o.member ),
               "Account: ${a} must already be at least a standard premium member of the community: ${c} to upgrade to mid premium.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_mid_premium_member( o.member ),
               "Account: ${a} is already a mid premium member of the community: ${c}.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_mid_premium_member( o.account ),
               "Account: ${a} is not authorized to add mid premium members to the community: ${c}.",
               ("a",o.account)("c",o.community));

            const community_member_request_object& request = *req_itr;
         
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.add_mid_premium_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.add_mid_premium_member_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr == member_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               asset liquid = _db.get_liquid_balance( o.member, community.mid_membership_price.symbol );

               FC_ASSERT( liquid >= community.mid_membership_price, 
                  "Account: ${a} has insufficient liquid balance to pay membership fee for community: ${c}.",
                  ("a",o.account)("c",o.community));

               const community_member_object& community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
               {
                  cmo.account = o.account;
                  cmo.member = o.member;
                  cmo.community = o.community;
                  cmo.member_type = member_type;
                  cmo.interface = o.interface;
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
                  cmo.created = now;
               });

               _db.process_community_premium_membership( community_member);

               ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
            else
            {
               const community_member_object& community_member = *member_itr;

               FC_ASSERT( o.expiration >= ( community_member.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( community_member, [&]( community_member_object& cmo )
               {
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
               });

               ilog( "Account: ${a} Edited Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
 
            _db.remove( request );
         }
         break;
         case community_permission_type::TOP_PREMIUM_PERMISSION:
         {
            FC_ASSERT( req_itr != req_idx.end(), 
               "Top Premium membership must be requested by the member to approve payment." );
            FC_ASSERT( community_permission.is_member( o.member ),
               "Account: ${a} must already be at least a member of the community: ${c} to upgrade to mid premium.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_standard_premium_member( o.member ),
               "Account: ${a} must already be at least a standard premium member of the community: ${c} to upgrade to mid premium.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_mid_premium_member( o.member ),
               "Account: ${a} must already be at least a mid premium member of the community: ${c} to upgrade to top premium.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_top_premium_member( o.member ),
               "Account: ${a} is already a top premium member of the community: ${c}.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_top_premium_member( o.account ),
               "Account: ${a} is not authorized to add top premium members to the community: ${c}.",
               ("a",o.account)("c",o.community));

            const community_member_request_object& request = *req_itr;
         
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.add_mid_premium_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.add_mid_premium_member_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr == member_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               asset liquid = _db.get_liquid_balance( o.member, community.mid_membership_price.symbol );

               FC_ASSERT( liquid >= community.mid_membership_price, 
                  "Account: ${a} has insufficient liquid balance to pay membership fee for community: ${c}.",
                  ("a",o.account)("c",o.community));

               const community_member_object& community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
               {
                  cmo.account = o.account;
                  cmo.member = o.member;
                  cmo.community = o.community;
                  cmo.member_type = member_type;
                  cmo.interface = o.interface;
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
                  cmo.created = now;
               });

               _db.process_community_premium_membership( community_member );

               ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
            else
            {
               const community_member_object& community_member = *member_itr;

               FC_ASSERT( o.expiration >= ( community_member.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( community_member, [&]( community_member_object& cmo )
               {
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
               });

               ilog( "Account: ${a} Edited Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
 
            _db.remove( request );
         }
         case community_permission_type::MODERATOR_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.member ), 
               "Account: ${a} must be a member before promotion to moderator of Community: ${c}.",
               ("a", o.member)("c", o.community));
            FC_ASSERT( !community_permission.is_moderator( o.member ),
               "Account: ${a} is already a moderator of the community: ${c}.",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_administrator( o.account ),
               "Account: ${a} must be an administrator to add moderators to Community: ${c}.",
               ("a", o.account)("c", o.community));
            
            _db.modify( community_permission, [&]( community_permission_object& cmo )
            {
               cmo.add_moderator( o.member );
               cmo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.add_moderator_community( o.community );
               afo.last_updated = now;
            });

            for( auto down : community_permission.downstream_moderator_federations )
            {
               const community_permission_object& down_community_permission = _db.get_community_permission( down );

               _db.modify( down_community_permission, [&]( community_permission_object& cmo )
               {
                  cmo.add_moderator( o.member );
                  cmo.last_updated = now;
               });

               _db.modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_admin_community( down );
                  afo.last_updated = now;
               });
            }

            if( member_itr == member_idx.end() )
            {
               const community_member_object& community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
               {
                  cmo.account = o.account;
                  cmo.member = o.member;
                  cmo.community = o.community;
                  cmo.member_type = member_type;
                  cmo.interface = o.interface;
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_moderator_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
                  cmo.created = now;
               });

               ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
            else
            {
               const community_member_object& community_member = *member_itr;

               _db.modify( community_member, [&]( community_member_object& cmo )
               {
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_moderator_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
               });

               ilog( "Account: ${a} Edited Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::ADMIN_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.member ), 
               "Account: ${a} must be a member before promotion to administrator of Community: ${c}.",
               ("a", o.member)("c", o.community));
            FC_ASSERT( community_permission.is_moderator( o.member ), 
               "Account: ${a} must be a moderator before promotion to administrator of Community: ${c}.",
               ("a", o.member)("c", o.community));
            FC_ASSERT( !community_permission.is_administrator( o.member ),
               "Account: ${a} is already an administrator of Community: ${c}.",
               ("a", o.member)("c", o.community));
            FC_ASSERT( community_permission.founder == o.account,
               "Account: ${a} must be the founder to add administrators to Community: ${c}.",
               ("a", o.account)("c", o.community));
         
            _db.modify( community_permission, [&]( community_permission_object& cmo )
            {
               cmo.add_administrator( o.member );
               cmo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.add_admin_community( o.community );
               afo.last_updated = now;
            });

            for( auto down : community_permission.downstream_admin_federations )
            {
               const community_permission_object& down_community_permission = _db.get_community_permission( down );

               _db.modify( down_community_permission, [&]( community_permission_object& cmo )
               {
                  cmo.add_administrator( o.member );
                  cmo.last_updated = now;
               });

               _db.modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_admin_community( down );
                  afo.last_updated = now;
               });
            }

            if( member_itr == member_idx.end() )
            {
               const community_member_object& community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
               {
                  cmo.account = o.account;
                  cmo.member = o.member;
                  cmo.community = o.community;
                  cmo.member_type = member_type;
                  cmo.interface = o.interface;
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_admin_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
                  cmo.created = now;
               });

               ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
            else
            {
               const community_member_object& community_member = *member_itr;

               _db.modify( community_member, [&]( community_member_object& cmo )
               {
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_admin_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
               });

               ilog( "Account: ${a} Edited Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::FOUNDER_PERMISSION:
         {
            FC_ASSERT( community.founder == o.account && 
               community_permission.founder == o.account,
               "Only the founder: ${f} of the community can transfer ownership.",
               ("f",community.founder) );
            FC_ASSERT( community_permission.is_member( o.member ), 
               "Account: ${a} must be an administrator before promotion to founder of Community: ${c}.",
               ("a", o.member)("c", o.community));
            FC_ASSERT( community_permission.is_moderator( o.member ), 
               "Account: ${a} must be an administrator before promotion to founder of Community: ${c}.",
               ("a", o.member)("c", o.community));
            FC_ASSERT( community_permission.is_administrator( o.member ), 
               "Account: ${a} must be an administrator before promotion to founder of Community: ${c}.",
               ("a", o.member)("c", o.community));

            const account_following_object& new_account_following = _db.get_account_following( o.member );
            const account_following_object& account_following = _db.get_account_following( o.account );

            _db.modify( community, [&]( community_object& co )
            {
               co.founder = o.member;
               co.last_updated = now;
            });

            _db.modify( community_permission, [&]( community_permission_object& cmo )
            {
               cmo.founder = o.member;
               cmo.last_updated = now;
            });

            _db.modify( new_account_following, [&]( account_following_object& afo )
            {
               afo.add_founder_community( o.community );
               afo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.remove_founder_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr == member_idx.end() )
            {
               const community_member_object& community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
               {
                  cmo.account = o.account;
                  cmo.member = o.member;
                  cmo.community = o.community;
                  cmo.member_type = member_type;
                  cmo.interface = o.interface;
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_secure_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
                  cmo.created = now;
               });

               ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }
            else
            {
               const community_member_object& community_member = *member_itr;

               _db.modify( community_member, [&]( community_member_object& cmo )
               {
                  cmo.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_secure_key, o.encrypted_community_key );
                  cmo.expiration = o.expiration;
                  cmo.last_updated = now;
               });

               ilog( "Account: ${a} Edited Community Member: ${m} of type: ${t} \n ${p} \n",
                  ("a",o.account)("m",o.member)("t",o.member_type)("p",community_member));
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::NONE_PERMISSION:
         {
            FC_ASSERT( false, "Cannot set community membership level to NONE" );
         }
         break;
         default:
         {
            FC_ASSERT( false, "Community Permission Type is invalid." );
         }
      }

      ilog( "Account: ${a} Added Community Member: ${m} of type: ${t} \n ${p} \n",
         ("a",o.account)("m",o.member)("t",o.member_type)("p",community_permission));
   }
   else
   {
      switch( member_type )
      {
         case community_permission_type::ALL_PERMISSION:
         {
            FC_ASSERT( false, "Cannot set community membership level to ALL" );
         }
         break;
         case community_permission_type::MEMBER_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.member ),
               "Account: ${a} is not a member of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_moderator( o.member ),
               "Account: ${a} cannot be removed while a moderator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_administrator( o.member ),
               "Account: ${a} cannot be removed while an administrator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.founder != o.member,
               "Account: ${a} cannot be removed while the founder of community: ${c}",
               ("a",o.member)("c",o.community));

            if( o.account != o.member )     // Account can remove itself from community membership.
            {
               FC_ASSERT( community_permission.is_authorized_remove( o.account ),
                  "Account: ${a} is not authorised to remove accounts from community: ${c}.",
                  ("a",o.account)("c",o.community));
            }
            
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.remove_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.remove_member_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr != member_idx.end() )
            {
               const community_member_object& community_member = *member_itr;

               ilog( "Removed: ${v}",("v",community_member));
               _db.remove( community_member );
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::STANDARD_PREMIUM_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.member ),
               "Account: ${a} is not a member of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_standard_premium_member( o.member ),
               "Account: ${a} is not a standard_premium member of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_moderator( o.member ),
               "Account: ${a} cannot be removed while a moderator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_administrator( o.member ),
               "Account: ${a} cannot be removed while an administrator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.founder != o.member,
               "Account: ${a} cannot be removed while the founder of community: ${c}",
               ("a",o.member)("c",o.community));

            if( o.account != o.member )     // Account can remove itself from community membership.
            {
               FC_ASSERT( community_permission.is_authorized_remove( o.account ),
                  "Account: ${a} is not authorised to remove accounts from community: ${c}.",
                  ("a",o.account)("c",o.community));
            }
            
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.remove_standard_premium_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.remove_standard_premium_member_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr != member_idx.end() )
            {
               const community_member_object& community_member = *member_itr;

               ilog( "Removed: ${v}",("v",community_member));
               _db.remove( community_member );
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::MID_PREMIUM_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.member ),
               "Account: ${a} is not a member of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_mid_premium_member( o.member ),
               "Account: ${a} is not a mid premium member of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_moderator( o.member ),
               "Account: ${a} cannot be removed while a moderator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_administrator( o.member ),
               "Account: ${a} cannot be removed while an administrator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.founder != o.member,
               "Account: ${a} cannot be removed while the founder of community: ${c}",
               ("a",o.member)("c",o.community));

            if( o.account != o.member )     // Account can remove itself from community membership.
            {
               FC_ASSERT( community_permission.is_authorized_remove( o.account ),
                  "Account: ${a} is not authorised to remove accounts from community: ${c}.",
                  ("a",o.account)("c",o.community));
            }
            
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.remove_mid_premium_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.remove_mid_premium_member_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr != member_idx.end() )
            {
               const community_member_object& community_member = *member_itr;

               ilog( "Removed: ${v}",("v",community_member));
               _db.remove( community_member );
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::TOP_PREMIUM_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.member ),
               "Account: ${a} is not a member of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.is_top_premium_member( o.member ),
               "Account: ${a} is not a top premium member of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_moderator( o.member ),
               "Account: ${a} cannot be removed while a moderator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_administrator( o.member ),
               "Account: ${a} cannot be removed while an administrator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.founder != o.member,
               "Account: ${a} cannot be removed while the founder of community: ${c}",
               ("a",o.member)("c",o.community));

            if( o.account != o.member )     // Account can remove itself from community membership.
            {
               FC_ASSERT( community_permission.is_authorized_remove( o.account ),
                  "Account: ${a} is not authorised to remove accounts from community: ${c}.",
                  ("a",o.account)("c",o.community));
            }
            
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.remove_top_premium_member( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.remove_top_premium_member_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr != member_idx.end() )
            {
               const community_member_object& community_member = *member_itr;

               ilog( "Removed: ${v}",("v",community_member));
               _db.remove( community_member );
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         case community_permission_type::MODERATOR_PERMISSION:
         {
            FC_ASSERT( community_permission.is_moderator( o.member ),
               "Account: ${a} is not a moderator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( !community_permission.is_administrator( o.member ),
               "Account: ${a} cannot be removed while an administrator of community: ${c}",
               ("a",o.member)("c",o.community));
            FC_ASSERT( community_permission.founder != o.member,
               "Account: ${a} cannot be removed while the founder of community: ${c}",
               ("a",o.member)("c",o.community));

            if( o.account != o.member )     // Account can remove itself from community membership.
            {
               FC_ASSERT( community_permission.is_administrator( o.account ),
                  "Account: ${a} is not authorised to remove moderators from community: ${c}.",
                  ("a",o.account)("c",o.community));
            }
            
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.remove_moderator( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.remove_moderator_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr != member_idx.end() )
            {
               const community_member_object& community_member = *member_itr;

               ilog( "Removed: ${v}",("v",community_member));
               _db.remove( community_member );
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::ADMIN_PERMISSION:
         {
            FC_ASSERT( community_permission.is_administrator( o.member ),
               "Account: ${a} is not an administrator of community: ${c}",
               ("a",o.member)("c",o.community) );
            FC_ASSERT( community_permission.founder != o.member,
               "Account: ${a} cannot be removed while the founder of community: ${c}",
               ("a",o.member)("c",o.community) );

            if( o.account != o.member )     // Account can remove itself from community membership.
            {
               FC_ASSERT( community_permission.founder == o.account,
                  "Account: ${a} is not authorised to remove admins from community: ${c}.",
                  ("a",o.account)("c",o.community));
            }
            
            _db.modify( community_permission, [&]( community_permission_object& cpo )
            {
               cpo.remove_administrator( o.member );
               cpo.last_updated = now;
            });

            _db.modify( account_following, [&]( account_following_object& afo )
            {
               afo.remove_admin_community( o.community );
               afo.last_updated = now;
            });

            if( member_itr != member_idx.end() )
            {
               const community_member_object& community_member = *member_itr;

               ilog( "Removed: ${v}",("v",community_member));
               _db.remove( community_member );
            }

            if( req_itr != req_idx.end() )
            {
               const community_member_request_object& request = *req_itr;
               _db.remove( request );
            }
         }
         break;
         case community_permission_type::FOUNDER_PERMISSION:
         {
            FC_ASSERT( false, "Cannot remove community founder" );
         }
         break;
         case community_permission_type::NONE_PERMISSION:
         {
            FC_ASSERT( false, "Cannot set community membership level to NONE" );
         }
         break;
         default:
         {
            FC_ASSERT( false, "Community Permission Type is invalid." );
         }
      }

      ilog( "Account: ${a} Removed Community Member: ${m} of type: ${t} \n ${p} \n",
         ("a",o.account)("m",o.member)("t",o.member_type)("p",community_permission));
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_member_request_evaluator::do_apply( const community_member_request_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active,
      "Community: ${s} must be active to create member requests.",
      ("s", o.community) );
   const community_permission_object& community_permission = _db.get_community_permission( o.community );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   community_permission_type member_type = community_permission_type::MEMBER_PERMISSION;

   for( size_t i = 0; i < community_permission_values.size(); i++ )
   {
      if( o.member_type == community_permission_values[ i ] )
      {
         member_type = community_permission_type( i );
         break;
      }
   }

   time_point now = _db.head_block_time();
   const auto& req_idx = _db.get_index< community_member_request_index >().indices().get< by_account_community_type >();
   auto req_itr = req_idx.find( std::make_tuple( o.account, o.community, member_type ) );

   if( o.requested )
   {
      switch( member_type )
      {
         case community_permission_type::ALL_PERMISSION:
         {
            FC_ASSERT( false, "Cannot set community membership level to ALL" );
         }
         break;
         case community_permission_type::MEMBER_PERMISSION:
         {
            FC_ASSERT( !community_permission.is_member( o.account ),
               "Account: ${a} is already a member of the community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_authorized_request( o.account ),
               "Account: ${a} is not authorised to request to join the community: ${c}.",
               ("a", o.account)("c", o.community));

            if( req_itr == req_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               _db.create< community_member_request_object >( [&]( community_member_request_object& cmro )
               {
                  cmro.account = o.account;
                  cmro.community = o.community;
                  cmro.interface = o.interface;
                  cmro.member_type = member_type;
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
                  cmro.created = now;
               });
            }
            else
            {
               const community_member_request_object& request = *req_itr;

               FC_ASSERT( o.expiration >= ( request.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( request, [&]( community_member_request_object& cmro )
               {
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
               });
            }
         }
         break;
         case community_permission_type::STANDARD_PREMIUM_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.account ),
               "Account: ${a} must be a member before they can upgrade to standard premium membership of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_authorized_request( o.account ),
               "Account: ${a} is not authorised to request to upgrade membership in the community: ${c}.",
               ("a", o.account)("c", o.community));

            if( req_itr == req_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               asset liquid = _db.get_liquid_balance( o.account, community.standard_membership_price.symbol );

               FC_ASSERT( liquid >= community.standard_membership_price, 
                  "Account: ${a} has insufficient liquid balance to pay membership fee for community: ${c}.",
                  ("a",o.account)("c",o.community));

               _db.create< community_member_request_object >( [&]( community_member_request_object& cmro )
               {
                  cmro.account = o.account;
                  cmro.community = o.community;
                  cmro.interface = o.interface;
                  cmro.member_type = member_type;
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
                  cmro.created = now;
               });
            }
            else
            {
               const community_member_request_object& request = *req_itr;

               FC_ASSERT( o.expiration >= ( request.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( request, [&]( community_member_request_object& cmro )
               {
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
               });
            }
         }
         break;
         case community_permission_type::MID_PREMIUM_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.account ),
               "Account: ${a} must be a member before they can upgrade to mid premium membership of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_standard_premium_member( o.account ),
               "Account: ${a} must already be at least a standard premium member of the community: ${c} to upgrade to mid premium.",
               ("a",o.account)("c",o.community));
            FC_ASSERT( community_permission.is_authorized_request( o.account ),
               "Account: ${a} is not authorised to request to upgrade membership in the community: ${c}.",
               ("a", o.account)("c", o.community));

            if( req_itr == req_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               asset liquid = _db.get_liquid_balance( o.account, community.mid_membership_price.symbol );

               FC_ASSERT( liquid >= community.mid_membership_price, 
                  "Account: ${a} has insufficient liquid balance to pay membership fee for community: ${c}.",
                  ("a",o.account)("c",o.community));

               _db.create< community_member_request_object >( [&]( community_member_request_object& cmro )
               {
                  cmro.account = o.account;
                  cmro.community = o.community;
                  cmro.interface = o.interface;
                  cmro.member_type = member_type;
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
                  cmro.created = now;
               });
            }
            else
            {
               const community_member_request_object& request = *req_itr;

               FC_ASSERT( o.expiration >= ( request.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( request, [&]( community_member_request_object& cmro )
               {
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
               });
            }
         }
         break;
         case community_permission_type::TOP_PREMIUM_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.account ),
               "Account: ${a} must be a member before they can upgrade to mid premium membership of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_standard_premium_member( o.account ),
               "Account: ${a} must already be at least a standard premium member of the community: ${c} to upgrade to mid premium.",
               ("a",o.account)("c",o.community));
            FC_ASSERT( community_permission.is_mid_premium_member( o.account ),
               "Account: ${a} must already be at least a mid premium member of the community: ${c} to upgrade to top premium.",
               ("a",o.account)("c",o.community));
            FC_ASSERT( community_permission.is_authorized_request( o.account ),
               "Account: ${a} is not authorised to request to upgrade membership in the community: ${c}.",
               ("a", o.account)("c", o.community));

            if( req_itr == req_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               asset liquid = _db.get_liquid_balance( o.account, community.mid_membership_price.symbol );

               FC_ASSERT( liquid >= community.mid_membership_price, 
                  "Account: ${a} has insufficient liquid balance to pay membership fee for community: ${c}.",
                  ("a",o.account)("c",o.community));

               _db.create< community_member_request_object >( [&]( community_member_request_object& cmro )
               {
                  cmro.account = o.account;
                  cmro.community = o.community;
                  cmro.interface = o.interface;
                  cmro.member_type = member_type;
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
                  cmro.created = now;
               });
            }
            else
            {
               const community_member_request_object& request = *req_itr;

               FC_ASSERT( o.expiration >= ( request.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( request, [&]( community_member_request_object& cmro )
               {
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
               });
            }
         }
         case community_permission_type::MODERATOR_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.account ), 
               "Account: ${a} must be a member before promotion to moderator of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( !community_permission.is_moderator( o.account ),
               "Account: ${a} is already a moderator of the community: ${c}.",
               ("a",o.account)("c",o.community));
            FC_ASSERT( community_permission.is_authorized_request( o.account ),
               "Account: ${a} is not authorised to request to become a moderator in the community: ${c}.",
               ("a", o.account)("c", o.community));

            if( req_itr == req_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               _db.create< community_member_request_object >( [&]( community_member_request_object& cmro )
               {
                  cmro.account = o.account;
                  cmro.community = o.community;
                  cmro.interface = o.interface;
                  cmro.member_type = member_type;
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
                  cmro.created = now;
               });
            }
            else
            {
               const community_member_request_object& request = *req_itr;

               FC_ASSERT( o.expiration >= ( request.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( request, [&]( community_member_request_object& cmro )
               {
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
               });
            }
         }
         break;
         case community_permission_type::ADMIN_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.account ), 
               "Account: ${a} must be a member before promotion to moderator of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_moderator( o.account ),
               "Account: ${a} is already a moderator of the community: ${c}.",
               ("a",o.account)("c",o.community));
            FC_ASSERT( !community_permission.is_administrator( o.account ),
               "Account: ${a} is already an administrator of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_authorized_request( o.account ),
               "Account: ${a} is not authorised to request to become a moderator in the community: ${c}.",
               ("a", o.account)("c", o.community));

            if( req_itr == req_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               _db.create< community_member_request_object >( [&]( community_member_request_object& cmro )
               {
                  cmro.account = o.account;
                  cmro.community = o.community;
                  cmro.interface = o.interface;
                  cmro.member_type = member_type;
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
                  cmro.created = now;
               });
            }
            else
            {
               const community_member_request_object& request = *req_itr;

               FC_ASSERT( o.expiration >= ( request.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( request, [&]( community_member_request_object& cmro )
               {
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
               });
            }
         }
         break;
         case community_permission_type::FOUNDER_PERMISSION:
         {
            FC_ASSERT( community_permission.is_member( o.account ), 
               "Account: ${a} must be a member before promotion to founder of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_moderator( o.account ), 
               "Account: ${a} must be a moderator before promotion to founder of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_administrator( o.account ), 
               "Account: ${a} must be an administrator before promotion to founder of Community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community.founder != o.account && 
               community_permission.founder != o.account,
               "Account: ${a} of is already the founder of the community: ${c}.",
               ("a", o.account)("c", o.community));
            FC_ASSERT( community_permission.is_authorized_request( o.account ),
               "Account: ${a} is not authorised to request to become the founder in the community: ${c}.",
               ("a", o.account)("c", o.community));

            if( req_itr == req_idx.end() )
            {
               FC_ASSERT( o.expiration >= ( now + fc::days(30) ),
                  "Expiration time must be at least 30 days in the future when creating new membership.",
                  ("c",o.community));

               _db.create< community_member_request_object >( [&]( community_member_request_object& cmro )
               {
                  cmro.account = o.account;
                  cmro.community = o.community;
                  cmro.interface = o.interface;
                  cmro.member_type = member_type;
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
                  cmro.created = now;
               });
            }
            else
            {
               const community_member_request_object& request = *req_itr;

               FC_ASSERT( o.expiration >= ( request.created + fc::days(30) ),
                  "Expiration time must be at least 30 days after creation when updating membership or expiration time.",
                  ("c",o.community));

               _db.modify( request, [&]( community_member_request_object& cmro )
               {
                  from_string( cmro.message, o.message );
                  cmro.expiration = o.expiration;
                  cmro.last_updated = now;
               });
            }
         }
         break;
         case community_permission_type::NONE_PERMISSION:
         {
            FC_ASSERT( false, "Cannot set community membership level to NONE" );
         }
         break;
         default:
         {
            FC_ASSERT( false, "Community Permission Type is invalid." );
         }
      }

      ilog( "Account: ${a} created Community Member Request of type: ${t} \n",
         ("a",o.account)("t",o.member_type));
   }
   else      // Removing request
   {
      FC_ASSERT( req_itr != req_idx.end(),
         "Request does not exist to remove." );

      const community_member_request_object& request = *req_itr;

      ilog( "Removed: ${v}",("v",request));
      _db.remove( request );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_member_vote_evaluator::do_apply( const community_member_vote_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));

   const account_object& member = _db.get_account( o.member );
   time_point now = _db.head_block_time();
   
   FC_ASSERT( member.active, 
      "Account: ${s} must be active to be voted for.",
      ("s", o.member) );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for member voting.",
      ("s", o.community) );
   const community_permission_object& community_permission = _db.get_community_permission( o.community );

   if( o.approved )
   {
      FC_ASSERT( account.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( community_permission.is_member( o.account ),
         "Account: ${a} must be a member before voting for a Member of Community: ${c}.",
         ("a", o.account)("c", o.community));
   }
   
   const auto& account_community_rank_idx = _db.get_index< community_member_vote_index >().indices().get< by_account_community_rank >();
   const auto& account_community_member_idx = _db.get_index< community_member_vote_index >().indices().get< by_account_community_member >();
   auto account_community_rank_itr = account_community_rank_idx.find( boost::make_tuple( o.account, o.community, o.vote_rank ) );   // vote at rank number
   auto account_community_member_itr = account_community_member_idx.find( boost::make_tuple( o.account, o.community, o.member ) );    // vote for moderator in community

   if( o.approved )   // Adding or modifying vote
   {
      if( account_community_member_itr == account_community_member_idx.end() && 
         account_community_rank_itr == account_community_rank_idx.end() )       // No vote for executive or rank, create new vote.
      {
         ilog( "Account: ${a} Adding Community Member Vote: ${m} of rank: ${t}",
            ("a",o.account)("m",o.member)("t",o.vote_rank));

         const community_member_vote_object& community_member_vote = _db.create< community_member_vote_object>( [&]( community_member_vote_object& cmvo )
         {
            cmvo.account = o.account;
            cmvo.member = o.member;
            cmvo.community = o.community;
            cmvo.vote_rank = o.vote_rank;
            cmvo.last_updated = now;
            cmvo.created = now;
         });
         
         _db.update_community_member_votes( account, o.community );

         ilog( "Account: ${a} Added Community Member Vote: ${m} of rank: ${t} \n ${p} \n",
            ("a",o.account)("m",o.member)("t",o.vote_rank)("p",community_member_vote));
      }
      else
      {
         if( account_community_member_itr != account_community_member_idx.end() && 
            account_community_rank_itr != account_community_rank_idx.end() )
         {
            FC_ASSERT( account_community_member_itr->member != account_community_rank_itr->member,
               "Vote at rank is already for the specified moderator." );
         }

         ilog( "Account: ${a} Editing Community Member Vote: ${m} of rank: ${t}",
            ("a",o.account)("m",o.member)("t",o.vote_rank));
         
         if( account_community_member_itr != account_community_member_idx.end() )
         {
            ilog( "Removed: ${v}",("v",*account_community_member_itr));
            _db.remove( *account_community_member_itr );
         }

         _db.update_community_member_votes( account, o.community, o.member, o.vote_rank );   // Remove existing moderator vote, and add at new rank.
      }
   }
   else       // Removing existing vote
   {
      ilog( "Account: ${a} Removing Community Member Vote: ${m} of rank: ${t}",
         ("a",o.account)("m",o.member)("t",o.vote_rank));

      if( account_community_member_itr != account_community_member_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_community_member_itr));
         _db.remove( *account_community_member_itr );
      }
      else if( account_community_rank_itr != account_community_rank_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_community_rank_itr));
         _db.remove( *account_community_rank_itr );
      }
      _db.update_community_member_votes( account, o.community );
   }

   ilog( "Account: ${a} Completed Operation Community Member Vote: ${m} of rank: ${t}",
      ("a",o.account)("m",o.member)("t",o.vote_rank));
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_blacklist_evaluator::do_apply( const community_blacklist_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));
   
   const account_object& member = _db.get_account( o.member );
   const community_object& community = _db.get_community( o.community );

   FC_ASSERT( community.active, 
      "Community: ${s} must be active for blacklist updating.",
      ("s", o.community) );
   FC_ASSERT( member.active, 
      "Member: ${s} must be active to add to blacklist.",
      ("s", o.member) );

   const community_permission_object& community_permission = _db.get_community_permission( o.community );
   time_point now = _db.head_block_time();
   
   FC_ASSERT( community_permission.is_authorized_remove( o.account ),
      "Account: ${a} is not authorised to add or remove accounts from the blacklist of community: ${c}.",
      ("a", o.account)("c", o.community));
   FC_ASSERT( !community_permission.is_member( o.member ),
      "Account: ${a} cannot be blacklisted while a member of a federated community: ${c}. Remove them first.",
      ("a", o.member)("c", o.community));
   FC_ASSERT( !community_permission.is_moderator( o.member ),
      "Account: ${a} cannot be blacklisted while a moderator of a federated community: ${c}. Remove them first.",
      ("a", o.member)("c", o.community));
   FC_ASSERT( !community_permission.is_administrator( o.member ),
      "Account: ${a} cannot be blacklisted while an administrator of a federated community: ${c}. Remove them first.",
      ("a", o.member)("c", o.community));
   FC_ASSERT( community_permission.founder != o.member,
      "Account: ${a} cannot be blacklisted while the founder of community: ${c}.", ("a", o.member)("c", o.community));

   _db.modify( community_permission, [&]( community_permission_object& cmo )
   {
      if( o.blacklisted )
      {
         cmo.blacklist.insert( o.member );
      }
      else
      {
         cmo.blacklist.erase( o.member );
      }
      cmo.last_updated = now;
   });
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_subscribe_evaluator::do_apply( const community_subscribe_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));

   const account_following_object& account_following = _db.get_account_following( o.account );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active to subscribe.",("s", o.community) );
   const community_permission_object& community_permission = _db.get_community_permission( o.community );
   time_point now = _db.head_block_time();

   if( o.subscribed )   // Adding subscription 
   {
      FC_ASSERT( !community_permission.is_subscriber( o.account ), 
         "Account: ${a} is already subscribed to the community ${c}.",
         ("a", o.account)("c", o.community));
   }
   else     // Removing subscription
   {
      FC_ASSERT( community_permission.is_subscriber( o.account ), 
         "Account: ${a} is not subscribed to the community ${c}.",
         ("a", o.account)("c", o.community));
   }

   if( o.added )
   {
      if( o.subscribed )     // Add subscriber
      {
         _db.modify( community_permission, [&]( community_permission_object& cmo )
         {
            cmo.subscribers.insert( o.account );
            cmo.last_updated = now;
         });

         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.add_followed_community( community.name );
            afo.last_updated = now;
         });

         const account_object& subscriber = _db.get_account( o.account );

         if( subscriber.membership == membership_tier_type::NONE )     // Check for the presence of an ad bid on this subscription.
         {
            const auto& bid_idx = _db.get_index< ad_bid_index >().indices().get< by_provider_metric_author_objective_price >();
            auto bid_itr = bid_idx.lower_bound( std::make_tuple( o.interface, ad_metric_type::SUBSCRIBE_METRIC, account_name_type(), o.community ) );

            while( bid_itr != bid_idx.end() &&
               bid_itr->provider == o.interface &&
               bid_itr->metric == ad_metric_type::SUBSCRIBE_METRIC &&
               bid_itr->author == account_name_type() &&
               to_string( bid_itr->objective ) == string( o.community ) )    // Retrieves highest paying bids for this subscription by this interface.
            {
               const ad_bid_object& bid = *bid_itr;
               const ad_audience_object& audience = _db.get_ad_audience( bid.bidder, bid.audience_id );

               if( !bid.is_delivered( o.account ) && audience.is_audience( o.account ) )
               {
                  _db.deliver_ad_bid( bid, subscriber );
                  break;
               }

               ++bid_itr;
            }
         }
      }
      else        // Add filter
      {
         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.add_filtered_community( community.name );
            afo.last_updated = now;
         });
      }
   }
   else
   {
      if( o.subscribed )     // Remove subscriber
      {
         _db.modify( community_permission, [&]( community_permission_object& cmo )
         {
            cmo.subscribers.erase( o.account );
            cmo.last_updated = now;
         });

         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.remove_followed_community( community.name );
            afo.last_updated = now;
         });
      }
      else        // Remove filter
      {
         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.remove_filtered_community( community.name );
            afo.last_updated = now; 
         });
      }
   }
   
   _db.update_community_in_feed( o.account, o.community );    // Add new feed objects or remove old feed objects for community in account's feed.

} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_federation_evaluator::do_apply( const community_federation_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));

   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active,
      "Community: ${s} must be active for federation requests.",
      ("s", o.community) );
   const community_permission_object& community_permission = _db.get_community_permission( o.community );

   FC_ASSERT( community_permission.is_administrator( o.account ),
      "Account: ${a} must be an Admin of the community: ${c}.",
      ("a", o.account)("c",o.community));

   const community_object& federated_community = _db.get_community( o.federated_community );
   FC_ASSERT( federated_community.active,
      "Community: ${s} must be active for federation.",
      ("s", o.federated_community));

   time_point now = _db.head_block_time();
   public_key_type public_key;
   community_federation_type community_federation = community_federation_type::MEMBER_FEDERATION;

   for( size_t i = 0; i < community_federation_values.size(); i++ )
   {
      if( o.federation_type == community_federation_values[ i ] )
      {
         community_federation = community_federation_type( i );
         break;
      }
   }

   switch( community_federation )
   {
      case community_federation_type::MEMBER_FEDERATION:
      {
         public_key = community.community_member_key;
      }
      break;
      case community_federation_type::MODERATOR_FEDERATION:
      {
         public_key = community.community_moderator_key;
      }
      break;
      case community_federation_type::ADMIN_FEDERATION:
      {
         public_key = community.community_admin_key;
      }
      break;
      default:
      {
         FC_ASSERT( false, 
            "Invalid Federation type." );
      }
   }

   community_name_type community_a_name;
   community_name_type community_b_name;

   if( community.id < federated_community.id )      // Community objects are sorted with lowest ID is community A.
   {
      community_a_name = community.name;
      community_b_name = federated_community.name;
   }
   else
   {
      community_b_name = community.name;
      community_a_name = federated_community.name;
   }

   const auto& federation_idx = _db.get_index< community_federation_index >().indices().get< by_communities >();
   auto federation_itr = federation_idx.find( std::make_tuple( community_a_name, community_b_name, community_federation ) );

   if( federation_itr == federation_idx.end() )       // No existing federation object of that type, creating new federation.
   {
      FC_ASSERT( o.accepted,
         "Federation of this type doesn't exist, must select to accept new federation" );

      switch( community_federation )
      {
         case community_federation_type::MEMBER_FEDERATION:
         {
            // No requirement for member level
         }
         break;
         case community_federation_type::MODERATOR_FEDERATION:
         {
            federation_itr = federation_idx.find( std::make_tuple( community_a_name, community_b_name, community_federation_type::MEMBER_FEDERATION ) );
            FC_ASSERT( federation_itr != federation_idx.end(),
               "Moderator Federation first requires member federation" );
         }
         break;
         case community_federation_type::ADMIN_FEDERATION:
         {
            federation_itr = federation_idx.find( std::make_tuple( community_a_name, community_b_name, community_federation_type::MODERATOR_FEDERATION ) );
            FC_ASSERT( federation_itr != federation_idx.end(),
               "Admin Federation first requires moderator federation" );
         }
         break;
         default:
         {
            FC_ASSERT( false, 
               "Invalid Federation type." );
         }
      }
      
      const community_federation_object& new_federation = _db.create< community_federation_object >( [&]( community_federation_object& cfo )
      {
         cfo.community_a = community_a_name;
         cfo.community_b = community_b_name;

         if( community_a_name == community.name )      // We're community A
         {
            cfo.encrypted_key_a = encrypted_keypair_type( federated_community.community_member_key, public_key, o.encrypted_community_key );
            from_string( cfo.message_a, o.message );
            from_string( cfo.json_a, o.json );
            cfo.share_accounts_a = o.share_accounts;
            cfo.approved_a = true;
         } 
         else        // We're community B
         {
            cfo.encrypted_key_b = encrypted_keypair_type( federated_community.community_member_key, public_key, o.encrypted_community_key );
            from_string( cfo.message_b, o.message );
            from_string( cfo.json_b, o.json );
            cfo.share_accounts_b = o.share_accounts;
            cfo.approved_b = true;
         }

         cfo.federation_type = community_federation;
         from_string( cfo.federation_id, o.federation_id );
         cfo.last_updated = now;
         cfo.created = now;
      });

      ilog( "Account: ${a} accepted new federation between community ${c} and ${f} - \n ${nf} \n",
         ("a",o.account)("c",o.community)("f",o.federated_community)("nf",new_federation));
   }
   else 
   {
      const community_federation_object& federation_obj = *federation_itr;

      if( o.accepted )           // Federation object found, adding returning acceptance or editing keys.
      {
         _db.modify( federation_obj, [&]( community_federation_object& cfo )
         {
            if( community_a_name == community.name )        // We're Community A
            {
               cfo.encrypted_key_a = encrypted_keypair_type( federated_community.community_member_key, public_key, o.encrypted_community_key );
               from_string( cfo.message_a, o.message );
               from_string( cfo.json_a, o.json );
               cfo.share_accounts_a = o.share_accounts;
               cfo.approved_a = true;
            } 
            else           // We're Community B
            {
               cfo.encrypted_key_b = encrypted_keypair_type( federated_community.community_member_key, public_key, o.encrypted_community_key );
               from_string( cfo.message_b, o.message );
               from_string( cfo.json_b, o.json );
               cfo.share_accounts_b = o.share_accounts;
               cfo.approved_b = true;
            }
         });

         if( federation_obj.approved() )
         {
            _db.process_community_federation( federation_obj );
         }
      }
      else       // Federation object is found, and is being removed.
      {
         _db.remove_community_federation( federation_obj );
      }

      ilog( "Account: ${a} updated federation between community: ${c} and ${f} - \n ${nf} \n",
      ("a", o.account)("c",o.community )("f",o.federated_community)("nf",federation_obj));
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_event_evaluator::do_apply( const community_event_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));

   const community_object& community = _db.get_community( o.community );

   FC_ASSERT( community.active, 
      "Community: ${s} must be active to subscribe.",
      ("s", o.community) );

   FC_ASSERT( o.event_price.symbol == community.reward_currency,
      "Event Price must be denominated in the Reward Currency of the community: ${s}.",
      ("s", community.reward_currency) );

   const community_permission_object& community_permission = _db.get_community_permission( o.community );

   FC_ASSERT( community_permission.is_authorized_event( o.account ),
      "Account: ${a} is not authorized to create events in community: ${c}.",
      ("a",o.account)("c",o.community));

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   if( community_permission.private_community )
   {
      FC_ASSERT( o.public_key.size(),
         "Events in Private Communities should be encrypted." );
      FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
         public_key_type( o.public_key ) == community.community_moderator_key || 
         public_key_type( o.public_key ) == community.community_admin_key ||
         public_key_type( o.public_key ) == community.community_secure_key ||
         public_key_type( o.public_key ) == community.community_standard_premium_key ||
         public_key_type( o.public_key ) == community.community_mid_premium_key ||
         public_key_type( o.public_key ) == community.community_top_premium_key,
         "Events in Private Communities must be encrypted with a community key.");
   }
   
   time_point now = _db.head_block_time();

   const auto& event_idx = _db.get_index< community_event_index >().indices().get< by_event_id >();
   auto event_itr = event_idx.find( boost::make_tuple( o.community, o.event_id ) );

   const auto& attend_idx = _db.get_index< community_event_attend_index >().indices().get< by_event_id >();
   auto attend_itr = attend_idx.lower_bound( boost::make_tuple( o.community, o.event_id ) );

   if( event_itr == event_idx.end() )
   {
      FC_ASSERT( o.active,
         "Event with this ID does not exist to remove." );

      _db.create< community_event_object >( [&]( community_event_object& ceo )
      {
         ceo.community = o.community;
         from_string( ceo.event_id, o.event_id );
         if( o.public_key.size() )
         {
            ceo.public_key = public_key_type( o.public_key );
         }
         from_string( ceo.event_name, o.event_name );
         from_string( ceo.location, o.location );
         ceo.latitude = o.latitude;
         ceo.longitude = o.longitude;

         if( o.details.size() )
         {
            from_string( ceo.details, o.details );
         }
         if( o.url.size() )
         {
            from_string( ceo.url, o.url );
         }
         if( o.json.size() )
         {
            from_string( ceo.json, o.json );
         }
         if( o.interface.size() )
         {
            ceo.interface = o.interface;
         }
         
         ceo.event_price = o.event_price;
         ceo.event_start_time = o.event_start_time;
         ceo.event_end_time = o.event_end_time;
         ceo.last_updated = now;
         ceo.created = now;
      });
   }
   else
   {
      const community_event_object& event = *event_itr;

      if( o.active )
      {
         if( attend_itr == attend_idx.end() || 
            attend_itr->community != event.community || 
            attend_itr->event_id != event.event_id )     // No attendees yet
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               from_string( ceo.event_name, o.event_name );
               from_string( ceo.location, o.location );
               ceo.latitude = o.latitude;
               ceo.longitude = o.longitude;

               if( o.details.size() )
               {
                  from_string( ceo.details, o.details );
               }
               if( o.url.size() )
               {
                  from_string( ceo.url, o.url );
               }
               if( o.json.size() )
               {
                  from_string( ceo.json, o.json );
               }
               
               ceo.event_price = o.event_price;
               ceo.event_start_time = o.event_start_time;
               ceo.event_end_time = o.event_end_time;
               ceo.last_updated = now;
            });
         }
         else    // Cannot change event price after first attendee
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               from_string( ceo.event_name, o.event_name );
               from_string( ceo.location, o.location );
               ceo.latitude = o.latitude;
               ceo.longitude = o.longitude;

               if( o.details.size() )
               {
                  from_string( ceo.details, o.details );
               }
               if( o.url.size() )
               {
                  from_string( ceo.url, o.url );
               }
               if( o.json.size() )
               {
                  from_string( ceo.json, o.json );
               }
               
               ceo.event_start_time = o.event_start_time;
               ceo.event_end_time = o.event_end_time;
               ceo.last_updated = now;
            });
         }
      }
      else
      {
         while( attend_itr != attend_idx.end() && 
            attend_itr->community == event.community && 
            attend_itr->event_id == event.event_id )
         {
            const community_event_attend_object& attend = *attend_itr;
            ++attend_itr;

            _db.remove( attend );
         }
         _db.remove( event );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_event_attend_evaluator::do_apply( const community_event_attend_operation& o )
{ try {
   const account_object& attendee = _db.get_account( o.attendee );
   FC_ASSERT( attendee.active,
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.attendee));

   const community_event_object& event = _db.get_community_event( o.community, o.event_id );
   const community_object& community = _db.get_community( o.community );

   FC_ASSERT( community.active,
      "Community: ${c} must be active to attend event: ${i}.",
      ("c", o.community)("i",o.event_id));

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   const community_permission_object& community_permission = _db.get_community_permission( o.community );
   
   FC_ASSERT( community_permission.is_member( o.attendee ),
      "Account: ${a} cannot interact with events within the community ${c}.",
      ("a",o.attendee)("c",o.community));

   if( community_permission.private_community )
   {
      FC_ASSERT( o.public_key.size(),
         "Events in Private Communities should be encrypted." );
      FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
         public_key_type( o.public_key ) == community.community_moderator_key || 
         public_key_type( o.public_key ) == community.community_admin_key ||
         public_key_type( o.public_key ) == community.community_secure_key ||
         public_key_type( o.public_key ) == community.community_standard_premium_key ||
         public_key_type( o.public_key ) == community.community_mid_premium_key ||
         public_key_type( o.public_key ) == community.community_top_premium_key,
         "Events in Private Communities must be encrypted with a community key.");
   }
   
   time_point now = _db.head_block_time();

   const auto& attend_idx = _db.get_index< community_event_attend_index >().indices().get< by_event_id >();
   auto attend_itr = attend_idx.lower_bound( boost::make_tuple( o.community, o.event_id, o.attendee ) );

   if( attend_itr == attend_idx.end() )    // No existing attendance object
   {
      FC_ASSERT( o.active,
         "Event Attendance for the attendee for this Event ID does not exist to remove." );

      if( o.attending )
      {
         asset liquid = _db.get_liquid_balance( o.attendee, event.event_price.symbol );

         FC_ASSERT( liquid.amount >= event.event_price.amount, 
            "Account: ${a} has insufficient liquid balance to attend event.",
            ("a",o.attendee) );

         _db.adjust_liquid_balance( o.attendee, -event.event_price );
         _db.adjust_reward_balance( community_permission.founder, event.event_price );
      }

      _db.create< community_event_attend_object >( [&]( community_event_attend_object& ceao )
      {
         ceao.attendee = o.attendee;
         ceao.community = o.community;
         from_string( ceao.event_id, o.event_id );
         if( o.public_key.size() )
         {
            ceao.public_key = public_key_type( o.public_key );
         }
         if( o.message.size() )
         {
            from_string( ceao.message, o.message );
         }
         if( o.json.size() )
         {
            from_string( ceao.json, o.json );
         }
         if( o.interface.size() )
         {
            ceao.interface = o.interface;
         }
         ceao.interested = o.interested;
         ceao.attending = o.attending;
         ceao.last_updated = now;
         ceao.created = now;
      });

      _db.modify( event, [&]( community_event_object& ceo )
      {
         if( o.attending )
         {
            ceo.attending++;
         }
         else
         {
            ceo.not_attending++;
         }
         if( o.interested )
         {
            ceo.interested++;
         }

         ceo.last_updated = now;
      });
   }
   else
   {
      const community_event_attend_object& attend = *attend_itr;

      if( o.active )
      {
         if( o.attending && !attend.attending )    // Update to attending
         {
            asset liquid = _db.get_liquid_balance( o.attendee, event.event_price.symbol );

            FC_ASSERT( liquid.amount >= event.event_price.amount, 
               "Account: ${a} has insufficient liquid balance to attend event.",
               ("a",o.attendee) );

            _db.adjust_liquid_balance( o.attendee, -event.event_price );
            _db.adjust_reward_balance( community_permission.founder, event.event_price );

            _db.modify( event, [&]( community_event_object& ceo )
            {
               ceo.attending++;
               ceo.not_attending--;
               ceo.last_updated = now;
            });
         }
         else if( !o.attending && attend.attending )    // Update to not attending
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               ceo.attending--;
               ceo.not_attending++;
               ceo.last_updated = now;
            });
         }

         if( o.interested && !attend.interested )    // Update to interested
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               ceo.interested++;
               ceo.last_updated = now;
            });
         }
         else if( !o.interested && attend.interested )    // Update to not interested
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               ceo.interested--;
               ceo.last_updated = now;
            });
         }

         _db.modify( attend, [&]( community_event_attend_object& ceao )
         {
            if( o.message.size() )
            {
               from_string( ceao.message, o.message );
            }
            if( o.json.size() )
            {
               from_string( ceao.json, o.json );
            }
            if( o.interface.size() )
            {
               ceao.interface = o.interface;
            }
            ceao.interested = o.interested;
            ceao.attending = o.attending;
            ceao.last_updated = now;
         });
      }
      else
      {
         if( !attend.attending )    // Deduct Not Attending
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               ceo.not_attending--;
               ceo.last_updated = now;
            });
         }
         else if( attend.attending )    // Deduct attending
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               ceo.attending--;
               ceo.last_updated = now;
            });
         }
         if( attend.interested )    // Deduct interested
         {
            _db.modify( event, [&]( community_event_object& ceo )
            {
               ceo.interested--;
               ceo.last_updated = now;
            });
         }

         _db.remove( attend );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_directive_evaluator::do_apply( const community_directive_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active,
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active to create directives.",
      ("s", o.community) );
   const community_permission_object& community_permission = _db.get_community_permission( o.community );
   FC_ASSERT( community_permission.is_authorized_directive( o.account ),
      "Account: ${a} is not authorized to create directives within community: ${c}.",
      ("a",o.account)("c",o.community));

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   if( community_permission.private_community )
   {
      FC_ASSERT( o.public_key.size(),
         "Directives in Private Communities should be encrypted." );
      FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
         public_key_type( o.public_key ) == community.community_moderator_key || 
         public_key_type( o.public_key ) == community.community_admin_key ||
         public_key_type( o.public_key ) == community.community_secure_key ||
         public_key_type( o.public_key ) == community.community_standard_premium_key ||
         public_key_type( o.public_key ) == community.community_mid_premium_key ||
         public_key_type( o.public_key ) == community.community_top_premium_key,
         "Directives in Private Communities must be encrypted with a community public key." );
   }
   
   time_point now = _db.head_block_time();

   const auto& directive_idx = _db.get_index< community_directive_index >().indices().get< by_directive_id >();
   auto directive_itr = directive_idx.find( boost::make_tuple( o.account, o.directive_id ) );
   const community_directive_object* root_directive_ptr = nullptr;
   uint16_t depth = 0;

   if( o.parent_account.size() || o.parent_directive_id.size() )
   {
      auto parent_itr = directive_idx.find( boost::make_tuple( o.parent_account, o.parent_directive_id ) );

      FC_ASSERT( parent_itr != directive_idx.end(), 
         "Parent directive not found" );

      root_directive_ptr = &( _db.get( parent_itr->root_directive ) );
      depth = parent_itr->depth+1;
   }

   if( directive_itr == directive_idx.end() )
   {
      FC_ASSERT( o.active,
         "Directive with this ID does not exist to remove." );

      _db.create< community_directive_object >( [&]( community_directive_object& cdo )
      {
         cdo.account = o.account;
         from_string( cdo.directive_id, o.directive_id );
         if( o.parent_account.size() || o.parent_directive_id.size() )
         {
            cdo.parent_account = o.parent_account;
            from_string( cdo.parent_directive_id, o.parent_directive_id );
            cdo.root_directive = root_directive_ptr->id;
            cdo.root = false;
         }
         else
         {
            cdo.parent_account = ROOT_POST_PARENT;
            from_string( cdo.parent_directive_id, "" );
            cdo.root_directive = cdo.id;
            cdo.root = true;
         }
         
         cdo.community = o.community;
         
         if( o.public_key.size() )
         {
            cdo.public_key = public_key_type( o.public_key );
         }
         if( o.interface.size() )
         {
            cdo.interface = o.interface;
         }
         if( o.details.size() )
         {
            from_string( cdo.details, o.details );
         }
         if( o.cover_image.size() )
         {
            from_string( cdo.cover_image, o.cover_image );
         }
         if( o.ipfs.size() )
         {
            from_string( cdo.ipfs, o.ipfs );
         }
         if( o.json.size() )
         {
            from_string( cdo.json, o.json );
         }
         cdo.depth = depth;
         cdo.net_votes = 0;
         cdo.directive_start_time = o.directive_start_time;
         cdo.directive_end_time = o.directive_end_time;
         cdo.last_updated = now;
         cdo.created = now;
         cdo.completed = false;
      });
   }
   else
   {
      const community_directive_object& directive = *directive_itr;

      if( o.active )
      {
         _db.modify( directive, [&]( community_directive_object& cdo )
         {
            if( o.public_key.size() )
            {
               cdo.public_key = public_key_type( o.public_key );
            }

            if( o.details.size() )
            {
               from_string( cdo.details, o.details );
            }
            if( o.cover_image.size() )
            {
               from_string( cdo.cover_image, o.cover_image );
            }
            if( o.ipfs.size() )
            {
               from_string( cdo.ipfs, o.ipfs );
            }
            if( o.json.size() )
            {
               from_string( cdo.json, o.json );
            }
            
            cdo.directive_start_time = o.directive_start_time;
            cdo.directive_end_time = o.directive_end_time;
            cdo.last_updated = now;
            cdo.completed = o.completed;
         });
      }
      else
      {
         _db.remove( directive );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_directive_vote_evaluator::do_apply( const community_directive_vote_operation& o )
{ try {
   const account_object& voter = _db.get_account( o.voter );
   FC_ASSERT( voter.active,
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.voter));

   const community_directive_object& directive = _db.get_community_directive( o.account, o.directive_id );
   const community_object& community = _db.get_community( directive.community );

   FC_ASSERT( community.active && community.enable_directives(), 
      "Community: ${s} must be active and have directives enabled to create directives.",
      ("s", community.name) );

   const community_permission_object& community_permission = _db.get_community_permission( directive.community );

   FC_ASSERT( community_permission.is_authorized_directive( o.account ),
      "Account: ${a} is not authorized to vote on directives within community: ${c}.",
      ("a",o.account)("c",directive.community));

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   if( community_permission.private_community )
   {
      FC_ASSERT( o.public_key.size(),
         "Directives in Private Communities should be encrypted." );
      FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
         public_key_type( o.public_key ) == community.community_moderator_key || 
         public_key_type( o.public_key ) == community.community_admin_key ||
         public_key_type( o.public_key ) == community.community_secure_key ||
         public_key_type( o.public_key ) == community.community_standard_premium_key ||
         public_key_type( o.public_key ) == community.community_mid_premium_key ||
         public_key_type( o.public_key ) == community.community_top_premium_key,
         "Directives in Private Communities must be encrypted with a community public key." );
   }
   
   time_point now = _db.head_block_time();

   const auto& vote_idx = _db.get_index< community_directive_vote_index >().indices().get< by_voter_community_directive >();
   auto vote_itr = vote_idx.find( boost::make_tuple( o.voter, directive.community, directive.id ) );
   
   if( vote_itr == vote_idx.end() )
   {
      FC_ASSERT( o.active,
         "Directive with this ID does not exist to remove." );

      _db.create< community_directive_vote_object >( [&]( community_directive_vote_object& cdvo )
      {
         cdvo.voter = o.voter;
         cdvo.directive = directive.id;
         cdvo.community = community.name;
         
         if( o.public_key.size() )
         {
            cdvo.public_key = public_key_type( o.public_key );
         }
         if( o.interface.size() )
         {
            cdvo.interface = o.interface;
         }
         if( o.details.size() )
         {
            from_string( cdvo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( cdvo.json, o.json );
         }
         cdvo.approve = o.approve;
         cdvo.last_updated = now;
         cdvo.created = now;
      });

      _db.modify( directive, [&]( community_directive_object& cdo )
      {
         if( o.approve )
         {
            cdo.net_votes++;
         }
         else
         {
            cdo.net_votes--;
         }
         cdo.last_updated = now;
      });
   }
   else
   {
      const community_directive_vote_object& vote = *vote_itr;

      if( o.active )
      {
         _db.modify( vote, [&]( community_directive_vote_object& cdvo )
         {
            if( o.public_key.size() )
            {
               cdvo.public_key = public_key_type( o.public_key );
            }
            if( o.details.size() )
            {
               from_string( cdvo.details, o.details );
            }
            if( o.json.size() )
            {
               from_string( cdvo.json, o.json );
            }
            cdvo.approve = o.approve;
            cdvo.last_updated = now;
         });

         _db.modify( directive, [&]( community_directive_object& cdo )
         {
            if( o.approve && !vote.approve )
            {
               cdo.net_votes = cdo.net_votes+2;
            }
            else if( !o.approve && vote.approve )
            {
               cdo.net_votes = cdo.net_votes-2;
            }
            cdo.last_updated = now;
         });
      }
      else
      {
         _db.modify( directive, [&]( community_directive_object& cdo )
         {
            if( vote.approve )
            {
               cdo.net_votes--;
            }
            else
            {
               cdo.net_votes++;
            }
            cdo.last_updated = now;
         });

         _db.remove( vote );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_directive_member_evaluator::do_apply( const community_directive_member_operation& o )
{ try {
   const account_object& account = _db.get_account( o.account );
   FC_ASSERT( account.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.account));

   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active && community.enable_directives(), 
      "Community: ${s} must be active and have directives enabled to create directives.",
      ("s", o.community) );

   const community_permission_object& community_permission = _db.get_community_permission( o.community );
   FC_ASSERT( community_permission.is_authorized_directive( o.account ),
      "Account: ${a} is not authorized to create a directive member within community: ${c}.",
      ("a",o.account)("c",o.community));

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   if( community_permission.private_community )
   {
      FC_ASSERT( o.public_key.size(),
         "Directives in Private Communities should be encrypted." );
      FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
         public_key_type( o.public_key ) == community.community_moderator_key || 
         public_key_type( o.public_key ) == community.community_admin_key ||
         public_key_type( o.public_key ) == community.community_secure_key ||
         public_key_type( o.public_key ) == community.community_standard_premium_key ||
         public_key_type( o.public_key ) == community.community_mid_premium_key ||
         public_key_type( o.public_key ) == community.community_top_premium_key,
         "Directives in Private Communities must be encrypted with a community public key." );
   }

   time_point now = _db.head_block_time();

   if( o.command_directive_id.size() )
   {
      const community_directive_object& command_directive = _db.get_community_directive( o.account, o.command_directive_id );
      FC_ASSERT( command_directive.root && !command_directive.completed,
         "Must assign an incomplete root command directive as an output directive for community directive member." );
   }

   if( o.delegate_directive_id.size() )
   {
      const community_directive_object& delegate_directive = _db.get_community_directive( o.account, o.delegate_directive_id );
      FC_ASSERT( delegate_directive.root && !delegate_directive.completed,
         "Must assign an incomplete root delegate directive as an output directive for community directive member." );
   }

   if( o.consensus_directive_id.size() )
   {
      const community_directive_object& consensus_directive = _db.get_community_directive( o.account, o.consensus_directive_id );
      FC_ASSERT( consensus_directive.root && !consensus_directive.completed,
         "Must assign an incomplete root consensus directive as an output directive for community directive member." );
   }

   if( o.emergent_directive_id.size() )
   {
      const community_directive_object& emergent_directive = _db.get_community_directive( o.account, o.emergent_directive_id );
      FC_ASSERT( emergent_directive.root && !emergent_directive.completed,
         "Must assign an incomplete root emergent directive as an output directive for community directive member." );
   }

   const auto& member_idx = _db.get_index< community_directive_member_index >().indices().get< by_member_community >();
   auto member_itr = member_idx.find( boost::make_tuple( o.account, o.community ) );

   if( member_itr == member_idx.end() )
   {
      _db.create< community_directive_member_object >( [&]( community_directive_member_object& cdmo )
      {
         cdmo.account = o.account;
         cdmo.community = o.community;
         
         if( o.interface.size() )
         {
            cdmo.interface = o.interface;
         }
         if( o.public_key.size() )
         {
            cdmo.public_key = public_key_type( o.public_key );
         }
         if( o.details.size() )
         {
            from_string( cdmo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( cdmo.json, o.json );
         }

         from_string( cdmo.command_directive_id, o.command_directive_id );
         from_string( cdmo.delegate_directive_id, o.delegate_directive_id );
         from_string( cdmo.consensus_directive_id, o.consensus_directive_id );
         from_string( cdmo.emergent_directive_id, o.emergent_directive_id );
         cdmo.active = true;
         cdmo.last_updated = now;
         cdmo.created = now;
      });

      if( o.command_directive_id.size() )
      {
         const community_directive_object& command_directive = _db.get_community_directive( o.account, o.command_directive_id );

         _db.modify( command_directive, [&]( community_directive_object& cdo )
         {
            cdo.member_active = true;
            cdo.last_updated = now;
         });
      }

      if( o.delegate_directive_id.size() )
      {
         const community_directive_object& delegate_directive = _db.get_community_directive( o.account, o.delegate_directive_id );

         _db.modify( delegate_directive, [&]( community_directive_object& cdo )
         {
            cdo.member_active = true;
            cdo.last_updated = now;
         });
      }

      if( o.consensus_directive_id.size() )
      {
         const community_directive_object& consensus_directive = _db.get_community_directive( o.account, o.consensus_directive_id );

         _db.modify( consensus_directive, [&]( community_directive_object& cdo )
         {
            cdo.member_active = true;
            cdo.last_updated = now;
         });
      }

      if( o.emergent_directive_id.size() )
      {
         const community_directive_object& emergent_directive = _db.get_community_directive( o.account, o.emergent_directive_id );

         _db.modify( emergent_directive, [&]( community_directive_object& cdo )
         {
            cdo.member_active = true;
            cdo.last_updated = now;
         });
      }
   }
   else
   {
      const community_directive_member_object& member = *member_itr;

      if( to_string( member.command_directive_id ) != o.command_directive_id || member.active != o.active  )
      {
         if( o.command_directive_id.size() && o.active )
         {
            const community_directive_object& new_command_directive = _db.get_community_directive( o.account, o.command_directive_id );

            _db.modify( new_command_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = true;
               cdo.last_updated = now;
            });
         }

         if( member.command_directive_id.size() && member.active )
         {
            const community_directive_object& old_command_directive = _db.get_community_directive( o.account, member.command_directive_id );

            _db.modify( old_command_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = false;
               cdo.last_updated = now;
            });
         }
      }

      if( to_string( member.delegate_directive_id ) != o.delegate_directive_id || member.active != o.active )
      {
         if( o.delegate_directive_id.size() && o.active )
         {
            const community_directive_object& new_delegate_directive = _db.get_community_directive( o.account, o.delegate_directive_id );

            _db.modify( new_delegate_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = true;
               cdo.last_updated = now;
            });
         }

         if( member.delegate_directive_id.size() && member.active )
         {
            const community_directive_object& old_delegate_directive = _db.get_community_directive( o.account, member.delegate_directive_id );

            _db.modify( old_delegate_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = false;
               cdo.last_updated = now;
            });
         }
      }

      if( to_string( member.consensus_directive_id ) != o.consensus_directive_id || member.active != o.active )
      {
         if( o.consensus_directive_id.size() && o.active )
         {
            const community_directive_object& new_consensus_directive = _db.get_community_directive( o.account, o.consensus_directive_id );

            _db.modify( new_consensus_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = true;
               cdo.last_updated = now;
            });
         }

         if( member.consensus_directive_id.size() && member.active )
         {
            const community_directive_object& old_consensus_directive = _db.get_community_directive( o.account, member.consensus_directive_id );

            _db.modify( old_consensus_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = false;
               cdo.last_updated = now;
            });
         }
      }

      if( to_string( member.emergent_directive_id ) != o.emergent_directive_id || member.active != o.active )
      {
         if( o.emergent_directive_id.size() && o.active )
         {
            const community_directive_object& new_emergent_directive = _db.get_community_directive( o.account, o.emergent_directive_id );

            _db.modify( new_emergent_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = true;
               cdo.last_updated = now;
            });
         }

         if( member.emergent_directive_id.size() && member.active )
         {
            const community_directive_object& old_emergent_directive = _db.get_community_directive( o.account, member.emergent_directive_id );

            _db.modify( old_emergent_directive, [&]( community_directive_object& cdo )
            {
               cdo.member_active = false;
               cdo.last_updated = now;
            });
         }
      }

      _db.modify( member, [&]( community_directive_member_object& cdmo )
      {
         if( o.public_key.size() )
         {
            cdmo.public_key = public_key_type( o.public_key );
         }
         if( o.details.size() )
         {
            from_string( cdmo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( cdmo.json, o.json );
         }

         from_string( cdmo.command_directive_id, o.command_directive_id );
         from_string( cdmo.delegate_directive_id, o.delegate_directive_id );
         from_string( cdmo.consensus_directive_id, o.consensus_directive_id );
         from_string( cdmo.emergent_directive_id, o.emergent_directive_id );
         cdmo.active = o.active;
         cdmo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_directive_member_vote_evaluator::do_apply( const community_directive_member_vote_operation& o )
{ try {
   const account_object& voter = _db.get_account( o.voter );
   FC_ASSERT( voter.active, 
      "Account: ${s} must be active to broadcast transaction.",
      ("s", o.voter) );
   
   const community_directive_member_object& voter_directive = _db.get_community_directive_member( o.voter, o.community );
   const community_directive_member_object& member_directive = _db.get_community_directive_member( o.member, o.community );
   const community_object& community = _db.get_community( o.community );

   FC_ASSERT( community.active, 
      "Community: ${s} must be active to create directives.",
      ("s",o.community));
   FC_ASSERT( voter_directive.active, 
      "Community Directive Member: ${m} must be active to create directive member votes.",
      ("m",voter_directive));
   FC_ASSERT( member_directive.active, 
      "Community Directive Member: ${m} must be active to create directive member votes.",
      ("m",member_directive));

   const community_permission_object& community_permission = _db.get_community_permission( o.community );

   FC_ASSERT( community_permission.is_authorized_directive( o.voter ),
      "Account: ${a} is not authorized to vote on community directive members in community: ${c}.",
      ("a",o.voter)("c",o.community));
   FC_ASSERT( community_permission.is_authorized_directive( o.member ),
      "Account: ${a} is not authorized to be voted as a community directive members in community: ${c}.",
      ("a",o.member)("c",o.community));

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",
         ("s", o.interface) );
   }

   if( community_permission.private_community )
   {
      FC_ASSERT( o.public_key.size(),
         "Directives in Private Communities should be encrypted." );
      FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
         public_key_type( o.public_key ) == community.community_moderator_key || 
         public_key_type( o.public_key ) == community.community_admin_key ||
         public_key_type( o.public_key ) == community.community_secure_key ||
         public_key_type( o.public_key ) == community.community_standard_premium_key ||
         public_key_type( o.public_key ) == community.community_mid_premium_key ||
         public_key_type( o.public_key ) == community.community_top_premium_key,
         "Directives in Private Communities must be encrypted with a community public key." );
   }
   
   time_point now = _db.head_block_time();

   const auto& vote_idx = _db.get_index< community_directive_member_vote_index >().indices().get< by_voter_community_member >();
   auto vote_itr = vote_idx.find( boost::make_tuple( o.voter, o.community, o.member ) );
   
   if( vote_itr == vote_idx.end() )
   {
      FC_ASSERT( o.active,
         "Directive with this ID does not exist to remove." );

      _db.create< community_directive_member_vote_object >( [&]( community_directive_member_vote_object& cdmvo )
      {
         cdmvo.voter = o.voter;
         cdmvo.member = o.member;
         cdmvo.community = o.community;
         
         if( o.public_key.size() )
         {
            cdmvo.public_key = public_key_type( o.public_key );
         }
         if( o.interface.size() )
         {
            cdmvo.interface = o.interface;
         }
         if( o.details.size() )
         {
            from_string( cdmvo.details, o.details );
         }
         if( o.json.size() )
         {
            from_string( cdmvo.json, o.json );
         }
         cdmvo.approve = o.approve;
         cdmvo.last_updated = now;
         cdmvo.created = now;
      });

      _db.modify( member_directive, [&]( community_directive_member_object& cdmo )
      {
         if( o.approve )
         {
            cdmo.net_votes++;
         }
         else
         {
            cdmo.net_votes--;
         }
         
         cdmo.last_updated = now;
      });
   }
   else
   {
      const community_directive_member_vote_object& vote = *vote_itr;

      if( o.active )
      {
         _db.modify( member_directive, [&]( community_directive_member_object& cdmo )
         {
            if( o.approve && !vote.approve )    // Changing from not approve to approve
            {
               cdmo.net_votes = cdmo.net_votes+2;
            }
            else if( !o.approve && vote.approve )
            {
               cdmo.net_votes = cdmo.net_votes-2;
            }
            
            cdmo.last_updated = now;
         });

         _db.modify( vote, [&]( community_directive_member_vote_object& cdmvo )
         {
            if( o.public_key.size() )
            {
               cdmvo.public_key = public_key_type( o.public_key );
            }
            if( o.interface.size() )
            {
               cdmvo.interface = o.interface;
            }
            if( o.details.size() )
            {
               from_string( cdmvo.details, o.details );
            }
            if( o.json.size() )
            {
               from_string( cdmvo.json, o.json );
            }
            cdmvo.approve = o.approve;
            cdmvo.last_updated = now;
         });
      }
      else
      {
         _db.modify( member_directive, [&]( community_directive_member_object& cdmo )
         {
            if( vote.approve )    // Removing approval
            {
               cdmo.net_votes--;
            }
            else                  // Removing disapproval
            {
               cdmo.net_votes++;
            }
            
            cdmo.last_updated = now;
         });

         _db.remove( vote );
      }
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain