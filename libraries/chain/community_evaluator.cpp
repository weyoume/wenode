
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
   const account_name_type& signed_for = o.founder;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& founder = _db.get_account( o.founder );
   time_point now = _db.head_block_time();
   FC_ASSERT( now > founder.last_community_created + MIN_COMMUNITY_CREATE_INTERVAL,
      "Founders can only create one community per day." );
   const community_object* community_ptr = _db.find_community( o.name );
   FC_ASSERT( community_ptr == nullptr,
      "Community with the name: ${n} already exists.", ("n", o.name) );

   community_privacy_type privacy_type = community_privacy_type::OPEN_PUBLIC_COMMUNITY;

   for( size_t i = 0; i < community_privacy_values.size(); i++ )
   {
      if( o.community_privacy == community_privacy_values[ i ] )
      {
         privacy_type = community_privacy_type( i );
         break;
      }
   }

   _db.create< community_object >( [&]( community_object& bo )
   {
      bo.name = o.name;
      bo.founder = founder.name;
      bo.community_privacy = privacy_type;
      from_string( bo.json, o.json );
      from_string( bo.json_private, o.json_private );
      from_string( bo.details, o.details );
      from_string( bo.url, o.url );
      bo.community_public_key = public_key_type( o.community_public_key );
      bo.created = now;
      bo.last_community_update = now;
      bo.last_post = now;
      bo.last_root_post = now;
   });

   const community_member_object& new_community_member = _db.create< community_member_object >( [&]( community_member_object& bmo )
   {
      bmo.name = o.name;
      bmo.founder = founder.name;
      bmo.subscribers.insert( founder.name );
      bmo.members.insert( founder.name );
      bmo.moderators.insert( founder.name );
      bmo.administrators.insert( founder.name );
   });

   _db.create< community_moderator_vote_object >( [&]( community_moderator_vote_object& v )
   {
      v.moderator = founder.name;
      v.account = founder.name;
      v.community = o.name;
      v.vote_rank = 1;
   });

   _db.modify( founder, [&]( account_object& a )
   {
      a.last_community_created = now;
   });

   _db.update_community_moderators( new_community_member );

} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_update_evaluator::do_apply( const community_update_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const community_object& community = _db.get_community( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( now > ( community.last_community_update + MIN_COMMUNITY_UPDATE_INTERVAL ),
      "Communities can only be updated once per 10 minutes." );

   const community_member_object& community_member = _db.get_community_member( o.community );

   FC_ASSERT( community_member.is_administrator( o.account ),
      "Only administrators of the community can update it.");

   const comment_object* pinned_post_ptr = nullptr;

   if( o.pinned_author.size() || o.pinned_permlink.size() )
   {
      pinned_post_ptr = _db.find_comment( o.pinned_author, o.pinned_permlink );
      FC_ASSERT( pinned_post_ptr != nullptr,
         "Cannot find valid Pinned Post." );
      FC_ASSERT( pinned_post_ptr->root == true,
         "Pinned post must be a root comment." );
      FC_ASSERT( pinned_post_ptr->community == o.community,
         "Pinned post must be contained within the community." );
   }

   _db.modify( community, [&]( community_object& bo )
   {
      from_string( bo.json, o.json );
      from_string( bo.json_private, o.json_private );
      from_string( bo.details, o.details );
      from_string( bo.url, o.url );
      bo.community_public_key = public_key_type( o.community_public_key );
      bo.last_community_update = now;
      bo.active = o.active;

      if( pinned_post_ptr != nullptr )
      {
         bo.pinned_post = pinned_post_ptr->id;
      }
   });
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_vote_mod_evaluator::do_apply( const community_vote_mod_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& voter = _db.get_account( o.account );
   const account_object& moderator_account = _db.get_account( o.moderator );
   FC_ASSERT( moderator_account.active, 
      "Account: ${s} must be active to be voted as moderator.",("s", o.moderator) );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for moderator voting.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );

   if( o.approved )
   {
      FC_ASSERT( voter.can_vote, 
         "Account has declined its voting rights." );
      FC_ASSERT( community_member.is_member( o.account ),
         "Account: ${a} must be a member before voting for a moderator of Community: ${b}.", ("a", o.account)("b", o.community));
      FC_ASSERT( community_member.is_moderator( o.moderator ),
         "Account: ${a} must be a moderator of Community: ${b}.", ("a", o.moderator)("b", o.community));
   }
   
   const auto& account_community_rank_idx = _db.get_index< community_moderator_vote_index >().indices().get< by_account_community_rank >();
   const auto& account_community_moderator_idx = _db.get_index< community_moderator_vote_index >().indices().get< by_account_community_moderator >();
   auto account_community_rank_itr = account_community_rank_idx.find( boost::make_tuple( o.account, o.community, o.vote_rank ) );   // vote at rank number
   auto account_community_moderator_itr = account_community_moderator_idx.find( boost::make_tuple( o.account, o.community, o.moderator ) );    // vote for moderator in community

   if( o.approved )   // Adding or modifying vote
   {
      if( account_community_moderator_itr == account_community_moderator_idx.end() && 
         account_community_rank_itr == account_community_rank_idx.end() )       // No vote for executive or rank, create new vote.
      {
         _db.create< community_moderator_vote_object>( [&]( community_moderator_vote_object& v )
         {
            v.moderator = o.moderator;
            v.account = o.account;
            v.community = o.community;
            v.vote_rank = o.vote_rank;
         });
         
         _db.update_community_moderator_votes( voter, o.community );
      }
      else
      {
         if( account_community_moderator_itr != account_community_moderator_idx.end() && account_community_rank_itr != account_community_rank_idx.end() )
         {
            FC_ASSERT( account_community_moderator_itr->moderator != account_community_rank_itr->moderator, 
               "Vote at rank is already for the specified moderator." );
         }
         
         if( account_community_moderator_itr != account_community_moderator_idx.end() )
         {
            _db.remove( *account_community_moderator_itr );
         }

         _db.update_community_moderator_votes( voter, o.community, o.moderator, o.vote_rank );   // Remove existing moderator vote, and add at new rank. 
      }
   }
   else       // Removing existing vote
   {
      if( account_community_moderator_itr != account_community_moderator_idx.end() )
      {
         _db.remove( *account_community_moderator_itr );
      }
      else if( account_community_rank_itr != account_community_rank_idx.end() )
      {
         _db.remove( *account_community_rank_itr );
      }
      _db.update_community_moderator_votes( voter, o.community );
   }

   _db.update_community_moderators( community_member );

} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_add_mod_evaluator::do_apply( const community_add_mod_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& moderator = _db.get_account( o.moderator );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for moderator voting.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( community_member.is_member( moderator.name ), 
      "Account: ${a} must be a member before promotion to moderator of Community: ${b}.", ("a", o.moderator)("b", o.community));
   
   if( o.added || o.account != o.moderator )     // Account can remove itself from community administrators.
   {
      FC_ASSERT( community_member.is_administrator( account.name ), 
         "Account: ${a} is not an administrator of the Community: ${b} and cannot add or remove moderators.", ("a", o.account)("b", o.community));
   }

   if( o.added )
   {
      FC_ASSERT( !community_member.is_moderator( moderator.name ), 
         "Account: ${a} is already a moderator of Community: ${b}.", ("a", o.moderator)("b", o.community));
      FC_ASSERT( !community_member.is_administrator( moderator.name ), 
         "Account: ${a} is already a administrator of Community: ${b}.", ("a", o.moderator)("b", o.community));
      FC_ASSERT( community_member.founder != moderator.name, 
         "Account: ${a} is already the Founder of Community: ${b}.", ("a", o.moderator)("b", o.community));
   }
   else
   {
      FC_ASSERT( community_member.is_moderator( moderator.name ),
         "Account: ${a} is not a moderator of Community: ${b}.", ("a", o.moderator)("b", o.community));
      FC_ASSERT( !community_member.is_administrator( moderator.name ),
         "Account: ${a} cannot be removed from moderators while an administrator of Community: ${b}.", ("a", o.moderator)("b", o.community));
      FC_ASSERT( community_member.founder != moderator.name,
         "Account: ${a} cannot be removed while the founder of Community: ${b}.", ("a", o.moderator)("b", o.community));
   }

   _db.modify( community_member, [&]( community_member_object& bmo )
   {
      if( o.added )
      {
         bmo.moderators.insert( moderator.name );
         bmo.last_updated = now;
      }
      else 
      {
         bmo.moderators.erase( moderator.name );
         bmo.last_updated = now;
      }
   });

} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_add_admin_evaluator::do_apply( const community_add_admin_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& administrator = _db.get_account( o.admin ); 
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for adding admins.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( community_member.is_member( administrator.name ), 
      "Account: ${a} must be a member before promotion to administrator of Community: ${b}.", ("a", o.admin)("b", o.community));
   FC_ASSERT( community_member.is_moderator( administrator.name ), 
      "Account: ${a} must be a moderator before promotion to administrator of Community: ${b}.", ("a", o.admin)("b", o.community));

   if( o.added || account.name != administrator.name )     // Account can remove itself from community administrators.  
   {
      FC_ASSERT( community_member.founder == account.name, 
         "Only the Founder: ${f} of the community can add or remove administrators.", ("f", community_member.founder));
   }
   if(o.added)
   {
      FC_ASSERT( !community_member.is_administrator( administrator.name ), 
         "Account: ${a} is already an administrator of Community: ${b}.", ("a", o.admin)("b", o.community));
      FC_ASSERT( community_member.founder != administrator.name, 
         "Account: ${a} is already the Founder of Community: ${b}.", ("a", o.admin)("b", o.community));
   }
   else
   {
      FC_ASSERT( community_member.is_administrator( administrator.name ), 
         "Account: ${a} is not an administrator of Community: ${b}.", ("a", o.admin)("b", o.community));
      FC_ASSERT( community_member.founder != administrator.name, 
         "Account: ${a} cannot be removed as administrator while the Founder of Community: ${b}.", ("a", o.admin)("b", o.community));
   }
   
   _db.modify( community_member, [&]( community_member_object& bmo )
   {
      if( o.added )
      {
         bmo.administrators.insert( administrator.name );
         bmo.last_updated = now;
      }
      else 
      {
         bmo.administrators.erase( administrator.name );
         bmo.last_updated = now;
      }
   });
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_transfer_ownership_evaluator::do_apply( const community_transfer_ownership_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_chief( o.signatory ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for ownership transfer.",("s", o.community) );
   const account_object& account = _db.get_account( o.account );
   const account_object& new_founder = _db.get_account( o.new_founder );
   FC_ASSERT( new_founder.active, 
      "Account: ${s} must be active to become the new community founder.",("s", o.new_founder) );
   time_point now = _db.head_block_time();
   const community_member_object& community_member = _db.get_community_member( o.community );

   FC_ASSERT( community.founder == account.name && community_member.founder == account.name,
      "Only the founder of the community can transfer ownership." );
   FC_ASSERT( now > community.last_community_update + MIN_COMMUNITY_UPDATE_INTERVAL, 
      "Communities can only be updated once per 10 minutes." );

   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.new_founder );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.account );

   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.account ),
      "Transfer is not authorized, due to recipient account's permisssions" );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.new_founder ),
      "Transfer is not authorized, due to sender account's permisssions" );

   _db.modify( community, [&]( community_object& bo )
   {
      bo.founder = o.new_founder;
      bo.last_community_update = now;
   });

   _db.modify( community_member, [&]( community_member_object& bmo )
   {
      bmo.founder = o.new_founder;
      bmo.last_updated = now;
   });
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_join_request_evaluator::do_apply( const community_join_request_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active,
      "Community: ${s} must be active for join requests.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   FC_ASSERT( !community_member.is_member( account.name ),
      "Account: ${a} is already a member of the community: ${b}.", ("a", o.account)("b", o.community));
   FC_ASSERT( community_member.is_authorized_request( account.name ),
      "Account: ${a} is not authorised to request to join the community: ${b}.", ("a", o.account)("b", o.community));
   time_point now = _db.head_block_time();
   const auto& req_idx = _db.get_index< community_join_request_index >().indices().get< by_account_community >();
   auto itr = req_idx.find( std::make_tuple( o.account, o.community ) );

   if( itr == req_idx.end())    // Request does not exist yet
   {
      FC_ASSERT( o.requested,
         "Community join request does not exist, requested should be set to true." );

      _db.create< community_join_request_object >( [&]( community_join_request_object& bjro )
      {
         bjro.account = account.name;
         bjro.community = community.name;
         from_string( bjro.message, o.message );
         bjro.expiration = now + CONNECTION_REQUEST_DURATION;
      });
   }
   else     // Request exists and is being deleted
   {
      FC_ASSERT( !o.requested,
         "Request already exists, Requested should be set to false to remove existing request." );
      _db.remove( *itr );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_join_invite_evaluator::do_apply( const community_join_invite_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& member = _db.get_account( o.member );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for join invitations.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   FC_ASSERT( !community_member.is_member( member.name ), 
      "Account: ${a} is already a member of the community: ${b}.", ("a", o.member)("b", o.community));
   FC_ASSERT( community_member.is_authorized_invite( account.name ),
      "Account: ${a} is not authorised to send community: ${b} join invitations.", ("a", o.account)("b", o.community));

   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.member );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.account );
   
   time_point now = _db.head_block_time();
   const auto& key_idx = _db.get_index< community_member_key_index >().indices().get< by_member_community >();
   const auto& inv_idx = _db.get_index< community_join_invite_index >().indices().get< by_member_community >();
   auto inv_itr = inv_idx.find( std::make_tuple( o.member, o.community ) );

   if( inv_itr == inv_idx.end())    // Invite does not exist yet
   {
      FC_ASSERT( o.invited, 
         "Community invite request does not exist, invited should be set to true." );
      FC_ASSERT( to_account_permissions.is_authorized_transfer( o.account ),
         "Invite is not authorized, due to recipient account's permisssions" );
      FC_ASSERT( from_account_permissions.is_authorized_transfer( o.member ),
         "Invite is not authorized, due to sender account's permisssions" );

      _db.create< community_join_invite_object >( [&]( community_join_invite_object& bjio )
      {
         bjio.account = account.name;
         bjio.member = member.name;
         bjio.community = community.name;
         from_string( bjio.message, o.message );
         bjio.expiration = now + CONNECTION_REQUEST_DURATION;
      });

      _db.create< community_member_key_object >( [&]( community_member_key_object& bmko )
      {
         bmko.account = account.name;
         bmko.member = member.name;
         bmko.community = o.community;
         bmko.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_public_key, o.encrypted_community_key );
      });
   }
   else     // Invite exists and is being deleted.
   {
      FC_ASSERT( !o.invited,
         "Invite already exists, Invited should be set to false to remove existing Invitation." );
      
      _db.remove( *inv_itr );
      auto key_itr = key_idx.find( std::make_tuple( o.member, o.community ) );
      _db.remove( *key_itr );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_join_accept_evaluator::do_apply( const community_join_accept_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account ); 
   const account_object& member = _db.get_account( o.member );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for join acceptance.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( !community_member.is_member( member.name ),
      "Account: ${a} is already a member of the community: ${b}.", ("a", o.member)("b", o.community));
   FC_ASSERT( community_member.is_authorized_invite( account.name ), 
      "Account: ${a} is not authorized to accept join requests to the community: ${b}.", ("a", o.account)("b", o.community));    // Ensure Account is a moderator.

   const auto& req_idx = _db.get_index< community_join_request_index >().indices().get< by_account_community >();
   auto req_itr = req_idx.find( std::make_tuple( o.member, o.community ) );

   FC_ASSERT( req_itr != req_idx.end(),
      "Community join request does not exist.");    // Ensure Request exists

   if( o.accepted )   // Accepting the request, skipped if rejecting
   {
      _db.modify( community_member, [&]( community_member_object& bmo )
      {
         bmo.members.insert( member.name );
         bmo.last_updated = now;
      });

      _db.create< community_member_key_object >( [&]( community_member_key_object& bmko )
      {
         bmko.account = account.name;
         bmko.member = member.name;
         bmko.community = o.community;
         bmko.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_public_key, o.encrypted_community_key );
      });
   }

   _db.remove( *req_itr );
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_invite_accept_evaluator::do_apply( const community_invite_accept_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const account_object& account = _db.get_account( o.account );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for invite acceptance.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( !community_member.is_member( account.name ), 
      "Account: ${a} is already a member of the community: ${b}.", ("a", o.account)("b", o.community));
   const auto& inv_idx = _db.get_index< community_join_invite_index >().indices().get< by_member_community >();
   auto itr = inv_idx.find( std::make_tuple( o.account, o.community ) );
   FC_ASSERT( itr != inv_idx.end(),
      "Community join invitation does not exist.");   // Ensure Invitation exists
   const community_join_invite_object& invite = *itr;

   const auto& key_idx = _db.get_index< community_member_key_index >().indices().get< by_member_community >();
   auto key_itr = key_idx.find( std::make_tuple( o.account, o.community ) );

   FC_ASSERT( community_member.is_authorized_invite( invite.account ), 
      "Account: ${a} is no longer authorised to send community: ${b} join invitations.", ("a", invite.account)("b", o.community));  // Ensure inviting account is still authorised to send invitations
   
   if( o.accepted )   // Accepting the request, skipped if rejecting
   {
      _db.modify( community_member, [&]( community_member_object& bmo )
      {
         bmo.members.insert( account.name );
         bmo.last_updated = now;
      });
   }
   else
   {
      _db.remove( *key_itr );
   }
   
   _db.remove( invite );
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_remove_member_evaluator::do_apply( const community_remove_member_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_object& member = _db.get_account( o.member );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active removing members.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( community_member.is_member( member.name ),
      "Account: ${a} is not a member of community: ${b}.", ("a", o.member)("b", o.community));
   FC_ASSERT( !community_member.is_moderator( member.name ),
      "Account: ${a} cannot be removed while a moderator of community: ${b}.", ("a", o.member)("b", o.community));
   FC_ASSERT( !community_member.is_administrator( member.name ),
      "Account: ${a} cannot be removed while an administrator of community: ${b}.", ("a", o.member)("b", o.community));
   FC_ASSERT( community_member.founder != member.name,
      "Account: ${a} cannot be removed while the founder of community: ${b}.", ("a", o.member)("b", o.community));

   const auto& key_idx = _db.get_index< community_member_key_index >().indices().get< by_member_community >();

   if( account.name != member.name )     // Account can remove itself from community membership.  
   {
      FC_ASSERT( community_member.is_authorized_blacklist( o.account ), 
         "Account: ${a} is not authorised to remove accounts from community: ${b}.", ("a", o.account)("b", o.community)); 
   }
   
   _db.modify( community_member, [&]( community_member_object& bmo )
   {
      bmo.members.erase( member.name );
      bmo.last_updated = now;
   });

   auto key_itr = key_idx.find( std::make_tuple( o.member, o.community ) );
   if( key_itr != key_idx.end() )
   {
      const community_member_key_object& key = *key_itr;
      _db.remove( key );
   }
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_blacklist_evaluator::do_apply( const community_blacklist_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_governance( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   
   const account_object& member = _db.get_account( o.member );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for blacklist updating.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();
   
   FC_ASSERT( community_member.is_authorized_blacklist( o.account ), 
      "Account: ${a} is not authorised to add or remove accounts from the blacklist of community: ${b}.", ("a", o.account)("b", o.community)); 
   FC_ASSERT( !community_member.is_member( member.name ),
      "Account: ${a} cannot be blacklisted while a member of community: ${b}. Remove them first.", ("a", o.member)("b", o.community));
   FC_ASSERT( !community_member.is_moderator( member.name ),
      "Account: ${a} cannot be blacklisted while a moderator of community: ${b}. Remove them first.", ("a", o.member)("b", o.community));
   FC_ASSERT( !community_member.is_administrator( member.name ),
      "Account: ${a} cannot be blacklisted while an administrator of community: ${b}. Remove them first.", ("a", o.member)("b", o.community));
   FC_ASSERT( community_member.founder != member.name,
      "Account: ${a} cannot be blacklisted while the founder of community: ${b}.", ("a", o.member)("b", o.community));

   _db.modify( community_member, [&]( community_member_object& bmo )
   {
      if( o.blacklisted )
      {
         bmo.blacklist.insert( member.name );
      }
      else
      {
         bmo.blacklist.erase( member.name );
      }
      bmo.last_updated = now;
   });
} FC_CAPTURE_AND_RETHROW( ( o )) }


void community_subscribe_evaluator::do_apply( const community_subscribe_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }
   const account_object& account = _db.get_account( o.account );
   const account_following_object& account_following = _db.get_account_following( o.account );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active to subscribe.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   if( o.subscribed )   // Adding subscription 
   {
      FC_ASSERT( community_member.is_authorized_interact( account.name ), 
         "Account: ${a} is not authorized to subscribe to the community ${b}.",("a", account.name)("b", o.community));
      FC_ASSERT( !community_member.is_subscriber( account.name ), 
         "Account: ${a} is already subscribed to the community ${b}.",("a", account.name)("b", o.community));
   }
   else     // Removing subscription
   {
      FC_ASSERT( community_member.is_subscriber( account.name ), 
         "Account: ${a} is not subscribed to the community ${b}.",("a", account.name)("b", o.community));
   }

   if( o.added )
   {
      if( o.subscribed )     // Add subscriber
      {
         _db.modify( community_member, [&]( community_member_object& bmo )
         {
            bmo.add_subscriber( account.name );
            bmo.last_updated = now;
         });

         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.add_followed_community( community.name );
            afo.last_updated = now;
         });
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
         _db.modify( community_member, [&]( community_member_object& bmo )
         {
            bmo.remove_subscriber( account.name );
            bmo.last_updated = now;
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


void community_event_evaluator::do_apply( const community_event_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const community_object& community = _db.get_community( o.community );

   FC_ASSERT( community.active, 
      "Community: ${s} must be active to subscribe.",("s", o.community) );

   const community_member_object& community_member = _db.get_community_member( o.community );

   FC_ASSERT( community_member.is_administrator( o.account ),
      "Only administrators of the community can create and update events within it." );

   flat_set< account_name_type > inv;

   for( account_name_type name : o.invited )
   {
      FC_ASSERT( community_member.is_member( name ),
         "Only members of the community can be invited to events within it." );
      inv.insert( name );
   }
   
   time_point now = _db.head_block_time();

   const auto& event_idx = _db.get_index< community_event_index >().indices().get< by_community_event_name >();
   auto event_itr = event_idx.find( boost::make_tuple( o.community, o.event_name ) );

   if( event_itr == event_idx.end() )
   {
      _db.create< community_event_object >( [&]( community_event_object& ceo )
      {
         ceo.account = o.account;
         ceo.community = o.community;
         from_string( ceo.event_name, o.event_name );
         from_string( ceo.location, o.location );
         from_string( ceo.details, o.details );
         from_string( ceo.url, o.url );
         from_string( ceo.json, o.json );
         ceo.invited = inv;
         ceo.event_start_time = o.event_start_time;
         ceo.event_end_time = o.event_end_time;
         ceo.last_updated = now;
         ceo.created = now;
      });
   }
   else
   {
      const community_event_object& event = *event_itr;

      _db.modify( event, [&]( community_event_object& ceo )
      {
         from_string( ceo.location, o.location );
         from_string( ceo.details, o.details );
         from_string( ceo.url, o.url );
         from_string( ceo.json, o.json );
         ceo.event_start_time = o.event_start_time;
         ceo.event_end_time = o.event_end_time;
         ceo.last_updated = now;
         ceo.invited = inv;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


void community_event_attend_evaluator::do_apply( const community_event_attend_operation& o )
{ try {
   const account_name_type& signed_for = o.account;
   const account_object& signatory = _db.get_account( o.signatory );
   FC_ASSERT( signatory.active, 
      "Account: ${s} must be active to broadcast transaction.",("s", o.signatory) );
   if( o.signatory != signed_for )
   {
      const account_object& signed_acc = _db.get_account( signed_for );
      FC_ASSERT( signed_acc.active, 
         "Account: ${s} must be active to broadcast transaction.",("s", signed_acc) );
      const account_business_object& b = _db.get_account_business( signed_for );
      FC_ASSERT( b.is_authorized_content( o.signatory, _db.get_account_permissions( signed_for ) ), 
         "Account: ${s} is not authorized to act as signatory for Account: ${a}.",("s", o.signatory)("a", signed_for) );
   }

   const community_event_object& event = _db.get_community_event( o.community, o.event_name );
   const community_object& community = _db.get_community( o.community );

   FC_ASSERT( community.active, 
      "Community: ${s} must be active to attend event.",("s", o.community) );

   const community_member_object& community_member = _db.get_community_member( o.community );
   
   FC_ASSERT( community_member.is_authorized_interact( o.account ),
      "Account: ${a} cannot interact with events within the community ${c}.",("a", o.account)("c", o.community) );
   
   time_point now = _db.head_block_time();

   if( o.not_attending )
   {
      _db.modify( event, [&]( community_event_object& ceo )
      {
         ceo.not_attending.insert( o.account );
         ceo.last_updated = now;
      });
   }
   else
   {
      _db.modify( event, [&]( community_event_object& ceo )
      {
         ceo.not_attending.erase( o.account );
         ceo.last_updated = now;
      });
   }

   if( o.attending )
   {
      _db.modify( event, [&]( community_event_object& ceo )
      {
         ceo.attending.insert( o.account );
         ceo.last_updated = now;
      });
   }
   else
   {
      _db.modify( event, [&]( community_event_object& ceo )
      {
         ceo.attending.erase( o.account );
         ceo.last_updated = now;
      });
   }

   if( o.interested )
   {
      _db.modify( event, [&]( community_event_object& ceo )
      {
         ceo.interested.insert( o.account );
         ceo.last_updated = now;
      });
   }
   else
   {
      _db.modify( event, [&]( community_event_object& ceo )
      {
         ceo.interested.erase( o.account );
         ceo.last_updated = now;
      });
   }
} FC_CAPTURE_AND_RETHROW( ( o ) ) }


} } // node::chain