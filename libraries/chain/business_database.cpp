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

using boost::container::flat_set;

const business_object& database::get_business( const account_name_type& account )const
{ try {
	return get< business_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const business_object* database::find_business( const account_name_type& account )const
{
   return find< business_object, by_account >( account );
}

const business_permission_object& database::get_business_permission( const account_name_type& account )const
{ try {
	return get< business_permission_object, by_account >( account );
} FC_CAPTURE_AND_RETHROW( (account) ) }

const business_permission_object* database::find_business_permission( const account_name_type& account )const
{
   return find< business_permission_object, by_account >( account );
}

const business_executive_object& database::get_business_executive( const account_name_type& business, const account_name_type& executive )const
{ try {
	return get< business_executive_object, by_business_executive >( boost::make_tuple( business, executive ) );
} FC_CAPTURE_AND_RETHROW( (business)(executive) ) }

const business_executive_object* database::find_business_executive( const account_name_type& business, const account_name_type& executive )const
{
   return find< business_executive_object, by_business_executive >( boost::make_tuple( business, executive ) );
}

const business_executive_vote_object& database::get_business_executive_vote( const account_name_type& director, const account_name_type& business )const
{ try {
	return get< business_executive_vote_object, by_director_business >( boost::make_tuple( director, business ) );
} FC_CAPTURE_AND_RETHROW( (director)(business) ) }

const business_executive_vote_object* database::find_business_executive_vote( const account_name_type& director, const account_name_type& business )const
{
   return find< business_executive_vote_object, by_director_business >( boost::make_tuple( director, business ) );
}

const business_director_object& database::get_business_director( const account_name_type& business, const account_name_type& director )const
{ try {
	return get< business_director_object, by_business_director >( boost::make_tuple( business, director ) );
} FC_CAPTURE_AND_RETHROW( (business)(director) ) }

const business_director_object* database::find_business_director( const account_name_type& business, const account_name_type& director )const
{
   return find< business_director_object, by_business_director >( boost::make_tuple( business, director ) );
}

const business_director_vote_object& database::get_business_director_vote( const account_name_type& account, const account_name_type& business, const account_name_type& director )const
{ try {
	return get< business_director_vote_object, by_account_business_director >( boost::make_tuple( account, business, director ) );
} FC_CAPTURE_AND_RETHROW( (account)(business)(director) ) }

const business_director_vote_object* database::find_business_director_vote( const account_name_type& account, const account_name_type& business, const account_name_type& director )const
{
   return find< business_director_vote_object, by_account_business_director >( boost::make_tuple( account, business, director ) );
}


/**
 * Updates the voting statistics, directors, and chief executive of the business.
 */
void database::update_business( const business_object& business )
{ try {
   const auto& business_director_vote_idx = get_index< business_director_vote_index >().indices().get< by_business_rank >();
   const auto& business_executive_vote_idx = get_index< business_executive_vote_index >().indices().get< by_director_business >();
   const auto& business_director_idx = get_index< business_director_index >().indices().get< by_business_director >();
   const auto& account_balance_idx = get_index< account_balance_index >().indices().get< by_symbol >();
   time_point now = head_block_time();

   const business_permission_object& business_permission = get_business_permission( business.account );
   flat_map< account_name_type, share_type > director_map;
   flat_map< account_name_type, share_type > executive_map;
   flat_set< account_name_type > director_set;
   flat_set< account_name_type > executive_set;
   vector< pair< account_name_type, share_type > > director_rank;
   vector< pair< account_name_type, share_type > > executive_rank;

   ilog( "Updating Business: \n ${b} \n",
      ("b",business));
   
   auto business_director_vote_itr = business_director_vote_idx.lower_bound( business.account );

   while( business_director_vote_itr != business_director_vote_idx.end() &&
      business_director_vote_itr->business == business.account )
   {
      const account_object& voter = get_account( business_director_vote_itr->account );
      share_type weight = get_equity_voting_power( business_director_vote_itr->account, business );

      while( business_director_vote_itr != business_director_vote_idx.end() && 
         business_director_vote_itr->business == business.account &&
         business_director_vote_itr->account == voter.name )
      {
         const business_director_vote_object& vote = *business_director_vote_itr;
         ++business_director_vote_itr;

         ilog( "Business: ${g} director vote (weight: ${w}): \n ${m} \n",
            ("g",business.account)("w",weight)("m",vote));

         if( director_map.count( vote.director ) )
         {
            director_map[ vote.director ] += share_type( weight.value >> vote.vote_rank );
         }
         else
         {
            // divides voting weight by 2^vote_rank, limiting total voting weight -> total voting power as votes increase.
            director_map[ vote.director ] = share_type( weight.value >> vote.vote_rank );
         }
      }
   }

   ilog( "Business: ${g} director votes: \n ${m} \n",
      ("g",business.account)("m",director_map));

   auto account_balance_itr = account_balance_idx.lower_bound( business.equity_asset );

   uint128_t equity_holder_count;

   while( account_balance_itr != account_balance_idx.end() &&
      account_balance_itr->symbol == business.equity_asset )
   {
      const account_balance_object& balance = *account_balance_itr;
      if( balance.staked_balance >= BLOCKCHAIN_PRECISION )
      {
         equity_holder_count++;
      }
      
      ++account_balance_itr;
   }

   uint64_t max_directors = approx_sqrt( equity_holder_count );

   for( auto dir : director_map )
   {
      director_rank.push_back( std::make_pair( dir.first, dir.second ) );
   }

   std::sort( director_rank.begin(), director_rank.end(), [&]( pair< account_name_type, share_type > a, pair< account_name_type, share_type > b )
   {
      return a.second < b.second;   // Ordered vector of all directors
   });

   for( auto dir : director_rank )
   {
      if( director_set.size() < max_directors )
      {
         director_set.insert( dir.first );
      }
      else
      {
         break;
      }
   }

   auto business_director_itr = business_director_idx.lower_bound( business.account );

   while( business_director_itr != business_director_idx.end() &&
      business_director_itr->business == business.account )
   {
      const business_director_object& director = *business_director_itr;

      modify( director, [&]( business_director_object& bdo )
      {
         bdo.appointed = false;     // Reset director appointed to false;
      });

      // If a director is active, and is in the top directors, and votes for an executive that is active, then appoint the director and count the executive vote.

      if( director.active )
      {
         if( std::find( director_set.begin(), director_set.end(), director.director ) != director_set.end() )
         {
            auto business_executive_vote_itr = business_executive_vote_idx.find( boost::make_tuple( director.director, business.account ) );

            if( business_executive_vote_itr != business_executive_vote_idx.end() )
            {
               const business_executive_vote_object& vote = *business_executive_vote_itr;
               const business_executive_object& executive = get_business_executive( business.account, vote.executive );

               if( executive.active )
               {
                  executive_map[ vote.executive ]++;

                  modify( director, [&]( business_director_object& bdo )
                  {
                     bdo.appointed = true;
                  });

                  ilog( "Business: ${g} appoints active director: \n ${d} \n ",
                     ("g",business.account)("m",director));
               }
            }
         }
      }

      ++business_director_itr;
   }

   ilog( "Business: ${g} executive votes: \n ${m} \n",
      ("g",business.account)("m",executive_map));

   for( auto e : executive_map )
   {
      executive_rank.push_back( std::make_pair( e.first, e.second ) );
   }
   
   std::sort( executive_rank.begin(), executive_rank.end(), [&]( pair< account_name_type, share_type > a, pair< account_name_type, share_type > b )
   {
      return a.second < b.second;  // Ordered vector of all active business executives.
   });

   account_name_type chief_executive_name = business_permission.chief_executive;

   auto executive_rank_itr = executive_rank.begin();

   if( executive_rank_itr != executive_rank.end() )
   {
      chief_executive_name = executive_rank_itr->first;
   }

   const business_executive_object& chief_executive = get_business_executive( business.account, chief_executive_name );
   
   modify( business_permission, [&]( business_permission_object& bpo )
   {
      bpo.directors = director_set;
      bpo.chief_executive = chief_executive.executive;
      bpo.last_updated = now;
   });

   ilog( "Updated Business Permissions: \n ${b} \n",
      ("b",business_permission ) );

} FC_CAPTURE_AND_RETHROW() }


/**
 * Updates the executive board votes and positions of officers in a business account.
 */
void database::update_business_set()
{ try {
   if( (head_block_num() % SET_UPDATE_BLOCK_INTERVAL) != 0 )    // Runs once per day
      return;

   const auto& business_idx = get_index< business_index >().indices().get< by_account >();
   auto business_itr = business_idx.begin();

   while( business_itr != business_idx.end() )
   {
      update_business( *business_itr );
      ++business_itr;
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Aligns business account officer votes in a continuous order
 */
void database::update_business_director_votes( const account_object& account, const account_name_type& business )
{ try {
   const auto& vote_idx = get_index< business_director_vote_index >().indices().get< by_account_business_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->business == business )
   {
      const business_director_vote_object& vote = *vote_itr;

      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( business_director_vote_object& bdvo )
         {
            bdvo.vote_rank = new_vote_rank;
            bdvo.last_updated = now;
         });
      }
      new_vote_rank++;
      ++vote_itr;  
   }
} FC_CAPTURE_AND_RETHROW() }


/**
 * Aligns business account officer votes in a continuous order, and inputs a new vote
 * at a specified vote number.
 */
void database::update_business_director_votes( const account_object& account, const account_name_type& business, 
   const account_object& director, uint16_t input_vote_rank )
{ try {
   const auto& vote_idx = get_index< business_director_vote_index >().indices().get< by_account_business_rank >();
   auto vote_itr = vote_idx.lower_bound( boost::make_tuple( account.name, business ) );
   time_point now = head_block_time();

   uint16_t new_vote_rank = 1;

   while( vote_itr != vote_idx.end() && 
      vote_itr->account == account.name &&
      vote_itr->business == business )
   {
      const business_director_vote_object& vote = *vote_itr;

      if( vote.vote_rank == input_vote_rank )
      {
         new_vote_rank++;
      }
      if( vote.vote_rank != new_vote_rank )
      {
         modify( vote, [&]( business_director_vote_object& bdvo )
         {
            bdvo.vote_rank = new_vote_rank;
            bdvo.last_updated = now;
         });
      }
      new_vote_rank++;
      ++vote_itr;  
   }

   create< business_director_vote_object >([&]( business_director_vote_object& bdvo )
   {
      bdvo.account = account.name;
      bdvo.director = director.name;
      bdvo.business = business;
      bdvo.vote_rank = input_vote_rank;
      bdvo.last_updated = now;
      bdvo.created = now;
   });
} FC_CAPTURE_AND_RETHROW() }

} } //node::chain