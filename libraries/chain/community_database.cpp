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

const community_member_key_object& database::get_community_member_key( const account_name_type& member, const community_name_type& community )const
{ try {
	return get< community_member_key_object, by_member_community >( boost::make_tuple( member, community) );
} FC_CAPTURE_AND_RETHROW( (community) ) }

const community_member_key_object* database::find_community_member_key( const account_name_type& member, const community_name_type& community )const
{
   return find< community_member_key_object, by_member_community >( boost::make_tuple( member, community) );
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
   const auto& vote_idx = get_index< community_moderator_vote_index >().indices().get< by_community_moderator >();
   flat_map< account_name_type, share_type > mod_weight;
   share_type total = 0;
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
 * Returns True when the account is a member of the community, or one of its Federated Communities.
 */
bool database::is_federated_member( const community_member_object& m, const account_name_type a )
{
   const auto& community_idx = get_index< community_member_index >().indices().get< by_name >();
   auto community_itr = community_idx.begin();

   flat_set< account_name_type > members = m.members;
   
   for( auto f : m.member_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->members )
         {
            members.insert( name );
         }
      }
   }

   for( auto f : m.moderator_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->members )
         {
            members.insert( name );
         }
      }
   }

   for( auto f : m.admin_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->members )
         {
            members.insert( name );
         }
      }
   }

   return std::find( members.begin(), members.end(), a ) != members.end();
}

/**
 * Returns True when the account is a moderator of the community, or one of its Federated Communities.
 */
bool database::is_federated_moderator( const community_member_object& m, const account_name_type a )
{
   const auto& community_idx = get_index< community_member_index >().indices().get< by_name >();
   auto community_itr = community_idx.begin();

   flat_set< account_name_type > moderators = m.moderators;

   for( auto f : m.moderator_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->moderators )
         {
            moderators.insert( name );
         }
      }
   }

   for( auto f : m.admin_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->moderators )
         {
            moderators.insert( name );
         }
      }
   }

   return std::find( moderators.begin(), moderators.end(), a ) != moderators.end();
}

/**
 * Returns True when the account is an administrator of the community, or one of its Federated Communities.
 */
bool database::is_federated_administrator( const community_member_object& m, const account_name_type a )
{
   const auto& community_idx = get_index< community_member_index >().indices().get< by_name >();
   auto community_itr = community_idx.begin();

   flat_set< account_name_type > administrators = m.administrators;
   
   for( auto f : m.admin_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->administrators )
         {
            administrators.insert( name );
         }
      }
   }

   return std::find( administrators.begin(), administrators.end(), a ) != administrators.end();
}

/**
 * Returns True when the account is a blacklisted account of the community, or one of its Federated Communities.
 */
bool database::is_federated_blacklisted( const community_member_object& m, const account_name_type a )
{
   const auto& community_idx = get_index< community_member_index >().indices().get< by_name >();
   auto community_itr = community_idx.begin();
   
   flat_set< account_name_type > blacklist = m.blacklist;
   
   for( auto f : m.member_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->blacklist )
         {
            blacklist.insert( name );
         }
      }
   }

   for( auto f : m.moderator_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->blacklist )
         {
            blacklist.insert( name );
         }
      }
   }

   for( auto f : m.admin_federations )
   {
      community_itr = community_idx.find( f );
      if( community_itr != community_idx.end() )
      {
         for( auto name : community_itr->blacklist )
         {
            blacklist.insert( name );
         }
      }
   }

   return std::find( blacklist.begin(), blacklist.end(), a ) != blacklist.end();
}

/**
 * Returns True when the account is authorized to interact with the community, or one of its Federated Communities.
 */
bool database::is_federated_authorized_interact( const community_member_object& m, const account_name_type a )
{
   if( is_federated_blacklisted( m, a ) )
   {
      return false;
   }

   switch( m.community_privacy )
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
         return is_federated_member( m, a );
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid community privacy: ${t}.",
            ("t",m.community_privacy) );
      }
   }
}

/**
 * Returns True when the account is authorized to create a root post within the community, or one of its Federated Communities.
 */
bool database::is_federated_authorized_author( const community_member_object& m, const account_name_type a )
{
   if( is_federated_blacklisted( m, a ) )
   {
      return false;
   }

   switch( m.community_privacy )
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
         return is_federated_member( m, a );
      }
      break;
      case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
      {
         return is_federated_moderator( m, a );
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid community privacy: ${t}.",
            ("t",m.community_privacy) );
      }
   }
}

/**
 * Returns True when the account is authorized to request to join the community, or one of its Federated Communities.
 */
bool database::is_federated_authorized_request( const community_member_object& m, const account_name_type a )
{
   if( is_federated_blacklisted( m, a ) )
   {
      return false;
   }

   switch( m.community_privacy )
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
            ("t",m.community_privacy) );
      }
   }
}


/**
 * Returns True when the account is authorized to create an invitation to join the community, or one of its Federated Communities.
 */
bool database::is_federated_authorized_invite( const community_member_object& m, const account_name_type a )
{
   if( is_federated_blacklisted( m, a ) )
   {
      return false;
   }

   switch( m.community_privacy )
   {
      case community_privacy_type::OPEN_PUBLIC_COMMUNITY:
      case community_privacy_type::GENERAL_PUBLIC_COMMUNITY:
      case community_privacy_type::EXCLUSIVE_PUBLIC_COMMUNITY:
      {
         return is_federated_member( m, a );
      }
      break;
      case community_privacy_type::CLOSED_PUBLIC_COMMUNITY:
      case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
      case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
      case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
      {
         return is_federated_moderator( m, a );
      }
      break;
      case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
      {
         return is_federated_administrator( m, a );
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid community privacy: ${t}.",
            ("t",m.community_privacy) );
      }
   }
}

bool database::is_federated_authorized_blacklist( const community_member_object& m, const account_name_type a )
{
   if( is_federated_blacklisted( m, a ) )
   {
      return false;
   }

   switch( m.community_privacy )
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
         return is_federated_moderator( m, a );
      }
      break;
      case community_privacy_type::OPEN_PRIVATE_COMMUNITY:
      case community_privacy_type::GENERAL_PRIVATE_COMMUNITY:
      case community_privacy_type::EXCLUSIVE_PRIVATE_COMMUNITY:
      case community_privacy_type::CLOSED_PRIVATE_COMMUNITY:
      {
         return is_federated_administrator( m, a );
      }
      break;
      default:
      {
         FC_ASSERT( false, "Invalid community privacy: ${t}.",
            ("t",m.community_privacy) );
      }
   }
}


} } //node::chain