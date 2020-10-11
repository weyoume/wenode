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

const community_member_object& database::get_community_member( const community_name_type& community )const
{ try {
	return get< community_member_object, by_name >( community );
} FC_CAPTURE_AND_RETHROW( (community) ) }

const community_member_object* database::find_community_member( const community_name_type& community )const
{
   return find< community_member_object, by_name >( community );
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



/**
 * Aligns community moderator votes in order of highest to lowest,
 * with continual ordering.
 */
void database::update_community_moderator_votes( const account_object& account, const community_name_type& community )
{
   const auto& vote_idx = get_index< community_moderator_vote_index >().indices().get< by_account_community_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, community ) );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->community == community )
   {
      const community_moderator_vote_object& vote = *vote_itr;
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( community_moderator_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }
      ++vote_itr;
      new_vote_rank++;
   }
}

/**
 * Aligns community moderator votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_community_moderator_votes( const account_object& account, const community_name_type& community, 
   const account_name_type& moderator, uint16_t input_vote_rank )
{
   const auto& vote_idx = get_index< community_moderator_vote_index >().indices().get< by_account_community_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, community ) );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->community == community )
   {
      const community_moderator_vote_object& vote = *vote_itr;
      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( community_moderator_vote_object& v )
         {
            v.vote_rank = new_vote_rank;   // Updates vote rank to linear order of index retrieval.
            v.last_updated = now;
         });
      }
      new_vote_rank++;
      ++vote_itr;
   }

   create< community_moderator_vote_object >([&]( community_moderator_vote_object& v )
   {
      v.account = account.name;
      v.moderator = moderator;
      v.community = community;
      v.vote_rank = input_vote_rank;
      v.last_updated = now;
      v.created = now;
   });
}



/**
 * Updates the voting power map of the moderators
 * in a community, which determines the distribution of the
 * moderation rewards for the community.
 */
void database::update_community_moderators( const community_member_object& community )
{ try {
   ilog( "Update Community moderators: ${c}", ("c", community.name) );
   asset_symbol_type reward_currency = get_community( community.name ).reward_currency;
   const asset_currency_data_object& currency = get_currency_data( reward_currency );
   price equity_price = get_liquidity_pool( reward_currency, currency.equity_asset ).hour_median_price;
   flat_map< account_name_type, share_type > mod_weight;
   share_type total = 0;

   const auto& vote_idx = get_index< community_moderator_vote_index >().indices().get< by_community>();
   auto vote_itr = vote_idx.lower_bound( community.name );

   while( vote_itr != vote_idx.end() && 
      vote_itr->community == community.name )
   {
      const community_moderator_vote_object& vote = *vote_itr;
      const account_object& voter = get_account( vote.account );
      share_type weight = get_voting_power( vote.account, equity_price );
      
      if( voter.proxied.size() )
      {
         weight += get_proxied_voting_power( voter, equity_price );
      }
      share_type w = share_type( weight.value >> vote.vote_rank );

      // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.

      if( mod_weight.count( vote.moderator ) == 0 )
      {
         mod_weight[ vote.moderator ] = 0;
      }

      mod_weight[ vote.moderator ] += w;
      total += w;
      ++vote_itr;
   }
   
   modify( community, [&]( community_member_object& b )
   {
      b.mod_weight = mod_weight;
      b.total_mod_weight = total;
   });

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
   
   const auto& community_idx = get_index< community_member_index >().indices().get< by_name >();
   auto community_itr = community_idx.begin();

   while( community_itr != community_idx.end() )
   {
      update_community_moderators( *community_itr );
      ++community_itr;
   }
} FC_CAPTURE_AND_RETHROW() }



/**
 * Deducts the daily membership fee for communities that have a membership price.
 * 
 * Pays this price to the founder of the community.
 */
