
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

/**
 * COMMUNITY TYPES
 * 
 * OPEN_PUBLIC_COMMUNITY,         ///< All Users can read, interact, post, and request to join. Accounts cannot be blacklisted.
 * GENERAL_PUBLIC_COMMUNITY,      ///< All Users can read, interact, post, and request to join.
 * EXCLUSIVE_PUBLIC_COMMUNITY,    ///< All users can read, interact, and request to join. Members can post and invite.
 * CLOSED_PUBLIC_COMMUNITY,       ///< All users can read, and request to join. Members can interact, post, and invite.
 * OPEN_PRIVATE_COMMUNITY,        ///< Members can read and interact, and create posts. Moderators can invite and accept.
 * GENERAL_PRIVATE_COMMUNITY,     ///< Members can read and interact, and create posts. Moderators can invite and accept. Cannot share posts.
 * EXCLUSIVE_PRIVATE_COMMUNITY,   ///< Members can read and interact, and post. Cannot share posts or request to join. Admins can invite and accept.
 * CLOSED_PRIVATE_COMMUNITY       ///< Members can read and interact. Moderators can post. Cannot share posts or request to join. Admins can invite and accept.
 */
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
   FC_ASSERT( now >= founder.last_community_created + MIN_COMMUNITY_CREATE_INTERVAL,
      "Founder: ${f} can only create one community per day, try again after: ${t}.",
      ("f",o.founder)("t", founder.last_community_created + MIN_COMMUNITY_CREATE_INTERVAL) );
   const community_object* community_ptr = _db.find_community( o.name );
   FC_ASSERT( community_ptr == nullptr,
      "Community with the name: ${n} already exists. Please select another name.", ("n", o.name) );
   const asset_object& reward_currency = _db.get_asset( o.reward_currency );
    FC_ASSERT( reward_currency.asset_type == asset_property_type::CURRENCY_ASSET,
      "Reward currency asset must be either a currency type asset." );

   if( o.interface.size() )
   {
      const account_object& interface_acc = _db.get_account( o.interface );
      FC_ASSERT( interface_acc.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
      const interface_object& interface = _db.get_interface( o.interface );
      FC_ASSERT( interface.active, 
         "Interface: ${s} must be active to broadcast transaction.",("s", o.interface) );
   }

   community_privacy_type privacy_type = community_privacy_type::OPEN_PUBLIC_COMMUNITY;

   for( size_t i = 0; i < community_privacy_values.size(); i++ )
   {
      if( o.community_privacy == community_privacy_values[ i ] )
      {
         privacy_type = community_privacy_type( i );
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

      co.community_privacy = privacy_type;
      co.community_member_key = public_key_type( o.community_member_key );
      co.community_moderator_key = public_key_type( o.community_moderator_key );
      co.community_admin_key = public_key_type( o.community_admin_key );
      co.interface = o.interface;

      co.max_rating = o.max_rating;
      co.flags = o.flags;
      co.permissions = o.permissions;
      co.reward_currency = o.reward_currency;
      co.membership_price = o.membership_price;
      co.created = now;
      co.last_updated = now;
      co.last_post = now;
      co.last_root_post = now;
      co.active = true;
   });

   const community_member_object& new_community_member = _db.create< community_member_object >( [&]( community_member_object& cmo )
   {
      cmo.name = o.name;
      cmo.founder = o.founder;
      cmo.subscribers.insert( o.founder );
      cmo.members.insert( o.founder );
      cmo.moderators.insert( o.founder );
      cmo.administrators.insert( o.founder );
      cmo.community_privacy = privacy_type;
      cmo.last_updated = now;
   });

   _db.create< community_moderator_vote_object >( [&]( community_moderator_vote_object& v )
   {
      v.moderator = o.founder;
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

   _db.update_community_moderators( new_community_member );

   ilog( "Founder: ${f} created new Community with privacy type: ${t}: \n ${c} \n ${m} \n",
      ("f", o.founder)("t",privacy_type)("c", new_community)("m",new_community_member) );

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

   // New permissions must be subset of old permissions.
   
   FC_ASSERT(!( o.permissions & ~community.permissions ), 
      "Cannot reinstate previously revoked community permissions.");
   
   // Changed flags must be subset of old permissions.

   FC_ASSERT(!(( o.flags ^ community.flags ) & ~community.permissions ),
      "Flag change is not possible within community permissions." );
   
   time_point now = _db.head_block_time();

   FC_ASSERT( now >= ( community.last_updated + MIN_COMMUNITY_UPDATE_INTERVAL ),
      "Communities can only be updated once per 10 minutes." );

   const community_member_object& community_member = _db.get_community_member( o.community );

   FC_ASSERT( community_member.is_administrator( o.account ),
      "Only administrators of the community can update it.");

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

   ilog( "Account: ${a} Updated community: ${c}",
      ("a",o.account)("c",community.name) );

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
   time_point now = _db.head_block_time();
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
         "Account: ${a} must be a member before voting for a Moderator of Community: ${b}.",
         ("a", o.account)("b", o.community));
      FC_ASSERT( community_member.is_moderator( o.moderator ),
         "Account: ${a} must be a Moderator of Community: ${b}.",
         ("a", o.moderator)("b", o.community));
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
            v.last_updated = now;
            v.created = now;
         });
         
         _db.update_community_moderator_votes( voter, o.community );
      }
      else
      {
         if( account_community_moderator_itr != account_community_moderator_idx.end() && 
            account_community_rank_itr != account_community_rank_idx.end() )
         {
            FC_ASSERT( account_community_moderator_itr->moderator != account_community_rank_itr->moderator, 
               "Vote at rank is already for the specified moderator." );
         }
         
         if( account_community_moderator_itr != account_community_moderator_idx.end() )
         {
            ilog( "Removed: ${v}",("v",*account_community_moderator_itr));
            _db.remove( *account_community_moderator_itr );
         }

         _db.update_community_moderator_votes( voter, o.community, o.moderator, o.vote_rank );   // Remove existing moderator vote, and add at new rank. 
      }
   }
   else       // Removing existing vote
   {
      if( account_community_moderator_itr != account_community_moderator_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_community_moderator_itr));
         _db.remove( *account_community_moderator_itr );
      }
      else if( account_community_rank_itr != account_community_rank_idx.end() )
      {
         ilog( "Removed: ${v}",("v",*account_community_rank_itr));
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

   const account_object& moderator = _db.get_account( o.moderator );
   const account_following_object& account_following = _db.get_account_following( o.moderator );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for moderator voting.",
      ("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( community_member.is_member( moderator.name ), 
      "Account: ${a} must be a member before promotion to moderator of Community: ${b}.",
      ("a", o.moderator)("b", o.community));
   
   if( o.added || o.account != o.moderator )     // Account can remove itself from community administrators.
   {
      FC_ASSERT( community_member.is_administrator( o.account ), 
         "Account: ${a} is not an administrator of the Community: ${b} and cannot add or remove moderators.",
         ("a", o.account)("b", o.community));
   }

   if( o.added )
   {
      FC_ASSERT( !community_member.is_moderator( moderator.name ), 
         "Account: ${a} is already a moderator of Community: ${b}.",
         ("a", o.moderator)("b", o.community));
      FC_ASSERT( !community_member.is_administrator( moderator.name ), 
         "Account: ${a} is already a administrator of Community: ${b}.",
         ("a", o.moderator)("b", o.community));
      FC_ASSERT( community_member.founder != moderator.name, 
         "Account: ${a} is already the Founder of Community: ${b}.",
         ("a", o.moderator)("b", o.community));
   }
   else
   {
      FC_ASSERT( community_member.is_moderator( moderator.name ),
         "Account: ${a} is not a moderator of Community: ${b}.",
         ("a", o.moderator)("b", o.community));
      FC_ASSERT( !community_member.is_administrator( moderator.name ),
         "Account: ${a} cannot be removed from moderators while an administrator of Community: ${b}.",
         ("a", o.moderator)("b", o.community));
      FC_ASSERT( community_member.founder != moderator.name,
         "Account: ${a} cannot be removed while the founder of Community: ${b}.",
         ("a", o.moderator)("b", o.community));
   }

   if( o.added )
   {
      _db.modify( community_member, [&]( community_member_object& cmo )
      {
         cmo.moderators.insert( moderator.name );
         cmo.last_updated = now;
      });

      _db.modify( account_following, [&]( account_following_object& afo )
      {
         afo.add_moderator_community( o.community );
         afo.last_updated = now;
      });

      for( auto down : community_member.downstream_moderator_federations )
      {
         const community_member_object& down_community_member = _db.get_community_member( down );

         _db.modify( down_community_member, [&]( community_member_object& cmo )
         {
            cmo.members.insert( moderator.name );
            cmo.moderators.insert( moderator.name );
            cmo.last_updated = now;
         });
      }
   }
   else
   {
      _db.modify( community_member, [&]( community_member_object& cmo )
      {
         cmo.moderators.erase( moderator.name );
         cmo.last_updated = now;
      });

      _db.modify( account_following, [&]( account_following_object& afo )
      {
         afo.remove_moderator_community( o.community );
         afo.last_updated = now;
      });
   }
   
   ilog( "Account: ${a} added Moderator: ${m} to Community: ${c} - \n ${mem} \n",
      ("a",o.account)("m",o.moderator)("c",o.community)("mem",community_member));
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

   const account_object& administrator = _db.get_account( o.admin ); 
   const account_following_object& account_following = _db.get_account_following( o.admin ); 
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for adding admins.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( community_member.is_member( administrator.name ), 
      "Account: ${a} must be a member before promotion to administrator of Community: ${b}.",
      ("a", o.admin)("b", o.community));
   FC_ASSERT( community_member.is_moderator( administrator.name ), 
      "Account: ${a} must be a moderator before promotion to administrator of Community: ${b}.",
      ("a", o.admin)("b", o.community));

   if( o.added || o.account != administrator.name )     // Account can remove itself from community administrators.
   {
      FC_ASSERT( community_member.founder == o.account, 
         "Only the Founder: ${f} of the community can add or remove administrators.",
         ("f", community_member.founder));
   }
   if( o.added )
   {
      FC_ASSERT( !community_member.is_administrator( administrator.name ),
         "Account: ${a} is already an administrator of Community: ${b}.",
         ("a", o.admin)("b", o.community));
      FC_ASSERT( community_member.founder != administrator.name, 
         "Account: ${a} is already the Founder of Community: ${b}.",
         ("a", o.admin)("b", o.community));
   }
   else
   {
      FC_ASSERT( community_member.is_administrator( administrator.name ),
         "Account: ${a} is not an administrator of Community: ${b}.",
         ("a", o.admin)("b", o.community));
      FC_ASSERT( community_member.founder != administrator.name,
         "Account: ${a} cannot be removed as administrator while the Founder of Community: ${b}.",
         ("a", o.admin)("b", o.community));
   }
   
   if( o.added )
   {
      _db.modify( community_member, [&]( community_member_object& cmo )
      {
         cmo.administrators.insert( administrator.name );
         cmo.last_updated = now;
      });

      _db.modify( account_following, [&]( account_following_object& afo )
      {
         afo.add_admin_community( o.community );
         afo.last_updated = now;
      });

      for( auto down : community_member.downstream_admin_federations )
      {
         const community_member_object& down_community_member = _db.get_community_member( down );

         _db.modify( down_community_member, [&]( community_member_object& cmo )
         {
            cmo.administrators.insert( administrator.name );
            cmo.last_updated = now;
         });

         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.add_admin_community( down );
            afo.last_updated = now;
         });
      }
   }
   else
   {
      _db.modify( community_member, [&]( community_member_object& cmo )
      {
         cmo.administrators.erase( administrator.name );
         cmo.last_updated = now;
      });

      _db.modify( account_following, [&]( account_following_object& afo )
      {
         afo.remove_admin_community( o.community );
         afo.last_updated = now;
      });
   }
   

   ilog( "Account: ${a} added Administrator: ${ad} to Community: ${c} - \n ${mem} \n",
      ("a",o.account)("ad",o.admin)("c",o.community)("mem",community_member));
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
      "Community: ${s} must be active for ownership transfer.",
      ("s", o.community) );
   const account_object& new_founder = _db.get_account( o.new_founder );
   FC_ASSERT( new_founder.active, 
      "Account: ${s} must be active to become the new community founder.",
      ("s", o.new_founder) );
   time_point now = _db.head_block_time();
   const community_member_object& community_member = _db.get_community_member( o.community );
   
   FC_ASSERT( community.founder == o.account && 
      community_member.founder == o.account,
      "Only the founder: ${f} of the community can transfer ownership.",
      ("f",community.founder) );
   FC_ASSERT( now > community.last_updated + MIN_COMMUNITY_UPDATE_INTERVAL, 
      "Communities can only be updated once per 10 minutes." );
   FC_ASSERT( community_member.is_member( o.new_founder ), 
      "Account: ${a} must be an administrator before promotion to founder of Community: ${b}.",
      ("a", o.new_founder)("b", o.community));
   FC_ASSERT( community_member.is_moderator( o.new_founder ), 
      "Account: ${a} must be an administrator before promotion to founder of Community: ${b}.",
      ("a", o.new_founder)("b", o.community));
   FC_ASSERT( community_member.is_administrator( o.new_founder ), 
      "Account: ${a} must be an administrator before promotion to founder of Community: ${b}.",
      ("a", o.new_founder)("b", o.community));

   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.new_founder );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.account );

   const account_following_object& new_account_following = _db.get_account_following( o.new_founder );
   const account_following_object& account_following = _db.get_account_following( o.account );

   FC_ASSERT( to_account_permissions.is_authorized_transfer( o.account ),
      "Transfer is not authorized, due to recipient account's permisssions" );
   FC_ASSERT( from_account_permissions.is_authorized_transfer( o.new_founder ),
      "Transfer is not authorized, due to sender account's permisssions" );

   _db.modify( community, [&]( community_object& co )
   {
      co.founder = o.new_founder;
      co.last_updated = now;
   });

   _db.modify( community_member, [&]( community_member_object& cmo )
   {
      cmo.founder = o.new_founder;
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
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active,
      "Community: ${s} must be active for join requests.",
      ("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   FC_ASSERT( !community_member.is_member( o.account ),
      "Account: ${a} is already a member of the community: ${b}.",
      ("a", o.account)("b", o.community));
   FC_ASSERT( community_member.is_authorized_request( o.account ),
      "Account: ${a} is not authorised to request to join the community: ${b}.",
      ("a", o.account)("b", o.community));

   time_point now = _db.head_block_time();
   const auto& req_idx = _db.get_index< community_join_request_index >().indices().get< by_account_community >();
   auto itr = req_idx.find( std::make_tuple( o.account, o.community ) );

   if( itr == req_idx.end())    // Request does not exist yet
   {
      FC_ASSERT( o.requested,
         "Community join request does not exist, requested should be set to true." );

      _db.create< community_join_request_object >( [&]( community_join_request_object& bjro )
      {
         bjro.account = o.account;
         bjro.community = community.name;
         from_string( bjro.message, o.message );
         bjro.expiration = now + CONNECTION_REQUEST_DURATION;
      });
   }
   else     // Request exists and is being deleted
   {
      FC_ASSERT( !o.requested,
         "Request already exists, Requested should be set to false to remove existing request." );
      ilog( "Removed: ${v}",("v",*itr));
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
   const account_object& member = _db.get_account( o.member );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for join invitations.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   FC_ASSERT( !community_member.is_member( member.name ), 
      "Account: ${a} is already a member of the community: ${b}.",
      ("a", o.member)("b", o.community));
   FC_ASSERT( community_member.is_authorized_invite( o.account ),
      "Account: ${a} is not authorised to send community: ${b} join invitations.",
      ("a", o.account)("b", o.community));

   const account_permission_object& to_account_permissions = _db.get_account_permissions( o.member );
   const account_permission_object& from_account_permissions = _db.get_account_permissions( o.account );
   
   time_point now = _db.head_block_time();
   const auto& key_idx = _db.get_index< community_member_key_index >().indices().get< by_member_community_type >();
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

      _db.create< community_join_invite_object >( [&]( community_join_invite_object& cjio )
      {
         cjio.account = o.account;
         cjio.member = member.name;
         cjio.community = community.name;
         from_string( cjio.message, o.message );
         cjio.expiration = now + CONNECTION_REQUEST_DURATION;
      });

      _db.create< community_member_key_object >( [&]( community_member_key_object& cmko )
      {
         cmko.account = o.account;
         cmko.member = member.name;
         cmko.community = o.community;
         cmko.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
      });
   }
   else     // Invite exists and is being deleted.
   {
      FC_ASSERT( !o.invited,
         "Invite already exists, Invited should be set to false to remove existing Invitation." );
      
      ilog( "Removed: ${v}",("v",*inv_itr));
      _db.remove( *inv_itr );

      auto key_itr = key_idx.lower_bound( std::make_tuple( o.member, o.community ) );

      while( key_itr != key_idx.end() && 
         key_itr->member == o.member && 
         key_itr->community == o.community )
      {
         const community_member_key_object& key = *key_itr;
         ++key_itr;

         ilog( "Removed: ${v}",("v",key));
         _db.remove( key );
      }
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

   const account_object& member = _db.get_account( o.member );
   const account_following_object& account_following = _db.get_account_following( o.member );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for join acceptance.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( !community_member.is_member( member.name ),
      "Account: ${a} is already a member of the community: ${b}.",
      ("a",o.member)("b",o.community));
   FC_ASSERT( community_member.is_authorized_invite( o.account ), 
      "Account: ${a} is not authorized to accept join requests to the community: ${b}.",
      ("a",o.account)("b",o.community));

   const auto& req_idx = _db.get_index< community_join_request_index >().indices().get< by_account_community >();
   auto req_itr = req_idx.find( boost::make_tuple( o.member, o.community ) );

   FC_ASSERT( req_itr != req_idx.end(),
      "Community join request does not exist.");    // Ensure Request exists

   if( o.accepted )   // Accepting the request, skipped if rejecting
   {
      _db.modify( community_member, [&]( community_member_object& cmo )
      {
         cmo.members.insert( member.name );
         cmo.last_updated = now;
      });

      _db.modify( account_following, [&]( account_following_object& afo )
      {
         afo.add_member_community( community.name );
         afo.last_updated = now;
      });

      for( auto down : community_member.downstream_member_federations )
      {
         const community_member_object& down_community_member = _db.get_community_member( down );

         _db.modify( down_community_member, [&]( community_member_object& cmo )
         {
            cmo.members.insert( member.name );
            cmo.last_updated = now;
         });

         _db.modify( account_following, [&]( account_following_object& afo )
         {
            afo.add_member_community( down );
            afo.last_updated = now;
         });
      }

      _db.create< community_member_key_object >( [&]( community_member_key_object& cmko )
      {
         cmko.account = o.account;
         cmko.member = member.name;
         cmko.community = o.community;
         cmko.encrypted_community_key = encrypted_keypair_type( member.secure_public_key, community.community_member_key, o.encrypted_community_key );
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

   const community_object& community = _db.get_community( o.community );
   const account_following_object& account_following = _db.get_account_following( o.account );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active for invite acceptance.",
      ("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( !community_member.is_member( o.account ), 
      "Account: ${a} is already a member of the community: ${b}.",
      ("a", o.account)("b", o.community));
      
   const auto& inv_idx = _db.get_index< community_join_invite_index >().indices().get< by_member_community >();
   auto inv_itr = inv_idx.find( boost::make_tuple( o.account, o.community ) );
   FC_ASSERT( inv_itr != inv_idx.end(),
      "Community join invitation does not exist.");   // Ensure Invitation exists
   const community_join_invite_object& invite = *inv_itr;

   FC_ASSERT( community_member.is_authorized_invite( invite.account ), 
      "Account: ${a} is not authorised to send community: ${b} join invitations.",
      ("a", invite.account)("b", o.community));  // Ensure inviting account is still authorised to send invitations
   
   if( o.accepted )   // Accepting the request, skipped if rejecting
   {
      _db.modify( community_member, [&]( community_member_object& cmo )
      {
         cmo.members.insert( o.account );
         cmo.last_updated = now;
      });

      _db.modify( account_following, [&]( account_following_object& afo )
      {
         afo.add_member_community( o.community );
         afo.last_updated = now;
      });

      for( auto down : community_member.downstream_member_federations )
      {
         const community_member_object& down_community_member = _db.get_community_member( down );

         _db.modify( down_community_member, [&]( community_member_object& cmo )
         {
            cmo.members.insert( o.account );
            cmo.last_updated = now;
         });
      }
   }
   else
   {
      const auto& key_idx = _db.get_index< community_member_key_index >().indices().get< by_member_community_type >();
      auto key_itr = key_idx.find( boost::make_tuple( o.account, o.community ) );

      while( key_itr != key_idx.end() && 
         key_itr->member == o.account && 
         key_itr->community == o.community )
      {
         const community_member_key_object& key = *key_itr;
         ++key_itr;

         ilog( "Removed: ${v}",("v",key));
         _db.remove( key );
      }
   }

   ilog( "Removed: ${v}",("v",invite));
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

   const account_object& member = _db.get_account( o.member );
   const account_following_object& account_following = _db.get_account_following( o.member );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active removing members.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   FC_ASSERT( community_member.is_member( member.name ),
      "Account: ${a} is not a member of community: ${b}",
      ("a",o.member)("b",o.community));
   FC_ASSERT( !community_member.is_moderator( member.name ),
      "Account: ${a} cannot be removed while a moderator of community: ${b}",
      ("a",o.member)("b",o.community));
   FC_ASSERT( !community_member.is_administrator( member.name ),
      "Account: ${a} cannot be removed while an administrator of community: ${b}",
      ("a",o.member)("b",o.community));
   FC_ASSERT( community_member.founder != member.name,
      "Account: ${a} cannot be removed while the founder of community: ${b}",
      ("a",o.member)("b",o.community));

   if( o.account != member.name )     // Account can remove itself from community membership.
   {
      FC_ASSERT( community_member.is_authorized_blacklist( o.account ),
         "Account: ${a} is not authorised to remove accounts from community: ${b}.",
         ("a",o.account)("b",o.community));
   }
   
   _db.modify( community_member, [&]( community_member_object& cmo )
   {
      cmo.members.erase( o.member );
      cmo.last_updated = now;
   });

   _db.modify( account_following, [&]( account_following_object& afo )
   {
      afo.remove_member_community( o.community );
      afo.last_updated = now;
   });

   const auto& key_idx = _db.get_index< community_member_key_index >().indices().get< by_member_community_type >();
   auto key_itr = key_idx.find( boost::make_tuple( o.member, o.community ) );

   while( key_itr != key_idx.end() && 
      key_itr->member == o.member && 
      key_itr->community == o.community )
   {
      const community_member_key_object& key = *key_itr;
      ++key_itr;

      ilog( "Removed: ${v}",("v",key));
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
      "Account: ${a} is not authorised to add or remove accounts from the blacklist of community: ${b}.",
      ("a", o.account)("b", o.community));
   FC_ASSERT( !community_member.is_member( member.name ),
      "Account: ${a} cannot be blacklisted while a member of a federated community: ${b}. Remove them first.",
      ("a", o.member)("b", o.community));
   FC_ASSERT( !community_member.is_moderator( member.name ),
      "Account: ${a} cannot be blacklisted while a moderator of a federated community: ${b}. Remove them first.",
      ("a", o.member)("b", o.community));
   FC_ASSERT( !community_member.is_administrator( member.name ),
      "Account: ${a} cannot be blacklisted while an administrator of a federated community: ${b}. Remove them first.",
      ("a", o.member)("b", o.community));
   FC_ASSERT( community_member.founder != member.name,
      "Account: ${a} cannot be blacklisted while the founder of community: ${b}.", ("a", o.member)("b", o.community));

   _db.modify( community_member, [&]( community_member_object& cmo )
   {
      if( o.blacklisted )
      {
         cmo.blacklist.insert( member.name );
      }
      else
      {
         cmo.blacklist.erase( member.name );
      }
      cmo.last_updated = now;
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

   const account_following_object& account_following = _db.get_account_following( o.account );
   const community_object& community = _db.get_community( o.community );
   FC_ASSERT( community.active, 
      "Community: ${s} must be active to subscribe.",("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );
   time_point now = _db.head_block_time();

   if( o.subscribed )   // Adding subscription 
   {
      FC_ASSERT( community_member.is_authorized_interact( o.account ), 
         "Account: ${a} is not authorized to subscribe to the community ${b}.",
         ("a", o.account)("b", o.community));
      FC_ASSERT( !community_member.is_subscriber( o.account ), 
         "Account: ${a} is already subscribed to the community ${b}.",
         ("a", o.account)("b", o.community));
   }
   else     // Removing subscription
   {
      FC_ASSERT( community_member.is_subscriber( o.account ), 
         "Account: ${a} is not subscribed to the community ${b}.",
         ("a", o.account)("b", o.community));
   }

   if( o.added )
   {
      if( o.subscribed )     // Add subscriber
      {
         _db.modify( community_member, [&]( community_member_object& cmo )
         {
            cmo.subscribers.insert( o.account );
            cmo.last_updated = now;
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
         _db.modify( community_member, [&]( community_member_object& cmo )
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
      "Community: ${s} must be active for federation requests.",
      ("s", o.community) );
   const community_member_object& community_member = _db.get_community_member( o.community );

   FC_ASSERT( community_member.is_administrator( o.account ),
      "Account: ${a} must be an Admin of the community: ${b}.",
      ("a", o.account)("b",o.community));

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

   const community_member_object& community_member_a = _db.get_community_member( community_a_name );
   const community_member_object& community_member_b = _db.get_community_member( community_b_name );

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

      ilog( "Account: ${a} accepted new federation between community ${b} and ${c} - \n ${f} \n",
         ("a",o.account)("b",o.community)("c",o.federated_community)("f",new_federation));
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

      ilog( "Account: ${a} updated federation between community: ${b} and ${c} - \n ${f} \n",
      ("a", o.account)("b",o.community )("c",o.federated_community)("f",federation_obj));
   }
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
      "Community: ${s} must be active to subscribe.",
      ("s", o.community) );

   FC_ASSERT( o.event_price.symbol == community.reward_currency,
      "Event Price must be denominated in the Reward Currency of the community: ${s}.",
      ("s", community.reward_currency) );

   const community_member_object& community_member = _db.get_community_member( o.community );

   FC_ASSERT( community_member.is_administrator( o.account ),
      "Only administrators of the community can create and update an event within it." );

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

   switch( community.community_privacy )
   {
      case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
      case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
      case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
      case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
      {
         // No public key required
      }
      break;
      case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
      case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
      case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
      case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
      {
         FC_ASSERT( o.public_key.size(),
            "Events in Private Communities should be encrypted." );
         FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
            public_key_type( o.public_key ) == community.community_moderator_key || 
            public_key_type( o.public_key ) == community.community_admin_key,
            "Events in Private Communities must be encrypted with a community key.");
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid Community Privacy type." );
      }
      break;
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
   const account_name_type& signed_for = o.attendee;
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

   const community_event_object& event = _db.get_community_event( o.community, o.event_id );
   const community_object& community = _db.get_community( o.community );

   FC_ASSERT( community.active, 
      "Community: ${s} must be active to attend event.",("s", o.community) );

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

   const community_member_object& community_member = _db.get_community_member( o.community );
   
   FC_ASSERT( community_member.is_authorized_interact( o.attendee ),
      "Account: ${a} cannot interact with events within the community ${c}.",
      ("a", o.attendee)("c", o.community));

   switch( community.community_privacy )
   {
      case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
      case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
      case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
      case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
      {
         // No public key required
      }
      break;
      case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
      case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
      case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
      case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
      {
         FC_ASSERT( o.public_key.size(),
            "Events in Private Communities should be encrypted." );
         FC_ASSERT( public_key_type( o.public_key ) == community.community_member_key || 
            public_key_type( o.public_key ) == community.community_moderator_key || 
            public_key_type( o.public_key ) == community.community_admin_key,
            "Events in Private Communities must be encrypted with a community key.");
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid Community Privacy type." );
      }
      break;
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
         _db.adjust_reward_balance( community_member.founder, event.event_price );
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
            _db.adjust_reward_balance( community_member.founder, event.event_price );

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


} } // node::chain