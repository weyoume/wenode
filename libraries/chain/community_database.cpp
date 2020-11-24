#include <node/protocol/node_operations.hpp>
#include <node/chain/block_summary_object.hpp>
#include <node/chain/custom_operation_interpreter.hpp>
#include <node/chain/database.hpp>
#include <node/chain/database_exceptions.hpp>
#include <node/chain/db_with.hpp>
#include <node/chain/evaluator_registry.hpp>
#include <node/chain/global_property_object.hpp>
#include <node/chain/history_object.hpp>
#include <node/chain/index.hpp>
#include <node/chain/node_evaluator.hpp>
#include <node/chain/node_objects.hpp>
#include <node/chain/transaction_object.hpp>
#include <node/chain/shared_db_merkle.hpp>
#include <node/chain/operation_notification.hpp>
#include <node/chain/producer_schedule.hpp>

#include <node/chain/util/asset.hpp>
#include <node/chain/util/reward.hpp>
#include <node/chain/util/uint256.hpp>
#include <node/chain/util/reward.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/uint128.hpp>

#include <fc/container/deque.hpp>

#include <fc/io/fstream.hpp>

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>


namespace node { namespace chain {

const community_object& database::get_community( const community_name_type& community )const
{ try {
	return get< community_object, by_name >( community );
} FC_CAPTURE_AND_RETHROW( (community) ) }

const community_object* database::find_community( const community_name_type& community )const
{
   return find< community_object, by_name >( community );
}

const community_permission_object& database::get_community_permission( const community_name_type& community )const
{ try {
	return get< community_permission_object, by_name >( community );
} FC_CAPTURE_AND_RETHROW( (community) ) }

const community_permission_object* database::find_community_permission( const community_name_type& community )const
{
   return find< community_permission_object, by_name >( community );
}

const community_event_object& database::get_community_event( const community_name_type& community, const shared_string& event_id )const
{ try {
	return get< community_event_object, by_event_id >( boost::make_tuple( community, event_id ) );
} FC_CAPTURE_AND_RETHROW( (community)(event_id) ) }

const community_event_object* database::find_community_event( const community_name_type& community, const shared_string& event_id )const
{
   return find< community_event_object, by_event_id >( boost::make_tuple( community, event_id ) );
}

const community_event_object& database::get_community_event( const community_name_type& community, const string& event_id )const
{ try {
	return get< community_event_object, by_event_id >( boost::make_tuple( community, event_id ) );
} FC_CAPTURE_AND_RETHROW( (community)(event_id) ) }

const community_event_object* database::find_community_event( const community_name_type& community, const string& event_id )const
{
   return find< community_event_object, by_event_id >( boost::make_tuple( community, event_id ) );
}

const community_directive_object& database::get_community_directive( const account_name_type& account, const shared_string& directive_id )const
{ try {
	return get< community_directive_object, by_directive_id >( boost::make_tuple( account, directive_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(directive_id) ) }

const community_directive_object* database::find_community_directive( const account_name_type& account, const shared_string& directive_id )const
{
   return find< community_directive_object, by_directive_id >( boost::make_tuple( account, directive_id ) );
}

const community_directive_object& database::get_community_directive( const account_name_type& account, const string& directive_id )const
{ try {
	return get< community_directive_object, by_directive_id >( boost::make_tuple( account, directive_id ) );
} FC_CAPTURE_AND_RETHROW( (account)(directive_id) ) }

const community_directive_object* database::find_community_directive( const account_name_type& account, const string& directive_id )const
{
   return find< community_directive_object, by_directive_id >( boost::make_tuple( account, directive_id ) );
}

const community_directive_member_object& database::get_community_directive_member( const account_name_type& member, const community_name_type& community )const
{ try {
	return get< community_directive_member_object, by_member_community >( boost::make_tuple( member, community ) );
} FC_CAPTURE_AND_RETHROW( (member)(community) ) }

const community_directive_member_object* database::find_community_directive_member( const account_name_type& member, const community_name_type& community )const
{
   return find< community_directive_member_object, by_member_community >( boost::make_tuple( member, community ) );
}


/**
 * Aligns community moderator votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_community_member_votes( const account_object& account, const community_name_type& community )
{ try {
   const auto& vote_idx = get_index< community_member_vote_index >().indices().get< by_account_community_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, community ) );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->community == community )
   {
      const community_member_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( community_member_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });

         ilog( "Account: ${a} Updated Community Member Vote: ${m} of rank: ${r} \n ${p} \n",
            ("a",account.name)("m",vote.member)("r",new_vote_rank)("p",vote));
      }
      ++vote_itr;
      new_vote_rank++;
   }

   update_community_moderators( community );
} FC_CAPTURE_AND_RETHROW() }


/**
 * Aligns community moderator votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_community_member_votes( const account_object& account, const community_name_type& community, 
   const account_name_type& member, uint16_t input_vote_rank )
{ try {
   const auto& vote_idx = get_index< community_member_vote_index >().indices().get< by_account_community_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, community ) );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->community == community )
   {
      const community_member_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( community_member_vote_object& cmvo )
         {
            cmvo.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            cmvo.last_updated = now;
         });

         ilog( "Account: ${a} Updated Community Member Vote: ${m} of rank: ${r} \n ${p} \n",
            ("a",account.name)("m",vote.member)("r",new_vote_rank)("p",vote));
      }
      new_vote_rank++;
      ++vote_itr;
   }

   const community_member_vote_object& community_member_vote = create< community_member_vote_object >([&]( community_member_vote_object& cmvo )
   {
      cmvo.account = account.name;
      cmvo.member = member;
      cmvo.community = community;
      cmvo.vote_rank = input_vote_rank;
      cmvo.last_updated = now;
      cmvo.created = now;
   });

   ilog( "Account: ${a} Added Community Member Vote: ${m} of rank: ${r} \n ${p} \n",
      ("a",account.name)("m",member)("r",input_vote_rank)("p",community_member_vote));

   update_community_moderators( community );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the voting power map of the moderators
 * in a community, which determines the distribution of the
 * moderation rewards for the community.
 */
void database::update_community_moderators( const community_name_type& community )
{ try {
   asset_symbol_type reward_currency = get_community( community ).reward_currency;
   const asset_currency_data_object& currency = get_currency_data( reward_currency );
   price equity_price = get_liquidity_pool( reward_currency, currency.equity_asset ).hour_median_price;
   flat_map< account_name_type, share_type > vote_weight;
   share_type total = 0;

   time_point now = head_block_time();

   ilog( "Begin Updating Community Moderators: \n ${c} \n",
      ("c",community) );

   const auto& vote_idx = get_index< community_member_vote_index >().indices().get< by_community_member_account_rank >();
   auto vote_itr = vote_idx.lower_bound( community );

   while( vote_itr != vote_idx.end() && 
      vote_itr->community == community )
   {
      const community_member_vote_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      share_type weight = get_voting_power( vote.account, equity_price );
      
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      share_type w = share_type( weight.value >> vote.vote_rank );

      // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.

      if( vote_weight.count( vote.member ) == 0 )
      {
         vote_weight[ vote.member ] = 0;
      }

      vote_weight[ vote.member ] += w;
      total += w;
      ++vote_itr;
   }

   const community_permission_object& community_permission = get_community_permission( community );
   
   modify( community_permission, [&]( community_permission_object& cpo )
   {
      cpo.vote_weight = vote_weight;
      cpo.total_vote_weight = total;
      cpo.last_updated = now;
   });

   vote_itr = vote_idx.begin();
   while( vote_itr != vote_idx.end() )
   {
      const community_member_vote_object& vote = *vote_itr;

      ilog( "Active Community Member Vote: \n ${v} \n",
         ("v",vote));

      ++vote_itr;
   }

   ilog( "Updated Community moderators: \n ${c} \n",
      ("c",community_permission) );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the voting power map of the moderators
 * in a community, which determines the distribution of
 * moderation rewards for the community.
 */
void database::update_community_moderator_set()
{ try {
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;
   
   const auto& community_idx = get_index< community_permission_index >().indices().get< by_name >();
   auto community_itr = community_idx.begin();

   while( community_itr != community_idx.end() )
   {
      update_community_moderators( community_itr->name );
      ++community_itr;
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Deducts the monthly premium membership fee for communities.
 * 
 * Pays this price to the founder of the community, 
 * and the members according to incoming community member votes.
 */
void database::process_community_premium_membership( const community_member_object& member )
{ try {
   const community_object& community = get_community( member.community );

   asset membership_price;

   switch( member.member_type )
   {
      case community_permission_type::STANDARD_PREMIUM_PERMISSION:
      {
         membership_price = community.standard_membership_price;
      }
      break;
      case community_permission_type::MID_PREMIUM_PERMISSION:
      {
         membership_price = community.mid_membership_price;
      }
      break;
      case community_permission_type::TOP_PREMIUM_PERMISSION:
      {
         membership_price = community.top_membership_price;
      }
      break;
      case community_permission_type::ALL_PERMISSION:
      case community_permission_type::MEMBER_PERMISSION:
      case community_permission_type::MODERATOR_PERMISSION:
      case community_permission_type::ADMIN_PERMISSION:
      case community_permission_type::FOUNDER_PERMISSION:
      case community_permission_type::NONE_PERMISSION:
      default:
      {
         FC_ASSERT( false, "Community Permission Type is invalid for a premium membership." );
      }
      break;
   }

   const account_object& member_account = get_account( member.member );
   const account_object& interface_account = get_account( member.interface );

   asset fee = ( membership_price * SUBSCRIPTION_FEE_PERCENT ) / PERCENT_100;
   asset remaining = membership_price - fee;
   asset total_paid = asset( 0, membership_price.symbol );

   asset network_fee = ( fee * NETWORK_SUBSCRIPTION_FEE_PERCENT ) / PERCENT_100;
   asset interface_fee = ( fee * INTERFACE_SUBSCRIPTION_FEE_PERCENT ) / PERCENT_100;

   asset network_paid = pay_network_fees( member_account, network_fee );
   asset interface_paid = pay_fee_share( interface_account, interface_fee, true );
   total_paid += network_paid;
   total_paid += interface_paid;

   adjust_liquid_balance( member.member, -membership_price );
   
   asset member_share = ( remaining * MEMBER_VOTE_SUBSCRIPTION_PERCENT ) / PERCENT_100;

   const community_permission_object& community_permission = get_community_permission( member.community );

   uint128_t total_weight( community_permission.total_vote_weight.value );
   
   for( auto mod : community_permission.vote_weight )
   {
      uint128_t weight( mod.second.value );
      uint128_t reward_amount = ( member_share.amount.value * weight ) / total_weight;
      asset reward = asset( int64_t( reward_amount.to_uint64() ), member_share.symbol );

      if( reward.amount > 0 )
      {
         const account_object& a = get_account( mod.first );
         adjust_reward_balance( a.name, reward );
         total_paid += reward;
      }
   }

   adjust_reward_balance( community_permission.founder, ( membership_price - total_paid ) );

   ilog( "Account: ${a} paid premium subscription amount: ${s} for community: ${c}",
      ("a",member.member)("s",membership_price.to_string())("c",member.community));
   
} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the state of community member objects 
 * membership, moderator, and admin lists and federation connections,
 * based on the federations that exist between them.
 * 
 * Cascades all upstream and downstream federation sets, 
 * and adds all memberships from upstream federations into the communities.
 */
void database::process_community_federation( const community_federation_object& federation )
{ try {
   const community_permission_object& community_permission_a = get_community_permission( federation.community_a );
   const community_permission_object& community_permission_b = get_community_permission( federation.community_b );
   time_point now = head_block_time();
   
   switch( federation.federation_type )
   {
      case community_federation_type::MEMBER_FEDERATION:
      {
         if( federation.share_accounts_a )       // B is accepted as A's Upstream Community
         {
            modify( community_permission_a, [&]( community_permission_object& cmo )
            {
               cmo.upstream_member_federations.insert( federation.community_b );
               for( auto name : community_permission_b.members )
               {
                  cmo.add_member( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_permission_b.members )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_member_community( federation.community_a );
                  afo.last_updated = now;
               });
            }

            modify( community_permission_b, [&]( community_permission_object& cmo )
            {
               cmo.downstream_member_federations.insert( federation.community_a );
               cmo.last_updated = now;
            });
         }
         if( federation.share_accounts_b )      // A is accepted as B's Upstream Community
         {
            modify( community_permission_b, [&]( community_permission_object& cmo )
            {
               cmo.upstream_member_federations.insert( federation.community_a );
               for( auto name : community_permission_a.members )
               {
                  cmo.add_member( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_permission_a.members )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_member_community( federation.community_b );
                  afo.last_updated = now;
               });
            }

            modify( community_permission_a, [&]( community_permission_object& cmo )
            {
               cmo.downstream_member_federations.insert( federation.community_b );
               cmo.last_updated = now;
            });
         }
      }
      break;
      case community_federation_type::MODERATOR_FEDERATION:
      {
         if( federation.share_accounts_a )       // B is accepted as A's Upstream Community
         {
            modify( community_permission_a, [&]( community_permission_object& cmo )
            {
               cmo.upstream_moderator_federations.insert( federation.community_b );
               for( auto name : community_permission_b.moderators )
               {
                  cmo.add_moderator( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_permission_b.moderators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_moderator_community( federation.community_a );
                  afo.last_updated = now;
               });
            }

            modify( community_permission_b, [&]( community_permission_object& cmo )
            {
               cmo.downstream_moderator_federations.insert( federation.community_a );
               cmo.last_updated = now;
            });
         }
         if( federation.share_accounts_b )      // A is accepted as B's Upstream Community
         {
            modify( community_permission_b, [&]( community_permission_object& cmo )
            {
               cmo.upstream_moderator_federations.insert( federation.community_a );
               for( auto name : community_permission_a.moderators )
               {
                  cmo.add_moderator( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_permission_a.moderators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_moderator_community( federation.community_b );
                  afo.last_updated = now;
               });
            }

            modify( community_permission_a, [&]( community_permission_object& cmo )
            {
               cmo.downstream_moderator_federations.insert( federation.community_b );
               cmo.last_updated = now;
            });
         }
      }
      break;
      case community_federation_type::ADMIN_FEDERATION:
      {
         if( federation.share_accounts_a )      // B is accepted as A's Upstream Community
         {
            modify( community_permission_a, [&]( community_permission_object& cmo )
            {
               cmo.upstream_admin_federations.insert( federation.community_b );
               for( auto name : community_permission_b.administrators )
               {
                  cmo.add_administrator( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_permission_b.administrators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_admin_community( federation.community_a );
                  afo.last_updated = now;
               });
            }

            modify( community_permission_b, [&]( community_permission_object& cmo )
            {
               cmo.downstream_admin_federations.insert( federation.community_a );
               cmo.last_updated = now;
            });
         }
         if( federation.share_accounts_b )      // A is accepted as B's Upstream Community
         {
            modify( community_permission_b, [&]( community_permission_object& cmo )
            {
               cmo.upstream_admin_federations.insert( federation.community_a );
               for( auto name : community_permission_a.administrators )
               {
                  cmo.add_administrator( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_permission_a.administrators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_admin_community( federation.community_b );
                  afo.last_updated = now;
               });
            }

            modify( community_permission_a, [&]( community_permission_object& cmo )
            {
               cmo.downstream_admin_federations.insert( federation.community_b );
               cmo.last_updated = now;
            });
         }
      }
      break;
      default:
      {
         FC_ASSERT( false, 
            "Invalid Federation type." );
      }
   }

} FC_CAPTURE_AND_RETHROW() }


/**
 * Removes the community federation from all community member objects
 * and deletes a federation object.
 */
void database::remove_community_federation( const community_federation_object& federation )
{ try {
   const community_permission_object& community_permission_a = get_community_permission( federation.community_a );
   const community_permission_object& community_permission_b = get_community_permission( federation.community_b );
   time_point now = head_block_time();

   modify( community_permission_a, [&]( community_permission_object& cmo )
   {
      switch( federation.federation_type )
      {
         case community_federation_type::MEMBER_FEDERATION:
         {
            cmo.upstream_member_federations.erase( federation.community_b );
            cmo.downstream_member_federations.erase( federation.community_b );
         }
         break;
         case community_federation_type::MODERATOR_FEDERATION:
         {
            cmo.upstream_moderator_federations.erase( federation.community_b );
            cmo.downstream_moderator_federations.erase( federation.community_b );
         }
         break;
         case community_federation_type::ADMIN_FEDERATION:
         {
            cmo.upstream_admin_federations.erase( federation.community_b );
            cmo.downstream_admin_federations.erase( federation.community_b );
         }
         break;
         default:
         {
            FC_ASSERT( false, 
               "Invalid Federation type." );
         }
      }

      cmo.last_updated = now;
   });

   modify( community_permission_b, [&]( community_permission_object& cmo )
   {
      switch( federation.federation_type )
      {
         case community_federation_type::MEMBER_FEDERATION:
         {
            cmo.upstream_member_federations.erase( federation.community_a );
            cmo.downstream_member_federations.erase( federation.community_a );
         }
         break;
         case community_federation_type::MODERATOR_FEDERATION:
         {
            cmo.upstream_moderator_federations.erase( federation.community_a );
            cmo.downstream_moderator_federations.erase( federation.community_a );
         }
         break;
         case community_federation_type::ADMIN_FEDERATION:
         {
            cmo.upstream_admin_federations.erase( federation.community_a );
            cmo.downstream_admin_federations.erase( federation.community_a );
         }
         break;
         default:
         {
            FC_ASSERT( false, 
               "Invalid Federation type." );
         }
      }

      cmo.last_updated = now;
   });

   ilog( "Removed: ${v}",("v",federation));
   remove( federation );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Checks if an account satisfies the verification requirements of a community.
 * 
 * Initially checks if the account has the required number of incoming verifications.
 * Then Recursively checks for the existence of a verification 
 * pathway with a degree lower than the required maximum distance.
 */
bool database::check_community_verification( const account_name_type& account, const community_permission_object& community, uint16_t depth )
{ try {
   const auto& verification_idx = get_index< account_verification_index >().indices().get< by_verified_verifier >();

   if( community.verifiers.empty() || community.min_verification_count == 0 )
   {
      return true;    // No verification checks required
   }

   if( depth == 0 )
   {
      auto verification_itr = verification_idx.lower_bound( account );
      uint16_t verification_count = 0;

      while( verification_itr != verification_idx.end() && 
         verification_itr->verified_account == account )
      {
         verification_count++;
         ++verification_itr;
      }
      if( verification_count < community.min_verification_count )
      {
         return false;
      }
   }

   if( depth > community.max_verification_distance )      // Base Case, end recursion.
   {
      return false;
   }
   else    // Recursive case
   {
      for( auto verifier : community.verifiers )     // Check if account is verified by the community verifiers.
      {
         auto verification_itr = verification_idx.find( boost::make_tuple( verifier, account ) );

         if( verification_itr != verification_idx.end() )
         {
            return true;    // Account found a valid verification pathway, pass true to the top frame.
         }
      }

      auto verification_itr = verification_idx.lower_bound( account );

      while( verification_itr != verification_idx.end() && 
         verification_itr->verified_account == account )    // Cycle through all verifiers of the account to find pathway.
      {
         if( check_community_verification( verification_itr->verifier_account, community, depth+1 ) )
         {
            return true;   // If any recursive call finds a valid pathway, return true
         }
         ++verification_itr;
      }

      return false;
   }

} FC_CAPTURE_AND_RETHROW() }

} } //node::chain