void database::process_community_membership_fees()
{ try {
   if( (head_block_num() % COMMUNITY_FEE_INTERVAL_BLOCKS != 0 ) )   // Runs once per week
      return;
   
   const auto& community_idx = get_index< community_index >().indices().get< by_membership_price >();
   auto community_itr = community_idx.begin();

   while( community_itr != community_idx.end() && 
      community_itr->membership_amount() >= share_type(0) )
   {
      const community_object& community = *community_itr;
      const community_member_object& community_member = get_community_member( community.name );

      for( auto name : community_member.members )
      {
         if( name != community_member.founder )      // All members except the founder pay membership price to the founder.
         {
            asset liquid = get_liquid_balance( name, community.membership_price.symbol );

            if( liquid.amount >= community.membership_price.amount )      // Pay daily community membership fee
            {
               adjust_liquid_balance( name, -community.membership_price );
               adjust_reward_balance( community.founder, community.membership_price );
            }
         }
      }

      ++community_itr;
   }
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
   const community_member_object& community_member_a = get_community_member( federation.community_a );
   const community_member_object& community_member_b = get_community_member( federation.community_b );
   time_point now = head_block_time();
   
   switch( federation.federation_type )
   {
      case community_federation_type::MEMBER_FEDERATION:
      {
         if( federation.share_accounts_a )       // B is accepted as A's Upstream Community
         {
            modify( community_member_a, [&]( community_member_object& cmo )
            {
               cmo.upstream_member_federations.insert( federation.community_b );
               for( auto name : community_member_b.members )
               {
                  cmo.members.insert( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_member_b.members )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_member_community( federation.community_a );
                  afo.last_updated = now;
               });
            }

            modify( community_member_b, [&]( community_member_object& cmo )
            {
               cmo.downstream_member_federations.insert( federation.community_a );
               cmo.last_updated = now;
            });
         }
         if( federation.share_accounts_b )      // A is accepted as B's Upstream Community
         {
            modify( community_member_b, [&]( community_member_object& cmo )
            {
               cmo.upstream_member_federations.insert( federation.community_a );
               for( auto name : community_member_a.members )
               {
                  cmo.members.insert( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_member_a.members )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_member_community( federation.community_b );
                  afo.last_updated = now;
               });
            }

            modify( community_member_a, [&]( community_member_object& cmo )
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
            modify( community_member_a, [&]( community_member_object& cmo )
            {
               cmo.upstream_moderator_federations.insert( federation.community_b );
               for( auto name : community_member_b.moderators )
               {
                  cmo.moderators.insert( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_member_b.moderators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_moderator_community( federation.community_a );
                  afo.last_updated = now;
               });
            }

            modify( community_member_b, [&]( community_member_object& cmo )
            {
               cmo.downstream_moderator_federations.insert( federation.community_a );
               cmo.last_updated = now;
            });
         }
         if( federation.share_accounts_b )      // A is accepted as B's Upstream Community
         {
            modify( community_member_b, [&]( community_member_object& cmo )
            {
               cmo.upstream_moderator_federations.insert( federation.community_a );
               for( auto name : community_member_a.moderators )
               {
                  cmo.moderators.insert( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_member_a.moderators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_moderator_community( federation.community_b );
                  afo.last_updated = now;
               });
            }

            modify( community_member_a, [&]( community_member_object& cmo )
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
            modify( community_member_a, [&]( community_member_object& cmo )
            {
               cmo.upstream_admin_federations.insert( federation.community_b );
               for( auto name : community_member_b.administrators )
               {
                  cmo.administrators.insert( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_member_b.administrators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_admin_community( federation.community_a );
                  afo.last_updated = now;
               });
            }

            modify( community_member_b, [&]( community_member_object& cmo )
            {
               cmo.downstream_admin_federations.insert( federation.community_a );
               cmo.last_updated = now;
            });
         }
         if( federation.share_accounts_b )      // A is accepted as B's Upstream Community
         {
            modify( community_member_b, [&]( community_member_object& cmo )
            {
               cmo.upstream_admin_federations.insert( federation.community_a );
               for( auto name : community_member_a.administrators )
               {
                  cmo.administrators.insert( name );
               }
               cmo.last_updated = now;
            });

            for( auto name : community_member_a.administrators )
            {
               const account_following_object& account_following = get_account_following( name );

               modify( account_following, [&]( account_following_object& afo )
               {
                  afo.add_admin_community( federation.community_b );
                  afo.last_updated = now;
               });
            }

            modify( community_member_a, [&]( community_member_object& cmo )
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
   const community_member_object& community_member_a = get_community_member( federation.community_a );
   const community_member_object& community_member_b = get_community_member( federation.community_b );
   time_point now = head_block_time();

   modify( community_member_a, [&]( community_member_object& cmo )
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

   modify( community_member_b, [&]( community_member_object& cmo )
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



} } //node::chain